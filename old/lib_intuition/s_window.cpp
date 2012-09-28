/*
lib_intuition/s_window.c - intuition.library struct Window

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
#include "s_window.h"
#include "../lib_graphics/s_rastport.h"
#include "s_screen.h"
#include "../lib_exec/s_msgport.h"
#include "../lib_graphics/s_layer.h"
#include "s_intuimessage.h"
#include "s_menu.h"

/* Predecl of destructor */
/* static void destructor(mmu_entry_t *entry); */

/* Object pool */
static mmu_pool_t *pool;

/* Init flag */
static int is_init = 0;

/* Read map function */
static int map_r(mmu_entry_t *entry, uint32_t offset, uint32_t size,
        uint8_t *data )
{
        uint32_t i;
        lib_intuition_s_window_t *obj;
	lib_graphics_s_rastport_t *rport = NULL;
	lib_intuition_s_screen_t *screen = NULL;
	lib_exec_s_msgport_t *userport = NULL;
	lib_graphics_s_layer_t *wlayer = NULL;
	lib_intuition_s_menu_t *menustrip = NULL;

        DEBUG(7) dprintf("intuition.library: struct Window: map_r() called. VAddr: 0x%x, Size: 0x%x\n", (unsigned int)(entry->startAddr+offset), (unsigned int)size);

        /* Get our object */
        obj = (lib_intuition_s_window_t *)entry->nativePtr;

        for(i=offset;i<(size+offset);i++) {
                switch(i) {

			/* LeftEdge */
			case 4: case 5:
				i = 5;
				break;

			/* TopEdge */
			case 6: case 7:
				i = 7;
				break;

			/* Width */
			case 8: case 9:
				i = 9;
				break;

			/* Height */
			case 10: case 11:
				i = 11;
				break;

                        /* MenuStrip */
                        case 28: case 29: case 30: case 31:
                                if(obj->real->MenuStrip == NULL) {
					menustrip = NULL;
				} else {
                                        menustrip = lib_intuition_s_menu_get_real(obj->real->MenuStrip);
                                }
                                i = 31;
				break;

                        /* WScreen */
                        case 46: case 47: case 48: case 49:
                                if(obj->real->WScreen == NULL) {
					screen = NULL;
				} else {
                                        screen = lib_intuition_s_screen_get_real(obj->real->WScreen);
                                }
                                i = 49;
				break;

                        /* RPort */
                        case 50: case 51: case 52: case 53:
                                if(obj->real->RPort == NULL) {
					rport = NULL;
				} else {
                                        rport = lib_graphics_s_rastport_get_real(obj->real->RPort);
                                }
                                i = 53;
				break;

			/* BorderLeft */
			case 54:
				break;

			/* BorderTop */
			case 55:
				break;

			/* BorderRight */
			case 56:
				break;

			/* BorderBottom */
			case 57:
				break;

                        /* UserPort */
                        case 86: case 87: case 88: case 89:
                                if(obj->real->UserPort == NULL) {
					userport = NULL;
				} else {
                                        userport = lib_exec_s_msgport_get_real(obj->real->UserPort);
					/* Make sure it is set up to handle
					   IntuiMessages */
					userport->type = MMU_TYPE_INTUITION_S_INTUIMESSAGE;
					userport->convertfn = lib_intuition_s_intuimessage_convert;
                                }
                                i = 89;
				break;

                        /* WLayer */
                        case 124: case 125: case 126: case 127:
                                if(obj->real->WLayer == NULL) {
					wlayer = NULL;
				} else {
                                        wlayer = lib_graphics_s_layer_get_real(obj->real->WLayer);
                                }
                                i = 127;
				break;

                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: intuition.library: struct Window: Read is not allowed. Offset=%u\n", (unsigned int)offset);
                                return 1;
                }
        }

        for(i=offset;i<(size+offset);i++) {
                switch(i) {

			/* LeftEdge */
			case 4:
				*data++ = (obj->real->LeftEdge >> 8) & 0xff;
				break;
			case 5:
				*data++ = (obj->real->LeftEdge) & 0xff;
				break;

			/* TopEdge */
			case 6:
				*data++ = (obj->real->TopEdge >> 8) & 0xff;
				break;
			case 7:
				*data++ = (obj->real->TopEdge) & 0xff;
				break;

			/* Width */
			case 8:
				*data++ = (obj->real->Width >> 8) & 0xff;
				break;
			case 9:
				*data++ = (obj->real->Width) & 0xff;
				break;

			/* Height */
			case 10:
				*data++ = (obj->real->Height >> 8) & 0xff;
				break;
			case 11:
				*data++ = (obj->real->Height) & 0xff;
				break;

                        /* MenuStrip */
                        case 28:
				if(menustrip) {
	                                *data++ = (menustrip->entry->startAddr >> 24) & 0xff;
				} else {
					*data++ = 0;
				}
                                break;
                        case 29:
				if(menustrip) {
	                                *data++ = (menustrip->entry->startAddr >> 16) & 0xff;
				} else {
					*data++ = 0;
				}
                                break;
                        case 30:
				if(menustrip) {
	                                *data++ = (menustrip->entry->startAddr >> 8) & 0xff;
				} else {
					*data++ = 0;
				}
                                break;
                        case 31:
				if(menustrip) {
	                                *data++ = (menustrip->entry->startAddr) & 0xff;
				} else {
					*data++ = 0;
				}
                                break;

                        /* WScreen */
                        case 46:
				if(screen) {
	                                *data++ = (screen->entry->startAddr >> 24) & 0xff;
				} else {
					*data++ = 0;
				}
                                break;
                        case 47:
				if(screen) {
	                                *data++ = (screen->entry->startAddr >> 16) & 0xff;
				} else {
					*data++ = 0;
				}
                                break;
                        case 48:
				if(screen) {
	                                *data++ = (screen->entry->startAddr >> 8) & 0xff;
				} else {
					*data++ = 0;
				}
                                break;
                        case 49:
				if(screen) {
	                                *data++ = (screen->entry->startAddr) & 0xff;
				} else {
					*data++ = 0;
				}
                                break;

                        /* RPort */
                        case 50:
				if(rport) {
	                                *data++ = (rport->entry->startAddr >> 24) & 0xff;
				} else {
					*data++ = 0;
				}
                                break;
                        case 51:
				if(rport) {
	                                *data++ = (rport->entry->startAddr >> 16) & 0xff;
				} else {
					*data++ = 0;
				}
                                break;
                        case 52:
				if(rport) {
	                                *data++ = (rport->entry->startAddr >> 8) & 0xff;
				} else {
					*data++ = 0;
				}
                                break;
                        case 53:
				if(rport) {
	                                *data++ = (rport->entry->startAddr) & 0xff;
				} else {
					*data++ = 0;
				}
                                break;

			/* BorderLeft */
			case 54:
				*data++ = obj->real->BorderLeft;
				break;

			/* BorderTop */
			case 55:
				*data++ = obj->real->BorderTop;
				break;

			/* BorderRight */
			case 56:
				*data++ = obj->real->BorderRight;
				break;

			/* BorderBottom */
			case 57:
				*data++ = obj->real->BorderBottom;
				break;

                        /* UserPort */
                        case 86:
				if(userport) {
	                                *data++ = (userport->entry->startAddr >> 24) & 0xff;
				} else {
					*data++ = 0;
				}
                                break;
                        case 87:
				if(userport) {
	                                *data++ = (userport->entry->startAddr >> 16) & 0xff;
				} else {
					*data++ = 0;
				}
                                break;
                        case 88:
				if(userport) {
	                                *data++ = (userport->entry->startAddr >> 8) & 0xff;
				} else {
					*data++ = 0;
				}
                                break;
                        case 89:
				if(userport) {
	                                *data++ = (userport->entry->startAddr) & 0xff;
				} else {
					*data++ = 0;
				}
                                break;

                        /* WLayer */
                        case 124:
				if(wlayer) {
	                                *data++ = (wlayer->entry->startAddr >> 24) & 0xff;
				} else {
					*data++ = 0;
				}
                                break;
                        case 125:
				if(wlayer) {
	                                *data++ = (wlayer->entry->startAddr >> 16) & 0xff;
				} else {
					*data++ = 0;
				}
                                break;
                        case 126:
				if(wlayer) {
	                                *data++ = (wlayer->entry->startAddr >> 8) & 0xff;
				} else {
					*data++ = 0;
				}
                                break;
                        case 127:
				if(wlayer) {
	                                *data++ = (wlayer->entry->startAddr) & 0xff;
				} else {
					*data++ = 0;
				}
                                break;

                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: intuition.library: struct Window: Read is not allowed. Offset=%u\n", (unsigned int)offset);
                                return 1;
                }
        }

        return 0;
}

/* Global init */
int lib_intuition_s_window_init()
{
	DEBUG(4) dprintf("intuition.library: lib_intuition_s_window_init() called\n");

        pool = mmu_pool_create(
                "intuition.s.Window",
                MMU_TYPE_INTUITION_S_WINDOW,
                LIB_INTUITION_S_WINDOW_SIZE,
                sizeof(lib_intuition_s_window_t)
        );

        if(pool == NULL)
                return 1;

	is_init = 1;
	return 0;
}

/* Global cleanup */
void lib_intuition_s_window_cleanup()
{
	DEBUG(4) dprintf("intuition.library: lib_intuition_s_window_cleanup() called\n");

        if( !is_init ) {
                DEBUG(2) dprintf("Warning: intuition.library: lib_intuition_s_window_cleanup: Not initialized\n");
                return;
        }

	mmu_pool_destroy(pool);

	is_init = 0;
}

/* Get an instance */
lib_intuition_s_window_t *lib_intuition_s_window_get_real(struct Window *real)
{
	mmu_entry_t *entry;
	lib_intuition_s_window_t *obj;
	uint32_t vaddr;
	char symbol[200];

	DEBUG(5) dprintf("intuition.library: lib_intuition_s_window_get_real() called\n");

	if(real == NULL)
		return NULL;

        /* Check to find if it already exists */
        entry = mmu_findEntryWithTypeReal(real, MMU_TYPE_INTUITION_S_WINDOW);
        if(entry != NULL) {

                /* Yes, return object */
                return (lib_intuition_s_window_t *)entry->nativePtr;
        }

        /* No, set up an object */
        entry = mmu_allocObj(pool, real);
        if(entry == NULL)
                 return NULL;

        /* Extract object */
        obj = (lib_intuition_s_window_t *)entry->nativePtr;

        /* Init structure */
	entry->read = map_r;
/*	entry->destructor = destructor; */
        obj->entry = entry;
        obj->real = real;

        DEBUG(3) {
                /* Add extra symbols */
                vaddr = entry->startAddr;
                sprintf(symbol, "!%08x.%s.LeftEdge", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+4, symbol);
                sprintf(symbol, "!%08x.%s.TopEdge", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+6, symbol);
                sprintf(symbol, "!%08x.%s.Width", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+8, symbol);
                sprintf(symbol, "!%08x.%s.Height", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+10, symbol);
                sprintf(symbol, "!%08x.%s.MenuStrip", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+28, symbol);
                sprintf(symbol, "!%08x.%s.WScreen", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+46, symbol);
                sprintf(symbol, "!%08x.%s.RPort", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+50, symbol);
                sprintf(symbol, "!%08x.%s.BorderLeft", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+54, symbol);
                sprintf(symbol, "!%08x.%s.BorderTop", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+55, symbol);
                sprintf(symbol, "!%08x.%s.BorderRight", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+56, symbol);
                sprintf(symbol, "!%08x.%s.BorderBottom", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+57, symbol);
                sprintf(symbol, "!%08x.%s.UserPort", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+86, symbol);
                sprintf(symbol, "!%08x.%s.WLayer", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+124, symbol);
        }

	return obj;
}

/* Get an instance */
lib_intuition_s_window_t *lib_intuition_s_window_get_vaddr(uint32_t vaddr)
{
	mmu_entry_t *entry;

	DEBUG(5) dprintf("intuition.library: lib_intuition_s_window_get_vaddr() called\n");

	if(vaddr == 0)
                return NULL;

        /* Search for an existing object */
        entry = mmu_findEntryWithType(vaddr, MMU_TYPE_INTUITION_S_WINDOW);

        /* This structure should not be created from user space */
        if(entry == NULL)
                return NULL;

        return (lib_intuition_s_window_t *)entry->nativePtr;
}

/* Destructor of mapping */
/*
static void destructor(mmu_entry_t *entry)
{
        lib_intuition_s_window_t *obj;

        DEBUG(5) dprintf("intuition.library: lib_intuition_s_window: destructor() called\n");

        obj = (lib_intuition_s_window_t *)entry->nativePtr;

        mmu_freeObj(entry);
}
*/
