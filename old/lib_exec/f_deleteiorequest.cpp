/*
lib_exec/f_deleteiorequest.c - exec.library DeleteIORequest function

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

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <proto/exec.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../hook.h"
#include "../emulator.h"
#include "s_iorequest.h"

/****************************
 Function: DeleteIORequest
   Vector: 110
   Offset: -660
Arguments: A0.L pointer to IORequest structure
  Returns: -
****************************/
int lib_exec_f_DeleteIORequest(emumsg_syscall_t *msg)
{
	cpu_t *cpu;
	lib_exec_s_iorequest_t *iorequest;

	DEBUG(4) dprintf("exec.library: lib_exec_DeleteIORequest() called\n");

	cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  IORequest ptr (A0): 0x%x\n",cpu->a[0]);

	/* NULL is ok */
	if(cpu->a[0] == 0)
		return HOOK_DONE;

	/* Get IORequest mapping */
	iorequest = lib_exec_s_iorequest_get_vaddr(cpu->a[0]);
	if(iorequest == NULL) {
		DEBUG(1) dprintf("Error: exec.library: lib_exec_DeleteIORequest: IORequest struct pointer is invalid\n");
		return HOOK_END_PROC;
	}

	/* Prepare syscall */
	msg->arg[0]._aptr = iorequest->real;

	DEBUG(5) dprintf("  IORequest ptr (native): %p\n",iorequest->real);

	/* Remember object */
	msg->internalPtr = iorequest;

        /* Do syscall */
        return HOOK_SYSCALL;
}

int lib_exec_f_DeleteIORequest_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */
	DeleteIORequest((struct IORequest *)msg->arg[0]._aptr);

	return HOOK_DONE;
}

int lib_exec_f_DeleteIORequest_3(emumsg_syscall_t *msg)
{
	lib_exec_s_iorequest_t *iorequest;

        DEBUG(4) dprintf("exec.library: lib_exec_f_DeleteIORequest_3() called\n");

	/* Recall object */
	iorequest = (lib_exec_s_iorequest_t *)msg->internalPtr;

	/* Delete it */
	mmu_delEntry(iorequest->entry);

	return HOOK_DONE;
}
