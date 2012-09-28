/*
lib_exec/f_copymem.c - exec.library CopyMem function

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

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../hook.h"
#include "../emulator.h"
#include "../vmem.h"

/****************************
 Function: CopyMem
   Vector: 104
   Offset: -624
Arguments: A0.L pointer to source memory
           A1.L pointer to destination memory
           D0.L size
  Returns: -
****************************/
int lib_exec_f_CopyMem(emumsg_syscall_t *msg)
{
	cpu_t *cpu;
	uint32_t cnt, src, dst;
	uint8_t data;
	int rc, rc2;

	DEBUG(4) dprintf("exec.library: lib_exec_f_CopyMem() called\n");

	cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  Source ptr (A0): 0x%x, Destination ptr (A1): 0x%x, Size (D0): %u\n",cpu->a[0],cpu->a[1],cpu->d[0]);

	if(cpu->a[0] == 0 || cpu->a[1] == 0 || cpu->d[0] == 0)
		return HOOK_DONE;

	/* Don't call the real syscall, we simulate it here and get
	   the benefits of it working with memory mappings */

	cnt = cpu->d[0];
	src = cpu->a[0];
	dst = cpu->a[1];

	while(0 < cnt--) {

		/* Use I-cache for read, D-cache for write to avoid
		   ping-pong effect */

		rc = READMEM_8_I(src++,&data);
		rc2 = WRITEMEM_8_D(data,dst++);

		if(rc > 0 || rc2 > 0) {
			DEBUG(1) dprintf("Error: exec.library: lib_exec_f_CopyMem: Memory access error\n");
			return HOOK_END_PROC;
		}
	}

	/* Done! */
	return HOOK_DONE;
}
