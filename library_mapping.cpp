/*
library_mapping.cpp - Virtual memory mapping base class for libraries

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

namespace emumiga
{
   library_mapping::library_mapping(int module_id, int vector_count, int lib_size):
      virtual_mapping(vector_count*8 + vector_count*6 + lib_size),
      module_id(module_id), vector_count(vector_count), vector_mem(NULL),
      emulator_mem(NULL)
   {
      DEBUG(5) dprintf("library_mapping::library_mapping() called. this=%p, module_id=%d, vector_count=%d, lib_size=%d\n", this, module_id, vector_count, lib_size);
      type = TYPE_SYS;
   }

   library_mapping::~library_mapping()
   {
      DEBUG(5) dprintf("library_mapping::~library_mapping() called. this=%p\n", this);
   }

   int library_mapping::init()
   {
      DEBUG(5) dprintf("library_mapping::init() called. this=%p\n", this);

      vector_mem = (uint8_t *)AllocVec(vector_count*6, MEMF_PUBLIC|MEMF_CLEAR);
      if(vector_mem == NULL)
         return ENOMEM;

      emulator_mem = (uint8_t *)AllocVec(vector_count*8, MEMF_PUBLIC|MEMF_CLEAR);
      if(vector_mem == NULL) {
         FreeVec(vector_mem);
         return ENOMEM;
      }

      return 0;
   }

   int library_mapping::dispose()
   {
      DEBUG(5) dprintf("library_mapping::dispose() called. this=%p\n", this);

      if(vector_mem) FreeVec(vector_mem);
      if(emulator_mem) FreeVec(emulator_mem);

      return virtual_mapping::dispose();
   }

   int library_mapping::mem_access(uint32_t address, uint32_t len, uint8_t *buffer, int read)
   {
      DEBUG(7) dprintf("library_mapping::mem_access() called. this=%p, address=0x%x, len=%d, buffer=%p, read=%d\n", this, address, len, buffer, read);

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
            // No object
            int distance = address - this->address;
            if(distance < vector_count*8) {
               // Inside the emulator instructions, read only!
               if(!read)
                  return EPERM;

               *buffer = emulator_mem[distance];
            } else {
               // Inside jump vectors
               if(read)
                  *buffer = vector_mem[distance - vector_count*8];
               else
                  vector_mem[distance - vector_count*8] = *buffer;
            }
            address++;
            buffer++;
         }
      }
      return 0;
   }

   void library_mapping::set_address(uint32_t address)
   {
      DEBUG(5) dprintf("library_mapping::set_address() called. this=%p, address=0x%x\n", this, address);

      // Call superclass assignment
      virtual_mapping::set_address(address);
      
      // Loop over all vectors, cur_fnum counts down
      int cur_fnum = vector_count;
      uint32_t cur_emu = address;
      uint8_t *vmem = vector_mem;
      uint8_t *emem = emulator_mem;
 
      while(cur_fnum > 0)
      {
         // Write jump vector
         *vmem++ = 0x4E;
         *vmem++ = 0xF9;
         *vmem++ = (uint8_t)((cur_emu >> 24) & 0xff);
         *vmem++ = (uint8_t)((cur_emu >> 16) & 0xff);
         *vmem++ = (uint8_t)((cur_emu >> 8 ) & 0xff);
         *vmem++ = (uint8_t)((cur_emu      ) & 0xff);

         // Write emulator instruction + rts
         *emem++ = 0xFD;
         *emem++ = 0x80;
         *emem++ = (uint8_t)((module_id >> 8) & 0xff);
         *emem++ = (uint8_t)((module_id     ) & 0xff);
         *emem++ = (uint8_t)((cur_fnum >> 8) & 0xff);
         *emem++ = (uint8_t)((cur_fnum     ) & 0xff);
         *emem++ = 0x4E;
         *emem++ = 0x75;

         cur_emu += 8;
         cur_fnum--;
      }
   }

   uint32_t library_mapping::get_base_address()
   {
      DEBUG(5) dprintf("library_mapping::get_base_address() called. this=%p\n", this);
      return address + vector_count*8 + vector_count*6;
   }

   int library_mapping::print_symbol(uint32_t address, char *str, int str_length)
   {
      DEBUG(7) dprintf("library_mapping::print_symbol() called. this=%p, address=0x%x, str=%p, str_length=%d\n", this, address, str, str_length);

      if(address < this->address || address >= (this->address + this->length))
         return -1;

      char symbstr[200];
      emumiga::symbol symb = {0, NULL};
      emulator *emu = emulator::getEmulator();
      module *mod = emu->module_reg.findByID(module_id);
      uint32_t offset = address - this->address;

      if(offset < (uint32_t)vector_count*8) {
         // Hook section
         snprintf(symbstr, sizeof(symbstr) - 1, "%s:hooks", mod->get_name());
         symbstr[sizeof(symbstr) - 1] = '\0';
         symb.text = symbstr;

      } else if (offset < (uint32_t)(vector_count*8 + vector_count*6) ) {
         // Jump vectors
         snprintf(symbstr, sizeof(symbstr) - 1, "%s", mod->get_name());
         symbstr[sizeof(symbstr) - 1] = '\0';
         symb.offset = (uint32_t)(vector_count*8 + vector_count*6);
         symb.text = symbstr;
      }
      // The library base virtual object should handle its own symbols

      int res = symb.print_system(str, str_length, offset - symb.offset);
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

} // namespace emumiga
