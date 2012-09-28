/*
dev_audio/s_ioaudio.c - audio.device struct IOAudio

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

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "audio.h"
#include "s_ioaudio.h"
#include "../lib_exec/s_iorequest.h"

/* Predecl of destructor */
static void destructor(mmu_entry_t *entry);

/* Read map function */
static int map_r(mmu_entry_t *entry, uint32_t offset, uint32_t size,
        uint8_t *data )
{
        uint32_t i;
	lib_exec_s_iorequest_t *obj;
	struct IOAudio *real;
	dev_audio_s_ioaudio_ext_t *ioaudio_ext;

        DEBUG(7) dprintf("audio.device: struct IOAudio: map_r() called. VAddr: 0x%x, Offset: 0x%x, Size: 0x%x\n", (unsigned int)(entry->startAddr+offset), (unsigned int)offset, (unsigned int)size);

        /* Get our object */
        obj = (lib_exec_s_iorequest_t *)entry->nativePtr;
	real = (struct IOAudio *)obj->real;
	ioaudio_ext = (dev_audio_s_ioaudio_ext_t *)obj->extended;

        for(i=offset;i<(size+offset);i++) {
                switch(i) {

			/* ioa_Request.io_Device */
			case 20: case 21: case 22: case 23:
				i = 23;
				break;

			/* ioa_Data */
			case 34: case 35: case 36: case 37:
				i = 37;
				break;

                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: audio.device: struct IOAudio: Read is not allowed. Offset=%u\n", (unsigned int)offset);
                                return 1;
                }
        }

        for(i=offset;i<(size+offset);i++) {
                switch(i) {

			/* ioa_Request.io_Device */
                        case 20:
                                *data++ = (dev_audio_base.entry->startAddr >> 24) & 0xff;
                                break;
                        case 21:
                                *data++ = (dev_audio_base.entry->startAddr >> 16) & 0xff;
                                break;
                        case 22:
                                *data++ = (dev_audio_base.entry->startAddr >> 8) & 0xff;
                                break;
                        case 23:
                                *data++ = (dev_audio_base.entry->startAddr) & 0xff;
                                break;

			/* ioa_Data */
			case 34:
				*data++ = ioaudio_ext->data_wtmp[0];
				break;
			case 35:
				*data++ = ioaudio_ext->data_wtmp[1];
				break;
			case 36:
				*data++ = ioaudio_ext->data_wtmp[2];
				break;
			case 37:
				*data++ = ioaudio_ext->data_wtmp[3];
				break;

                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: audio.device: struct IOAudio: Read is not allowed. Offset=%u\n", (unsigned int)offset);
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
	struct IOAudio *real;
	dev_audio_s_ioaudio_ext_t *ioaudio_ext;
	uint32_t vaddr;

        DEBUG(7) dprintf("audio.device: struct IOAudio: map_w() called. VAddr: 0x%x, Offset: 0x%x, Size: 0x%x\n", (unsigned int)(entry->startAddr+offset), (unsigned int)offset, (unsigned int)size);

        /* Get our object */
        obj = (lib_exec_s_iorequest_t *)entry->nativePtr;
	real = (struct IOAudio *)obj->real;
	ioaudio_ext = (dev_audio_s_ioaudio_ext_t *)obj->extended;

        for(i=offset;i<(size+offset);i++) {
                switch(i) {

			/* ioa_Request.io_Command */
			case 28: case 29:
				i = 29;
				break;

			/* ioa_Request.io_Flags */
			case 30:
				break;

			/* ioa_Data */
			case 34:
				ioaudio_ext->data_wtmp[0] = *data++;
				ioaudio_ext->data_wmask |= 1 << 0;
				break;
			case 35:
				ioaudio_ext->data_wtmp[1] = *data++;
				ioaudio_ext->data_wmask |= 1 << 1;
				break;
			case 36:
				ioaudio_ext->data_wtmp[2] = *data++;
				ioaudio_ext->data_wmask |= 1 << 2;
				break;
			case 37:
				ioaudio_ext->data_wtmp[3] = *data++;
				ioaudio_ext->data_wmask |= 1 << 3;
				break;

			/* ioa_Length */
			case 38: case 39: case 40: case 41:
				i = 41;
				break;

			/* ioa_Period */
			case 42: case 43:
				i = 43;
				break;

			/* ioa_Volume */
			case 44: case 45:
				i = 45;
				break;

			/* ioa_Cycles */
			case 46: case 47:
				i = 47;
				break;

                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: audio.device: struct IOAudio: Write is not allowed. Offset=%u\n", (unsigned int)offset);
                                return 1;
                }
        }

        for(i=offset;i<(size+offset);i++) {
                switch(i) {

			/* ioa_Request.io_Command */
                        case 28:
				real->ioa_Request.io_Command &= 0x00ff;
				real->ioa_Request.io_Command |= (*data++ << 8);
                                break;
                        case 29:
				real->ioa_Request.io_Command &= 0xff00;
				real->ioa_Request.io_Command |= (*data++);
                                break;

			/* ioa_Request.io_Flags */
                        case 30:
				real->ioa_Request.io_Flags = *data++;
                                break;

			/* ioa_Data */
			case 34: case 35: case 36: case 37:
				if(ioaudio_ext->data_wmask == 0x0f) {
					vaddr = ((uint32_t)ioaudio_ext->data_wtmp[0] << 24) |
						((uint32_t)ioaudio_ext->data_wtmp[1] << 16) |
						((uint32_t)ioaudio_ext->data_wtmp[2] << 8) |
						((uint32_t)ioaudio_ext->data_wtmp[3]);

					real->ioa_Data = vptr(vaddr);
					ioaudio_ext->data_wmask = 0;
				}
				i = 37;
				break;

			/* ioa_Length */
                        case 38:
				real->ioa_Length &= 0x00ffffff;
				real->ioa_Length |= (*data++ << 24);
                                break;
                        case 39:
				real->ioa_Length &= 0xff00ffff;
				real->ioa_Length |= (*data++ << 16);
                                break;
                        case 40:
				real->ioa_Length &= 0xffff00ff;
				real->ioa_Length |= (*data++ << 8);
                                break;
                        case 41:
				real->ioa_Length &= 0xffffff00;
				real->ioa_Length |= (*data++);
                                break;

			/* ioa_Period */
                        case 42:
				real->ioa_Period &= 0x00ff;
				real->ioa_Period |= (*data++ << 8);
                                break;
                        case 43:
				real->ioa_Period &= 0xff00;
				real->ioa_Period |= (*data++);
                                break;

			/* ioa_Volume */
                        case 44:
				real->ioa_Volume &= 0x00ff;
				real->ioa_Volume |= (*data++ << 8);
                                break;
                        case 45:
				real->ioa_Volume &= 0xff00;
				real->ioa_Volume |= (*data++);
                                break;

			/* ioa_Cycles */
                        case 46:
				real->ioa_Cycles &= 0x00ff;
				real->ioa_Cycles |= (*data++ << 8);
                                break;
                        case 47:
				real->ioa_Cycles &= 0xff00;
				real->ioa_Cycles |= (*data++);
                                break;

                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: audio.device: struct IOAudio: Write is not allowed. Offset=%u\n", (unsigned int)offset);
                                return 1;
                }
        }
        return 0;
}

/* Convert IORequest structure into a IOAudio structure */
int dev_audio_s_ioaudio_convert(lib_exec_s_iorequest_t *iorequest)
{
	mmu_entry_t *entry;
	char symbol[200];
	uint8_t req[DEV_AUDIO_S_IOAUDIO_SIZE-LIB_EXEC_S_IOREQUEST_SIZE];
	uint8_t *src;
	struct IOAudio *real;
	uint32_t u32;
	uint16_t u16;
	dev_audio_s_ioaudio_ext_t *ioaudio_ext;

        DEBUG(5) dprintf("audio.device: dev_audio_s_ioaudio_convert() called\n");

	if(iorequest->type == MMU_TYPE_AUDIO_S_IOAUDIO) {
		/* Already correct type */
		return 0;
	}

	if(iorequest->type != MMU_TYPE_EXEC_S_IOREQUEST) {
		/* Set to another type.. trouble */
		return 1;
	}

	/* We have an undefined type, convert it */

	iorequest->type = MMU_TYPE_AUDIO_S_IOAUDIO;

	entry = iorequest->entry;

	/* Override handlers */
	entry->read = map_r;
	entry->write = map_w;
	entry->destructor = destructor;

	/* Allocate extension structure */
	ioaudio_ext = (dev_audio_s_ioaudio_ext_t *)AllocVec(sizeof(dev_audio_s_ioaudio_ext_t),MEMF_PUBLIC|MEMF_CLEAR);
	if(ioaudio_ext == NULL) {
	        DEBUG(5) dprintf("audio.device: dev_audio_s_ioaudio_convert: Could not allocate memory for extended IOAudio structure\n");
		return 1;
	}
	iorequest->extended = ioaudio_ext;

	/* Copy the extension data */
	src = (uint8_t *) iorequest->real;
	src += sizeof(struct IORequest);
	memcpy(req,src,DEV_AUDIO_S_IOAUDIO_SIZE-LIB_EXEC_S_IOREQUEST_SIZE);

	/* Clear out the extension data from the native object */
	memset(src,0,DEV_AUDIO_S_IOAUDIO_SIZE-LIB_EXEC_S_IOREQUEST_SIZE);

	/* Use the native object */
	real = (struct IOAudio *)iorequest->real;

	/* Parse and initialize extension data */

	/* ioa_Data */
	u32 = (uint32_t)req[2] << 24;
	u32 |= (uint32_t)req[3] << 16;
	u32 |= (uint32_t)req[4] << 8;
	u32 |= (uint32_t)req[5];
	/* TODO: Better checks here */
	real->ioa_Data = vptr(u32);

	/* ioa_Length */
	u32 = (uint32_t)req[6] << 24;
	u32 |= (uint32_t)req[7] << 16;
	u32 |= (uint32_t)req[8] << 8;
	u32 |= (uint32_t)req[9];
	real->ioa_Length = (ULONG)u32;

	/* ioa_Period */
	u16 = (uint16_t)req[10] << 8;
	u16 |= (uint16_t)req[11];
	real->ioa_Period = (UWORD)u16;

	/* ioa_Volume */
	u16 = (uint16_t)req[12] << 8;
	u16 |= (uint16_t)req[13];
	real->ioa_Volume = (UWORD)u16;

	/* ioa_Cycles */
	u16 = (uint16_t)req[14] << 8;
	u16 |= (uint16_t)req[15];
	real->ioa_Cycles = (UWORD)u16;

        DEBUG(3) {
		/* Remove old symbols */
//                mmu_removeSymbols(entry);

                /* Add symbols */
                sprintf(symbol, "!%08x.audio.s.IOAudio", entry->startAddr);
                mmu_addSymbol(entry, entry->startAddr, symbol);
                sprintf(symbol, "!%08x.audio.s.IOAudio.ioa_Request.io_Device", entry->startAddr);
                mmu_addSymbol(entry, entry->startAddr + 20, symbol);
                sprintf(symbol, "!%08x.audio.s.IOAudio.ioa_Request.io_Command", entry->startAddr);
                mmu_addSymbol(entry, entry->startAddr + 28, symbol);
                sprintf(symbol, "!%08x.audio.s.IOAudio.ioa_Request.io_Flags", entry->startAddr);
                mmu_addSymbol(entry, entry->startAddr + 30, symbol);
                sprintf(symbol, "!%08x.audio.s.IOAudio.ioa_Data", entry->startAddr);
                mmu_addSymbol(entry, entry->startAddr + 34, symbol);
                sprintf(symbol, "!%08x.audio.s.IOAudio.ioa_Length", entry->startAddr);
                mmu_addSymbol(entry, entry->startAddr + 38, symbol);
                sprintf(symbol, "!%08x.audio.s.IOAudio.ioa_Period", entry->startAddr);
                mmu_addSymbol(entry, entry->startAddr + 42, symbol);
                sprintf(symbol, "!%08x.audio.s.IOAudio.ioa_Volume", entry->startAddr);
                mmu_addSymbol(entry, entry->startAddr + 44, symbol);
                sprintf(symbol, "!%08x.audio.s.IOAudio.ioa_Cycles", entry->startAddr);
                mmu_addSymbol(entry, entry->startAddr + 46, symbol);
                sprintf(symbol, "!%08x.audio.s.IOAudio.ioa_WriteMsg", entry->startAddr);
                mmu_addSymbol(entry, entry->startAddr + 48, symbol);
        }

	return 0;
}

/* Destructor of mapping */
static void destructor(mmu_entry_t *entry)
{
	lib_exec_s_iorequest_t *iorequest;
	dev_audio_s_ioaudio_ext_t *ioaudio_ext;

        DEBUG(5) dprintf("audio.device: dev_audio_s_ioaudio: destructor() called\n");

	iorequest = (lib_exec_s_iorequest_t *)entry->nativePtr;

	ioaudio_ext = (dev_audio_s_ioaudio_ext_t *)iorequest->extended;

	if(ioaudio_ext != NULL)
		FreeVec(ioaudio_ext);

	mmu_freeObj(entry);
}
