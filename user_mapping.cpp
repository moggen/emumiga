/*
user_mapping.cpp - Virtual memory mapping created in userspace

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

   user_mapping::user_mapping(uint32_t length, uint8_t *real_memory):
      virtual_mapping(length), real_memory(real_memory)
   {
      DEBUG(5) dprintf("user_mapping::user_mapping() called. this=%p, length=%d, real_memory=%p\n", this, length, real_memory);

      this->type = TYPE_USER;
   }

   user_mapping::~user_mapping()
   {
      DEBUG(5) dprintf("user_mapping::~user_mapping() called. this=%p\n", this);

      FreeVec(real_memory);
   }

   user_mapping *user_mapping::create(uint32_t length)
   {
      DEBUG(5) dprintf("user_mapping::create() called. length=%d\n", length);

      uint8_t *mem = (uint8_t *)AllocVec(length, MEMF_PUBLIC|MEMF_CLEAR);
      if(!mem)
	 return NULL;

      user_mapping *mapping = new user_mapping(length, mem);
 
      if(!mapping) {
	 FreeVec(mem);
         return NULL;
      }

      emulator *emu = emulator::getEmulator();
      uint32_t new_address = emu->v_space.add_mapping(mapping);

      // TODO: Add to r_space too?

      if(new_address == 0xFFFFFFFF) {
	 delete mapping;
         return NULL;
      }

      return mapping;
   }

   int user_mapping::mem_access(uint32_t address, uint32_t len, uint8_t *buffer, int read)
   {
      DEBUG(7) dprintf("user_mapping::mem_access() called. this=%p, address=0x%x, len=%d, buffer=%p, read=%d\n", this, address, len, buffer, read);

      uint32_t end, blockend, blocklen;
      virtual_object *object;
      int retc;

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
	    if(read)
	       *buffer = real_memory[address - this->address];
	    else
	       real_memory[address - this->address] = *buffer;

	    address++;
	    buffer++;
	 }
      }
      return 0;
   }

   int user_mapping::print_symbol(uint32_t address, char *str, int str_length)
   {
      static symbol const map_symbols[] = {
         {0, "user"}, {0, NULL}
      };

      DEBUG(7) dprintf("user_mapping::print_symbol() called. this=%p, address=0x%x, str=%p, str_length=%d\n", this, address, str, str_length);

      return print_symbol_std(&map_symbols[0], address, str, str_length);
   }

   uint8_t *user_mapping::get_real_buffer(uint32_t address, uint32_t length)
   {
      // In range?
      if(address < this->address || this->address+this->length < address+length)
         return NULL;

      if(vobjlist.contains_objects(address, length))
         return NULL;

      return real_memory + (address - this->address);
   }

} // namespace emumiga
