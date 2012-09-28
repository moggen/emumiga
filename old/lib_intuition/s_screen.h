/*
lib_intuition/s_screen.h - Header file for intuition.library
	struct Screen

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

#ifndef _LIB_INTUITION_S_SCREEN_H_
#define _LIB_INTUITION_S_SCREEN_H_

#include <stdint.h>
#include <intuition/screens.h>

#include "../mmu/mmu.h"
#include "../lib_graphics/s_viewport.h"
#include "../lib_graphics/s_rastport.h"
#include "../lib_graphics/s_layer_info.h"
#include "../lib_graphics/s_bitmap.h"

/* Kickstart 2.0 */
#define LIB_INTUITION_S_SCREEN_SIZE 346

/* Structure for simulated object */
struct lib_intuition_s_screen_struct {
        mmu_entry_t *entry;
	struct Screen *real;

	lib_graphics_s_viewport_t *ViewPort;
	lib_graphics_s_rastport_t *RastPort;
	lib_graphics_s_bitmap_t *BitMap;
	lib_graphics_s_layer_info_t *LayerInfo;
};
typedef struct lib_intuition_s_screen_struct lib_intuition_s_screen_t;

/* Predecls */
int lib_intuition_s_screen_init();
void lib_intuition_s_screen_cleanup();
lib_intuition_s_screen_t *lib_intuition_s_screen_get_real(struct Screen *real);
lib_intuition_s_screen_t *lib_intuition_s_screen_get_vaddr(uint32_t vaddr);

#endif
