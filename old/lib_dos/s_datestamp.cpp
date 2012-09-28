/*
lib_dos/s_datestamp.c - dos.library struct DateStamp

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
#include <dos/dos.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../macros.h"
#include "s_datestamp.h"

/* Predecl of destructor */
static void destructor(mmu_entry_t *entry);

/* Object pool */
static mmu_pool_t *pool;

/* Init flag */
static int is_init = 0;

/* Read map function */
static int map_r(mmu_entry_t *entry, uint32_t offset, uint32_t size,
        uint8_t *data __attribute__((unused)))
{
        uint32_t i;
        lib_dos_s_datestamp_t *obj;

        DEBUG(7) dprintf("dos.library: struct DateStamp: map_r() called. VAddr: 0x%x, Size: 0x%x\n", (unsigned int)(entry->startAddr+offset), (unsigned int)size);

        /* Get our object */
        obj = (lib_dos_s_datestamp_t *)entry->nativePtr;

        for(i=offset;i<(size+offset);i++) {
		switch(i) {
			default:
	                	/* Nothing found, signal error */
	                        dprintf("Error: dos.library: struct DateStamp: Read is not allowed. Offset=%u\n", (unsigned int)offset);
	                        return 1;
		}
	}

        for(i=offset;i<(size+offset);i++) {
                switch(i) {
                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: dos.library: struct DateStamp: Read is not allowed. Offset=%u\n", (unsigned int)offset);
                                return 1;
                }
        }

	return 0;
}

/* Write map function */
static int map_w(mmu_entry_t *entry, uint32_t offset, uint32_t size,
        uint8_t *data )
{
        uint32_t i;
        lib_dos_s_datestamp_t *obj;

        DEBUG(7) dprintf("dos.library: struct DateStamp: map_w() called. VAddr: 0x%x, Size: 0x%x\n", (unsigned int)(entry->startAddr+offset), (unsigned int)size);

        /* Get our object */
        obj = (lib_dos_s_datestamp_t *)entry->nativePtr;

        for(i=offset;i<(size+offset);i++) {
                switch(i) {

			/* ds_Days */
			case 0: case 1: case 2: case 3:
				i = 3;
				break;

			/* ds_Minute */
			case 4: case 5: case 6: case 7:
				i = 7;
				break;

			/* ds_Tick */
			case 8: case 9: case 10: case 11:
				i = 11;
				break;

                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: dos.library: struct DateStamp: Write is not allowed. Offset=%u\n", (unsigned int)offset);
                                return 1;
                }
	}

        for(i=offset;i<(size+offset);i++) {
                switch(i) {

			/* ds_Days */
			case 0:
				obj->real->ds_Days &= 0x00ffffff;
				obj->real->ds_Days |= ((uint32_t)*data++ << 24);
				break;
			case 1:
				obj->real->ds_Days &= 0xff00ffff;
				obj->real->ds_Days |= ((uint32_t)*data++ << 16);
				break;
			case 2:
				obj->real->ds_Days &= 0xffff00ff;
				obj->real->ds_Days |= ((uint32_t)*data++ << 8);
				break;
			case 3:
				obj->real->ds_Days &= 0xffffff00;
				obj->real->ds_Days |= ((uint32_t)*data++);
				break;

			/* ds_Minute */
			case 4:
				obj->real->ds_Minute &= 0x00ffffff;
				obj->real->ds_Minute |= ((uint32_t)*data++ << 24);
				break;
			case 5:
				obj->real->ds_Minute &= 0xff00ffff;
				obj->real->ds_Minute |= ((uint32_t)*data++ << 16);
				break;
			case 6:
				obj->real->ds_Minute &= 0xffff00ff;
				obj->real->ds_Minute |= ((uint32_t)*data++ << 8);
				break;
			case 7:
				obj->real->ds_Minute &= 0xffffff00;
				obj->real->ds_Minute |= ((uint32_t)*data++);
				break;

			/* ds_Tick */
			case 8:
				obj->real->ds_Tick &= 0x00ffffff;
				obj->real->ds_Tick |= ((uint32_t)*data++ << 24);
				break;
			case 9:
				obj->real->ds_Tick &= 0xff00ffff;
				obj->real->ds_Tick |= ((uint32_t)*data++ << 16);
				break;
			case 10:
				obj->real->ds_Tick &= 0xffff00ff;
				obj->real->ds_Tick |= ((uint32_t)*data++ << 8);
				break;
			case 11:
				obj->real->ds_Tick &= 0xffffff00;
				obj->real->ds_Tick |= ((uint32_t)*data++);
				break;

                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: dos.library: struct DateStamp: Write is not allowed. Offset=%u\n", (unsigned int)offset);
                                return 1;
                }
        }

	return 0;
}

/* Global init */
int lib_dos_s_datestamp_init()
{
	DEBUG(4) dprintf("dos.library: lib_dos_s_datestamp_init() called\n");

        pool = mmu_pool_create(
                "dos.s.DateStamp",
                MMU_TYPE_DOS_S_DATESTAMP,
                LIB_DOS_S_DATESTAMP_SIZE,
                sizeof(lib_dos_s_datestamp_t)
        );

        if(pool == NULL)
                return 1;

	is_init = 1;
	return 0;
}

/* Global cleanup */
void lib_dos_s_datestamp_cleanup()
{
	DEBUG(4) dprintf("dos.library: lib_dos_s_datestamp_cleanup() called\n");

	if( !is_init ) {
		DEBUG(2) dprintf("Warning: dos.library: lib_dos_s_datestamp_cleanup: Not initialized\n");
		return;
	}

	mmu_pool_destroy(pool);

	is_init = 0;
}

/* Get an instance */
lib_dos_s_datestamp_t *lib_dos_s_datestamp_get_real(struct DateStamp *real)
{
	mmu_entry_t *entry;
	lib_dos_s_datestamp_t *obj;
	uint32_t vaddr;
	char symbol[200];

	DEBUG(5) dprintf("dos.library: lib_dos_s_datestamp_get_real() called\n");

        if(real == NULL)
		return NULL;

        /* Check to find if it already exists */
        entry = mmu_findEntryWithTypeReal(real, MMU_TYPE_DOS_S_DATESTAMP);
        if(entry != NULL) {

                /* Yes, return object */
                return (lib_dos_s_datestamp_t *)entry->nativePtr;
        }

        /* No, set up an object */
        entry = mmu_allocObj(pool, real);
        if(entry == NULL)
                 return NULL;

        /* Extract object */
        obj = (lib_dos_s_datestamp_t *)entry->nativePtr;

        /* Init structure */
 	entry->read = map_r;
	entry->write = map_w;
	entry->destructor = destructor;
	obj->entry = entry;
	obj->real = real;
	obj->userSpace = 0;	/* Created from real memory */

	DEBUG(3) {
		/* Add extra symbols */
		vaddr = entry->startAddr;
		sprintf(symbol, "!%08x.%s.ds_Days", vaddr, pool->name);
		mmu_addSymbol(entry, vaddr, symbol);
		sprintf(symbol, "!%08x.%s.ds_Minute", vaddr, pool->name);
		mmu_addSymbol(entry, vaddr+4, symbol);
		sprintf(symbol, "!%08x.%s.ds_Tick", vaddr, pool->name);
		mmu_addSymbol(entry, vaddr+8, symbol);
	}

	return obj;
}

/* Get an instance */
lib_dos_s_datestamp_t *lib_dos_s_datestamp_get_vaddr(uint32_t vaddr)
{
	mmu_entry_t *entry;
	lib_dos_s_datestamp_t *obj;
	struct DateStamp *real;
	char symbol[200];

	uint32_t ds_days, ds_minute, ds_tick;

	DEBUG(5) dprintf("dos.library: lib_dos_s_datestamp_get_vaddr() called\n");

	if(vaddr == 0)
		return NULL;

        /* Check to find if it already exists */
        entry = mmu_findEntryWithType(vaddr, MMU_TYPE_DOS_S_DATESTAMP);
        if(entry != NULL) {

                /* Yes, return object */
                return (lib_dos_s_datestamp_t *)entry->nativePtr;
        }

	/* TODO: Check VADDR interval and clear any old mappings in it */

	/* Read initial settings */
	if( READMEM_32(vaddr,&ds_days) ||
	    READMEM_32(vaddr+4,&ds_minute) ||
	    READMEM_32(vaddr+8,&ds_tick)
	) {
		return NULL;
	}

	/* Allocate memory for real structure */
	real = (struct DateStamp *)AllocVec(sizeof(struct DateStamp), MEMF_PUBLIC|MEMF_CLEAR);
	if(real == NULL) {
		return NULL;
	}

	/* Handle initial settings */
	real->ds_Days = ds_days;
	real->ds_Minute = ds_minute;
	real->ds_Tick = ds_tick;

        /* Set up an object */
        entry = mmu_allocObjVAddr(pool, real, vaddr);
        if(entry == NULL) {
		FreeVec(real);
                return NULL;
	}

        /* Extract object */
        obj = (lib_dos_s_datestamp_t *)entry->nativePtr;

        /* Init structure */
        entry->read = map_r;
        entry->write = map_w;
        entry->destructor = destructor;
	obj->entry = entry;
        obj->real = real;
        obj->userSpace = 1;     /* Created from user space */

	DEBUG(3) {
		/* Add extra symbols */
		sprintf(symbol, "!%08x.%s.ds_Days", vaddr, pool->name);
		mmu_addSymbol(entry, vaddr, symbol);
		sprintf(symbol, "!%08x.%s.ds_Minute", vaddr, pool->name);
		mmu_addSymbol(entry, vaddr+4, symbol);
		sprintf(symbol, "!%08x.%s.ds_Tick", vaddr, pool->name);
		mmu_addSymbol(entry, vaddr+8, symbol);
	}

	return obj;
}

/* Destructor of mapping */
static void destructor(mmu_entry_t *entry)
{
	lib_dos_s_datestamp_t *obj;
	struct DateStamp *real;

	DEBUG(5) dprintf("dos.library: lib_dos_s_datestamp: destructor() called\n");

	obj = (lib_dos_s_datestamp_t *)entry->nativePtr;

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
