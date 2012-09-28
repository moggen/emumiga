/*
lib_exec/f_rawdofmt.c - exec.library RawDoFmt function

Copyright (C) 2009  Magnus Öberg

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

#include <aros/asmcall.h>
#include <proto/exec.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../hook.h"
#include "../emulator.h"

#define LIB_EXEC_F_RAWDOFMT_BUFFER_SIZE 1000

struct lib_exec_f_rawdofmt_struct {
        uint8_t *buf;
        uint32_t end_vptr;
};

typedef struct lib_exec_f_rawdofmt_struct lib_exec_f_rawdofmt_t;

static int is_init = 0;

static uint8_t helper_data[6];
static uint32_t helper_vaddr;

int lib_exec_f_rawdofmt_init(uint16_t mnum)
{
	DEBUG(4) dprintf("exec.library: lib_exec_f_rawdofmt_init() called\n");

        /* Set up the RawDoFmt helper code */
	helper_data[0] = 0xFD;
	helper_data[1] = 0x80;
	helper_data[2] = (uint8_t)(mnum >> 8);
	helper_data[3] = (uint8_t)(mnum & 0xff);
	helper_data[4] = 0xff;
	helper_data[5] = (uint8_t)-87;

	helper_vaddr = vallocmem_rom(helper_data,6,"!exec.helper.RawDoFmt");
        if(helper_vaddr == 0) {
                dprintf("exec.library: lib_exec_f_rawdofmt_init: vallocmem_rom() failed\n");
                return 1;
        }

	is_init = 1;
	return 0;
}

void lib_exec_f_rawdofmt_cleanup()
{
	DEBUG(4) dprintf("exec.library: lib_exec_f_rawdofmt_cleanup() called\n");

        if( !is_init ) {
                DEBUG(2) dprintf("exec.library: lib_exec_f_rawdofmt_cleanup: Not initialized\n");
                return;
        }

        /* Delete mapping for RawDoFmt helper */
        vfreemem_rom(helper_vaddr);

	is_init = 0;
}


AROS_UFH2(void, lib_exec_f_RawDoFmt_callback,
	AROS_UFHA(UBYTE, ch,   D0),
	AROS_UFHA(APTR , data, A3))
{
	AROS_USERFUNC_INIT

	emumsg_syscall_t *msg;
	cpu_t *cpu;
	int ret;

	msg = (emumsg_syscall_t *)data;
	cpu = msg->proc->cpu;

	/* Save PC */
	cpu->a[7] -= 4;
	WRITEMEM_32(cpu->pc, cpu->a[7]);

	/* Set return address for RTS in the end of the callback
           so it executes the special TRAP */
	cpu->a[7] -= 4;
	WRITEMEM_32(helper_vaddr, cpu->a[7]);

	/* Resume at callback function */
	cpu->pc = cpu->a[2];

	/* Provide the char */
	cpu->d[0] = (uint32_t)ch;

	/* Set func to -87. call_hook3 should not run
	   lib_exec_f_RawDoFmt_3()  now. */
	msg->func = -87;

	/* Signal back to emulator to continue emulating */
	ReplyMsg((struct Message *)msg);

        /* Handle syscalls from callback */
        ret = emulator_doEvents(msg->proc->syscall_port);

	/* TODO: How should errors in callbacks be handled here? */

	/* Callback has reached RTS, and the special TRAP */

	/* Restore the original PC */
	READMEM_32(cpu->a[7], &(cpu->pc));
	cpu->a[7] += 4;

	/* Return back to AROS' RawDoFmt for more processing */
	AROS_USERFUNC_EXIT
}

/****************************
 Function: RawDoFmt
   Vector: 87
   Offset: -522
Arguments: A0.L pointer to format string
           A1.L pointer to data stream
           A2.L pointer to putchar function
           A3.L parameter value to putchar function
  Returns: D0.L pointer to next item in the data stream after this operation
  Comment: The putchar function is called with these arguments
           D0.B the char to output
           A3.L the parameter value
           The function is called with the NULL char ('\0') at the end
****************************/
int lib_exec_f_RawDoFmt(emumsg_syscall_t *msg)
{
	cpu_t *cpu;
	uint8_t *fptr, *bptr, *sptr;
	uint8_t ch, *buf;
	uint32_t fvptr, dvptr;
	int rc, status, f_long;
	uint16_t data16;
	uint32_t data32;
	lib_exec_f_rawdofmt_t *internal;

	DEBUG(4) dprintf("exec.library: lib_exec_f_RawDoFmt() called\n");
	cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  Format string (A0): 0x%x, Data stream (A1): 0x%x, Putchar callback (A2): 0x%x, Parameter (A3): 0x%x\n",cpu->a[0],cpu->a[1],cpu->a[2],cpu->a[3]);

	if(cpu->a[0] == 0) {
		DEBUG(1) dprintf("Warning: exec.library: lib_exec_f_RawDoFmt() called with NULL format string, skipping\n");
		return HOOK_DONE;
	}
	if(cpu->a[2] == 0) {
		DEBUG(1) dprintf("Warning: exec.library: lib_exec_f_RawDoFmt() called with NULL putchar callback, skipping\n");
		return HOOK_DONE;
	}

	/* TODO: Implement some kind of check for MMU entry ending within
	         the string
	*/

	/* Allocate a native buffer for the converted native data stream */
	internal = (lib_exec_f_rawdofmt_t *)AllocVec(sizeof(lib_exec_f_rawdofmt_t), MEMF_PUBLIC|MEMF_CLEAR);
	if(internal == NULL) {
		dprintf("Error: exec.library: lib_exec_f_RawDoFmt: AllocVec failed for internal struct\n");
		return HOOK_END_PROC;
	}

	/* Allocate a native buffer for the converted native data stream */
	buf = (uint8_t *)AllocVec(LIB_EXEC_F_RAWDOFMT_BUFFER_SIZE,MEMF_PUBLIC|MEMF_CLEAR);
	if(buf == NULL) {
		FreeVec(internal);
		dprintf("Error: exec.library: lib_exec_f_RawDoFmt: AllocVec failed for buffer\n");
		return HOOK_END_PROC;
	}
	internal->buf = buf;
	bptr = buf;

	/* Loop through the string to build a special mapping for the
	   data stream */
	fvptr = cpu->a[0];
	dvptr = cpu->a[1];
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

	/* Store away data stream pointer to be used as return value */
	internal->end_vptr = dvptr;

	fptr = vptr(cpu->a[0]);

	DEBUG(5) dprintf("  Format string (native): %p, Data stream (native): %p\n",fptr,buf);

	/* Prepare syscall */
	msg->arg[0]._strptr = (STRPTR)fptr;
	msg->arg[1]._aptr = (APTR)buf;
	msg->arg[2]._aptr = (APTR)&lib_exec_f_RawDoFmt_callback;
	msg->arg[3]._aptr = (APTR)msg;

	/* Remember internal data */
	msg->internalPtr = internal;

	return HOOK_SYSCALL;
}


int lib_exec_f_RawDoFmt_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */

	msg->arg[0]._aptr = RawDoFmt(msg->arg[0]._strptr,msg->arg[1]._aptr,(void (*)())msg->arg[2]._aptr,msg->arg[3]._aptr);

	/* Make sure that func is 87 so that call_hook3 works */
	msg->func = 87;

	return HOOK_DONE;
}

int lib_exec_f_RawDoFmt_3(emumsg_syscall_t *msg)
{
	cpu_t *cpu;
	lib_exec_f_rawdofmt_t *internal;

	DEBUG(4) dprintf("exec.library: lib_exec_f_RawDoFmt_3() called\n");
	cpu = msg->proc->cpu;

	/* Recall internal data */
	internal = (lib_exec_f_rawdofmt_t *)msg->internalPtr;

	/* Return value */
	cpu->d[0] = internal->end_vptr;

	/* Free memory for buffer and internal */
	FreeVec(internal->buf);
	FreeVec(internal);

	DEBUG(5) dprintf("  Next item (D0): 0x%x\n",cpu->d[0]);

	return HOOK_DONE;
}

int lib_exec_f_RawDoFmt_cb(emumsg_syscall_t *msg __attribute__((unused)))
{
	DEBUG(4) dprintf("exec.library: lib_exec_f_RawDoFmt_cb() called\n");

	/* Do nothing, just pause emulation and pass on to native process */
	return HOOK_SYSCALL;
}

int lib_exec_f_RawDoFmt_cb_2(emumsg_syscall_t *msg __attribute__((unused)))
{
	/* Special signal to end emulator_doEvents */
	return HOOK_CALLBACK;
}

int lib_exec_f_RawDoFmt_cb_3(emumsg_syscall_t *msg __attribute__((unused)))
{
	DEBUG(4) dprintf("exec.library: lib_exec_f_RawDoFmt_cb_3() called\n");

	/* No action */
	return HOOK_DONE;
}

