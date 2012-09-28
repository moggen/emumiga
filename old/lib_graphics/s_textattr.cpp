/*
lib_graphics/s_textattr.c - graphics.library struct TextAttr/TTextAttr

Copyright (C) 2010  Magnus �berg

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
#include <graphics/text.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../macros.h"
#include "s_textattr.h"

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
        lib_graphics_s_textattr_t *obj;

        DEBUG(7) dprintf("graphics.library: struct TextAttr: map_r() called. VAddr: 0x%x, Size: 0x%x\n", (unsigned int)(entry->startAddr+offset), (unsigned int)size);

        /* Get our object */
        obj = (lib_graphics_s_textattr_t *)entry->nativePtr;

        for(i=offset;i<(size+offset);i++) {
                switch(i) {
                        /* ta_YSize */
                        case 4: case 5:
                                i = 5;
                                break;

                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: graphics.library: struct TextAttr: Read is not allowed. Offset=%u\n", (unsigned int)offset);
                                return 1;
                }
        }

        for(i=offset;i<(size+offset);i++) {
                switch(i) {

                        /* ta_YSize */
                        case 4:
				*data++ = (obj->real->ta_YSize >> 8) & 0xff;
                                break;
                        case 5:
				*data++ = (obj->real->ta_YSize) & 0xff;
                                break;

                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: graphics.library: struct TextAttr: Read is not allowed. Offset=%u\n", (unsigned int)offset);
                                return 1;
                }
        }

        return 0;
}


/* Global init */
int lib_graphics_s_textattr_init()
{
	DEBUG(4) dprintf("graphics.library: lib_graphics_s_textattr_init() called\n");

        pool = mmu_pool_create(
                "graphics.s.TextAttr",
                MMU_TYPE_GRAPHICS_S_TEXTATTR,
                LIB_GRAPHICS_S_TEXTATTR_SIZE,
                sizeof(lib_graphics_s_textattr_t)
        );

        if(pool == NULL)
                return 1;

	is_init = 1;
	return 0;
}

/* Global cleanup */
void lib_graphics_s_textattr_cleanup()
{
	DEBUG(4) dprintf("graphics.library: lib_graphics_s_textattr_cleanup() called\n");

	if( !is_init ) {
		DEBUG(2) dprintf("Warning: graphics.library: lib_graphics_s_textattr_cleanup: Not initialized\n");
		return;
	}

	mmu_pool_destroy(pool);

	is_init = 0;
}

/* Get an instance */
lib_graphics_s_textattr_t *lib_graphics_s_textattr_get_real(struct TextAttr *real)
{
	mmu_entry_t *entry;
	lib_graphics_s_textattr_t *obj;
	int extra;
	uint32_t vaddr;
	char symbol[200];

	DEBUG(5) dprintf("graphics.library: lib_graphics_s_textattr_get_real() called\n");

        if(real == NULL)
		return NULL;

        /* Check to find if it already exists */
        entry = mmu_findEntryWithTypeReal(real, MMU_TYPE_GRAPHICS_S_TEXTATTR);
        if(entry != NULL) {

                /* Yes, return object */
                return (lib_graphics_s_textattr_t *)entry->nativePtr;
        }

	/* Check if we are an TTextAttr */
	extra = 0;
	if(real->ta_Style & FSB_TAGGED) {
		extra = 4;
	}

        /* Set the object */
        entry = mmu_allocObjExtra(pool, real, extra);
        if(entry == NULL)
                 return NULL;

        /* Extract object */
        obj = (lib_graphics_s_textattr_t *)entry->nativePtr;

        /* Init structure */
	entry->read = map_r;
	entry->destructor = destructor;
	obj->entry = entry;
	obj->real = real;
	obj->userSpace = 0;	/* Created from real memory */
	obj->extra = extra;	/* If we are an ITextAttr */

	DEBUG(3) {
		/* Add extra symbols */
		vaddr = entry->startAddr;
		sprintf(symbol, "!%08x.%s.ta_YSize", vaddr, pool->name);
		mmu_addSymbol(entry, vaddr+4, symbol);
	}

	return obj;
}

/* Get an instance */
lib_graphics_s_textattr_t *lib_graphics_s_textattr_get_vaddr(uint32_t vaddr)
{
	mmu_entry_t *entry;
	lib_graphics_s_textattr_t *obj;
	struct TextAttr *real;
	char symbol[200];

	uint32_t ta_Name_vaddr;
	uint8_t *ta_Name_real;
	uint16_t ta_YSize;
	uint8_t ta_Style, ta_Flags;

	DEBUG(5) dprintf("graphics.library: lib_graphics_s_textattr_get_vaddr() called\n");

	if(vaddr == 0)
		return NULL;

        /* Check to find if it already exists */
        entry = mmu_findEntryWithType(vaddr, MMU_TYPE_GRAPHICS_S_TEXTATTR);
        if(entry != NULL) {

                /* Yes, return object */
                return (lib_graphics_s_textattr_t *)entry->nativePtr;
        }

	/* TODO: Check VADDR interval and clear any old mappings in it */

	/* Read parameters */
	if(
	    READMEM_32(vaddr,&ta_Name_vaddr) ||
	    READMEM_16(vaddr+4,&ta_YSize) ||
	    READMEM_8(vaddr+6,&ta_Style) ||
	    READMEM_8(vaddr+7,&ta_Flags)
        ) {
		return NULL;
	}

	/* Is this a TTextAttr? */
	if(ta_Style & FSB_TAGGED) {
		DEBUG(2) dprintf("Warning: graphics.library: struct TextAttr: Creation of TTextAttr from user memory is not supported. Ignoring tag part.\n");
		ta_Style &= ~FSB_TAGGED;
	}

	/* Check font name string */
	if(ta_Name_vaddr == 0) {
		return NULL;
	}
	ta_Name_real = vptr(ta_Name_vaddr);	/* TODO: Better check */
	if(ta_Name_real == NULL) {
		return NULL;
	}

	/* Allocate memory for structure */
	real = (struct TextAttr *)AllocVec(sizeof(struct TextAttr), MEMF_PUBLIC|MEMF_CLEAR);
	if(real == NULL) {
		return NULL;
	}

	/* Set up real object */
	real->ta_Name = (STRPTR)ta_Name_real;
	real->ta_YSize = ta_YSize;
	real->ta_Style = ta_Style;
	real->ta_Flags = ta_Flags;

        /* Set up an object */
        entry = mmu_allocObjVAddr(pool, real, vaddr);
        if(entry == NULL) {
		FreeVec(real);
                return NULL;
	}

        /* Extract object */
        obj = (lib_graphics_s_textattr_t *)entry->nativePtr;

        /* Init structure */
	entry->read = map_r;
        entry->destructor = destructor;
	obj->entry = entry;
        obj->real = real;
        obj->userSpace = 1;     /* Created from user space */
	obj->extra = 0;

	DEBUG(3) {
		/* Add extra symbols */
		vaddr = entry->startAddr;
		sprintf(symbol, "!%08x.%s.ta_YSize", vaddr, pool->name);
		mmu_addSymbol(entry, vaddr+4, symbol);
	}

	return obj;
}

/* Destructor of mapping */
static void destructor(mmu_entry_t *entry)
{
	lib_graphics_s_textattr_t *obj;
	struct TextAttr *real;

	DEBUG(5) dprintf("graphics.library: lib_graphics_s_textattr: destructor() called\n");

	obj = (lib_graphics_s_textattr_t *)entry->nativePtr;

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
