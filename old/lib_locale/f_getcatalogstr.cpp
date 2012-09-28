/*
lib_locale/f_getcatalogstr.c - locale.library GetCatalogStr function

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
#include "s_catalog.h"

/****************************
 Function: GetCatalogStr
   Vector: 12
   Offset: -72
Arguments: A0.L Pointer to struct Catalog
           D0.L String number
           A1.L Pointer to default string
  Returns: D0.L Pointer to localized string
****************************/
int lib_locale_f_GetCatalogStr(emumsg_syscall_t *msg)
{
        cpu_t *cpu;
	lib_locale_s_catalog_t *catalog;
	struct Catalog *catalog_real;

	DEBUG(4) dprintf("locale.library: lib_locale_f_GetCatalogStr() called\n");

        cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  Catalog ptr (A0): 0x%x, String number (D0): 0x%x, Default string ptr (A1): 0x%x\n",cpu->a[0],cpu->d[0],cpu->a[1]);

	/* Null is ok here */
	catalog_real = NULL;
	if(cpu->a[0] != 0) {
		catalog = lib_locale_s_catalog_get_vaddr(cpu->a[0]);
	        if(catalog == NULL) {
	                DEBUG(1) dprintf("Error: locale.library: lib_locale_f_GetCatalogStr: Catalog struct pointer is invalid\n");
	                return HOOK_END_PROC;
	        }
		catalog_real = catalog->real;
	}

	/* Prepare syscall */
	msg->arg[0]._aptr = (APTR)catalog_real;
	msg->arg[1]._long = (LONG)cpu->d[0];

	/* Use a trick here. Provide a dummy string pointer, in this case
	   the pointer to the syscal msg, to detect if fallback to the
	   default string is done. If there is a fallback we just return
	   the provided default string VADDR. No mappings are needed */
	msg->arg[2]._strptr = (STRPTR)msg;

	DEBUG(5) dprintf("  Catalog ptr (native): %p, Default string ptr (native): %p\n",catalog_real,msg);

	/* Do syscall */
	return HOOK_SYSCALL;
}

int lib_locale_f_GetCatalogStr_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */
	/* Explicit cast to STRPTR to remove AROS const warning */
	msg->arg[0]._strptr = (STRPTR)GetCatalogStr((struct Catalog *)msg->arg[0]._aptr,msg->arg[1]._long,msg->arg[2]._strptr);

	return HOOK_DONE;
}

int lib_locale_f_GetCatalogStr_3(emumsg_syscall_t *msg)
{
	cpu_t *cpu;
	STRPTR real;

	DEBUG(4) dprintf("locale.library: lib_locale_f_GetCatalogStr_3() called\n");

	cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  String ptr (native): %p\n", msg->arg[0]._strptr);

	/* Extract return values */
	real = msg->arg[0]._strptr;

	if(real == NULL) {
		cpu->d[0] = 0;
		DEBUG(5) dprintf("  String ptr (D0): 0x%x\n",0);
		return HOOK_DONE;
	}

	/* Detect fallback to default string */
	if(real == (STRPTR)msg) {
		/* Yes, easy case return string from A1 */
		cpu->d[0] = cpu->a[1];
	} else {
		/* No, got a string from the catalog, must set up map */

		/* TODO: Implement this, must use some kind of key - value
			 collection to remember all mappings */

		DEBUG(1) dprintf("Error: locale.library: lib_locale_f_GetCatalogStr_3: A string from the catalog was returned. That is not implemented yet.\n");
		return HOOK_END_PROC;
	}

	DEBUG(5) dprintf("  String ptr (D0): 0x%x\n", cpu->d[0]);

	return HOOK_DONE;
}
