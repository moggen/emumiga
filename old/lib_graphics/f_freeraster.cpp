/*
lib_graphics/f_freeraster.c - graphics.library
	FreeRaster function

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
 Function: FreeRaster
   Vector: 82
   Offset: -492
Arguments: A0.L ptr to raster
           D0.L width
           D1.L height
  Returns: -
****************************/
int lib_graphics_f_FreeRaster(emumsg_syscall_t *msg)
{
        cpu_t *cpu;
	uint32_t vaddr;
	mmu_entry_t *entry;
	APTR real;

	DEBUG(4) dprintf("graphics.library: lib_graphics_f_FreeRaster() called\n");

        cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  Raster ptr (A0): 0x%x, Width (D0): %d, Height (D1): %d\n",cpu->a[0],cpu->d[0],cpu->d[1]);

	vaddr = cpu->a[0];
	if(vaddr == 0) {
		DEBUG(2) dprintf("Warning: graphics.library: lib_graphics_f_FreeRaster: Attempt to free NULL pointer ignored\n");
		return HOOK_DONE;
	}

	/* Find the memory block */
	entry = mmu_findEntryWithType(vaddr, MMU_TYPE_VALLOCMEM_RAM);
	if(entry == NULL) {
		DEBUG(1) dprintf("Error: graphics.library: lib_graphics_f_FreeRaster: Could not find a memory block\n");
		return HOOK_END_PROC;
	}
	real = (APTR)entry->nativePtr;

	/* Free vmem range */
	vfreemem_ram(vaddr);

	/* Prepare syscall */
	msg->arg[0]._aptr = real;
	msg->arg[1]._ulong = cpu->d[0];
	msg->arg[2]._ulong = cpu->d[1];


	/* Do syscall */
	return HOOK_SYSCALL;
}

int lib_graphics_f_FreeRaster_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */
	FreeRaster(
		(PLANEPTR)msg->arg[0]._aptr,
		msg->arg[1]._ulong,
		msg->arg[2]._ulong
	);

	return HOOK_DONE;
}

int lib_graphics_f_FreeRaster_3(emumsg_syscall_t *msg)
{
	DEBUG(4) dprintf("graphics.library: lib_graphics_f_FreeRaster_3() called\n");

	return HOOK_DONE;
}
