/*
modules/exec/zeropage.cpp - Virtual memory mapping for the first page in memory

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

#include "exec.h"

namespace emumiga
{
   namespace exec
   {

      zeropage::zeropage():
         virtual_mapping(4096), execbase(0)
      {
         DEBUG(5) dprintf("exec::zeropage::zeropage() called. this=%p\n", this);
         type = TYPE_ROM;
      }

      zeropage::~zeropage()
      {
         DEBUG(5) dprintf("exec::zeropage::~zeropage() called. this=%p\n", this);
      }

      int zeropage::add_object(virtual_object *object)
      {
         DEBUG(5) dprintf("exec::zeropage::add_object() called. this=%p, object=%p\n", this, object);
         // This mapping does not allow virtual objects in it
         return EPERM;
      }

      int zeropage::add_object_no_clean(virtual_object *object)
      {
         DEBUG(5) dprintf("exec::zeropage::add_object_no_clean() called. this=%p, object=%p\n", this, object);
         // This mapping does not allow virtual objects in it
         return EPERM;
      }

      int zeropage::mem_access(uint32_t address, uint32_t len, uint8_t *buffer, int read)
      {
         DEBUG(7) dprintf("exec::zeropage::mem_access() called. this=%p, address=0x%x, len=%d, buffer=%p, read=%d\n", this, address, len, buffer, read);

         if(!read)
            return EPERM;

         for(uint32_t addr=address; addr<(address+len); addr++) {
            switch(addr) {
               case 4: *buffer = (uint8_t)((execbase >> 24) & 0xff); break;
               case 5: *buffer = (uint8_t)((execbase >> 16) & 0xff); break;
               case 6: *buffer = (uint8_t)((execbase >> 8 ) & 0xff); break;
               case 7: *buffer = (uint8_t)((execbase      ) & 0xff); break;
               default: return EPERM;
            }
            buffer++;
         }
         return 0;
      }

      void zeropage::set_execbase(uint32_t address)
      {
         DEBUG(5) dprintf("exec::zeropage::set_execbase() called. this=%p, address=0x%x\n", this, address);
         execbase = address;
      }

      int zeropage::print_symbol(uint32_t address, char *str, int str_length)
      {
         static symbol const map_symbols[] = {
            {0, "zeropage"}, {4,"AbsExecBase"}, {0, NULL}
         };

         DEBUG(7) dprintf("exec::zeropage:print_symbol() called. this=%p, address=0x%x, str=%p, str_length=%d\n", this, address, str, str_length);

         return print_symbol_std(&map_symbols[0], address, str, str_length);
      }

   } // namespace exec
} // namespace emumiga
