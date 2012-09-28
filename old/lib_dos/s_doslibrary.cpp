/*
lib_dos/s_doslibrary.c - Simulation of AmigaOS dos.library

Copyright (C) 2007, 2008, 2009, 2010  Magnus Öberg

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
#include <exec/libraries.h>
#include <dos/dosextens.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../memory/mapping.h"
#include "../vmem.h"
#include "../hook.h"
#include "../emulator.h"
#include "s_doslibrary.h"
//#include "functions.h"
#include "s_process.h"
/*
#include "s_filehandle.h"
#include "s_commandlineinterface.h"
#include "s_filelock.h"
#include "s_anchorpath.h"
#include "s_achain.h"
#include "s_rdargs.h"
#include "s_datestamp.h"
*/

namespace emumiga {
namespace lib_dos {

/*
using memory;

// Kickstart 2.0
#define LIB_DOS_SIZE 66
#define LIB_DOS_VECTORS 164

// Mappings
static mapping *vectors_entry = 0;
static mapping *base_entry = 0;

// Hook module number
static uint16_t mnum;
*/

// Init flag
static int is_init = 0;

// Predecls
static int hook1(emumsg_syscall_t *msg);
static int hook2(emumsg_syscall_t *msg);
static int hook3(emumsg_syscall_t *msg);

// Global init
int s_doslibrary::init()
{
/*
	uint32_t vaddr;
	uint32_t cur_emu, cur_vec;
	uint16_t cur_fnum;
	uint8_t *addr;
*/
	DEBUG(3) dprintf("dos.library: init() called\n");
/*
	// Get a hook module number
	mnum = allocate_hook("dos.library", hook1, hook2, hook3);

	// Allocate virtual space for
	//  - Emulator instruction + rts = 8 bytes * VECTORS
	//  - Standard jmp table = 6 bytes * VECTORS
	//  - Library struct
	vaddr = vallocmem_nomap((6+8)*LIB_DOS_VECTORS+LIB_DOS_SIZE);

	// Allocate real memory for
	//  - Emulator instruction + rts = 8 bytes * VECTORS
	//  - Standard jmp table = 6 bytes * VECTORS
	addr = (uint8_t *)AllocVec((6+8)*LIB_DOS_VECTORS, MEMF_PUBLIC|MEMF_CLEAR);

	// Set up binary mapping
	vetors_entry = mmu_addEntry(
                vaddr,
                (6+8)*LIB_DOS_VECTORS,
		MMU_TYPE_DOS_VECTORS,
		NULL
	);
	vectors_entry->read = mmu_fn_r_binary;
	vectors_entry->write = mmu_fn_w_binary;
	vectors_entry->nativePtr = addr;

	DEBUG(3) {
	        // Add a symbol for it
	        mmu_addSymbol(vectors_entry, vaddr, "!dos.vec");
	}

	// Loop over all vectors, cur_fnum counts down
	cur_fnum = LIB_DOS_VECTORS;
	cur_emu = vaddr;
	cur_vec = cur_emu + 8*LIB_DOS_VECTORS;
	while(cur_fnum > 0)
	{
		// Write emulator instruction + rts
		WRITEMEM_16(0xFD80,cur_emu);
		WRITEMEM_16(mnum,cur_emu+2);
		WRITEMEM_16(cur_fnum,cur_emu+4);
		WRITEMEM_16(0x4E75,cur_emu+6);

		// Write jump vector
		WRITEMEM_16(0x4EF9,cur_vec);
		WRITEMEM_32(cur_emu,cur_vec+2);

		cur_emu += 8;
		cur_vec += 6;
		cur_fnum--;
	}

	// Write-protect the dos base
	base_entry->write = mmu_fn_protected;

	// Calculate library struct address
	vaddr += (6+8)*LIB_DOS_VECTORS;

	DEBUG(3) {
	        // Add implemented LVO symbols
	        mmu_addSymbol(vectors_entry, vaddr-30, "!dos.vec.Open");
	        mmu_addSymbol(vectors_entry, vaddr-36, "!dos.vec.Close");
	        mmu_addSymbol(vectors_entry, vaddr-42, "!dos.vec.Read");
	        mmu_addSymbol(vectors_entry, vaddr-48, "!dos.vec.Write");
	        mmu_addSymbol(vectors_entry, vaddr-54, "!dos.vec.Input");
	        mmu_addSymbol(vectors_entry, vaddr-60, "!dos.vec.Output");
	        mmu_addSymbol(vectors_entry, vaddr-90, "!dos.vec.UnLock");
	        mmu_addSymbol(vectors_entry, vaddr-96, "!dos.vec.DupLock");
	        mmu_addSymbol(vectors_entry, vaddr-138, "!dos.vec.CreateProc");
	        mmu_addSymbol(vectors_entry, vaddr-150, "!dos.vec.LoadSeg");
	        mmu_addSymbol(vectors_entry, vaddr-156, "!dos.vec.UnLoadSeg");
	        mmu_addSymbol(vectors_entry, vaddr-198, "!dos.vec.Delay");
	        mmu_addSymbol(vectors_entry, vaddr-792, "!dos.vec.CheckSignal");
	        mmu_addSymbol(vectors_entry, vaddr-798, "!dos.vec.ReadArgs");
	        mmu_addSymbol(vectors_entry, vaddr-822, "!dos.vec.MatchFirst");
	        mmu_addSymbol(vectors_entry, vaddr-828, "!dos.vec.MatchNext");
	        mmu_addSymbol(vectors_entry, vaddr-834, "!dos.vec.MatchEnd");
	        mmu_addSymbol(vectors_entry, vaddr-858, "!dos.vec.FreeArgs");
	        mmu_addSymbol(vectors_entry, vaddr-948, "!dos.vec.PutStr");
	        mmu_addSymbol(vectors_entry, vaddr-954, "!dos.vec.VPrintf");
	}

	// Set up the special memory mapping
        base_entry = mmu_addEntry(
                vaddr,
                LIB_DOS_SIZE,
		MMU_TYPE_DOS_DOSBASE,
		NULL
	);
	base_entry->read = mmu_fn_protected;
	base_entry->write = mmu_fn_protected;
	base_entry->nativePtr = (void *)&lib_dos_base;

	DEBUG(3) {
	        // Add symbols
	        mmu_addSymbol(base_entry, vaddr, "!DosBase");
	}
*/
	is_init = 1;

	// Init subsystems
        if( s_process::init() ) { cleanup(); return 1; }
/*
	if( lib_dos_s_filehandle_init() ) { lib_dos_cleanup(); return 1; }
	if( lib_dos_s_commandlineinterface_init() ) { lib_dos_cleanup(); return 1; }
	if( lib_dos_s_filelock_init() ) { lib_dos_cleanup(); return 1; }
	if( lib_dos_s_rdargs_init() ) { lib_dos_cleanup(); return 1; }
	if( lib_dos_s_anchorpath_init() ) { lib_dos_cleanup(); return 1; }
	if( lib_dos_s_achain_init() ) { lib_dos_cleanup(); return 1; }
	if( lib_dos_s_datestamp_init() ) { lib_dos_cleanup(); return 1; }
*/
	return 0;
}

// Global cleanup
void s_doslibrary::cleanup()
{
/*
	uint32_t vaddr;
	uint8_t	*addr;
*/
	DEBUG(3) dprintf("dos.library: cleanup() called\n");

	// Cleanup subsystems
/*
        lib_dos_s_datestamp_cleanup();
        lib_dos_s_achain_cleanup();
        lib_dos_s_anchorpath_cleanup();
        lib_dos_s_rdargs_cleanup();
        lib_dos_s_filelock_cleanup();
        lib_dos_s_commandlineinterface_cleanup();
        lib_dos_s_filehandle_cleanup();
*/
        s_process::cleanup();

        if( !is_init ) {
                DEBUG(2) dprintf("Warning: dos.library: cleanup: Not initialized.\n");
                return;
        }
/*
	// Delete mmu entry, DosLibrary structure
	mmu_delEntry(obj->entry);

	vaddr = obj->vec_entry->startAddr;
	addr = (uint8_t *)obj->vec_entry->nativePtr;

	// Delete mmu entry, vectors
	mmu_delEntry(obj->vec_entry);

	// Free real memory, vectors
	FreeVec(addr);

	// Delete vmemory, vectors + DosLibrary structure
	vfreemem_nomap(vaddr);

	// Free hook module number
	free_hook(mnum);
*/

	is_init = 0;
}

static int hook1(emumsg_syscall_t *msg)
{
	switch(msg->func){
/*
		case 5:
			return lib_dos_f_Open(msg);
		case 6:
			return lib_dos_f_Close(msg);
//		case 7:
//			return lib_dos_f_Read(cpu);
		case 8:
			return lib_dos_f_Write(msg);
		case 9:
			return lib_dos_f_Input(msg);
		case 10:
			return lib_dos_f_Output(msg);
		case 15:
			return lib_dos_f_UnLock(msg);
		case 16:
			return lib_dos_f_DupLock(msg);
//		case 23:
//			return lib_dos_f_CreateProc(cpu);
		case 25:
			return lib_dos_f_LoadSeg(msg);
		case 26:
			return lib_dos_f_UnLoadSeg(msg);
		case 33:
			return lib_dos_f_Delay(msg);
		case 132:
			return lib_dos_f_CheckSignal(msg);
		case 133:
			return lib_dos_f_ReadArgs(msg);
		case 137:
			return lib_dos_f_MatchFirst(msg);
		case 138:
			return lib_dos_f_MatchNext(msg);
		case 139:
			return lib_dos_f_MatchEnd(msg);
		case 143:
			return lib_dos_f_FreeArgs(msg);
		case 158:
			return lib_dos_f_PutStr(msg);
		case 159:
			return lib_dos_f_VPrintf(msg);
*/
	}
	dprintf("Error: dos.library: hook1: Unimplemented function %u\n", msg->func);
	return HOOK_END_PROC;
}

static int hook2(emumsg_syscall_t *msg)
{
	switch(msg->func){
/*
		case 5:
			return lib_dos_f_Open_2(msg);
		case 6:
			return lib_dos_f_Close_2(msg);
		case 8:
			return lib_dos_f_Write_2(msg);
		case 9:
			return lib_dos_f_Input_2(msg);
		case 10:
			return lib_dos_f_Output_2(msg);
		case 15:
			return lib_dos_f_UnLock_2(msg);
		case 16:
			return lib_dos_f_DupLock_2(msg);
		case 33:
			return lib_dos_f_Delay_2(msg);
		case 132:
			return lib_dos_f_CheckSignal_2(msg);
		case 133:
			return lib_dos_f_ReadArgs_2(msg);
		case 137:
			return lib_dos_f_MatchFirst_2(msg);
		case 138:
			return lib_dos_f_MatchNext_2(msg);
		case 139:
			return lib_dos_f_MatchEnd_2(msg);
		case 143:
			return lib_dos_f_FreeArgs_2(msg);
		case 158:
			return lib_dos_f_PutStr_2(msg);
		case 159:
			return lib_dos_f_VPrintf_2(msg);
*/
	}
	dprintf("Error: dos.library: hook2: Unimplemented function %u\n", msg->func);
	return HOOK_END_PROC;
}

static int hook3(emumsg_syscall_t *msg)
{
	switch(msg->func){
/*
		case 5:
			return lib_dos_f_Open_3(msg);
		case 6:
			return lib_dos_f_Close_3(msg);
		case 8:
			return lib_dos_f_Write_3(msg);
		case 9:
			return lib_dos_f_Input_3(msg);
		case 10:
			return lib_dos_f_Output_3(msg);
		case 15:
			return lib_dos_f_UnLock_3(msg);
		case 16:
			return lib_dos_f_DupLock_3(msg);
		case 33:
			return lib_dos_f_Delay_3(msg);
		case 132:
			return lib_dos_f_CheckSignal_3(msg);
		case 133:
			return lib_dos_f_ReadArgs_3(msg);
		case 137:
			return lib_dos_f_MatchFirst_3(msg);
		case 138:
			return lib_dos_f_MatchNext_3(msg);
		case 139:
			return lib_dos_f_MatchEnd_3(msg);
		case 143:
			return lib_dos_f_FreeArgs_3(msg);
		case 158:
			return lib_dos_f_PutStr_3(msg);
		case 159:
			return lib_dos_f_VPrintf_3(msg);
*/
	}
	dprintf("Error: dos.library: hook3: Unimplemented function %u\n", msg->func);
	return HOOK_END_PROC;
}
