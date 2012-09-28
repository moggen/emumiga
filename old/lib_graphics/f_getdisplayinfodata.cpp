/*
lib_graphics/f_getdisplayinfodata.c - graphics.library
	GetDisplayInfoData function

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
#include <graphics/displayinfo.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../hook.h"
#include "../emulator.h"
#include "s_displayinfohandle.h"
#include "s_displayinfo.h"

/*
#include "s_dimensioninfo.h"
#include "s_monitorinfo.h"
#include "s_nameinfo.h"
*/

/****************************
 Function: GetDisplayInfoData
   Vector: 126
   Offset: -756
Arguments: A0.L pointer to a DisplayInfoHandle
           A1.L pointer to buffer to be filled in
           D0.L size of the buffer
           D1.L TagID, type of data to get
           D2.L DisplayID, used if A0 is NULL
  Returns: D0.L result size
****************************/
int lib_graphics_f_GetDisplayInfoData(emumsg_syscall_t *msg)
{
        cpu_t *cpu;
	lib_graphics_s_displayinfohandle_t *displayinfohandle;
	uint8_t *buffer_real;
	ULONG tagid, usesize;

	DEBUG(4) dprintf("graphics.library: lib_graphics_f_GetDisplayInfoData() called\n");

        cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  DisplayInfoHandle (A0): 0x%x, Buffer ptr (A1): 0x%x, Buffer size (D0): %d, TagID (D1): 0x%x, DisplayID (D2): 0x%x\n",cpu->a[0],cpu->a[1],cpu->d[0],cpu->d[1],cpu->d[2]);

	if(cpu->a[0] == 0 ) {
		DEBUG(1) dprintf("Error: graphics.library: lib_graphics_f_GetDisplayInfoData: DisplayInfoHandle must be provided.\n");
		return HOOK_END_PROC;
	}

	displayinfohandle = lib_graphics_s_displayinfohandle_get_vaddr(cpu->a[0]);
	if(displayinfohandle == NULL) {
		DEBUG(1) dprintf("Error: graphics.library: lib_graphics_f_GetDisplayInfoData: DisplayInfoHandle pointer is invalid\n");
		return HOOK_END_PROC;
	}

	/* TODO: Check validity of the VADDR memory range */

	tagid = (ULONG)cpu->d[1];
	if( tagid == DTAG_DISP ) {
		/* DisplayInfo */
		if(cpu->d[0] < LIB_GRAPHICS_S_DISPLAYINFO_SIZE) {
			DEBUG(1) dprintf("Error: graphics.library: lib_graphics_f_GetDisplayInfoData: TagID DTAG_DISP: Provided buffer size will not fit the structure. Provided size: %d\n",cpu->d[0]);
			return HOOK_END_PROC;
		}
		usesize = sizeof(struct DisplayInfo);
/*
	} else if( tagid == DTAG_DIMS ) {
	} else if( tagid == DTAG_MNTR ) {
	} else if( tagid == DTAG_NAME ) {
*/
	} else {
		DEBUG(1) dprintf("Error: graphics.library: lib_graphics_f_GetDisplayInfoData: Unsupported tagID: 0x%x\n",tagid);
		return HOOK_END_PROC;
	}

	/* Allocate a buffer in real space for the data, it will later be
	   mapped into VMEM */
	buffer_real = (uint8_t *)AllocVec(usesize, MEMF_PUBLIC|MEMF_CLEAR);
	if(buffer_real == NULL) {
		DEBUG(1) dprintf("Error: graphics.library: lib_graphics_f_GetDisplayInfoData: TagID DTAG_DISP: Could not allocate memory for buffer\n");
		return HOOK_END_PROC;
	}

	/* Prepare syscall */
	msg->arg[0]._aptr = (APTR)displayinfohandle->real;
	msg->arg[1]._aptr = buffer_real;
	msg->arg[2]._ulong = usesize;
	msg->arg[3]._ulong = tagid;
	msg->arg[4]._ulong = 0;

	/* Remember the buffer */
	msg->internalPtr = buffer_real;

	/* Do syscall */
	return HOOK_SYSCALL;
}

int lib_graphics_f_GetDisplayInfoData_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */
	msg->arg[0]._ulong = GetDisplayInfoData(
		(DisplayInfoHandle)msg->arg[0]._aptr,
		(UBYTE *)msg->arg[1]._aptr,
		msg->arg[2]._ulong,
		msg->arg[3]._ulong,
		msg->arg[4]._ulong
	);

	return HOOK_DONE;
}

int lib_graphics_f_GetDisplayInfoData_3(emumsg_syscall_t *msg)
{
	cpu_t *cpu;
	uint8_t *buffer_real;
	uint32_t result, useresult;
	ULONG tagid;
	lib_graphics_s_displayinfo_t *displayinfo;
	struct DisplayInfo *displayinfo_real;
/*
	lib_graphics_s_dimensioninfo_t *dimensioninfo;
	lib_graphics_s_monitorinfo_t *monitorinfo;
	lib_graphics_s_nameinfo_t *nameinfo;
*/
	DEBUG(4) dprintf("graphics.library: lib_graphics_f_GetDisplayInfoData_3() called\n");

	cpu = msg->proc->cpu;

	/* Recall the buffer */
	buffer_real = (uint8_t *)msg->internalPtr;

	/* Extract return values */
	result = (uint32_t)msg->arg[0]._ulong;

	DEBUG(5) dprintf("  Result size (native): %d\n",result);

	if(result == 0) {
		DEBUG(5) dprintf("  Result size (D0): %d\n",0);
		FreeVec(buffer_real);
		return HOOK_DONE;
	}

	/* Different mappings depending on the tagID */
	tagid = (ULONG)cpu->d[1];
	if( tagid == DTAG_DISP ) {
		/* DisplayInfo */

		/* Cast the buffer to the right type */
		displayinfo_real = (struct DisplayInfo *)buffer_real;

		/* Check the returned size */
		if(result < sizeof(struct DisplayInfo)) {
			DEBUG(1) dprintf("Error: graphics.library: lib_graphics_f_GetDisplayInfoData_3: TagID DTAG_DISP: returned size is too small\n");
			FreeVec(buffer_real);
			return HOOK_END_PROC;
		}

		/* Get a user space mapping */
		displayinfo = lib_graphics_s_displayinfo_get_real_in_user(displayinfo_real, cpu->a[1]);
		if(displayinfo == NULL) {
			DEBUG(1) dprintf("Error: graphics.library: lib_graphics_f_GetDisplayInfoData_3: Could not create mapping for DisplayInfo\n");
			FreeVec(buffer_real);
			return HOOK_END_PROC;
		}

		useresult = LIB_GRAPHICS_S_DISPLAYINFO_SIZE;
/*
	} else if( tagid == DTAG_DIMS ) {
	} else if( tagid == DTAG_MNTR ) {
	} else if( tagid == DTAG_NAME ) {
*/
	}

	cpu->d[0] = useresult;

	DEBUG(5) dprintf("  Result size (D0): %d\n", cpu->d[0]);

	return HOOK_DONE;
}
