/*
lib_exec/functions.h - Header file for exec.library syscall functions

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

#ifndef _LIB_EXEC_FUNCTIONS_H_
#define _LIB_EXEC_FUNCTIONS_H_

#include "../emulator.h"

/* 33, AllocMem */
int lib_exec_f_AllocMem(emumsg_syscall_t *msg);
int lib_exec_f_AllocMem_2(emumsg_syscall_t *msg);
int lib_exec_f_AllocMem_3(emumsg_syscall_t *msg);

/* 35, FreeMem */
int lib_exec_f_FreeMem(emumsg_syscall_t *msg);
int lib_exec_f_FreeMem_2(emumsg_syscall_t *msg);
int lib_exec_f_FreeMem_3(emumsg_syscall_t *msg);

/* 49, FindTask */
int lib_exec_f_FindTask(emumsg_syscall_t *msg);

/* 51, SetSignal */
int lib_exec_f_SetSignal(emumsg_syscall_t *msg);
int lib_exec_f_SetSignal_2(emumsg_syscall_t *msg);
int lib_exec_f_SetSignal_3(emumsg_syscall_t *msg);

/* 53, Wait */
int lib_exec_f_Wait(emumsg_syscall_t *msg);
int lib_exec_f_Wait_2(emumsg_syscall_t *msg);
int lib_exec_f_Wait_3(emumsg_syscall_t *msg);

/* 62, GetMsg */
int lib_exec_f_GetMsg(emumsg_syscall_t *msg);
int lib_exec_f_GetMsg_2(emumsg_syscall_t *msg);
int lib_exec_f_GetMsg_3(emumsg_syscall_t *msg);

/* 63, ReplyMsg */
int lib_exec_f_ReplyMsg(emumsg_syscall_t *msg);
int lib_exec_f_ReplyMsg_2(emumsg_syscall_t *msg);
int lib_exec_f_ReplyMsg_3(emumsg_syscall_t *msg);

/* 69, CloseLibrary */
int lib_exec_f_CloseLibrary(emumsg_syscall_t *msg);

/* 74, OpenDevice */
int lib_exec_f_OpenDevice(emumsg_syscall_t *msg);
int lib_exec_f_OpenDevice_2(emumsg_syscall_t *msg);
int lib_exec_f_OpenDevice_3(emumsg_syscall_t *msg);

/* 75, CloseDevice */
int lib_exec_f_CloseDevice(emumsg_syscall_t *msg);
int lib_exec_f_CloseDevice_2(emumsg_syscall_t *msg);
int lib_exec_f_CloseDevice_3(emumsg_syscall_t *msg);

/* 77, SendIO */
int lib_exec_f_SendIO(emumsg_syscall_t *msg);
int lib_exec_f_SendIO_2(emumsg_syscall_t *msg);
int lib_exec_f_SendIO_3(emumsg_syscall_t *msg);

/* 78, CheckIO */
int lib_exec_f_CheckIO(emumsg_syscall_t *msg);
int lib_exec_f_CheckIO_2(emumsg_syscall_t *msg);
int lib_exec_f_CheckIO_3(emumsg_syscall_t *msg);

/* 79, WaitIO */
int lib_exec_f_WaitIO(emumsg_syscall_t *msg);
int lib_exec_f_WaitIO_2(emumsg_syscall_t *msg);
int lib_exec_f_WaitIO_3(emumsg_syscall_t *msg);

/* 80, AbortIO */
int lib_exec_f_AbortIO(emumsg_syscall_t *msg);
int lib_exec_f_AbortIO_2(emumsg_syscall_t *msg);
int lib_exec_f_AbortIO_3(emumsg_syscall_t *msg);

/* 87, RawDoFmt */
int lib_exec_f_RawDoFmt(emumsg_syscall_t *msg);
int lib_exec_f_RawDoFmt_2(emumsg_syscall_t *msg);
int lib_exec_f_RawDoFmt_3(emumsg_syscall_t *msg);

/* -87, RawDoFmt callback */
int lib_exec_f_RawDoFmt_cb(emumsg_syscall_t *msg);
int lib_exec_f_RawDoFmt_cb_2(emumsg_syscall_t *msg);
int lib_exec_f_RawDoFmt_cb_3(emumsg_syscall_t *msg);

/* 92, OpenLibrary */
int lib_exec_f_OpenLibrary(emumsg_syscall_t *msg);

/* 104, CopyMem */
int lib_exec_f_CopyMem(emumsg_syscall_t *msg);

/* 109, CreateIORequest */
int lib_exec_f_CreateIORequest(emumsg_syscall_t *msg);
int lib_exec_f_CreateIORequest_2(emumsg_syscall_t *msg);
int lib_exec_f_CreateIORequest_3(emumsg_syscall_t *msg);

/* 110, DeleteIORequest */
int lib_exec_f_DeleteIORequest(emumsg_syscall_t *msg);
int lib_exec_f_DeleteIORequest_2(emumsg_syscall_t *msg);
int lib_exec_f_DeleteIORequest_3(emumsg_syscall_t *msg);

/* 111, CreateMsgPort */
int lib_exec_f_CreateMsgPort(emumsg_syscall_t *msg);
int lib_exec_f_CreateMsgPort_2(emumsg_syscall_t *msg);
int lib_exec_f_CreateMsgPort_3(emumsg_syscall_t *msg);

/* 112, DeleteMsgPort */
int lib_exec_f_DeleteMsgPort(emumsg_syscall_t *msg);
int lib_exec_f_DeleteMsgPort_2(emumsg_syscall_t *msg);
int lib_exec_f_DeleteMsgPort_3(emumsg_syscall_t *msg);

/* 114, AllocVec */
int lib_exec_f_AllocVec(emumsg_syscall_t *msg);
int lib_exec_f_AllocVec_2(emumsg_syscall_t *msg);
int lib_exec_f_AllocVec_3(emumsg_syscall_t *msg);

/* 115, FreeVec */
int lib_exec_f_FreeVec(emumsg_syscall_t *msg);
int lib_exec_f_FreeVec_2(emumsg_syscall_t *msg);
int lib_exec_f_FreeVec_3(emumsg_syscall_t *msg);

#endif
