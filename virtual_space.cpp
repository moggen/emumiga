/*
virtual_space.cpp - Virtual memory space

Copyright (C) 2011  Magnus Öberg

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

   virtual_space::virtual_space()
   {
      DEBUG(5) dprintf("virtual_space::virtual_space() called. this=%p\n", this);
   }

   virtual_space::~virtual_space()
   {
      DEBUG(5) dprintf("virtual_space::~virtual_space() called. this=%p\n", this);
   }

   uint32_t virtual_space::add_mapping(virtual_mapping *mapping)
   {
      DEBUG(5) dprintf("virtual_space::add_mapping() called. this=%p, mapping=%p\n", this, mapping);

      return vmaplist.add(mapping);
   }

   uint32_t virtual_space::add_mapping_absolute(virtual_mapping *mapping, uint32_t address)
   {
      DEBUG(5) dprintf("virtual_space::add_mapping_absolute() called. this=%p, mapping=%p, address=0x%x\n", this, mapping, address);

      return vmaplist.add_absolute(mapping, address);
   }

   int virtual_space::rem_mapping(virtual_mapping *mapping)
   {
      DEBUG(5) dprintf("virtual_space::rem_mapping() called. this=%p, mapping=%p\n", this, mapping);

      return vmaplist.rem(mapping);
   }

   virtual_mapping *virtual_space::find_mapping(uint32_t address)
   {
      DEBUG(7) dprintf("virtual_space::find_mapping() called. this=%p, address=0x%x\n", this, address);

      return vmaplist.find_containing(address);
   }

   virtual_mapping *virtual_space::find_mapping_exact(uint32_t address)
   {
      DEBUG(7) dprintf("virtual_space::find_mapping_exact() called. this=%p, address=0x%x\n", this, address);

      return vmaplist.find(address);
   }

   virtual_object *virtual_space::find_object(uint32_t address, object_type *type)
   {
      DEBUG(7) dprintf("virtual_space::find_object() called. this=%p, address=0x%x, type=%p (%s)\n", this, address, type, type->get_name());

      virtual_mapping *vmap = vmaplist.find_containing(address);
      if(!vmap)
	 return NULL;

      return vmap->find_object(address, type);
   }

   int virtual_space::readmem_8(uint32_t address, uint8_t *data)
   {
      DEBUG(7) dprintf("virtual_space::readmem_8() called. this=%p, address=0x%x, data=%p\n", this, address, data);

      return mem_access(address, 1, data, 1);
   }

   int virtual_space::readmem_16(uint32_t address, uint16_t *data)
   {
      DEBUG(7) dprintf("virtual_space::readmem_16() called. this=%p, address=0x%x, data=%p\n", this, address, data);

      uint8_t raw[2];
      int ret = mem_access(address & ~1, 2, raw, 1);
      if(ret)
	 return ret;

      *data = 
	 ((uint16_t)raw[0] << 8) |
	 ((uint16_t)raw[1] << 0) ;

      return 0;
   }

   int virtual_space::readmem_32(uint32_t address, uint32_t *data)
   {
      DEBUG(7) dprintf("virtual_space::readmem_32() called. this=%p, address=0x%x, data=%p\n", this, address, data);

      uint8_t raw[4];
      int ret = mem_access(address & ~1, 4, raw, 1);
      if(ret)
	 return ret;

      *data = 
	 ((uint32_t)raw[0] << 24) |
	 ((uint32_t)raw[1] << 16) |
	 ((uint32_t)raw[2] <<  8) |
	 ((uint32_t)raw[3] <<  0) ;

      return 0;
   }

   int virtual_space::writemem_8(uint8_t data, uint32_t address)
   {
      DEBUG(7) dprintf("virtual_space::writemem_8() called. this=%p, data=0x%02x, address=0x%x\n", this, (unsigned int)data, address);

      return mem_access(address, 1, &data, 0);
   }

   int virtual_space::writemem_16(uint16_t data, uint32_t address)
   {
      DEBUG(7) dprintf("virtual_space::writemem_16() called. this=%p, data=0x%04x, address=0x%x\n", this, (unsigned int)data, address);

      uint8_t raw[2];

      raw[0] = (uint8_t)((data >> 8) & 0xff);
      raw[1] = (uint8_t)((data >> 0) & 0xff);

      return mem_access(address & ~1, 2, raw, 0);
   }

   int virtual_space::writemem_32(uint32_t data, uint32_t address)
   {
      DEBUG(7) dprintf("virtual_space::writemem_32() called. this=%p, data=0x%08x, address=0x%x\n", this, (unsigned int)data, address);

      uint8_t raw[4];

      raw[0] = (uint8_t)((data >> 24) & 0xff);
      raw[1] = (uint8_t)((data >> 16) & 0xff);
      raw[2] = (uint8_t)((data >>  8) & 0xff);
      raw[3] = (uint8_t)((data >>  0) & 0xff);

      return mem_access(address & ~1, 4, raw, 0);
   }

   int virtual_space::mem_access(uint32_t address, uint32_t len, uint8_t *buffer, int read)
   {
      DEBUG(7) dprintf("virtual_space::mem_access() called. this=%p, address=0x%x, len=%d, buffer=%p, read=%d\n", this, address, len, buffer, read);

      uint32_t end, blockend, blocklen;
      virtual_mapping *mapping;
      int retc;

      end = address + len;
      if(end < address)
	 return EINVAL;

      while(address < end) {
	 mapping = find_mapping(address);
	 if(mapping == NULL)
	    return EPERM;

	 blockend = mapping->get_address() + mapping->get_length();
	 if(blockend > end)
	    blockend = end;

	 blocklen = blockend-address;

	 retc = mapping->mem_access(address, blocklen, buffer, read);

	 if(retc)
	    return retc;

	 address += blocklen;
	 buffer += blocklen;
      }
      return 0;
   }

   int virtual_space::print_symbol(uint32_t address, char *str, int str_length)
   {
      DEBUG(7) dprintf("virtual_space::print_symbol() called. this=%p, address=0x%x, str=%p, str_length=%d\n", this, address, str, str_length);

      virtual_mapping *vmap = vmaplist.find_containing(address);

      if(vmap == NULL)
         return -1;

      return vmap->print_symbol(address, str, str_length);
   }

} // namespace emumiga
