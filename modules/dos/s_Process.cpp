/*
modules/dos/s_Process.cpp - Virtual object for dos struct Process

Copyright (C) 2012  Magnus Öberg

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

#include "dos.h"

namespace emumiga
{
   namespace dos
   {
      object_type *s_Process::type = NULL;
      symbol const s_Process::symbols[] = {
         {0,"Process"},
         {172,"Process:pr_CLI"},
         {0,NULL}
      };

      s_Process::s_Process():
         virtual_object(type, symbols)
      {
         DEBUG(5) dprintf("dos::s_Process::s_Process() called. this=%p\n", this);
      }

      s_Process::~s_Process()
      {
         DEBUG(5) dprintf("dos::s_Process::~s_Process() called. this=%p\n", this);
      }

      // Return value conversion functions
      static virtual_object *cfa_func(uint32_t address)
      {
         return s_Process::create_from_address(address);
      }

      static virtual_object *cfr_func(void *real)
      {
         return s_Process::create_from_real(real);
      }

      int s_Process::init(module *mod)
      {
         DEBUG(5) dprintf("dos::s_Process::init() called. mod=%p\n", mod);

         type = new object_type(mod, id, prio, size, real_size, "Process", cfa_func, cfr_func);

         if(type == NULL)
            return ENOMEM;

         return 0;
      }

      void s_Process::cleanup()
      {
         DEBUG(5) dprintf("dos::s_Process::cleanup() called.\n");

         delete type;
         type = NULL;
      }
     
      s_Process *s_Process::create_from_address(uint32_t address)
      {
         DEBUG(5) dprintf("dos::s_Process::create_from_address() called. address=0x%x\n", address);
         emulator *emu = emulator::getEmulator();
         s_Process *obj = static_cast<s_Process *>(emu->v_space.find_object(address, type));
         if(obj)
            return obj;

         obj = new s_Process();
         if(obj == NULL) {
            dprintf("Error: dos::s_Process::create_from_address(): Could not create object.\n");
            return NULL;
         }

         if(obj->setup_from_address(address)) {
            dprintf("Error: dos::s_Process::create_from_address(): Could not set up object.\n");
            obj->dispose();
            return NULL;
         }

         return obj;
      }

      s_Process *s_Process::create_from_real(void *real)
      {
         DEBUG(5) dprintf("dos::s_Process::create_from_real() called. real=%p\n", real);

         emulator *emu = emulator::getEmulator();
         s_Process *obj = static_cast<s_Process *>(emu->r_space.find_object(real, type));
         if(obj)
            return obj;

         obj = new s_Process();
         if(obj == NULL) {
            dprintf("Error: dos::s_Process::create_from_real(): Could not create object.\n");
            return NULL;
         }

         if(obj->setup_from_real(real)) {
            dprintf("Error: dos::s_Process::create_from_real(): Could not set up object.\n");
            obj->dispose();
            return NULL;
         }

         return obj;
      }

   } // namespace dos
} // namespace emumiga
