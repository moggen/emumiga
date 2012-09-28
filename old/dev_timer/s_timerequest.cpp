/*
dev_timer/s_timerequest.c - timer.device struct timerequest

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

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "timer.h"
#include "s_timerequest.h"
#include "../lib_exec/s_iorequest.h"

/* Read map function */
static int map_r(mmu_entry_t *entry, uint32_t offset, uint32_t size,
        uint8_t *data )
{
        uint32_t i;

        DEBUG(7) dprintf("timer.device: struct timerequest: map_r() called. VAddr: 0x%x, Offset: 0x%x, Size: 0x%x\n", (unsigned int)(entry->startAddr+offset), (unsigned int)offset, (unsigned int)size);

        for(i=offset;i<(size+offset);i++) {
                switch(i) {

			/* tr_node.io_Device */
			case 20: case 21: case 22: case 23:
				i = 23;
				break;

                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: timer.device: struct timerequest: Read is not allowed. Offset=%u\n", (unsigned int)offset);
                                return 1;
                }
        }

        for(i=offset;i<(size+offset);i++) {
                switch(i) {

			/* tr_node.io_Device */
                        case 20:
                                *data++ = (dev_timer_base.entry->startAddr >> 24) & 0xff;
                                break;
                        case 21:
                                *data++ = (dev_timer_base.entry->startAddr >> 16) & 0xff;
                                break;
                        case 22:
                                *data++ = (dev_timer_base.entry->startAddr >> 8) & 0xff;
                                break;
                        case 23:
                                *data++ = (dev_timer_base.entry->startAddr) & 0xff;
                                break;

                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: timer.device: struct timerequest: Read is not allowed. Offset=%u\n", (unsigned int)offset);
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
	lib_exec_s_iorequest_t *obj;
	struct timerequest *real;

        DEBUG(7) dprintf("timer.device: struct timerequest: map_w() called. VAddr: 0x%x, Offset: 0x%x, Size: 0x%x\n", (unsigned int)(entry->startAddr+offset), (unsigned int)offset, (unsigned int)size);

        /* Get our object */
        obj = (lib_exec_s_iorequest_t *)entry->nativePtr;
	real = (struct timerequest *)obj->real;

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
                                dprintf("Error: timer.device: struct timerequest: Write is not allowed. Offset=%u\n", (unsigned int)offset);
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
                                dprintf("Error: timer.device: struct timerequest: Write is not allowed. Offset=%u\n", (unsigned int)offset);
                                return 1;
                }
        }
        return 0;
}

/* Convert IORequest structure into a timerequest structure */
int dev_timer_s_timerequest_convert(lib_exec_s_iorequest_t *iorequest)
{
	mmu_entry_t *entry;
	char symbol[200];

	DEBUG(5) dprintf("timer.device: dev_timer_s_timerequest_convert() called\n");

	if(iorequest->type == MMU_TYPE_TIMER_S_TIMEREQUEST) {
		/* Already correct type */
		return 0;
	}

	if(iorequest->type != MMU_TYPE_EXEC_S_IOREQUEST) {
		/* Set to another type.. trouble */
		return 1;
	}

	/* We have an undefined type, convert it */

	iorequest->type = MMU_TYPE_TIMER_S_TIMEREQUEST;

	entry = iorequest->entry;
	entry->read = map_r;
	entry->write = map_w;

        DEBUG(3) {
		/* Remove old symbols */
//                mmu_removeSymbols(entry);

                /* Add symbols */
                sprintf(symbol, "!%08x.timer.s.timerequest", entry->startAddr);
                mmu_addSymbol(entry, entry->startAddr, symbol);
                sprintf(symbol, "!%08x.timer.s.timerequest.tr_node.io_Device", entry->startAddr);
                mmu_addSymbol(entry, entry->startAddr + 20, symbol);
                sprintf(symbol, "!%08x.timer.s.timerequest.tr_node.io_Command", entry->startAddr);
                mmu_addSymbol(entry, entry->startAddr + 28, symbol);
                sprintf(symbol, "!%08x.timer.s.timerequest.tr_time.tv_secs", entry->startAddr);
                mmu_addSymbol(entry, entry->startAddr + 32, symbol);
                sprintf(symbol, "!%08x.timer.s.timerequest.tr_time.tv_micro", entry->startAddr);
                mmu_addSymbol(entry, entry->startAddr + 36, symbol);
        }

	return 0;
}
