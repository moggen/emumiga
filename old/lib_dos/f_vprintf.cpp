/*
lib_exec/f_vprintf.c - dos.library VPrintf function

Copyright (C) 2009, 2010  Magnus Öberg

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
#include <proto/dos.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../hook.h"
#include "../emulator.h"

#define LIB_DOS_F_VPRINTF_BUFFER_SIZE 1000

/****************************
 Function: VPrintf
   Vector: 159
   Offset: -954
Arguments: D1.L pointer to format string
           D2.L pointer to data array
  Returns: D0.L number of bytes written
****************************/
int lib_dos_f_VPrintf(emumsg_syscall_t *msg)
{
	cpu_t *cpu;
	uint8_t *fptr, *bptr, *sptr;
	uint8_t ch, *buf;
	uint32_t fvptr, dvptr;
	int rc, status, f_long;
	uint16_t data16;
	uint32_t data32;

	DEBUG(4) dprintf("dos.library: lib_dos_f_VPrintf() called\n");
	cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  Format string (D1): 0x%x, Data stream (D2): 0x%x\n",cpu->d[1],cpu->d[2]);

	if(cpu->d[1] == 0) {
		DEBUG(1) dprintf("Warning: dos.library: lib_dos_f_VPrintf() called with NULL format string, skipping\n");
		return HOOK_DONE;
	}

	/* TODO: Implement some kind of check for MMU entry ending within
	         the string
	*/

        /* Allocate a native buffer for the converted native data stream */
        buf = (uint8_t *)AllocVec(LIB_DOS_F_VPRINTF_BUFFER_SIZE,MEMF_PUBLIC|MEMF_CLEAR);
        if(buf == NULL) {
                dprintf("Error: dos.library: lib_dos_f_VPrintf: AllocVec failed for buffer\n");
                return HOOK_END_PROC;
        }
	bptr = buf;

	/* Loop through the string to build a special mapping for the
	   data stream */
	fvptr = cpu->d[1];
	dvptr = cpu->d[2];
	status = 0;
	f_long = 0;

	rc = READMEM_8(fvptr++, &ch);
	if(rc) return rc;

	while(ch != '\0'){
		if(status == 0) {
			/* Idle, look for a % */
			if(ch == '%') {
                                f_long = 0;
                                status = 1;
                        }
                } else {
			/* Got a % */
                        if(ch=='-' || (ch>='0' && ch<='9') || ch=='.') {
				/* Format parameters */

                        } else if(ch=='l') {
				/* Long indicator */
                                f_long = 1;

                        } else if(ch=='d' || ch=='u' || ch=='x' || ch=='c') {
				/* Signed decimal, unsigned decimal,
				   hexadecimal or character */

				if(f_long) {
					/* 32bit data */
					rc = READMEM_32(dvptr, &data32);
					if(rc) return rc;
					dvptr += 4;

					*((ULONG *)bptr)=(ULONG)data32;
					bptr += sizeof(LONG);

				} else {
					/* 16bit data */
					rc = READMEM_16(dvptr, &data16);
					if(rc) return rc;
					dvptr += 2;

					*((UWORD *)bptr)=(UWORD)data32;
					bptr += sizeof(UWORD);

				}
				status = 0;

                        } else if(ch=='s') {
				/* String */

				/* 32bit pointer */
				rc = READMEM_32(dvptr, &data32);
				if(rc) return rc;
				dvptr += 4;

				/* Get native pointer to string */
				sptr = vptr(data32);

				/* TODO: Check mappings */

				*((APTR *)bptr)=(APTR)sptr;
				bptr += sizeof(APTR);

				status = 0;

                        } else if(ch=='b') {
				/* BString */

				/* 32bit pointer */
				rc = READMEM_32(dvptr, &data32);
				if(rc) return rc;
				dvptr += 4;

				/* BPTR -> APTR */
				data32 <<= 2;

				/* Get native pointer to string */
				sptr = vptr(data32);

				/* TODO: Check mappings */

				*((BPTR *)bptr)=MKBADDR((APTR)sptr);
				bptr += sizeof(BPTR);

				status = 0;

                        } else {
				/* Literal % or illegal char */
                                status = 0;
                        }
                }
		rc = READMEM_8(fvptr++, &ch);
		if(rc) return rc;
        }

	fptr = vptr(cpu->d[1]);

	DEBUG(5) dprintf("  Format string (native): %p, Data stream (native): %p\n",fptr,buf);

	/* Prepare syscall */
	msg->arg[0]._strptr = (STRPTR)fptr;
	msg->arg[1]._aptr = (APTR)buf;

	/* Remember buffer */
	msg->internalPtr = buf;

	return HOOK_SYSCALL;
}


int lib_dos_f_VPrintf_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */

	msg->arg[0]._long = VPrintf(msg->arg[0]._strptr,(IPTR *)msg->arg[1]._aptr);

	return HOOK_DONE;
}

int lib_dos_f_VPrintf_3(emumsg_syscall_t *msg)
{
	cpu_t *cpu;
	uint8_t *buf;

	DEBUG(4) dprintf("dos.library: lib_dos_f_VPrintf_3() called\n");
	cpu = msg->proc->cpu;

	/* Recall buffer */
	buf = (uint8_t *)msg->internalPtr;

	/* Return value */
	cpu->d[0] = (uint32_t)msg->arg[0]._long;

	/* Free memory for buffer */
	FreeVec(buf);

	DEBUG(5) dprintf("  Bytes written (D0): 0x%x\n",cpu->d[0]);

	return HOOK_DONE;
}
