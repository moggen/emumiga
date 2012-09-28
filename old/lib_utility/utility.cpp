/*
lib_utility/utility.c - Simulation of AmigaOS utility.library

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
#include "../hook.h"
#include "../emulator.h"
#include "utility.h"
#include "functions.h"

#include "s_hook.h"

/* Utility base object */
lib_utility_t lib_utility_base;

/* Hook module number */
static uint16_t mnum;

/* Init flag */
static int is_init = 0;

/* Predecls */
static int hook1(emumsg_syscall_t *msg);
static int hook2(emumsg_syscall_t *msg);
static int hook3(emumsg_syscall_t *msg);

/* Global init */
int lib_utility_init()
{
	uint32_t vaddr;
	uint32_t cur_emu, cur_vec;
	uint16_t cur_fnum;
	uint8_t *addr;
	mmu_entry_t *entry;

	DEBUG(3) dprintf("utility.library: lib_utility_init() called\n");

	/* Get a hook module number */
	mnum = allocate_hook("utility.library", hook1, hook2, hook3);

	/* Clear memory */
        memset((void *)&lib_utility_base,0,sizeof(lib_utility_t));

	/* Allocate virtual space for
	    - Emulator instruction + rts = 8 bytes * VECTORS
	    - Standard jmp table = 6 bytes * VECTORS
	    - Library struct
	*/
	vaddr = vallocmem_nomap((6+8)*LIB_UTILITY_VECTORS+LIB_UTILITY_SIZE);

	/* Allocate real memory for
	    - Emulator instruction + rts = 8 bytes * VECTORS
	    - Standard jmp table = 6 bytes * VECTORS
	*/
	addr = (uint8_t *)AllocVec((6+8)*LIB_UTILITY_VECTORS, MEMF_PUBLIC|MEMF_CLEAR);

	/* Set up binary mapping */
	entry = lib_utility_base.vec_entry = mmu_addEntry(
                vaddr,
                (6+8)*LIB_UTILITY_VECTORS,
		MMU_TYPE_UTILITY_VECTORS,
		NULL
	);
	entry->read = mmu_fn_r_binary;
	entry->write = mmu_fn_w_binary;
	entry->nativePtr = addr;

	DEBUG(3) {
	        /* Add a symbol for it */
	        mmu_addSymbol(lib_utility_base.vec_entry, vaddr, "!utility.vec");
	}

	/* Loop over all vectors, cur_fnum counts down */
	cur_fnum = LIB_UTILITY_VECTORS;
	cur_emu = vaddr;
	cur_vec = cur_emu + 8*LIB_UTILITY_VECTORS;
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

	/* Write-protect the utility base */
	entry->write = mmu_fn_protected;

	/* Calculate library struct address */
	vaddr += (6+8)*LIB_UTILITY_VECTORS;

	DEBUG(3) {
	        /* Add implemented LVO symbols */
/*
	        mmu_addSymbol(lib_utility_base.vec_entry, vaddr-30, "!utility.vec.");
*/
	}

	/* Set up the special memory mapping */
        entry = lib_utility_base.entry = mmu_addEntry(
                vaddr,
                LIB_UTILITY_SIZE,
		MMU_TYPE_UTILITY_UTILITYBASE,
		NULL
	);
	entry->read = mmu_fn_protected;
	entry->write = mmu_fn_protected;
	entry->nativePtr = (void *)&lib_utility_base;

	DEBUG(3) {
	        /* Add symbols */
	        mmu_addSymbol(lib_utility_base.entry, vaddr, "!UtilityBase");
	}

	is_init = 1;

	/* Init subsystems */
        if( lib_utility_s_hook_init() ) { lib_utility_cleanup(); return 1; }

	return 0;
}

/* Global cleanup */
void lib_utility_cleanup()
{
	uint32_t vaddr;
	uint8_t	*addr;
	lib_utility_t *obj;

	DEBUG(3) dprintf("utility.library: lib_utility_cleanup() called\n");

	/* Cleanup subsystems */
        lib_utility_s_hook_cleanup();

        if( !is_init ) {
                DEBUG(2) dprintf("Warning: utility.library: lib_utility_cleanup: Not initialized.\n");
                return;
        }

	obj = &lib_utility_base;

	/* Delete mmu entry, UtilityLibrary structure */
	mmu_delEntry(obj->entry);

	vaddr = obj->vec_entry->startAddr;
	addr = (uint8_t *)obj->vec_entry->nativePtr;

	/* Delete mmu entry, vectors */
	mmu_delEntry(obj->vec_entry);

	/* Free real memory, vectors */
	FreeVec(addr);

	/* Delete vmemory, vectors + UtilityLibrary structure */
	vfreemem_nomap(vaddr);

	/* Free hook module number */
	free_hook(mnum);

	is_init = 0;
}

static int hook1(emumsg_syscall_t *msg)
{
	switch(msg->func){

/*
		case 5:
			return lib_utility_x(msg);
*/
	}
	dprintf("Error: utility.library: hook1: Unimplemented function %u\n", msg->func);
	return HOOK_END_PROC;
}

static int hook2(emumsg_syscall_t *msg)
{
	switch(msg->func){
/*
		case 5:
			return lib_utility_x_2(msg);
*/
	}
	dprintf("Error: utility.library: hook2: Unimplemented function %u\n", msg->func);
	return HOOK_END_PROC;
}

static int hook3(emumsg_syscall_t *msg)
{
	switch(msg->func){
/*
		case 5:
			return lib_utility_x_3(msg);
*/
	}
	dprintf("Error: utility.library: hook3: Unimplemented function %u\n", msg->func);
	return HOOK_END_PROC;
}
