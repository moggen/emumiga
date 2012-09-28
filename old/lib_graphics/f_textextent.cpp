/*
lib_graphics/f_textextent.c - graphics.library TextExtent function

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
#include <proto/graphics.h>
#include <graphics/rastport.h>
#include <graphics/text.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../hook.h"
#include "../emulator.h"
#include "s_rastport.h"
#include "s_textextent.h"

/****************************
 Function: TextExtent
   Vector: 115
   Offset: -690
Arguments: A1.L Pointer to struct RastPort
           A0.L Pointer to text string
           D0.W Character count
           A2.L Pointer to struct TextExtent to be filled in
  Returns: -
****************************/
int lib_graphics_f_TextExtent(emumsg_syscall_t *msg)
{
        cpu_t *cpu;
	lib_graphics_s_rastport_t *rastport;
	uint8_t *string_real;
	lib_graphics_s_textextent_t *textextent;
	struct TextExtent *textextent_real;

	DEBUG(4) dprintf("graphics.library: lib_graphics_f_TextExtent() called\n");

        cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  RastPort ptr (A1): 0x%x, String ptr (A0): 0x%x, Character count: (D0.W): %d, Pointer to TextExtent (A2): 0x%x\n",cpu->a[1],cpu->a[0],cpu->d[0]&0xffff,cpu->a[2]);

	rastport = lib_graphics_s_rastport_get_vaddr(cpu->a[1]);
	if(rastport == NULL) {
		DEBUG(1) dprintf("Error: graphics.library: lib_graphics_f_TextExtent: RastPort struct pointer is invalid\n");
		return HOOK_END_PROC;
	}

	string_real = vptr(cpu->a[0]);   /* TODO: Better check */
	if(string_real == NULL) {
		DEBUG(1) dprintf("Error: graphics.library: lib_graphics_f_TextExtent: Stringd pointer is invalid\n");
		return HOOK_END_PROC;
	}

	/* Allocate a buffer in real space for the TextEvent structure */
	textextent_real = (struct TextExtent *)AllocVec(sizeof(struct TextExtent), MEMF_PUBLIC|MEMF_CLEAR);
        if(textextent_real == NULL) {
                DEBUG(1) dprintf("Error: graphics.library: lib_graphics_f_TextExtent: Could not allocate memory for TextExtent\n");
                return HOOK_END_PROC;
        }
	textextent = lib_graphics_s_textextent_get_real_in_user(textextent_real, cpu->a[2]);
	if(textextent == NULL) {
		DEBUG(1) dprintf("Error: graphics.library: lib_graphics_f_TextExtent: Could not create mapping for TextExtent\n");
		FreeVec(textextent_real);
		return HOOK_END_PROC;
	}

	/* Prepare syscall */
	msg->arg[0]._aptr = (APTR)rastport->real;
	msg->arg[1]._strptr = (STRPTR)string_real;
	msg->arg[2]._word = (WORD)(cpu->d[0] & 0xffff);
	msg->arg[3]._aptr = (APTR)textextent->real;

	DEBUG(5) dprintf("  RastPort ptr (native): %p, String ptr (native): %p, Pointer to TextExtent (native): %p\n",rastport->real,string_real,textextent->real);

	/* Do syscall */
	return HOOK_SYSCALL;
}

int lib_graphics_f_TextExtent_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */
	TextExtent(
		(struct RastPort *)msg->arg[0]._aptr,
		msg->arg[1]._strptr,
		msg->arg[2]._word,
		(struct TextExtent *)msg->arg[3]._aptr
	);

	return HOOK_DONE;
}

int lib_graphics_f_TextExtent_3(emumsg_syscall_t *msg __attribute__((unused)))
{
	DEBUG(4) dprintf("graphics.library: lib_graphics_f_TextExtent_3() called\n");

	return HOOK_DONE;
}
