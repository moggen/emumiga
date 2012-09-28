/*
lib_graphics/functions.h - Header file for graphics.library syscall functions

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

#ifndef _LIB_GRAPHICS_FUNCTIONS_H_
#define _LIB_GRAPHICS_FUNCTIONS_H_

#include "../emulator.h"

/* 9, TextLength */
int lib_graphics_f_TextLength(emumsg_syscall_t *msg);
int lib_graphics_f_TextLength_2(emumsg_syscall_t *msg);
int lib_graphics_f_TextLength_3(emumsg_syscall_t *msg);

/* 10, Text */
int lib_graphics_f_Text(emumsg_syscall_t *msg);
int lib_graphics_f_Text_2(emumsg_syscall_t *msg);
int lib_graphics_f_Text_3(emumsg_syscall_t *msg);

/* 11, SetFont */
int lib_graphics_f_SetFont(emumsg_syscall_t *msg);
int lib_graphics_f_SetFont_2(emumsg_syscall_t *msg);
int lib_graphics_f_SetFont_3(emumsg_syscall_t *msg);

/* 12, OpenFont */
int lib_graphics_f_OpenFont(emumsg_syscall_t *msg);
int lib_graphics_f_OpenFont_2(emumsg_syscall_t *msg);
int lib_graphics_f_OpenFont_3(emumsg_syscall_t *msg);

/* 13, CloseFont */
int lib_graphics_f_CloseFont(emumsg_syscall_t *msg);
int lib_graphics_f_CloseFont_2(emumsg_syscall_t *msg);
int lib_graphics_f_CloseFont_3(emumsg_syscall_t *msg);

/* 30, DrawEllipse */
int lib_graphics_f_DrawEllipse(emumsg_syscall_t *msg);
int lib_graphics_f_DrawEllipse_2(emumsg_syscall_t *msg);
int lib_graphics_f_DrawEllipse_3(emumsg_syscall_t *msg);

/* 31, AreaEllipse */
int lib_graphics_f_AreaEllipse(emumsg_syscall_t *msg);
int lib_graphics_f_AreaEllipse_2(emumsg_syscall_t *msg);
int lib_graphics_f_AreaEllipse_3(emumsg_syscall_t *msg);

/* 38, WaitBlit */
int lib_graphics_f_WaitBlit(emumsg_syscall_t *msg);
int lib_graphics_f_WaitBlit_2(emumsg_syscall_t *msg);
int lib_graphics_f_WaitBlit_3(emumsg_syscall_t *msg);

/* 39, SetRast */
int lib_graphics_f_SetRast(emumsg_syscall_t *msg);
int lib_graphics_f_SetRast_2(emumsg_syscall_t *msg);
int lib_graphics_f_SetRast_3(emumsg_syscall_t *msg);

/* 40, Move */
int lib_graphics_f_Move(emumsg_syscall_t *msg);
int lib_graphics_f_Move_2(emumsg_syscall_t *msg);
int lib_graphics_f_Move_3(emumsg_syscall_t *msg);

/* 41, Draw */
int lib_graphics_f_Draw(emumsg_syscall_t *msg);
int lib_graphics_f_Draw_2(emumsg_syscall_t *msg);
int lib_graphics_f_Draw_3(emumsg_syscall_t *msg);

/* 42, AreaMove */
int lib_graphics_f_AreaMove(emumsg_syscall_t *msg);
int lib_graphics_f_AreaMove_2(emumsg_syscall_t *msg);
int lib_graphics_f_AreaMove_3(emumsg_syscall_t *msg);

/* 43, AreaDraw */
int lib_graphics_f_AreaDraw(emumsg_syscall_t *msg);
int lib_graphics_f_AreaDraw_2(emumsg_syscall_t *msg);
int lib_graphics_f_AreaDraw_3(emumsg_syscall_t *msg);

/* 44, AreaEnd */
int lib_graphics_f_AreaEnd(emumsg_syscall_t *msg);
int lib_graphics_f_AreaEnd_2(emumsg_syscall_t *msg);
int lib_graphics_f_AreaEnd_3(emumsg_syscall_t *msg);

/* 47, InitArea */
int lib_graphics_f_InitArea(emumsg_syscall_t *msg);
int lib_graphics_f_InitArea_2(emumsg_syscall_t *msg);
int lib_graphics_f_InitArea_3(emumsg_syscall_t *msg);

/* 51, RectFill */
int lib_graphics_f_RectFill(emumsg_syscall_t *msg);
int lib_graphics_f_RectFill_2(emumsg_syscall_t *msg);
int lib_graphics_f_RectFill_3(emumsg_syscall_t *msg);

/* 57, SetAPen */
int lib_graphics_f_SetAPen(emumsg_syscall_t *msg);
int lib_graphics_f_SetAPen_2(emumsg_syscall_t *msg);
int lib_graphics_f_SetAPen_3(emumsg_syscall_t *msg);

/* 58, SetBPen */
int lib_graphics_f_SetBPen(emumsg_syscall_t *msg);
int lib_graphics_f_SetBPen_2(emumsg_syscall_t *msg);
int lib_graphics_f_SetBPen_3(emumsg_syscall_t *msg);

/* 59, SetDrMd */
int lib_graphics_f_SetDrMd(emumsg_syscall_t *msg);
int lib_graphics_f_SetDrMd_2(emumsg_syscall_t *msg);
int lib_graphics_f_SetDrMd_3(emumsg_syscall_t *msg);

/* 78, InitTmpRas */
int lib_graphics_f_InitTmpRas(emumsg_syscall_t *msg);
int lib_graphics_f_InitTmpRas_2(emumsg_syscall_t *msg);
int lib_graphics_f_InitTmpRas_3(emumsg_syscall_t *msg);

/* 82, AllocRaster */
int lib_graphics_f_AllocRaster(emumsg_syscall_t *msg);
int lib_graphics_f_AllocRaster_2(emumsg_syscall_t *msg);
int lib_graphics_f_AllocRaster_3(emumsg_syscall_t *msg);

/* 83, FreeRaster */
int lib_graphics_f_FreeRaster(emumsg_syscall_t *msg);
int lib_graphics_f_FreeRaster_2(emumsg_syscall_t *msg);
int lib_graphics_f_FreeRaster_3(emumsg_syscall_t *msg);

/* 85, OrRectRegion */
int lib_graphics_f_OrRectRegion(emumsg_syscall_t *msg);
int lib_graphics_f_OrRectRegion_2(emumsg_syscall_t *msg);
int lib_graphics_f_OrRectRegion_3(emumsg_syscall_t *msg);

/* 86, NewRegion */
int lib_graphics_f_NewRegion(emumsg_syscall_t *msg);
int lib_graphics_f_NewRegion_2(emumsg_syscall_t *msg);
int lib_graphics_f_NewRegion_3(emumsg_syscall_t *msg);

/* 89, DisposeRegion */
int lib_graphics_f_DisposeRegion(emumsg_syscall_t *msg);
int lib_graphics_f_DisposeRegion_2(emumsg_syscall_t *msg);
int lib_graphics_f_DisposeRegion_3(emumsg_syscall_t *msg);

/* 115, TextExtent */
int lib_graphics_f_TextExtent(emumsg_syscall_t *msg);
int lib_graphics_f_TextExtent_2(emumsg_syscall_t *msg);
int lib_graphics_f_TextExtent_3(emumsg_syscall_t *msg);

/* 121, FindDisplayInfo */
int lib_graphics_f_FindDisplayInfo(emumsg_syscall_t *msg);
int lib_graphics_f_FindDisplayInfo_2(emumsg_syscall_t *msg);
int lib_graphics_f_FindDisplayInfo_3(emumsg_syscall_t *msg);

/* 126, GetDisplayInfoData */
int lib_graphics_f_GetDisplayInfoData(emumsg_syscall_t *msg);
int lib_graphics_f_GetDisplayInfoData_2(emumsg_syscall_t *msg);
int lib_graphics_f_GetDisplayInfoData_3(emumsg_syscall_t *msg);

/* 132, GetVPModeID */
int lib_graphics_f_GetVPModeID(emumsg_syscall_t *msg);
int lib_graphics_f_GetVPModeID_2(emumsg_syscall_t *msg);
int lib_graphics_f_GetVPModeID_3(emumsg_syscall_t *msg);

#endif
