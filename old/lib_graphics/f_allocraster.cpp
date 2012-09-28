/*
lib_graphics/f_allocraster.c - graphics.library
	AllocRaster function

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
#include <graphics/gfx.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../hook.h"
#include "../emulator.h"

/****************************
 Function: AllocRaster
   Vector: 82
   Offset: -492
Arguments: D0.L width
           D1.L height
  Returns: D0.L ptr to raster
****************************/
int lib_graphics_f_AllocRaster(emumsg_syscall_t *msg)
{
        cpu_t *cpu;

	DEBUG(4) dprintf("graphics.library: lib_graphics_f_AllocRaster() called\n");

        cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  Width (D0): %d, Height (D1): %d\n",cpu->d[0],cpu->d[1]);

	/* Prepare syscall */
	msg->arg[0]._ulong = cpu->d[0];
	msg->arg[1]._ulong = cpu->d[1];

	/* Do syscall */
	return HOOK_SYSCALL;
}

int lib_graphics_f_AllocRaster_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */
	msg->arg[0]._aptr = AllocRaster(
		msg->arg[0]._ulong,
		msg->arg[1]._ulong
	);

	return HOOK_DONE;
}

int lib_graphics_f_AllocRaster_3(emumsg_syscall_t *msg)
{
	cpu_t *cpu;
	uint32_t vaddr;

	DEBUG(4) dprintf("graphics.library: lib_graphics_f_AllocRaster_3() called\n");

        cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  Raster ptr (native): %p\n",msg->arg[0]._aptr);

	if(msg->arg[0]._aptr == NULL) {
		cpu->d[0] = 0;
		DEBUG(5) dprintf("  Raster ptr (D0): 0x%x\n",0);
		return HOOK_DONE;
	}

	vaddr = vallocmem_ram((uint8_t *)msg->arg[0]._aptr,RASSIZE(cpu->d[0],cpu->d[1]),"lib.graphics.AllocRaster");
	if(vaddr == 0) {
		DEBUG(1) dprintf("Error: graphics.library: lib_graphics_f_AllocRaster_3: Could not map allocated data into vmem\n");
		return HOOK_END_PROC;
	}

	cpu->d[0] = vaddr;

	DEBUG(5) dprintf("  Raster ptr (D0): 0x%x\n",cpu->d[0]);

	return HOOK_DONE;
}
