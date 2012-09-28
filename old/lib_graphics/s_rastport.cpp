/*
lib_graphics/s_rastport.c - graphics.library struct RastPort

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
#include <graphics/rastport.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "s_rastport.h"
#include "s_layer.h"
#include "s_bitmap.h"
#include "s_tmpras.h"
#include "s_areainfo.h"
#include "s_gelsinfo.h"
#include "s_textfont.h"

/* Predecl of destructor */
static void destructor(mmu_entry_t *entry);

/* Object pool */
static mmu_pool_t *pool;

/* Init flag */
static int is_init = 0;

/* Read map function */
static int map_r(mmu_entry_t *entry, uint32_t offset, uint32_t size,
        uint8_t *data )
{
        uint32_t i;
        lib_graphics_s_rastport_t *obj;
	lib_graphics_s_layer_t *layer = NULL;
	lib_graphics_s_bitmap_t *bitmap = NULL;
	lib_graphics_s_tmpras_t *tmpras = NULL;
	lib_graphics_s_areainfo_t *areainfo = NULL;
	lib_graphics_s_gelsinfo_t *gelsinfo = NULL;
	lib_graphics_s_textfont_t *font = NULL;

        DEBUG(7) dprintf("graphics.library: struct RastPort: map_r() called. VAddr: 0x%x, Size: 0x%x\n", (unsigned int)(entry->startAddr+offset), (unsigned int)size);

        /* Get our object */
        obj = (lib_graphics_s_rastport_t *)entry->nativePtr;

        for(i=offset;i<(size+offset);i++) {
                switch(i) {

                        /* Layer */
                        case 0: case 1: case 2: case 3:
				if(obj->real->Layer == NULL) {
					layer = NULL;
				} else {
					layer = lib_graphics_s_layer_get_real(obj->real->Layer);
					if(layer == NULL) {
        		                        DEBUG(1) dprintf("Error: graphics.library: struct RastPort: Could not create mapping for member: Layer\n");
	        	                        return 1;
					}
				}
				obj->layer_wmask = 0;
                                i = 3;
                                break;

                        /* BitMap */
                        case 4: case 5: case 6: case 7:
				if(obj->real->BitMap == NULL) {
					bitmap = NULL;
				} else {
					bitmap = lib_graphics_s_bitmap_get_real(obj->real->BitMap);
					if(bitmap == NULL) {
        		                        DEBUG(1) dprintf("Error: graphics.library: struct RastPort: Could not create mapping for member: BitMap\n");
	        	                        return 1;
					}
				}
				obj->bitmap_wmask = 0;
                                i = 7;
                                break;

                        /* AreaPtrn */
                        case 8: case 9: case 10: case 11:
				if(obj->real->AreaPtrn != NULL) {
       		                        DEBUG(2) dprintf("Warning: graphics.library: struct RastPort: AreaPtrn pointer is not yet handled, ignoring\n");
				}
				obj->areaptrn_wmask = 0;
                                i = 11;
                                break;

                        /* TmpRas */
                        case 12: case 13: case 14: case 15:
				if(obj->real->TmpRas == NULL) {
					tmpras = NULL;
				} else {
					tmpras = lib_graphics_s_tmpras_get_real(obj->real->TmpRas);
					if(tmpras == NULL) {
        		                        DEBUG(1) dprintf("Error: graphics.library: struct RastPort: Could not create mapping for member: TmpRas\n");
	        	                        return 1;
					}
				}
				obj->tmpras_wmask = 0;
                                i = 15;
                                break;

                        /* AreaInfo */
                        case 16: case 17: case 18: case 19:
				if(obj->real->AreaInfo == NULL) {
					areainfo = NULL;
				} else {
					areainfo = lib_graphics_s_areainfo_get_real(obj->real->AreaInfo);
					if(areainfo == NULL) {
        		                        DEBUG(1) dprintf("Error: graphics.library: struct RastPort: Could not create mapping for member: AreaInfo\n");
	        	                        return 1;
					}
				}
				obj->areainfo_wmask = 0;
                                i = 19;
                                break;

                        /* GelsInfo */
                        case 20: case 21: case 22: case 23:
				if(obj->real->GelsInfo == NULL) {
					gelsinfo = NULL;
				} else {
					gelsinfo = lib_graphics_s_gelsinfo_get_real(obj->real->GelsInfo);
					if(gelsinfo == NULL) {
        		                        DEBUG(1) dprintf("Error: graphics.library: struct RastPort: Could not create mapping for member: GelsInfo\n");
	        	                        return 1;
					}
				}
				obj->gelsinfo_wmask = 0;
                                i = 23;
                                break;

			/* Mask */
			case 24:
				break;

			/* FgPen */
			case 25:
				break;

			/* BgPen */
			case 26:
				break;

			/* AOlPen */
			case 27:
				break;

			/* DrawMode */
			case 28:
				break;

			/* AreaPtSz */
			case 29:
				break;

			/* linpatcnt */
			case 30:
				break;

			/* dummy */
			case 31:
				break;

			/* Flags */
			case 32: case 33:
				i = 33;
				break;

			/* LinePtrn */
			case 34: case 35:
				i = 35;
				break;

			/* cp_x */
			case 36: case 37:
				i = 37;
				break;

			/* cp_y */
			case 38: case 39:
				i = 39;
				break;

			/* minterms[0] ... */
			case 40: case 41: case 42: case 43:
			case 44: case 45: case 46: case 47:
				i = 47;
				break;

			/* PenWidth */
			case 48: case 49:
				i = 49;
				break;

			/* PenHeight */
			case 50: case 51:
				i = 51;
				break;

                        /* Font */
                        case 52: case 53: case 54: case 55:
				if(obj->real->Font == NULL) {
					font = NULL;
				} else {
					font = lib_graphics_s_textfont_get_real(obj->real->Font);
					if(font == NULL) {
        		                        DEBUG(1) dprintf("Error: graphics.library: struct RastPort: Could not create mapping for member: Font\n");
	        	                        return 1;
					}
				}
				obj->font_wmask = 0;
                                i = 55;
                                break;

			/* AlgoStyle */
			case 56:
				break;

			/* TxFlags */
			case 57:
				break;

			/* TxHeight */
			case 58: case 59:
				i = 59;
				break;

			/* TxWidth */
			case 60: case 61:
				i = 61;
				break;

			/* TxBaseline */
			case 62: case 63:
				i = 63;
				break;

			/* TxSpacing */
			case 64: case 65:
				i = 65;
				break;

			/* RP_User */
			case 66: case 67: case 68: case 69:
				i = 69;
				break;

			/* longreserved[0] ... */
			case 70: case 71: case 72: case 73:
			case 74: case 75: case 76: case 77:
				i = 77;
				break;

			/* wordreserved[0] ... */
			case 78: case 79: case 80: case 81:
			case 82: case 83: case 84: case 85:
			case 86: case 87: case 88: case 89:
			case 90: case 91:
				i = 91;
				break;

			/* reserved[0] ... */
			case 92: case 93: case 94: case 95:
			case 96: case 97: case 98: case 99:
				i = 99;
				break;

                        default:
                                /* Nothing found, signal error */
                                DEBUG(1) dprintf("Error: graphics.library: struct RastPort: Read is not allowed. Offset=%u\n", (unsigned int)offset);
                                return 1;
                }
        }

        for(i=offset;i<(size+offset);i++) {
                switch(i) {

                        /* Layer */
                        case 0:
				if(layer == NULL) {
					*data++ = 0;
				} else {
	                                *data++ = (layer->entry->startAddr >> 24) & 0xff;
				}
                                break;
                        case 1:
				if(layer == NULL) {
					*data++ = 0;
				} else {
                                	*data++ = (layer->entry->startAddr >> 16) & 0xff;
				}
                                break;
                        case 2:
				if(layer == NULL) {
					*data++ = 0;
				} else {
                                	*data++ = (layer->entry->startAddr >> 8) & 0xff;
				}
                                break;
                        case 3:
				if(layer == NULL) {
					*data++ = 0;
				} else {
                                	*data++ = (layer->entry->startAddr) & 0xff;
				}
                                break;

                        /* BitMap */
                        case 4:
				if(bitmap == NULL) {
					*data++ = 0;
				} else {
	                                *data++ = (bitmap->entry->startAddr >> 24) & 0xff;
				}
                                break;
                        case 5:
				if(bitmap == NULL) {
					*data++ = 0;
				} else {
	                                *data++ = (bitmap->entry->startAddr >> 16) & 0xff;
				}
                                break;
                        case 6:
				if(bitmap == NULL) {
					*data++ = 0;
				} else {
	                                *data++ = (bitmap->entry->startAddr >> 8) & 0xff;
				}
                                break;
                        case 7:
				if(bitmap == NULL) {
					*data++ = 0;
				} else {
	                                *data++ = (bitmap->entry->startAddr) & 0xff;
				}
                                break;

                        /* AreaPtrn */
                        case 8: case 9: case 10: case 11:
				*data++ = 0xff;
				break;

                        /* TmpRas */
                        case 12:
				if(tmpras == NULL) {
					*data++ = 0;
				} else {
	                                *data++ = (tmpras->entry->startAddr >> 24) & 0xff;
				}
                                break;
                        case 13:
				if(tmpras == NULL) {
					*data++ = 0;
				} else {
	                                *data++ = (tmpras->entry->startAddr >> 16) & 0xff;
				}
                                break;
                        case 14:
				if(tmpras == NULL) {
					*data++ = 0;
				} else {
	                                *data++ = (tmpras->entry->startAddr >> 8) & 0xff;
				}
                                break;
                        case 15:
				if(tmpras == NULL) {
					*data++ = 0;
				} else {
	                                *data++ = (tmpras->entry->startAddr) & 0xff;
				}
                                break;

                        /* AreaInfo */
                        case 16:
				if(areainfo == NULL) {
					*data++ = 0;
				} else {
	                                *data++ = (areainfo->entry->startAddr >> 24) & 0xff;
				}
                                break;
                        case 17:
				if(areainfo == NULL) {
					*data++ = 0;
				} else {
	                                *data++ = (areainfo->entry->startAddr >> 16) & 0xff;
				}
                                break;
                        case 18:
				if(areainfo == NULL) {
					*data++ = 0;
				} else {
	                                *data++ = (areainfo->entry->startAddr >> 8) & 0xff;
				}
                                break;
                        case 19:
				if(areainfo == NULL) {
					*data++ = 0;
				} else {
	                                *data++ = (areainfo->entry->startAddr) & 0xff;
				}
                                break;

                        /* GelsInfo */
                        case 20:
				if(gelsinfo == NULL) {
					*data++ = 0;
				} else {
	                                *data++ = (gelsinfo->entry->startAddr >> 24) & 0xff;
				}
                                break;
                        case 21:
				if(gelsinfo == NULL) {
					*data++ = 0;
				} else {
	                                *data++ = (gelsinfo->entry->startAddr >> 16) & 0xff;
				}
                                break;
                        case 22:
				if(gelsinfo == NULL) {
					*data++ = 0;
				} else {
	                                *data++ = (gelsinfo->entry->startAddr >> 8) & 0xff;
				}
                                break;
                        case 23:
				if(gelsinfo == NULL) {
					*data++ = 0;
				} else {
	                                *data++ = (gelsinfo->entry->startAddr) & 0xff;
				}
                                break;

			/* Mask */
			case 24:
				*data++ = obj->real->Mask;
				break;

			/* FgPen */
			case 25:
				*data++ = obj->real->FgPen;
				break;

			/* BgPen */
			case 26:
				*data++ = obj->real->BgPen;
				break;

			/* AOlPen */
			case 27:
				*data++ = obj->real->AOlPen;
				break;

			/* DrawMode */
			case 28:
				*data++ = obj->real->DrawMode;
				break;

			/* AreaPtSz */
			case 29:
				*data++ = obj->real->AreaPtSz;
				break;

			/* linpatcnt */
			case 30:
				*data++ = obj->real->linpatcnt;
				break;

			/* dummy */
			case 31:
				*data++ = obj->real->dummy;
				break;

			/* Flags */
			case 32:
				*data++ = (obj->real->Flags >> 8) & 0xff;
				break;
			case 33:
				*data++ = (obj->real->Flags) & 0xff;
				break;

			/* LinePtrn */
			case 34:
				*data++ = (obj->real->LinePtrn >> 8) & 0xff;
				break;
			case 35:
				*data++ = (obj->real->LinePtrn) & 0xff;
				break;

			/* cp_x */
			case 36:
				*data++ = (obj->real->cp_x >> 8) & 0xff;
				break;
			case 37:
				*data++ = (obj->real->cp_x) & 0xff;
				break;

			/* xp_y */
			case 38:
				*data++ = (obj->real->cp_y >> 8) & 0xff;
				break;
			case 39:
				*data++ = (obj->real->cp_y) & 0xff;
				break;

			/* minterms[0] ... */
			case 40:
				*data++ = obj->real->minterms[0];
				break;
			case 41:
				*data++ = obj->real->minterms[1];
				break;
			case 42:
				*data++ = obj->real->minterms[2];
				break;
			case 43:
				*data++ = obj->real->minterms[3];
				break;
			case 44:
				*data++ = obj->real->minterms[4];
				break;
			case 45:
				*data++ = obj->real->minterms[5];
				break;
			case 46:
				*data++ = obj->real->minterms[6];
				break;
			case 47:
				*data++ = obj->real->minterms[7];
				break;

			/* PenWidth */
			case 48:
				*data++ = (obj->real->PenWidth >> 8) & 0xff;
				break;
			case 49:
				*data++ = (obj->real->PenWidth) & 0xff;
				break;

			/* PenHeight */
			case 50:
				*data++ = (obj->real->PenHeight >> 8) & 0xff;
				break;
			case 51:
				*data++ = (obj->real->PenHeight) & 0xff;
				break;

                        /* Font */
                        case 52:
				if(font == NULL) {
					*data++ = 0;
				} else {
	                                *data++ = (font->entry->startAddr >> 24) & 0xff;
				}
                                break;
                        case 53:
				if(font == NULL) {
					*data++ = 0;
				} else {
	                                *data++ = (font->entry->startAddr >> 16) & 0xff;
				}
                                break;
                        case 54:
				if(font == NULL) {
					*data++ = 0;
				} else {
	                                *data++ = (font->entry->startAddr >> 8) & 0xff;
				}
                                break;
                        case 55:
				if(font == NULL) {
					*data++ = 0;
				} else {
	                                *data++ = (font->entry->startAddr) & 0xff;
				}
                                break;

			/* AlgoStyle */
			case 56:
				*data++ = obj->real->AlgoStyle;
				break;

			/* TxFlags */
			case 57:
				*data++ = obj->real->TxFlags;
				break;

			/* TxHeight */
			case 58:
				*data++ = (obj->real->TxHeight >> 8) & 0xff;
				break;
			case 59:
				*data++ = (obj->real->TxHeight) & 0xff;
				break;

			/* TxWidth */
			case 60:
				*data++ = (obj->real->TxWidth >> 8) & 0xff;
				break;
			case 61:
				*data++ = (obj->real->TxWidth) & 0xff;
				break;

			/* TxBaseline */
			case 62:
				*data++ = (obj->real->TxBaseline >> 8) & 0xff;
				break;
			case 63:
				*data++ = (obj->real->TxBaseline) & 0xff;
				break;

			/* TxSpacing */
			case 64:
				*data++ = (obj->real->TxSpacing >> 8) & 0xff;
				break;
			case 65:
				*data++ = (obj->real->TxSpacing) & 0xff;
				break;

			/* RP_User */
			case 66:
				*data++ = ((uint32_t)obj->real->RP_User >> 24) & 0xff;
				break;
			case 67:
				*data++ = ((uint32_t)obj->real->RP_User >> 16) & 0xff;
				break;
			case 68:
				*data++ = ((uint32_t)obj->real->RP_User >> 8) & 0xff;
				break;
			case 69:
				*data++ = ((uint32_t)obj->real->RP_User) & 0xff;
				break;

			/* longreserved[0] ... */
			case 70:
				*data++ = (obj->real->longreserved[0] >> 24) & 0xff;
				break;
			case 71:
				*data++ = (obj->real->longreserved[0] >> 16) & 0xff;
				break;
			case 72:
				*data++ = (obj->real->longreserved[0] >> 8) & 0xff;
				break;
			case 73:
				*data++ = (obj->real->longreserved[0]) & 0xff;
				break;
			case 74:
				*data++ = (obj->real->longreserved[1] >> 24) & 0xff;
				break;
			case 75:
				*data++ = (obj->real->longreserved[1] >> 16) & 0xff;
				break;
			case 76:
				*data++ = (obj->real->longreserved[1] >> 8) & 0xff;
				break;
			case 77:
				*data++ = (obj->real->longreserved[1]) & 0xff;
				break;

			/* wordreserved[0] ... */
			case 78:
				*data++ = (obj->real->wordreserved[0] >> 8) & 0xff;
				break;
			case 79:
				*data++ = (obj->real->wordreserved[0]) & 0xff;
				break;
			case 80:
				*data++ = (obj->real->wordreserved[1] >> 8) & 0xff;
				break;
			case 81:
				*data++ = (obj->real->wordreserved[1]) & 0xff;
				break;
			case 82:
				*data++ = (obj->real->wordreserved[2] >> 8) & 0xff;
				break;
			case 83:
				*data++ = (obj->real->wordreserved[2]) & 0xff;
				break;
			case 84:
				*data++ = (obj->real->wordreserved[3] >> 8) & 0xff;
				break;
			case 85:
				*data++ = (obj->real->wordreserved[3]) & 0xff;
				break;
			case 86:
				*data++ = (obj->real->wordreserved[4] >> 8) & 0xff;
				break;
			case 87:
				*data++ = (obj->real->wordreserved[4]) & 0xff;
				break;
			case 88:
				*data++ = (obj->real->wordreserved[5] >> 8) & 0xff;
				break;
			case 89:
				*data++ = (obj->real->wordreserved[5]) & 0xff;
				break;
			case 90:
				*data++ = (obj->real->wordreserved[6] >> 8) & 0xff;
				break;
			case 91:
				*data++ = (obj->real->wordreserved[6]) & 0xff;
				break;

			/* reserved[0] ... */
			case 92:
				*data++ = obj->real->reserved[0];
				break;
			case 93:
				*data++ = obj->real->reserved[1];
				break;
			case 94:
				*data++ = obj->real->reserved[2];
				break;
			case 95:
				*data++ = obj->real->reserved[3];
				break;
			case 96:
				*data++ = obj->real->reserved[4];
				break;
			case 97:
				*data++ = obj->real->reserved[5];
				break;
			case 98:
				*data++ = obj->real->reserved[6];
				break;
			case 99:
				*data++ = obj->real->reserved[7];
				break;

                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: graphics.library: struct RastPort: Read is not allowed. Offset=%u\n", (unsigned int)offset);
                                return 1;
                }
        }

        return 0;
}

/* Write map function */
static int map_w(mmu_entry_t *entry, uint32_t offset, uint32_t size,
        uint8_t *data )
{
        uint32_t i, vaddr;
        lib_graphics_s_rastport_t *obj;
	lib_graphics_s_layer_t *layer;
	lib_graphics_s_bitmap_t *bitmap;
	lib_graphics_s_tmpras_t *tmpras;
	lib_graphics_s_areainfo_t *areainfo;
	lib_graphics_s_gelsinfo_t *gelsinfo;
	lib_graphics_s_textfont_t *font;

        DEBUG(7) dprintf("graphics.library: struct RastPort: map_w() called. VAddr: 0x%x, Size: 0x%x\n", (unsigned int)(entry->startAddr+offset), (unsigned int)size);

        /* Get our object */
        obj = (lib_graphics_s_rastport_t *)entry->nativePtr;


        for(i=offset;i<(size+offset);i++) {
                switch(i) {

                        /* Layer */
                        case 0:
				obj->layer_wtmp[0] = *data++;
				obj->layer_wmask |= 1 << 0;
                                break;
                        case 1:
				obj->layer_wtmp[1] = *data++;
				obj->layer_wmask |= 1 << 1;
                                break;
                        case 2:
				obj->layer_wtmp[2] = *data++;
				obj->layer_wmask |= 1 << 2;
                                break;
                        case 3:
				obj->layer_wtmp[3] = *data++;
				obj->layer_wmask |= 1 << 3;
                                break;

                        /* BitMap */
                        case 4:
				obj->bitmap_wtmp[0] = *data++;
				obj->bitmap_wmask |= 1 << 0;
                                break;
                        case 5:
				obj->bitmap_wtmp[1] = *data++;
				obj->bitmap_wmask |= 1 << 1;
                                break;
                        case 6:
				obj->bitmap_wtmp[2] = *data++;
				obj->bitmap_wmask |= 1 << 2;
                                break;
                        case 7:
				obj->bitmap_wtmp[3] = *data++;
				obj->bitmap_wmask |= 1 << 3;
                                break;

                        /* AreaPtrn */
                        case 8:
				obj->areaptrn_wtmp[0] = *data++;
				obj->areaptrn_wmask |= 1 << 0;
				break;
			case 9:
				obj->areaptrn_wtmp[1] = *data++;
				obj->areaptrn_wmask |= 1 << 1;
				break;
			case 10:
				obj->areaptrn_wtmp[2] = *data++;
				obj->areaptrn_wmask |= 1 << 2;
				break;
			case 11:
				obj->areaptrn_wtmp[3] = *data++;
				obj->areaptrn_wmask |= 1 << 3;
				break;

                        /* TmpRas */
                        case 12:
				obj->tmpras_wtmp[0] = *data++;
				obj->tmpras_wmask |= 1 << 0;
                                break;
                        case 13:
				obj->tmpras_wtmp[1] = *data++;
				obj->tmpras_wmask |= 1 << 1;
                                break;
                        case 14:
				obj->tmpras_wtmp[2] = *data++;
				obj->tmpras_wmask |= 1 << 2;
                                break;
                        case 15:
				obj->tmpras_wtmp[3] = *data++;
				obj->tmpras_wmask |= 1 << 3;
                                break;

                        /* AreaInfo */
                        case 16:
				obj->areainfo_wtmp[0] = *data++;
				obj->areainfo_wmask |= 1 << 0;
                                break;
                        case 17:
				obj->areainfo_wtmp[1] = *data++;
				obj->areainfo_wmask |= 1 << 1;
                                break;
                        case 18:
				obj->areainfo_wtmp[2] = *data++;
				obj->areainfo_wmask |= 1 << 2;
                                break;
                        case 19:
				obj->areainfo_wtmp[3] = *data++;
				obj->areainfo_wmask |= 1 << 3;
                                break;

                        /* GelsInfo */
                        case 20:
				obj->gelsinfo_wtmp[0] = *data++;
				obj->gelsinfo_wmask |= 1 << 0;
                                break;
                        case 21:
				obj->gelsinfo_wtmp[1] = *data++;
				obj->gelsinfo_wmask |= 1 << 1;
                                break;
                        case 22:
				obj->gelsinfo_wtmp[2] = *data++;
				obj->gelsinfo_wmask |= 1 << 2;
                                break;
                        case 23:
				obj->gelsinfo_wtmp[3] = *data++;
				obj->gelsinfo_wmask |= 1 << 3;
                                break;

			/* Mask */
			case 24:
				obj->real->Mask = *data++;
				break;

			/* FgPen */
			case 25:
				obj->real->FgPen = *data++;
				break;

			/* BgPen */
			case 26:
				obj->real->BgPen = *data++;
				break;

			/* AOlPen */
			case 27:
				obj->real->AOlPen = *data++;
				break;

			/* DrawMode */
			case 28:
				obj->real->DrawMode = *data++;
				break;

			/* AreaPtSz */
			case 29:
				obj->real->AreaPtSz = *data++;
				break;

			/* linpatcnt */
			case 30:
				obj->real->linpatcnt = *data++;
				break;

			/* dummy */
			case 31:
				obj->real->dummy = *data++;
				break;

			/* Flags */
			case 32:
				obj->real->Flags &= 0x00ff;
				obj->real->Flags |= ((uint16_t)*data++ << 8);
				break;
			case 33:
				obj->real->Flags &= 0xff00;
				obj->real->Flags |= ((uint16_t)*data++);
				break;

			/* LinePtrn */
			case 34:
				obj->real->LinePtrn &= 0x00ff;
				obj->real->LinePtrn |= ((uint16_t)*data++ << 8);
				break;
			case 35:
				obj->real->LinePtrn &= 0xff00;
				obj->real->LinePtrn |= ((uint16_t)*data++);
				break;

			/* cp_x */
			case 36:
				obj->real->cp_x &= 0x00ff;
				obj->real->cp_x |= ((uint16_t)*data++ << 8);
				break;
			case 37:
				obj->real->cp_x &= 0xff00;
				obj->real->cp_x |= ((uint16_t)*data++);
				break;

			/* cp_y */
			case 38:
				obj->real->cp_y &= 0x00ff;
				obj->real->cp_y |= ((uint16_t)*data++ << 8);
				break;
			case 39:
				obj->real->cp_y &= 0xff00;
				obj->real->cp_y |= ((uint16_t)*data++);
				break;

			/* minterms[0] ... */
			case 40:
				obj->real->minterms[0] = *data++;
				break;
			case 41:
				obj->real->minterms[1] = *data++;
				break;
			case 42:
				obj->real->minterms[2] = *data++;
				break;
			case 43:
				obj->real->minterms[3] = *data++;
				break;
			case 44:
				obj->real->minterms[4] = *data++;
				break;
			case 45:
				obj->real->minterms[5] = *data++;
				break;
			case 46:
				obj->real->minterms[6] = *data++;
				break;
			case 47:
				obj->real->minterms[7] = *data++;
				break;

			/* PenWidth */
			case 48:
				obj->real->PenWidth &= 0x00ff;
				obj->real->PenWidth |= ((uint16_t)*data++ << 8);
				break;
			case 49:
				obj->real->PenWidth &= 0xff00;
				obj->real->PenWidth |= ((uint16_t)*data++);
				break;

			/* PenHeight */
			case 50:
				obj->real->PenHeight &= 0x00ff;
				obj->real->PenHeight |= ((uint16_t)*data++ << 8);
				break;
			case 51:
				obj->real->PenHeight &= 0xff00;
				obj->real->PenHeight |= ((uint16_t)*data++);
				break;

                        /* Font */
                        case 52:
				obj->font_wtmp[0] = *data++;
				obj->font_wmask |= 1 << 0;
                                break;
                        case 53:
				obj->font_wtmp[1] = *data++;
				obj->font_wmask |= 1 << 1;
                                break;
                        case 54:
				obj->font_wtmp[2] = *data++;
				obj->font_wmask |= 1 << 2;
                                break;
                        case 55:
				obj->font_wtmp[3] = *data++;
				obj->font_wmask |= 1 << 3;
                                break;

			/* AlgoStyle */
			case 56:
				obj->real->AlgoStyle = *data++;
				break;

			/* TxFlags */
			case 57:
				obj->real->TxFlags = *data++;
				break;

			/* TxHeight */
			case 58:
				obj->real->TxHeight &= 0x00ff;
				obj->real->TxHeight |= ((uint16_t)*data++ << 8);
				break;
			case 59:
				obj->real->TxHeight &= 0xff00;
				obj->real->TxHeight |= ((uint16_t)*data++);
				break;

			/* TxWidth */
			case 60:
				obj->real->TxWidth &= 0x00ff;
				obj->real->TxWidth |= ((uint16_t)*data++ << 8);
				break;
			case 61:
				obj->real->TxWidth &= 0xff00;
				obj->real->TxWidth |= ((uint16_t)*data++);
				break;

			/* TxBaseline */
			case 62:
				obj->real->TxBaseline &= 0x00ff;
				obj->real->TxBaseline |= ((uint16_t)*data++ << 8);
				break;
			case 63:
				obj->real->TxBaseline &= 0xff00;
				obj->real->TxBaseline |= ((uint16_t)*data++);
				break;

			/* TxSpacing */
			case 64:
				obj->real->TxSpacing &= 0x00ff;
				obj->real->TxSpacing |= ((uint16_t)*data++ << 8);
				break;
			case 65:
				obj->real->TxSpacing &= 0xff00;
				obj->real->TxSpacing |= ((uint16_t)*data++);
				break;

			/* RP_User */
			case 66:
				vaddr = (uint32_t)obj->real->RP_User;
				vaddr &= 0x00ffffff;
				vaddr |= ((uint32_t)*data++ << 24);
				obj->real->RP_User = (void **)vaddr;
				break;
			case 67:
				vaddr = (uint32_t)obj->real->RP_User;
				vaddr &= 0xff00ffff;
				vaddr |= ((uint32_t)*data++ << 16);
				obj->real->RP_User = (void **)vaddr;
				break;
			case 68:
				vaddr = (uint32_t)obj->real->RP_User;
				vaddr &= 0xffff00ff;
				vaddr |= ((uint32_t)*data++ << 8);
				obj->real->RP_User = (void **)vaddr;
				break;
			case 69:
				vaddr = (uint32_t)obj->real->RP_User;
				vaddr &= 0xffffff00;
				vaddr |= ((uint32_t)*data++);
				obj->real->RP_User = (void **)vaddr;
				break;

			/* longreserved[0] ... */
			case 70:
				obj->real->longreserved[0] &= 0x00ffffff;
				obj->real->longreserved[0] |= ((uint32_t)*data++ << 24);
				break;
			case 71:
				obj->real->longreserved[0] &= 0xff00ffff;
				obj->real->longreserved[0] |= ((uint32_t)*data++ << 16);
				break;
			case 72:
				obj->real->longreserved[0] &= 0xffff00ff;
				obj->real->longreserved[0] |= ((uint32_t)*data++ << 8);
				break;
			case 73:
				obj->real->longreserved[0] &= 0xffffff00;
				obj->real->longreserved[0] |= ((uint32_t)*data++);
				break;
			case 74:
				obj->real->longreserved[1] &= 0x00ffffff;
				obj->real->longreserved[1] |= ((uint32_t)*data++ << 24);
				break;
			case 75:
				obj->real->longreserved[1] &= 0xff00ffff;
				obj->real->longreserved[1] |= ((uint32_t)*data++ << 16);
				break;
			case 76:
				obj->real->longreserved[1] &= 0xffff00ff;
				obj->real->longreserved[1] |= ((uint32_t)*data++ << 8);
				break;
			case 77:
				obj->real->longreserved[1] &= 0xffffff00;
				obj->real->longreserved[1] |= ((uint32_t)*data++);
				break;

			/* wordreserved[0] ... */
			case 78:
				obj->real->wordreserved[0] &= 0x00ff;
				obj->real->wordreserved[0] |= ((uint16_t)*data++ << 8);
				break;
			case 79:
				obj->real->wordreserved[0] &= 0xff00;
				obj->real->wordreserved[0] |= ((uint16_t)*data++);
				break;
			case 80:
				obj->real->wordreserved[1] &= 0x00ff;
				obj->real->wordreserved[1] |= ((uint16_t)*data++ << 8);
				break;
			case 81:
				obj->real->wordreserved[1] &= 0xff00;
				obj->real->wordreserved[1] |= ((uint16_t)*data++);
				break;
			case 82:
				obj->real->wordreserved[2] &= 0x00ff;
				obj->real->wordreserved[2] |= ((uint16_t)*data++ << 8);
				break;
			case 83:
				obj->real->wordreserved[2] &= 0xff00;
				obj->real->wordreserved[2] |= ((uint16_t)*data++);
				break;
			case 84:
				obj->real->wordreserved[3] &= 0x00ff;
				obj->real->wordreserved[3] |= ((uint16_t)*data++ << 8);
				break;
			case 85:
				obj->real->wordreserved[3] &= 0xff00;
				obj->real->wordreserved[3] |= ((uint16_t)*data++);
				break;
			case 86:
				obj->real->wordreserved[4] &= 0x00ff;
				obj->real->wordreserved[4] |= ((uint16_t)*data++ << 8);
				break;
			case 87:
				obj->real->wordreserved[4] &= 0xff00;
				obj->real->wordreserved[4] |= ((uint16_t)*data++);
				break;
			case 88:
				obj->real->wordreserved[5] &= 0x00ff;
				obj->real->wordreserved[5] |= ((uint16_t)*data++ << 8);
				break;
			case 89:
				obj->real->wordreserved[5] &= 0xff00;
				obj->real->wordreserved[5] |= ((uint16_t)*data++);
				break;
			case 90:
				obj->real->wordreserved[6] &= 0x00ff;
				obj->real->wordreserved[6] |= ((uint16_t)*data++ << 8);
				break;
			case 91:
				obj->real->wordreserved[6] &= 0xff00;
				obj->real->wordreserved[6] |= ((uint16_t)*data++);
				break;

			/* reserved[0] ... */
			case 92:
				obj->real->reserved[0] = *data++;
				break;
			case 93:
				obj->real->reserved[1] = *data++;
				break;
			case 94:
				obj->real->reserved[2] = *data++;
				break;
			case 95:
				obj->real->reserved[3] = *data++;
				break;
			case 96:
				obj->real->reserved[4] = *data++;
				break;
			case 97:
				obj->real->reserved[5] = *data++;
				break;
			case 98:
				obj->real->reserved[6] = *data++;
				break;
			case 99:
				obj->real->reserved[7] = *data++;
				break;

                        default:
                                /* Nothing found, signal error */
                                dprintf("Error: graphics.library: struct RastPort: Write is not allowed. Offset=%u\n", (unsigned int)offset);
                                return 1;
                }
        }

	/* Check for updated pointers */

        for(i=offset;i<(size+offset);i++) {
                switch(i) {

                        /* Layer */
                        case 0: case 1: case 2: case 3:
				if(obj->layer_wmask == 0x0f) {
					vaddr = ((uint32_t)obj->layer_wtmp[0] << 24) |
					        ((uint32_t)obj->layer_wtmp[1] << 16) |
					        ((uint32_t)obj->layer_wtmp[2] << 8) |
					        ((uint32_t)obj->layer_wtmp[3]);

					layer = lib_graphics_s_layer_get_vaddr(vaddr);
					if(layer != NULL) {
						obj->real->Layer = layer->real;
					} else {
						obj->real->Layer = NULL;
					}
					obj->layer_wmask = 0;
				}
                                i = 3;
                                break;

                        /* BitMap */
                        case 4: case 5: case 6: case 7:
				if(obj->bitmap_wmask == 0x0f) {
					vaddr = ((uint32_t)obj->bitmap_wtmp[0] << 24) |
					        ((uint32_t)obj->bitmap_wtmp[1] << 16) |
					        ((uint32_t)obj->bitmap_wtmp[2] << 8) |
					        ((uint32_t)obj->bitmap_wtmp[3]);

					bitmap = lib_graphics_s_bitmap_get_vaddr(vaddr);
					if(bitmap != NULL) {
						obj->real->BitMap = bitmap->real;
					} else {
						obj->real->BitMap = NULL;
					}
					obj->bitmap_wmask = 0;
				}
                                i = 7;
                                break;

                        /* AreaPtrn */
                        case 8: case 9: case 10: case 11:
				if(obj->areaptrn_wmask == 0x0f) {
					vaddr = ((uint32_t)obj->areaptrn_wtmp[0] << 24) |
					        ((uint32_t)obj->areaptrn_wtmp[1] << 16) |
					        ((uint32_t)obj->areaptrn_wtmp[2] << 8) |
					        ((uint32_t)obj->areaptrn_wtmp[3]);

					if(vaddr != 0) {
	       		                        DEBUG(2) dprintf("Warning: graphics.library: struct RastPort: AreaPtrn pointer is not yet handled, ignoring\n");
					}
					obj->real->AreaPtrn = NULL;

					obj->areaptrn_wmask = 0;
				}
                                i = 11;
                                break;

                        /* TmpRas */
                        case 12: case 13: case 14: case 15:
				if(obj->tmpras_wmask == 0x0f) {
					vaddr = ((uint32_t)obj->tmpras_wtmp[0] << 24) |
					        ((uint32_t)obj->tmpras_wtmp[1] << 16) |
					        ((uint32_t)obj->tmpras_wtmp[2] << 8) |
					        ((uint32_t)obj->tmpras_wtmp[3]);

					tmpras = lib_graphics_s_tmpras_get_vaddr(vaddr);
					if(tmpras != NULL) {
						obj->real->TmpRas = tmpras->real;
					} else {
						obj->real->TmpRas = NULL;
					}
					obj->tmpras_wmask = 0;
				}
                                i = 15;
                                break;

                        /* AreaInfo */
                        case 16: case 17: case 18: case 19:
				if(obj->areainfo_wmask == 0x0f) {
					vaddr = ((uint32_t)obj->areainfo_wtmp[0] << 24) |
					        ((uint32_t)obj->areainfo_wtmp[1] << 16) |
					        ((uint32_t)obj->areainfo_wtmp[2] << 8) |
					        ((uint32_t)obj->areainfo_wtmp[3]);

					areainfo = lib_graphics_s_areainfo_get_vaddr(vaddr);
					if(areainfo != NULL) {
						obj->real->AreaInfo = areainfo->real;
					} else {
						obj->real->AreaInfo = NULL;
					}
					obj->areainfo_wmask = 0;
				}
                                i = 19;
                                break;

                        /* GelsInfo */
                        case 20: case 21: case 22: case 23:
				if(obj->gelsinfo_wmask == 0x0f) {
					vaddr = ((uint32_t)obj->gelsinfo_wtmp[0] << 24) |
					        ((uint32_t)obj->gelsinfo_wtmp[1] << 16) |
					        ((uint32_t)obj->gelsinfo_wtmp[2] << 8) |
					        ((uint32_t)obj->gelsinfo_wtmp[3]);

					gelsinfo = lib_graphics_s_gelsinfo_get_vaddr(vaddr);
					if(gelsinfo != NULL) {
						obj->real->GelsInfo = gelsinfo->real;
					} else {
						obj->real->GelsInfo = NULL;
					}
					obj->gelsinfo_wmask = 0;
				}
                                i = 23;
                                break;

                        /* Font */
                        case 52: case 53: case 54: case 55:
				if(obj->font_wmask == 0x0f) {
					vaddr = ((uint32_t)obj->font_wtmp[0] << 24) |
					        ((uint32_t)obj->font_wtmp[1] << 16) |
					        ((uint32_t)obj->font_wtmp[2] << 8) |
					        ((uint32_t)obj->font_wtmp[3]);

					font = lib_graphics_s_textfont_get_vaddr(vaddr);
					if(font != NULL) {
						obj->real->Font = font->real;
					} else {
						obj->real->Font = NULL;
					}
					obj->font_wmask = 0;
				}
                                i = 55;
                                break;
                }
        }

        return 0;
}

/* Global init */
int lib_graphics_s_rastport_init()
{
	DEBUG(4) dprintf("graphics.library: lib_graphics_s_rastport_init() called\n");

        pool = mmu_pool_create(
                "graphics.s.RastPort",
                MMU_TYPE_GRAPHICS_S_RASTPORT,
                LIB_GRAPHICS_S_RASTPORT_SIZE,
                sizeof(lib_graphics_s_rastport_t)
        );

        if(pool == NULL)
                return 1;

	is_init = 1;
	return 0;
}

/* Global cleanup */
void lib_graphics_s_rastport_cleanup()
{
	DEBUG(4) dprintf("graphics.library: lib_graphics_s_rastport_cleanup() called\n");

        if( !is_init ) {
                DEBUG(2) dprintf("Warning: graphics.library: lib_graphics_s_rastport_cleanup: Not initialized\n");
                return;
        }

	mmu_pool_destroy(pool);

	is_init = 0;
}

/* Get an instance */
lib_graphics_s_rastport_t *lib_graphics_s_rastport_get_real(struct RastPort *real)
{
	mmu_entry_t *entry;
	lib_graphics_s_rastport_t *obj;
	uint32_t vaddr;
	char symbol[200];

	DEBUG(5) dprintf("graphics.library: lib_graphics_s_rastport_get_real() called\n");

	if(real == NULL)
		return NULL;

        /* Check to find if it already exists */
        entry = mmu_findEntryWithTypeReal(real, MMU_TYPE_GRAPHICS_S_RASTPORT);
        if(entry != NULL) {

                /* Yes, return object */
                return (lib_graphics_s_rastport_t *)entry->nativePtr;
        }

        /* No, set up an object */
        entry = mmu_allocObj(pool, real);
        if(entry == NULL)
                 return NULL;

        /* Extract object */
        obj = (lib_graphics_s_rastport_t *)entry->nativePtr;

        /* Init structure */
	entry->read = map_r;
	entry->write = map_w;
	entry->destructor = destructor;
        obj->entry = entry;
        obj->real = real;
	obj->userSpace = 0;	/* Created from real memory */

        DEBUG(3) {
                /* Add extra symbols */
		vaddr = entry->startAddr;
                sprintf(symbol, "!%08x.%s.Layer", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+0, symbol);
                sprintf(symbol, "!%08x.%s.BitMap", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+4, symbol);
                sprintf(symbol, "!%08x.%s.AreaPtrn", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+8, symbol);
                sprintf(symbol, "!%08x.%s.TmpRas", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+12, symbol);
                sprintf(symbol, "!%08x.%s.AreaInfo", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+16, symbol);
                sprintf(symbol, "!%08x.%s.GelsInfo", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+20, symbol);
                sprintf(symbol, "!%08x.%s.Mask", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+24, symbol);
                sprintf(symbol, "!%08x.%s.FgPen", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+25, symbol);
                sprintf(symbol, "!%08x.%s.BgPen", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+26, symbol);
                sprintf(symbol, "!%08x.%s.AOlPen", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+27, symbol);
                sprintf(symbol, "!%08x.%s.DrawMode", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+28, symbol);
                sprintf(symbol, "!%08x.%s.AreaPtSz", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+29, symbol);
                sprintf(symbol, "!%08x.%s.linpatcnt", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+30, symbol);
                sprintf(symbol, "!%08x.%s.dummy", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+31, symbol);
                sprintf(symbol, "!%08x.%s.Flags", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+32, symbol);
                sprintf(symbol, "!%08x.%s.LinePtrn", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+34, symbol);
                sprintf(symbol, "!%08x.%s.cp_x", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+36, symbol);
                sprintf(symbol, "!%08x.%s.cp_y", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+38, symbol);
                sprintf(symbol, "!%08x.%s.minterms[0]", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+40, symbol);
                sprintf(symbol, "!%08x.%s.PenWidth", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+48, symbol);
                sprintf(symbol, "!%08x.%s.PenHeight", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+50, symbol);
                sprintf(symbol, "!%08x.%s.Font", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+52, symbol);
                sprintf(symbol, "!%08x.%s.AlgoStyle", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+56, symbol);
                sprintf(symbol, "!%08x.%s.TxFlags", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+57, symbol);
                sprintf(symbol, "!%08x.%s.TxHeight", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+58, symbol);
                sprintf(symbol, "!%08x.%s.TxWidth", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+60, symbol);
                sprintf(symbol, "!%08x.%s.TxBaseline", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+62, symbol);
                sprintf(symbol, "!%08x.%s.TxSpacing", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+64, symbol);
                sprintf(symbol, "!%08x.%s.RP_User", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+66, symbol);
                sprintf(symbol, "!%08x.%s.longreserved[0]", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+70, symbol);
                sprintf(symbol, "!%08x.%s.wordreserved[0]", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+78, symbol);
                sprintf(symbol, "!%08x.%s.reserved[0]", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+92, symbol);
        }

	return obj;
}

/* Get an instance */
lib_graphics_s_rastport_t *lib_graphics_s_rastport_get_vaddr(uint32_t vaddr)
{
	mmu_entry_t *entry;
	lib_graphics_s_rastport_t *obj;
	struct RastPort *real;

	lib_graphics_s_layer_t *layer;
	uint32_t layer_vaddr;
	struct Layer *layer_real;
	uint32_t areaptrn_vaddr;
	lib_graphics_s_bitmap_t *bitmap;
	uint32_t bitmap_vaddr;
	struct BitMap *bitmap_real;
	lib_graphics_s_tmpras_t *tmpras;
	uint32_t tmpras_vaddr;
	struct TmpRas *tmpras_real;
	lib_graphics_s_areainfo_t *areainfo;
	uint32_t areainfo_vaddr;
	struct AreaInfo *areainfo_real;
	lib_graphics_s_gelsinfo_t *gelsinfo;
	uint32_t gelsinfo_vaddr;
	struct GelsInfo *gelsinfo_real;
	uint8_t mask, fgpen, bgpen, aolpen;
	uint8_t drawmode, areaptsz, linpatcnt, dummy;
	uint16_t flags, lineptrn, cp_x, cp_y;
	uint8_t minterms[8];
	uint16_t penwidth, penheight;
	lib_graphics_s_textfont_t *font;
	struct TextFont *font_real;
	uint32_t font_vaddr;
	uint8_t algostyle, txflags;
	uint16_t txheight, txwidth, txbaseline, txspacing;
	uint32_t rp_user;
	uint32_t longreserved[2];
	uint16_t wordreserved[7];
	uint8_t reserved[8];

	char symbol[200];

	DEBUG(5) dprintf("graphics.library: lib_graphics_s_rastport_get_vaddr() called\n");

	if(vaddr == 0)
                return NULL;

        /* Search for an existing object */
        entry = mmu_findEntryWithType(vaddr, MMU_TYPE_GRAPHICS_S_RASTPORT);
        if(entry != NULL)
	        return (lib_graphics_s_rastport_t *)entry->nativePtr;

        /* This structure can be created from user space */

	/* Read parameters */
	if(
		READMEM_32(vaddr,&layer_vaddr) ||
		READMEM_32(vaddr+4,&bitmap_vaddr) ||
		READMEM_32(vaddr+8,&areaptrn_vaddr) ||
		READMEM_32(vaddr+12,&tmpras_vaddr) ||
		READMEM_32(vaddr+16,&areainfo_vaddr) ||
		READMEM_32(vaddr+20,&gelsinfo_vaddr) ||
		READMEM_8(vaddr+24,&mask) ||
		READMEM_8(vaddr+25,&fgpen) ||
		READMEM_8(vaddr+26,&bgpen) ||
		READMEM_8(vaddr+27,&aolpen) ||
		READMEM_8(vaddr+28,&drawmode) ||
		READMEM_8(vaddr+29,&areaptsz) ||
		READMEM_8(vaddr+30,&linpatcnt) ||
		READMEM_8(vaddr+31,&dummy) ||
		READMEM_16(vaddr+32,&flags) ||
		READMEM_16(vaddr+34,&lineptrn) ||
		READMEM_16(vaddr+36,&cp_x) ||
		READMEM_16(vaddr+38,&cp_y) ||
		READMEM_8(vaddr+40,&minterms[0]) ||
		READMEM_8(vaddr+41,&minterms[1]) ||
		READMEM_8(vaddr+42,&minterms[2]) ||
		READMEM_8(vaddr+43,&minterms[3]) ||
		READMEM_8(vaddr+44,&minterms[4]) ||
		READMEM_8(vaddr+45,&minterms[5]) ||
		READMEM_8(vaddr+46,&minterms[6]) ||
		READMEM_8(vaddr+47,&minterms[7]) ||
		READMEM_16(vaddr+48,&penwidth) ||
		READMEM_16(vaddr+50,&penheight) ||
		READMEM_32(vaddr+52,&font_vaddr) ||
		READMEM_8(vaddr+56,&algostyle) ||
		READMEM_8(vaddr+57,&txflags) ||
		READMEM_16(vaddr+58,&txheight) ||
		READMEM_16(vaddr+60,&txwidth) ||
		READMEM_16(vaddr+62,&txbaseline) ||
		READMEM_16(vaddr+64,&txspacing) ||
		READMEM_32(vaddr+66,&rp_user) ||
		READMEM_32(vaddr+70,&longreserved[0]) ||
		READMEM_32(vaddr+74,&longreserved[1]) ||
		READMEM_16(vaddr+78,&wordreserved[0]) ||
		READMEM_16(vaddr+80,&wordreserved[1]) ||
		READMEM_16(vaddr+82,&wordreserved[2]) ||
		READMEM_16(vaddr+84,&wordreserved[3]) ||
		READMEM_16(vaddr+86,&wordreserved[4]) ||
		READMEM_16(vaddr+88,&wordreserved[5]) ||
		READMEM_16(vaddr+90,&wordreserved[6]) ||
		READMEM_8(vaddr+92,&reserved[0]) ||
		READMEM_8(vaddr+93,&reserved[1]) ||
		READMEM_8(vaddr+94,&reserved[2]) ||
		READMEM_8(vaddr+95,&reserved[3]) ||
		READMEM_8(vaddr+96,&reserved[4]) ||
		READMEM_8(vaddr+97,&reserved[5]) ||
		READMEM_8(vaddr+98,&reserved[6]) ||
		READMEM_8(vaddr+99,&reserved[7])
        ) {
                return NULL;
        }

	layer_real = NULL;
	if(layer_vaddr != 0) {
		layer = lib_graphics_s_layer_get_vaddr(layer_vaddr);
		if(layer == NULL) {
                	DEBUG(1) dprintf("Error: graphics.library: struct RastPort: Could not create mapping for member: Layer\n");
	                return NULL;
		}
		layer_real = layer->real;
	}

	bitmap_real = NULL;
	if(bitmap_vaddr != 0) {
		bitmap = lib_graphics_s_bitmap_get_vaddr(bitmap_vaddr);
		if(bitmap == NULL) {
                	DEBUG(1) dprintf("Error: graphics.library: struct RastPort: Could not create mapping for member: BitMap\n");
	                return NULL;
		}
		bitmap_real = bitmap->real;
	}

	tmpras_real = NULL;
	if(tmpras_vaddr != 0) {
		tmpras = lib_graphics_s_tmpras_get_vaddr(tmpras_vaddr);
		if(tmpras == NULL) {
                	DEBUG(1) dprintf("Error: graphics.library: struct RastPort: Could not create mapping for member: TmpRas\n");
	                return NULL;
		}
		tmpras_real = tmpras->real;
	}

	areainfo_real = NULL;
	if(areainfo_vaddr != 0) {
		areainfo = lib_graphics_s_areainfo_get_vaddr(areainfo_vaddr);
		if(areainfo == NULL) {
                	DEBUG(1) dprintf("Error: graphics.library: struct RastPort: Could not create mapping for member: AreaInfo\n");
	                return NULL;
		}
		areainfo_real = areainfo->real;
	}

	gelsinfo_real = NULL;
	if(gelsinfo_vaddr != 0) {
		gelsinfo = lib_graphics_s_gelsinfo_get_vaddr(gelsinfo_vaddr);
		if(gelsinfo == NULL) {
                	DEBUG(1) dprintf("Error: graphics.library: struct RastPort: Could not create mapping for member: GelsInfo\n");
	                return NULL;
		}
		gelsinfo_real = gelsinfo->real;
	}

	font_real = NULL;
	if(font_vaddr != 0) {
		font = lib_graphics_s_textfont_get_vaddr(font_vaddr);
		if(font == NULL) {
                	DEBUG(1) dprintf("Error: graphics.library: struct RastPort: Could not create mapping for member: Font\n");
	                return NULL;
		}
		font_real = font->real;
	}

        /* Allocate memory for structure */
        real = (struct RastPort *)AllocVec(sizeof(struct RastPort), MEMF_PUBLIC|MEMF_CLEAR);
        if(real == NULL) {
                return NULL;
        }

        /* Set up real object */
	real->Layer = layer_real;
	real->BitMap = bitmap_real;
	real->AreaPtrn = NULL;
	real->TmpRas = tmpras_real;
	real->AreaInfo = areainfo_real;
	real->GelsInfo = gelsinfo_real;
	real->Mask = mask;
	real->FgPen = (BYTE)fgpen;
	real->BgPen = (BYTE)bgpen;
	real->AOlPen = (BYTE)aolpen;
	real->DrawMode = (BYTE)drawmode;
	real->AreaPtSz = (BYTE)areaptsz;
	real->linpatcnt = (BYTE)linpatcnt;
	real->dummy = (BYTE)dummy;
	real->Flags = flags;
	real->LinePtrn = lineptrn;
	real->cp_x = (WORD)cp_x;
	real->cp_y = (WORD)cp_y;
	real->minterms[0] = minterms[0];
	real->minterms[1] = minterms[1];
	real->minterms[2] = minterms[2];
	real->minterms[3] = minterms[3];
	real->minterms[4] = minterms[4];
	real->minterms[5] = minterms[5];
	real->minterms[6] = minterms[6];
	real->minterms[7] = minterms[7];
	real->PenWidth = (WORD)penwidth;
	real->PenHeight = (WORD)penheight;
	real->Font = font_real;
	real->AlgoStyle = algostyle;
	real->TxFlags = txflags;
	real->TxHeight = txheight;
	real->TxWidth = txwidth;
	real->TxBaseline = txbaseline;
	real->TxSpacing = (WORD)txspacing;
	real->RP_User = (APTR *)rp_user;
	real->longreserved[0] = longreserved[0];
	real->longreserved[1] = longreserved[1];
	real->wordreserved[0] = wordreserved[0];
	real->wordreserved[1] = wordreserved[1];
	real->wordreserved[2] = wordreserved[2];
	real->wordreserved[3] = wordreserved[3];
	real->wordreserved[4] = wordreserved[4];
	real->wordreserved[5] = wordreserved[5];
	real->wordreserved[6] = wordreserved[6];
	real->reserved[0] = reserved[0];
	real->reserved[1] = reserved[1];
	real->reserved[2] = reserved[2];
	real->reserved[3] = reserved[3];
	real->reserved[4] = reserved[4];
	real->reserved[5] = reserved[5];
	real->reserved[6] = reserved[6];
	real->reserved[7] = reserved[7];

        /* Set up an object */
        entry = mmu_allocObjVAddr(pool, real, vaddr);
        if(entry == NULL) {
                FreeVec(real);
                return NULL;
        }

        /* Extract object */
        obj = (lib_graphics_s_rastport_t *)entry->nativePtr;

        /* Init structure */
	entry->read = map_r;
	entry->write = map_w;
	entry->destructor = destructor;
        obj->entry = entry;
        obj->real = real;
	obj->userSpace = 1;	/* Created from user space */

        DEBUG(3) {
                /* Add extra symbols */
                sprintf(symbol, "!%08x.%s.Layer", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+0, symbol);
                sprintf(symbol, "!%08x.%s.BitMap", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+4, symbol);
                sprintf(symbol, "!%08x.%s.AreaPtrn", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+8, symbol);
                sprintf(symbol, "!%08x.%s.TmpRas", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+12, symbol);
                sprintf(symbol, "!%08x.%s.AreaInfo", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+16, symbol);
                sprintf(symbol, "!%08x.%s.GelsInfo", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+20, symbol);
                sprintf(symbol, "!%08x.%s.Mask", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+24, symbol);
                sprintf(symbol, "!%08x.%s.FgPen", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+25, symbol);
                sprintf(symbol, "!%08x.%s.BgPen", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+26, symbol);
                sprintf(symbol, "!%08x.%s.AOlPen", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+27, symbol);
                sprintf(symbol, "!%08x.%s.DrawMode", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+28, symbol);
                sprintf(symbol, "!%08x.%s.AreaPtSz", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+29, symbol);
                sprintf(symbol, "!%08x.%s.linpatcnt", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+30, symbol);
                sprintf(symbol, "!%08x.%s.dummy", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+31, symbol);
                sprintf(symbol, "!%08x.%s.Flags", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+32, symbol);
                sprintf(symbol, "!%08x.%s.LinePtrn", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+34, symbol);
                sprintf(symbol, "!%08x.%s.cp_x", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+36, symbol);
                sprintf(symbol, "!%08x.%s.cp_y", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+38, symbol);
                sprintf(symbol, "!%08x.%s.minterms[0]", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+40, symbol);
                sprintf(symbol, "!%08x.%s.PenWidth", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+48, symbol);
                sprintf(symbol, "!%08x.%s.PenHeight", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+50, symbol);
                sprintf(symbol, "!%08x.%s.Font", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+52, symbol);
                sprintf(symbol, "!%08x.%s.AlgoStyle", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+56, symbol);
                sprintf(symbol, "!%08x.%s.TxFlags", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+57, symbol);
                sprintf(symbol, "!%08x.%s.TxHeight", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+58, symbol);
                sprintf(symbol, "!%08x.%s.TxWidth", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+60, symbol);
                sprintf(symbol, "!%08x.%s.TxBaseline", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+62, symbol);
                sprintf(symbol, "!%08x.%s.TxSpacing", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+64, symbol);
                sprintf(symbol, "!%08x.%s.RP_User", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+66, symbol);
                sprintf(symbol, "!%08x.%s.longreserved[0]", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+70, symbol);
                sprintf(symbol, "!%08x.%s.wordreserved[0]", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+78, symbol);
                sprintf(symbol, "!%08x.%s.reserved[0]", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+92, symbol);
        }

	return obj;
}

/* Get an instance as a substructure */
lib_graphics_s_rastport_t *lib_graphics_s_rastport_get_substruct(struct RastPort *real, uint32_t vaddr)
{
	mmu_entry_t *entry;
	lib_graphics_s_rastport_t *obj;
	char symbol[200];

	DEBUG(5) dprintf("graphics.library: lib_graphics_s_rastport_get_substruct() called\n");

	if(real == NULL)
		return NULL;

        /* Check to find if it already exists */
        entry = mmu_findEntryWithTypeReal(real, MMU_TYPE_GRAPHICS_S_RASTPORT);
        if(entry != NULL) {

                /* Yes, return object */
                return (lib_graphics_s_rastport_t *)entry->nativePtr;
        }

        /* No, set up a substruct object */
        entry = mmu_allocObjSubStruct(pool, real, vaddr);
        if(entry == NULL)
                 return NULL;

        /* Extract object */
        obj = (lib_graphics_s_rastport_t *)entry->nativePtr;

        /* Init structure */
	entry->read = map_r;
	entry->write = map_w;
	entry->destructor = destructor;
        obj->entry = entry;
        obj->real = real;
	obj->userSpace = 0;	/* Created from real memory */

        DEBUG(3) {
                /* Add extra symbols */
		vaddr = entry->startAddr;
                sprintf(symbol, "!%08x.%s.Layer", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+0, symbol);
                sprintf(symbol, "!%08x.%s.BitMap", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+4, symbol);
                sprintf(symbol, "!%08x.%s.AreaPtrn", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+8, symbol);
                sprintf(symbol, "!%08x.%s.TmpRas", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+12, symbol);
                sprintf(symbol, "!%08x.%s.AreaInfo", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+16, symbol);
                sprintf(symbol, "!%08x.%s.GelsInfo", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+20, symbol);
                sprintf(symbol, "!%08x.%s.Mask", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+24, symbol);
                sprintf(symbol, "!%08x.%s.FgPen", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+25, symbol);
                sprintf(symbol, "!%08x.%s.BgPen", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+26, symbol);
                sprintf(symbol, "!%08x.%s.AOlPen", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+27, symbol);
                sprintf(symbol, "!%08x.%s.DrawMode", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+28, symbol);
                sprintf(symbol, "!%08x.%s.AreaPtSz", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+29, symbol);
                sprintf(symbol, "!%08x.%s.linpatcnt", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+30, symbol);
                sprintf(symbol, "!%08x.%s.dummy", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+31, symbol);
                sprintf(symbol, "!%08x.%s.Flags", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+32, symbol);
                sprintf(symbol, "!%08x.%s.LinePtrn", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+34, symbol);
                sprintf(symbol, "!%08x.%s.cp_x", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+36, symbol);
                sprintf(symbol, "!%08x.%s.cp_y", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+38, symbol);
                sprintf(symbol, "!%08x.%s.minterms[0]", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+40, symbol);
                sprintf(symbol, "!%08x.%s.PenWidth", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+48, symbol);
                sprintf(symbol, "!%08x.%s.PenHeight", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+50, symbol);
                sprintf(symbol, "!%08x.%s.Font", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+52, symbol);
                sprintf(symbol, "!%08x.%s.AlgoStyle", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+56, symbol);
                sprintf(symbol, "!%08x.%s.TxFlags", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+57, symbol);
                sprintf(symbol, "!%08x.%s.TxHeight", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+58, symbol);
                sprintf(symbol, "!%08x.%s.TxWidth", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+60, symbol);
                sprintf(symbol, "!%08x.%s.TxBaseline", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+62, symbol);
                sprintf(symbol, "!%08x.%s.TxSpacing", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+64, symbol);
                sprintf(symbol, "!%08x.%s.RP_User", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+66, symbol);
                sprintf(symbol, "!%08x.%s.longreserved[0]", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+70, symbol);
                sprintf(symbol, "!%08x.%s.wordreserved[0]", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+78, symbol);
                sprintf(symbol, "!%08x.%s.reserved[0]", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+92, symbol);
        }

	return obj;
}

/* Destructor of mapping */
static void destructor(mmu_entry_t *entry)
{
	lib_graphics_s_rastport_t *obj;
        struct RastPort *real;

        DEBUG(5) dprintf("graphics.library: lib_graphics_s_rastport: destructor() called\n");

        obj = (lib_graphics_s_rastport_t *)entry->nativePtr;

        /* Created from userspace? */
        if(obj->userSpace) {

                /* TODO: Copy back real object data to underlying memory? */

                real = obj->real;
                mmu_freeObjVAddr(entry);
                FreeVec(real);

        } else {

                mmu_freeObj(entry);
        }
}
