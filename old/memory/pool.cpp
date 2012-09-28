/*
mmu/pool.c - Mapping object memory pool handling

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

#include <proto/exec.h>

#include "../vmem.h"
#include "../debug.h"
#include "mapping.h"

namespace emumiga {
namespace memory {

/* Predecl */
static void mmu_pool_destructor(mapping *entry);
static void mmu_pool_destructor_vaddr(mapping *entry);

static mmu_pool_t *root_pool;
static int is_init=0;

static void **getSlotPtr(mmu_pool_t *pool)
{
	mmu_pool_chunk_t *chunk;
	int i;

	/* Find an empty slot in the pool chunk list */
	chunk = pool->first_chunk;
	while(1) {
		if(chunk == NULL) {
			/* Out of chunks, allocate a new */
			chunk = (mmu_pool_chunk_t *)AllocVec(sizeof(mmu_pool_chunk_t),MEMF_PUBLIC|MEMF_CLEAR);
			if(chunk == NULL)
				return NULL;
			chunk->next = pool->first_chunk;
			pool->first_chunk = chunk;
		}
		for(i=0;i<MMU_POOL_CHUNKSIZE;i++){
			if(chunk->objects[i] == NULL) {
				return &(chunk->objects[i]);
			}
		}
		chunk = chunk->next;
	}
	return NULL;
}

static void freeSlot(mapping *entry)
{
	mmu_pool_chunk_t *chunk;
	int i;

	chunk = entry->pool->first_chunk;
	while(1) {
		if(chunk == NULL)
			return;

		for(i=0;i<MMU_POOL_CHUNKSIZE;i++){
			if(chunk->objects[i] == entry->nativePtr) {
				chunk->objects[i] = NULL;
				return;
			}
		}
		chunk = chunk->next;
	}
}

int mmu_pool_init()
{
	DEBUG(3) dprintf("mmu: mmu_pool_init() called\n");

	root_pool = NULL;

	is_init = 1;
	return 0;
}

void mmu_pool_cleanup()
{
        DEBUG(3) dprintf("mmu: mmu_pool_cleanup() called\n");

        if( !is_init )
                return;

	is_init = 0;
}

mmu_pool_t *mmu_pool_create(char const *name, int type, int vsize, int objsize)
{
	mmu_pool_t *pool;

	DEBUG(4) dprintf("mmu: mmu_pool_create() called. Object name: %s\n",name);

	pool = (mmu_pool_t *)AllocVec(sizeof(mmu_pool_t),MEMF_PUBLIC|MEMF_CLEAR);

	if(pool == NULL)
		return NULL;

	pool->name = name;
	pool->type = type;
	pool->vsize = vsize;
	pool->objsize = objsize;

	pool->next = root_pool;
	root_pool = pool;

	return pool;
}

void mmu_pool_destroy(mmu_pool_t *pool)
{
	mmu_pool_t *cur,**prevp;
	mmu_pool_chunk_t *chunk,*nextchunk;

	DEBUG(4) dprintf("mmu: mmu_pool_destroy() called\n");

	if(pool == NULL)
		return;

	cur = root_pool;
	prevp = &root_pool;
	while(cur != NULL && cur != pool) {
		prevp = &(cur->next);
		cur = cur->next;
	}
	if(cur == NULL)
		return;

	*prevp = cur->next;

	chunk = pool->first_chunk;
	while(chunk != NULL) {
		nextchunk = chunk->next;
		FreeVec(chunk);
		chunk = nextchunk;
	}
	FreeVec(pool);
}

mapping *mmu_allocObj(mmu_pool_t *pool, void *real)
{
	return mmu_allocObjExtra(pool, real, 0);
}

mapping *mmu_allocObjExtra(mmu_pool_t *pool, void *real, int extraSize)
{
	void *obj,**objptr;
	uint32_t vaddr;
	mapping *entry;
	char symbol[200];

	if(pool == NULL)
		return NULL;

	if(real == NULL)
		return NULL;

	DEBUG(5) dprintf("mmu: mmu_allocObjExtra() called for pool: %s, extra size: %d\n", pool->name, extraSize);

	/* Find an empty slot in the pool chunk list */
	objptr = getSlotPtr(pool);
	if(objptr == NULL)
		return NULL;

	/* Virtual memory space */
        vaddr = vallocmem_nomap(pool->vsize + extraSize);
	if(vaddr == 0)
		return NULL;

	/* Object structure */
	obj = AllocVec(pool->objsize, MEMF_PUBLIC|MEMF_CLEAR);
	if(obj == NULL) {
		vfreemem_nomap(vaddr);
		return NULL;
	}

	/* Add entry */
	entry = mmu_addEntry(vaddr, pool->vsize+extraSize, pool->type, real);
	if(entry == NULL) {
		FreeVec(obj);
		vfreemem_nomap(vaddr);
		return NULL;
	}

	entry->read = mmu_fn_protected;
	entry->write = mmu_fn_protected;
	entry->nativePtr = obj;
	entry->destructor = mmu_pool_destructor;
	entry->pool = pool;

	*objptr = obj;

        DEBUG(3) {
                /* Add symbols */
                sprintf(symbol, "!%08x.%s", (unsigned int)vaddr,pool->name);
                mmu_addSymbol(entry, vaddr, symbol);
        }

	return entry;
}

void mmu_freeObj(mapping *entry)
{
	if(entry == NULL)
		return;
	if(entry->pool == NULL)
		return;
	if(entry->nativePtr == NULL)
		return;

	DEBUG(5) dprintf("mmu: mmu_freeObj() called for pool: %s\n", entry->pool->name);

	freeSlot(entry);

	vfreemem_nomap(entry->startAddr);
	FreeVec(entry->nativePtr);
}

/* Default MMU destructor */
static void mmu_pool_destructor(mapping *entry)
{
        DEBUG(5) dprintf("mmu: mmu_pool_destructor() called for pool: %s\n", entry->pool->name);

	mmu_freeObj(entry);
}

mapping *mmu_allocObjVAddr(mmu_pool_t *pool, void *real, uint32_t vaddr)
{
	return mmu_allocObjVAddrExtra(pool, real, vaddr, 0);
}

mapping *mmu_allocObjVAddrExtra(
	mmu_pool_t *pool,
	void *real,
	uint32_t vaddr,
	int extraSize )
{
	void *obj,**objptr;
	mapping *entry;
	char symbol[200];

	if(pool == NULL)
		return NULL;

	if(real == NULL)
		return NULL;

	if(vaddr == 0)
		return NULL;

	DEBUG(5) dprintf("mmu: mmu_allocObjVAddrExtra() called for pool: %s, extra size: %d\n", pool->name, extraSize);

	/* Find an empty slot in the pool chunk list */
	objptr = getSlotPtr(pool);
	if(objptr == NULL)
		return NULL;

	/* Object structure */
	obj = AllocVec(pool->objsize, MEMF_PUBLIC|MEMF_CLEAR);
	if(obj == NULL) {
		vfreemem_nomap(vaddr);
		return NULL;
	}

	/* TODO: Check if vaddr range really is inside user memory */

	/* Add entry */
	entry = mmu_addEntry(vaddr, pool->vsize+extraSize, pool->type, real);
	if(entry == NULL) {
		FreeVec(obj);
		return NULL;
	}

	entry->read = mmu_fn_protected;
	entry->write = mmu_fn_protected;
	entry->nativePtr = obj;
	entry->destructor = mmu_pool_destructor_vaddr;
	entry->pool = pool;

	*objptr = obj;

        DEBUG(3) {
                /* Add symbols */
                sprintf(symbol, "!%08x.%s", (unsigned int)vaddr,pool->name);
                mmu_addSymbol(entry, vaddr, symbol);
        }

	return entry;
}

void mmu_freeObjVAddr(mapping *entry)
{
	if(entry == NULL)
		return;
	if(entry->pool == NULL)
		return;
	if(entry->nativePtr == NULL)
		return;

	DEBUG(5) dprintf("mmu: mmu_freeObj() called for pool: %s\n", entry->pool->name);

	freeSlot(entry);

	FreeVec(entry->nativePtr);
}

/* Default MMU destructor */
static void mmu_pool_destructor_vaddr(mapping *entry)
{
        DEBUG(5) dprintf("mmu: mmu_pool_destructor_vaddr() called for pool: %s\n", entry->pool->name);

	mmu_freeObjVAddr(entry);
}

/* For setting up substructure mappings */
mapping *mmu_allocObjSubStruct(mmu_pool_t *pool, void *real, uint32_t vaddr)
{
	return mmu_allocObjVAddr(pool, real, vaddr);
}

mapping *mmu_allocObjSubStructExtra(
	mmu_pool_t *pool,
	void *real,
	uint32_t vaddr,
	int extraSize )
{
	return mmu_allocObjVAddrExtra(pool, real, vaddr, extraSize);
}

void mmu_freeObjSubStruct(mapping *entry)
{
	mmu_freeObjVAddr(entry);
}

} // namespace memory
} // namespace emumiga
