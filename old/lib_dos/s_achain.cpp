/*
lib_dos/s_achain.c - dos.library struct AChain

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
#include "s_achain.h"
#include "s_filelock.h"

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
        lib_dos_s_achain_t *obj;
	struct FileLock *an_lock_real;
	lib_dos_s_filelock_t *an_lock;
	uint32_t an_lock_bvaddr;

        DEBUG(7) dprintf("dos.library: struct AChain: map_r() called. VAddr: 0x%x, Size: 0x%x\n", (unsigned int)(entry->startAddr+offset), (unsigned int)size);

        /* Get our object */
        obj = (lib_dos_s_achain_t *)entry->nativePtr;

        for(i=offset;i<(size+offset);i++) {
                switch(i) {
                        /* an_Lock, BPTR!! */
                        case 8: case 9: case 10: case 11:
				an_lock_real = (struct FileLock *)BADDR(obj->real->an_Lock);
                                an_lock = lib_dos_s_filelock_get_real(an_lock_real);
				an_lock_bvaddr = an_lock->entry->startAddr >> 2;
                                i = 11;
                                break;

                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: dos.library: struct AChain: Read is not allowed. Offset=%u\n", (unsigned int)offset);
                                return 1;
                }
        }

        for(i=offset;i<(size+offset);i++) {
                switch(i) {
                        /* an_Lock */
                        case 8:
                                *data++ = (an_lock_bvaddr >> 24) & 0xff;
                                break;
                        case 9:
                                *data++ = (an_lock_bvaddr >> 16) & 0xff;
                                break;
                        case 10:
                                *data++ = (an_lock_bvaddr >> 8) & 0xff;
                                break;
                        case 11:
                                *data++ = (an_lock_bvaddr) & 0xff;
                                break;

                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: dos.library: struct AChain: Read is not allowed. Offset=%u\n", (unsigned int)offset);
                                return 1;
                }
        }

        return 0;
}

/* Global init */
int lib_dos_s_achain_init()
{
	DEBUG(4) dprintf("dos.library: lib_dos_s_achain_init() called\n");

        pool = mmu_pool_create(
                "dos.s.AChain",
                MMU_TYPE_DOS_S_ACHAIN,
                LIB_DOS_S_ACHAIN_SIZE,
                sizeof(lib_dos_s_achain_t)
        );

        if(pool == NULL)
                return 1;

	is_init = 1;
	return 0;
}

/* Global cleanup */
void lib_dos_s_achain_cleanup()
{
	DEBUG(4) dprintf("dos.library: lib_dos_s_achain_cleanup() called\n");

        if( !is_init ) {
                DEBUG(2) dprintf("Warning: dos.library: lib_dos_s_achain_cleanup: Not initialized\n");
                return;
        }

	mmu_pool_destroy(pool);

	is_init = 0;
}

/* Get an instance */
lib_dos_s_achain_t *lib_dos_s_achain_get_real(struct AChain *real)
{
	mmu_entry_t *entry;
	lib_dos_s_achain_t *obj;

	DEBUG(5) dprintf("dos.library: lib_dos_s_achain_get_real() called\n");

	if(real == NULL)
		return NULL;

        /* Check to find if it already exists */
        entry = mmu_findEntryWithTypeReal(real, MMU_TYPE_DOS_S_ACHAIN);
        if(entry != NULL) {

                /* Yes, return object */
                return (lib_dos_s_achain_t *)entry->nativePtr;
        }

        /* No, set up an object */
        entry = mmu_allocObj(pool, real);
        if(entry == NULL)
                 return NULL;

        /* Extract object */
        obj = (lib_dos_s_achain_t *)entry->nativePtr;

        /* Init structure */
	entry->read = map_r;
	entry->destructor = destructor;
        obj->entry = entry;
        obj->real = real;

        return obj;
}

/* Get an instance */
lib_dos_s_achain_t *lib_dos_s_achain_get_vaddr(uint32_t vaddr)
{
        mmu_entry_t *entry;

        DEBUG(5) dprintf("dos.library: lib_dos_s_achain_get_vaddr() called\n");

        if(vaddr == 0)
                return NULL;

        /* Search for an existing object */
        entry = mmu_findEntryWithType(vaddr, MMU_TYPE_DOS_S_ACHAIN);

        /* This structure should not be created from user space */
        if(entry == NULL)
                return NULL;

        return (lib_dos_s_achain_t *)entry->nativePtr;
}

/* Destructor of mapping */
static void destructor(mmu_entry_t *entry)
{
	lib_dos_s_achain_t *obj;
	struct FileLock *lock_real;
	mmu_entry_t *lock_entry;

	DEBUG(5) dprintf("dos.library: lib_dos_s_achain: destructor() called\n");

	obj = (lib_dos_s_achain_t *)entry->nativePtr;

	/* Find and delete any lock mapping for our lock */
	lock_real = (struct FileLock *)BADDR(obj->real->an_Lock);
	lock_entry = mmu_findEntryWithTypeReal(lock_real, MMU_TYPE_DOS_S_FILELOCK);
	if(lock_entry != NULL) {
		mmu_delEntry(lock_entry);
	}

	/* Cleanup our object */
	mmu_freeObj(entry);
}
