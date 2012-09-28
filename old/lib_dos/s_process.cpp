/*
lib_dos/s_process.c - dos.library struct Process

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
#include <unistd.h>

#include <proto/exec.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../macros.h"
#include "s_process.h"
#include "s_commandlineinterface.h"
#include "s_filelock.h"
/*
#include "s_filehandle.h"
*/

namespace emumiga {
namespace lib_dos {

#if 0
/* Read map function */
static int map_r(mmu_entry_t *entry, uint32_t offset, uint32_t size,
        uint8_t *data )
{
	uint32_t i;
	s_process *proc;
	uint32_t splower_vaddr = 0;
	struct FileLock *currentdir_real;
	lib_dos_s_filelock_t *currentdir;
	uint32_t currentdir_vaddr = 0;
	struct CommandLineInterface *cli_real;
	lib_dos_s_commandlineinterface_t *cli;
	uint32_t cli_vaddr = 0;

	DEBUG(7) dprintf("dos.library: struct Process: map_r() called. VAddr: 0x%x, Size: 0x%x\n", (unsigned int)(entry->startAddr+offset), (unsigned int)size);

	/* Get our object */
	proc = (lib_dos_s_process_t *)entry->nativePtr;

	for(i=offset;i<(size+offset);i++) {
		switch(i) {

			/* tc_SPLower */
			case 58: case 59: case 60: case 61:
				splower_vaddr = proc->vaddr_stack + proc->stacksize;
				i = 61;
				break;

			/* pr_CurrentDir */
			case 152: case 153: case 154: case 155:
				currentdir_real = (struct FileLock *)BADDR(proc->real->pr_CurrentDir);
				currentdir = lib_dos_s_filelock_get_real(currentdir_real);
				if(currentdir != NULL) {
					/* BPTR! */
					currentdir_vaddr = currentdir->entry->startAddr >> 2;
				} else {
					currentdir_vaddr = 0;
				}
				i = 155;
				break;

			/* pr_CLI */
			case 172: case 173: case 174: case 175:
				cli_real = (struct CommandLineInterface *)BADDR(proc->real->pr_CLI);
				if(proc->cpu != NULL) {
					/* Simulated process, use special method */
					cli = lib_dos_s_commandlineinterface_get_real_sim(cli_real,proc);
				} else {
					/* No simulation */
					cli = lib_dos_s_commandlineinterface_get_real(cli_real);
				}
				if(cli != NULL) {
					/* BPTR! */
					cli_vaddr = cli->entry->startAddr >> 2;
				} else {
					cli_vaddr = 0;
				}
				i = 175;
				break;

			default:
				/* Nothing found, signal error */
			        dprintf("Error: dos.library: struct Process: Read is not allowed. Offset=%u\n", (unsigned int)offset);
			        return 1;
		}
	}

	for(i=offset;i<(size+offset);i++) {
		switch(i) {

			/* tc_SPLower */
			case 58:
				*data++ = (splower_vaddr >> 24) & 0xff;
				break;
			case 59:
				*data++ = (splower_vaddr >> 16) & 0xff;
				break;
			case 60:
				*data++ = (splower_vaddr >> 8) & 0xff;
				break;
			case 61:
				*data++ = (splower_vaddr) & 0xff;
				break;

			/* pr_CurrentDir */
			case 152:
				*data++ = (currentdir_vaddr >> 24) & 0xff;
				break;
			case 153:
				*data++ = (currentdir_vaddr >> 16) & 0xff;
				break;
			case 154:
				*data++ = (currentdir_vaddr >> 8) & 0xff;
				break;
			case 155:
				*data++ = (currentdir_vaddr) & 0xff;
				break;

			/* pr_CLI */
			case 172:
				*data++ = (cli_vaddr >> 24) & 0xff;
				break;
			case 173:
				*data++ = (cli_vaddr >> 16) & 0xff;
				break;
			case 174:
				*data++ = (cli_vaddr >> 8) & 0xff;
				break;
			case 175:
				*data++ = (cli_vaddr) & 0xff;
				break;

			default:
				/* Nothing found, signal error */
			        dprintf("Error: dos.library: struct Process: Read is not allowed. Offset=%u\n", (unsigned int)offset);
			        return 1;
		}
	}
	return 0;
}
#endif

// Init flag
int s_process::is_init = 0;

// Global init
int s_process::init()
{
	DEBUG(4) dprintf("dos.library: s_process::init() called\n");

        pool = memory::mmu_pool_create(
                "dos.s.Process",
                MMU_TYPE_DOS_S_PROCESS,
                LIB_DOS_S_PROCESS_SIZE,
                sizeof(s_process)
        );

        if(pool == NULL)
                return 1;

	// -1 for uninitalized state
	currentProcessIdx = -1;

	// Allocate and set up emulator instruction to end the task
	endInstruction = vallocmem(6,"!endInstruction");
	if(endInstruction == 0)
		return 1;

	WRITEMEM_16(0xFD80, endInstruction);
	WRITEMEM_32(0, endInstruction+2);

	is_init = 1;
	return 0;
}

// Global cleanup
void s_process::cleanup()
{
	DEBUG(4) dprintf("dos.library: s_process::cleanup() called\n");

        if( !is_init ) {
                DEBUG(2) dprintf("Warning: dos.library: s_process::cleanup: Not initialized\n");
                return;
        }

	// Free end instruction memory area
	vfreemem(endInstruction);

	mmu_pool_destroy(pool);

	is_init = 0;
}

// Get an instance
s_process *s_process::get_real(struct Process *real)
{
	mmu_entry_t *entry;
        lib_dos_s_process_t *obj;
	uint32_t vaddr;
	char symbol[200];

	DEBUG(5) dprintf("dos.library: s_process::get_real() called\n");

	if(real == NULL)
		return NULL;

        // Check to find if it already exists
        entry = mmu_findEntryWithTypeReal(real, MMU_TYPE_DOS_S_PROCESS);
        if(entry != NULL) {

                // Yes, return object
                return (lib_dos_s_process_t *)entry->nativePtr;
        }

        // No, set up an object
        entry = mmu_allocObj(pool, real);
        if(entry == NULL)
                 return NULL;

        // Extract object
        obj = (lib_dos_s_process_t *)entry->nativePtr;

        // Init structure
	entry->read = map_r;
        entry->destructor = destructor;
        obj->entry = entry;
        obj->real = real;

	DEBUG(3) {
                // Add extra symbols
		vaddr = entry->startAddr;
		sprintf(symbol, "!%08x.%s.tc_SPLower", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+58, symbol);
		sprintf(symbol, "!%08x.%s.pr_MsgPort", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+92, symbol);
		sprintf(symbol, "!%08x.%s.pr_CurrentDir", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+152, symbol);
		sprintf(symbol, "!%08x.%s.pr_CIS", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+156, symbol);
		sprintf(symbol, "!%08x.%s.pr_COS", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+160, symbol);
		sprintf(symbol, "!%08x.%s.pr_CLI", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+172, symbol);
		sprintf(symbol, "!%08x.%s.pr_CES", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+224, symbol);
        }

	return obj;
}

// Get an instance
s_process *s_process::get_vaddr(uint32_t vaddr)
{
        mmu_entry_t *entry;

        DEBUG(5) dprintf("dos.library: s_process::get_vaddr() called\n");

        if(vaddr == 0)
                return NULL;

        // Search for an existing object
        entry = mmu_findEntryWithType(vaddr, MMU_TYPE_DOS_S_PROCESS);

        // This structure should not be created from user space
        if(entry == NULL)
                return NULL;

        return (lib_dos_s_process_t *)entry->nativePtr;
}


// Dispose of mapping
int s_process::dispose()
{
	DEBUG(5) dprintf("dos.library: s_process::dispose() called\n");

	if(cpu != NULL) {
		DEBUG(1) dprintf("Warning: dos.library: s_process::dispose: Attempt to dispose of a Process emulation structure with an emulation context!\n");
		return 0;
	}

	delete this;
	return 1;
}

} // namespace lib_dos
} // namespace emumiga
