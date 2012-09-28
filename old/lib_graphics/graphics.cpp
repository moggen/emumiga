/*
lib_graphics/graphics.c - Simulation of AmigaOS graphics.library

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

#include <proto/exec.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../hook.h"
#include "../emulator.h"
#include "graphics.h"
#include "functions.h"
#include "s_textfont.h"
#include "s_textattr.h"
#include "s_viewport.h"
#include "s_displayinfohandle.h"
#include "s_displayinfo.h"
#include "s_rastport.h"
#include "s_textextent.h"
#include "s_layer.h"
#include "s_bitmap.h"
#include "s_tmpras.h"
#include "s_areainfo.h"
#include "s_gelsinfo.h"
#include "s_layer_info.h"
#include "s_region.h"
#include "s_rectangle.h"

/* Graphics base object */
lib_graphics_t lib_graphics_base;

/* Hook module number */
static uint16_t mnum;

/* Init flag */
static int is_init = 0;

/* Predecls */
static int hook1(emumsg_syscall_t *msg);
static int hook2(emumsg_syscall_t *msg);
static int hook3(emumsg_syscall_t *msg);

/* Global init */
int lib_graphics_init()
{
	uint32_t vaddr;
	uint32_t cur_emu, cur_vec;
	uint16_t cur_fnum;
	uint8_t *addr;
	mmu_entry_t *entry;

	DEBUG(3) dprintf("graphics.library: lib_graphics_init() called\n");

	/* Get a hook module number */
	mnum = allocate_hook("graphics.library", hook1, hook2, hook3);

	/* Clear memory */
        memset((void *)&lib_graphics_base,0,sizeof(lib_graphics_t));

	/* Allocate virtual space for
	    - Emulator instruction + rts = 8 bytes * VECTORS
	    - Standard jmp table = 6 bytes * VECTORS
	    - Library struct
	*/
	vaddr = vallocmem_nomap((6+8)*LIB_GRAPHICS_VECTORS+LIB_GRAPHICS_SIZE);

	/* Allocate real memory for
	    - Emulator instruction + rts = 8 bytes * VECTORS
	    - Standard jmp table = 6 bytes * VECTORS
	*/
	addr = (uint8_t *)AllocVec((6+8)*LIB_GRAPHICS_VECTORS, MEMF_PUBLIC|MEMF_CLEAR);

	/* Set up binary mapping */
	entry = lib_graphics_base.vec_entry = mmu_addEntry(
                vaddr,
                (6+8)*LIB_GRAPHICS_VECTORS,
		MMU_TYPE_GRAPHICS_VECTORS,
		NULL
	);
	entry->read = mmu_fn_r_binary;
	entry->write = mmu_fn_w_binary;
	entry->nativePtr = addr;

	DEBUG(3) {
	        /* Add a symbol for it */
	        mmu_addSymbol(lib_graphics_base.vec_entry, vaddr, "!graphics.vec");
	}

	/* Loop over all vectors, cur_fnum counts down */
	cur_fnum = LIB_GRAPHICS_VECTORS;
	cur_emu = vaddr;
	cur_vec = cur_emu + 8*LIB_GRAPHICS_VECTORS;
	while(cur_fnum > 0)
	{
		/* Write emulator instruction + rts */
		WRITEMEM_16(0xFD80,cur_emu);
		WRITEMEM_16(mnum,cur_emu+2);
		WRITEMEM_16(cur_fnum,cur_emu+4);
		WRITEMEM_16(0x4E75,cur_emu+6);

		/* Write jump vector */
		WRITEMEM_16(0x4EF9,cur_vec);
		WRITEMEM_32(cur_emu,cur_vec+2);

		cur_emu += 8;
		cur_vec += 6;
		cur_fnum--;
	}

	/* Write-protect the graphics base */
	entry->write = mmu_fn_protected;

	/* Calculate library struct address */
	vaddr += (6+8)*LIB_GRAPHICS_VECTORS;

	DEBUG(3) {
	        /* Add implemented LVO symbols */
	        mmu_addSymbol(lib_graphics_base.vec_entry, vaddr-54, "!graphics.vec.TextLength");
	        mmu_addSymbol(lib_graphics_base.vec_entry, vaddr-60, "!graphics.vec.Text");
	        mmu_addSymbol(lib_graphics_base.vec_entry, vaddr-66, "!graphics.vec.SetFont");
	        mmu_addSymbol(lib_graphics_base.vec_entry, vaddr-72, "!graphics.vec.OpenFont");
	        mmu_addSymbol(lib_graphics_base.vec_entry, vaddr-78, "!graphics.vec.CloseFont");
	        mmu_addSymbol(lib_graphics_base.vec_entry, vaddr-180, "!graphics.vec.DrawEllipse");
	        mmu_addSymbol(lib_graphics_base.vec_entry, vaddr-186, "!graphics.vec.AreaEllipse");
	        mmu_addSymbol(lib_graphics_base.vec_entry, vaddr-228, "!graphics.vec.WaitBlit");
	        mmu_addSymbol(lib_graphics_base.vec_entry, vaddr-234, "!graphics.vec.SetRast");
	        mmu_addSymbol(lib_graphics_base.vec_entry, vaddr-240, "!graphics.vec.Move");
	        mmu_addSymbol(lib_graphics_base.vec_entry, vaddr-246, "!graphics.vec.Draw");
	        mmu_addSymbol(lib_graphics_base.vec_entry, vaddr-252, "!graphics.vec.AreaMove");
	        mmu_addSymbol(lib_graphics_base.vec_entry, vaddr-258, "!graphics.vec.AreaDraw");
	        mmu_addSymbol(lib_graphics_base.vec_entry, vaddr-264, "!graphics.vec.AreaEnd");
	        mmu_addSymbol(lib_graphics_base.vec_entry, vaddr-282, "!graphics.vec.InitArea");
	        mmu_addSymbol(lib_graphics_base.vec_entry, vaddr-306, "!graphics.vec.RectFill");
	        mmu_addSymbol(lib_graphics_base.vec_entry, vaddr-342, "!graphics.vec.SetAPen");
	        mmu_addSymbol(lib_graphics_base.vec_entry, vaddr-348, "!graphics.vec.SetBPen");
	        mmu_addSymbol(lib_graphics_base.vec_entry, vaddr-354, "!graphics.vec.SetDrMd");
	        mmu_addSymbol(lib_graphics_base.vec_entry, vaddr-468, "!graphics.vec.InitTmpRas");
	        mmu_addSymbol(lib_graphics_base.vec_entry, vaddr-492, "!graphics.vec.AllocRaster");
	        mmu_addSymbol(lib_graphics_base.vec_entry, vaddr-498, "!graphics.vec.FreeRaster");
	        mmu_addSymbol(lib_graphics_base.vec_entry, vaddr-510, "!graphics.vec.OrRectRegion");
	        mmu_addSymbol(lib_graphics_base.vec_entry, vaddr-516, "!graphics.vec.NewRegion");
	        mmu_addSymbol(lib_graphics_base.vec_entry, vaddr-534, "!graphics.vec.DisposeRegion");
	        mmu_addSymbol(lib_graphics_base.vec_entry, vaddr-690, "!graphics.vec.TextExtent");
	        mmu_addSymbol(lib_graphics_base.vec_entry, vaddr-726, "!graphics.vec.FindDisplayInfo");
	        mmu_addSymbol(lib_graphics_base.vec_entry, vaddr-756, "!graphics.vec.GetDisplayInfoData");
	        mmu_addSymbol(lib_graphics_base.vec_entry, vaddr-792, "!graphics.vec.GetVPModeID");
	}

	/* Set up the special memory mapping */
        entry = lib_graphics_base.entry = mmu_addEntry(
                vaddr,
                LIB_GRAPHICS_SIZE,
		MMU_TYPE_GRAPHICS_GRAPHICSBASE,
		NULL
	);
	entry->read = mmu_fn_protected;
	entry->write = mmu_fn_protected;
	entry->nativePtr = (void *)&lib_graphics_base;

	DEBUG(3) {
	        /* Add symbols */
	        mmu_addSymbol(lib_graphics_base.entry, vaddr, "!GraphicsBase");
	}

	is_init = 1;

	/* Init subsystems */

        if( lib_graphics_s_textfont_init() ) { lib_graphics_cleanup(); return 1; }
        if( lib_graphics_s_textattr_init() ) { lib_graphics_cleanup(); return 1; }
        if( lib_graphics_s_viewport_init() ) { lib_graphics_cleanup(); return 1; }
        if( lib_graphics_s_displayinfohandle_init() ) { lib_graphics_cleanup(); return 1; }
        if( lib_graphics_s_displayinfo_init() ) { lib_graphics_cleanup(); return 1; }
        if( lib_graphics_s_rastport_init() ) { lib_graphics_cleanup(); return 1; }
        if( lib_graphics_s_textextent_init() ) { lib_graphics_cleanup(); return 1; }
        if( lib_graphics_s_layer_init() ) { lib_graphics_cleanup(); return 1; }
        if( lib_graphics_s_bitmap_init() ) { lib_graphics_cleanup(); return 1; }
        if( lib_graphics_s_tmpras_init() ) { lib_graphics_cleanup(); return 1; }
        if( lib_graphics_s_areainfo_init() ) { lib_graphics_cleanup(); return 1; }
        if( lib_graphics_s_gelsinfo_init() ) { lib_graphics_cleanup(); return 1; }
        if( lib_graphics_s_layer_info_init() ) { lib_graphics_cleanup(); return 1; }
        if( lib_graphics_s_region_init() ) { lib_graphics_cleanup(); return 1; }
        if( lib_graphics_s_rectangle_init() ) { lib_graphics_cleanup(); return 1; }

	return 0;
}

/* Global cleanup */
void lib_graphics_cleanup()
{
	uint32_t vaddr;
	uint8_t	*addr;
	lib_graphics_t *obj;

	DEBUG(3) dprintf("graphics.library: lib_graphics_cleanup() called\n");

	/* Cleanup subsystems */
        lib_graphics_s_rectangle_cleanup();
        lib_graphics_s_region_cleanup();
        lib_graphics_s_layer_info_cleanup();
        lib_graphics_s_gelsinfo_cleanup();
        lib_graphics_s_areainfo_cleanup();
        lib_graphics_s_tmpras_cleanup();
        lib_graphics_s_bitmap_cleanup();
        lib_graphics_s_layer_cleanup();
        lib_graphics_s_textextent_cleanup();
        lib_graphics_s_rastport_cleanup();
        lib_graphics_s_displayinfo_cleanup();
        lib_graphics_s_displayinfohandle_cleanup();
        lib_graphics_s_viewport_cleanup();
        lib_graphics_s_textattr_cleanup();
        lib_graphics_s_textfont_cleanup();

        if( !is_init ) {
                DEBUG(2) dprintf("Warning: graphics.library: lib_graphics_cleanup: Not initialized.\n");
                return;
        }

	obj = &lib_graphics_base;

	/* Delete mmu entry, GraphicsLibrary structure */
	mmu_delEntry(obj->entry);

	vaddr = obj->vec_entry->startAddr;
	addr = (uint8_t *)obj->vec_entry->nativePtr;

	/* Delete mmu entry, vectors */
	mmu_delEntry(obj->vec_entry);

	/* Free real memory, vectors */
	FreeVec(addr);

	/* Delete vmemory, vectors + GraphicsLibrary structure */
	vfreemem_nomap(vaddr);

	/* Free hook module number */
	free_hook(mnum);

	is_init = 0;
}

static int hook1(emumsg_syscall_t *msg)
{
	switch(msg->func){

		case 9:
			return lib_graphics_f_TextLength(msg);
		case 10:
			return lib_graphics_f_Text(msg);
		case 11:
			return lib_graphics_f_SetFont(msg);
		case 12:
			return lib_graphics_f_OpenFont(msg);
		case 13:
			return lib_graphics_f_CloseFont(msg);
		case 30:
			return lib_graphics_f_DrawEllipse(msg);
		case 31:
			return lib_graphics_f_AreaEllipse(msg);
		case 38:
			return lib_graphics_f_WaitBlit(msg);
		case 39:
			return lib_graphics_f_SetRast(msg);
		case 40:
			return lib_graphics_f_Move(msg);
		case 41:
			return lib_graphics_f_Draw(msg);
		case 42:
			return lib_graphics_f_AreaMove(msg);
		case 43:
			return lib_graphics_f_AreaDraw(msg);
		case 44:
			return lib_graphics_f_AreaEnd(msg);
		case 47:
			return lib_graphics_f_InitArea(msg);
		case 51:
			return lib_graphics_f_RectFill(msg);
		case 57:
			return lib_graphics_f_SetAPen(msg);
		case 58:
			return lib_graphics_f_SetBPen(msg);
		case 59:
			return lib_graphics_f_SetDrMd(msg);
		case 78:
			return lib_graphics_f_InitTmpRas(msg);
		case 82:
			return lib_graphics_f_AllocRaster(msg);
		case 83:
			return lib_graphics_f_FreeRaster(msg);
		case 85:
			return lib_graphics_f_OrRectRegion(msg);
		case 86:
			return lib_graphics_f_NewRegion(msg);
		case 89:
			return lib_graphics_f_DisposeRegion(msg);
		case 115:
			return lib_graphics_f_TextExtent(msg);
		case 121:
			return lib_graphics_f_FindDisplayInfo(msg);
		case 126:
			return lib_graphics_f_GetDisplayInfoData(msg);
		case 132:
			return lib_graphics_f_GetVPModeID(msg);

	}
	dprintf("Error: graphics.library: hook1: Unimplemented function %u\n", msg->func);
	return HOOK_END_PROC;
}

static int hook2(emumsg_syscall_t *msg)
{
	switch(msg->func){

		case 9:
			return lib_graphics_f_TextLength_2(msg);
		case 10:
			return lib_graphics_f_Text_2(msg);
		case 11:
			return lib_graphics_f_SetFont_2(msg);
		case 12:
			return lib_graphics_f_OpenFont_2(msg);
		case 13:
			return lib_graphics_f_CloseFont_2(msg);
		case 30:
			return lib_graphics_f_DrawEllipse_2(msg);
		case 31:
			return lib_graphics_f_AreaEllipse_2(msg);
		case 38:
			return lib_graphics_f_WaitBlit_2(msg);
		case 39:
			return lib_graphics_f_SetRast_2(msg);
		case 40:
			return lib_graphics_f_Move_2(msg);
		case 41:
			return lib_graphics_f_Draw_2(msg);
		case 42:
			return lib_graphics_f_AreaMove_2(msg);
		case 43:
			return lib_graphics_f_AreaDraw_2(msg);
		case 44:
			return lib_graphics_f_AreaEnd_2(msg);
		case 47:
			return lib_graphics_f_InitArea_2(msg);
		case 51:
			return lib_graphics_f_RectFill_2(msg);
		case 57:
			return lib_graphics_f_SetAPen_2(msg);
		case 58:
			return lib_graphics_f_SetBPen_2(msg);
		case 59:
			return lib_graphics_f_SetDrMd_2(msg);
		case 78:
			return lib_graphics_f_InitTmpRas_2(msg);
		case 82:
			return lib_graphics_f_AllocRaster_2(msg);
		case 83:
			return lib_graphics_f_FreeRaster_2(msg);
		case 85:
			return lib_graphics_f_OrRectRegion_2(msg);
		case 86:
			return lib_graphics_f_NewRegion_2(msg);
		case 89:
			return lib_graphics_f_DisposeRegion_2(msg);
		case 115:
			return lib_graphics_f_TextExtent_2(msg);
		case 121:
			return lib_graphics_f_FindDisplayInfo_2(msg);
		case 126:
			return lib_graphics_f_GetDisplayInfoData_2(msg);
		case 132:
			return lib_graphics_f_GetVPModeID_2(msg);

	}
	dprintf("Error: graphics.library: hook2: Unimplemented function %u\n", msg->func);
	return HOOK_END_PROC;
}

static int hook3(emumsg_syscall_t *msg)
{
	switch(msg->func){

		case 9:
			return lib_graphics_f_TextLength_3(msg);
		case 10:
			return lib_graphics_f_Text_3(msg);
		case 11:
			return lib_graphics_f_SetFont_3(msg);
		case 12:
			return lib_graphics_f_OpenFont_3(msg);
		case 13:
			return lib_graphics_f_CloseFont_3(msg);
		case 30:
			return lib_graphics_f_DrawEllipse_3(msg);
		case 31:
			return lib_graphics_f_AreaEllipse_3(msg);
		case 38:
			return lib_graphics_f_WaitBlit_3(msg);
		case 39:
			return lib_graphics_f_SetRast_3(msg);
		case 40:
			return lib_graphics_f_Move_3(msg);
		case 41:
			return lib_graphics_f_Draw_3(msg);
		case 42:
			return lib_graphics_f_AreaMove_3(msg);
		case 43:
			return lib_graphics_f_AreaDraw_3(msg);
		case 44:
			return lib_graphics_f_AreaEnd_3(msg);
		case 47:
			return lib_graphics_f_InitArea_3(msg);
		case 51:
			return lib_graphics_f_RectFill_3(msg);
		case 57:
			return lib_graphics_f_SetAPen_3(msg);
		case 58:
			return lib_graphics_f_SetBPen_3(msg);
		case 59:
			return lib_graphics_f_SetDrMd_3(msg);
		case 78:
			return lib_graphics_f_InitTmpRas_3(msg);
		case 82:
			return lib_graphics_f_AllocRaster_3(msg);
		case 83:
			return lib_graphics_f_FreeRaster_3(msg);
		case 85:
			return lib_graphics_f_OrRectRegion_3(msg);
		case 86:
			return lib_graphics_f_NewRegion_3(msg);
		case 89:
			return lib_graphics_f_DisposeRegion_3(msg);
		case 115:
			return lib_graphics_f_TextExtent_3(msg);
		case 121:
			return lib_graphics_f_FindDisplayInfo_3(msg);
		case 126:
			return lib_graphics_f_GetDisplayInfoData_3(msg);
		case 132:
			return lib_graphics_f_GetVPModeID_3(msg);

	}
	dprintf("Error: graphics.library: hook3: Unimplemented function %u\n", msg->func);
	return HOOK_END_PROC;
}
