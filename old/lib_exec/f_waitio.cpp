/*
lib_exec/f_waitio.c - exec.library WaitIO function

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

#include <proto/exec.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../hook.h"
#include "../emulator.h"
#include "s_iorequest.h"

/****************************
 Function: WaitIO
   Vector: 79
   Offset: -474
Arguments: A1.L pointer to struct IORequest
  Returns: D0.L error code
****************************/
int lib_exec_f_WaitIO(emumsg_syscall_t *msg)
{
	cpu_t *cpu;
	lib_exec_s_iorequest_t *iorequest;

	DEBUG(4) dprintf("exec.library: lib_exec_f_WaitIO() called\n");

	cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  IORequest ptr (A1): 0x%x\n", cpu->a[1]);

	/* Find object mapping */
	iorequest = lib_exec_s_iorequest_get_vaddr(cpu->a[1]);
	if(iorequest == NULL) {
		DEBUG(1) dprintf("Error: exec.library: lib_exec_f_WaitIO: IORequest struct pointer is invalid\n");
		return HOOK_END_PROC;
	}

	/* Prepare syscall */
	msg->arg[0]._aptr = iorequest->real;

	DEBUG(5) dprintf("  IORequest ptr (native): %p\n", iorequest->real);

	/* Do syscall */
	return HOOK_SYSCALL;
}

int lib_exec_f_WaitIO_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */
	msg->arg[0]._byte = WaitIO((struct IORequest *)msg->arg[0]._aptr);

	return HOOK_DONE;
}

int lib_exec_f_WaitIO_3(emumsg_syscall_t *msg)
{
	cpu_t *cpu;

	DEBUG(4) dprintf("exec.library: lib_exec_f_WaitIO_3() called\n");

	cpu = msg->proc->cpu;

	/* Extract return values */
	cpu->d[0] = (uint32_t)(int32_t)msg->arg[0]._byte;

	DEBUG(5) dprintf("  Error code (D0): 0x%x\n",cpu->d[0]);

	return HOOK_DONE;
}
