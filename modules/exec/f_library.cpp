/*
modules/exec/f_library.cpp - exec.library library syscalls

Copyright (C) 2007-2011  Magnus Öberg

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

      module::hook_status main::f_OpenLibrary_1(context *ctx)
      {
         //****************************
         //  Function: OpenLibrary
         //    Vector: 92
         //    Offset: -552
         // Arguments: A1.L pointer to library name string
         //            D0.L lowest acceptable version number, 0 means any version
         //   Returns: D0.L pointer to library or 0 if failure
         //****************************

         DEBUG(5) dprintf("exec::main::f_OpenLibrary_1() called. this=%p, ctx=%p\n", this, ctx);

         DEBUG(5) dprintf("  Name of library (A1): 0x%x, Version (D0): 0x%x\n",ctx->cpu.a[1],ctx->cpu.d[0]);
     
         char libname[100];
         if(strlcpy_from_vspace(ctx, libname, ctx->cpu.a[1], sizeof(libname))) {
            dprintf("Error: exec::main::f_OpenLibrary_1(): Library name is not readable\n");
            return HOOK_ERROR;
         }

         DEBUG(5) dprintf("Library name: %s\n", libname);

         ctx->cpu.d[0] = 0;

         module *lib_module = ctx->emu->module_reg.get_module(libname);
         if(lib_module == NULL) {
            DEBUG(5) dprintf("Could not get module\n");
            return HOOK_DONE;
         }

         if(lib_module->get_type() != MODULE_LIBRARY) {
            DEBUG(5) dprintf("Module is of wrong type\n");
            return HOOK_DONE;
         }

         // All library modules must override get_base_address()
         uint32_t base = lib_module->get_base_address();
         if(base == 0xffffffff) {
            dprintf("Error: exec::main::f_OpenLibrary_1(): Library module \"%s\" has no base address\n", libname);
            return HOOK_ERROR;
         }
 
         ctx->cpu.d[0] = base;
         return HOOK_DONE;
      }

      module::hook_status main::f_CloseLibrary_1(context *ctx)
      {
         DEBUG(5) dprintf("exec::main::f_CloseLibrary_1() called. this=%p, ctx=%p\n", this, ctx);

         return HOOK_DONE;
      }
   }
}
