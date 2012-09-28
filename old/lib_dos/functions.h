/*
lib_dos/functions.h - Header file for dos.library syscall functions

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

#ifndef _LIB_DOS_FUNCTIONS_H_
#define _LIB_DOS_FUNCTIONS_H_

#include "../emulator.h"

/* 5, Open */
int lib_dos_f_Open(emumsg_syscall_t *msg);
int lib_dos_f_Open_2(emumsg_syscall_t *msg);
int lib_dos_f_Open_3(emumsg_syscall_t *msg);

/* 6, Close */
int lib_dos_f_Close(emumsg_syscall_t *msg);
int lib_dos_f_Close_2(emumsg_syscall_t *msg);
int lib_dos_f_Close_3(emumsg_syscall_t *msg);

/* 8, Write */
int lib_dos_f_Write(emumsg_syscall_t *msg);
int lib_dos_f_Write_2(emumsg_syscall_t *msg);
int lib_dos_f_Write_3(emumsg_syscall_t *msg);

/* 9, Input */
int lib_dos_f_Input(emumsg_syscall_t *msg);
int lib_dos_f_Input_2(emumsg_syscall_t *msg);
int lib_dos_f_Input_3(emumsg_syscall_t *msg);

/* 10, Output */
int lib_dos_f_Output(emumsg_syscall_t *msg);
int lib_dos_f_Output_2(emumsg_syscall_t *msg);
int lib_dos_f_Output_3(emumsg_syscall_t *msg);

/* 15, UnLock */
int lib_dos_f_UnLock(emumsg_syscall_t *msg);
int lib_dos_f_UnLock_2(emumsg_syscall_t *msg);
int lib_dos_f_UnLock_3(emumsg_syscall_t *msg);

/* 16, DupLock */
int lib_dos_f_DupLock(emumsg_syscall_t *msg);
int lib_dos_f_DupLock_2(emumsg_syscall_t *msg);
int lib_dos_f_DupLock_3(emumsg_syscall_t *msg);

/* 25, LoadSeg */
int lib_dos_f_LoadSeg(emumsg_syscall_t *msg);

/* 26, UnLoadSeg */
int lib_dos_f_UnLoadSeg(emumsg_syscall_t *msg);

/* 33, Delay */
int lib_dos_f_Delay(emumsg_syscall_t *msg);
int lib_dos_f_Delay_2(emumsg_syscall_t *msg);
int lib_dos_f_Delay_3(emumsg_syscall_t *msg);

/* 132, CheckSignal */
int lib_dos_f_CheckSignal(emumsg_syscall_t *msg);
int lib_dos_f_CheckSignal_2(emumsg_syscall_t *msg);
int lib_dos_f_CheckSignal_3(emumsg_syscall_t *msg);

/* 133, ReadArgs */
int lib_dos_f_ReadArgs(emumsg_syscall_t *msg);
int lib_dos_f_ReadArgs_2(emumsg_syscall_t *msg);
int lib_dos_f_ReadArgs_3(emumsg_syscall_t *msg);

/* 137, MatchFirst */
int lib_dos_f_MatchFirst(emumsg_syscall_t *msg);
int lib_dos_f_MatchFirst_2(emumsg_syscall_t *msg);
int lib_dos_f_MatchFirst_3(emumsg_syscall_t *msg);

/* 138, MatchNext */
int lib_dos_f_MatchNext(emumsg_syscall_t *msg);
int lib_dos_f_MatchNext_2(emumsg_syscall_t *msg);
int lib_dos_f_MatchNext_3(emumsg_syscall_t *msg);

/* 139, MatchEnd */
int lib_dos_f_MatchEnd(emumsg_syscall_t *msg);
int lib_dos_f_MatchEnd_2(emumsg_syscall_t *msg);
int lib_dos_f_MatchEnd_3(emumsg_syscall_t *msg);

/* 143, FreeArgs */
int lib_dos_f_FreeArgs(emumsg_syscall_t *msg);
int lib_dos_f_FreeArgs_2(emumsg_syscall_t *msg);
int lib_dos_f_FreeArgs_3(emumsg_syscall_t *msg);

/* 158, PutStr */
int lib_dos_f_PutStr(emumsg_syscall_t *msg);
int lib_dos_f_PutStr_2(emumsg_syscall_t *msg);
int lib_dos_f_PutStr_3(emumsg_syscall_t *msg);

/* 159, VPrintf */
int lib_dos_f_VPrintf(emumsg_syscall_t *msg);
int lib_dos_f_VPrintf_2(emumsg_syscall_t *msg);
int lib_dos_f_VPrintf_3(emumsg_syscall_t *msg);

#endif
