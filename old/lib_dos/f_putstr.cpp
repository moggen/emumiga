/*
lib_exec/f_putstr.c - dos.library PutStr function

Copyright (C) 2009, 2010  Magnus Öberg

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

#include <proto/dos.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../hook.h"
#include "../emulator.h"

/****************************
 Function: PutStr
   Vector: 158
   Offset: -948
Arguments: D1.L pointer to string
  Returns: D0.L result code 0=OK, -1=Error
****************************/
int lib_dos_f_PutStr(emumsg_syscall_t *msg)
{
	cpu_t *cpu;
	uint8_t *sptr;

	DEBUG(4) dprintf("dos.library: lib_dos_f_PutStr() called\n");
	cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  String (D1): 0x%x\n",cpu->d[1]);

	if(cpu->d[1] == 0) {
		DEBUG(1) dprintf("Warning: dos.library: lib_dos_f_PutStr() called with NULL string, skipping\n");
		return HOOK_DONE;
	}

	/* TODO: Implement some kind of check for MMU entry ending within
	         the string
	*/

	sptr = vptr(cpu->d[1]);

	DEBUG(5) dprintf("  String (native): %p\n",sptr);

	/* Prepare syscall */
	msg->arg[0]._strptr = (STRPTR)sptr;

	return HOOK_SYSCALL;
}


int lib_dos_f_PutStr_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */

	msg->arg[0]._long = PutStr(msg->arg[0]._strptr);

	return HOOK_DONE;
}

int lib_dos_f_PutStr_3(emumsg_syscall_t *msg)
{
	cpu_t *cpu;

	DEBUG(4) dprintf("dos.library: lib_dos_f_PutStr_3() called\n");
	cpu = msg->proc->cpu;

	/* Return value */
	cpu->d[0] = (uint32_t)msg->arg[0]._long;

	DEBUG(5) dprintf("  Result code (D0): 0x%x\n",cpu->d[0]);

	return HOOK_DONE;
}
