/*
lib_graphics/s_gelsinfo.c - graphics.library struct GelsInfo

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

#include <graphics/gfx.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "s_gelsinfo.h"

/* Object pool */
static mmu_pool_t *pool;

/* Init flag */
static int is_init = 0;

/* Global init */
int lib_graphics_s_gelsinfo_init()
{
	DEBUG(4) dprintf("graphics.library: lib_graphics_s_gelsinfo_init() called\n");

        pool = mmu_pool_create(
                "graphics.s.GelsInfo",
                MMU_TYPE_GRAPHICS_S_GELSINFO,
                LIB_GRAPHICS_S_GELSINFO_SIZE,
                sizeof(lib_graphics_s_gelsinfo_t)
        );

        if(pool == NULL)
                return 1;

	is_init = 1;
	return 0;
}

/* Global cleanup */
void lib_graphics_s_gelsinfo_cleanup()
{
	DEBUG(4) dprintf("graphics.library: lib_graphics_s_gelsinfo_cleanup() called\n");

        if( !is_init ) {
                DEBUG(2) dprintf("Warning: graphics.library: lib_graphics_s_gelsinfo_cleanup: Not initialized\n");
                return;
        }

	mmu_pool_destroy(pool);

	is_init = 0;
}

/* Get an instance */
lib_graphics_s_gelsinfo_t *lib_graphics_s_gelsinfo_get_real(struct GelsInfo *real)
{
	mmu_entry_t *entry;
	lib_graphics_s_gelsinfo_t *obj;

	DEBUG(5) dprintf("graphics.library: lib_graphics_s_gelsinfo_get_real() called\n");

	if(real == NULL)
		return NULL;

        /* Check to find if it already exists */
        entry = mmu_findEntryWithTypeReal(real, MMU_TYPE_GRAPHICS_S_GELSINFO);
        if(entry != NULL) {

                /* Yes, return object */
                return (lib_graphics_s_gelsinfo_t *)entry->nativePtr;
        }

        /* No, set up an object */
        entry = mmu_allocObj(pool, real);
        if(entry == NULL)
                 return NULL;

        /* Extract object */
        obj = (lib_graphics_s_gelsinfo_t *)entry->nativePtr;

        /* Init structure */
        obj->entry = entry;
        obj->real = real;

	return obj;
}

/* Get an instance */
lib_graphics_s_gelsinfo_t *lib_graphics_s_gelsinfo_get_vaddr(uint32_t vaddr)
{
	mmu_entry_t *entry;

	DEBUG(5) dprintf("graphics.library: lib_graphics_s_gelsinfo_get_vaddr() called\n");

	if(vaddr == 0)
                return NULL;

        /* Search for an existing object */
        entry = mmu_findEntryWithType(vaddr, MMU_TYPE_GRAPHICS_S_GELSINFO);

        /* This structure should not be created from user space */
        if(entry == NULL)
                return NULL;

        return (lib_graphics_s_gelsinfo_t *)entry->nativePtr;
}

/* Get an instance as a substructure */
lib_graphics_s_gelsinfo_t *lib_graphics_s_gelsinfo_get_substruct(struct GelsInfo *real, uint32_t vaddr)
{
	mmu_entry_t *entry;
	lib_graphics_s_gelsinfo_t *obj;

	DEBUG(5) dprintf("graphics.library: lib_graphics_s_gelsinfo_get_substruct() called\n");

	if(real == NULL)
		return NULL;

        /* Check to find if it already exists */
        entry = mmu_findEntryWithTypeReal(real, MMU_TYPE_GRAPHICS_S_GELSINFO);
        if(entry != NULL) {

                /* Yes, return object */
                return (lib_graphics_s_gelsinfo_t *)entry->nativePtr;
        }

        /* No, set up a substruct object */
        entry = mmu_allocObjSubStruct(pool, real, vaddr);
        if(entry == NULL)
                 return NULL;

        /* Extract object */
        obj = (lib_graphics_s_gelsinfo_t *)entry->nativePtr;

        /* Init structure */
        obj->entry = entry;
        obj->real = real;

	return obj;
}
