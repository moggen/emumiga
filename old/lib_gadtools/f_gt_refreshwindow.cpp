/*
lib_gadtools/f_gt_refreshwindow.c - gadtools.library GT_RefreshWindow function

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
#include <proto/gadtools.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../hook.h"
#include "../emulator.h"
#include "../lib_intuition/s_window.h"
#include "../lib_intuition/s_requester.h"

/****************************
 Function: GT_RefreshWindow
   Vector: 14
   Offset: -84
Arguments: A0.L Pointer to Window
           A1.L Pointer to Requester
  Returns: -
****************************/
int lib_gadtools_f_GT_RefreshWindow(emumsg_syscall_t *msg)
{
        cpu_t *cpu;

	lib_intuition_s_window_t *window;
	lib_intuition_s_requester_t *requester;
	struct Window *window_real;
	struct Requester *requester_real;

	DEBUG(4) dprintf("gadtools.library: lib_gadtools_f_GT_RefreshWindow() called\n");

        cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  Window ptr (A0): 0x%x, Requester ptr (A1): 0x%x\n",cpu->a[0],cpu->a[1]);

	window_real = NULL;
        if(cpu->a[0] != 0) {
		window = lib_intuition_s_window_get_vaddr(cpu->a[0]);
		if(window == NULL) {
			DEBUG(1) dprintf("Error: gadtools.library: lib_gadtools_f_GT_RefreshWindow: Window pointer is invalid\n");
			return HOOK_END_PROC;
		}
		window_real = window->real;
	}

	requester_real = NULL;
        if(cpu->a[1] != 0) {
		requester = lib_intuition_s_requester_get_vaddr(cpu->a[1]);
		if(requester == NULL) {
			DEBUG(1) dprintf("Error: gadtools.library: lib_gadtools_f_GT_RefreshWindow: Requester pointer is invalid\n");
			return HOOK_END_PROC;
		}
		requester_real = requester->real;
	}

	/* Prepare syscall */

	msg->arg[0]._aptr = (APTR)window_real;
	msg->arg[1]._aptr = (APTR)requester_real;

	DEBUG(5) dprintf("  Window ptr (native): %p, Requester ptr (native): %p\n",window_real,requester_real); 

	/* Do syscall */
	return HOOK_SYSCALL;
}

int lib_gadtools_f_GT_RefreshWindow_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */
	GT_RefreshWindow(
		(struct Window *)msg->arg[1]._aptr,
		(struct Requester *)msg->arg[2]._aptr
	);

	return HOOK_DONE;
}

int lib_gadtools_f_GT_RefreshWindow_3(emumsg_syscall_t *msg)
{
	DEBUG(4) dprintf("gadtools.library: lib_gadtools_f_GT_RefreshWindow_3() called\n");

	return HOOK_DONE;
}
