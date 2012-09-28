/*
virtual_object.cpp - Virtual memory object

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

   virtual_object::virtual_object(object_type *type, symbol const *symbols):
      _type(type), _symbols(symbols), address(0xFFFFFFFF), real_address(NULL), mapping(NULL), mother(NULL)
   {
      DEBUG(5) dprintf("virtual_object::virtual_object() called. this=%p, type=%p (%s), symbols=%p\n", this, type, type->get_name(), symbols);
   }

   virtual_object::~virtual_object()
   {
      DEBUG(5) dprintf("virtual_object::~virtual_object() called. this=%p\n", this);
   }

   int virtual_object::dispose()
   {
      DEBUG(5) dprintf("virtual_object::dispose() called. this=%p\n", this);

      // If we have a mother object, let it handle the disposal.
      // A mother object must unregister from its child objects before it
      // in turn calls dispose() on them, otherwise we will have an
      // infinite loop.

      if(mother != NULL)
         return mother->dispose();

      if(mapping) {
         // We are added to a virtual mapping, remove us from it!
         mapping->rem_object(this);
      }

      if(real_address != NULL) {
         emulator *emu = emulator::getEmulator();
         int retc = emu->r_space.object_disposing(this);
         if(retc) return retc;
      }

      delete this;
      return 0;
   }

   object_type *virtual_object::get_type()
   {
      return _type;
   }

   void virtual_object::set_address(uint32_t address)
   {
      this->address = address;
   }

   uint32_t virtual_object::get_address()
   {
      return address;
   }

   uint32_t virtual_object::get_size()
   {
      return _type->get_size();
   }

   int virtual_object::get_real_size()
   {
      return _type->get_real_size();
   }

   uint32_t virtual_object::get_prio()
   {
      return _type->get_priority();
   }

   void virtual_object::set_real_address(void *real_address)
   {
      if(this->real_address != NULL) {
         DEBUG(1) dprintf("Warning: virtual_object::set_real_address() was called but the address is already set.\n");
         return;
      }

      this->real_address = real_address;

      emulator *emu = emulator::getEmulator();
      emu->r_space.object_setting_address(this);

      // TODO: Handle errors here?
   }

   void *virtual_object::get_real_address()
   {
      return real_address;
   }

   void virtual_object::set_virtual_mapping(virtual_mapping *mapping)
   {
      this->mapping = mapping;
   }

   virtual_mapping *virtual_object::get_virtual_mapping()
   {
      return mapping;
   }

   virtual_object *virtual_object::get_mother()
   {
      return mother;
   }

   void virtual_object::set_mother(virtual_object *obj)
   {
      this->mother = obj;
   }

   int virtual_object::mem_access(uint32_t address, uint32_t len, uint8_t *buffer, int read)
   {
      DEBUG(7) dprintf("virtual_object::mem_access() called. this=%p, address=0x%x, len=%d, buffer=%p, read=%d\n", this, address, len, buffer, read);

      // This method should be overridden
      return EPERM;
   }

   int virtual_object::setup_from_address(uint32_t address)
   {
      DEBUG(5) dprintf("virtual_object::setup_from_address() called. address=0x%x\n", address);

/* TBD
      sys_mapping *mapping = new sys_mapping(get_type());
      if(mapping == NULL) {
         dprintf("Error: virtual_object::setup_from_address(): Could not create sys_mapping.\n");
         return ENOMEM;
      }
 
      emulator *emu = emulator::getEmulator();
      uint32_t addr = emu->v_space.add_mapping(mapping);
      if(addr == 0xFFFFFFFF) {
         dprintf("Error: virtual_object::setup_from_address(): Could not add sys_mapping to virtual space.\n");
         mapping->dispose();
         return EFAULT;
      }

      set_address(addr);

      if(mapping->add_object(this)) {
         dprintf("Error: virtual_object::setup_from_address(): Could not add object to sys_mapping.\n");
         mapping->dispose();
         return EFAULT;
      }

      set_address_address(address);
*/
      return 0;
   }

   int virtual_object::setup_from_real(void *real)
   {
      DEBUG(5) dprintf("virtual_object::setup_from_real() called. real=%p\n", real);

      sys_mapping *mapping = new sys_mapping(get_type());
      if(mapping == NULL) {
         dprintf("Error: virtual_object::setup_from_real(): Could not create sys_mapping.\n");
         return ENOMEM;
      }
 
      emulator *emu = emulator::getEmulator();
      uint32_t addr = emu->v_space.add_mapping(mapping);
      if(addr == 0xFFFFFFFF) {
         dprintf("Error: virtual_object::setup_from_real(): Could not add sys_mapping to virtual space.\n");
         mapping->dispose();
         return EFAULT;
      }

      set_address(addr);

      if(mapping->add_object(this)) {
         dprintf("Error: virtual_object::setup_from_real(): Could not add object to sys_mapping.\n");
         mapping->dispose();
         return EFAULT;
      }

      set_real_address(real);

      return 0;
   }

   int virtual_object::print_symbol(uint32_t address, char *str, int str_length)
   {

      DEBUG(7) dprintf("virtual_object::print_symbol() called. this=%p, address=0x%x, str=%p, str_length=%d\n", this, address, str, str_length);

      if(address < this->address || address >= (this->address + this->_type->get_size()))
         return -1;

      symbol const *best_symbol = symbol::array_lookup(this->_symbols, address-this->address);
      if(best_symbol == NULL)
         return -1;

      return best_symbol->print_system(str, str_length, address-this->address);
   }

} // namespace emumiga
