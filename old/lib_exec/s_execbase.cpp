/*
lib_exec/s_execbase.cpp - Simulation of AmigaOS exec.library

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

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <proto/exec.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../memory/mapping.h"
#include "../vmem.h"
#include "../hook.h"
#include "s_execbase.h"
/*
#include "functions.h"
#include "f_rawdofmt.h"
#include "s_msgport.h"
#include "s_iorequest.h"
#include "s_message.h"

#include "../lib_dos/s_process.h"
*/

namespace emumiga {
namespace lib_exec {
/*
// Kickstart 2.0
#define LIB_EXEC_SIZE 616
#define LIB_EXEC_VECTORS 130

// Mappings
static memory::mapping *entry;
static memory::mapping *vec_entry;
static memory::mapping *first_entry;

// Hook module number
static uint16_t mnum;
*/

// Init flag
static int is_init = 0;

/* Predecls */
static int hook1(emumsg_syscall_t *msg);
static int hook2(emumsg_syscall_t *msg);
static int hook3(emumsg_syscall_t *msg);

/*
// Read map function for first page in memory
static int map_first_r(mmu_entry_t *entry, uint32_t offset, uint32_t size,
        uint8_t *data )
{
	uint32_t i;
	uint32_t execbase;

	DEBUG(7) dprintf("exec.library: map_first_r() called. VAddr: 0x%x, Size: 0x%x\n", (unsigned int)(entry->startAddr+offset), (unsigned int)size);

	execbase = lib_exec_base.entry->startAddr;

	for(i=offset;i<(size+offset);i++) {
		switch(i) {

			case 4:
				*data++ = (execbase >> 24) & 0xff;
				break;
			case 5:
				*data++ = (execbase >> 16) & 0xff;
				break;
			case 6:
				*data++ = (execbase >> 8) & 0xff;
				break;
			case 7:
				*data++ = (execbase) & 0xff;
				break;

			default:
			        dprintf("Error: exec.library: First memory page: Read is not allowed. Offset=%u\n", (unsigned int)i);
			        return 1;
		}
	}
	return 0;
}

// Read map function for struct ExecBase
static int map_r(mmu_entry_t *entry, uint32_t offset, uint32_t size,
        uint8_t *data )
{
	uint32_t i;
	lib_dos_s_process_t *thistask_proc;
	uint32_t thistask_vaddr=0;

	DEBUG(7) dprintf("exec.library: map_r() called. VAddr: 0x%x, Offset: 0x%x, Size: 0x%x\n", (unsigned int)(entry->startAddr+offset), (unsigned int)offset, (unsigned int)size);


	for(i=offset;i<(size+offset);i++) {
		switch(i) {

			// LibNode.lib_Version
			case 20: case 21:
				break;

			// ThisTask
			case 276: case 277: case 278: case 279:
				thistask_proc = emulator_current_process();
				if(thistask_proc != NULL) {
					thistask_vaddr = thistask_proc->entry->startAddr;
				} else {
					thistask_vaddr = 0;
				}
				i = 279;
				break;

			default:
				// Nothing found, signal error
			        dprintf("Error: exec.library: struct ExecLibrary: Read is not allowed. Offset=%u\n", (unsigned int)offset);
			        return 1;
		}
	}

	for(i=offset;i<(size+offset);i++) {
		switch(i) {

			// LibNode.lib_Version
			case 20:
				*data++ = 0;
				break;
			case 21:
				// 36 = Exec 2.0
				*data++ = 36;
				break;

			// ThisTask
			case 276:
				*data++ = (thistask_vaddr >> 24) & 0xff;
				break;
			case 277:
				*data++ = (thistask_vaddr >> 16) & 0xff;
				break;
			case 278:
				*data++ = (thistask_vaddr >> 8) & 0xff;
				break;
			case 279:
				*data++ = (thistask_vaddr) & 0xff;
				break;

			default:
				// Nothing found, signal error
			        dprintf("Error: exec.library: struct ExecLibrary: Read is not allowed. Offset=%u\n", (unsigned int)offset);
			        return 1;
		}
	}
	return 0;
}
*/

// Global init
int s_execbase::init()
{
/*
	uint32_t vaddr;
	uint32_t cur_emu, cur_vec;
	uint16_t cur_fnum;
	uint8_t *addr;
*/
	DEBUG(3) dprintf("exec.library: init() called\n");
/*
	// Get a hook module number
	mnum = allocate_hook("exec.library", hook1, hook2, hook3);

	// Clear memory
        memset((void *)&lib_exec_base,0,sizeof(lib_exec_t));

        // Set up the special memory mapping for the first page
        lib_exec_base.first_entry = mmu_addEntry(
                0,
                VMEM_RESERVE,
		MMU_TYPE_VALLOCMEM,
		NULL
	);
	lib_exec_base.first_entry->read = map_first_r;
	lib_exec_base.first_entry->write = mmu_fn_protected;

	DEBUG(3) {
		// Add a symbol for it
		mmu_addSymbol(lib_exec_base.first_entry, 0, "!zeropage");

		// Add a symbol for AbsExecBase
		mmu_addSymbol(lib_exec_base.first_entry, 4, "!exec.AbsExecBase");
	}

	// Allocate virtual space for
	//  - Emulator instruction + rts = 8 bytes * VECTORS
	//  - Standard jmp table = 6 bytes * VECTORS
	//  - Exec library struct
	vaddr = vallocmem_nomap((6+8)*LIB_EXEC_VECTORS+LIB_EXEC_SIZE);

        // Allocate real memory for
        //  - Emulator instruction + rts = 8 bytes * VECTORS
        //  - Standard jmp table = 6 bytes * VECTORS
        addr = (uint8_t *)AllocVec((6+8)*LIB_EXEC_VECTORS, MEMF_PUBLIC|MEMF_CLEAR);

        // Set up binary mapping
        lib_exec_base.vec_entry = mmu_addEntry(
                vaddr,
                (6+8)*LIB_EXEC_VECTORS,
		MMU_TYPE_EXEC_VECTORS,
		NULL
	);
	lib_exec_base.vec_entry->read = mmu_fn_r_binary;
	lib_exec_base.vec_entry->write = mmu_fn_w_binary;
	lib_exec_base.vec_entry->nativePtr = addr;

	DEBUG(3) {
		// Add a symbol for it
		mmu_addSymbol(lib_exec_base.vec_entry, vaddr, "!exec.vec");
	}

	// Loop over all vectors, cur_fnum counts down
	cur_fnum = LIB_EXEC_VECTORS;
	cur_emu = vaddr;
	cur_vec = cur_emu + 8*LIB_EXEC_VECTORS;
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

	// Write protect the exec base
	lib_exec_base.vec_entry->write = mmu_fn_protected;

        // Calculate library struct address
        vaddr += (6+8)*LIB_EXEC_VECTORS;

	DEBUG(3) {
		// Add implemented LVO symbols
		mmu_addSymbol(lib_exec_base.vec_entry, vaddr-198, "!exec.vec.AllocMem");
		mmu_addSymbol(lib_exec_base.vec_entry, vaddr-210, "!exec.vec.FreeMem");
		mmu_addSymbol(lib_exec_base.vec_entry, vaddr-294, "!exec.vec.FindTask");
		mmu_addSymbol(lib_exec_base.vec_entry, vaddr-306, "!exec.vec.SetSignal");
		mmu_addSymbol(lib_exec_base.vec_entry, vaddr-318, "!exec.vec.Wait");
		mmu_addSymbol(lib_exec_base.vec_entry, vaddr-324, "!exec.vec.Signal");
		mmu_addSymbol(lib_exec_base.vec_entry, vaddr-330, "!exec.vec.AllocSignal");
		mmu_addSymbol(lib_exec_base.vec_entry, vaddr-336, "!exec.vec.FreeSignal");
		mmu_addSymbol(lib_exec_base.vec_entry, vaddr-366, "!exec.vec.PutMsg");
		mmu_addSymbol(lib_exec_base.vec_entry, vaddr-372, "!exec.vec.GetMsg");
		mmu_addSymbol(lib_exec_base.vec_entry, vaddr-378, "!exec.vec.ReplyMsg");
		mmu_addSymbol(lib_exec_base.vec_entry, vaddr-384, "!exec.vec.WaitPort");
		mmu_addSymbol(lib_exec_base.vec_entry, vaddr-414, "!exec.vec.CloseLibrary");
		mmu_addSymbol(lib_exec_base.vec_entry, vaddr-444, "!exec.vec.OpenDevice");
		mmu_addSymbol(lib_exec_base.vec_entry, vaddr-450, "!exec.vec.CloseDevice");
		mmu_addSymbol(lib_exec_base.vec_entry, vaddr-462, "!exec.vec.SendIO");
		mmu_addSymbol(lib_exec_base.vec_entry, vaddr-468, "!exec.vec.CheckIO");
		mmu_addSymbol(lib_exec_base.vec_entry, vaddr-474, "!exec.vec.WaitIO");
		mmu_addSymbol(lib_exec_base.vec_entry, vaddr-480, "!exec.vec.AbortIO");
		mmu_addSymbol(lib_exec_base.vec_entry, vaddr-522, "!exec.vec.RawDoFmt");
		mmu_addSymbol(lib_exec_base.vec_entry, vaddr-528, "!exec.vec.GetCC");
		mmu_addSymbol(lib_exec_base.vec_entry, vaddr-552, "!exec.vec.OpenLibrary");
		mmu_addSymbol(lib_exec_base.vec_entry, vaddr-624, "!exec.vec.CopyMem");
		mmu_addSymbol(lib_exec_base.vec_entry, vaddr-654, "!exec.vec.CreateIORequest");
		mmu_addSymbol(lib_exec_base.vec_entry, vaddr-660, "!exec.vec.DeleteIORequest");
		mmu_addSymbol(lib_exec_base.vec_entry, vaddr-666, "!exec.vec.CreateMsgPort");
		mmu_addSymbol(lib_exec_base.vec_entry, vaddr-672, "!exec.vec.DeleteMsgPort");
		mmu_addSymbol(lib_exec_base.vec_entry, vaddr-684, "!exec.vec.AllocVec");
		mmu_addSymbol(lib_exec_base.vec_entry, vaddr-690, "!exec.vec.FreeVec");
	}

        // Set up the special memory mapping
        lib_exec_base.entry = mmu_addEntry(
                vaddr,
                LIB_EXEC_SIZE,
		MMU_TYPE_EXEC_EXECBASE,
		NULL
	);

	lib_exec_base.entry->read = map_r;
	lib_exec_base.entry->write = mmu_fn_protected;
	lib_exec_base.entry->nativePtr = (void *)&lib_exec_base;

	DEBUG(3) {
		// Add symbols
		mmu_addSymbol(lib_exec_base.entry, vaddr, "!ExecBase");
		mmu_addSymbol(lib_exec_base.entry, vaddr+20, "!ExecBase.lib_Version");
		mmu_addSymbol(lib_exec_base.entry, vaddr+276, "!ExecBase.ThisTask");
	}
*/
	is_init = 1;

	// Init subsystems
/*
	if( lib_exec_f_rawdofmt_init(mnum) ) { lib_exec_cleanup(); return 1; }
	if( lib_exec_s_msgport_init() ) { lib_exec_cleanup(); return 1; }
	if( lib_exec_s_iorequest_init() ) { lib_exec_cleanup(); return 1; }
	if( lib_exec_s_message_init() ) { lib_exec_cleanup(); return 1; }
*/
	return 0;
}

// Global cleanup
void s_execbase::cleanup()
{
/*
	uint32_t vaddr;
	uint8_t *addr;
	lib_exec_t *obj;
*/
	DEBUG(3) dprintf("exec.library: cleanup() called\n");

	// Cleanup subsystems
/*
	lib_exec_s_message_cleanup();
	lib_exec_s_iorequest_cleanup();
	lib_exec_s_msgport_cleanup();
	lib_exec_f_rawdofmt_cleanup();
*/
        if( !is_init ) {
                DEBUG(2) dprintf("exec.library: cleanup: Not initialized\n");
                return;
        }

/*
	// Delete mmu entry, first page
        mmu_delEntry(obj->first_entry);

	// Delete mmu entry, ExecLibrary structure
        mmu_delEntry(obj->entry);

        vaddr = obj->vec_entry->startAddr;
        addr = (uint8_t *)obj->vec_entry->nativePtr;

	// Delete mmu entry, vectors
        mmu_delEntry(obj->vec_entry);

	// Free real memory, vectors
        FreeVec(addr);

	// Delete vmemory, vectors + ExecLibrary structure
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
		case 33:
			return lib_exec_f_AllocMem(msg);
		case 35:
			return lib_exec_f_FreeMem(msg);
		case 49:
			return lib_exec_f_FindTask(msg);
		case 51:
			return lib_exec_f_SetSignal(msg);
		case 53:
			return lib_exec_f_Wait(msg);
//		case 54:
//			return lib_exec_f_Signal(cpu);
//		case 55:
//			return lib_exec_f_AllocSignal(cpu);
//		case 56:
//			return lib_exec_f_FreeSignal(cpu);
//		case 61:
//			return lib_exec_f_PutMsg(cpu);
		case 62:
			return lib_exec_f_GetMsg(msg);
		case 63:
			return lib_exec_f_ReplyMsg(msg);
//		case 64:
//			return lib_exec_f_WaitPort(cpu);
		case 69:
			return lib_exec_f_CloseLibrary(msg);
		case 74:
			return lib_exec_f_OpenDevice(msg);
		case 75:
			return lib_exec_f_CloseDevice(msg);
		case 77:
			return lib_exec_f_SendIO(msg);
		case 78:
			return lib_exec_f_CheckIO(msg);
		case 79:
			return lib_exec_f_WaitIO(msg);
		case 80:
			return lib_exec_f_AbortIO(msg);
		case 87:
			return lib_exec_f_RawDoFmt(msg);
		case -87:
			return lib_exec_f_RawDoFmt_cb(msg);
//		case 88:
//			return lib_exec_f_GetCC(cpu);
		case 92:
			return lib_exec_f_OpenLibrary(msg);
		case 104:
			return lib_exec_f_CopyMem(msg);
		case 109:
			return lib_exec_f_CreateIORequest(msg);
		case 110:
			return lib_exec_f_DeleteIORequest(msg);
		case 111:
			return lib_exec_f_CreateMsgPort(msg);
		case 112:
			return lib_exec_f_DeleteMsgPort(msg);
		case 114:
			return lib_exec_f_AllocVec(msg);
		case 115:
			return lib_exec_f_FreeVec(msg);
*/
	}
	dprintf("Error: exec.library: hook1: Unimplemented function %u\n", msg->func);
	return HOOK_END_PROC;
}

static int hook2(emumsg_syscall_t *msg)
{
	switch(msg->func){
/*
		case 33:
			return lib_exec_f_AllocMem_2(msg);
		case 35:
			return lib_exec_f_FreeMem_2(msg);
		case 51:
			return lib_exec_f_SetSignal_2(msg);
		case 53:
			return lib_exec_f_Wait_2(msg);
		case 62:
			return lib_exec_f_GetMsg_2(msg);
		case 63:
			return lib_exec_f_ReplyMsg_2(msg);
		case 74:
			return lib_exec_f_OpenDevice_2(msg);
		case 75:
			return lib_exec_f_CloseDevice_2(msg);
		case 77:
			return lib_exec_f_SendIO_2(msg);
		case 78:
			return lib_exec_f_CheckIO_2(msg);
		case 79:
			return lib_exec_f_WaitIO_2(msg);
		case 80:
			return lib_exec_f_AbortIO_2(msg);
		case 87:
			return lib_exec_f_RawDoFmt_2(msg);
		case -87:
			return lib_exec_f_RawDoFmt_cb_2(msg);
		case 109:
			return lib_exec_f_CreateIORequest_2(msg);
		case 110:
			return lib_exec_f_DeleteIORequest_2(msg);
		case 111:
			return lib_exec_f_CreateMsgPort_2(msg);
		case 112:
			return lib_exec_f_DeleteMsgPort_2(msg);
		case 114:
			return lib_exec_f_AllocVec_2(msg);
		case 115:
			return lib_exec_f_FreeVec_2(msg);
*/
	}
	dprintf("Error: exec.library: hook2: Unimplemented function %u\n", msg->func);
	return HOOK_END_PROC;
}

static int hook3(emumsg_syscall_t *msg)
{
	switch(msg->func){
/*
		case 33:
			return lib_exec_f_AllocMem_3(msg);
		case 35:
			return lib_exec_f_FreeMem_3(msg);
		case 51:
			return lib_exec_f_SetSignal_3(msg);
		case 53:
			return lib_exec_f_Wait_3(msg);
		case 62:
			return lib_exec_f_GetMsg_3(msg);
		case 63:
			return lib_exec_f_ReplyMsg_3(msg);
		case 74:
			return lib_exec_f_OpenDevice_3(msg);
		case 75:
			return lib_exec_f_CloseDevice_3(msg);
		case 77:
			return lib_exec_f_SendIO_3(msg);
		case 78:
			return lib_exec_f_CheckIO_3(msg);
		case 79:
			return lib_exec_f_WaitIO_3(msg);
		case 80:
			return lib_exec_f_AbortIO_3(msg);
		case 87:
			return lib_exec_f_RawDoFmt_3(msg);
		case -87:
			return lib_exec_f_RawDoFmt_cb_3(msg);
		case 109:
			return lib_exec_f_CreateIORequest_3(msg);
		case 110:
			return lib_exec_f_DeleteIORequest_3(msg);
		case 111:
			return lib_exec_f_CreateMsgPort_3(msg);
		case 112:
			return lib_exec_f_DeleteMsgPort_3(msg);
		case 114:
			return lib_exec_f_AllocVec_3(msg);
		case 115:
			return lib_exec_f_FreeVec_3(msg);
*/
	}
	dprintf("Error: exec.library: hook3: Unimplemented function %u\n", msg->func);
	return HOOK_END_PROC;
}
