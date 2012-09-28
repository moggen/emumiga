/*
lib_exec/f_opendevice.c - exec.library OpenDevice function

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
#include "../dev_timer/timer.h"
#include "../dev_timer/s_timerequest.h"
#include "../dev_audio/audio.h"
#include "../dev_audio/s_ioaudio.h"

/****************************
 Function: OpenDevice
   Vector: 74
   Offset: -444
Arguments: A0.L pointer to device name string
           D0.L unit number
           A1.L pointer to IORequest structure
           D1.L flags
  Returns: D0.L error code or 0 if ok
****************************/
int lib_exec_f_OpenDevice(emumsg_syscall_t *msg)
{
	cpu_t *cpu;
	uint32_t vaddr;
	char ch, devname[300], *cur, *nameptr;
	uint8_t tmp8;
	int rc;
	lib_exec_s_iorequest_t *iorequest;

	DEBUG(4) dprintf("exec.library: lib_exec_OpenDevice() called\n");

	cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  Name of device (A0): 0x%x, Unit number (D0): %d, IORequest ptr (A1): 0x%x, Flags (D1): 0x%x\n",cpu->a[0],cpu->d[0],cpu->a[1],cpu->d[1]);

	if(cpu->a[0] == 0 || cpu->a[1] == 0) {
		cpu->d[0] = 116; /* ERROR_REQUIRED_ARG_MISSING */
		return HOOK_DONE;
	}

	cur = devname;
	vaddr = cpu->a[0];
	do {
		rc = READMEM_8(vaddr++, &tmp8);
		if(rc) {
			dprintf("Error: exec.library: lib_exec_OpenDevice: Device name is not readable\n");
			return HOOK_END_PROC;
		}
		ch = (char)tmp8;;
		*cur++ = ch;
	} while(ch != '\0');

	DEBUG(5) dprintf("Device name: %s\n",devname);

	/* Copy name and remember it for later */
	nameptr = (char *)AllocVec(strlen(devname)+1, MEMF_PUBLIC|MEMF_CLEAR);
	if(nameptr == NULL) {
		dprintf("Error: exec.library: lib_exec_OpenDevice: Could not allocate memory for name\n");
		return HOOK_END_PROC;
	}
	strcpy(nameptr, devname);
	msg->internalPtr = (void *)nameptr;


	/* Get IORequest mapping */
	iorequest = lib_exec_s_iorequest_get_vaddr(cpu->a[1]);
	if(iorequest == NULL) {
		DEBUG(1) dprintf("Error: exec.library: lib_exec_OpenDevice: IORequest struct pointer is invalid\n");
		FreeVec(nameptr);
		return HOOK_END_PROC;
	}

	/* Check the name */
	if( strcmp("timer.device",nameptr ) == 0 ) {
		rc = dev_timer_s_timerequest_convert(iorequest);
		if(rc) {
			DEBUG(1) dprintf("Error: exec.library: lib_exec_OpenDevice: Conversion of IORequest to timerequest failed!\n");
			FreeVec(nameptr);
			return HOOK_END_PROC;
		}
	} else if( strcmp("audio.device",nameptr ) == 0 ) {
		rc = dev_audio_s_ioaudio_convert(iorequest);
		if(rc) {
			DEBUG(1) dprintf("Error: exec.library: lib_exec_OpenDevice: Conversion of IORequest to IOAudio failed!\n");
			FreeVec(nameptr);
			return HOOK_END_PROC;
		}
	} else {
		/* Device is not supported */
		DEBUG(2) dprintf("Warning: exec.library: lib_exec_f_OpenDevice: Device \"%s\" is not yet emulated.\n",devname);
		cpu->d[0] = 205; /* ERROR_OBJECT_NOT_FOUND */
		FreeVec(nameptr);
		return HOOK_DONE;
	}

	/* Prepare syscall */
	msg->arg[0]._strptr = (STRPTR)nameptr;
	msg->arg[1]._ulong = cpu->d[0];
	msg->arg[2]._aptr = iorequest->real;
	msg->arg[3]._ulong = cpu->d[1];

	DEBUG(5) dprintf("  Name of device (native): %p, IORequest ptr (native): %p\n",nameptr,iorequest->real);

        /* Do syscall */
        return HOOK_SYSCALL;
}

int lib_exec_f_OpenDevice_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */
	msg->arg[0]._byte = OpenDevice(msg->arg[0]._strptr,msg->arg[1]._ulong,(struct IORequest *)msg->arg[2]._aptr,msg->arg[3]._ulong);

	return HOOK_DONE;
}

int lib_exec_f_OpenDevice_3(emumsg_syscall_t *msg)
{
        cpu_t *cpu;
	uint32_t retcode;
	char *nameptr;
	struct IORequest *iorequest_real;

        DEBUG(4) dprintf("exec.library: lib_exec_f_OpenDevice_3() called\n");

	cpu = msg->proc->cpu;

	retcode = (uint32_t)(int32_t)msg->arg[0]._byte;

	DEBUG(5) dprintf("  Error code (D0): 0x%x\n", retcode);

	cpu->d[0] = retcode;

	/* Recall name pointer */
	nameptr = (char *)msg->internalPtr;

	/* Get IORequest */
	iorequest_real = (struct IORequest *)msg->arg[2]._aptr;

	/* Set up the base pointer for the device */
	if( strcmp("timer.device",nameptr ) == 0 ) {
		TimerBase = iorequest_real->io_Device;
	} else if( strcmp("audio.device",nameptr ) == 0 ) {
		AudioBase = iorequest_real->io_Device;
	}

	FreeVec(nameptr);

	return HOOK_DONE;
}
