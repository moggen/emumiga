/*
lib_locale/f_formatdate.c - locale.library FormatDate function

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

#include <proto/locale.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../hook.h"
#include "../emulator.h"

#include "s_locale.h"
#include "../lib_utility/s_hook.h"
#include "../lib_dos/s_datestamp.h"

/* Init flag */
static int is_init = 0;

/* Helper data */
static uint8_t helper_data[6];
static uint32_t helper_vaddr;

/* Global init */
int lib_locale_f_formatdate_init(uint16_t mnum)
{
        DEBUG(4) dprintf("locale.library: lib_locale_f_formatdate_init() called\n");

        /* Set up helper code */
        helper_data[0] = 0xFD;
        helper_data[1] = 0x80;
        helper_data[2] = (uint8_t)((mnum >> 8) & 0xff);
        helper_data[3] = (uint8_t)((mnum) & 0xff);
        helper_data[4] = 0xff;
        helper_data[5] = (uint8_t)-10;

        helper_vaddr = vallocmem_rom(helper_data,6,"!locale.helper.FormatDate");
        if(helper_vaddr == 0) {
                dprintf("locale.library: lib_locale_f_formatdate_init: vallocmem_rom() failed\n");
                return 1;
        }

        is_init = 1;
        return 0;
}

void lib_locale_f_formatdate_cleanup()
{
        DEBUG(4) dprintf("locale.library: lib_locale_f_formatdate_cleanup() called\n");

        if( !is_init ) {
                DEBUG(2) dprintf("locale.library: lib_locale_f_formatdate_cleanup: Not initialized\n");
                return;
        }

        /* Delete mapping for helper */
        vfreemem_rom(helper_vaddr);

        is_init = 0;
}

/****************************
 Function: FormatDate
   Vector: 10
   Offset: -60
Arguments: A0.L pointer to struct Locale
           A1.L pointer to format string
           A2.L pointer to struct DateStamp
           A3.L pointer to struct Hook
  Returns: -
  Comment: Issues callbacks via the hook structure. Parameters:
           A0.L pointer to struct Hook
           A1.B the emitted character
           A2.L pointer to struct Locale
           The function is called with the NULL char ('\0') at the end
****************************/
int lib_locale_f_FormatDate(emumsg_syscall_t *msg)
{
	cpu_t *cpu;
	lib_locale_s_locale_t *locale;
	struct Locale *locale_real;
	lib_dos_s_datestamp_t *datestamp;
	lib_utility_s_hook_t *hook;
	STRPTR fstr;

	DEBUG(4) dprintf("locale.library: lib_locale_f_FormatDate() called\n");

	cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  Locale ptr (A0): 0x%x, Format string ptr (A1): 0x%x, DateStamp ptr (A2): 0x%x, Hook ptr (A3): 0x%x\n",cpu->a[0],cpu->a[1],cpu->a[2],cpu->a[3]);

	if(cpu->a[2] == 0) {
		DEBUG(2) dprintf("Warning: locale.library: lib_locale_f_FormatDate: Pointer to struct DateStamp is NULL.\n");
		return HOOK_DONE;
	}
	if(cpu->a[3] == 0) {
		DEBUG(2) dprintf("Warning: locale.library: lib_locale_f_FormatDate: Pointer to struct Hook is NULL.\n");
		return HOOK_DONE;
	}

	/* Can be null */
	locale_real = NULL;
	locale = lib_locale_s_locale_get_vaddr(cpu->a[0]);
	if(locale != NULL) {
		locale_real = locale->real;
	}

	/* Can be null */
	fstr = (STRPTR)vptr(cpu->a[1]);
	/* TODO: Implement some kind of check for MMU entry ending within
	         the string
	*/

	datestamp = lib_dos_s_datestamp_get_vaddr(cpu->a[2]);
	if(datestamp == NULL) {
		DEBUG(1) dprintf("Error: locale.library: lib_locale_f_FormatDate: DateStamp pointer is invalid\n");
		return HOOK_END_PROC;
	}

	hook = lib_utility_s_hook_get_vaddr(cpu->a[3]);
	if(hook == NULL) {
		DEBUG(1) dprintf("Error: locale.library: lib_locale_f_FormatDate: Hook pointer is invalid\n");
		return HOOK_END_PROC;
	}

	DEBUG(5) dprintf("  Locale ptr (native): %p, Format string ptr (native): %p, DateStamp ptr (native): %p, Hook ptr (native): %p\n",locale_real,fstr,datestamp->real,hook->real);

	/* Prepare the CPU structure for callback */

	/* Save PC */
	cpu->a[7] -= 4;
	if(WRITEMEM_32(cpu->pc, cpu->a[7])){
		DEBUG(1) dprintf("Error: locale.library: lib_locale_f_FormatDate: Could not write to stack\n");
		return HOOK_END_PROC;
	}

	/* Set return address for RTS in the end of the callback
           so it executes the special TRAP */
	cpu->a[7] -= 4;
	if(WRITEMEM_32(helper_vaddr, cpu->a[7])){
		DEBUG(1) dprintf("Error: locale.library: lib_locale_f_FormatDate: Could not write to stack\n");
		return HOOK_END_PROC;
	}

	/* Put the msg into the hook so it can be reached from within
	   the callback */
	hook->msg = msg;

	/* Prepare syscall */
	msg->arg[0]._aptr = (APTR)locale_real;
	msg->arg[1]._strptr = fstr;
	msg->arg[2]._aptr = (APTR)datestamp->real;
	msg->arg[3]._aptr = (APTR)hook->real;

	return HOOK_SYSCALL;
}


int lib_locale_f_FormatDate_2(emumsg_syscall_t *msg)
{
	/* Switch to callback vector. Any callbacks will reply to the msg
	   and it will call stage 3 of the callback instead of stage 3
	   of this call. */

	msg->func = -10;

	/* Make real syscall */
	FormatDate(
		(struct Locale *)msg->arg[0]._aptr,
		msg->arg[1]._strptr,
		(struct DateStamp *)msg->arg[2]._aptr,
		(struct Hook *)msg->arg[3]._aptr
	);

	/* Switch to normal vector. All callbacks are done and now we
	   want to go to stage 3 of this function. */
	msg->func = 10;

	return HOOK_DONE;
}

int lib_locale_f_FormatDate_3(emumsg_syscall_t *msg)
{
	cpu_t *cpu;

	DEBUG(4) dprintf("locale.library: lib_locale_f_FormatDate_3() called\n");

	cpu = msg->proc->cpu;

	/* Restore the CPU structure */

	/* Return address for RTS */
	cpu->a[7] += 4;

	/* Restore original PC */
	READMEM_32(cpu->a[7], &(cpu->pc));
	cpu->a[7] += 4;

	return HOOK_DONE;
}

int lib_locale_f_FormatDate_cb(emumsg_syscall_t *msg)
{
	/* This syscall is triggered when the special trap is
	   hit. This means that we are returning from the
	   callback. */

	cpu_t *cpu;

        DEBUG(4) dprintf("exec.library: lib_locale_f_FormatDate_cb() called\n");

	cpu = msg->proc->cpu;

	/* The callback just run RTS. Restore the SP so
	   the callback can be used again */
	cpu->a[7] -= 4;

        /* Pass on to native process */
        return HOOK_SYSCALL;
}

int lib_locale_f_FormatDate_cb_2(emumsg_syscall_t *msg)
{
        /* Special signal that will end the emulator_doEvents()
	   loop inside the real AROS callback code. This will
	   end the callback function and FormatDate will
	   continue, possibly triggering another callback */

        return HOOK_CALLBACK;
}

int lib_locale_f_FormatDate_cb_3(emumsg_syscall_t *msg)
{
	/* The reply message of a newly entered callback will
	   end up here. The argument collection of the msg
	   contains the callback arguments. */

	cpu_t *cpu;
	lib_utility_s_hook_t *hook;
	lib_locale_s_locale_t *locale;

        DEBUG(4) dprintf("exec.library: lib_locale_f_FormatDate_cb_3() called\n");

	DEBUG(5) dprintf("  Hook ptr (native): %p, Locale ptr (native): %p\n",msg->arg[0]._aptr,msg->arg[2]._aptr);

	cpu = msg->proc->cpu;

	/* Resolve hook ptr */
	hook = lib_utility_s_hook_get_real((struct Hook *)msg->arg[0]._aptr);
	if(hook == NULL) {
		DEBUG(1) dprintf("Error: locale.library: lib_locale_f_FormatDate_cb_3: Hook pointer is invalid\n");
		return HOOK_END_PROC;
	}
	cpu->a[0] = hook->entry->startAddr;

	/* "message" is actually the char */
	cpu->a[1] = (uint32_t)msg->arg[1]._aptr;

	/* "object" is a pointer to the Locale struct, can be NULL */
	if(msg->arg[2]._aptr == NULL) {
		cpu->a[2] = 0;
	} else {
		locale = lib_locale_s_locale_get_real((struct Locale *)msg->arg[2]._aptr);
	}

	/* Set up the PC so it will call the emulated callback */
	cpu->pc = hook->entry_vaddr;

	DEBUG(5) dprintf("  Hook ptr (A0): 0x%x, Char (A1): 0x%x, Locale ptr (A2): 0x%x\n",cpu->a[0],cpu->a[1],cpu->a[2]);

	/* Everything is set up in the CPU so we will simply
	   continue emulating. */
        return HOOK_DONE;
}
