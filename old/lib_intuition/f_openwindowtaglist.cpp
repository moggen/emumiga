/*
lib_intuition/f_openwindowtaglist.c - intuition.library OpenWindowTagList
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
#include "s_screen.h"

/****************************
 Function: OpenWindowTagList
   Vector: 101
   Offset: -606
Arguments: A0.L Pointer to NewWindow struct
           A1.L Pointer to TagItem list
  Returns: D0.L pointer to the Window
****************************/
int lib_intuition_f_OpenWindowTagList(emumsg_syscall_t *msg)
{
        cpu_t *cpu;
	struct NewWindow *newwindow_real;
	struct TagItem *tagitems_real, *curtag;
	int i;
	uint32_t vaddr, tag, data;
	uint8_t *real;
	lib_intuition_s_screen_t *screen;

	DEBUG(4) dprintf("intuition.library: lib_intuition_f_OpenWindowTagList() called\n");

        cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  NewWindow ptr (A0): 0x%x, TagItem ptr (A1): 0x%x\n",cpu->a[0],cpu->a[1]);

	newwindow_real = NULL;
        if(cpu->a[0] != 0) {
                DEBUG(2) dprintf("Warning: intuition.library: lib_intuition_f_OpenWindowTagList: NewWindow structure was provided, but is not supported yet. Ignoring\n");
        }

	tagitems_real = NULL;
        if(cpu->a[1] != 0) {
		/* TagItems provided, count them first */
		i = 0;
		vaddr = cpu->a[1];
		do {
			if(READMEM_32(vaddr,&tag) || READMEM_32(vaddr+4,&data)) {
		                DEBUG(1) dprintf("Error: intuition.library: lib_intuition_f_OpenWindowTagList: Error reading tag list\n");
		                return HOOK_END_PROC;
		        }
			vaddr += 8;
			i++;
		} while (tag != 0);

		/* Counted, allocate real mem */
		tagitems_real = (struct TagItem *)AllocVec(sizeof(struct TagItem) * i, MEMF_PUBLIC|MEMF_CLEAR);
		if(tagitems_real == NULL) {
	                DEBUG(1) dprintf("Error: intuition.library: lib_intuition_f_OpenWindowTagList: Could not allocate memory for tag list\n");
	                return HOOK_END_PROC;
		}

		/* Iterate through it */
		curtag = tagitems_real;
		vaddr = cpu->a[1];
		do {
			if(READMEM_32(vaddr,&tag) || READMEM_32(vaddr+4,&data)) {
		                DEBUG(1) dprintf("Error: intuition.library: lib_intuition_f_OpenWindowTagList: Error reading tag list\n");
		                return HOOK_END_PROC;
		        }
			vaddr += 8;

			switch(tag) {

				case 0:			/* TAG_DONE */
					curtag->ti_Tag = TAG_DONE;
					curtag->ti_Data = 0;
					curtag++;
					break;

				case 1:			/* TAG_IGNORE */
					break;

				case 0x80000066:	/* WA_Width */
					curtag->ti_Tag = WA_Width;
					curtag->ti_Data = data;
					curtag++;
					break;

				case 0x80000067:	/* WA_Height */
					curtag->ti_Tag = WA_Height;
					curtag->ti_Data = data;
					curtag++;
					break;

				case 0x8000006a:	/* WA_IDCMP */
					curtag->ti_Tag = WA_IDCMP;
					curtag->ti_Data = data;
					curtag++;
					break;

				case 0x8000006e:	/* WA_Title */
					curtag->ti_Tag = WA_Title;
					real = vptr(data);
					/* TODO: Better check */
					curtag->ti_Data = (STACKIPTR)real;
					curtag++;
					break;

				case 0x80000072:	/* WA_MinWidth */
					curtag->ti_Tag = WA_MinWidth;
					curtag->ti_Data = data;
					curtag++;
					break;

				case 0x80000073:	/* WA_MinHeight */
					curtag->ti_Tag = WA_MinHeight;
					curtag->ti_Data = data;
					curtag++;
					break;

				case 0x80000074:	/* WA_MaxWidth */
					curtag->ti_Tag = WA_MaxWidth;
					curtag->ti_Data = data;
					curtag++;
					break;

				case 0x80000075:	/* WA_MaxHeight */
					curtag->ti_Tag = WA_MaxHeight;
					curtag->ti_Data = data;
					curtag++;
					break;

				case 0x80000076:	/* WA_InnerWidth */
					curtag->ti_Tag = WA_InnerWidth;
					curtag->ti_Data = data;
					curtag++;
					break;

				case 0x80000077:	/* WA_InnerHeight */
					curtag->ti_Tag = WA_InnerHeight;
					curtag->ti_Data = data;
					curtag++;
					break;

				case 0x80000079:	/* WA_PubScreen */
					screen = lib_intuition_s_screen_get_vaddr(data);
					if(screen != NULL) {
						curtag->ti_Tag = WA_PubScreen;
						curtag->ti_Data = (STACKIPTR)screen->real;
						curtag++;
					}
					break;

				case 0x8000007d:	/* WA_Zoom */
					curtag->ti_Tag = WA_Zoom;
					real = vptr(data);
					/* TODO: Better check */
					curtag->ti_Data = (STACKIPTR)real;
					curtag++;
					break;

				case 0x80000081:	/* WA_SizeGadget */
					curtag->ti_Tag = WA_SizeGadget;
					curtag->ti_Data = data;
					curtag++;
					break;

				case 0x80000082:	/* WA_DragBar */
					curtag->ti_Tag = WA_DragBar;
					curtag->ti_Data = data;
					curtag++;
					break;

				case 0x80000083:	/* WA_DepthGadget */
					curtag->ti_Tag = WA_DepthGadget;
					curtag->ti_Data = data;
					curtag++;
					break;

				case 0x80000084:	/* WA_CloseGadget */
					curtag->ti_Tag = WA_CloseGadget;
					curtag->ti_Data = data;
					curtag++;
					break;

				case 0x80000089:	/* WA_Activate */
					curtag->ti_Tag = WA_Activate;
					curtag->ti_Data = data;
					curtag++;
					break;

				case 0x8000008c:	/* WA_SimpleRefresh */
					curtag->ti_Tag = WA_SimpleRefresh;
					curtag->ti_Data = data;
					curtag++;
					break;

				case 0x8000008f:	/* WA_SizeBBottom */
					curtag->ti_Tag = WA_SizeBBottom;
					curtag->ti_Data = data;
					curtag++;
					break;

				case 0x80000090:	/* WA_AutoAdjust */
					curtag->ti_Tag = WA_AutoAdjust;
					curtag->ti_Data = data;
					curtag++;
					break;

				case 0x80000093:	/* WA_NewLookMenus */
					curtag->ti_Tag = WA_NewLookMenus;
					curtag->ti_Data = data;
					curtag++;
					break;

				default:
			                DEBUG(2) dprintf("Warning: intuition.library: lib_intuition_f_OpenWindowTagList: Unsupported tag. Tag: 0x%x, Data: 0x%x\n", tag, data);
					break;
			}
		} while (tag != 0);
	}

	/* Prepare syscall */

	msg->arg[0]._aptr = (APTR)newwindow_real;
	msg->arg[1]._aptr = (APTR)tagitems_real;

	DEBUG(5) dprintf("  NewWindow ptr (native): %p, TagItem ptr (native): %p\n",newwindow_real,tagitems_real);

	/* Do syscall */
	return HOOK_SYSCALL;
}

int lib_intuition_f_OpenWindowTagList_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */
	msg->arg[0]._aptr = OpenWindowTagList((struct NewWindow *)msg->arg[0]._aptr,(struct TagItem *)msg->arg[1]._aptr);

	return HOOK_DONE;
}

int lib_intuition_f_OpenWindowTagList_3(emumsg_syscall_t *msg)
{
	cpu_t *cpu;
	struct Window *window_real;
	lib_intuition_s_window_t *window;

	DEBUG(4) dprintf("intuition.library: lib_intuition_f_OpenWindowTagList_3() called\n");

	cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  Window ptr (native): %p\n", msg->arg[0]._aptr);

	/* Extract return values */
	window_real = (struct Window *)msg->arg[0]._aptr;

	if(window_real == NULL) {
		cpu->d[0] = 0;
		DEBUG(5) dprintf("  Window ptr (D0): 0x%x\n",0);
		return HOOK_DONE;
	}

	window = lib_intuition_s_window_get_real(window_real);
	if(window == NULL) {
		DEBUG(1) dprintf("Error: intuition.library: lib_intuition_f_OpenWindowTagList_3: Could not create mapping for Window\n");
		return HOOK_END_PROC;
	}

	cpu->d[0] = window->entry->startAddr;

	DEBUG(5) dprintf("  Window ptr (D0): 0x%x\n", cpu->d[0]);

	return HOOK_DONE;
}
