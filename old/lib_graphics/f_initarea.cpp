/*
lib_graphics/f_initarea.c - graphics.library
	InitArea function

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

#include <proto/exec.h>
#include <proto/graphics.h>
#include <graphics/rastport.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../hook.h"
#include "../emulator.h"
#include "s_areainfo.h"

/****************************
 Function: InitArea
   Vector: 47
   Offset: -282
Arguments: A0.L pointer to struct AreaInfo
           A1.L pointer to buffer of vectors
           D0.L max number of vectors
  Returns: -
****************************/
int lib_graphics_f_InitArea(emumsg_syscall_t *msg)
{
        cpu_t *cpu;
	lib_graphics_s_areainfo_t *areainfo;
	uint32_t maxvectors;
	uint8_t *buffer_real;

	DEBUG(4) dprintf("graphics.library: lib_graphics_f_InitArea() called\n");

        cpu = msg->proc->cpu;

	maxvectors = cpu->d[0];

	DEBUG(5) dprintf("  AreaInfo ptr (A0): 0x%x, Buffer ptr (A1): 0x%x, Number of vectors (D0.W): %d\n",cpu->a[0],cpu->a[1],maxvectors);

	if(cpu->a[0] == 0 ) {
		DEBUG(1) dprintf("Error: graphics.library: lib_graphics_f_InitArea: AreaInfo must be provided.\n");
		return HOOK_END_PROC;
	}

	areainfo = lib_graphics_s_areainfo_get_vaddr_noconvert(cpu->a[0]);
	if(areainfo == NULL) {
		DEBUG(1) dprintf("Error: graphics.library: lib_graphics_f_InitArea: AreaInfo pointer is invalid\n");
		return HOOK_END_PROC;
	}

	/* Check if we already has set up a buffer for this structure */
	if(areainfo->buffer != NULL) {
		/* Free it first */
		FreeVec(areainfo->buffer);
		areainfo->buffer = NULL;
	}

	/* Alloc mem for buffer */
	buffer_real = (uint8_t *)AllocVec(maxvectors*5, MEMF_PUBLIC|MEMF_CLEAR);
	if(buffer_real == NULL) {
		DEBUG(1) dprintf("Error: graphics.library: lib_graphics_f_InitArea: Could not allocate memory for buffer\n");
		return HOOK_END_PROC;
	}

	/* Remember it */
	areainfo->buffer = buffer_real;

	/* Prepare syscall */
	msg->arg[0]._aptr = (APTR)areainfo->real;
	msg->arg[1]._aptr = buffer_real;
	msg->arg[2]._long = (LONG)maxvectors;

	/* Do syscall */
	return HOOK_SYSCALL;
}

int lib_graphics_f_InitArea_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */
	InitArea(
		(struct AreaInfo *)msg->arg[0]._aptr,
		msg->arg[1]._aptr,
		msg->arg[2]._long
	);

	return HOOK_DONE;
}

int lib_graphics_f_InitArea_3(emumsg_syscall_t *msg __attribute__((unused)))
{

	DEBUG(4) dprintf("graphics.library: lib_graphics_f_InitArea_3() called\n");

	return HOOK_DONE;
}
