/*
virtual_space.h - Virtual memory space

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

#ifndef EMUMIGA_H
#error "Don't include individual headers, use emumiga.h"
#endif

namespace emumiga {

   class virtual_space {
     public:
      virtual_space();
      ~virtual_space();

      // Instance attributes
      vmap_list vmaplist;

      // Instance methods
      uint32_t add_mapping(virtual_mapping *mapping);
      uint32_t add_mapping_absolute(virtual_mapping *mapping, uint32_t address);
      int rem_mapping(virtual_mapping *mapping);

      virtual_mapping *find_mapping(uint32_t address);
      virtual_mapping *find_mapping_exact(uint32_t address);
      virtual_object *find_object(uint32_t address, object_type *type);

      int readmem_8(uint32_t address, uint8_t *data);
      int readmem_16(uint32_t address, uint16_t *data);
      int readmem_32(uint32_t address, uint32_t *data);
      int writemem_8(uint8_t data, uint32_t address);
      int writemem_16(uint16_t data, uint32_t address);
      int writemem_32(uint32_t data, uint32_t address);
      int mem_access(uint32_t address, uint32_t len, uint8_t *buffer, int read);

      int print_symbol(uint32_t address, char *str, int str_length);
   };

} // namespace emumiga
