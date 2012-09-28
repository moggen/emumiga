/*
lib_gadtools/functions.h - Header file for gadtools.library syscall functions

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

#ifndef _LIB_GADTOOLS_FUNCTIONS_H_
#define _LIB_GADTOOLS_FUNCTIONS_H_

#include "../emulator.h"

/* 5, CreateGadgetA */
int lib_gadtools_f_CreateGadgetA(emumsg_syscall_t *msg);
int lib_gadtools_f_CreateGadgetA_2(emumsg_syscall_t *msg);
int lib_gadtools_f_CreateGadgetA_3(emumsg_syscall_t *msg);

/* 6, FreeGadgets */
int lib_gadtools_f_FreeGadgets(emumsg_syscall_t *msg);
int lib_gadtools_f_FreeGadgets_2(emumsg_syscall_t *msg);
int lib_gadtools_f_FreeGadgets_3(emumsg_syscall_t *msg);

/* 8, CreateMenusA */
int lib_gadtools_f_CreateMenusA(emumsg_syscall_t *msg);
int lib_gadtools_f_CreateMenusA_2(emumsg_syscall_t *msg);
int lib_gadtools_f_CreateMenusA_3(emumsg_syscall_t *msg);

/* 9, FreeMenus */
int lib_gadtools_f_FreeMenus(emumsg_syscall_t *msg);
int lib_gadtools_f_FreeMenus_2(emumsg_syscall_t *msg);
int lib_gadtools_f_FreeMenus_3(emumsg_syscall_t *msg);

/* 11, LayoutMenusA */
int lib_gadtools_f_LayoutMenusA(emumsg_syscall_t *msg);
int lib_gadtools_f_LayoutMenusA_2(emumsg_syscall_t *msg);
int lib_gadtools_f_LayoutMenusA_3(emumsg_syscall_t *msg);

/* 12, GT_GetIMsg */
int lib_gadtools_f_GT_GetIMsg(emumsg_syscall_t *msg);
int lib_gadtools_f_GT_GetIMsg_2(emumsg_syscall_t *msg);
int lib_gadtools_f_GT_GetIMsg_3(emumsg_syscall_t *msg);

/* 13, GT_ReplyIMsg */
int lib_gadtools_f_GT_ReplyIMsg(emumsg_syscall_t *msg);
int lib_gadtools_f_GT_ReplyIMsg_2(emumsg_syscall_t *msg);
int lib_gadtools_f_GT_ReplyIMsg_3(emumsg_syscall_t *msg);

/* 14, GT_RefreshWindow */
int lib_gadtools_f_GT_RefreshWindow(emumsg_syscall_t *msg);
int lib_gadtools_f_GT_RefreshWindow_2(emumsg_syscall_t *msg);
int lib_gadtools_f_GT_RefreshWindow_3(emumsg_syscall_t *msg);

/* 19, CreateContext */
int lib_gadtools_f_CreateContext(emumsg_syscall_t *msg);
int lib_gadtools_f_CreateContext_2(emumsg_syscall_t *msg);
int lib_gadtools_f_CreateContext_3(emumsg_syscall_t *msg);

/* 20, DrawBevelBoxA */
int lib_gadtools_f_DrawBevelBoxA(emumsg_syscall_t *msg);
int lib_gadtools_f_DrawBevelBoxA_2(emumsg_syscall_t *msg);
int lib_gadtools_f_DrawBevelBoxA_3(emumsg_syscall_t *msg);

/* 21, GetVisualInfoA */
int lib_gadtools_f_GetVisualInfoA(emumsg_syscall_t *msg);
int lib_gadtools_f_GetVisualInfoA_2(emumsg_syscall_t *msg);
int lib_gadtools_f_GetVisualInfoA_3(emumsg_syscall_t *msg);

/* 22, FreeVisualInfo */
int lib_gadtools_f_FreeVisualInfo(emumsg_syscall_t *msg);
int lib_gadtools_f_FreeVisualInfo_2(emumsg_syscall_t *msg);
int lib_gadtools_f_FreeVisualInfo_3(emumsg_syscall_t *msg);

#endif
