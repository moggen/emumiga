/*
lib_locale/f_closelocale.c - locale.library CloseLocale function

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

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <proto/locale.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../hook.h"
#include "../emulator.h"
#include "s_locale.h"

/****************************
 Function: CloseLocale
   Vector: 7
   Offset: -42
Arguments: A0.L pointer to struct Locale
  Returns: -
****************************/
int lib_locale_f_CloseLocale(emumsg_syscall_t *msg)
{
	cpu_t *cpu;
	lib_locale_s_locale_t *locale;

	DEBUG(4) dprintf("locale.library: lib_locale_f_CloseLocale() called\n");

	cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  Locale ptr (A0): 0x%x\n",cpu->a[0]);

	/* NULL is ok */
	if(cpu->a[0] == 0) {
		return HOOK_DONE;
	}

	locale = lib_locale_s_locale_get_vaddr(cpu->a[0]);
	if(locale == NULL) {
		DEBUG(1) dprintf("Error: locale.library: lib_locale_f_CloseLocale: Locale pointer is invalid\n");
		return HOOK_END_PROC;
	}

	/* Prepare syscall */
	msg->arg[0]._aptr = (APTR)locale->real;

	DEBUG(5) dprintf("  Locale ptr (native): %p\n",locale->real);

	/* Delete mapping */
	mmu_delEntry(locale->entry);

	return HOOK_SYSCALL;
}


int lib_locale_f_CloseLocale_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */
	CloseLocale(
		(struct Locale *)msg->arg[0]._aptr
	);

	return HOOK_DONE;
}

int lib_locale_f_CloseLocale_3(emumsg_syscall_t *msg)
{
	DEBUG(4) dprintf("locale.library: lib_locale_f_CloseLocale_3() called\n");
	return HOOK_DONE;
}
