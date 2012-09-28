/*
lib_dos/s_anchorpath.c - dos.library struct AnchorPath

Copyright (C) 2009, 2010  Magnus Öberg

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
#include <dos/dosasl.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../macros.h"
#include "s_anchorpath.h"
#include "s_achain.h"

/* Predecl of destructor */
static void destructor(mmu_entry_t *entry);

/* Object pool */
static mmu_pool_t *pool;

/* Init flag */
static int is_init = 0;

/* Read map function */
static int map_r(mmu_entry_t *entry, uint32_t offset, uint32_t size,
        uint8_t *data )
{
        uint32_t i;
        lib_dos_s_anchorpath_t *obj;
	uint8_t *extra;
	int32_t fib_direntrytype = 0;
	int32_t ap_foundbreak = 0;
	lib_dos_s_achain_t *ap_last = NULL;

        DEBUG(7) dprintf("dos.library: struct AnchorPath: map_r() called. VAddr: 0x%x, Size: 0x%x\n", (unsigned int)(entry->startAddr+offset), (unsigned int)size);

        /* Get our object */
        obj = (lib_dos_s_anchorpath_t *)entry->nativePtr;

	extra = (uint8_t *)obj->real + sizeof(struct AnchorPath);

        for(i=offset;i<(size+offset);i++) {
                if(i<LIB_DOS_S_ANCHORPATH_SIZE) {
                        /* Structure */
			if(i>=28 && i<136) continue;
                        switch(i) {
				/* ap_Last */
				case 4: case 5: case 6: case 7:
					ap_last = lib_dos_s_achain_get_real(obj->real->ap_Last);
					i = 7;
					break;
					/* TODO: Track AChain mappings */

				/* ap_FoundBreak */
				case 12: case 13: case 14: case 15:
					ap_foundbreak = obj->real->ap_FoundBreak;
					break;

				/* ap_Flags */
				case 16:
					break;

				/* ap_Info.fib_DirEntryType */
				case 24: case 25: case 26: case 27:
					fib_direntrytype = obj->real->ap_Info.fib_DirEntryType;
					break;

				/* ap_Info.fib_FileName */
				/* case 28 .. 135, handled above */

	                        default:
	                                /* Nothing found, signal error */
	                                dprintf("Error: dos.library: struct AnchorPath: Read is not allowed. Offset=%u\n", (unsigned int)offset);
	                                return 1;
	                }
		}
	}

        for(i=offset;i<(size+offset);i++) {
		if(i<LIB_DOS_S_ANCHORPATH_SIZE) {
			/* Structure */
			if(i>=28 && i<136) {
				*data++ = obj->real->ap_Info.fib_FileName[i-28];
				continue;
			}
	                switch(i) {

				/* ap_Last */
				case 4:
					if(ap_last) {
						*data++ = (ap_last->entry->startAddr >> 24) & 0xff;
					} else {
						*data++ = 0;
					}
					break;
				case 5:
					if(ap_last) {
						*data++ = (ap_last->entry->startAddr >> 16) & 0xff;
					} else {
						*data++ = 0;
					}
					break;
				case 6:
					if(ap_last) {
						*data++ = (ap_last->entry->startAddr >> 8) & 0xff;
					} else {
						*data++ = 0;
					}
					break;
				case 7:
					if(ap_last) {
						*data++ = (ap_last->entry->startAddr) & 0xff;
					} else {
						*data++ = 0;
					}
					break;

				/* ap_FoundBreak */
				case 12:
					*data++ = (ap_foundbreak >> 24) & 0xff;
					break;
				case 13:
					*data++ = (ap_foundbreak >> 16) & 0xff;
					break;
				case 14:
					*data++ = (ap_foundbreak >> 8) & 0xff;
					break;
				case 15:
					*data++ = (ap_foundbreak) & 0xff;
					break;

				/* ap_Flags */
				case 16:
					*data++ = obj->real->ap_Flags;
					break;

				/* ap_Info.fib_DirEntryType */
				case 24:
					*data++ = (fib_direntrytype >> 24) & 0xff;
					break;
				case 25:
					*data++ = (fib_direntrytype >> 16) & 0xff;
					break;
				case 26:
					*data++ = (fib_direntrytype >> 8) & 0xff;
					break;
				case 27:
					*data++ = (fib_direntrytype) & 0xff;
					break;

				/* ap_Info.fib_FileName */
				/* case 28 .. 135, handled above */

	                        default:
	                                /* Nothing found, signal error */
	                                dprintf("Error: dos.library: struct AnchorPath: Read is not allowed. Offset=%u\n", (unsigned int)offset);
	                                return 1;
	                }
		} else {
			/* Extra data */
                        *data++ = *(extra+i-LIB_DOS_S_ANCHORPATH_SIZE);
		}
        }

	return 0;
}

/* Write map function */
static int map_w(mmu_entry_t *entry, uint32_t offset, uint32_t size,
        uint8_t *data )
{
        uint32_t i;
        lib_dos_s_anchorpath_t *obj;
	uint8_t *extra;

        DEBUG(7) dprintf("dos.library: struct AnchorPath: map_w() called. VAddr: 0x%x, Size: 0x%x\n", (unsigned int)(entry->startAddr+offset), (unsigned int)size);

        /* Get our object */
        obj = (lib_dos_s_anchorpath_t *)entry->nativePtr;

	extra = (uint8_t *)obj->real + sizeof(struct AnchorPath);

        for(i=offset;i<(size+offset);i++) {
                if(i<LIB_DOS_S_ANCHORPATH_SIZE) {
                        /* Structure */
                        switch(i) {
				/* ap_Flags */
				case 16:
					break;

	                        default:
	                                /* Nothing found, signal error */
	                                dprintf("Error: dos.library: struct AnchorPath: Write is not allowed. Offset=%u\n", (unsigned int)offset);
	                                return 1;
	                }
		}
	}

        for(i=offset;i<(size+offset);i++) {
		if(i<LIB_DOS_S_ANCHORPATH_SIZE) {
			/* Structure */
	                switch(i) {

				/* ap_Flags */
				case 16:
					obj->real->ap_Flags = *data++;
					break;

	                        default:
	                                /* Nothing found, signal error */
	                                dprintf("Error: dos.library: struct AnchorPath: Write is not allowed. Offset=%u\n", (unsigned int)offset);
	                                return 1;
	                }
		} else {
			/* Extra data */
                        *(extra+i-LIB_DOS_S_ANCHORPATH_SIZE) = *data++;
		}
        }

	return 0;
}

/* Global init */
int lib_dos_s_anchorpath_init()
{
	DEBUG(4) dprintf("dos.library: lib_dos_s_anchorpath_init() called\n");

        pool = mmu_pool_create(
                "dos.s.AnchorPath",
                MMU_TYPE_DOS_S_ANCHORPATH,
                LIB_DOS_S_ANCHORPATH_SIZE,
                sizeof(lib_dos_s_anchorpath_t)
        );

        if(pool == NULL)
                return 1;

	is_init = 1;
	return 0;
}

/* Global cleanup */
void lib_dos_s_anchorpath_cleanup()
{
	DEBUG(4) dprintf("dos.library: lib_dos_s_anchorpath_cleanup() called\n");

	if( !is_init ) {
		DEBUG(2) dprintf("Warning: dos.library: lib_dos_s_anchorpath_cleanup: Not initialized\n");
		return;
	}

	mmu_pool_destroy(pool);

	is_init = 0;
}

/* Get an instance */
lib_dos_s_anchorpath_t *lib_dos_s_anchorpath_get_real(struct AnchorPath *real)
{
	mmu_entry_t *entry;
	lib_dos_s_anchorpath_t *obj;

	DEBUG(5) dprintf("dos.library: lib_dos_s_anchorpath_get_real() called\n");

        if(real == NULL)
		return NULL;

        /* Check to find if it already exists */
        entry = mmu_findEntryWithTypeReal(real, MMU_TYPE_DOS_S_ANCHORPATH);
        if(entry != NULL) {

                /* Yes, return object */
                return (lib_dos_s_anchorpath_t *)entry->nativePtr;
        }

	/* TODO: Include any appended string in mapping? */

        /* No, set up an object */
        entry = mmu_allocObj(pool, real);
        if(entry == NULL)
                 return NULL;

        /* Extract object */
        obj = (lib_dos_s_anchorpath_t *)entry->nativePtr;

        /* Init structure */
 	entry->read = map_r;
	entry->write = map_w;
	entry->destructor = destructor;
	obj->entry = entry;
	obj->real = real;
	obj->userSpace = 0;	/* Created from real memory */

	return obj;
}

/* Get an instance */
lib_dos_s_anchorpath_t *lib_dos_s_anchorpath_get_vaddr(uint32_t vaddr)
{
	mmu_entry_t *entry;
	lib_dos_s_anchorpath_t *obj;
	struct AnchorPath *real;
	char symbol[200];
	uint32_t ap_breakbits, src_vaddr;
	uint8_t ap_flags, *dst_real;
	uint16_t ap_strlen, i;

	DEBUG(5) dprintf("dos.library: lib_dos_s_anchorpath_get_vaddr() called\n");

	if(vaddr == 0)
		return NULL;

        /* Check to find if it already exists */
        entry = mmu_findEntryWithType(vaddr, MMU_TYPE_DOS_S_ANCHORPATH);
        if(entry != NULL) {

                /* Yes, return object */
                return (lib_dos_s_anchorpath_t *)entry->nativePtr;
        }

	/* TODO: Check VADDR interval and clear any old mappings in it */

	/* Read initial settings */
	if( READMEM_32(vaddr+8,&ap_breakbits) ||
	    READMEM_8(vaddr+16,&ap_flags) ||
	    READMEM_16(vaddr+18,&ap_strlen)
	) {
		return NULL;
	}

	DEBUG(5) {
		if(ap_strlen>0)
			dprintf("dos.library: struct AnchorPath: Extra data: %u\n",ap_strlen);
	}

	/* Allocate memory for real structure + extra string space */
	real = (struct AnchorPath *)AllocVec(sizeof(struct AnchorPath) + ap_strlen, MEMF_PUBLIC|MEMF_CLEAR);
	if(real == NULL) {
		return NULL;
	}

	/* Handle initial settings */
	if(ap_breakbits != 0) {
		DEBUG(2) dprintf("Warning: dos.library: struct AnchorPath: Initial ap_BreakBits value is not handled. Value: 0x%x\n",ap_breakbits);
	}
	real->ap_Flags = ap_flags;
	real->ap_Strlen = ap_strlen;

	/* Copy string part */
	src_vaddr = vaddr + LIB_DOS_S_ANCHORPATH_SIZE;
	dst_real = (uint8_t *)real;
	dst_real += sizeof(struct AnchorPath);
	i = ap_strlen;
	while(i>0) {
		if(READMEM_8(src_vaddr++, dst_real++)) {
			FreeVec(real);
			return NULL;
		}
		i--;
	}

        /* Set up an object */
        entry = mmu_allocObjVAddrExtra(pool, real, vaddr, ap_strlen);
        if(entry == NULL) {
		FreeVec(real);
                return NULL;
	}

        /* Extract object */
        obj = (lib_dos_s_anchorpath_t *)entry->nativePtr;

        /* Init structure */
        entry->read = map_r;
        entry->write = map_w;
        entry->destructor = destructor;
	obj->entry = entry;
        obj->real = real;
        obj->userSpace = 1;     /* Created from user space */

	DEBUG(3) {
		if(ap_strlen > 0) {
			/* Add extra symbol for the string part */
			sprintf(symbol, "!%08x.%s.Extra", vaddr, pool->name);
			mmu_addSymbol(entry, vaddr+LIB_DOS_S_ANCHORPATH_SIZE, symbol);
		}
	}

	return obj;
}

/* Destructor of mapping */
static void destructor(mmu_entry_t *entry)
{
	lib_dos_s_anchorpath_t *obj;
	struct AChain *achain_real;
	mmu_entry_t *achain_entry;
	struct AnchorPath *real;

	DEBUG(5) dprintf("dos.library: lib_dos_s_anchorpath: destructor() called\n");

	obj = (lib_dos_s_anchorpath_t *)entry->nativePtr;

	/* Loop through achain list and look for mappings */
	achain_real = obj->real->ap_Base;
	while(achain_real != NULL) {
/* dprintf("Looking up achain: %p\n",achain_real); */
		achain_entry = mmu_findEntryWithTypeReal(achain_real, MMU_TYPE_DOS_S_ACHAIN);
		if(achain_entry != NULL) {
			mmu_delEntry(achain_entry);
		}
		achain_real = achain_real->an_Child;
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
