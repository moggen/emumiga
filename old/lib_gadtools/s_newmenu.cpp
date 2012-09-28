/*
lib_gadtools/s_newmenu.c - gadtools.library struct NewMenu

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
#include <libraries/gadtools.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "s_newmenu.h"

/* Predecl of destructor */
static void destructor(mmu_entry_t *entry);

/* Object pool */
static mmu_pool_t *pool;

/* Init flag */
static int is_init = 0;

/* Global init */
int lib_gadtools_s_newmenu_init()
{
	DEBUG(4) dprintf("gadtools.library: lib_gadtools_s_newmenu_init() called\n");

        pool = mmu_pool_create(
                "gadtools.s.NewMenu",
                MMU_TYPE_GADTOOLS_S_NEWMENU,
                LIB_GADTOOLS_S_NEWMENU_SIZE,
                sizeof(lib_gadtools_s_newmenu_t)
        );

        if(pool == NULL)
                return 1;

	is_init = 1;
	return 0;
}

/* Global cleanup */
void lib_gadtools_s_newmenu_cleanup()
{
	DEBUG(4) dprintf("gadtools.library: lib_gadtools_s_newmenu_cleanup() called\n");

        if( !is_init ) {
                DEBUG(2) dprintf("Warning: gadtools.library: lib_gadtools_s_newmenu_cleanup: Not initialized\n");
                return;
        }

	mmu_pool_destroy(pool);

	is_init = 0;
}

/* Get an instance */
lib_gadtools_s_newmenu_t *lib_gadtools_s_newmenu_get_real(struct NewMenu *real)
{
	mmu_entry_t *entry;

	DEBUG(5) dprintf("gadtools.library: lib_gadtools_s_newmenu_get_real() called\n");

	if(real == NULL)
		return NULL;

        /* Check to find if it already exists */
        entry = mmu_findEntryWithTypeReal(real, MMU_TYPE_GADTOOLS_S_NEWMENU);

        /* This structure should not be created from real space */
        if(entry == NULL)
                return NULL;

        return (lib_gadtools_s_newmenu_t *)entry->nativePtr;
}

/* Get an instance */
lib_gadtools_s_newmenu_t *lib_gadtools_s_newmenu_get_vaddr(uint32_t vaddr)
{
	mmu_entry_t *entry;
	lib_gadtools_s_newmenu_t *obj;
	uint32_t current_vaddr;
	int count, i;
	struct NewMenu *newmenu_real, *current_menu;
	uint8_t nm_type;
	uint32_t nm_label, nm_commkey, nm_mutualexclude, nm_userdata;
	uint16_t nm_flags;
	char symbol[200];

	DEBUG(5) dprintf("gadtools.library: lib_gadtools_s_newmenu_get_vaddr() called\n");

	if(vaddr == 0)
                return NULL;

        /* Search for an existing object */
        entry = mmu_findEntryWithType(vaddr, MMU_TYPE_GADTOOLS_S_NEWMENU);
        if(entry != NULL)
                return (lib_gadtools_s_newmenu_t *)entry->nativePtr;

	/* Count items */
	count = 0;
	current_vaddr = vaddr;
	do {
		if(READMEM_8(current_vaddr,&nm_type)) {
			return NULL;
		}
		current_vaddr += LIB_GADTOOLS_S_NEWMENU_SIZE;
		count++;
	} while(nm_type != 0);

	DEBUG(5) dprintf("gadtools.library: lib_gadtools_s_newmenu_get_vaddr: Item count: %d\n",count);

	/* Allocate mem */
	newmenu_real = (struct NewMenu *)AllocVec(sizeof(struct NewMenu) * count, MEMF_PUBLIC|MEMF_CLEAR);
	if(newmenu_real == NULL) {
		return NULL;
	}

	/* Set up */
	current_vaddr = vaddr;
	current_menu = newmenu_real;
	do {
		if(
			READMEM_8(  current_vaddr,    &nm_type          ) ||
			READMEM_32( current_vaddr+2,  &nm_label         ) ||
			READMEM_32( current_vaddr+6,  &nm_commkey       ) ||
			READMEM_16( current_vaddr+10, &nm_flags         ) ||
			READMEM_32( current_vaddr+12, &nm_mutualexclude ) ||
			READMEM_32( current_vaddr+16, &nm_userdata      )
		) {
			FreeVec(newmenu_real);
			return NULL;
		}

		/* nm_Type */
		current_menu->nm_Type = nm_type;

		/* nm_Label */
		if(nm_label == 0) {
			current_menu->nm_Label = NULL;
		} else if(nm_label == 0xffffffff) {
			current_menu->nm_Label = NM_BARLABEL;
		} else {
			current_menu->nm_Label = (STRPTR)vptr(nm_label);
			/* TODO: Better checks, make copy? */
		}
		/* nm_CommKey */
		if(nm_commkey == 0) {
			current_menu->nm_CommKey = NULL;
		} else {
			current_menu->nm_CommKey = (STRPTR)vptr(nm_commkey);
			/* TODO: Better checks, make copy? */
		}

		/* nm_Flags */
		current_menu->nm_Flags = nm_flags;

		/* nm_MutualExclude */
		current_menu->nm_MutualExclude = (LONG)nm_mutualexclude;

		/* nm_UserData */
		/* Keep the vaddr in this field. The OS should not read this
		   field, only the application */
		current_menu->nm_UserData = (APTR)nm_userdata;

		current_vaddr += LIB_GADTOOLS_S_NEWMENU_SIZE;
		current_menu++;

	} while(nm_type != 0);

	/* Set up an object. Normal size is one item but this is really
	   an array so we need count-1 more items in this mapping */
	entry = mmu_allocObjVAddrExtra(pool, newmenu_real, vaddr, LIB_GADTOOLS_S_NEWMENU_SIZE * (count-1));
	if(entry == NULL) {
		FreeVec(newmenu_real);
		return NULL;
	}

	/* Extract object */
	obj = (lib_gadtools_s_newmenu_t *)entry->nativePtr;

	/* Init structure */
        entry->destructor = destructor;
        obj->entry = entry;
        obj->real = newmenu_real;
	obj->count = count;	/* Count of items */
        obj->userSpace = 1;     /* Created from user space */

        DEBUG(3) {
                for(i=1;i<count;i++) {
                        /* Add symbol for array entries */
                        sprintf(symbol, "!%08x.%s[%d]", vaddr, pool->name, i);
                        mmu_addSymbol(entry, vaddr + i*LIB_GADTOOLS_S_NEWMENU_SIZE, symbol);
                }
        }

        return obj;
}
/* Destructor of mapping */
static void destructor(mmu_entry_t *entry)
{
	lib_gadtools_s_newmenu_t *obj;
	struct NewMenu *real;

	DEBUG(5) dprintf("gadtools.library: lib_gadtools_s_newmenu: destructor() called\n");

	obj = (lib_gadtools_s_newmenu_t *)entry->nativePtr;

	/* Created from userspace? */
	if(obj->userSpace) {

		/* TODO: Copy back real object data to underlying memory? */

		real = obj->real;
		mmu_freeObjVAddr(entry);
		FreeVec(real);

	} else {
		/* Should never happen, but in case.. */
		mmu_freeObj(entry);
	}
}
