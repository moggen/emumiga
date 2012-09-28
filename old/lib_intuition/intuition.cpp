/*
lib_intuition/intuition.c - Simulation of AmigaOS intuition.library

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
#include "intuition.h"
#include "functions.h"
#include "s_screen.h"
#include "s_drawinfo.h"
#include "s_window.h"
#include "s_menu.h"
#include "s_menuitem.h"
#include "s_gadget.h"
#include "s_requester.h"

/* Intuition base object */
lib_intuition_t lib_intuition_base;

/* Hook module number */
static uint16_t mnum;

/* Init flag */
static int is_init = 0;

/* Predecls */
static int hook1(emumsg_syscall_t *msg);
static int hook2(emumsg_syscall_t *msg);
static int hook3(emumsg_syscall_t *msg);

/* Global init */
int lib_intuition_init()
{
	uint32_t vaddr;
	uint32_t cur_emu, cur_vec;
	uint16_t cur_fnum;
	uint8_t *addr;
	mmu_entry_t *entry;

	DEBUG(3) dprintf("intuition.library: lib_intuition_init() called\n");

	/* Get a hook module number */
	mnum = allocate_hook("intuition.library", hook1, hook2, hook3);

	/* Clear memory */
        memset((void *)&lib_intuition_base,0,sizeof(lib_intuition_t));

	/* Allocate virtual space for
	    - Emulator instruction + rts = 8 bytes * VECTORS
	    - Standard jmp table = 6 bytes * VECTORS
	    - Library struct
	*/
	vaddr = vallocmem_nomap((6+8)*LIB_INTUITION_VECTORS+LIB_INTUITION_SIZE);

	/* Allocate real memory for
	    - Emulator instruction + rts = 8 bytes * VECTORS
	    - Standard jmp table = 6 bytes * VECTORS
	*/
	addr = (uint8_t *)AllocVec((6+8)*LIB_INTUITION_VECTORS, MEMF_PUBLIC|MEMF_CLEAR);

	/* Set up binary mapping */
	entry = lib_intuition_base.vec_entry = mmu_addEntry(
                vaddr,
                (6+8)*LIB_INTUITION_VECTORS,
		MMU_TYPE_INTUITION_VECTORS,
		NULL
	);
	entry->read = mmu_fn_r_binary;
	entry->write = mmu_fn_w_binary;
	entry->nativePtr = addr;

	DEBUG(3) {
	        /* Add a symbol for it */
	        mmu_addSymbol(lib_intuition_base.vec_entry, vaddr, "!intuition.vec");
	}

	/* Loop over all vectors, cur_fnum counts down */
	cur_fnum = LIB_INTUITION_VECTORS;
	cur_emu = vaddr;
	cur_vec = cur_emu + 8*LIB_INTUITION_VECTORS;
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

	/* Write-protect the intuition base */
	entry->write = mmu_fn_protected;

	/* Calculate library struct address */
	vaddr += (6+8)*LIB_INTUITION_VECTORS;

	DEBUG(3) {
	        /* Add implemented LVO symbols */
	        mmu_addSymbol(lib_intuition_base.vec_entry, vaddr-54, "!intuition.vec.ClearMenuStrip");
	        mmu_addSymbol(lib_intuition_base.vec_entry, vaddr-72, "!intuition.vec.CloseWindow");
	        mmu_addSymbol(lib_intuition_base.vec_entry, vaddr-144, "!intuition.vec.ItemAddress");
	        mmu_addSymbol(lib_intuition_base.vec_entry, vaddr-264, "!intuition.vec.SetMenuStrip");
	        mmu_addSymbol(lib_intuition_base.vec_entry, vaddr-276, "!intuition.vec.SetWindowTitles");
	        mmu_addSymbol(lib_intuition_base.vec_entry, vaddr-354, "!intuition.vec.BeginRefresh");
	        mmu_addSymbol(lib_intuition_base.vec_entry, vaddr-366, "!intuition.vec.EndRefresh");
	        mmu_addSymbol(lib_intuition_base.vec_entry, vaddr-432, "!intuition.vec.RefreshGList");
	        mmu_addSymbol(lib_intuition_base.vec_entry, vaddr-438, "!intuition.vec.AddGList");
	        mmu_addSymbol(lib_intuition_base.vec_entry, vaddr-510, "!intuition.vec.LockPubScreen");
	        mmu_addSymbol(lib_intuition_base.vec_entry, vaddr-516, "!intuition.vec.UnlockPubScreen");
	        mmu_addSymbol(lib_intuition_base.vec_entry, vaddr-606, "!intuition.vec.OpenWindowTagList");
	        mmu_addSymbol(lib_intuition_base.vec_entry, vaddr-690, "!intuition.vec.GetScreenDrawInfo");
	        mmu_addSymbol(lib_intuition_base.vec_entry, vaddr-696, "!intuition.vec.FreeScreenDrawInfo");
	        mmu_addSymbol(lib_intuition_base.vec_entry, vaddr-702, "!intuition.vec.ResetMenuStrip");
	}

	/* Set up the special memory mapping */
        entry = lib_intuition_base.entry = mmu_addEntry(
                vaddr,
                LIB_INTUITION_SIZE,
		MMU_TYPE_INTUITION_INTUITIONBASE,
		NULL
	);
	entry->read = mmu_fn_protected;
	entry->write = mmu_fn_protected;
	entry->nativePtr = (void *)&lib_intuition_base;

	DEBUG(3) {
	        /* Add symbols */
	        mmu_addSymbol(lib_intuition_base.entry, vaddr, "!IntuitionBase");
	}

	is_init = 1;

	/* Init subsystems */
        if( lib_intuition_s_screen_init() ) { lib_intuition_cleanup(); return 1; }
        if( lib_intuition_s_drawinfo_init() ) { lib_intuition_cleanup(); return 1; }
        if( lib_intuition_s_window_init() ) { lib_intuition_cleanup(); return 1; }
        if( lib_intuition_s_menu_init() ) { lib_intuition_cleanup(); return 1; }
        if( lib_intuition_s_menuitem_init() ) { lib_intuition_cleanup(); return 1; }
        if( lib_intuition_s_gadget_init() ) { lib_intuition_cleanup(); return 1; }
        if( lib_intuition_s_requester_init() ) { lib_intuition_cleanup(); return 1; }

	return 0;
}

/* Global cleanup */
void lib_intuition_cleanup()
{
	uint32_t vaddr;
	uint8_t	*addr;
	lib_intuition_t *obj;

	DEBUG(3) dprintf("intuition.library: lib_intuition_cleanup() called\n");

	/* Cleanup subsystems */
        lib_intuition_s_requester_cleanup();
        lib_intuition_s_gadget_cleanup();
        lib_intuition_s_menuitem_cleanup();
        lib_intuition_s_menu_cleanup();
        lib_intuition_s_window_cleanup();
        lib_intuition_s_drawinfo_cleanup();
        lib_intuition_s_screen_cleanup();

        if( !is_init ) {
                DEBUG(2) dprintf("Warning: intuition.library: lib_intuition_cleanup: Not initialized.\n");
                return;
        }

	obj = &lib_intuition_base;

	/* Delete mmu entry, IntuitionLibrary structure */
	mmu_delEntry(obj->entry);

	vaddr = obj->vec_entry->startAddr;
	addr = (uint8_t *)obj->vec_entry->nativePtr;

	/* Delete mmu entry, vectors */
	mmu_delEntry(obj->vec_entry);

	/* Free real memory, vectors */
	FreeVec(addr);

	/* Delete vmemory, vectors + IntuitionLibrary structure */
	vfreemem_nomap(vaddr);

	/* Free hook module number */
	free_hook(mnum);

	is_init = 0;
}

static int hook1(emumsg_syscall_t *msg)
{
	switch(msg->func){

		case 9:
			return lib_intuition_f_ClearMenuStrip(msg);
		case 12:
			return lib_intuition_f_CloseWindow(msg);
		case 24:
			return lib_intuition_f_ItemAddress(msg);
		case 44:
			return lib_intuition_f_SetMenuStrip(msg);
		case 46:
			return lib_intuition_f_SetWindowTitles(msg);
		case 59:
			return lib_intuition_f_BeginRefresh(msg);
		case 61:
			return lib_intuition_f_EndRefresh(msg);
		case 72:
			return lib_intuition_f_RefreshGList(msg);
		case 73:
			return lib_intuition_f_AddGList(msg);
		case 85:
			return lib_intuition_f_LockPubScreen(msg);
		case 86:
			return lib_intuition_f_UnlockPubScreen(msg);
		case 101:
			return lib_intuition_f_OpenWindowTagList(msg);
		case 115:
			return lib_intuition_f_GetScreenDrawInfo(msg);
		case 116:
			return lib_intuition_f_FreeScreenDrawInfo(msg);
		case 117:
			return lib_intuition_f_ResetMenuStrip(msg);

	}
	dprintf("Error: intuition.library: hook1: Unimplemented function %u\n", msg->func);
	return HOOK_END_PROC;
}

static int hook2(emumsg_syscall_t *msg)
{
	switch(msg->func){

		case 9:
			return lib_intuition_f_ClearMenuStrip_2(msg);
		case 12:
			return lib_intuition_f_CloseWindow_2(msg);
		case 24:
			return lib_intuition_f_ItemAddress_2(msg);
		case 44:
			return lib_intuition_f_SetMenuStrip_2(msg);
		case 46:
			return lib_intuition_f_SetWindowTitles_2(msg);
		case 59:
			return lib_intuition_f_BeginRefresh_2(msg);
		case 61:
			return lib_intuition_f_EndRefresh_2(msg);
		case 72:
			return lib_intuition_f_RefreshGList_2(msg);
		case 73:
			return lib_intuition_f_AddGList_2(msg);
		case 85:
			return lib_intuition_f_LockPubScreen_2(msg);
		case 86:
			return lib_intuition_f_UnlockPubScreen_2(msg);
		case 101:
			return lib_intuition_f_OpenWindowTagList_2(msg);
		case 115:
			return lib_intuition_f_GetScreenDrawInfo_2(msg);
		case 116:
			return lib_intuition_f_FreeScreenDrawInfo_2(msg);
		case 117:
			return lib_intuition_f_ResetMenuStrip_2(msg);

	}
	dprintf("Error: intuition.library: hook2: Unimplemented function %u\n", msg->func);
	return HOOK_END_PROC;
}

static int hook3(emumsg_syscall_t *msg)
{
	switch(msg->func){

		case 9:
			return lib_intuition_f_ClearMenuStrip_3(msg);
		case 12:
			return lib_intuition_f_CloseWindow_3(msg);
		case 24:
			return lib_intuition_f_ItemAddress_3(msg);
		case 44:
			return lib_intuition_f_SetMenuStrip_3(msg);
		case 46:
			return lib_intuition_f_SetWindowTitles_3(msg);
		case 59:
			return lib_intuition_f_BeginRefresh_3(msg);
		case 61:
			return lib_intuition_f_EndRefresh_3(msg);
		case 72:
			return lib_intuition_f_RefreshGList_3(msg);
		case 73:
			return lib_intuition_f_AddGList_3(msg);
		case 85:
			return lib_intuition_f_LockPubScreen_3(msg);
		case 86:
			return lib_intuition_f_UnlockPubScreen_3(msg);
		case 101:
			return lib_intuition_f_OpenWindowTagList_3(msg);
		case 115:
			return lib_intuition_f_GetScreenDrawInfo_3(msg);
		case 116:
			return lib_intuition_f_FreeScreenDrawInfo_3(msg);
		case 117:
			return lib_intuition_f_ResetMenuStrip_3(msg);

	}
	dprintf("Error: intuition.library: hook3: Unimplemented function %u\n", msg->func);
	return HOOK_END_PROC;
}
