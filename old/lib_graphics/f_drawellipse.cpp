/*
lib_graphics/f_drawellipse.c - graphics.library
	DrawEllipse function

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

#include <proto/graphics.h>
#include <graphics/rastport.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../hook.h"
#include "../emulator.h"
#include "s_rastport.h"

/****************************
 Function: DrawEllipse
   Vector: 30
   Offset: -180
Arguments: A1.L pointer to struct RastPort
           D0.L center x coordinate
           D1.L center y coordinate
           D2.L horizontal radius
           D3.L vertical radius
  Returns: -
****************************/
int lib_graphics_f_DrawEllipse(emumsg_syscall_t *msg)
{
        cpu_t *cpu;
	lib_graphics_s_rastport_t *rastport;

	DEBUG(4) dprintf("graphics.library: lib_graphics_f_DrawEllipse() called\n");

        cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  RastPort ptr (A1): 0x%x, Center X (D0): %d, Center Y (D1): %d, Radius X (D2): %d, Radius Y (D3): %d\n",cpu->a[1],cpu->d[0],cpu->d[1],cpu->d[2],cpu->d[3]);

	if(cpu->a[1] == 0 ) {
		DEBUG(1) dprintf("Error: graphics.library: lib_graphics_f_DrawEllipse: RastPort must be provided.\n");
		return HOOK_END_PROC;
	}

	rastport = lib_graphics_s_rastport_get_vaddr(cpu->a[1]);
	if(rastport == NULL) {
		DEBUG(1) dprintf("Error: graphics.library: lib_graphics_f_DrawEllipse: RastPort pointer is invalid\n");
		return HOOK_END_PROC;
	}

	/* Prepare syscall */
	msg->arg[0]._aptr = (APTR)rastport->real;
	msg->arg[1]._long = cpu->d[0];
	msg->arg[2]._long = cpu->d[1];
	msg->arg[3]._long = cpu->d[2];
	msg->arg[4]._long = cpu->d[3];

	DEBUG(5) dprintf("  RastPort ptr (native): %p\n",rastport->real);

	/* Do syscall */
	return HOOK_SYSCALL;
}

int lib_graphics_f_DrawEllipse_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */
	DrawEllipse(
		(struct RastPort *)msg->arg[0]._aptr,
		msg->arg[1]._long,
		msg->arg[2]._long,
		msg->arg[3]._long,
		msg->arg[4]._long
	);

	return HOOK_DONE;
}

int lib_graphics_f_DrawEllipse_3(emumsg_syscall_t *msg)
{
	DEBUG(4) dprintf("graphics.library: lib_graphics_f_DrawEllipse_3() called\n");

	return HOOK_DONE;
}
