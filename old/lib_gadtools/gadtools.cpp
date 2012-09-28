/*
lib_gadtools/gadtools.c - Simulation of AmigaOS gadtools.library

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
#include "gadtools.h"
#include "functions.h"
#include "s_visualinfo.h"
#include "s_newmenu.h"
#include "s_newgadget.h"

/* Gadtools base object */
lib_gadtools_t lib_gadtools_base;

/* Hook module number */
static uint16_t mnum;

/* Init flag */
static int is_init = 0;

/* Predecls */
static int hook1(emumsg_syscall_t *msg);
static int hook2(emumsg_syscall_t *msg);
static int hook3(emumsg_syscall_t *msg);

/* Global init */
int lib_gadtools_init()
{
	uint32_t vaddr;
	uint32_t cur_emu, cur_vec;
	uint16_t cur_fnum;
	uint8_t *addr;
	mmu_entry_t *entry;

	DEBUG(3) dprintf("gadtools.library: lib_gadtools_init() called\n");

	/* Get a hook module number */
	mnum = allocate_hook("gadtools.library", hook1, hook2, hook3);

	/* Clear memory */
        memset((void *)&lib_gadtools_base,0,sizeof(lib_gadtools_t));

	/* Allocate virtual space for
	    - Emulator instruction + rts = 8 bytes * VECTORS
	    - Standard jmp table = 6 bytes * VECTORS
	    - Library struct
	*/
	vaddr = vallocmem_nomap((6+8)*LIB_GADTOOLS_VECTORS+LIB_GADTOOLS_SIZE);

	/* Allocate real memory for
	    - Emulator instruction + rts = 8 bytes * VECTORS
	    - Standard jmp table = 6 bytes * VECTORS
	*/
	addr = (uint8_t *)AllocVec((6+8)*LIB_GADTOOLS_VECTORS, MEMF_PUBLIC|MEMF_CLEAR);

	/* Set up binary mapping */
	entry = lib_gadtools_base.vec_entry = mmu_addEntry(
                vaddr,
                (6+8)*LIB_GADTOOLS_VECTORS,
		MMU_TYPE_GADTOOLS_VECTORS,
		NULL
	);
	entry->read = mmu_fn_r_binary;
	entry->write = mmu_fn_w_binary;
	entry->nativePtr = addr;

	DEBUG(3) {
	        /* Add a symbol for it */
	        mmu_addSymbol(lib_gadtools_base.vec_entry, vaddr, "!gadtools.vec");
	}

	/* Loop over all vectors, cur_fnum counts down */
	cur_fnum = LIB_GADTOOLS_VECTORS;
	cur_emu = vaddr;
	cur_vec = cur_emu + 8*LIB_GADTOOLS_VECTORS;
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

	/* Write-protect the gadtools base */
	entry->write = mmu_fn_protected;

	/* Calculate library struct address */
	vaddr += (6+8)*LIB_GADTOOLS_VECTORS;

	DEBUG(3) {
	        /* Add implemented LVO symbols */
	        mmu_addSymbol(lib_gadtools_base.vec_entry, vaddr-30, "!gadtools.vec.CreateGadgetA");
	        mmu_addSymbol(lib_gadtools_base.vec_entry, vaddr-36, "!gadtools.vec.FreeGadgets");
	        mmu_addSymbol(lib_gadtools_base.vec_entry, vaddr-48, "!gadtools.vec.CreateMenusA");
	        mmu_addSymbol(lib_gadtools_base.vec_entry, vaddr-54, "!gadtools.vec.FreeMenus");
	        mmu_addSymbol(lib_gadtools_base.vec_entry, vaddr-66, "!gadtools.vec.LayoutMenusA");
	        mmu_addSymbol(lib_gadtools_base.vec_entry, vaddr-72, "!gadtools.vec.GT_GetIMsg");
	        mmu_addSymbol(lib_gadtools_base.vec_entry, vaddr-78, "!gadtools.vec.GT_ReplyIMsg");
	        mmu_addSymbol(lib_gadtools_base.vec_entry, vaddr-84, "!gadtools.vec.GT_RefreshWindow");
	        mmu_addSymbol(lib_gadtools_base.vec_entry, vaddr-114, "!gadtools.vec.CreateContext");
	        mmu_addSymbol(lib_gadtools_base.vec_entry, vaddr-120, "!gadtools.vec.DrawBevelBoxA");
	        mmu_addSymbol(lib_gadtools_base.vec_entry, vaddr-126, "!gadtools.vec.GetVisualInfoA");
	        mmu_addSymbol(lib_gadtools_base.vec_entry, vaddr-132, "!gadtools.vec.FreeVisualInfo");
	}

	/* Set up the special memory mapping */
        entry = lib_gadtools_base.entry = mmu_addEntry(
                vaddr,
                LIB_GADTOOLS_SIZE,
		MMU_TYPE_GADTOOLS_GADTOOLSBASE,
		NULL
	);
	entry->read = mmu_fn_protected;
	entry->write = mmu_fn_protected;
	entry->nativePtr = (void *)&lib_gadtools_base;

	DEBUG(3) {
	        /* Add symbols */
	        mmu_addSymbol(lib_gadtools_base.entry, vaddr, "!GadtoolsBase");
	}

	is_init = 1;

	/* Init subsystems */
        if( lib_gadtools_s_visualinfo_init() ) { lib_gadtools_cleanup(); return 1; }
        if( lib_gadtools_s_newmenu_init() ) { lib_gadtools_cleanup(); return 1; }
        if( lib_gadtools_s_newgadget_init() ) { lib_gadtools_cleanup(); return 1; }

	return 0;
}

/* Global cleanup */
void lib_gadtools_cleanup()
{
	uint32_t vaddr;
	uint8_t	*addr;
	lib_gadtools_t *obj;

	DEBUG(3) dprintf("gadtools.library: lib_gadtools_cleanup() called\n");

	/* Cleanup subsystems */
        lib_gadtools_s_newgadget_cleanup();
        lib_gadtools_s_newmenu_cleanup();
        lib_gadtools_s_visualinfo_cleanup();

        if( !is_init ) {
                DEBUG(2) dprintf("Warning: gadtools.library: lib_gadtools_cleanup: Not initialized.\n");
                return;
        }

	obj = &lib_gadtools_base;

	/* Delete mmu entry, GadtoolsLibrary structure */
	mmu_delEntry(obj->entry);

	vaddr = obj->vec_entry->startAddr;
	addr = (uint8_t *)obj->vec_entry->nativePtr;

	/* Delete mmu entry, vectors */
	mmu_delEntry(obj->vec_entry);

	/* Free real memory, vectors */
	FreeVec(addr);

	/* Delete vmemory, vectors + GadtoolsLibrary structure */
	vfreemem_nomap(vaddr);

	/* Free hook module number */
	free_hook(mnum);

	is_init = 0;
}

static int hook1(emumsg_syscall_t *msg)
{
	switch(msg->func){

		case 5:
			return lib_gadtools_f_CreateGadgetA(msg);
		case 6:
			return lib_gadtools_f_FreeGadgets(msg);
		case 8:
			return lib_gadtools_f_CreateMenusA(msg);
		case 9:
			return lib_gadtools_f_FreeMenus(msg);
		case 11:
			return lib_gadtools_f_LayoutMenusA(msg);
		case 12:
			return lib_gadtools_f_GT_GetIMsg(msg);
		case 13:
			return lib_gadtools_f_GT_ReplyIMsg(msg);
		case 14:
			return lib_gadtools_f_GT_RefreshWindow(msg);
		case 19:
			return lib_gadtools_f_CreateContext(msg);
		case 20:
			return lib_gadtools_f_DrawBevelBoxA(msg);
		case 21:
			return lib_gadtools_f_GetVisualInfoA(msg);
		case 22:
			return lib_gadtools_f_FreeVisualInfo(msg);

	}
	dprintf("Error: gadtools.library: hook1: Unimplemented function %u\n", msg->func);
	return HOOK_END_PROC;
}

static int hook2(emumsg_syscall_t *msg)
{
	switch(msg->func){

		case 5:
			return lib_gadtools_f_CreateGadgetA_2(msg);
		case 6:
			return lib_gadtools_f_FreeGadgets_2(msg);
		case 8:
			return lib_gadtools_f_CreateMenusA_2(msg);
		case 9:
			return lib_gadtools_f_FreeMenus_2(msg);
		case 11:
			return lib_gadtools_f_LayoutMenusA_2(msg);
		case 12:
			return lib_gadtools_f_GT_GetIMsg_2(msg);
		case 13:
			return lib_gadtools_f_GT_ReplyIMsg_2(msg);
		case 14:
			return lib_gadtools_f_GT_RefreshWindow_2(msg);
		case 19:
			return lib_gadtools_f_CreateContext_2(msg);
		case 20:
			return lib_gadtools_f_DrawBevelBoxA_2(msg);
		case 21:
			return lib_gadtools_f_GetVisualInfoA_2(msg);
		case 22:
			return lib_gadtools_f_FreeVisualInfo_2(msg);

	}
	dprintf("Error: gadtools.library: hook2: Unimplemented function %u\n", msg->func);
	return HOOK_END_PROC;
}

static int hook3(emumsg_syscall_t *msg)
{
	switch(msg->func){

		case 5:
			return lib_gadtools_f_CreateGadgetA_3(msg);
		case 6:
			return lib_gadtools_f_FreeGadgets_3(msg);
		case 8:
			return lib_gadtools_f_CreateMenusA_3(msg);
		case 9:
			return lib_gadtools_f_FreeMenus_3(msg);
		case 11:
			return lib_gadtools_f_LayoutMenusA_3(msg);
		case 12:
			return lib_gadtools_f_GT_GetIMsg_3(msg);
		case 13:
			return lib_gadtools_f_GT_ReplyIMsg_3(msg);
		case 14:
			return lib_gadtools_f_GT_RefreshWindow_3(msg);
		case 19:
			return lib_gadtools_f_CreateContext_3(msg);
		case 20:
			return lib_gadtools_f_DrawBevelBoxA_3(msg);
		case 21:
			return lib_gadtools_f_GetVisualInfoA_3(msg);
		case 22:
			return lib_gadtools_f_FreeVisualInfo_3(msg);

	}
	dprintf("Error: gadtools.library: hook3: Unimplemented function %u\n", msg->func);
	return HOOK_END_PROC;
}
