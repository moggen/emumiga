/*
module.cpp - Module mother class

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

   module::module():
      id(0)
   {
      DEBUG(5) dprintf("module::module() called. this=%p\n", this);

      emu = emulator::getEmulator();
   }

   module::~module()
   {
      DEBUG(5) dprintf("module::~module() called. this=%p\n", this);
   }

   void module::setID(int id)
   {
      DEBUG(5) dprintf("module::setID() called. this=%p, id=%d\n", this, id);
      this->id = id;
   }

   int module::getID()
   {
      DEBUG(5) dprintf("module::getID() called. this=%p\n", this);
      return id;
   }

   int module::init()
   {
      DEBUG(5) dprintf("module::init() called. this=%p\n", this);

      // Call virtual method to get name
      const char *name = this->get_name();

      if(0 == emu->module_reg.registerModule(this, name))
         return EFAULT;

      return 0;
   }

   void module::cleanup()
   {
      DEBUG(5) dprintf("module::cleanup() called. this=%p\n", this);

      emu->module_reg.unRegisterModule(this);
   }

   module::hook_status module::syscall_hook_1(context *ctx)
   {
      DEBUG(5) dprintf("module::syscall_hook_1() called. this=%p, ctx=%p\n", this, ctx);

      dprintf("Error: module::syscall_hook_1(): Base function was called. Function %d.\n", ctx->cpu.emu_func);
      return HOOK_ERROR;
   }

   module::hook_status module::syscall_hook_2(context *ctx)
   {
      DEBUG(5) dprintf("module::syscall_hook_2() called. this=%p, ctx=%p\n", this, ctx);

      dprintf("Error: module::syscall_hook_2(): Base function was called. Function %d.\n", ctx->cpu.emu_func);
      return HOOK_ERROR;
   }

   module::hook_status module::syscall_hook_3(context *ctx)
   {
      DEBUG(5) dprintf("module::syscall_hook_3() called. this=%p, ctx=%p\n", this, ctx);

      dprintf("Error: module::syscall_hook_3(): Base function was called. Function %d.\n", ctx->cpu.emu_func);

      return HOOK_ERROR;
   }

   const char *module::get_name()
   {
//      DEBUG(5) dprintf("module::get_name() called. this=%p\n", this);
      return NULL;
   }

   module::module_type module::get_type()
   {
      DEBUG(5) dprintf("module::get_type() called. this=%p\n", this);
      return MODULE_OTHER;
   }

   uint32_t module::get_base_address()
   {
      DEBUG(5) dprintf("module::get_base_address() called. this=%p\n", this);
      return 0xffffffff;
   }

} // namespace emumiga
