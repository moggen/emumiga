/*
virtual_mapping.cpp - Virtual memory mapping

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

   virtual_mapping::virtual_mapping(uint32_t length):
      type(TYPE_NONE), address(0xFFFFFFFF), length(length)
   {
      DEBUG(5) dprintf("virtual_mapping::virtual_mapping() called. this=%p, length=%d\n", this, length);
   }

   virtual_mapping::~virtual_mapping()
   {
      DEBUG(5) dprintf("virtual_mapping::~virtual_mapping() called. this=%p\n", this);
   }

   int virtual_mapping::dispose()
   {
      DEBUG(5) dprintf("virtual_mapping::dispose() called. this=%p\n", this);

      if(address != 0xFFFFFFFF) {
         emulator *emu = emulator::getEmulator();
         emu->v_space.rem_mapping(this);
      }

      vobjlist.dispose_all();

      delete this;
      return 0;
   }

   int virtual_mapping::add_object(virtual_object *object)
   {
      DEBUG(5) dprintf("virtual_mapping::add_object() called. this=%p, object=%p\n", this, object);

      // TODO: check validity of object address and length

      int ret = vobjlist.add(object, false);
      if(!ret)
	 object->set_virtual_mapping(this);
      return ret;
   }

   int virtual_mapping::add_object_no_clean(virtual_object *object)
   {
      DEBUG(5) dprintf("virtual_mapping::add_object_no_clean() called. this=%p, object=%p\n", this, object);

      // TODO: check validity of object address and length

      int ret = vobjlist.add(object, true);
      if(!ret)
	 object->set_virtual_mapping(this);
      return ret;
   }

   int virtual_mapping::rem_object(virtual_object *object)
   {
      DEBUG(5) dprintf("virtual_mapping::rem_object() called. this=%p, object=%p\n", this, object);

      int ret = vobjlist.rem(object);
      if(!ret)
	 object->set_virtual_mapping(NULL);
      return ret;
   }

   int virtual_mapping::object_count()
   {
      DEBUG(5) dprintf("virtual_mapping::object_count() called. this=%p\n", this);

      return vobjlist.count();
   }

   virtual_object *virtual_mapping::find_object(uint32_t address, object_type *type)
   {
      DEBUG(7) dprintf("virtual_mapping::find_object() called. this=%p, address=0x%x, type=%p (%s)\n", this, address, type, type->get_name());

      return vobjlist.find_object(address, type);
   }

   int virtual_mapping::mem_access(uint32_t address, uint32_t len, uint8_t *buffer, int read)
   {
      DEBUG(7) dprintf("virtual_mapping::mem_access() called. this=%p, address=0x%x, len=%d, buffer=%p, read=%d\n", this, address, len, buffer, read);

      uint32_t end, blockend, blocklen;
      virtual_object *object;
      int retc;

      end = address + len;
      if(end < address)
	 return EINVAL;

      while(address < end) {
	 object = vobjlist.find_containing(address, &blocklen);
	 if(object == NULL)
	    return EPERM;

	 blockend = address + blocklen;
	 if(blockend > end)
	    blockend = end;

	 blocklen = blockend-address;

	 retc = object->mem_access(address, blocklen, buffer, read);

	 if(retc)
	    return retc;

	 address += blocklen;
	 buffer += blocklen;
      }
      return 0;
   }

   uint32_t virtual_mapping::get_address()
   {
      return address;
   }

   uint32_t virtual_mapping::get_length()
   {
      return length;
   }

   void virtual_mapping::set_address(uint32_t address)
   {
      DEBUG(5) dprintf("virtual_mapping::set_address() called. this=%p, address=0x%x\n", this, address);
      this->address = address;
   }

   virtual_mapping::mapping_type_t virtual_mapping::get_type()
   {
      return type;
   }

   int virtual_mapping::print_symbol_std(symbol const *symbol, uint32_t address, char *str, int str_length)
   {
      if(address < this->address || address >= (this->address + this->length))
         return -1;

      uint32_t offset = address - this->address;
      emumiga::symbol const *best = symbol;
      while(symbol->text != NULL) {
         if(symbol->offset > offset)
            break;
         best = symbol;
         symbol++;
      }

      int res = best->print_system(str, str_length, offset - best->offset);
      if(res < 0)
         return res;

      virtual_object *vobj = vobjlist.find_containing(address, NULL);

      if(vobj == NULL)
         return -1;

      res = vobj->print_symbol(address, str, str_length);
      if(res < 0)
         return res;

      return str_length - strlen(str) - 1;
   }

   int virtual_mapping::print_symbol(uint32_t address, char *str, int str_length)
   {
      static symbol const map_symbols[] = {
         {0, "virtual_mapping"}, {0, NULL}
      };

      DEBUG(7) dprintf("virtual_mapping::print_symbol() called. this=%p, address=0x%x, str=%p, str_length=%d\n", this, address, str, str_length);

      return print_symbol_std(&map_symbols[0], address, str, str_length);
   }

} // namespace emumiga
