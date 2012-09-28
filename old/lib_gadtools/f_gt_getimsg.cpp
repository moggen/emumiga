/*
lib_gadtools/f_gt_getimsg.c - gadtools.library GT_GetIMsg function

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

#include <proto/gadtools.h>
#include <exec/ports.h>
#include <intuition/intuition.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../hook.h"
#include "../emulator.h"
#include "../lib_exec/s_msgport.h"
#include "../lib_exec/s_message.h"

/****************************
 Function: GT_GetIMsg
   Vector: 12
   Offset: -72
Arguments: A0.L pointer to struct MsgPort
  Returns: D0.L pointer to stuct IntuiMessage
****************************/
int lib_gadtools_f_GT_GetIMsg(emumsg_syscall_t *msg)
{
        cpu_t *cpu;
	lib_exec_s_msgport_t *msgport;

	DEBUG(4) dprintf("gadtools.library: lib_gadtools_f_GT_GetIMsg() called\n");

        cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  MsgPort ptr (A0): 0x%x\n",cpu->a[0]);

	/* Find message port mapping */
	msgport = lib_exec_s_msgport_get_vaddr(cpu->a[0]);
	if(msgport == NULL) {
		DEBUG(1) dprintf("Error: gadtools.library: lib_gadtools_f_GT_GetIMsg: MsgPort struct pointer is invalid\n");
		return HOOK_END_PROC;
	}

	/* Check the type of the MsgPort so it handles InutiMessages */
	if(msgport->type != MMU_TYPE_INTUITION_S_INTUIMESSAGE) {
		DEBUG(1) dprintf("Error: gadtools.library: lib_gadtools_f_GT_GetIMsg: MsgPort is not set up to handle IntuiMessages\n");
		return HOOK_END_PROC;
	}

	/* Prepare syscall */
	msg->arg[0]._aptr = (APTR)msgport->real;

	/* Remember object */
	msg->internalPtr = msgport;

	/* Do syscall */
	return HOOK_SYSCALL;
}

int lib_gadtools_f_GT_GetIMsg_2(emumsg_syscall_t *msg)
{
	/* Make real syscall */
	msg->arg[0]._aptr = GT_GetIMsg(
		(struct MsgPort *)msg->arg[0]._aptr
	);

	return HOOK_DONE;
}

int lib_gadtools_f_GT_GetIMsg_3(emumsg_syscall_t *msg)
{
	cpu_t *cpu;
	lib_exec_s_msgport_t *msgport;
	struct IntuiMessage *imsg_real;
	lib_exec_s_message_t *message;

	DEBUG(4) dprintf("gadtools.library: lib_gadtools_f_GT_GetIMsg_3() called\n");

	cpu = msg->proc->cpu;

	DEBUG(5) dprintf("  IntuiMessage (native): %p\n", msg->arg[0]._aptr);

	/* Recall object */
	msgport = (lib_exec_s_msgport_t *)msg->internalPtr;

	/* Extract return values */
	imsg_real = (struct IntuiMessage *)msg->arg[0]._aptr;

	if(imsg_real == NULL) {
		cpu->d[0] = 0;
		DEBUG(5) dprintf("  IntuiMessage ptr (D0): 0x%x\n",0);
		return HOOK_DONE;
	}

	message = lib_exec_s_message_get_real((struct Message *)imsg_real);
	if(message == NULL) {
		DEBUG(1) dprintf("Error: gadtools.library: lib_gadtools_f_GT_GetIMsg_3: Could not create mapping for message\n");
		return HOOK_END_PROC;
	}

	/* Convert it to the type set up in the MsgPort */
	if(msgport->convertfn(message)) {
		DEBUG(1) dprintf("Error: gadtools.library: lib_gadtools_f_GT_GetIMsg_3: Could not convert Message to IntuiMessage\n");
		return HOOK_END_PROC;
	}

	cpu->d[0] = message->entry->startAddr;

	DEBUG(5) dprintf("  IntuiMessage ptr (D0): 0x%x\n", cpu->d[0]);

	return HOOK_DONE;
}
