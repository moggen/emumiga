/*
lib_dos/f_readags.c - dos.library ReadArgs function

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dosextens.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../hook.h"
#include "../emulator.h"

#include "s_rdargs.h"

/****************************
 Function: ReadArgs
   Vector: 133
   Offset: -798
Arguments: D1.L pointer to argument string
           D2.L pointer to array of longs
           D3.L pointer to structure RDArgs
  Returns: D0.L pointer to structure RDArgs
****************************/
int lib_dos_f_ReadArgs(emumsg_syscall_t *msg)
{
        cpu_t *cpu;
	struct RDArgs *rdargs_real;
	lib_dos_s_rdargs_t *rdargs;
	int slen, i;
	uint8_t *controlStr, *sptr, prevch;
	uint32_t longdata;
	uint8_t *argstring;

	DEBUG(4) dprintf("dos.library: lib_dos_f_ReadArgs() called\n");
        cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  Argument string (D1): 0x%x, Ptr array (D2): 0x%x, Ptr RDArgs (D3): 0x%x\n",cpu->d[1],cpu->d[2],cpu->d[3]);

	if(cpu->d[3] != 0) {
		dprintf("Error: dos.library: ReadArgs: Supplying a RDArgs pointer is not supported\n");
		return HOOK_END_PROC;
	}

	/* TODO: Implement some kind of check for MMU entry ending within
                 the string
	*/

	sptr = vptr(cpu->d[1]);
	if(sptr == NULL) {
		/* Illegal VADDR */
		dprintf("Error: dos.library: ReadArgs: String pointer adress is invalid. Ptr=0x%x\n",cpu->d[1]);
		return HOOK_END_PROC;
	}

	DEBUG(5) dprintf("  Argument string (native): %p\n",sptr);

	slen = strlen((char *)sptr);
	if(slen==0) {
		/* Nothing to do.. */
		cpu->d[0]=0;
		return HOOK_DONE;
	}

	controlStr = sptr;

/* DEBUG
	dprintf("Control string: %s\n",controlStr);
*/

	/* TODO: Is AllocDosObject suitable to use from the library process? */

	/* Prepare our own RDArgs structure */
	rdargs_real = (struct RDArgs *)AllocDosObject(DOS_RDARGS, NULL);
	if(rdargs_real == NULL) {
		dprintf("Error: dos.library: ReadArgs: Could not allocate dos object for struct RDArgs\n");
		return HOOK_END_PROC;
	}

	/* Get mapped structure */
	rdargs = lib_dos_s_rdargs_get_real(rdargs_real);
	if(rdargs == 0) {
		/* Error */
		dprintf("Error: dos.library: ReadArgs: Failed to create RDArgs mapping\n");
		FreeDosObject(DOS_RDARGS, rdargs_real);
		return HOOK_END_PROC;
	}

	rdargs->cnt = 1;
	rdargs->type[rdargs->cnt-1] = LIB_DOS_S_RDARGS_ARGTYPE_STRING;
	prevch = '\0';
	while(*sptr != '\0'){
		if(*sptr == ','){
			/* New arg */
			rdargs->cnt++;
			if(rdargs->cnt > LIB_DOS_S_RDARGS_ARGS_MAX) {
				/* Too many */
				dprintf("Error: dos.library: ReadArgs: Too many arguments\n");
				mmu_delEntry(rdargs->entry);
				FreeDosObject(DOS_RDARGS, rdargs_real);
				return HOOK_END_PROC;
			}
			rdargs->type[rdargs->cnt-1] = LIB_DOS_S_RDARGS_ARGTYPE_STRING;
		}
		if(prevch == '/'){
			switch(*sptr) {
				case 's':
				case 'S':
				case 't':
				case 'T':
					rdargs->type[rdargs->cnt-1] = LIB_DOS_S_RDARGS_ARGTYPE_BOOLEAN;
					break;

				case 'n':
				case 'N':
					rdargs->type[rdargs->cnt-1] = LIB_DOS_S_RDARGS_ARGTYPE_INTEGER;
 					break;

				case 'm':
				case 'M':
					rdargs->type[rdargs->cnt-1] = LIB_DOS_S_RDARGS_ARGTYPE_STRINGARRAY;
					break;

				default:
					break;
			}
		}

		prevch = *sptr++;
	}

/* DEBUG
	for(i=0;i < rdargs->cnt;i++) {
		dprintf("Arg %d type %d\n",i,rdargs->type[i]);
	}
*/

	/* Init defaults */
	for(i=0;i < rdargs->cnt;i++) {
		if(READMEM_32(cpu->d[2]+4*i, &longdata)) {
			/* Read failed */
			dprintf("Error: dos.library: ReadArgs: Failed to read initially set values\n");
			mmu_delEntry(rdargs->entry);
			FreeDosObject(DOS_RDARGS, rdargs_real);
			return HOOK_END_PROC;
		}
		rdargs->data[i] = (IPTR)longdata;
		if(longdata != 0) {
			if(rdargs->type[i] == LIB_DOS_S_RDARGS_ARGTYPE_STRING ||
			   rdargs->type[i] == LIB_DOS_S_RDARGS_ARGTYPE_STRINGARRAY) {
				/* A string was defined. Set the IPTR to point to the struct.
				   This way we can determine if ReadArgs() has set a new pointer
                                   value without having to set up mappings for the original strings.
				   ReadArgs() is not reading the initial strings anyway.
				*/
				rdargs->data[i] = (IPTR)rdargs;
			}
		}
	}

	/* Init RDArgs CSource with our argument string */
	argstring = vptr(msg->proc->vaddr_args);
	rdargs_real->RDA_Source.CS_Buffer = argstring;
	rdargs_real->RDA_Source.CS_Length = strlen((char *)argstring);
	rdargs_real->RDA_Source.CS_CurChr = 0;

	/* Prepare syscall */
	msg->arg[0]._strptr = (STRPTR)controlStr;
	msg->arg[1]._aptr = (APTR)rdargs->data;
	msg->arg[2]._aptr = (APTR)rdargs_real;

	/* Remember the rdargs structure */
	msg->internalPtr = rdargs;

	/* Do syscall */
	return HOOK_SYSCALL;
}

int lib_dos_f_ReadArgs_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */

	msg->arg[0]._aptr = ReadArgs(msg->arg[0]._strptr,(IPTR *)msg->arg[1]._aptr,(struct RDArgs *)msg->arg[2]._aptr);

	return HOOK_DONE;
}

int lib_dos_f_ReadArgs_3(emumsg_syscall_t *msg)
{
        cpu_t *cpu;
	struct RDArgs *rdargs_real, *rdargs_real2;
	lib_dos_s_rdargs_t *rdargs;
	int i, slen;

	DEBUG(4) dprintf("dos.library: lib_dos_f_ReadArgs_3() called\n");

        cpu = msg->proc->cpu;

	/* Recall rdargs structure */
	rdargs = (lib_dos_s_rdargs_t *)msg->internalPtr;

	/* Find real rdargs */
	rdargs_real = rdargs->real;

	DEBUG(5) dprintf("  RDArgs (native): %p\n",rdargs_real);
	DEBUG(5) dprintf("  RDArgs (D0): 0x%x\n",rdargs->entry->startAddr);

	/* An RDArgs structure returned? */
	rdargs_real2 = (struct RDArgs *)msg->arg[0]._aptr;
	if(rdargs_real2 == NULL) {
		/* Something went wrong */
		cpu->d[0]=0;
		mmu_delEntry(rdargs->entry);
		FreeDosObject(DOS_RDARGS, rdargs_real);
		return HOOK_DONE;
	}

	if(rdargs_real2 != rdargs_real) {
		/* Should return the same! */
		dprintf("Error: dos.library: ReadArgs: Other RDArgs structure than provided was returned\n");
		mmu_delEntry(rdargs->entry);
		FreeDosObject(DOS_RDARGS, rdargs_real);
		return HOOK_END_PROC;
	}

	/* Set results */
	for(i=0;i < rdargs->cnt;i++) {
		if(rdargs->data[i] == 0) {
			if(WRITEMEM_32(0,cpu->d[2]+4*i)) {
				/* Write failed */
				dprintf("Error: dos.library: ReadArgs: Failed to write value\n");
				/* Delete the mapping and free argtypes */
				mmu_delEntry(rdargs->entry);
				FreeDosObject(DOS_RDARGS, rdargs_real);
				return HOOK_END_PROC;
			}
		} else {
			if(rdargs->type[i] == LIB_DOS_S_RDARGS_ARGTYPE_STRING ||
			   rdargs->type[i] == LIB_DOS_S_RDARGS_ARGTYPE_STRINGARRAY) {
				/* String data type, if it is pointing to something other
				   than the argtypes struct it has changed and requires a new
				   mapping. */
				if(rdargs->data[i] != (IPTR)rdargs) {
					/* Yes it has changed! */
					slen = strlen((char *)rdargs->data[i]);
					rdargs->argmem[i] = vallocmem_rom((uint8_t *)rdargs->data[i], slen+1, NULL);
					if(rdargs->argmem[i] == 0) {
						dprintf("Error: dos.library: ReadArgs: Failed to create string mapping\n");
						/* Delete the mapping and free argtypes */
						mmu_delEntry(rdargs->entry);
						FreeDosObject(DOS_RDARGS, rdargs_real);
						return HOOK_END_PROC;
					}
				}
			} else {
				/* An integer type, just write it */
				if(WRITEMEM_32((uint32_t)rdargs->data[i],cpu->d[2]+4*i)) {
					/* Write failed */
					dprintf("Error: dos.library: ReadArgs: Failed to write value\n");
					mmu_delEntry(rdargs->entry);
					FreeDosObject(DOS_RDARGS, rdargs_real);
					return HOOK_END_PROC;
				}
			}
		}
	}

	/* All ok, return mapping */
	cpu->d[0] = rdargs->entry->startAddr;

	return HOOK_DONE;
}
