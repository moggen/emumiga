/*
lib_gadtools/f_createmenusa.c - gadtools.library CreateMenusA function

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

#include <proto/gadtools.h>
#include <intuition/intuition.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../hook.h"
#include "../emulator.h"
#include "s_newmenu.h"
#include "../lib_intuition/s_menu.h"

/****************************
 Function: CreateMenusA
   Vector: 8
   Offset: -48
Arguments: A0.L pointer to NewMenu array
           A1.L pointer to tag array
  Returns: D0.L pointer to Menu stucture
****************************/
int lib_gadtools_f_CreateMenusA(emumsg_syscall_t *msg)
{
        cpu_t *cpu;
	lib_gadtools_s_newmenu_t *newmenu;
	struct NewMenu *newmenu_real;
	struct TagItem *tags_real;

	DEBUG(4) dprintf("gadtools.library: lib_gadtools_f_CreateMenusA() called\n");

        cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  NewMenu ptr (A0): 0x%x, Tag array (A1): 0x%x\n",cpu->a[0],cpu->a[1]);

	newmenu_real = NULL;
	if(cpu->a[0] == 0) {
		/* Fail softly */
		cpu->d[0] = 0;
		DEBUG(5) dprintf("  Menu ptr (D0): 0x%x\n", cpu->d[0]);
		return HOOK_DONE;
	}

	newmenu = lib_gadtools_s_newmenu_get_vaddr(cpu->a[0]);
        if(newmenu == NULL) {
                DEBUG(1) dprintf("Error: gadtools.library: lib_gadtools_f_CreateMenusA: NewMenu pointer is invalid\n");
                return HOOK_END_PROC;
        }
	newmenu_real = newmenu->real;

	tags_real = NULL;
	if(cpu->a[1] != 0) {
                DEBUG(2) dprintf("Warning: gadtools.library: lib_gadtools_f_CreateMenusA: Tags are provided but is not supported yet\n");
	}

	/* Prepare syscall */
	msg->arg[0]._aptr = (APTR)newmenu_real;
	msg->arg[1]._aptr = (APTR)tags_real;

	DEBUG(5) dprintf("  NewMenu ptr (native): %p, Tag array (native): %p\n",newmenu_real,tags_real);

	/* Do syscall */
	return HOOK_SYSCALL;
}

int lib_gadtools_f_CreateMenusA_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */
	msg->arg[0]._aptr = CreateMenusA((struct NewMenu *)msg->arg[0]._aptr,(struct TagItem *)msg->arg[1]._aptr);

	return HOOK_DONE;
}

int lib_gadtools_f_CreateMenusA_3(emumsg_syscall_t *msg)
{
	cpu_t *cpu;
	lib_intuition_s_menu_t *menu;
	struct Menu *menu_real;

	DEBUG(4) dprintf("gadtools.library: lib_gadtools_f_CreateMenusA_3() called\n");

	cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  Menu ptr (native): %p\n", msg->arg[0]._aptr);

	/* Extract return values */
	menu_real = (struct Menu *)msg->arg[0]._aptr;

	if(menu_real == NULL) {
		cpu->d[0] = 0;
		DEBUG(5) dprintf("  Menu ptr (D0): 0x%x\n",0);
		return HOOK_DONE;
	}

	menu = lib_intuition_s_menu_get_real(menu_real);
	if(menu == NULL) {
		DEBUG(1) dprintf("Error: gadtools.library: lib_gadtools_f_CreateMenusA_3: Could not create mapping for Menu\n");
		return HOOK_END_PROC;
	}

	cpu->d[0] = menu->entry->startAddr;

	DEBUG(5) dprintf("  Menu ptr (D0): 0x%x\n", cpu->d[0]);

	return HOOK_DONE;
}
