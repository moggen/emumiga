/*
lib_gadtools/f_creategadgeta.c - gadtools.library CreateGadgetA function

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
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../hook.h"
#include "../emulator.h"
#include "s_newgadget.h"
#include "../lib_intuition/s_gadget.h"

/****************************
 Function: CreateGadgetA
   Vector: 5
   Offset: -30
Arguments: D0.L kind
           A0.L pointer to previos gadget or NULL
           A1.L pointer to a NewGadget structure
           A2.L pointer to tag array
  Returns: D0.L pointer to gadget stucture
****************************/
int lib_gadtools_f_CreateGadgetA(emumsg_syscall_t *msg)
{
        cpu_t *cpu;

	lib_intuition_s_gadget_t *prevgadget;
	lib_gadtools_s_newgadget_t *newgadget;
	struct TagItem *tagitems_real, *curtag;
	int i;
	uint32_t vaddr, tag, data;


	DEBUG(4) dprintf("gadtools.library: lib_gadtools_f_CreateGadgetA() called\n");

        cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  Kind (D0): %d, Previous gadget (A0): 0x%x, NewGadget ptr (A1): 0x%x, Tag array (A2): 0x%x\n",cpu->d[0],cpu->a[0],cpu->a[1],cpu->a[2]);

	prevgadget = lib_intuition_s_gadget_get_vaddr(cpu->a[0]);
        if(prevgadget == NULL) {
                DEBUG(1) dprintf("Error: gadtools.library: lib_gadtools_f_CreateGadgetA: Previous gadget pointer is invalid\n");
                return HOOK_END_PROC;
        }

	newgadget = lib_gadtools_s_newgadget_get_vaddr(cpu->a[1]);
        if(newgadget == NULL) {
                DEBUG(1) dprintf("Error: gadtools.library: lib_gadtools_f_CreateGadgetA: NewGadget pointer is invalid\n");
                return HOOK_END_PROC;
        }

	tagitems_real = NULL;
	if(cpu->a[2] != 0) {
		/* TagItems provided, count them first */
		i = 0;
		vaddr = cpu->a[2];
		do {
			if(READMEM_32(vaddr,&tag) || READMEM_32(vaddr+4,&data)) {
				DEBUG(1) dprintf("Error: gadtools.library: lib_gadtools_f_CreateGadgetA: Error reading tag list\n");
				return HOOK_END_PROC;
			}
			vaddr += 8;
			i++;
		} while (tag != 0);

		/* Counted, allocate real mem */
		tagitems_real = (struct TagItem *)AllocVec(sizeof(struct TagItem) * i, MEMF_PUBLIC|MEMF_CLEAR);
		if(tagitems_real == NULL) {
			DEBUG(1) dprintf("Error: gadtools.library: lib_gadtools_f_CreateGadgetA: Could not allocate memory for tag list\n");
			return HOOK_END_PROC;
		}

		/* Iterate through it */
		curtag = tagitems_real;
		vaddr = cpu->a[2];
		do {
			if(READMEM_32(vaddr,&tag) || READMEM_32(vaddr+4,&data)) {
				DEBUG(1) dprintf("Error: gadtools.library: lib_gadtools_f_CreateGadgetA: Error reading tag list\n");
				return HOOK_END_PROC;
			}
			vaddr += 8;

			switch(tag) {

				case 0:                 /* TAG_DONE */
					curtag->ti_Tag = TAG_DONE;
					curtag->ti_Data = 0;
					curtag++;
					break;

				case 1:                 /* TAG_IGNORE */
					break;


				case 0x80030015:	/* GA_Immediate */
					curtag->ti_Tag = GA_Immediate;
					curtag->ti_Data = data;
					curtag++;

				case 0x80030016:	/* GA_RelVerify */
					curtag->ti_Tag = GA_RelVerify;
					curtag->ti_Data = data;
					curtag++;

				case 0x80080026:	/* GTSL_Min */
					curtag->ti_Tag = GTSL_Min;
					curtag->ti_Data = data;
					curtag++;
					break;

				case 0x80080027:	/* GTSL_Max */
					curtag->ti_Tag = GTSL_Max;
					curtag->ti_Data = data;
					curtag++;
					break;

				case 0x80080028:	/* GTSL_Level */
					curtag->ti_Tag = GTSL_Level;
					curtag->ti_Data = data;
					curtag++;
					break;

				default:
					DEBUG(2) dprintf("Warning: gadtools.library: lib_gadtools_f_CreateGadgetA: Unsupported tag. Tag: 0x%x, Data: 0x%x\n", tag, data);
					break;
			}
		} while (tag != 0);
	}

	/* Prepare syscall */
	msg->arg[0]._ulong = (ULONG)cpu->d[0];
	msg->arg[1]._aptr = (APTR)prevgadget->real;
	msg->arg[2]._aptr = (APTR)newgadget->real;
	msg->arg[3]._aptr = (APTR)tagitems_real;

	DEBUG(5) dprintf("  Previous gadget (native): %p, NewGadget ptr (native): %p, Tag array (native): %p\n",prevgadget->real,newgadget->real,tagitems_real);

	/* Do syscall */
	return HOOK_SYSCALL;
}

int lib_gadtools_f_CreateGadgetA_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */
	msg->arg[0]._aptr = CreateGadgetA(
		msg->arg[0]._ulong,
		(struct Gadget *)msg->arg[1]._aptr,
		(struct NewGadget *)msg->arg[2]._aptr,
		(struct TagItem *)msg->arg[3]._aptr
	);

	return HOOK_DONE;
}

int lib_gadtools_f_CreateGadgetA_3(emumsg_syscall_t *msg)
{
	cpu_t *cpu;
	lib_intuition_s_gadget_t *gadget;
	struct Gadget *gadget_real;

	DEBUG(4) dprintf("gadtools.library: lib_gadtools_f_CreateGadgetA_3() called\n");

	cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  Gadget ptr (native): %p\n", msg->arg[0]._aptr);

	/* Extract return values */
	gadget_real = (struct Gadget *)msg->arg[0]._aptr;

	if(gadget_real == NULL) {
		cpu->d[0] = 0;
		DEBUG(5) dprintf("  Gadget ptr (D0): 0x%x\n",0);
		return HOOK_DONE;
	}

	gadget = lib_intuition_s_gadget_get_real(gadget_real);
	if(gadget == NULL) {
		DEBUG(1) dprintf("Error: gadtools.library: lib_gadtools_f_CreateGadgetA_3: Could not create mapping for Gadget\n");
		return HOOK_END_PROC;
	}

	cpu->d[0] = gadget->entry->startAddr;

	DEBUG(5) dprintf("  Gadget ptr (D0): 0x%x\n", cpu->d[0]);

	return HOOK_DONE;
}
