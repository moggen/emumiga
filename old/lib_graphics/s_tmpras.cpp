/*
lib_graphics/s_tmpras.c - graphics.library struct TmpRas

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <proto/exec.h>
#include <graphics/rastport.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "s_tmpras.h"

/* Predecl of destructor */
static void destructor(mmu_entry_t *entry);

/* Object pool */
static mmu_pool_t *pool;

/* Init flag */
static int is_init = 0;

/* Global init */
int lib_graphics_s_tmpras_init()
{
	DEBUG(4) dprintf("graphics.library: lib_graphics_s_tmpras_init() called\n");

        pool = mmu_pool_create(
                "graphics.s.TmpRas",
                MMU_TYPE_GRAPHICS_S_TMPRAS,
                LIB_GRAPHICS_S_TMPRAS_SIZE,
                sizeof(lib_graphics_s_tmpras_t)
        );

        if(pool == NULL)
                return 1;

	is_init = 1;
	return 0;
}

/* Global cleanup */
void lib_graphics_s_tmpras_cleanup()
{
	DEBUG(4) dprintf("graphics.library: lib_graphics_s_tmpras_cleanup() called\n");

        if( !is_init ) {
                DEBUG(2) dprintf("Warning: graphics.library: lib_graphics_s_tmpras_cleanup: Not initialized\n");
                return;
        }

	mmu_pool_destroy(pool);

	is_init = 0;
}

/* Get an instance */
lib_graphics_s_tmpras_t *lib_graphics_s_tmpras_get_real(struct TmpRas *real)
{
	mmu_entry_t *entry;
	lib_graphics_s_tmpras_t *obj;
	uint32_t vaddr;
	char symbol[200];

	DEBUG(5) dprintf("graphics.library: lib_graphics_s_tmpras_get_real() called\n");

	if(real == NULL)
		return NULL;

        /* Check to find if it already exists */
        entry = mmu_findEntryWithTypeReal(real, MMU_TYPE_GRAPHICS_S_TMPRAS);
        if(entry != NULL) {

                /* Yes, return object */
                return (lib_graphics_s_tmpras_t *)entry->nativePtr;
        }

        /* No, set up an object */
        entry = mmu_allocObj(pool, real);
        if(entry == NULL)
                 return NULL;

        /* Extract object */
        obj = (lib_graphics_s_tmpras_t *)entry->nativePtr;

        /* Init structure */
	entry->destructor = destructor;
        obj->entry = entry;
        obj->real = real;
	obj->userSpace = 0;     /* Created from real memory */

	DEBUG(3) {
		/* Add extra symbols */
		vaddr = entry->startAddr;
		sprintf(symbol, "!%08x.%s.RasPtr", vaddr, pool->name);
		mmu_addSymbol(entry, vaddr+0, symbol);
		sprintf(symbol, "!%08x.%s.Size", vaddr, pool->name);
		mmu_addSymbol(entry, vaddr+4, symbol);
	}

	return obj;
}

/* Get an instance */
lib_graphics_s_tmpras_t *lib_graphics_s_tmpras_get_vaddr(uint32_t vaddr)
{
	mmu_entry_t *entry;
	lib_graphics_s_tmpras_t *obj;
	struct TmpRas *real;
	uint32_t rasptr_vaddr;
	BYTE *rasptr_real;
	uint32_t size;
	char symbol[200];

	DEBUG(5) dprintf("graphics.library: lib_graphics_s_tmpras_get_vaddr() called\n");

	if(vaddr == 0)
                return NULL;

        /* Search for an existing object */
        entry = mmu_findEntryWithType(vaddr, MMU_TYPE_GRAPHICS_S_TMPRAS);

        /* This structure should not be created from user space */
        if(entry != NULL)
	        return (lib_graphics_s_tmpras_t *)entry->nativePtr;

	/* This structure can be created from user space */

	/* Read parameters */
	if(
		READMEM_32(vaddr,&rasptr_vaddr) ||
		READMEM_32(vaddr+4,&size)
	) {
                return NULL;
	}

	rasptr_real = NULL;
	if(rasptr_vaddr != 0) {
		rasptr_real = (BYTE *)vptr(rasptr_vaddr);
		/* TODO: validity checkings */
	}

	/* Allocate memory for structure */
	real = (struct TmpRas *)AllocVec(sizeof(struct TmpRas), MEMF_PUBLIC|MEMF_CLEAR);
	if(real == NULL) {
		return NULL;
	}

	/* Set up real object */
	real->RasPtr = rasptr_real;
	real->Size = size;

	/* Set up an object */
	entry = mmu_allocObjVAddr(pool, real, vaddr);
	if(entry == NULL) {
		FreeVec(real);
		return NULL;
	}

	/* Extract object */
	obj = (lib_graphics_s_tmpras_t *)entry->nativePtr;

	/* Init structure */
	entry->destructor = destructor;
	obj->entry = entry;
	obj->real = real;
	obj->userSpace = 1;     /* Created from user space */

	DEBUG(3) {
		/* Add extra symbols */
		sprintf(symbol, "!%08x.%s.RasPtr", vaddr, pool->name);
		mmu_addSymbol(entry, vaddr+0, symbol);
		sprintf(symbol, "!%08x.%s.Size", vaddr, pool->name);
		mmu_addSymbol(entry, vaddr+4, symbol);
	}

	return obj;
}

/* Get an instance */
lib_graphics_s_tmpras_t *lib_graphics_s_tmpras_get_vaddr_noconvert(uint32_t vaddr)
{
	mmu_entry_t *entry;
	lib_graphics_s_tmpras_t *obj;
	struct TmpRas *real;
	char symbol[200];

	DEBUG(5) dprintf("graphics.library: lib_graphics_s_tmpras_get_vaddr() called\n");

	if(vaddr == 0)
                return NULL;

        /* Search for an existing object */
        entry = mmu_findEntryWithType(vaddr, MMU_TYPE_GRAPHICS_S_TMPRAS);
        if(entry != NULL)
	        return (lib_graphics_s_tmpras_t *)entry->nativePtr;

	/* Allocate memory for structure */
	real = (struct TmpRas *)AllocVec(sizeof(struct TmpRas), MEMF_PUBLIC|MEMF_CLEAR);
	if(real == NULL) {
		return NULL;
	}

	/* Set up an object */
	entry = mmu_allocObjVAddr(pool, real, vaddr);
	if(entry == NULL) {
		FreeVec(real);
		return NULL;
	}

	/* Extract object */
	obj = (lib_graphics_s_tmpras_t *)entry->nativePtr;

	/* Init structure */
	entry->destructor = destructor;
	obj->entry = entry;
	obj->real = real;
	obj->userSpace = 1;     /* Created from user space */

	DEBUG(3) {
		/* Add extra symbols */
		sprintf(symbol, "!%08x.%s.RasPtr", vaddr, pool->name);
		mmu_addSymbol(entry, vaddr+0, symbol);
		sprintf(symbol, "!%08x.%s.Size", vaddr, pool->name);
		mmu_addSymbol(entry, vaddr+4, symbol);
	}

	return obj;
}

/* Get an instance as a substructure */
lib_graphics_s_tmpras_t *lib_graphics_s_tmpras_get_substruct(struct TmpRas *real, uint32_t vaddr)
{
	mmu_entry_t *entry;
	lib_graphics_s_tmpras_t *obj;
	char symbol[200];

	DEBUG(5) dprintf("graphics.library: lib_graphics_s_tmpras_get_substruct() called\n");

	if(real == NULL)
		return NULL;

        /* Check to find if it already exists */
        entry = mmu_findEntryWithTypeReal(real, MMU_TYPE_GRAPHICS_S_TMPRAS);
        if(entry != NULL) {

                /* Yes, return object */
                return (lib_graphics_s_tmpras_t *)entry->nativePtr;
        }

        /* No, set up a substruct object */
        entry = mmu_allocObjSubStruct(pool, real, vaddr);
        if(entry == NULL)
                 return NULL;

        /* Extract object */
        obj = (lib_graphics_s_tmpras_t *)entry->nativePtr;

        /* Init structure */
	entry->destructor = destructor;
        obj->entry = entry;
        obj->real = real;
	obj->userSpace = 0;     /* Created from real memory */

	DEBUG(3) {
		/* Add extra symbols */
		sprintf(symbol, "!%08x.%s.RasPtr", vaddr, pool->name);
		mmu_addSymbol(entry, vaddr+0, symbol);
		sprintf(symbol, "!%08x.%s.Size", vaddr, pool->name);
		mmu_addSymbol(entry, vaddr+4, symbol);
	}

	return obj;
}

/* Destructor of mapping */
static void destructor(mmu_entry_t *entry)
{
        lib_graphics_s_tmpras_t *obj;
        struct TmpRas *real;

        DEBUG(5) dprintf("graphics.library: lib_graphics_s_tmpras: destructor() called\n");

        obj = (lib_graphics_s_tmpras_t *)entry->nativePtr;

        /* Is there a buffert installed via InitTmpRas() ? */
        if(obj->buffer != NULL) {
                FreeVec(obj->buffer);
        }

        /* Created from userspace? */
        if(obj->userSpace) {

                /* TODO: Copy back real object data to underlying memory? */

                real = obj->real;
                mmu_freeObjVAddr(entry);
                FreeVec(real);

        } else {

                mmu_freeObj(entry);
        }
}
