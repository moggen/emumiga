/*
modules/exec/s_ExecBase.cpp - Virtual object for struct ExecBase

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
#include "../dos/dos.h"

namespace emumiga
{
   namespace exec
   {
      object_type *s_ExecBase::type = NULL;
      symbol const s_ExecBase::symbols[] = {
         {0,"ExecBase"},
         {276,"ExecBase:ThisTask"},
         {0,NULL}
      };

      s_ExecBase::s_ExecBase():
         virtual_object(type, symbols)
      {
         DEBUG(5) dprintf("exec::s_ExecBase::s_ExecBase() called. this=%p\n", this);
      }

      s_ExecBase::~s_ExecBase()
      {
         DEBUG(5) dprintf("exec::s_ExecBase::~s_ExecBase() called. this=%p\n", this);
      }

      // Return value conversion functions
      static virtual_object *cfa_func(uint32_t address)
      {
         return s_ExecBase::create_from_address(address);
      }

      static virtual_object *cfr_func(void *real)
      {
         return s_ExecBase::create_from_real(real);
      }


      int s_ExecBase::init(module *mod)
      {
         DEBUG(5) dprintf("exec::s_ExecBase::init() called. mod=%p\n", mod);

         type = new object_type(mod, id, prio, size, real_size, "ExecBase", cfa_func, cfr_func);

         if(type == NULL)
            return ENOMEM;

         return 0;
      }

      void s_ExecBase::cleanup()
      {
         DEBUG(5) dprintf("exec::s_ExecBase::cleanup() called.\n");

         delete type;
         type = NULL;
      }
     
      s_ExecBase *s_ExecBase::create_from_address(uint32_t address)
      {
         DEBUG(5) dprintf("exec::s_ExecBase::create_from_address() called. address=0x%x\n", address);
         dprintf("Error: exec::s_ExecBase::create_from_address() is not implemented!\n");
         return NULL;
      }

      s_ExecBase *s_ExecBase::create_from_real(void *real)
      {
         DEBUG(5) dprintf("exec::s_ExecBase::create_from_real() called. real=%p\n", real);

         dprintf("Error: exec::s_ExecBase::create_from_real() is not implemented!\n");
         return NULL;
      }

      int s_ExecBase::mem_access(uint32_t address, uint32_t len, uint8_t *buffer, int read)
      {
         DEBUG(1) dprintf("exec::s_ExecBase::mem_access() called. this=%p, address=0x%x, len=%d, buffer=%p, read=%d\n", this, address, len, buffer, read);

         if(!read)
            return EPERM;

         address -= this->address; // Make relative to start of object

         uint32_t f_ThisTask = 0;

         for(uint32_t addr=address; addr<(address+len); addr++) {
            switch(addr) {
               case 276: case 277: case 278: case 279:
               {
                  emulator *emu = emulator::getEmulator();
                  dos::s_Process *proc = dos::s_Process::create_from_real(emu->current_context->proc);
                  if(proc == NULL)
                     return EFAULT;

                  f_ThisTask = proc->get_address();

                  addr = 279;
                  break;
               }

               default: return EPERM;
            }
         }

         for(uint32_t addr=address; addr<(address+len); addr++) {
            switch(addr) {
               case 276: *buffer = (uint8_t)((f_ThisTask >> 24) & 0xff); break;
               case 277: *buffer = (uint8_t)((f_ThisTask >> 16) & 0xff); break;
               case 278: *buffer = (uint8_t)((f_ThisTask >> 8 ) & 0xff); break;
               case 279: *buffer = (uint8_t)((f_ThisTask      ) & 0xff); break;
               default: return EPERM;
            }
            buffer++;
         }
         return 0;
      }

   } // namespace exec
} // namespace emumiga
