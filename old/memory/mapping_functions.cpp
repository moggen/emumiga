/*
memory/mapping_functions.c - Simple memory mapping functions

Copyright (C) 2007, 2008, 2009, 2010  Magnus Öberg

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

#include <stdint.h>

#include "../debug.h"
#include "mapping.h"

namespace emumiga {
namespace memory {

/* Binary reading from nativePtr */
int mapping::fn_r_binary(uint32_t offset, uint32_t size, uint8_t *data)
{
	uint8_t *native = (uint8_t *)native_start + offset;

        DEBUG(7) dprintf("mmu: mmy_fn_r_binary() called. VAddr: 0x%x, Size: 0x%x, Native: %p\n", vaddr_start+offset, size, native);

	while(size--){
		*data++ = *native++;
	}

	/* 0 means OK. */
	return 0;
}

/* Binary writing to nativePtr */
int mapping::fn_w_binary(uint32_t offset, uint32_t size, uint8_t *data)
{
	uint8_t *native = (uint8_t *)native_start + offset;

        DEBUG(7) dprintf("mmu: mmy_fn_w_binary() called. VAddr: 0x%x, Size: 0x%x, Native: %p\n", vaddr_start+offset, size, native);

	while(size--){
		*native++ = *data++;
	}

	/* 0 means OK. */
	return 0;
}

/* Protected, always fails. Can be used for both reading and writing */
int mapping::fn_protected(
	uint32_t offset,
	uint32_t size __attribute__((unused)),
	uint8_t *data __attribute__((unused))
)
{
	dprintf("Error: Reference of protected address: 0x%x\n", offset);

	/* 0 means OK. */
	return 1;
}

/* Unmapped, always fails. Can be used for both reading and writing */
int mapping::fn_unmapped(
	uint32_t offset,
	uint32_t size __attribute__((unused)),
        uint8_t *data __attribute__((unused))
)
{
	dprintf("Error: Reference of unmapped address: 0x%x\n", offset);

	/* 0 means OK. */
	return 1;
}

} // namespace memory
} // namespace emumiga
