/*
lib_locale/f_opencatalog.c - locale.library OpenCatalog function

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

#include <proto/locale.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../hook.h"
#include "../emulator.h"
#include "s_locale.h"
#include "s_catalog.h"

/****************************
 Function: OpenCatalog
   Vector: 25
   Offset: -150
Arguments: A0.L pointer to locale struct
           A1.L catalog name string
           A2.L pointer to tag array
  Returns: D0.L pointer to struct Catalog
****************************/
int lib_locale_f_OpenCatalog(emumsg_syscall_t *msg)
{
        cpu_t *cpu;
	lib_locale_s_locale_t *locale;
	struct Locale *locale_real;
	uint8_t *string_real;
	struct TagItem *tags_real;

	DEBUG(4) dprintf("locale.library: lib_locale_f_OpenCatalog() called\n");

        cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  Locale ptr (A0): 0x%x, Catalog name (A1): 0x%x, Tag array (A2): 0x%x\n",cpu->a[0],cpu->a[1],cpu->a[2]);

	locale_real = NULL;
	if(cpu->a[0] != 0) {
		locale = lib_locale_s_locale_get_vaddr(cpu->a[0]);
	        if(locale == NULL) {
	                DEBUG(1) dprintf("Error: locale.library: lib_locale_f_OpenCatalog: Locale struct pointer is invalid\n");
	                return HOOK_END_PROC;
	        }
		locale_real = locale->real;
	}

	string_real = NULL;
	if(cpu->a[1] != 0) {
	        string_real = vptr(cpu->a[1]);
	        if(string_real == NULL) {
	                DEBUG(1) dprintf("Error: locale.library: lib_locale_f_OpenCatalog: Catalog name string pointer is invalid\n");
	                return HOOK_END_PROC;
	        }
	}

	tags_real = NULL;
	if(cpu->a[2] != 0) {
                DEBUG(2) dprintf("Warning: locale.library: lib_locale_f_OpenCatalog: Tags are provided but is not supported yet\n");
	}

	/* Prepare syscall */
	msg->arg[0]._aptr = (APTR)locale_real;
	msg->arg[1]._strptr = (STRPTR)string_real;
	msg->arg[2]._aptr = (APTR)tags_real;

	DEBUG(5) dprintf("  Locale ptr (native): %p, Catalog name (native): %p, Tag array (native): %p\n",locale_real,string_real,tags_real);

	/* Do syscall */
	return HOOK_SYSCALL;
}

int lib_locale_f_OpenCatalog_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */
	msg->arg[0]._aptr = OpenCatalog((struct Locale *)msg->arg[0]._aptr,msg->arg[1]._strptr,(struct TagItem *)msg->arg[2]._aptr);

	return HOOK_DONE;
}

int lib_locale_f_OpenCatalog_3(emumsg_syscall_t *msg)
{
	cpu_t *cpu;
	struct Catalog *real;
	lib_locale_s_catalog_t *catalog;

	DEBUG(4) dprintf("locale.library: lib_locale_f_OpenCatalog_3() called\n");

	cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  Catalog (native): %p\n", msg->arg[0]._aptr);

	/* Extract return values */
	real = (struct Catalog *)msg->arg[0]._aptr;

	if(real == NULL) {
		cpu->d[0] = 0;
		DEBUG(5) dprintf("  Catalog (D0): 0x%x\n",0);
		return HOOK_DONE;
	}

	catalog = lib_locale_s_catalog_get_real(real);
	if(catalog == NULL) {
		DEBUG(1) dprintf("Error: locale.library: lib_locale_f_OpenCatalog_3: Could not create mapping for catalog\n");
		return HOOK_END_PROC;
	}

	cpu->d[0] = catalog->entry->startAddr;

	DEBUG(5) dprintf("  Catalog (D0): 0x%x\n", cpu->d[0]);

	return HOOK_DONE;
}
