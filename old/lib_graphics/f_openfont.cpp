/*
lib_graphics/f_openfont.c - graphics.library OpenFont function

Copyright (C) 2010  Magnus �berg

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
#include <graphics/text.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../hook.h"
#include "../emulator.h"
#include "s_textfont.h"
#include "s_textattr.h"

/****************************
 Function: OpenFont
   Vector: 12
   Offset: -72
Arguments: A0.L Pointer to struct TextAttr/TTextAttr
  Returns: D0.L pointer to struct TextFont
****************************/
int lib_graphics_f_OpenFont(emumsg_syscall_t *msg)
{
        cpu_t *cpu;
	lib_graphics_s_textattr_t *textattr;

	DEBUG(4) dprintf("graphics.library: lib_graphics_f_OpenFont() called\n");

        cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  (T)TextAttr ptr (A0): 0x%x\n",cpu->a[0]);

	/* Null is ok */
	textattr = lib_graphics_s_textattr_get_vaddr(cpu->a[0]);

	/* Prepare syscall */
	if(textattr) {
		msg->arg[0]._aptr = (APTR)textattr->real;
	} else {
		msg->arg[0]._aptr = NULL;
	}

	DEBUG(5) dprintf("  (T)TextAttr ptr (native): %p\n",textattr->real);

	/* Do syscall */
	return HOOK_SYSCALL;
}

int lib_graphics_f_OpenFont_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */
	msg->arg[0]._aptr = OpenFont((struct TextAttr *)msg->arg[0]._aptr);

	return HOOK_DONE;
}

int lib_graphics_f_OpenFont_3(emumsg_syscall_t *msg)
{
	cpu_t *cpu;
	struct TextFont *real;
	lib_graphics_s_textfont_t *textfont;

	DEBUG(4) dprintf("graphics.library: lib_graphics_f_OpenFont_3() called\n");

	cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  TextFont ptr (native): %p\n", msg->arg[0]._aptr);

	/* Extract return values */
	real = (struct TextFont *)msg->arg[0]._aptr;

	if(real == NULL) {
		cpu->d[0] = 0;
		DEBUG(5) dprintf("  TextFont ptr (D0): 0x%x\n",0);
		return HOOK_DONE;
	}

	textfont = lib_graphics_s_textfont_get_real(real);
	if(textfont == NULL) {
		DEBUG(1) dprintf("Error: graphics.library: lib_graphics_f_OpenFont_3: Could not create mapping for TextFont\n");
		return HOOK_END_PROC;
	}

	cpu->d[0] = textfont->entry->startAddr;

	DEBUG(5) dprintf("  TextFont ptr (D0): 0x%x\n", cpu->d[0]);

	return HOOK_DONE;
}
