/*
memory/mapping_list.cpp - Skip-list collection for mappings

Copyright (C) 2010  Magnus Öberg

The license for this program is dual license:
  AROS Public License, version 1.1 or later (APL1.1+)
AND
  GNU Lesser General Public License, version 2.1 or later (LGPL2.1+)

Dual licensing means for this program that anyone that wants to use,
modify or distribute all or parts of this program can choose the best
suiting license of APL1.1+ or LGPL2.1+ and must follow the terms described
in that license. Choosing only one license disables the other license and
references to the disabled license in code and documentation may be removed.
This text paragraph should be removed at the same time. It is also permitted
to keep this exact dual licensing. The copyrights are not affected by
selecting only one license and remain in full.

-- APL conditions --

  The contents of this file are subject to the AROS Public License Version 1.1
  (the "License"); you may not use this file except in compliance with the
  License. You may obtain a copy of the License at

  http://www.aros.org/license.html

  Software distributed under the License is distributed on an "AS IS" basis,
  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for
  the specific language governing rights and limitations under the License.

-- LGPL conditions --

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA  02110-1301  USA

--
*/

#include "../debug.h"
#include "mapping_list.h"

namespace emumiga {
namespace memory {

mapping_list::block::block()
{
	int i;

	prev = NULL;
	next = NULL;
	snext = NULL;
	count = 0;

	for( i = 0 ; i < MMU_SLIST_SIZE ; i++ )
	{
		vaddr[i] = 0;
		real[i] = NULL;
		entry[i] = NULL;
	}
}

mapping_list::iterator::iterator(block *_block, int _index)
{
	bl = _block;
	index = _index;
}

mapping *mapping_list::iterator::current()
{
	if(bl == NULL)
		return NULL;

	if(index < 0 || index >= bl->count)
		return NULL;

	return bl->entry[index];
}

int mapping_list::iterator::moveNext()
{
	if(index < 0)
		index = -1;

	while(1) {
		if(bl == NULL)
			return 1;

		index++;

		if(index < bl->count)
			break;

		bl = bl->next;
		index = -1;
	}

	return 0;
}

int mapping_list::iterator::movePrev()
{

	while(1) {
		if(bl == NULL)
			return 1;

		if(index > bl->count)
			index = bl->count;

		index--;

		if(index >= 0)
			break;

		bl = bl->prev;
		index = MMU_SLIST_SIZE;
	}

	return 0;
}

mapping_list::block *mapping_list::best_block_vaddr(
	mapping_list::block *root,
	uint32_t vaddr
)
{
	mapping_list::block *block, *bestblock;

	if(root == NULL)
		return NULL;

	/* Keep track of best candidate so far */
	bestblock = root;

	/* Travese big steps */
	block = bestblock->snext;
	while(block != NULL) {

		/* Passed it? */
		if(block->vaddr[0] > vaddr)
			break;

		bestblock = block;
		block = bestblock->snext;
	}

	/* Traverse small steps */
	block = bestblock->next;
	while(block != NULL) {

		/* Passed it? */
		if(block->vaddr[0] > vaddr)
			break;

		bestblock = block;
		block = bestblock->next;
	}

	return bestblock;
}

mapping_list::block *mapping_list::best_block_real(
	mapping_list::block *root,
	APTR real
)
{
	mapping_list::block *block, *bestblock;

	if(root == NULL)
		return NULL;

	/* Keep track of best candidate so far */
	bestblock = root;

	/* Travese big steps */
	block = bestblock->snext;
	while(block != NULL) {

		/* Passed it? */
		if(block->real[0] > real)
			break;

		bestblock = block;
		block = bestblock->snext;
	}

	/* Traverse small steps */
	block = bestblock->next;
	while(block != NULL) {

		/* Passed it? */
		if(block->real[0] > real)
			break;

		bestblock = block;
		block = bestblock->next;
	}

	return bestblock;
}

mapping *mapping_list::find_entry_prio(uint32_t vaddr, int prio)
{
	int j;
	mapping *entry;
	mapping_list::block *bestblock;

	/* Get best matching block */
	bestblock = best_block_vaddr(&root_vaddr[prio], vaddr);

	/* Find the best item */
	for(j = bestblock->count-1 ; j >= 0 ; j--) {
		if(bestblock->vaddr[j] <= vaddr)
			break;
	}

	// Nothing found?
	if(j < 0)
		return NULL;

	// Is this entry covering the address?
	entry = bestblock->entry[j];

	if(entry == NULL) {
		dprintf("Error: mapping_list::find_entry_prio(): Corruption! vaddr=%x\n", vaddr);
		return NULL;
	}
	if(
		entry->vaddr_start <= vaddr &&
		vaddr < (entry->vaddr_start + entry->vaddr_length)
	) {
		return entry;
	}

	// No match
	return NULL;
}

mapping *mapping_list::findEnclosingVAddr(uint32_t vaddr)
{
	int prio;
	mapping *entry;

	/* Go through all system object priorities starting on the highest. */
	for(prio = MMU_PRIO_GET_LEVEL(MMU_PRIO_SYSTEM)-1 ; prio >= 0 ; prio--) {

		entry = find_entry_prio(vaddr, prio);

		if(entry != NULL)
			return entry;
	}

	/* Go through all normal memory mappings */
	entry = find_entry_prio(vaddr, MMU_PRIO_GET_LEVEL(MMU_PRIO_MEM));
	if(entry != NULL)
		return entry;

	/* Go through root mappings */
	entry = find_entry_prio(vaddr, MMU_PRIO_GET_LEVEL(MMU_PRIO_ROOT));
	if(entry != NULL)
		return entry;

	/* No match in any priority */
	return NULL;
}

mapping *mapping_list::findEnclosingVAddrLimits(uint32_t vaddr, uint32_t *start, uint32_t *end)
{
	int prio;
	mapping *entry, *cur;
	mapping_list::block *bl;
	uint32_t lo_vaddr, hi_vaddr;

	// Find enclosing entry if exists
	entry = findEnclosingVAddr(vaddr);

	if(entry == NULL)
		return NULL;

	prio = MMU_PRIO_GET_LEVEL(entry->type);

	lo_vaddr = entry->vaddr_start;
	hi_vaddr = entry->vaddr_start + entry->vaddr_length - 1;

	if(prio <= MMU_PRIO_GET_LEVEL(MMU_PRIO_SYSTEM)) {
		// If we hit a system object, it is never
		// overshadowed by anything
		prio = 0;
	}

	// Prio above MMU_PRIO_SYSTEM can be overshadowed
	// Go through all levels below
	for(prio = prio-1 ; prio >= 0 ; prio--) {

		/* Get best matching block */
		bl = best_block_vaddr(&root_vaddr[prio], vaddr);

		if(bl == NULL)
			continue;

		// Use iterator
		iterator iter = iterator(bl, 0);

		// Go back so that the current entry ends before
		// our block starts
		cur = iter.current();
		while((cur->vaddr_start + cur->vaddr_length - 1) >= lo_vaddr) {
			if(!iter.movePrev())
				break;
			cur = iter.current();
		}

		// Check all entries that start before vaddr
		while(cur->vaddr_start < vaddr) {

			// Does it shadow the lower part?
			if(lo_vaddr < (cur->vaddr_start + cur->vaddr_length)) {
				lo_vaddr = cur->vaddr_start + cur->vaddr_length;
			}

			if(!iter.moveNext())
				break;
			cur = iter.current();
		}

		// Ok we have passed vaddr, check if hi_vaddr should
		// be adjusted
		if(cur->vaddr_start <= hi_vaddr) {
			hi_vaddr = cur->vaddr_start - 1;
		}
	}

	*start = lo_vaddr;
	*end = hi_vaddr;
	return entry;
}

mapping *mapping_list::findTypeByVAddr(uint32_t type, uint32_t vaddr)
{
	mapping *entry;
	entry = find_entry_prio(vaddr, MMU_PRIO_GET_LEVEL(type));

	// Demand exact hit in this method
	if(entry->vaddr_start != vaddr)
		return NULL;

	// Wrong kind?
	if(entry->type != type)
		return NULL;

	return entry;
}

mapping *mapping_list::findTypeByReal(uint32_t type, APTR real)
{
	int prio, j;
	mapping *entry;
	mapping_list::block *bestblock;

	prio = MMU_PRIO_GET_LEVEL(type);

	/* Get best matching block */
	bestblock = best_block_real(&root_real[prio], real);

	/* Find the best item */
	for(j = bestblock->count-1 ; j >= 0 ; j--) {
		if(bestblock->real[j] == real) {
			/* Found it */
			break;
		}
	}

	// Nothing found?
	if(j < 0)
		return NULL;

	entry = bestblock->entry[j];

	if(entry == NULL) {
		dprintf("Error: mapping_list::find_real(): Corruption! real=%p\n", real);
		return NULL;
	}

	// Wrong kind?
	if(entry->type != type)
		return NULL;

	return entry;
}

int mapping_list::add(mapping *entry)
{
	int prio, j;
	mapping_list::block *bestblock, *newblock;
	uint32_t vaddr;
	APTR real;

	/* Get the correct prio */
	prio = MMU_PRIO_GET_LEVEL(entry->type);

	/* ------------------------------ */
	/* --- Step 1, VADDR indexing --- */
	/* ------------------------------ */

	vaddr = entry->vaddr_start;

	/* Get best matching block */
	bestblock = best_block_vaddr(&root_vaddr[prio], vaddr);

	/* Enough space left? */
	if(bestblock->count == MMU_SLIST_SIZE) {

		/* Oh no, we must split */
		newblock = new block();
		if(newblock == NULL)
			return 1;

		/* Move the items above the threshold into the new block */
		for(j = MMU_SLIST_THRESHOLD ; j < MMU_SLIST_SIZE ; j++) {
			newblock->vaddr[j-MMU_SLIST_THRESHOLD] = bestblock->vaddr[j];
			newblock->entry[j-MMU_SLIST_THRESHOLD] = bestblock->entry[j];
			newblock->count++;
			bestblock->count--;

		}

		/* Link it in */
		newblock->prev = bestblock;
		newblock->next = bestblock->next;
		newblock->snext = bestblock->snext;
		bestblock->next = newblock;
		if(newblock->next != NULL) {
			newblock->next->prev = newblock;
		}

		/* Reindex the skip pointers */
		reindex(&root_vaddr[prio]);

		/* Back to business, should we use the new block
		   for the new item? */
		if(newblock->vaddr[0] < vaddr)
			bestblock = newblock;
	}

	/* bestblock is the one to use, and it has at least one free slot */

	/* Traverse the items backwards */
	for(j = bestblock->count-1; j >= 0 ; j--) {

		/* Found the right place to insert? */
		if(vaddr > bestblock->vaddr[j]) {
			/* Yep, put it and stop */
			bestblock->vaddr[j+1] = vaddr;
			bestblock->entry[j+1] = entry;
			break;
		}

		/* Not the right place, move the current item up one place */
		bestblock->vaddr[j+1] = bestblock->vaddr[j];
		bestblock->entry[j+1] = bestblock->entry[j];
	}

	/* Special case, first slot */
	if(j < 0) {
		bestblock->vaddr[0] = vaddr;
		bestblock->entry[0] = entry;
	}

	/* One added */
	bestblock->count++;

	/* ----------------------------- */
	/* --- Step 2, REAL indexing --- */
	/* ----------------------------- */

	/* Do we have a real pointer? */
	real = entry->real_start;
	if(real == NULL)
		/* No but that is ok */
		return 0;

	/* We have a real pointer, do it all again */

	/* Get best matching block */
	bestblock = best_block_real(&root_real[prio], real);

	/* Enough space left? */
	if(bestblock->count == MMU_SLIST_SIZE) {

		/* Oh no, we must split */
		newblock = new block();
		if(newblock == NULL)
			return 1;

		/* Move the items above the threshold into the new block */
		for(j = MMU_SLIST_THRESHOLD ; j < MMU_SLIST_SIZE ; j++) {
			newblock->real[j-MMU_SLIST_THRESHOLD] = bestblock->real[j];
			newblock->entry[j-MMU_SLIST_THRESHOLD] = bestblock->entry[j];
			newblock->count++;
			bestblock->count--;
		}

		/* Link it in */
		newblock->prev = bestblock;
		newblock->next = bestblock->next;
		newblock->snext = bestblock->snext;
		bestblock->next = newblock;
		if(newblock->next != NULL) {
			newblock->next->prev = newblock;
		}

		/* Reindex the skip pointers */
		reindex(&root_real[prio]);

		/* Back to business, should we use the new block
		   for the new item? */
		if(newblock->real[0] < real)
			bestblock = newblock;
	}

	/* bestblock is the one to use, and it has at least one free slot */

	/* Traverse the items backwards */
	for(j = bestblock->count-1 ; j >= 0 ; j--) {

		/* Found the right place to insert? */
		if(real > bestblock->real[j]) {
			/* Yep, put it and stop */
			bestblock->real[j+1] = real;
			bestblock->entry[j+1] = entry;
			break;
		}

		/* Not the right place, move the current item up one place */
		bestblock->real[j+1] = bestblock->real[j];
		bestblock->entry[j+1] = bestblock->entry[j];
	}

	/* Special case, first slot */
	if(j < 0) {
		bestblock->real[0] = real;
		bestblock->entry[0] = entry;
	}

	/* One added */
	bestblock->count++;

	return 0;
}

int mapping_list::remove(mapping *entry)
{
	int prio, i, j;
	mapping_list::block *bestblock, *nextblock;
	uint32_t vaddr;
	APTR real;

	/* Get the correct prio */
	prio = MMU_PRIO_GET_LEVEL(entry->type);

	/* ------------------------------ */
	/* --- Step 1, VADDR indexing --- */
	/* ------------------------------ */

	vaddr = entry->vaddr_start;

	/* Get best matching block */
	bestblock = best_block_vaddr(&root_vaddr[prio], vaddr);

	/* Found the best block, find the item */
	for(j = bestblock->count-1; j >= 0 ; j--) {
		if(bestblock->entry[j] == entry) {
			break;
		}
	}

	/* Nothing found?! */
	if(j < 0)
		return 1;

	/* Remove our item by moving those above down one slot */
	for(i = j+1 ; i < bestblock->count ; i++) {
		bestblock->vaddr[i-1] = bestblock->vaddr[i];
		bestblock->entry[i-1] = bestblock->entry[i];
	}

	/* One removed */
	bestblock->count--;

	/* Is there a next block? */
	nextblock = bestblock->next;
	if(nextblock != NULL) {
		/* Should we join them? */
		if(bestblock->count + nextblock->count < MMU_SLIST_THRESHOLD) {
			/* Ok, we should join them */

			i = bestblock->count;
			for(j = 0 ; j < nextblock->count ; j++) {
				bestblock->vaddr[i] = nextblock->vaddr[j];
				bestblock->entry[i] = nextblock->entry[j];
				i++;
			}
			bestblock->count = i;

			/* Unlink next node */
			bestblock->next = nextblock->next;
			if(nextblock->next != NULL) {
				nextblock->next->prev = bestblock;
			}

			/* We MUST reindex, there can be a skip pointer
			   pointing at nextblock */
			reindex(&root_vaddr[prio]);

			/* Free block */
			delete nextblock;
		}
	}

	/* ----------------------------- */
	/* --- Step 2, REAL indexing --- */
	/* ----------------------------- */

	/* Do we have a real pointer? */
	real = entry->real_start;
	if(real == NULL)
		/* No but that is ok */
		return 0;

	/* We have a real pointer, do it all again */

	/* Get best matching block */
	bestblock = best_block_real(&root_real[prio], real);

	/* Found the best block, find the item */
	for(j = bestblock->count-1 ; j >= 0 ; j--) {
		if(bestblock->entry[j] == entry) {
			break;
		}
	}

	/* Nothing found?! */
	if(j < 0)
		return 1;

	/* Remove our item by moving those above down one slot */
	for(i= j+1 ; i < bestblock->count ; i++) {
		bestblock->real[i-1] = bestblock->real[i];
		bestblock->entry[i-1] = bestblock->entry[i];
	}

	/* One removed */
	bestblock->count--;

	/* Is there a next block? */
	nextblock = bestblock->next;
	if(nextblock != NULL) {
		/* Should we join them? */
		if(bestblock->count + nextblock->count < MMU_SLIST_THRESHOLD) {
			/* Ok, we should join them */

			i = bestblock->count;
			for(j=0;j < nextblock->count; j++) {
				bestblock->real[i] = nextblock->real[j];
				bestblock->entry[i] = nextblock->entry[j];
				i++;
			}
			bestblock->count = i;

			/* Unlink next node */
			bestblock->next = nextblock->next;
			if(nextblock->next != NULL) {
				nextblock->next->prev = bestblock;
			}

			/* We MUST reindex, there can be a skip pointer
			   pointing at nextblock */
			reindex(&root_real[prio]);

			/* Free block */
			delete nextblock;
		}
	}

	return 0;
}

void mapping_list::reindex(mapping_list::block *block)
{
	mapping_list::block *history[MMU_SLIST_SKIP];
	int i,hidx;

	/* history contains the latest blocks, a FIFO */
	for(i = 0 ; i < MMU_SLIST_SKIP ; i++) {
		history[i] = NULL;
	}
	hidx = 0;

	while(block != NULL) {

		/* Is there something falling out of the FIFO? */
		if(history[hidx] != NULL) {
			/* Yep, and it's skip next should point here */
			history[hidx]->snext = block;
		}

		/* Push this one into the FIFO */
		history[hidx] = block;

		/* Our skip next will be set when this item falls out
		   of the FIFO later, but set it to NULL for the moment */
		block->snext = NULL;

		/* Keep the counter correct */
		hidx++;
		if(hidx == MMU_SLIST_SKIP)
			hidx = 0;

		/* Go on */
		block = block->next;

	}
}

bool mapping_list::validate(mapping *entry)
{
	int prio, idx;
	uint32_t lo_vaddr, hi_vaddr;
	block *bl;
	mapping *cur;

	// The live mappings must maintain a strict hierarchical order.
        // A new mapping must be checked with all other live mappings
	// according to these rules:
	//
	// If the new mapping has higher priority than the compared one:
        //   The new mapping must be totally separated from the compared one
        //   or the new mapping must totally enclose the compared one
	//
	// If the new mapping has the same priority than the compared one:
        //   The new mapping must be totally separated from the compared one
	//
	// If the new mapping has lower priority than the compared one:
        //   The new mapping must be totally separated from the compared one
        //   or the new mapping must be totally enclosed by the compared one
	//
	// These rules must be fulfilled between ALL live mappings
	// Zero sized mappings are not allowed

	// Must have a length
	if(entry->vaddr_length <= 0)
		return 1;

	// Get the correct prio
	prio = MMU_PRIO_GET_LEVEL(entry->type);

	lo_vaddr = entry->vaddr_start;
	hi_vaddr = lo_vaddr + entry->vaddr_length - 1;

	// Iterate over all lower prios
	for( idx = 0 ; idx < prio ; idx++ ){

		// Look up a block to start from.
		bl = best_block_vaddr(&root_vaddr[idx], lo_vaddr);

		if(bl == NULL)
			continue;

		// Use iterator
		iterator iter = iterator(bl, 0);

		// Go back so that the current entry ends before
		// our block starts
		cur = iter.current();
		while((cur->vaddr_start + cur->vaddr_length - 1) >= lo_vaddr) {
			if(!iter.movePrev())
				break;
			cur = iter.current();
		}

		// Check all entries that start before lo_vaddr
		while(cur->vaddr_start < lo_vaddr) {

			// Must end before our block
			if((cur->vaddr_start + cur->vaddr_length - 1) >= lo_vaddr)
				return 1;

			if(!iter.moveNext())
				break;
			cur = iter.current();
		}

		// Check all entries that start on or after lo_vaddr but
		// before or on hi_vaddr
		while(cur->vaddr_start >= lo_vaddr && cur->vaddr_start <= hi_vaddr) {

			// Must end at latest on our hi_vaddr
			if((cur->vaddr_start + cur->vaddr_length - 1) > hi_vaddr)
				return 1;

			if(!iter.moveNext())
				break;
			cur = iter.current();
		}

		// Entries startins after hi_vaddr are all ok
	}

	// Check same prio
	idx = prio;

	// Look up a block to start from.
	bl = best_block_vaddr(&root_vaddr[idx], lo_vaddr);

	if(bl != NULL) {

		// Use iterator
		iterator iter = iterator(bl, 0);

		// Go back so that the current entry ends before
		// our block starts
		cur = iter.current();
		while((cur->vaddr_start + cur->vaddr_length - 1) >= lo_vaddr) {
			if(!iter.movePrev())
				break;
			cur = iter.current();
		}

		// Check all entries that start before lo_vaddr
		while(cur->vaddr_start < lo_vaddr) {

			// Must end before our block
			if((cur->vaddr_start + cur->vaddr_length - 1) >= lo_vaddr)
				return 1;

			if(!iter.moveNext())
				break;
			cur = iter.current();
		}

		// Fail if current entry start on or after lo_vaddr but
		// before or on hi_vaddr
		if(cur->vaddr_start >= lo_vaddr && cur->vaddr_start <= hi_vaddr)
			return 1;

		// Entries starting after hi_addr are all ok
	}

	// Iterate over all higher prios
	for( idx = prio+1 ; idx < MMU_PRIO_LEVELS ; idx++ ){

		// Look up a block to start from.
		bl = best_block_vaddr(&root_vaddr[idx], lo_vaddr);

		if(bl == NULL)
			continue;

		// Use iterator
		iterator iter = iterator(bl, 0);

		// Go back so that the current entry ends before
		// our block starts
		cur = iter.current();
		while((cur->vaddr_start + cur->vaddr_length - 1) >= lo_vaddr) {
			if(!iter.movePrev())
				break;
			cur = iter.current();
		}

		// Check all entries that start before or on lo_vaddr
		while(cur->vaddr_start <= lo_vaddr) {

			// Is it ending before hi_vaddr?
			if((cur->vaddr_start + cur->vaddr_length - 1) < hi_vaddr) {
				// It must end before lo_vaddr
				if((cur->vaddr_start + cur->vaddr_length - 1) >= lo_vaddr)
					return 1;
			}

			if(!iter.moveNext())
				break;
			cur = iter.current();
		}

		// Fail if current entry start after lo_vaddr but
		// before or on hi_vaddr
		if(cur->vaddr_start > lo_vaddr && cur->vaddr_start <= hi_vaddr)
			return 1;

		// Entries starting after hi_addr are all ok
	}

	// All OK
	return 0;
}

} // namespace memory
} // namespace emumiga
