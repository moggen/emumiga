/*
lib_dos/f_matchfirst.c - dos.library MatchFirst function

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
#include "s_anchorpath.h"

/****************************
 Function: MatchFirst
   Vector: 137
   Offset: -822
Arguments: D1.L pattern string
           D2.L pointer to anchor struct
  Returns: D0.L 0=success
****************************/
int lib_dos_f_MatchFirst(emumsg_syscall_t *msg)
{
        cpu_t *cpu;
	uint8_t *string_real;
	lib_dos_s_anchorpath_t *ap;

	DEBUG(4) dprintf("dos.library: lib_dos_f_MatchFirst() called\n");

        cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  Pattern (D1): 0x%x, AnchorPath (D2): 0x%x\n",cpu->d[1],cpu->d[2]);

        string_real = vptr(cpu->d[1]);
        if(string_real == NULL) {
                DEBUG(1) dprintf("Error: dos.library: lib_dos_f_MatchFirst: Pattern string pointer is invalid\n");
                return HOOK_END_PROC;
        }

	ap = lib_dos_s_anchorpath_get_vaddr(cpu->d[2]);
	if(ap == NULL) {
		cpu->d[0]=0xffffffff;
		return HOOK_DONE;
	}

	/* Prepare syscall */
	msg->arg[0]._strptr = (STRPTR)string_real;
	msg->arg[1]._aptr = (APTR)(ap->real);

	DEBUG(5) dprintf("  Pattern (native): %p, AnchorPath (native): %p\n",string_real,ap->real);

	/* Do syscall */
	return HOOK_SYSCALL;
}

int lib_dos_f_MatchFirst_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */
	msg->arg[0]._long = MatchFirst(msg->arg[0]._strptr, (struct AnchorPath *)msg->arg[1]._aptr);

	return HOOK_DONE;
}

int lib_dos_f_MatchFirst_3(emumsg_syscall_t *msg)
{
	cpu_t *cpu;

	DEBUG(4) dprintf("dos.library: lib_dos_f_MatchFirst_3() called\n");

	cpu = msg->proc->cpu;

	cpu->d[0] = msg->arg[0]._long;

	DEBUG(5) dprintf("  Error (D0): %d\n", msg->arg[0]._long);

	return HOOK_DONE;
}
