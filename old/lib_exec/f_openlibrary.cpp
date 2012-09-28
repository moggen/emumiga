/*
lib_exec/f_openlibrary.c - exec.library OpenLibrary function

Copyright (C) 2007, 2008, 2009, 2010  Magnus Öberg

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

#include <proto/exec.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../hook.h"
#include "../emulator.h"

/* For declarations of libbases */
#include "../lib_dos/dos.h"
#include "../lib_intuition/intuition.h"
#include "../lib_graphics/graphics.h"
#include "../lib_utility/utility.h"
#include "../lib_gadtools/gadtools.h"
#include "../lib_icon/icon.h"
#include "../lib_layers/layers.h"
#include "../lib_locale/locale.h"

/****************************
 Function: OpenLibrary
   Vector: 92
   Offset: -552
Arguments: A1.L pointer to library name string
           D0.L lowest acceptable version number, 0 means any version
  Returns: D0.L pointer to library or 0 if failure
****************************/
int lib_exec_f_OpenLibrary(emumsg_syscall_t *msg)
{
	cpu_t *cpu;
	uint32_t vaddr;
	char ch, libname[300], *cur;
	uint8_t tmp8;
	int rc;

	DEBUG(4) dprintf("exec.library: lib_exec_OpenLibrary() called\n");

	cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  Name of library (A1): 0x%x, Version (D0): 0x%x\n",cpu->a[1],cpu->d[0]);

	cur = libname;
	vaddr = cpu->a[1];
	do {
		rc = READMEM_8(vaddr++, &tmp8);
		if(rc) {
			dprintf("Error: exec.library: lib_exec_OpenLibrary: Library name is not readable\n");
			return HOOK_END_PROC;
		}
		ch = (char)tmp8;;
		*cur++ = ch; 
	} while(ch != '\0');

	DEBUG(5) dprintf("Library name: %s\n",libname);

	cpu->d[0] = 0;

	if(strcmp("dos.library",libname) == 0){
		cpu->d[0] = lib_dos_base.entry->startAddr;
	} else if(strcmp("intuition.library",libname) == 0){
		cpu->d[0] = lib_intuition_base.entry->startAddr;
	} else if(strcmp("graphics.library",libname) == 0){
		cpu->d[0] = lib_graphics_base.entry->startAddr;
	} else if(strcmp("utility.library",libname) == 0){
		cpu->d[0] = lib_utility_base.entry->startAddr;
	} else if(strcmp("gadtools.library",libname) == 0){
		cpu->d[0] = lib_gadtools_base.entry->startAddr;
	} else if(strcmp("icon.library",libname) == 0){
		cpu->d[0] = lib_icon_base.entry->startAddr;
	} else if(strcmp("layers.library",libname) == 0){
		cpu->d[0] = lib_layers_base.entry->startAddr;
	} else if(strcmp("locale.library",libname) == 0){
		cpu->d[0] = lib_locale_base.entry->startAddr;
	} else {
		DEBUG(2) dprintf("Warning: exec.library: lib_exec_f_OpenLibrary: Library \"%s\" is not yet emulated.\n",libname);
	}

	return HOOK_DONE;
}
