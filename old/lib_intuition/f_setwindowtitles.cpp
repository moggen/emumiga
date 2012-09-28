/*
lib_intuition/f_setwindowtitles.c - intuition.library SetWindowTitles
	function

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

#include <proto/intuition.h>
#include <intuition/intuition.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../hook.h"
#include "../emulator.h"
#include "s_window.h"

/****************************
 Function: SetWindowTitles
   Vector: 46
   Offset: -276
Arguments: A0.L Pointer to Window struct
           A1.L Pointer to window title string
           A2.L Pointer to screen title string
  Returns: -
****************************/
int lib_intuition_f_SetWindowTitles(emumsg_syscall_t *msg)
{
        cpu_t *cpu;
	lib_intuition_s_window_t *window;
	STRPTR wstr, sstr;

	DEBUG(4) dprintf("intuition.library: lib_intuition_f_SetWindowTitles() called\n");

        cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  Window ptr (A0): 0x%x, Window title ptr (A1): 0x%x, Screen title ptr (A2): 0x%x\n",cpu->a[0],cpu->a[1],cpu->a[2]);

	/* Window ptr */
	window = lib_intuition_s_window_get_vaddr(cpu->a[0]);
        if(window == NULL) {
		DEBUG(1) dprintf("Error: intuition.library: lib_intuition_f_SetWindowTitles: Window pointer is invalid\n");
		return HOOK_END_PROC;
        }

	/* Window title */
	if(cpu->a[1] == 0) {
		wstr = NULL;
	} else if(cpu->a[1] == 0xffffffff) {
		wstr = (STRPTR)-1;
	} else {
		wstr = (STRPTR)vptr(cpu->a[1]);
	}

	/* Screen title */
	if(cpu->a[2] == 0) {
		sstr = NULL;
	} else if(cpu->a[2] == 0xffffffff) {
		sstr = (STRPTR)-1;
	} else {
		sstr = (STRPTR)vptr(cpu->a[2]);
	}

	/* TODO: Safer string handling */

	/* Prepare syscall */
	msg->arg[0]._aptr = (APTR)window->real;
	msg->arg[1]._strptr = wstr;
	msg->arg[2]._strptr = sstr;

	DEBUG(5) dprintf("  Window ptr (native): %p, Window title ptr (native): %p, Screen title ptr (native): %p\n",window->real,wstr,sstr);

	/* Do syscall */
	return HOOK_SYSCALL;
}

int lib_intuition_f_SetWindowTitles_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */
	SetWindowTitles(
		(struct Window *)msg->arg[0]._aptr,
		msg->arg[1]._strptr,
		msg->arg[2]._strptr
	);

	return HOOK_DONE;
}

int lib_intuition_f_SetWindowTitles_3(emumsg_syscall_t *msg __attribute__((unused)))
{
	DEBUG(4) dprintf("intuition.library: lib_intuition_f_SetWindowTitles_3() called\n");

	return HOOK_DONE;
}
