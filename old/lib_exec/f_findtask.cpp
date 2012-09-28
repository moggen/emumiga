/*
lib_exec/f_findtask.c - exec.library FindTask function

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

#include <proto/exec.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../hook.h"
#include "../emulator.h"

/****************************
 Function: FindTask
   Vector: 49
   Offset: -294
Arguments: A1.L Ptr to name of task or NULL to find the current
  Returns: D0.L Ptr to task/process
****************************/
int lib_exec_f_FindTask(emumsg_syscall_t *msg)
{
	cpu_t *cpu;
	lib_dos_s_process_t *proc;
	uint32_t vaddr;

	DEBUG(4) dprintf("exec.library: lib_exec_f_FindTask() called\n");

	cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  Name of task (A1): 0x%x\n",cpu->a[1]);

	/* We can only hande NULL input so far */
	if(cpu->a[1] != 0) {
	        dprintf("Error: exec.library: lib_exec_f_FindTask: Non-NULL argument is not implemented\n");
		return HOOK_END_PROC;
	}

	/* TODO: Do real syscall for named processes */

	/* Get process */
	proc = emulator_current_process();

	DEBUG(5) dprintf("  Pointer to task (native): %p\n",proc);

	if(proc != NULL) {
		vaddr = proc->entry->startAddr;
	} else {
		vaddr = 0;
	}
	cpu->d[0] = vaddr;

	DEBUG(5) dprintf("  Pointer to task (D0): 0x%x\n",cpu->d[0]);

	return HOOK_DONE;
}
