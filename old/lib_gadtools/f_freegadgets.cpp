/*
lib_gadtools/f_freegadgets.c - gadtools.library FreeGadgets function

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
#include "../lib_intuition/s_gadget.h"

/****************************
 Function: FreeGadgets
   Vector: 6
   Offset: -36
Arguments: A0.L pointer to Gadget structure
  Returns: -
****************************/
int lib_gadtools_f_FreeGadgets(emumsg_syscall_t *msg)
{
        cpu_t *cpu;
	mmu_entry_t *entry;
	struct Gadget *gadget_real;

	DEBUG(4) dprintf("gadtools.library: lib_gadtools_f_FreeGadgets() called\n");

        cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  Gadget ptr (A0): 0x%x\n",cpu->a[0]);

	/* NULL is OK */
	if(cpu->a[0] == 0) {
		return HOOK_DONE;
	}

	/* Find our entry */
	entry = mmu_findEntryWithType(cpu->a[0], MMU_TYPE_INTUITION_S_GADGET);
	if(entry == NULL) {
                DEBUG(1) dprintf("Error: gadtools.library: lib_gadtools_f_FreeGadgets: Gadget pointer is invalid\n");
		return HOOK_END_PROC;
	}

	gadget_real = (struct Gadget *)entry->real;

	/* Prepare syscall */
	msg->arg[0]._aptr = (APTR)gadget_real;

	DEBUG(5) dprintf("  Gadget ptr (native): %p\n",gadget_real);

	/* Loop through the gadget linked list and free all mappings */
	while(gadget_real != NULL) {
		entry = mmu_findEntryWithTypeReal(gadget_real, MMU_TYPE_INTUITION_S_GADGET);
		if(entry != NULL) {
			mmu_delEntry(entry);
		}
		gadget_real = gadget_real->NextGadget;
	}

	/* Do syscall */
	return HOOK_SYSCALL;
}

int lib_gadtools_f_FreeGadgets_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */
	FreeGadgets((struct Gadget *)msg->arg[0]._aptr);

	return HOOK_DONE;
}

int lib_gadtools_f_FreeGadgets_3(emumsg_syscall_t *msg)
{
	DEBUG(4) dprintf("gadtools.library: lib_gadtools_f_FreeGadgets_3() called\n");

	return HOOK_DONE;
}
