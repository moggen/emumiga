/*
lib_exec/f_freemem.c - exec.library FreeMem function

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

#include <proto/exec.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../hook.h"
#include "../emulator.h"
#include "../vmem.h"

/****************************
 Function: FreeMem
   Vector: 35
   Offset: -210
Arguments: A1.L pointer to memory
           D0.L size in bytes
  Returns: -
****************************/
int lib_exec_f_FreeMem(emumsg_syscall_t *msg)
{
	cpu_t *cpu;
	mmu_entry_t *entry;

	DEBUG(4) dprintf("exec.library: lib_exec_f_FreeMem() called\n");

	cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  Memory ptr (A1): 0x%x, Size (D0): %u\n",cpu->a[1],cpu->d[0]);

	if(cpu->a[1] == 0)
		return HOOK_DONE;

	/* Find the memory mapping */
	entry = mmu_findEntryWithType(cpu->a[1], MMU_TYPE_EXEC_ALLOCMEM);
	if(entry == NULL) {
		DEBUG(1) dprintf("Warning: exec.library: lib_exec_f_FreeMem(): Attempt to free non-allocated memory\n");
		return HOOK_DONE;
	}
	if(entry->startAddr != cpu->a[1]) {
		DEBUG(1) dprintf("Warning: exec.library: lib_exec_f_FreeMem(): Attempt to free non-allocated memory\n");
		return HOOK_DONE;
	}
	if(entry->length != cpu->d[0]) {
		DEBUG(1) dprintf("Warning: exec.library: lib_exec_f_FreeMem(): Attempt to partly free memory\n");
		return HOOK_DONE;
	}

	/* Prepare syscall */
	msg->arg[0]._aptr = (APTR)entry->nativePtr;
	msg->arg[1]._ulong = (ULONG)cpu->d[0];

	DEBUG(5) dprintf("  Memory ptr (native): %p\n",msg->arg[0]._aptr);

	/* Do syscall */
	return HOOK_SYSCALL;
}

int lib_exec_f_FreeMem_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */
	FreeMem(msg->arg[0]._aptr,msg->arg[1]._ulong);

	return HOOK_DONE;
}

int lib_exec_f_FreeMem_3(emumsg_syscall_t *msg)
{
	cpu_t *cpu;
	mmu_entry_t *entry;

	DEBUG(4) dprintf("exec.library: lib_exec_f_FreeMem() called\n");

	cpu = msg->proc->cpu;

	/* Find the memory mapping */
	entry = mmu_findEntryWithType(cpu->a[1], MMU_TYPE_EXEC_ALLOCMEM);

	/* Delete it */
	mmu_delEntry(entry);

	return HOOK_DONE;
}
