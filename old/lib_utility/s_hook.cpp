/*
lib_utility/s_hook.c - utility.library struct Hook

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
#include <utility/hooks.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../macros.h"
#include "../emulator.h"

#include "s_hook.h"

/* Object pool */
static mmu_pool_t *pool;

/* Init flag */
static int is_init = 0;

/* Read map function */
static int map_r(mmu_entry_t *entry, uint32_t offset, uint32_t size,
        uint8_t *data )
{
        uint32_t i;
        lib_utility_s_hook_t *obj;

        DEBUG(7) dprintf("utility.library: struct Hook: map_r() called. VAddr: 0x%x, Size: 0x%x\n", (unsigned int)(entry->startAddr+offset), (unsigned int)size);

        /* Get our object */
        obj = (lib_utility_s_hook_t *)entry->nativePtr;

        for(i=offset;i<(size+offset);i++) {
                switch(i) {
                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: utility.library: struct Hook: Read is not allowed. Offset=%u\n", (unsigned int)offset);
                                return 1;
                }
        }

        for(i=offset;i<(size+offset);i++) {
                switch(i) {
                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: utility.library: struct Hook: Read is not allowed. Offset=%u\n", (unsigned int)offset);
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
        lib_utility_s_hook_t *obj;

        DEBUG(7) dprintf("utility.library: struct Hook: map_w() called. VAddr: 0x%x, Size: 0x%x\n", (unsigned int)(entry->startAddr+offset), (unsigned int)size);

        /* Get our object */
        obj = (lib_utility_s_hook_t *)entry->nativePtr;

        for(i=offset;i<(size+offset);i++) {
                switch(i) {
			/* h_Entry */
			case 8: case 9: case 10: case 11:
				i = 11;
				break;

                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: utility.library: struct Hook: Write is not allowed. Offset=%u\n", (unsigned int)offset);
                                return 1;
                }
        }

        for(i=offset;i<(size+offset);i++) {
                switch(i) {
			/* h_Entry */
			/* Write to internal variables,
			   not to the real object */
			case 8:
				obj->entry_vaddr &= 0x00ffffff;
				obj->entry_vaddr |= ((uint32_t)*data++ << 24);
				break;
			case 9:
				obj->entry_vaddr &= 0xff00ffff;
				obj->entry_vaddr |= ((uint32_t)*data++ << 16);
				break;
			case 10:
				obj->entry_vaddr &= 0xffff00ff;
				obj->entry_vaddr |= ((uint32_t)*data++ << 8);
				break;
			case 11:
				obj->entry_vaddr &= 0xffffff00;
				obj->entry_vaddr |= ((uint32_t)*data++);
				break;

                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: utility.library: struct Hook: Write is not allowed. Offset=%u\n", (unsigned int)offset);
                                return 1;
                }
        }

        return 0;
}

/* Global init */
int lib_utility_s_hook_init()
{
	DEBUG(4) dprintf("utility.library: lib_utility_s_hook_init() called\n");

        pool = mmu_pool_create(
                "utility.s.Hook",
                MMU_TYPE_UTILITY_S_HOOK,
                LIB_UTILITY_S_HOOK_SIZE,
                sizeof(lib_utility_s_hook_t)
        );

        if(pool == NULL)
                return 1;

	is_init = 1;
	return 0;
}

/* Global cleanup */
void lib_utility_s_hook_cleanup()
{
	DEBUG(4) dprintf("utility.library: lib_utility_s_hook_cleanup() called\n");

	if( !is_init ) {
		DEBUG(2) dprintf("Warning: utility.library: lib_utility_s_hook_cleanup: Not initialized\n");
		return;
	}

	mmu_pool_destroy(pool);

	is_init = 0;
}

/* Signature as defined in utility/hooks.h (AROS) */
AROS_UFH3(void, lib_utility_s_hook_callback,
        AROS_UFHA(struct Hook * , hook   , A0),
        AROS_UFHA(APTR          , object , A2),
        AROS_UFHA(APTR          , message, A1))
{
        AROS_USERFUNC_INIT

	lib_utility_s_hook_t *obj;
	emumsg_syscall_t *msg;
        cpu_t *cpu;
	int ret;
	struct MsgPort *syscall_port;

	/* Get emulation structures */
	obj = (lib_utility_s_hook_t *)hook->h_Data;
	msg = obj->msg;
	syscall_port = msg->proc->syscall_port;

        cpu = msg->proc->cpu;

	/* The CPU stack pointer is here always set to point out the
	   special TRAP section, so RTS can be used. */

	/* We must set PC to beginning of callback */
	cpu->pc = obj->entry_vaddr;

	/* Pass the arguments to the callback handling */
	msg->arg[0]._aptr = (APTR)hook;
	msg->arg[1]._aptr = (APTR)message;
	msg->arg[2]._aptr = (APTR)object;

        /* Signal back to emulator to continue emulating,
	   it will resume at the start of the callback */
        ReplyMsg((struct Message *)msg);

        /* Run the event loop handling new syscalls etc. */
        ret = emulator_doEvents(syscall_port);

	/* The event loop returned, the callback is finished */

        /* TODO: How should errors in callbacks be handled here? */

        AROS_USERFUNC_EXIT
}

/* Get an instance */
lib_utility_s_hook_t *lib_utility_s_hook_get_real(struct Hook *real)
{
	mmu_entry_t *entry;

	DEBUG(5) dprintf("utility.library: lib_utility_s_hook_get_real() called\n");

        if(real == NULL)
		return NULL;

        /* Check to find if it already exists */
        entry = mmu_findEntryWithTypeReal(real, MMU_TYPE_UTILITY_S_HOOK);
        if(entry != NULL) {

                /* Yes, return object */
                return (lib_utility_s_hook_t *)entry->nativePtr;
        }

	/* This structure should not be created from real space */
	return NULL;
}

/* Get an instance */
lib_utility_s_hook_t *lib_utility_s_hook_get_vaddr(uint32_t vaddr)
{
	mmu_entry_t *entry;
	lib_utility_s_hook_t *obj;
	struct Hook *real;
	char symbol[200];

	uint32_t h_MinNode__mln_Succ;
	uint32_t h_MinNode__mln_Pred;
	uint32_t h_Entry;
	uint32_t h_SubEntry;
	uint32_t h_Data;

	DEBUG(5) dprintf("utility.library: lib_utility_s_hook_get_vaddr() called\n");

	if(vaddr == 0)
		return NULL;

        /* Check to find if it already exists */
        entry = mmu_findEntryWithType(vaddr, MMU_TYPE_UTILITY_S_HOOK);
        if(entry != NULL) {

                /* Yes, return object */
                return (lib_utility_s_hook_t *)entry->nativePtr;
        }

	/* TODO: Check VADDR interval and clear any old mappings in it */

	/* Read parameters */
	if(
	    READMEM_32(vaddr,&h_MinNode__mln_Succ) ||
	    READMEM_32(vaddr+4,&h_MinNode__mln_Pred) ||
	    READMEM_32(vaddr+8,&h_Entry) ||
	    READMEM_32(vaddr+12,&h_SubEntry) ||
	    READMEM_32(vaddr+16,&h_Data)
        ) {
		return NULL;
	}

	/* Is the MinNode struct filled in */
	if(h_MinNode__mln_Succ != 0 || h_MinNode__mln_Pred != 0) {
		DEBUG(2) dprintf("Warning: utility.library: struct Hook: Ignoring MinNode data when creating from user memory.\n");
	}

	/* Check if there is a callback entry */
	if(h_Entry == 0) {
		DEBUG(2) dprintf("Warning: utility.library: struct Hook: Callback address is NULL\n");
		return NULL;
	}

	/* Allocate memory for structure */
	real = (struct Hook *)AllocVec(sizeof(struct Hook), MEMF_PUBLIC|MEMF_CLEAR);
	if(real == NULL) {
		return NULL;
	}

	/* Set up real object */
	real->h_MinNode.mln_Succ = NULL;
	real->h_MinNode.mln_Pred = NULL;
	real->h_Entry = (HOOKFUNC)lib_utility_s_hook_callback;
	real->h_SubEntry = NULL;
	real->h_Data = NULL;

        /* Set up an object */
        entry = mmu_allocObjVAddr(pool, real, vaddr);
        if(entry == NULL) {
		FreeVec(real);
                return NULL;
	}

        /* Extract object */
        obj = (lib_utility_s_hook_t *)entry->nativePtr;

	/* Remember object at callback */
	real->h_Data = (APTR)obj;

        /* Init structure */
	entry->read = map_r;
	entry->write = map_w;
	obj->entry = entry;
        obj->real = real;
	obj->entry_vaddr = h_Entry;
	obj->subentry_vaddr = h_SubEntry;
	obj->data_vaddr = h_Data;

	DEBUG(3) {
		/* Add extra symbols */
		vaddr = entry->startAddr;
		sprintf(symbol, "!%08x.%s.h_MinNode.mln_Succ", vaddr, pool->name);
		mmu_addSymbol(entry, vaddr, symbol);
		sprintf(symbol, "!%08x.%s.h_MinNode.mln_Pred", vaddr, pool->name);
		mmu_addSymbol(entry, vaddr+4, symbol);
		sprintf(symbol, "!%08x.%s.h_Entry", vaddr, pool->name);
		mmu_addSymbol(entry, vaddr+8, symbol);
		sprintf(symbol, "!%08x.%s.h_SubEntry", vaddr, pool->name);
		mmu_addSymbol(entry, vaddr+12, symbol);
		sprintf(symbol, "!%08x.%s.h_Data", vaddr, pool->name);
		mmu_addSymbol(entry, vaddr+16, symbol);
	}

	return obj;
}
