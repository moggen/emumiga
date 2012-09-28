/*
lib_intuition/f_refreshglist.c - intuition.library RefreshGList function

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
#include <proto/intuition.h>
#include <intuition/intuition.h>
#include <utility/tagitem.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../hook.h"
#include "../emulator.h"
#include "s_window.h"
#include "s_gadget.h"
#include "s_requester.h"

/****************************
 Function: RefreshGList
   Vector: 72
   Offset: -432
Arguments: A0.L Pointer to Gadget
           A1.L Pointer to Window
           A2.L Pointer to Requester
           D0.L Maximum number
  Returns: -
****************************/
int lib_intuition_f_RefreshGList(emumsg_syscall_t *msg)
{
        cpu_t *cpu;

	lib_intuition_s_gadget_t *gadget;
	lib_intuition_s_window_t *window;
	lib_intuition_s_requester_t *requester;
	struct Gadget *gadget_real;
	struct Window *window_real;
	struct Requester *requester_real;

	DEBUG(4) dprintf("intuition.library: lib_intuition_f_RefreshGList() called\n");

        cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  Gadget ptr (A0): 0x%x, Window ptr (A1): 0x%x, Requester ptr (A2): 0x%x, Maximum number (D0): %d\n",cpu->a[0],cpu->a[1],cpu->a[2],cpu->d[0]);

	gadget_real = NULL;
        if(cpu->a[0] != 0) {
		gadget = lib_intuition_s_gadget_get_vaddr(cpu->a[0]);
		if(gadget == NULL) {
			DEBUG(1) dprintf("Error: intuition.library: lib_intuition_f_RefreshGList: Gadget pointer is invalid\n");
			return HOOK_END_PROC;
		}
		gadget_real = gadget->real;
	}

	window_real = NULL;
        if(cpu->a[1] != 0) {
		window = lib_intuition_s_window_get_vaddr(cpu->a[1]);
		if(window == NULL) {
			DEBUG(1) dprintf("Error: intuition.library: lib_intuition_f_RefreshGList: Window pointer is invalid\n");
			return HOOK_END_PROC;
		}
		window_real = window->real;
	}

	requester_real = NULL;
        if(cpu->a[2] != 0) {
		requester = lib_intuition_s_requester_get_vaddr(cpu->a[2]);
		if(requester == NULL) {
			DEBUG(1) dprintf("Error: intuition.library: lib_intuition_f_RefreshGList: Requester pointer is invalid\n");
			return HOOK_END_PROC;
		}
		requester_real = requester->real;
	}

	/* Prepare syscall */

	msg->arg[0]._aptr = (APTR)gadget_real;
	msg->arg[1]._aptr = (APTR)window_real;
	msg->arg[2]._aptr = (APTR)requester_real;
	msg->arg[3]._long = (LONG)cpu->d[0];

	DEBUG(5) dprintf("  Gadget ptr (native): %p, Window ptr (native): %p, Requester ptr (native): %p\n",gadget_real,window_real,requester_real); 

	/* Do syscall */
	return HOOK_SYSCALL;
}

int lib_intuition_f_RefreshGList_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */
	RefreshGList(
		(struct Gadget *)msg->arg[0]._aptr,
		(struct Window *)msg->arg[1]._aptr,
		(struct Requester *)msg->arg[2]._aptr,
		msg->arg[3]._long
	);

	return HOOK_DONE;
}

int lib_intuition_f_RefreshGList_3(emumsg_syscall_t *msg __attribute__((unused)))
{
	DEBUG(4) dprintf("intuition.library: lib_intuition_f_RefreshGList_3() called\n");

	return HOOK_DONE;
}
