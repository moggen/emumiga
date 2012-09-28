/*
library_mapping.h - Virtual memory mapping base class for libraries

Copyright (C) 2011, 2012  Magnus �berg

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

namespace emumiga
{
   class library_mapping : public virtual_mapping {
     public:

      // Constructors
      library_mapping(int module_id, int vector_count, int lib_size);

      // Base class overrides
      virtual ~library_mapping();
      virtual int mem_access(uint32_t address, uint32_t len, uint8_t *buffer, int read);
      virtual void set_address(uint32_t address);
      virtual int print_symbol(uint32_t address, char *str, int str_length);

      // Instance methods
      virtual int init();
      virtual int dispose();
      uint32_t get_base_address();

      // Instance attributes
      int module_id;
      int vector_count;
      uint8_t *vector_mem;
      uint8_t *emulator_mem;

   };

} // namespace emumiga
