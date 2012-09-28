/*
lib_dos/f_close.c - dos.library Close function

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

#include <proto/dos.h>
#include <dos/dosextens.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../hook.h"
#include "../emulator.h"

#include "s_filehandle.h"

/****************************
 Function: Close
   Vector: 6
   Offset: -36
Arguments: D1.L BCPL pointer to file handle
  Returns: D0.L Boolean success value
****************************/
int lib_dos_f_Close(emumsg_syscall_t *msg)
{
	cpu_t *cpu;
	mmu_entry_t *entry;
	uint32_t vaddr;
	lib_dos_s_filehandle_t *file;

	DEBUG(4) dprintf("dos.library: lib_dos_f_Close() called\n");

	cpu = msg->proc->cpu;

	/* Make normal vaddr */
	vaddr = cpu->d[1] << 2;

	DEBUG(5) dprintf("  File handle (D1): 0x%x (APTR: 0x%x)\n",cpu->d[1],vaddr);

	/* Handle NULL */
	if(vaddr == 0) {
		cpu->d[0] = 0;
		DEBUG(5) dprintf("  Success (D0): 0x%x\n",0);
		return HOOK_DONE;
	}

	/* Find the instance mapping */
	entry = mmu_findEntryWithType(vaddr, MMU_TYPE_DOS_S_FILEHANDLE);
	if(entry == NULL) {
		DEBUG(1) dprintf("Warning: dos.library: Close(): Attempt to close something that is not a file.\n");
		cpu->d[0] = 0;
		DEBUG(5) dprintf("  Success (D0): 0x%x\n",0);
		return HOOK_DONE;
	}
	if(entry->startAddr != vaddr) {
		DEBUG(1) dprintf("Warning: dos.library: Close(): Attempt to close something that is not a file.\n");
		cpu->d[0] = 0;
		DEBUG(5) dprintf("  Success (D0): 0x%x\n",0);
		return HOOK_DONE;
	}

	/* Extract emulation struct */
	file = (lib_dos_s_filehandle_t *) entry->nativePtr;

	/* Prepare syscall */
	msg->arg[0]._bptr = MKBADDR(file->real);

	DEBUG(5) dprintf("  File handle (native): %p\n",msg->arg[0]._bptr);

	/* Do syscall */
	return HOOK_SYSCALL;
}

int lib_dos_f_Close_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */
	msg->arg[0]._bool = Close(msg->arg[0]._bptr);

	return HOOK_DONE;
}

int lib_dos_f_Close_3(emumsg_syscall_t *msg)
{
	cpu_t *cpu;
	mmu_entry_t *entry;
	uint32_t vaddr;

	DEBUG(4) dprintf("dos.library: lib_dos_f_Close_3() called\n");

	cpu = msg->proc->cpu;

	/* Extract return values */
	cpu->d[0] = msg->arg[0]._bool ? 1 : 0;

	DEBUG(5) dprintf("  Success (D0): 0x%x\n", cpu->d[0]);

	if(msg->arg[0]._bool) {
		/* Successful close, delete simulation object */
		vaddr = cpu->d[1] << 2;
		entry = mmu_findEntryWithType(vaddr, MMU_TYPE_DOS_S_FILEHANDLE);
		mmu_delEntry(entry);
	}

	return HOOK_DONE;
}
