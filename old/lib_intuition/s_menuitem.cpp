/*
lib_intuition/s_menuitem.c - intuition.library struct MenuItem

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

#include <intuition/intuition.h>
#include <libraries/gadtools.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "s_menuitem.h"

/* Object pool */
static mmu_pool_t *pool;

/* Init flag */
static int is_init = 0;

/* Read map function */
static int map_r(mmu_entry_t *entry, uint32_t offset, uint32_t size,
        uint8_t *data )
{
        uint32_t i;
        lib_intuition_s_menuitem_t *obj;
        lib_intuition_s_menuitem_t *nextitem = NULL, *subitem = NULL;
	uint32_t userdata = 0;

	/* TODO: Go through all objects and remove ponters to other
                 objects. References must not be stored for GC to work */

        DEBUG(7) dprintf("intuition.library: struct MenuItem: map_r() called. VAddr: 0x%x, Size: 0x%x\n", (unsigned int)(entry->startAddr+offset), (unsigned int)size);

        /* Get our object */
        obj = (lib_intuition_s_menuitem_t *)entry->nativePtr;

        for(i=offset;i<(size+offset);i++) {
                switch(i) {

                        /* NextItem */
                        case 0: case 1: case 2: case 3:
				nextitem = lib_intuition_s_menuitem_get_real(obj->real->NextItem);
                                i = 3;
                                break;

                        /* Flags */
                        case 12: case 13:
                                i = 13;
                                break;

                        /* SubItem */
                        case 28: case 29: case 30: case 31:
				subitem = lib_intuition_s_menuitem_get_real(obj->real->SubItem);
                                i = 31;
                                break;

                        /* NextSelect */
                        case 32: case 33:
                                i = 33;
                                break;

                        /* Gadtools extension: UserData */
                        case 34: case 35: case 36: case 37:
				userdata = (uint32_t)GTMENUITEM_USERDATA(obj->real);
                                i = 37;
                                break;

                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: intuition.library: struct MenuItem: Read is not allowed. Offset=%u\n", (unsigned int)offset);
                                return 1;
                }
        }

        for(i=offset;i<(size+offset);i++) {
                switch(i) {

                        /* NextItem */
                        case 0:
				if(nextitem != NULL)
	                                *data++ = (nextitem->entry->startAddr >> 24) & 0xff;
				else
					*data++ = 0;
                                break;
                        case 1:
				if(nextitem != NULL)
	                                *data++ = (nextitem->entry->startAddr >> 16) & 0xff;
				else
					*data++ = 0;
                                break;
                        case 2:
				if(nextitem != NULL)
	                                *data++ = (nextitem->entry->startAddr >> 8) & 0xff;
				else
					*data++ = 0;
                                break;
                        case 3:
				if(nextitem != NULL)
	                                *data++ = (nextitem->entry->startAddr) & 0xff;
				else
					*data++ = 0;
                                break;

                        /* Flags */
                        case 12:
                                *data++ = (obj->real->Flags >> 8) & 0xff;
                                break;
                        case 13:
                                *data++ = (obj->real->Flags) & 0xff;
                                break;

                        /* SubItem */
                        case 28:
				if(subitem != NULL)
	                                *data++ = (subitem->entry->startAddr >> 24) & 0xff;
				else
					*data++ = 0;
                                break;
                        case 29:
				if(subitem != NULL)
	                                *data++ = (subitem->entry->startAddr >> 16) & 0xff;
				else
					*data++ = 0;
                                break;
                        case 30:
				if(subitem != NULL)
	                                *data++ = (subitem->entry->startAddr >> 8) & 0xff;
				else
					*data++ = 0;
                                break;
                        case 31:
				if(subitem != NULL)
	                                *data++ = (subitem->entry->startAddr) & 0xff;
				else
					*data++ = 0;
                                break;

                        /* NextSelect */
                        case 32:
                                *data++ = (obj->real->NextSelect >> 8) & 0xff;
                                break;
                        case 33:
                                *data++ = (obj->real->NextSelect) & 0xff;
                                break;

                        /* Gadtools extension: UserData */
                        case 34:
                                *data++ = (userdata >> 24) & 0xff;
                                break;
                        case 35:
                                *data++ = (userdata >> 16) & 0xff;
                                break;
                        case 36:
                                *data++ = (userdata >> 8) & 0xff;
                                break;
                        case 37:
                                *data++ = (userdata) & 0xff;
                                break;

                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: intuition.library: struct MenuItem: Read is not allowed. Offset=%u\n", (unsigned int)offset);
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
        lib_intuition_s_menuitem_t *obj;

        DEBUG(7) dprintf("intuition.library: struct MenuItem: map_w() called. VAddr: 0x%x, Size: 0x%x\n", (unsigned int)(entry->startAddr+offset), (unsigned int)size);

        /* Get our object */
        obj = (lib_intuition_s_menuitem_t *)entry->nativePtr;

        for(i=offset;i<(size+offset);i++) {
                switch(i) {

                        /* Flags */
                        case 12: case 13:
                                i = 13;
                                break;

                        /* MutualExclude */
                        case 14: case 15: case 16: case 17:
                                i = 17;
                                break;

                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: intuition.library: struct MenuItem: Write is not allowed. Offset=%u\n", (unsigned int)offset);
                                return 1;
                }
        }

        for(i=offset;i<(size+offset);i++) {
                switch(i) {

                        /* Flags */
                        case 12:
				obj->real->Flags &= 0x00ff;
				obj->real->Flags |= (*data++ << 8);
                                break;
                        case 13:
				obj->real->Flags &= 0xff00;
				obj->real->Flags |= (*data++);
                                break;

                        /* MutualExclude */
                        case 14:
				obj->real->MutualExclude &= 0x00ffffff;
				obj->real->MutualExclude |= (*data++ << 24);
                                break;
                        case 15:
				obj->real->MutualExclude &= 0xff00ffff;
				obj->real->MutualExclude |= (*data++ << 16);
                                break;
                        case 16:
				obj->real->MutualExclude &= 0xffff00ff;
				obj->real->MutualExclude |= (*data++ << 8);
                                break;
                        case 17:
				obj->real->MutualExclude &= 0xffffff00;
				obj->real->MutualExclude |= (*data++);
                                break;

			/* TODO: Optimize writes of larger sizes */

                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: intuition.library: struct MenuItem: Write is not allowed. Offset=%u\n", (unsigned int)offset);
                                return 1;
                }
        }

        return 0;
}

/* Global init */
int lib_intuition_s_menuitem_init()
{
	DEBUG(4) dprintf("intuition.library: lib_intuition_s_menuitem_init() called\n");

        pool = mmu_pool_create(
                "intuition.s.MenuItem",
                MMU_TYPE_INTUITION_S_MENUITEM,
                LIB_INTUITION_S_MENUITEM_SIZE,
                sizeof(lib_intuition_s_menuitem_t)
        );

        if(pool == NULL)
                return 1;

	is_init = 1;
	return 0;
}

/* Global cleanup */
void lib_intuition_s_menuitem_cleanup()
{
	DEBUG(4) dprintf("intuition.library: lib_intuition_s_menuitem_cleanup() called\n");

        if( !is_init ) {
                DEBUG(2) dprintf("Warning: intuition.library: lib_intuition_s_menuitem_cleanup: Not initialized\n");
                return;
        }

	mmu_pool_destroy(pool);

	is_init = 0;
}

/* Get an instance */
lib_intuition_s_menuitem_t *lib_intuition_s_menuitem_get_real(struct MenuItem *real)
{
	mmu_entry_t *entry;
	lib_intuition_s_menuitem_t *obj;
        uint32_t vaddr;
        char symbol[200];

	DEBUG(5) dprintf("intuition.library: lib_intuition_s_menuitem_get_real() called\n");

	if(real == NULL)
		return NULL;

        /* Check to find if it already exists */
        entry = mmu_findEntryWithTypeReal(real, MMU_TYPE_INTUITION_S_MENUITEM);
        if(entry != NULL) {

                /* Yes, return object */
                return (lib_intuition_s_menuitem_t *)entry->nativePtr;
        }

        /* No, set up an object */
        entry = mmu_allocObj(pool, real);
        if(entry == NULL)
                 return NULL;

        /* Extract object */
        obj = (lib_intuition_s_menuitem_t *)entry->nativePtr;

        /* Init structure */
	entry->read = map_r;
	entry->write = map_w;
        obj->entry = entry;
        obj->real = real;

        DEBUG(3) {
                /* Add extra symbols */
                vaddr = entry->startAddr;
                sprintf(symbol, "!%08x.%s.NextItem", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+0, symbol);
                sprintf(symbol, "!%08x.%s.Flags", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+12, symbol);
                sprintf(symbol, "!%08x.%s.MutualExclude", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+14, symbol);
                sprintf(symbol, "!%08x.%s.SubItem", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+28, symbol);
                sprintf(symbol, "!%08x.%s.NextSelect", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+32, symbol);
                sprintf(symbol, "!%08x.%s.UserData(extension)", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+34, symbol);
        }

	return obj;
}

/* Get an instance */
lib_intuition_s_menuitem_t *lib_intuition_s_menuitem_get_vaddr(uint32_t vaddr)
{
	mmu_entry_t *entry;

	DEBUG(5) dprintf("intuition.library: lib_intuition_s_menuitem_get_vaddr() called\n");

	if(vaddr == 0)
                return NULL;

        /* Search for an existing object */
        entry = mmu_findEntryWithType(vaddr, MMU_TYPE_INTUITION_S_MENUITEM);

        /* This structure should not be created from user space */
        if(entry == NULL)
                return NULL;

        return (lib_intuition_s_menuitem_t *)entry->nativePtr;
}
