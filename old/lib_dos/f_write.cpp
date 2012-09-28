/*
lib_dos/f_write.c - dos.library Write function

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
 Function: Write
   Vector: 8
   Offset: -48
Arguments: D1.L BCPL pointer to file handle
           D2.L Pointer to buffer
           D3.L Length
  Returns: D0.L Bytes written
****************************/
int lib_dos_f_Write(emumsg_syscall_t *msg)
{
	cpu_t *cpu;
	lib_dos_s_filehandle_t *obj;
	uint32_t vaddr;
	uint8_t *buffer;

	DEBUG(4) dprintf("dos.library: lib_dos_f_Write() called\n");

	cpu = msg->proc->cpu;

	/* Make normal vaddr */
	vaddr = cpu->d[1] << 2;

	DEBUG(5) dprintf("  File handle (D1): 0x%x (APTR: 0x%x), Pointer (D2): 0x%x, Length (D3): %u\n",cpu->d[1],vaddr,cpu->d[2],cpu->d[3]);


	/* Find the simulation instance */
	obj = lib_dos_s_filehandle_get_vaddr(vaddr);
	if(obj == NULL) {
		DEBUG(1) dprintf("Error: dos.library: lib_dos_f_Write: Attempt to write to something that is not a file.\n");
		return HOOK_END_PROC;
	}

	/* Buffer */
	buffer = vptr(cpu->d[2]);

	/* Prepare syscall */
	msg->arg[0]._bptr = MKBADDR(obj->real);
	msg->arg[1]._aptr = (APTR)buffer;
	msg->arg[2]._long = (LONG)cpu->d[3];

	DEBUG(5) dprintf("  File handle (native): %p, Pointer (native): %p\n",msg->arg[0]._bptr,msg->arg[1]._aptr);

	/* Do syscall */
	return HOOK_SYSCALL;
}

int lib_dos_f_Write_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */
	msg->arg[0]._long = Write(msg->arg[0]._bptr, msg->arg[1]._aptr, msg->arg[2]._long);

	return HOOK_DONE;
}


int lib_dos_f_Write_3(emumsg_syscall_t *msg)
{
	cpu_t *cpu;

	DEBUG(4) dprintf("dos.library: lib_dos_f_Write_3() called\n");

	cpu = msg->proc->cpu;

	/* Extract return values */
	cpu->d[0] = msg->arg[0]._long;

	DEBUG(5) dprintf("  Bytes written (D0): 0x%x\n", cpu->d[0]);

	return HOOK_DONE;
}
