/*
lib_exec/f_allocmem.c - exec.library AllocMem function

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
 Function: AllocMem
   Vector: 33
   Offset: -198
Arguments: D0.L size in bytes
           D1.L attributes
  Returns: D0.L pointer to allocated memory or 0 if it failed
****************************/
int lib_exec_f_AllocMem(emumsg_syscall_t *msg)
{
	cpu_t *cpu;

	DEBUG(4) dprintf("exec.library: lib_exec_f_AllocMem() called\n");

	cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  Size (D0): %u, Attributes (D1): 0x%x\n",cpu->d[0],cpu->d[1]);

	/* Prepare syscall */
	msg->arg[0]._ulong = (ULONG)cpu->d[0];
	msg->arg[1]._ulong = (ULONG)cpu->d[1];

	/* Do syscall */
	return HOOK_SYSCALL;
}

int lib_exec_f_AllocMem_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */
	msg->arg[0]._aptr = AllocMem(msg->arg[0]._ulong, msg->arg[1]._ulong);

        return HOOK_DONE;
}

int lib_exec_f_AllocMem_3(emumsg_syscall_t *msg)
{
	cpu_t *cpu;
	APTR realmem;
	uint32_t vaddr;
	mmu_entry_t *entry;
	char symbol[200];

	DEBUG(4) dprintf("exec.library: lib_exec_f_AllocMem_3() called\n");

	cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  Memory ptr (native): %p\n", msg->arg[0]._aptr);

	/* Extract return values */
	realmem = msg->arg[0]._aptr;

	/* No mem? */
	if(realmem == NULL) {
		cpu->d[0] = 0;
		DEBUG(5) dprintf("  Memory ptr (D0): 0x%x\n",0);
		return HOOK_DONE;
	}

	/* Allocate virtual 68k memory */
	vaddr = vallocmem_nomap(cpu->d[0]);
	/* TODO: Error check */

	/* Set up MMU entry */
	entry = mmu_addEntry(vaddr, cpu->d[0], MMU_TYPE_EXEC_ALLOCMEM, NULL);
	entry->read = mmu_fn_r_binary;
	entry->write = mmu_fn_w_binary;
	entry->nativePtr = realmem;

	if(CPU_IS_SET(cpu,CPU_FLAG_DISASM)) {
                /* Add symbol */
                sprintf(symbol, "!%08x.mem", (unsigned int)vaddr);
                mmu_addSymbol(entry, vaddr, symbol);
	}

	cpu->d[0] = vaddr;

	DEBUG(5) dprintf("  Memory ptr (D0): 0x%x\n",vaddr);

	return HOOK_DONE;
}
