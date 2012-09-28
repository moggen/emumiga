/*
dev_audio/audio.c - Simulation of AmigaOS audio.device

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

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <proto/exec.h>
#include <exec/devices.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../hook.h"
#include "../emulator.h"
#include "audio.h"
#include "functions.h"

/* Global audio base, set by Exec OpenDevice() */
struct Device *AudioBase;

/* Audio base object */
dev_audio_t dev_audio_base;

/* Hook module number */
static uint16_t mnum;

/* Init flag */
static int is_init = 0;

/* Predecls */
static int hook1(emumsg_syscall_t *msg);
static int hook2(emumsg_syscall_t *msg);
static int hook3(emumsg_syscall_t *msg);

/* Global init */
int dev_audio_init()
{
	uint32_t vaddr;
	uint32_t cur_emu, cur_vec;
	uint16_t cur_fnum;
	uint8_t *addr;

	DEBUG(3) dprintf("audio.device: dev_audio_init() called\n");

	/* Get a hook module number */
	mnum = allocate_hook("audio.device", hook1, hook2, hook3);

        /* Clear memory */
	memset((void *)&dev_audio_base,0,sizeof(dev_audio_t));

	/* Allocate virtual space for
	    - Emulator instruction + rts = 8 bytes * VECTORS
	    - Standard jmp table = 6 bytes * VECTORS
	    - Audio device struct
	*/
	vaddr = vallocmem_nomap((6+8)*DEV_AUDIO_VECTORS+DEV_AUDIO_SIZE);

        /* Allocate real memory for
            - Emulator instruction + rts = 8 bytes * VECTORS
            - Standard jmp table = 6 bytes * VECTORS
        */
	addr = (uint8_t *)AllocVec((6+8)*DEV_AUDIO_VECTORS, MEMF_PUBLIC|MEMF_CLEAR);

        /* Set up binary mapping */
	dev_audio_base.vec_entry = mmu_addEntry(
                vaddr,
                (6+8)*DEV_AUDIO_VECTORS,
		MMU_TYPE_AUDIO_VECTORS,
		NULL
	);
	dev_audio_base.vec_entry->read = mmu_fn_r_binary;
	dev_audio_base.vec_entry->write = mmu_fn_w_binary;
	dev_audio_base.vec_entry->nativePtr = addr;

	DEBUG(3) {
		/* Add a symbol for it */
		mmu_addSymbol(dev_audio_base.vec_entry, vaddr, "!audio.vec");
	}

	/* Loop over all vectors, cur_fnum counts down */
	cur_fnum = DEV_AUDIO_VECTORS;
	cur_emu = vaddr;
	cur_vec = cur_emu + 8*DEV_AUDIO_VECTORS;
	while(cur_fnum > 0)
	{
		/* Write emulator instruction + rts */
		WRITEMEM_16(0xFD80,cur_emu);
		WRITEMEM_16(mnum,cur_emu+2);
		WRITEMEM_16(cur_fnum,cur_emu+4);
		WRITEMEM_16(0x4E75,cur_emu+6);

		/* Write jump vector */
		WRITEMEM_16(0x4EF9,cur_vec);
		WRITEMEM_32(cur_emu,cur_vec+2);
		cur_emu += 8;
		cur_vec += 6;
		cur_fnum--;
	}

	/* Write protect the audio base */
	dev_audio_base.vec_entry->write = mmu_fn_protected;

        /* Calculate device struct address */
        vaddr += (6+8)*DEV_AUDIO_VECTORS;

	DEBUG(3) {
		/* Add implemented LVO symbols */
		mmu_addSymbol(dev_audio_base.vec_entry, vaddr-30, "!audio.vec.BeginIO");
	}

        /* Set up the special memory mapping */
        dev_audio_base.entry = mmu_addEntry(
                vaddr,
                DEV_AUDIO_SIZE,
		MMU_TYPE_AUDIO_AUDIOBASE,
		NULL
	);

	dev_audio_base.entry->read = mmu_fn_protected;
	dev_audio_base.entry->write = mmu_fn_protected;
	dev_audio_base.entry->nativePtr = (void *)&dev_audio_base;

	DEBUG(3) {
		/* Add symbols */
		mmu_addSymbol(dev_audio_base.entry, vaddr, "!AudioBase");
	}

	is_init = 1;

	return 0;
}

/* Global cleanup */
void dev_audio_cleanup()
{
	uint32_t vaddr;
	uint8_t *addr;
	dev_audio_t *obj;

	DEBUG(3) dprintf("audio.device: dev_audio_cleanup() called\n");

        if( !is_init ) {
                DEBUG(2) dprintf("audio.device: dev_audio_cleanup: Not initialized\n");
                return;
        }

	obj = &dev_audio_base;

	/* Delete mmu entry, AudioDevice structure */
        mmu_delEntry(obj->entry);

        vaddr = obj->vec_entry->startAddr;
        addr = (uint8_t *)obj->vec_entry->nativePtr;

	/* Delete mmu entry, vectors */
        mmu_delEntry(obj->vec_entry);

	/* Free real memory, vectors */
        FreeVec(addr);

	/* Delete vmemory, vectors + AudioDevice structure */
        vfreemem_nomap(vaddr);

	/* Free hook module number */
	free_hook(mnum);

	is_init = 0;
}

static int hook1(emumsg_syscall_t *msg)
{
	switch(msg->func){
		case 5:
			return dev_audio_f_BeginIO(msg);
	}
	dprintf("Error: audio.device: hook1: Unimplemented function %u\n", msg->func);
	return HOOK_END_PROC;
}

static int hook2(emumsg_syscall_t *msg)
{
	switch(msg->func){
		case 5:
			return dev_audio_f_BeginIO_2(msg);
	}
	dprintf("Error: audio.device: hook2: Unimplemented function %u\n", msg->func);
	return HOOK_END_PROC;
}

static int hook3(emumsg_syscall_t *msg)
{
	switch(msg->func){
		case 5:
			return dev_audio_f_BeginIO_3(msg);
	}
	dprintf("Error: audio.device: hook3: Unimplemented function %u\n", msg->func);
	return HOOK_END_PROC;
}
