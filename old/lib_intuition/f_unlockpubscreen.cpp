/*
lib_intuition/f_unlockpubscreen.c - intuition.library UnlockPubScreen function

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

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../hook.h"
#include "../emulator.h"
#include "s_screen.h"

/****************************
 Function: UnlockPubScreen
   Vector: 86
   Offset: -516
Arguments: A0.L screen name string
           A1.L pointer to struct Screen
  Returns: -
****************************/
int lib_intuition_f_UnlockPubScreen(emumsg_syscall_t *msg)
{
        cpu_t *cpu;
	uint8_t *string_real;
	lib_intuition_s_screen_t *screen;
	struct Screen *screen_real;

	DEBUG(4) dprintf("intuition.library: lib_intuition_f_UnlockPubScreen() called\n");

        cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  Screen name (A0): 0x%x, Screen ptr (A1): 0x%x\n",cpu->a[0],cpu->a[1]);

	string_real = NULL;
	if(cpu->a[0] != 0) {
	        string_real = vptr(cpu->a[0]);
	        if(string_real == NULL) {
	                DEBUG(1) dprintf("Error: intuition.library: lib_intuition_f_UnlockPubScreen: Screen name string pointer is invalid\n");
	                return HOOK_END_PROC;
	        }
	}

	screen_real = NULL;
	if(cpu->a[1] != 0) {
		screen = lib_intuition_s_screen_get_vaddr(cpu->a[1]);
		if(screen == NULL) {
	                DEBUG(1) dprintf("Error: intuition.library: lib_intuition_f_UnlockPubScreen: Screen pointer is invalid\n");
			return HOOK_END_PROC;
		}
		screen_real = screen->real;
	}

	/* Prepare syscall */
	msg->arg[0]._aptr = (APTR)string_real;
	msg->arg[1]._aptr = (APTR)screen_real;

	DEBUG(5) dprintf("  Screen name (native): %p, Screen ptr (native): %p\n",string_real,screen_real);

	/* Do syscall */
	return HOOK_SYSCALL;
}

int lib_intuition_f_UnlockPubScreen_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */
	UnlockPubScreen(
		(UBYTE *)msg->arg[0]._aptr,
		(struct Screen *)msg->arg[1]._aptr
	);

	return HOOK_DONE;
}

int lib_intuition_f_UnlockPubScreen_3(emumsg_syscall_t *msg __attribute__((unused)))
{
	DEBUG(4) dprintf("intuition.library: lib_intuition_f_UnlockPubScreen_3() called\n");
	return HOOK_DONE;
}
