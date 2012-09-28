/*
virtual_object.h - Virtual memory object

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

#ifndef EMUMIGA_H
#error "Don't include individual headers, use emumiga.h"
#endif

namespace emumiga {

   class virtual_object {
     public:
      virtual_object(object_type *type, symbol const *symbols);
      virtual ~virtual_object();

      // Instance attributes
      object_type *_type;
      symbol const *_symbols;
      uint32_t address;
      void *real_address;
      virtual_mapping *mapping;
      virtual_object *mother;

      // Instance methods
      virtual int dispose();
      object_type *get_type();
      uint32_t get_size();
      int get_real_size();
      uint32_t get_prio();
      void set_address(uint32_t address);
      uint32_t get_address();
      void set_real_address(void *real_address);
      void *get_real_address();
      void set_virtual_mapping(virtual_mapping *mapping);
      virtual_mapping *get_virtual_mapping();
      virtual_object *get_mother();
      void set_mother(virtual_object *obj);
      virtual int mem_access(uint32_t address, uint32_t len, uint8_t *buffer, int read);
      int setup_from_address(uint32_t address);
      int setup_from_real(void *real);
      virtual int print_symbol(uint32_t address, char *str, int str_length);
   };

} // namespace emumiga
