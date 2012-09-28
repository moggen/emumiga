/*
lib_intuition/s_intuimessage.c - intuition.library struct IntuiMessage

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

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "s_intuimessage.h"
#include "s_gadget.h"
#include "../lib_exec/s_message.h"

/* Read map function */
static int map_r(mmu_entry_t *entry, uint32_t offset, uint32_t size,
        uint8_t *data )
{
        uint32_t i;
	lib_exec_s_message_t *obj;
	struct IntuiMessage *real;
	uint32_t iaddress_vaddr = 0;
	lib_intuition_s_gadget_t *gadget;
	mmu_entry_t *entry2;

        DEBUG(7) dprintf("intuition.library: struct IntuiMessage: map_r() called. VAddr: 0x%x, Offset: 0x%x, Size: 0x%x\n", (unsigned int)(entry->startAddr+offset), (unsigned int)offset, (unsigned int)size);

        /* Get our object */
        obj = (lib_exec_s_message_t *)entry->nativePtr;
	real = (struct IntuiMessage *)obj->real;

        for(i=offset;i<(size+offset);i++) {
                switch(i) {

			/* Class */
			case 20: case 21: case 22: case 23:
				i = 23;
				break;

			/* Code */
			case 24: case 25:
				i = 25;
				break;

			/* Qualifier */
			case 26: case 27:
				i = 27;
				break;

			/* IAddress */
			case 28: case 29: case 30: case 31:
				if(real->IAddress == NULL) {
					iaddress_vaddr = 0;
					i = 31;
					break;
				}
				switch(real->Class) {

					case IDCMP_GADGETDOWN:
					case IDCMP_GADGETUP:

						/* IAddress is a gadget */
						gadget = lib_intuition_s_gadget_get_real((struct Gadget *)real->IAddress);
						if(gadget == NULL) {
			                                dprintf("Error: intuition.library: struct IntuiMessage: IAddress could not be set up as a Gadget structure. Class=0x%x, Offset=%u\n", real->Class, i);
			                                return 1;
						}
						iaddress_vaddr = gadget->entry->startAddr;
						break;

					case IDCMP_MOUSEMOVE:

						/* IAddress is a gadget OR a window */
						entry2 = mmu_findEntryWithTypeReal(real->IAddress, MMU_TYPE_INTUITION_S_GADGET);
						if(entry2 != NULL) {
							iaddress_vaddr = entry2->startAddr;
							break;
						}
						entry2 = mmu_findEntryWithTypeReal(real->IAddress, MMU_TYPE_INTUITION_S_WINDOW);
						if(entry2 != NULL) {
							iaddress_vaddr = entry2->startAddr;
							break;
						}
		                                dprintf("Error: intuition.library: struct IntuiMessage: IAddress could not be set up as a Gadget or Window structure. Class=0x%x, Offset=%u\n", real->Class, i);
		                                return 1;

					default:
						/* IAddress is set but we
						   don't know the type of
						   it.. */
		                                dprintf("Error: intuition.library: struct IntuiMessage: IAddress is set but the type of it is not known. Class=0x%x, IAddress=%p, Offset=%u\n", real->Class, real->IAddress, i);
		                                return 1;
				}
				i = 31;
				break;

                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: intuition.library: struct IntuiMessage: Read is not allowed. Offset=%u\n", (unsigned int)offset);
                                return 1;
                }
        }

        for(i=offset;i<(size+offset);i++) {
                switch(i) {

			/* Class */
                        case 20:
                                *data++ = (real->Class >> 24) & 0xff;
                                break;
                        case 21:
                                *data++ = (real->Class >> 16) & 0xff;
                                break;
                        case 22:
                                *data++ = (real->Class >> 8) & 0xff;
                                break;
                        case 23:
                                *data++ = (real->Class) & 0xff;
                                break;

			/* Code */
                        case 24:
                                *data++ = (real->Code >> 8) & 0xff;
                                break;
                        case 25:
                                *data++ = (real->Code) & 0xff;
                                break;

			/* Qualifier */
                        case 26:
                                *data++ = (real->Qualifier >> 8) & 0xff;
                                break;
                        case 27:
                                *data++ = (real->Qualifier) & 0xff;
                                break;

			/* IAddress */
                        case 28:
                                *data++ = (iaddress_vaddr >> 24) & 0xff;
                                break;
                        case 29:
                                *data++ = (iaddress_vaddr >> 16) & 0xff;
                                break;
                        case 30:
                                *data++ = (iaddress_vaddr >> 8) & 0xff;
                                break;
                        case 31:
                                *data++ = (iaddress_vaddr) & 0xff;
                                break;

                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: intuition.library: struct IntuiMessage: Read is not allowed. Offset=%u\n", (unsigned int)offset);
                                return 1;
                }
        }
        return 0;
}

#if 0
/* Write map function */
static int map_w(mmu_entry_t *entry, uint32_t offset, uint32_t size,
        uint8_t *data )
{
        uint32_t i;
	lib_exec_s_message_t *obj;
	struct IntuiMessage *real;

        DEBUG(7) dprintf("intuition.library: struct IntuiMessage: map_w() called. VAddr: 0x%x, Offset: 0x%x, Size: 0x%x\n", (unsigned int)(entry->startAddr+offset), (unsigned int)offset, (unsigned int)size);

        /* Get our object */
        obj = (lib_exec_s_message_t *)entry->nativePtr;
	real = (struct IntuiMessage *)obj->real;

        for(i=offset;i<(size+offset);i++) {
                switch(i) {

			/* tr_node.io_Command */
			case 28: case 29:
				i = 29;
				break;

			/* tr_time.tv_secs */
			case 32: case 33: case 34: case 35:
				i = 35;
				break;

			/* tr_time.tv_micro */
			case 36: case 37: case 38: case 39:
				i = 39;
				break;

                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: intuition.library: struct IntuiMessage: Write is not allowed. Offset=%u\n", (unsigned int)offset);
                                return 1;
                }
        }

        for(i=offset;i<(size+offset);i++) {
                switch(i) {

			/* tr_node.io_Command */
                        case 28:
				real->tr_node.io_Command &= 0x00ff;
				real->tr_node.io_Command |= (*data++ << 8);
                                break;
                        case 29:
				real->tr_node.io_Command &= 0xff00;
				real->tr_node.io_Command |= (*data++);
                                break;

			/* tr_time.tv_secs */
                        case 32:
                                real->tr_time.tv_secs &= 0x00ffffff;
                                real->tr_time.tv_secs |= (*data++ << 24);
                                break;
                        case 33:
                                real->tr_time.tv_secs &= 0xff00ffff;
                                real->tr_time.tv_secs |= (*data++ << 16);
                                break;
                        case 34:
                                real->tr_time.tv_secs &= 0xffff00ff;
                                real->tr_time.tv_secs |= (*data++ << 8);
                                break;
                        case 35:
                                real->tr_time.tv_secs &= 0xffffff00;
                                real->tr_time.tv_secs |= (*data++);
                                break;

			/* tr_time.tv_micro */
                        case 36:
                                real->tr_time.tv_micro &= 0x00ffffff;
                                real->tr_time.tv_micro |= (*data++ << 24);
                                break;
                        case 37:
                                real->tr_time.tv_micro &= 0xff00ffff;
                                real->tr_time.tv_micro |= (*data++ << 16);
                                break;
                        case 38:
                                real->tr_time.tv_micro &= 0xffff00ff;
                                real->tr_time.tv_micro |= (*data++ << 8);
                                break;
                        case 39:
                                real->tr_time.tv_micro &= 0xffffff00;
                                real->tr_time.tv_micro |= (*data++);
                                break;

                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: intuition.library: struct IntuiMessage: Write is not allowed. Offset=%u\n", (unsigned int)offset);
                                return 1;
                }
        }
        return 0;
}
#endif

/* Convert Message structure into a IntuiMessage structure */
int lib_intuition_s_intuimessage_convert(lib_exec_s_message_t *message)
{
	mmu_entry_t *entry;
	char symbol[200];

	DEBUG(5) dprintf("intuition.library: lib_intuition_s_intuimessage_convert() called\n");

	if(message->type == MMU_TYPE_INTUITION_S_INTUIMESSAGE) {
		/* Already correct type */
		return 0;
	}

	if(message->type != MMU_TYPE_EXEC_S_MESSAGE) {
		/* Set to another type.. trouble */
		return 1;
	}

	/* We have an undefined type, convert it */

	if(message->size < LIB_INTUITION_S_INTUIMESSAGE_SIZE) {
		/* Wrong size */
		return 1;
	}

	message->type = MMU_TYPE_INTUITION_S_INTUIMESSAGE;

	entry = message->entry;
	entry->read = map_r;
/*	entry->write = map_w; */

        DEBUG(3) {
		/* Remove old symbols */
//                mmu_removeSymbols(entry);

                /* Add symbols */
                sprintf(symbol, "!%08x.intuition.s.IntuiMessage", entry->startAddr);
                mmu_addSymbol(entry, entry->startAddr, symbol);
                sprintf(symbol, "!%08x.intuition.s.IntuiMessage.Class", entry->startAddr);
                mmu_addSymbol(entry, entry->startAddr + 20, symbol);
                sprintf(symbol, "!%08x.intuition.s.IntuiMessage.Code", entry->startAddr);
                mmu_addSymbol(entry, entry->startAddr + 24, symbol);
                sprintf(symbol, "!%08x.intuition.s.IntuiMessage.Qualifier", entry->startAddr);
                mmu_addSymbol(entry, entry->startAddr + 26, symbol);
                sprintf(symbol, "!%08x.intuition.s.IntuiMessage.IAddress", entry->startAddr);
                mmu_addSymbol(entry, entry->startAddr + 28, symbol);
        }

	return 0;
}
