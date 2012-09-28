/*
lib_gadtools/f_createcontext.c - gadtools.library CreateContext function

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
#include <proto/exec.h>
#include <intuition/intuition.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../hook.h"
#include "../emulator.h"
#include "../lib_intuition/s_gadget.h"

/****************************
 Function: CreateContext
   Vector: 19
   Offset: -114
Arguments: A0.L pointer to pointer to struct Gadget
  Returns: D0.L pointer to stuct Gadget
****************************/
int lib_gadtools_f_CreateContext(emumsg_syscall_t *msg)
{
        cpu_t *cpu;
	struct Gadget **ptr;

	DEBUG(4) dprintf("gadtools.library: lib_gadtools_f_CreateContext() called\n");

        cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  Gadget ptr ptr (A0): 0x%x\n",cpu->a[0]);

	/* Allocate temporary space for the pointer */
	ptr = (struct Gadget **)AllocVec(sizeof(struct Gadget *),MEMF_PUBLIC|MEMF_CLEAR);
	if(ptr == NULL) {
                DEBUG(1) dprintf("Error: gadtools.library: lib_gadtools_f_CreateContext: Could not allocate temporary memory\n");
                return HOOK_END_PROC;
        }

	/* Prepare syscall */
	msg->arg[0]._aptr = (APTR)ptr;

	/* Remember memory */
	msg->internalPtr = ptr;

	/* Do syscall */
	return HOOK_SYSCALL;
}

int lib_gadtools_f_CreateContext_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */
	msg->arg[0]._aptr = CreateContext(
		(struct Gadget **)msg->arg[0]._aptr
	);

	return HOOK_DONE;
}

int lib_gadtools_f_CreateContext_3(emumsg_syscall_t *msg)
{
	cpu_t *cpu;
	lib_intuition_s_gadget_t *gadget, *gadget2;
	struct Gadget *gadget_real, *gadget2_real;
	struct Gadget **ptr;
	uint32_t ptr_vaddr, gadget2_vaddr;
	int rc;

	DEBUG(4) dprintf("gadtools.library: lib_gadtools_f_CreateContext_3() called\n");

	cpu = msg->proc->cpu;

	/* Extract return values */
	gadget_real = (struct Gadget *)msg->arg[0]._aptr;

	/* Recall memory */
	ptr = (struct Gadget **)msg->internalPtr;

	/* Get whatever the syscall wrote */
	gadget2_real = *ptr;

	DEBUG(5) dprintf("  Gadget ptr (native): %p, Written Gadget ptr (native): %p\n", gadget_real, gadget2_real);

	/* Free the temporary memory */
	FreeVec(ptr);

	/* Get the vaddr */
	ptr_vaddr = cpu->a[0];

	cpu->d[0] = 0;
	if(gadget_real != NULL) {
		gadget = lib_intuition_s_gadget_get_real(gadget_real);
		if(gadget == NULL) {
			DEBUG(1) dprintf("Error: gadtools.library: lib_gadtools_f_CreateContext_3: Could not create mapping for Gadget\n");
			return HOOK_END_PROC;
		}
		cpu->d[0] = gadget->entry->startAddr;
	}

	gadget2_vaddr = 0;
	if(gadget2_real != NULL) {
		gadget2 = lib_intuition_s_gadget_get_real(gadget2_real);
		if(gadget2 == NULL) {
			DEBUG(1) dprintf("Error: gadtools.library: lib_gadtools_f_CreateContext_3: Could not create mapping for Gadget\n");
			return HOOK_END_PROC;
		}
		gadget2_vaddr = gadget2->entry->startAddr;
	}

	/* Write to the pointer pointer */
	rc = WRITEMEM_32(gadget2_vaddr, ptr_vaddr);
	if(rc) {
		DEBUG(1) dprintf("Error: gadtools.library: lib_gadtools_f_CreateContext_3: Memory write to Gadget pointer failed\n");
		return HOOK_END_PROC;
	}

	DEBUG(5) dprintf("  Gadget ptr (D0): 0x%x, Written Gadget ptr: 0x%x\n", cpu->d[0], gadget2_vaddr);

	return HOOK_DONE;
}
