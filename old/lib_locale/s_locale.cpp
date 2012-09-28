/*
lib_locale/s_locale.c - locale.library struct Locale

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

#include <libraries/locale.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../macros.h"
#include "s_locale.h"

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
        uint32_t i,slen;
        lib_locale_s_locale_t *obj;

        DEBUG(7) dprintf("locale.library: struct Locale: map_r() called. VAddr: 0x%x, Size: 0x%x\n", (unsigned int)(entry->startAddr+offset), (unsigned int)size);

        /* Get our object */
        obj = (lib_locale_s_locale_t *)entry->nativePtr;

        for(i=offset;i<(size+offset);i++) {
                switch(i) {
                        /* loc_TimeFormat */
                        case 80: case 81: case 82: case 83:
				if(obj->loc_TimeFormat_vaddr == 0) {
					slen = strlen((char *)obj->real->loc_TimeFormat);
					obj->loc_TimeFormat_vaddr = vallocmem_rom((uint8_t *)obj->real->loc_TimeFormat,slen+1,"locale.s.locale.loc_TimeFormat string");
				}
                                i = 83;
                                break;

                        /* loc_ShortDateFormat */
                        case 88: case 89: case 90: case 91:
				if(obj->loc_ShortDateFormat_vaddr == 0) {
					slen = strlen((char *)obj->real->loc_ShortDateFormat);
					obj->loc_ShortDateFormat_vaddr = vallocmem_rom((uint8_t *)obj->real->loc_ShortDateFormat,slen+1,"locale.s.locale.loc_ShortDateFormat string");
				}
                                i = 91;
                                break;

                        /* loc_ShortTimeFormat */
                        case 92: case 93: case 94: case 95:
				if(obj->loc_ShortTimeFormat_vaddr == 0) {
					slen = strlen((char *)obj->real->loc_ShortTimeFormat);
					obj->loc_ShortTimeFormat_vaddr = vallocmem_rom((uint8_t *)obj->real->loc_ShortTimeFormat,slen+1,"locale.s.locale.loc_ShortTimeFormat string");
				}
                                i = 95;
                                break;

			default:
                        	/* Nothing found, signal error */
                        	dprintf("Error: locale.library: struct Locale: Read is not allowed. Offset=%u\n", (unsigned int)offset);
                        	return 1;
                }
        }

        for(i=offset;i<(size+offset);i++) {
                switch(i) {

                        /* loc_TimeFormat */
                        case 80:
				*data++ = (obj->loc_TimeFormat_vaddr >> 24) & 0xff;
                                break;
                        case 81:
				*data++ = (obj->loc_TimeFormat_vaddr >> 16) & 0xff;
                                break;
                        case 82:
				*data++ = (obj->loc_TimeFormat_vaddr >> 8) & 0xff;
                                break;
                        case 83:
				*data++ = (obj->loc_TimeFormat_vaddr) & 0xff;
                                break;

                        /* loc_ShortDateFormat */
                        case 88:
				*data++ = (obj->loc_ShortDateFormat_vaddr >> 24) & 0xff;
                                break;
                        case 89:
				*data++ = (obj->loc_ShortDateFormat_vaddr >> 16) & 0xff;
                                break;
                        case 90:
				*data++ = (obj->loc_ShortDateFormat_vaddr >> 8) & 0xff;
                                break;
                        case 91:
				*data++ = (obj->loc_ShortDateFormat_vaddr) & 0xff;
                                break;

                        /* loc_ShortTimeFormat */
                        case 92:
				*data++ = (obj->loc_ShortTimeFormat_vaddr >> 24) & 0xff;
                                break;
                        case 93:
				*data++ = (obj->loc_ShortTimeFormat_vaddr >> 16) & 0xff;
                                break;
                        case 94:
				*data++ = (obj->loc_ShortTimeFormat_vaddr >> 8) & 0xff;
                                break;
                        case 95:
				*data++ = (obj->loc_ShortTimeFormat_vaddr) & 0xff;
                                break;

			default:
                        	/* Nothing found, signal error */
                        	dprintf("Error: locale.library: struct Locale: Read is not allowed. Offset=%u\n", (unsigned int)offset);
                        	return 1;
                }
        }

        return 0;
}


/* Global init */
int lib_locale_s_locale_init()
{
	DEBUG(4) dprintf("locale.library: lib_locale_s_locale_init() called\n");

        pool = mmu_pool_create(
                "locale.s.Locale",
                MMU_TYPE_LOCALE_S_LOCALE,
                LIB_LOCALE_S_LOCALE_SIZE,
                sizeof(lib_locale_s_locale_t)
        );

        if(pool == NULL)
		return 1;

	is_init = 1;
	return 0;
}

/* Global cleanup */
void lib_locale_s_locale_cleanup()
{
	DEBUG(4) dprintf("locale.library: lib_locale_s_locale_cleanup() called\n");

        if( !is_init ) {
                DEBUG(2) dprintf("Warning: locale.library: lib_locale_s_locale_cleanup: Not initialized\n");
                return;
        }

	mmu_pool_destroy(pool);

	is_init = 0;
}

/* Get an instance */
lib_locale_s_locale_t *lib_locale_s_locale_get_real(struct Locale *real)
{
	mmu_entry_t *entry;
	lib_locale_s_locale_t *obj;
	uint32_t vaddr;
	char symbol[200];

	DEBUG(5) dprintf("locale.library: lib_locale_s_locale_get_real() called\n");

	if(real == NULL)
		return NULL;

	/* Check to find if it already exists */
        entry = mmu_findEntryWithTypeReal(real, MMU_TYPE_LOCALE_S_LOCALE);
        if(entry != NULL) {

		/* Yes, return object */
	        return (lib_locale_s_locale_t *)entry->nativePtr;
	}

        /* No, set up an object */
        entry = mmu_allocObj(pool, real);
        if(entry == NULL)
                 return NULL;

	/* Extract object */
       	obj = (lib_locale_s_locale_t *)entry->nativePtr;

	/* Init structure */
	entry->read = map_r;
	entry->destructor = destructor;
	obj->entry = entry;
	obj->real = real;

	DEBUG(3) {
		/* Add extra symbols */
		vaddr = entry->startAddr;
		sprintf(symbol, "!%08x.%s.loc_TimeFormat", vaddr, pool->name);
		mmu_addSymbol(entry, vaddr+80, symbol);
		sprintf(symbol, "!%08x.%s.loc_ShortDateFormat", vaddr, pool->name);
		mmu_addSymbol(entry, vaddr+88, symbol);
		sprintf(symbol, "!%08x.%s.loc_ShortTimeFormat", vaddr, pool->name);
		mmu_addSymbol(entry, vaddr+92, symbol);
	}

	return obj;
}

/* Get an instance */
lib_locale_s_locale_t *lib_locale_s_locale_get_vaddr(uint32_t vaddr)
{
	mmu_entry_t *entry;

	DEBUG(5) dprintf("locale.library: lib_locale_s_locale_get_vaddr() called\n");

	if(vaddr == 0)
                return NULL;

        /* Search for an existing object */
        entry = mmu_findEntryWithType(vaddr, MMU_TYPE_LOCALE_S_LOCALE);

        /* This structure should not be created from user space */
        if(entry == NULL)
                return NULL;

        return (lib_locale_s_locale_t *)entry->nativePtr;
}

/* Destructor of mapping */
static void destructor(mmu_entry_t *entry)
{
        lib_locale_s_locale_t *obj;

        DEBUG(5) dprintf("locale.library: lib_locale_s_locale: destructor() called\n");

        obj = (lib_locale_s_locale_t *)entry->nativePtr;

        /* loc_TimeFormat created? */
        if(obj->loc_TimeFormat_vaddr) {
                vfreemem_rom(obj->loc_TimeFormat_vaddr);
        }

        /* loc_ShortDateFormat created? */
        if(obj->loc_ShortDateFormat_vaddr) {
                vfreemem_rom(obj->loc_ShortDateFormat_vaddr);
        }

        /* loc_ShortTimeFormat created? */
        if(obj->loc_ShortTimeFormat_vaddr) {
                vfreemem_rom(obj->loc_ShortTimeFormat_vaddr);
        }

        mmu_freeObj(entry);
}
