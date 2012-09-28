/*
lib_intuition/s_screen.c - intuition.library struct Screen

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

#include <intuition/screens.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "s_screen.h"
#include "../lib_graphics/s_textattr.h"
#include "../lib_graphics/s_viewport.h"
#include "../lib_graphics/s_rastport.h"
#include "../lib_graphics/s_layer_info.h"
#include "../lib_graphics/s_bitmap.h"

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
        lib_intuition_s_screen_t *obj;
	lib_graphics_s_textattr_t *font = NULL;
	int rc;

        DEBUG(7) dprintf("intuition.library: struct Screen: map_r() called. VAddr: 0x%x, Size: 0x%x\n", (unsigned int)(entry->startAddr+offset), (unsigned int)size);

        /* Get our object */
        obj = (lib_intuition_s_screen_t *)entry->nativePtr;

        for(i=offset;i<(size+offset);i++) {

		/* Special handling of substructs */

		/* RastPort */
		if(i >= 84 && i < 184) {

			rc = obj->RastPort->entry->read(
				obj->RastPort->entry,
				i-84,
				1,
				data++
			);
			if(rc) {
				return rc;
			}

		/* BitMap */
		} else if(i >= 184 && i < 224) {

			rc = obj->BitMap->entry->read(
				obj->BitMap->entry,
				i-184,
				1,
				data++
			);
			if(rc) {
				return rc;
			}

                } else switch(i) {

                        /* Width */
                        case 12: case 13:
                                i = 13;
				break;

                        /* Height */
                        case 14: case 15:
                                i = 15;
				break;

                        /* WBorTop */
                        case 35:
                                i = 35;
				break;

                        /* Font */
                        case 40: case 41: case 42: case 43:
                                font = lib_graphics_s_textattr_get_real(obj->real->Font);
                                i = 43;
                                break;

                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: intuition.library: struct Screen: Read is not allowed. Offset=%u\n", (unsigned int)offset);
                                return 1;
                }
        }

        for(i=offset;i<(size+offset);i++) {

		/* Special handling of substructs */

		/* RastPort */
		if(i >= 84 && i < 184) {
			/* NOOP */
		/* BitMap */
		} else if(i >= 184 && i < 224) {
			/* NOOP */
                } else switch(i) {

			/* Width */
                        case 12:
                                *data++ = (obj->real->Width >> 8) & 0xff;
                                break;
                        case 13:
                                *data++ = (obj->real->Width) & 0xff;
                                break;

			/* Height */
                        case 14:
                                *data++ = (obj->real->Height >> 8) & 0xff;
                                break;
                        case 15:
                                *data++ = (obj->real->Height) & 0xff;
                                break;

                        /* WBorTop */
                        case 35:
                                *data++ = (obj->real->WBorTop) & 0xff;
                                break;

                        /* Font */
                        case 40:
				if(font) {
	                                *data++ = (font->entry->startAddr >> 24) & 0xff;
				} else {
					*data++ = 0;
				}
                                break;
                        case 41:
				if(font) {
	                                *data++ = (font->entry->startAddr >> 16) & 0xff;
				} else {
					*data++ = 0;
				}
                                break;
                        case 42:
				if(font) {
	                                *data++ = (font->entry->startAddr >> 8) & 0xff;
				} else {
					*data++ = 0;
				}
                                break;
                        case 43:
				if(font) {
	                                *data++ = (font->entry->startAddr) & 0xff;
				} else {
					*data++ = 0;
				}
                                break;

                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: intuition.library: struct Screen: Read is not allowed. Offset=%u\n", (unsigned int)offset);
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
        lib_intuition_s_screen_t *obj;
	int rc;

        DEBUG(7) dprintf("intuition.library: struct Screen: map_w() called. VAddr: 0x%x, Size: 0x%x\n", (unsigned int)(entry->startAddr+offset), (unsigned int)size);

        /* Get our object */
        obj = (lib_intuition_s_screen_t *)entry->nativePtr;

        for(i=offset;i<(size+offset);i++) {

		/* Special handling of substructs */

		/* RastPort */
		if(i >= 84 && i < 184) {

			rc = obj->RastPort->entry->write(
				obj->RastPort->entry,
				i-84,
				1,
				data++
			);
			if(rc) {
				return rc;
			}

                } else switch(i) {

                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: intuition.library: struct Screen: Write is not allowed. Offset=%u\n", (unsigned int)offset);
                                return 1;
                }
        }

        for(i=offset;i<(size+offset);i++) {

		/* Special handling of substructs */

		/* RastPort */
		if(i >= 84 && i < 184) {
			/* NOOP */
                } else switch(i) {

                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: intuition.library: struct Screen: Write is not allowed. Offset=%u\n", (unsigned int)offset);
                                return 1;
                }
        }

        return 0;
}

/* Global init */
int lib_intuition_s_screen_init()
{
	DEBUG(4) dprintf("intuition.library: lib_intuition_s_screen_init() called\n");

        pool = mmu_pool_create(
                "intuition.s.Screen",
                MMU_TYPE_INTUITION_S_SCREEN,
                LIB_INTUITION_S_SCREEN_SIZE,
                sizeof(lib_intuition_s_screen_t)
        );

        if(pool == NULL)
                return 1;

	is_init = 1;
	return 0;
}

/* Global cleanup */
void lib_intuition_s_screen_cleanup()
{
	DEBUG(4) dprintf("intuition.library: lib_intuition_s_screen_cleanup() called\n");

        if( !is_init ) {
                DEBUG(2) dprintf("Warning: intuition.library: lib_intuition_s_screen_cleanup: Not initialized\n");
                return;
        }

	mmu_pool_destroy(pool);

	is_init = 0;
}

/* Get an instance */
lib_intuition_s_screen_t *lib_intuition_s_screen_get_real(struct Screen *real)
{
	mmu_entry_t *entry;
	lib_intuition_s_screen_t *obj;
	uint32_t vaddr;
	char symbol[200];

	DEBUG(5) dprintf("intuition.library: lib_intuition_s_screen_get_real() called\n");

	if(real == NULL)
		return NULL;

        /* Check to find if it already exists */
        entry = mmu_findEntryWithTypeReal(real, MMU_TYPE_INTUITION_S_SCREEN);
        if(entry != NULL) {

                /* Yes, return object */
                return (lib_intuition_s_screen_t *)entry->nativePtr;
        }

        /* No, set up an object */
        entry = mmu_allocObj(pool, real);
        if(entry == NULL)
                 return NULL;

        /* Extract object */
        obj = (lib_intuition_s_screen_t *)entry->nativePtr;

        /* Init structure */
	entry->read = map_r;
	entry->write = map_w;
	entry->destructor = destructor;
        obj->entry = entry;
        obj->real = real;

        DEBUG(3) {
                /* Add extra symbols */
                vaddr = entry->startAddr;
                sprintf(symbol, "!%08x.%s.Width", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+12, symbol);
                sprintf(symbol, "!%08x.%s.Height", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+14, symbol);
                sprintf(symbol, "!%08x.%s.WBorTop", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+35, symbol);
                sprintf(symbol, "!%08x.%s.Font", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+40, symbol);
                sprintf(symbol, "!%08x.%s.ViewPort", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+44, symbol);
                sprintf(symbol, "!%08x.%s.RastPort", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+84, symbol);
                sprintf(symbol, "!%08x.%s.BitMap", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+184, symbol);
                sprintf(symbol, "!%08x.%s.LayerInfo", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+224, symbol);
        }

	/* Set up mappings for substructures */

	/* ViewPort */
	obj->ViewPort = lib_graphics_s_viewport_get_substruct(
				&(real->ViewPort),
				entry->startAddr + 44
			);

	/* RastPort */
	obj->RastPort = lib_graphics_s_rastport_get_substruct(
				&(real->RastPort),
				entry->startAddr + 84
			);

	/* BitMap */
	obj->BitMap = lib_graphics_s_bitmap_get_substruct(
				&(real->BitMap),
				entry->startAddr + 184
			);

	/* LayerInfo */
	obj->LayerInfo = lib_graphics_s_layer_info_get_substruct(
				&(real->LayerInfo),
				entry->startAddr + 224
			);

	return obj;
}

/* Get an instance */
lib_intuition_s_screen_t *lib_intuition_s_screen_get_vaddr(uint32_t vaddr)
{
	mmu_entry_t *entry;

	DEBUG(5) dprintf("intuition.library: lib_intuition_s_screen_get_vaddr() called\n");

	if(vaddr == 0)
                return NULL;

        /* Search for an existing object */
        entry = mmu_findEntryWithType(vaddr, MMU_TYPE_INTUITION_S_SCREEN);

        /* This structure should not be created from user space */
        if(entry == NULL)
                return NULL;

        return (lib_intuition_s_screen_t *)entry->nativePtr;
}

/* Destructor of mapping */
static void destructor(mmu_entry_t *entry)
{
        lib_intuition_s_screen_t *obj;

        DEBUG(5) dprintf("intuition.library: lib_intuition_s_screen: destructor() called\n");

        obj = (lib_intuition_s_screen_t *)entry->nativePtr;

        /* LayerInfo mapped?? */
        if(obj->LayerInfo) {
                mmu_delEntry(obj->LayerInfo->entry);
        }

        /* BitMap mapped?? */
        if(obj->BitMap) {
                mmu_delEntry(obj->BitMap->entry);
        }

        /* RastPort mapped?? */
        if(obj->RastPort) {
                mmu_delEntry(obj->RastPort->entry);
        }

        /* ViewPort mapped?? */
        if(obj->ViewPort) {
                mmu_delEntry(obj->ViewPort->entry);
        }

        mmu_freeObj(entry);
}
