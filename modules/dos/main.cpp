/*
modules/dos/main.cpp - dos.library main class

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

#include "dos.h"

namespace emumiga
{
   namespace dos
   {
      main::main()
      {
         DEBUG(5) dprintf("dos::main::main() called. this=%p\n", this);
      }

      int main::init()
      {
         DEBUG(5) dprintf("dos::main::init() called. this=%p\n", this);

         // Let module init register our callbacks
         int retc = module::init();
         if(retc)
            return retc;

         // Init structure types
         retc = s_DosLibrary::init(this); if(retc) return retc;
         retc = s_FileHandle::init(this); if(retc) return retc;
         retc = s_Process::init(this); if(retc) return retc;

         // Create an dosbase special mapping
         dosbase_mapping = new dosbase(getID());
         if(!dosbase_mapping)
            return ENOMEM;

         retc = dosbase_mapping->init();
         if(retc)
            return retc;

         if(emu->v_space.add_mapping(dosbase_mapping) == 0xffffffff)
            return EFAULT;

         // Extract the virtual address to the dosbase object
         uint32_t dosbase_address = dosbase_mapping->get_base_address();

         // Create the DosLibrary system object
         dosbase_object = new s_DosLibrary();
         dosbase_object->set_address(dosbase_address);

         // Let it override the proper parts of the mapping
         dosbase_mapping->add_object(dosbase_object);

         return 0;
      }

      void main::cleanup()
      {
         DEBUG(5) dprintf("dos::main::cleanup() called. this=%p\n", this);

         // Dispose dosbase special mapping
         dosbase_mapping->dispose();

         // Cleanup structure types
         s_Process::cleanup();
         s_FileHandle::cleanup();
         s_DosLibrary::cleanup();

         // Let module cleanup unregister our callbacks
         module::cleanup();
      }

      module::hook_status main::syscall_hook_1(context *ctx)
      {
         DEBUG(5) dprintf("dos::main::syscall_hook_1() called. this=%p, ctx=%p\n", this, ctx);

         switch(ctx->cpu.emu_func)
         {
            case 8: return f_Write_1(ctx);
            case 9: return f_Input_1(ctx);
            case 10: return f_Output_1(ctx);
            case 25: return f_LoadSeg_1(ctx);
         }

         dprintf("Error: dos::main::syscall_hook_1(): Function %d is not yet emulated.\n", ctx->cpu.emu_func);
         return HOOK_ERROR;
      }

      module::hook_status main::syscall_hook_2(context *ctx)
      {
         DEBUG(5) dprintf("dos::main::syscall_hook_2() called. this=%p, ctx=%p\n", this, ctx);

         switch(ctx->cpu.emu_func)
         {
            case 8: return f_Write_2(ctx);
            case 9: return f_Input_2(ctx);
            case 10: return f_Output_2(ctx);
         }

         dprintf("Error: dos::main::syscall_hook_2(): Function %d is not yet emulated.\n", ctx->cpu.emu_func);
         return HOOK_ERROR;
      }

      module::hook_status main::syscall_hook_3(context *ctx)
      {
         DEBUG(5) dprintf("dos::main::syscall_hook_3() called. this=%p, ctx=%p\n", this, ctx);

         switch(ctx->cpu.emu_func)
         {
            case 8: return f_Write_3(ctx);
            case 9: return f_Input_3(ctx);
            case 10: return f_Output_3(ctx);
         }

         dprintf("Error: dos::main::syscall_hook_3(): Function %d is not yet emulated.\n", ctx->cpu.emu_func);
         return HOOK_ERROR;
      }

      const char *main::get_name()
      {
//         DEBUG(5) dprintf("dos::main::get_name() called. this=%p\n", this);
         return "dos.library";
      }

      module::module_type main::get_type()
      {
         DEBUG(5) dprintf("dos::main::get_type() called. this=%p\n", this);
         return MODULE_LIBRARY;
      }

      uint32_t main::get_base_address()
      {
         DEBUG(5) dprintf("dos::main::get_base_address() called. this=%p\n", this);

         if(dosbase_mapping == NULL)
            return 0xffffffff;

         return dosbase_mapping->get_base_address();
      }
   }
}
