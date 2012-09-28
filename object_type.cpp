/*
object_type.cpp - AmigaOS/AROS types

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

   object_type::object_type(module *mod, int id, uint32_t priority, uint32_t size,
                            int real_size, char const *name, address_func *address_cb,
                            real_func *real_cb):
      _module(mod), _id(id), _priority(priority), _size(size), _real_size(real_size),
      _name(name), _address_cb(address_cb), _real_cb(real_cb)
   {
      DEBUG(5) dprintf("object_type::object_type() called. this=%p, module=%p, id=%d, priority=%d, size=%d, real_size=%d, name=\"%s\", address_cb=%p, real_cb=%p\n", this, mod, id, priority, size, real_size, name, address_cb, real_cb);
   }

   object_type::~object_type()
   {
      DEBUG(5) dprintf("object_type::~object_type() called. this=%p\n", this);
   }

   module *object_type::get_module()
   {
      DEBUG(5) dprintf("object_type::get_module() called. this=%p\n", this);
      return _module;
   }

   int object_type::get_id()
   {
      DEBUG(5) dprintf("object_type::get_id() called. this=%p\n", this);
      return _id;
   }

   uint32_t object_type::get_priority()
   {
      DEBUG(5) dprintf("object_type::get_priority() called. this=%p\n", this);
      return _priority;
   }

   uint32_t object_type::get_size()
   {
      DEBUG(5) dprintf("object_type::get_size() called. this=%p\n", this);
      return _size;
   }

   int object_type::get_real_size()
   {
      DEBUG(5) dprintf("object_type::get_real_size() called. this=%p\n", this);
      return _real_size;
   }

   char const *object_type::get_name()
   {
      DEBUG(5) dprintf("object_type::get_name() called. this=%p\n", this);
      return _name;
   }

   bool object_type::equals(object_type *type)
   {
      DEBUG(5) dprintf("object_type::equals() called. this=%p, type=%p (%s)\n", this, type, type->get_name());

      if(this == type)
         return true;

      if(_id == type->get_id() && _module == type->get_module())
         return true;

      return false;
   }

   virtual_object *object_type::create_from_address(uint32_t address)
   {
      DEBUG(5) dprintf("object_type::create_from_address() called. this=%p, address=0x%x\n", this, address);

      if(_address_cb == NULL) {
         dprintf("Error: object_type::create_from_address(): Callback missing for module '%s' structure '%s'\n", _module->get_name(), _name);
         return NULL;
      }

      return (*_address_cb)(address);
   }

   virtual_object *object_type::create_from_real(void *real)
   {
      DEBUG(5) dprintf("object_type::create_from_real() called. this=%p, real=%p\n", this, real);

      if(_real_cb == NULL) {
         dprintf("Error: object_type::create_from_real(): Callback missing for module '%s' structure '%s'\n", _module->get_name(), _name);
         return NULL;
      }

      return (*_real_cb)(real);
   }

} // namespace emumiga
