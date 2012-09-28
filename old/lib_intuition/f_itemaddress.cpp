/*
lib_intuition/f_itemaddress.c - intuition.library ItemAddress
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
#include "s_menu.h"
#include "s_menuitem.h"

/****************************
 Function: ItemAddress
   Vector: 24
   Offset: -144
Arguments: A0.L Pointer to Menu struct
           D0.W Menu number
  Returns: D0.L pointer to a MenuItem or NULL
****************************/
int lib_intuition_f_ItemAddress(emumsg_syscall_t *msg)
{
        cpu_t *cpu;
	lib_intuition_s_menu_t *menu;

	DEBUG(4) dprintf("intuition.library: lib_intuition_f_ItemAddress() called\n");

        cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  Menu ptr (A0): 0x%x, Menu number (D0): 0x%x\n",cpu->a[0],cpu->d[0]);

	if((cpu->d[0] & 0xffff) == 0xffff) {
		/* MENUNULL */
		cpu->d[0] = 0;
		DEBUG(5) dprintf("  MenuItem ptr (D0): 0x%x\n",0);
		return HOOK_DONE;
	}

        if(cpu->a[0] == 0) {
                DEBUG(1) dprintf("Error: intuition.library: lib_intuition_f_ItemAddress: Menu pointer is null\n");
		return HOOK_END_PROC;
	}

	menu = lib_intuition_s_menu_get_vaddr(cpu->a[0]);
        if(menu == NULL) {
                DEBUG(1) dprintf("Error: intuition.library: lib_intuition_f_ItemAddress: Menu pointer is invalid\n");
                return HOOK_END_PROC;
        }

	/* Prepare syscall */
	msg->arg[0]._aptr = (APTR)menu->real;
	msg->arg[1]._uword = (UWORD)(cpu->d[0] & 0xffff);

	DEBUG(5) dprintf("  Menu ptr (native): %p\n",menu->real);

	/* Do syscall */
	return HOOK_SYSCALL;
}

int lib_intuition_f_ItemAddress_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */
	msg->arg[0]._aptr = ItemAddress((struct Menu *)msg->arg[0]._aptr, msg->arg[1]._uword);

	return HOOK_DONE;
}

int lib_intuition_f_ItemAddress_3(emumsg_syscall_t *msg)
{
	cpu_t *cpu;
	struct MenuItem *menuitem_real;
	lib_intuition_s_menuitem_t *menuitem;

	DEBUG(4) dprintf("intuition.library: lib_intuition_f_ItemAddress_3() called\n");

	cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  MenuItem ptr (native): %p\n", msg->arg[0]._aptr);

	/* Extract return values */
	menuitem_real = (struct MenuItem *)msg->arg[0]._aptr;

	if(menuitem_real == NULL) {
		cpu->d[0] = 0;
		DEBUG(5) dprintf("  MenuItem ptr (D0): 0x%x\n",0);
		return HOOK_DONE;
	}

	menuitem = lib_intuition_s_menuitem_get_real(menuitem_real);
	if(menuitem == NULL) {
		DEBUG(1) dprintf("Error: intuition.library: lib_intuition_f_ItemAddress_3: Could not create mapping for MenuItem\n");
		return HOOK_END_PROC;
	}

	cpu->d[0] = menuitem->entry->startAddr;

	DEBUG(5) dprintf("  MenuItem ptr (D0): 0x%x\n", cpu->d[0]);

	return HOOK_DONE;
}
