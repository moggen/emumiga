/*
lib_exec/s_iorequest.c - exec.library struct IORequest

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

#include <exec/ports.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "s_iorequest.h"

/* Object pool */
static mmu_pool_t *pool;

/* Init flag */
static int is_init = 0;

/* Read map function */
static int map_r(mmu_entry_t *entry, uint32_t offset, uint32_t size,
        uint8_t *data __attribute__((unused)))
{
	uint32_t i;
	lib_exec_s_iorequest_t *obj;

	DEBUG(7) dprintf("exec.library: struct IORequest: map_r() called. VAddr: 0x%x, Offset: 0x%x, Size: 0x%x\n", (unsigned int)(entry->startAddr+offset), (unsigned int)offset, (unsigned int)size);

	/* Get our object */
	obj = (lib_exec_s_iorequest_t *)entry->nativePtr;

	for(i=offset;i<(size+offset);i++) {
		switch(i) {

			default:
				/* Nothing found, signal error */
				dprintf("Error: exec.library: struct IORequest: Read is not allowed. Offset=%u\n", (unsigned int)offset);
				return 1;
		}
	}

	for(i=offset;i<(size+offset);i++) {
		switch(i) {

			default:
				/* Nothing found, signal error */
				dprintf("Error: exec.library: struct IORequest: Read is not allowed. Offset=%u\n", (unsigned int)offset);
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
	lib_exec_s_iorequest_t *obj;
	uint8_t *extension;

	DEBUG(7) dprintf("exec.library: struct IORequest: map_w() called. VAddr: 0x%x, Offset: 0x%x, Size: 0x%x\n", (unsigned int)(entry->startAddr+offset), (unsigned int)offset, (unsigned int)size);

	/* Get our object */
	obj = (lib_exec_s_iorequest_t *)entry->nativePtr;

	/* Get a pointer to the first byte after the IORequest structure */
	extension = (uint8_t *)obj->real;
	extension += sizeof(struct IORequest);

	for(i=offset;i<(size+offset);i++) {

		/* Special handling of extension data, see below */
		if(i>=32)
			continue;

		switch(i) {

			/* io_Message.mn_Node.ln_Pri */
			case 9:
				break;

			default:
				/* Nothing found, signal error */
				dprintf("Error: exec.library: struct IORequest: Write is not allowed. Offset=%u\n", (unsigned int)offset);
				return 1;
		}
	}

	for(i=offset;i<(size+offset);i++) {

		/* Special handling of extension data. Just store it as
		   raw binary big-endian data directly after the IORequest
		   structure. We don't yet know what kind of IORequest this
		   really is. When this IORequest is converted into the real
		   one, that conversion function will have to parse this
		   extended data and set up converted fields. */
		if(i>=32) {
			extension[i-32] = *data++;
			continue;
		}
		switch(i) {

			/* io_Message.mn_Node.ln_Pri */
			case 9:
				obj->real->io_Message.mn_Node.ln_Pri = *data++;
				break;

			default:
				/* Nothing found, signal error */
				dprintf("Error: exec.library: struct IORequest: Write is not allowed. Offset=%u\n", (unsigned int)offset);
				return 1;
		}
	}
	return 0;
}

/* Global init */
int lib_exec_s_iorequest_init()
{
	DEBUG(4) dprintf("exec.library: lib_exec_s_iorequest_init() called\n");

        pool = mmu_pool_create(
                "exec.s.IORequest",
                MMU_TYPE_EXEC_S_IOREQUEST,
                LIB_EXEC_S_IOREQUEST_SIZE,
                sizeof(lib_exec_s_iorequest_t)
        );

        if(pool == NULL)
                return 1;

	is_init = 1;
	return 0;
}

/* Global cleanup */
void lib_exec_s_iorequest_cleanup()
{
	DEBUG(4) dprintf("exec.library: lib_exec_s_iorequest_cleanup() called\n");

        if( !is_init ) {
                DEBUG(2) dprintf("Warning: exec.library: lib_exec_s_iorequest_cleanup: Not initialized\n");
                return;
        }

	mmu_pool_destroy(pool);

	is_init = 0;
}

/* Get an instance */
lib_exec_s_iorequest_t *lib_exec_s_iorequest_get_real(struct IORequest *real, int size)
{
	mmu_entry_t *entry;
	lib_exec_s_iorequest_t *obj;
	uint32_t vaddr;
	char symbol[200];

	DEBUG(5) dprintf("exec.library: lib_exec_s_iorequest_get_real() called\n");
	if(real == NULL)
		return NULL;

        /* Check to find if it already exists */
        entry = mmu_findEntryWithTypeReal(real, MMU_TYPE_EXEC_S_IOREQUEST);
        if(entry != NULL) {

	        /* Extract object */
	        obj = (lib_exec_s_iorequest_t *)entry->nativePtr;

		if(size > 0 && size != obj->size) {
			/* Something strange going on */
			DEBUG(2) dprintf("Warning: exec.library: lib_exec_s_iorequest_get_real: Existing IORequest mapping found but provided size does not match!\n");
		}

                /* Return object */
                return obj;
        }

        /* No, set up an object */
        entry = mmu_allocObjExtra(pool, real, size-LIB_EXEC_S_IOREQUEST_SIZE);
        if(entry == NULL)
                 return NULL;

        /* Extract object */
        obj = (lib_exec_s_iorequest_t *)entry->nativePtr;

        /* Init structure */
	entry->read = map_r;
	entry->write = map_w;
        obj->entry = entry;
        obj->real = real;
	obj->size = size;
	obj->type = MMU_TYPE_EXEC_S_IOREQUEST;

	DEBUG(3) {
		/* Add extra symbols */
		vaddr = entry->startAddr;
		sprintf(symbol, "!%08x.%s.io_Message.mn_Node.ln_Pri", vaddr, pool->name);
		mmu_addSymbol(entry, vaddr+9, symbol);
	}

	return obj;
}

/* Get an instance */
lib_exec_s_iorequest_t *lib_exec_s_iorequest_get_vaddr(uint32_t vaddr)
{
	mmu_entry_t *entry;
	lib_exec_s_iorequest_t *obj;

	DEBUG(5) dprintf("exec.library: lib_exec_s_iorequest_get_vaddr() called\n");

	if(vaddr == 0)
                return NULL;

        /* Search for an existing object */
        entry = mmu_findEntryWithType(vaddr, MMU_TYPE_EXEC_S_IOREQUEST);

        /* This structure should not be created from user space */
        if(entry == NULL)
                return NULL;

        /* Extract object */
        obj = (lib_exec_s_iorequest_t *)entry->nativePtr;

        return obj;
}
