/*
rom_mapping.cpp - Virtual memory mapping in ROM memory

Copyright (C) 2011, 2012  Magnus Öberg

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

#include "emumiga.h"

namespace emumiga {

   rom_mapping::rom_mapping(uint32_t length, uint8_t const *real_memory)
      :virtual_mapping(length)
   {
      DEBUG(5) dprintf("rom_mapping::rom_mapping() called. this=%p, length=%d, real_memory=%p\n", this, length, real_memory);

      this->type = TYPE_ROM;
      this->real_memory = real_memory;
   }

   rom_mapping::~rom_mapping()
   {
      DEBUG(5) dprintf("rom_mapping::~rom_mapping() called. this=%p\n", this);
   }

   int rom_mapping::mem_access(uint32_t address, uint32_t len, uint8_t *buffer, int read)
   {
      DEBUG(7) dprintf("rom_mapping::mem_access() called. this=%p, address=0x%x, len=%d, buffer=%p, read=%d\n", this, address, len, buffer, read);

      uint32_t end, blockend, blocklen;
      virtual_object *object;
      int retc;

      if(!read)
	 return EPERM;

      end = address + len;
      if(end < address)
	 return EINVAL;

      while(address < end) {
	 object = vobjlist.find_containing(address, &blocklen);

	 if(object != NULL) {
	    // Object, delegate access
	    blockend = address + blocklen;

	    if(blockend > end)
	       blockend = end;

	    blocklen = blockend-address;

	    retc = object->mem_access(address, blocklen, buffer, read);

	    if(retc)
	       return retc;

	    address += blocklen;
	    buffer += blocklen;

	 } else {
	    // No object, just binary data
	    *buffer = real_memory[address - this->address];

	    address++;
	    buffer++;
	 }
      }
      return 0;
   }

   int rom_mapping::print_symbol(uint32_t address, char *str, int str_length)
   {
      static symbol const map_symbols[] = {
         {0, "rom"}, {0, NULL}
      };

      DEBUG(7) dprintf("rom_mapping::print_symbol() called. this=%p, address=0x%x, str=%p, str_length=%d\n", this, address, str, str_length);

      return print_symbol_std(&map_symbols[0], address, str, str_length);
   }

} // namespace emumiga
