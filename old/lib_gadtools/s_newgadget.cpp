/*
lib_gadtools/s_newgadget.c - gadtools.library struct NewGadget

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
#include <graphics/text.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "s_newgadget.h"
#include "s_visualinfo.h"
#include "../lib_graphics/s_textattr.h"


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
	lib_gadtools_s_newgadget_t *obj;

	DEBUG(7) dprintf("gadtools.library: struct NewGaget: map_r() called. VAddr: 0x%x, Size: 0x%x\n", (unsigned int)(entry->startAddr+offset), (unsigned int)size);

	/* Get our object */
	obj = (lib_gadtools_s_newgadget_t *)entry->nativePtr;

	for(i=offset;i<(size+offset);i++) {
		switch(i) {

			/* ng_LeftEdge */
			case 0: case 1:
				i = 1;
				break;

			/* ng_TopEdge */
			case 2: case 3:
				i = 3;
				break;

			/* ng_Width */
			case 4: case 5:
				i = 5;
				break;

			/* ng_Height */
			case 6: case 7:
				i = 7;
				break;

			/* ng_GadgetText */
			case 8: case 9: case 10: case 11:
				obj->gadgettext_wmask = 0;
				i = 11;
				break;

			default:
				/* Nothing found, signal error */
				DEBUG(1) dprintf("Error: gadtools.library: struct NewGadget: Read is not allowed. Offset=%u\n", (unsigned int)offset);
				return 1;
		}
	}

	for(i=offset;i<(size+offset);i++) {
		switch(i) {

			/* ng_LeftEdge */
			case 0:
				*data++ = (obj->real->ng_LeftEdge >> 8) & 0xff;
				break;
			case 1:
				*data++ = (obj->real->ng_LeftEdge) & 0xff;
				break;

			/* ng_TopEdge */
			case 2:
				*data++ = (obj->real->ng_TopEdge >> 8) & 0xff;
				break;
			case 3:
				*data++ = (obj->real->ng_TopEdge) & 0xff;
				break;

			/* ng_Width */
			case 4:
				*data++ = (obj->real->ng_Width >> 8) & 0xff;
				break;
			case 5:
				*data++ = (obj->real->ng_Width) & 0xff;
				break;

			/* ng_Height */
			case 6:
				*data++ = (obj->real->ng_Height >> 8) & 0xff;
				break;
			case 7:
				*data++ = (obj->real->ng_Height) & 0xff;
				break;

			/* ng_GadgetText */
			case 8:
				*data++ = (obj->gadgettext_vaddr >> 24) & 0xff;
				break;
			case 9:
				*data++ = (obj->gadgettext_vaddr >> 16) & 0xff;
				break;
			case 10:
				*data++ = (obj->gadgettext_vaddr >> 8) & 0xff;
				break;
			case 11:
				*data++ = (obj->gadgettext_vaddr) & 0xff;
				break;

			/* ng_GadgetID */
			case 16:
				*data++ = (obj->real->ng_GadgetID >> 8) & 0xff;
				break;
			case 17:
				*data++ = (obj->real->ng_GadgetID) & 0xff;
				break;

			default:
				/* Nothing found, signal error */
				DEBUG(1) dprintf("Error: gadtools.library: struct NewGadget: Read is not allowed. Offset=%u\n", (unsigned int)offset);
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
	lib_gadtools_s_newgadget_t *obj;
	uint32_t vaddr;

	DEBUG(7) dprintf("gadtools.library: struct NewGaget: map_w() called. VAddr: 0x%x, Size: 0x%x\n", (unsigned int)(entry->startAddr+offset), (unsigned int)size);

	/* Get our object */
	obj = (lib_gadtools_s_newgadget_t *)entry->nativePtr;

	for(i=offset;i<(size+offset);i++) {
		switch(i) {

			/* ng_LeftEdge */
			case 0:
				obj->real->ng_LeftEdge &= 0x00ff;
				obj->real->ng_LeftEdge |= ((uint16_t)*data++ << 8);
				break;
			case 1:
				obj->real->ng_LeftEdge &= 0xff00;
				obj->real->ng_LeftEdge |= ((uint16_t)*data++);
				break;

			/* ng_TopEdge */
			case 2:
				obj->real->ng_TopEdge &= 0x00ff;
				obj->real->ng_TopEdge |= ((uint16_t)*data++ << 8);
				break;
			case 3:
				obj->real->ng_TopEdge &= 0xff00;
				obj->real->ng_TopEdge |= ((uint16_t)*data++);
				break;

			/* ng_Width */
			case 4:
				obj->real->ng_Width &= 0x00ff;
				obj->real->ng_Width |= ((uint16_t)*data++ << 8);
				break;
			case 5:
				obj->real->ng_Width &= 0xff00;
				obj->real->ng_Width |= ((uint16_t)*data++);
				break;

			/* ng_Height */
			case 6:
				obj->real->ng_Height &= 0x00ff;
				obj->real->ng_Height |= ((uint16_t)*data++ << 8);
				break;
			case 7:
				obj->real->ng_Height &= 0xff00;
				obj->real->ng_Height |= ((uint16_t)*data++);
				break;

			/* ng_GadgetText */
			case 8:
				obj->gadgettext_wtmp[0] = *data++;
				obj->gadgettext_wmask |= 1 << 0;
				break;
			case 9:
				obj->gadgettext_wtmp[1] = *data++;
				obj->gadgettext_wmask |= 1 << 1;
				break;
			case 10:
				obj->gadgettext_wtmp[2] = *data++;
				obj->gadgettext_wmask |= 1 << 2;
				break;
			case 11:
				obj->gadgettext_wtmp[3] = *data++;
				obj->gadgettext_wmask |= 1 << 3;
				break;

			/* ng_GadgetID */
			case 16:
				obj->real->ng_GadgetID &= 0x00ff;
				obj->real->ng_GadgetID |= ((uint16_t)*data++ << 8);
				break;
			case 17:
				obj->real->ng_GadgetID &= 0xff00;
				obj->real->ng_GadgetID |= ((uint16_t)*data++);
				break;

			default:
				/* Nothing found, signal error */
				DEBUG(1) dprintf("Error: gadtools.library: struct NewGadget: Write is not allowed. Offset=%u\n", (unsigned int)offset);
				return 1;
		}
	}

	/* Check for updated pointers */

	for(i=offset;i<(size+offset);i++) {
		switch(i) {

			/* ng_GadgetText */
			case 8: case 9: case 10: case 11:
				if(obj->gadgettext_wmask == 0x0f) {
					vaddr = ((uint32_t)obj->gadgettext_wtmp[0] << 24) |
						((uint32_t)obj->gadgettext_wtmp[1] << 16) |
						((uint32_t)obj->gadgettext_wtmp[2] << 8) |
						((uint32_t)obj->gadgettext_wtmp[3]);

					obj->real->ng_GadgetText = (STRPTR)vptr(vaddr);
					obj->gadgettext_vaddr = vaddr;
				}
				i = 11;
				break;
		}
	}


	return 0;
}

/* Global init */
int lib_gadtools_s_newgadget_init()
{
	DEBUG(4) dprintf("gadtools.library: lib_gadtools_s_newgadget_init() called\n");

        pool = mmu_pool_create(
                "gadtools.s.NewGadget",
                MMU_TYPE_GADTOOLS_S_NEWGADGET,
                LIB_GADTOOLS_S_NEWGADGET_SIZE,
                sizeof(lib_gadtools_s_newgadget_t)
        );

        if(pool == NULL)
                return 1;

	is_init = 1;
	return 0;
}

/* Global cleanup */
void lib_gadtools_s_newgadget_cleanup()
{
	DEBUG(4) dprintf("gadtools.library: lib_gadtools_s_newgadget_cleanup() called\n");

        if( !is_init ) {
                DEBUG(2) dprintf("Warning: gadtools.library: lib_gadtools_s_newgadget_cleanup: Not initialized\n");
                return;
        }

	mmu_pool_destroy(pool);

	is_init = 0;
}

/* Get an instance */
lib_gadtools_s_newgadget_t *lib_gadtools_s_newgadget_get_real(struct NewGadget *real)
{
	mmu_entry_t *entry;

	DEBUG(5) dprintf("gadtools.library: lib_gadtools_s_newgadget_get_real() called\n");

	if(real == NULL)
		return NULL;

        /* Check to find if it already exists */
        entry = mmu_findEntryWithTypeReal(real, MMU_TYPE_GADTOOLS_S_NEWGADGET);
	if(entry != NULL) {
		/* Yes, return object */
		return (lib_gadtools_s_newgadget_t *)entry->nativePtr;
	}

        /* This structure should not be created from real space */
        return NULL;

/*
        return (lib_gadtools_s_newgadget_t *)entry->nativePtr;
*/
}

/* Get an instance */
lib_gadtools_s_newgadget_t *lib_gadtools_s_newgadget_get_vaddr(uint32_t vaddr)
{
	mmu_entry_t *entry;
	lib_gadtools_s_newgadget_t *obj;
	struct NewGadget *real;

	uint16_t ng_leftedge, ng_topedge, ng_width, ng_height;
	uint16_t ng_gadgetid;
	uint32_t ng_gadgettext, ng_textattr, ng_flags;
	uint32_t ng_visualinfo, ng_userdata;

	lib_graphics_s_textattr_t *ng_textattr_obj;
	lib_gadtools_s_visualinfo_t *ng_visualinfo_obj;

	char symbol[200];

	DEBUG(5) dprintf("gadtools.library: lib_gadtools_s_newgadget_get_vaddr() called\n");

	if(vaddr == 0)
                return NULL;

        /* Search for an existing object */
        entry = mmu_findEntryWithType(vaddr, MMU_TYPE_GADTOOLS_S_NEWGADGET);
        if(entry != NULL)
                return (lib_gadtools_s_newgadget_t *)entry->nativePtr;

	/* This structure can be created from user space */

	/* Read parameters */
	if(
		READMEM_16(  vaddr,   &ng_leftedge   ) ||
		READMEM_16( vaddr+2,  &ng_topedge    ) ||
		READMEM_16( vaddr+4,  &ng_width      ) ||
		READMEM_16( vaddr+6,  &ng_height     ) ||
		READMEM_32( vaddr+8,  &ng_gadgettext ) ||
		READMEM_32( vaddr+12, &ng_textattr   ) ||
		READMEM_16( vaddr+16, &ng_gadgetid   ) ||
		READMEM_32( vaddr+18, &ng_flags      ) ||
		READMEM_32( vaddr+22, &ng_visualinfo ) ||
		READMEM_32( vaddr+26, &ng_userdata   )
	) {
		return NULL;
	}

	/* Allocate memory for structure */
	real = (struct NewGadget *)AllocVec(sizeof(struct NewGadget), MEMF_PUBLIC|MEMF_CLEAR);
	if(real == NULL) {
		return NULL;
	}

	/* Set up real object */
	real->ng_LeftEdge = ng_leftedge;
	real->ng_TopEdge = ng_topedge;
	real->ng_Width = ng_height;
	real->ng_Height = ng_height;

	if(ng_gadgettext != 0) {
		real->ng_GadgetText = (STRPTR)vptr(ng_gadgettext);
		/* TODO: Better checks, make copy? */
	}

	if(ng_textattr != 0) {
		ng_textattr_obj = lib_graphics_s_textattr_get_vaddr(ng_textattr);
		if(ng_textattr_obj == NULL) {
			FreeVec(real);
			return NULL;
		}
		real->ng_TextAttr = ng_textattr_obj->real;
	}

	real->ng_GadgetID = ng_gadgetid;
	real->ng_Flags = ng_flags;

	if(ng_visualinfo != 0) {
		ng_visualinfo_obj = lib_gadtools_s_visualinfo_get_vaddr(ng_visualinfo);
		if(ng_visualinfo_obj == NULL) {
			FreeVec(real);
			return NULL;
		}
		real->ng_VisualInfo = ng_visualinfo_obj->real;
	}

	/* Keep the vaddr in this field. The OS should not read this
	   field, only the application */
	real->ng_UserData = (APTR)ng_userdata;


	/* Set up an object. */
	entry = mmu_allocObjVAddr(pool, real, vaddr);
	if(entry == NULL) {
		FreeVec(real);
		return NULL;
	}

	/* Extract object */
	obj = (lib_gadtools_s_newgadget_t *)entry->nativePtr;

	/* Init structure */
	entry->read = map_r;
	entry->write = map_w;
        entry->destructor = destructor;
        obj->entry = entry;
        obj->real = real;
        obj->userSpace = 1;     /* Created from user space */

	obj->gadgettext_vaddr = ng_gadgettext;

        DEBUG(3) {
                sprintf(symbol, "!%08x.%s.ng_LeftEdge", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+0, symbol);
                sprintf(symbol, "!%08x.%s.ng_TopEdge", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+2, symbol);
                sprintf(symbol, "!%08x.%s.ng_Width", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+4, symbol);
                sprintf(symbol, "!%08x.%s.ng_Height", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+6, symbol);
                sprintf(symbol, "!%08x.%s.ng_GadgetText", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+8, symbol);
                sprintf(symbol, "!%08x.%s.ng_TextAttr", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+12, symbol);
                sprintf(symbol, "!%08x.%s.ng_GagetID", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+16, symbol);
                sprintf(symbol, "!%08x.%s.ng_Flags", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+18, symbol);
                sprintf(symbol, "!%08x.%s.ng_VisualInfo", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+22, symbol);
                sprintf(symbol, "!%08x.%s.ng_UserData", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+26, symbol);
        }

        return obj;
}
/* Destructor of mapping */
static void destructor(mmu_entry_t *entry)
{
	lib_gadtools_s_newgadget_t *obj;
	struct NewGadget *real;

	DEBUG(5) dprintf("gadtools.library: lib_gadtools_s_newgadget: destructor() called\n");

	obj = (lib_gadtools_s_newgadget_t *)entry->nativePtr;

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
