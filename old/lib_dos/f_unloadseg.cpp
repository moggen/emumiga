/*
lib_dos/f_unloadseg.c - dos.library UnLoadSeg function

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

#include <stdio.h>
#include <stdlib.h>

#include <proto/dos.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../hook.h"
#include "../emulator.h"

/****************************
 Function: UnLoadSeg
   Vector: 26
   Offset: -156
Arguments: D1.L BCPL pointer to first segment
  Returns: D0.L Success, 0=Fail or NULL pointer supplied
****************************/
int lib_dos_f_UnLoadSeg(emumsg_syscall_t *msg)
{
	cpu_t *cpu;
	uint32_t thisseg, tmp;
	int rc;

	DEBUG(4) dprintf("dos.library: UnLoadSeg() called\n");

	cpu = msg->proc->cpu;

	thisseg = cpu->d[1] << 2;
	if(thisseg==0){
		cpu->d[0]=0;
		return HOOK_DONE;
	}
	while(thisseg != 0)
	{
		rc = READMEM_32(thisseg, &tmp);
		if(rc) return rc;
		tmp <<= 2;
		vfreemem(thisseg);
		thisseg = tmp;
	}
	cpu->d[0]=1;

	return HOOK_DONE;
}
