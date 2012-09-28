/*
lib_gadtools/f_drawbevelboxa.c - gadtools.library DrawBevelBoxA function

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
#include "../lib_graphics/s_rastport.h"
#include "s_visualinfo.h"

/****************************
 Function: DrawBevelBoxA
   Vector: 20
   Offset: -120
Arguments: A0.L Pointer to RastPort
           D0.L Left
           D1.L Top
           D2.L Width
           D3.L Height
           A1.L Pointer to taglist
  Returns: -
****************************/
int lib_gadtools_f_DrawBevelBoxA(emumsg_syscall_t *msg)
{
        cpu_t *cpu;

	lib_graphics_s_rastport_t *rastport;
	lib_gadtools_s_visualinfo_t *visualinfo;
	struct RastPort *rastport_real;
	struct TagItem *tagitems_real, *curtag;
	int i;
	uint32_t vaddr, tag, data;

	DEBUG(4) dprintf("gadtools.library: lib_gadtools_f_DrawBevelBoxA() called\n");

        cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  RastPort ptr (A0): 0x%x, Left (D0): %d, Top (D1): %d, Width (D2): %d, Height (D3): %d, Tag array (A1): 0x%x\n",cpu->a[0],cpu->d[0],cpu->d[1],cpu->d[2],cpu->d[3],cpu->a[1]);

	rastport = lib_graphics_s_rastport_get_vaddr(cpu->a[0]);
	if(rastport == NULL) {
		DEBUG(1) dprintf("Error: gadtools.library: lib_gadtools_f_DrawBevelBoxA: Rastport pointer is invalid\n");
		return HOOK_END_PROC;
	}
	rastport_real = rastport->real;

	tagitems_real = NULL;
	if(cpu->a[1] != 0) {
		/* TagItems provided, count them first */
		i = 0;
		vaddr = cpu->a[1];
		do {
			if(READMEM_32(vaddr,&tag) || READMEM_32(vaddr+4,&data)) {
				DEBUG(1) dprintf("Error: gadtools.library: lib_gadtools_f_DrawBevelBoxA: Error reading tag list\n");
				return HOOK_END_PROC;
			}
			vaddr += 8;
			i++;
		} while (tag != 0);

		/* Counted, allocate real mem */
		tagitems_real = (struct TagItem *)AllocVec(sizeof(struct TagItem) * i, MEMF_PUBLIC|MEMF_CLEAR);
		if(tagitems_real == NULL) {
			DEBUG(1) dprintf("Error: gadtools.library: lib_gadtools_f_DrawBevelBoxA: Could not allocate memory for tag list\n");
			return HOOK_END_PROC;
		}

		/* Iterate through it */
		curtag = tagitems_real;
		vaddr = cpu->a[1];
		do {
			if(READMEM_32(vaddr,&tag) || READMEM_32(vaddr+4,&data)) {
				DEBUG(1) dprintf("Error: gadtools.library: lib_gadtools_f_DrawBevelBoxA: Error reading tag list\n");
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

                                case 0x80080033:	/* GTBB_Recessed */
                                        curtag->ti_Tag = GTBB_Recessed;
                                        curtag->ti_Data = data;
                                        curtag++;
                                        break;

                                case 0x80080034:	/* GT_VisualInfo */
					visualinfo = lib_gadtools_s_visualinfo_get_vaddr(data);
					if(visualinfo != NULL) {
	                                        curtag->ti_Tag = GT_VisualInfo;
	                                        curtag->ti_Data = (STACKIPTR)visualinfo->real;
	                                        curtag++;
					}
                                        break;

				default:
					DEBUG(2) dprintf("Warning: gadtools.library: lib_gadtools_f_DrawBevelBoxA: Unsupported tag. Tag: 0x%x, Data: 0x%x\n", tag, data);
					break;
			}
		} while (tag != 0);
	}

	/* Prepare syscall */

	msg->arg[0]._aptr = (APTR)rastport_real;
	msg->arg[1]._long = (LONG)cpu->d[0];
	msg->arg[2]._long = (LONG)cpu->d[1];
	msg->arg[3]._long = (LONG)cpu->d[2];
	msg->arg[4]._long = (LONG)cpu->d[3];
	msg->arg[5]._aptr = (APTR)tagitems_real;

	DEBUG(5) dprintf("  RastPort ptr (native): %p, Tag array (native): %p\n",rastport_real,tagitems_real);

	/* Do syscall */
	return HOOK_SYSCALL;
}

int lib_gadtools_f_DrawBevelBoxA_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */
	DrawBevelBoxA(
		(struct RastPort *)msg->arg[0]._aptr,
		msg->arg[1]._long,
		msg->arg[2]._long,
		msg->arg[3]._long,
		msg->arg[4]._long,
		(struct TagItem *)msg->arg[5]._aptr
	);

	return HOOK_DONE;
}

int lib_gadtools_f_DrawBevelBoxA_3(emumsg_syscall_t *msg)
{
	DEBUG(4) dprintf("gadtools.library: lib_gadtools_f_DrawBevelBoxA_3() called\n");

	return HOOK_DONE;
}
