/*
memory/memory.c - General memory functions

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
#include "memory.h"
#include "mapping.h"
#include "cache.h"

namespace emumiga {
namespace memory {

int readData(uint32_t addr, uint32_t length, uint8_t *buffer)
{
	mapping *entry;
	uint32_t offset;
	int ret;

	while(length > 0){
		entry = mapping::findEnclosingVAddr(addr);
		if(entry == NULL)
			return 1;
		offset = addr - entry->vaddr_start;
		ret = entry->read(offset, 1, buffer);
		if(ret != 0)
			return ret;

		addr++;
		length--;
		buffer++;
	}
	return 0;
}

int writeData(uint32_t addr, uint32_t length, uint8_t *buffer)
{
	mapping *entry;
	uint32_t offset;
	int ret;

	while(length > 0){
		entry = mapping::findEnclosingVAddr(addr);
		if(entry == NULL)
			return 1;
		offset = addr - entry->vaddr_start;
		ret = entry->write(offset, 1, buffer);
		if(ret != 0)
			return ret;

		addr++;
		length--;
		buffer++;
	}
	return 0;
}

int readData_cached(uint32_t addr, uint32_t length, uint8_t *buffer, cache *cache)
{
	mapping *entry;
	uint32_t offset;
	int ret;

	while(length > 0){
		entry = cache->findEnclosingVAddr(addr);
		if(entry == NULL)
			return 1;
		offset = addr - entry->vaddr_start;
		ret = entry->read(offset, 1, buffer);
		if(ret != 0)
			return ret;

		addr++;
		length--;
		buffer++;
	}
	return 0;
}

int writeData_cached(uint32_t addr, uint32_t length, uint8_t *buffer, cache *cache)
{
	mapping *entry;
	uint32_t offset;
	int ret;

	while(length > 0){
		entry = cache->findEnclosingVAddr(addr);
		if(entry == NULL)
			return 1;
		offset = addr - entry->vaddr_start;
		ret = entry->write(offset, 1, buffer);
		if(ret != 0)
			return ret;

		addr++;
		length--;
		buffer++;
	}
	return 0;
}

void dump()
{
/*
	int idx;
	mapping *cur;
	char const *pname;
	uint32_t start, length, type;
	mmu_pool_t *pool;
	void *real;
	mmu_symbol_t *symbol;

	dprintf("-------------------\n");
	dprintf("- MEMORY MAP DUMP -\n");
	dprintf("-------------------\n");

	// Iterate through the table
	for(idx=0;idx<MMU_TABLESIZE;idx++){
		cur = &(mmu_table->entry[idx]);
		if(cur->length != 0){
			// Ok found a valid entry
			start = cur->startAddr;
			length = cur->length;
			type = cur->type;
			real = cur->real;

			pool = cur->pool;
			pname = "";
			if(pool != NULL) {
				pname = pool->name;
			} else {
				symbol = cur->symbols;
				while(symbol != NULL) {
					if(symbol->addr == start) {
						pname = symbol->name;
					}
					symbol = symbol->next;
				}
			}

			dprintf("0x%08x: l=%u, t=0x%08x, r=%p - %s\n",start,length,type,real,pname);
		}
	}

	dprintf("-------------------\n");
*/
}

void clearRange(
	uint32_t start __attribute__((unused)),
	uint32_t length __attribute__((unused))
)
{
/*
	int idx, goOn, count;
	mapping *cur, *best;
	uint32_t end;

	end = start + length;

	count = 0;
	goOn = 1;
	while(goOn) {
		// Iterate through the table and look for entries in
		// the range
		for(idx=0;idx<MMU_TABLESIZE;idx++){
			cur = &(mmu_table->entry[idx]);
			if(cur->length > 0){
				// Not empty

				// Below threshold?
				if(cur->type < MMU_PRIO_CLEAR)
					continue;

				// Inside our range?
				if(
					cur->startAddr < end &&
					start < (cur->startAddr + cur->length)
				){

					// Check if there is a higher
					// prioritized mapping overlapping
					best = mmu_findEntry(cur->startAddr);

					// Found something ?
					if(cur == best) {
						// No, we are top dog. Clear
						mmu_delEntry(cur);

						// Keep count
						count++;

						// This might cause more than
						// one change to the mmu array.
						// We must restart the seek.
						idx = -1;
						break;
					}
				}
			}
		}
		// Are we done?
		if(idx == MMU_TABLESIZE)
			goOn = 0;
	}

	DEBUG(5) {
		if(count > 0) {
			dprintf("mmu: mmu_clearRange(): Cleared %d entries\n", count);
		}
	}
*/
}

} // namespace memory
} // namespace emumiga
