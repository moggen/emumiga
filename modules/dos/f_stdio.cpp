/*
modules/dos/f_stdio.cpp - dos.library emulation of standard IO functions.

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

#include "dos.h"

namespace emumiga
{
   namespace dos
   {
      module::hook_status main::f_Write_1(context *ctx)
      {
         //****************************
         //  Function: Write
         //    Vector: 8
         //    Offset: -48
         // Arguments: D1.L BCPL pointer to file handle
         //            D2.L Pointer to buffer
         //            D3.L Length
         //   Returns: D0.L Bytes written or -1 for error
         //****************************

         DEBUG(4) dprintf("dos::main::f_Write_1() called. this=%p, ctx=%p\n", this, ctx);

         DEBUG(5) dprintf("  File handle (D1): 0x%x\n", ctx->cpu.d[1]);
         DEBUG(5) dprintf("  Buffer pointer (D2): 0x%x\n", ctx->cpu.d[2]);
         DEBUG(5) dprintf("  Length (D3): %d\n", (int)ctx->cpu.d[3]);

         uint32_t filehandle = ctx->cpu.d[1] << 2;

         // Find or setup emulated object
         s_FileHandle *handle = s_FileHandle::create_from_address(filehandle);

         if(handle == NULL) {
            ctx->cpu.d[0] = -1;
            return HOOK_DONE;
         }

         DEBUG(5) dprintf("  File handle (native): %p\n", handle->get_real_address());

         virtual_mapping *buffer_mapping = ctx->emu->v_space.find_mapping(ctx->cpu.d[2]);
         if(buffer_mapping == NULL) {
            ctx->cpu.d[0] = -1;
            return HOOK_DONE;
         }

         if(buffer_mapping->get_type() != virtual_mapping::TYPE_USER) {
            ctx->cpu.d[0] = -1;
            return HOOK_DONE;
         }

         user_mapping *buffer_user = static_cast<user_mapping *>(buffer_mapping);

         uint8_t *real = buffer_user->get_real_buffer(ctx->cpu.d[2], ctx->cpu.d[3]);
         if(real == NULL) {
            ctx->cpu.d[0] = -1;
            return HOOK_DONE;
         }

         DEBUG(5) dprintf("  Buffer pointer (native): %p\n", real);

         // Setup arguments
         ctx->arg[0]._bptr = MKBADDR(handle->get_real_address());
         ctx->arg[1]._aptr = real;
         ctx->arg[2]._long = (LONG)ctx->cpu.d[3];

         return HOOK_SYSCALL;
      }

      module::hook_status main::f_Write_2(context *ctx)
      {
         // Make real syscall
         ctx->arg[0]._long = Write(ctx->arg[0]._bptr, ctx->arg[1]._aptr, ctx->arg[2]._long);

         return HOOK_DONE;
      }

      module::hook_status main::f_Write_3(context *ctx)
      {
         DEBUG(4) dprintf("dos::main::f_Write_3() called. this=%p, ctx=%p\n", this, ctx);

         // Extract return values
         ctx->cpu.d[0] = (uint32_t)ctx->arg[0]._long;

         DEBUG(5) dprintf("  Bytes written (D0): %d\n", (int)ctx->cpu.d[0]);

         return HOOK_DONE;
      }

      module::hook_status main::f_Input_1(context *ctx)
      {
         //****************************
         //  Function: Input
         //    Vector: 9
         //    Offset: -54
         // Arguments: -
         //   Returns: D0.L BCPL pointer to file handle
         //****************************

         DEBUG(4) dprintf("dos::main::f_Input_1() called. this=%p, ctx=%p\n", this, ctx);

         return HOOK_SYSCALL;
      }

      module::hook_status main::f_Input_2(context *ctx)
      {
         // Make real syscall
         ctx->arg[0]._bptr = Input();

         return HOOK_DONE;
      }

      module::hook_status main::f_Input_3(context *ctx)
      {
         DEBUG(4) dprintf("dos::main::f_Input_3() called. this=%p, ctx=%p\n", this, ctx);

         DEBUG(5) dprintf("  File handle (native): %p\n", ctx->arg[0]._bptr);

         ctx->cpu.d[0] = 0;

         // Extract return values
         struct FileHandle *real = (struct FileHandle *)BADDR(ctx->arg[0]._bptr);

         // Find or setup emulated object
         s_FileHandle *handle = s_FileHandle::create_from_real(real);

         if(handle != NULL) {
            ctx->cpu.d[0] = (handle->get_address() >> 2);
         }

         DEBUG(5) dprintf("  File handle (D0): 0x%x\n", ctx->cpu.d[0]);

         return HOOK_DONE;
      }

      module::hook_status main::f_Output_1(context *ctx)
      {
         //****************************
         //  Function: Output
         //    Vector: 10
         //    Offset: -60
         // Arguments: -
         //   Returns: D0.L BCPL pointer to file handle
         //****************************

         DEBUG(4) dprintf("dos::main::f_Output_1() called. this=%p, ctx=%p\n", this, ctx);

         return HOOK_SYSCALL;
      }

      module::hook_status main::f_Output_2(context *ctx)
      {
         // Make real syscall
         ctx->arg[0]._bptr = Output();

         return HOOK_DONE;
      }

      module::hook_status main::f_Output_3(context *ctx)
      {
         DEBUG(4) dprintf("dos::main::f_Output_3() called. this=%p, ctx=%p\n", this, ctx);

         DEBUG(5) dprintf("  File handle (native): %p\n", ctx->arg[0]._bptr);

         ctx->cpu.d[0] = 0;

         // Extract return values
         struct FileHandle *real = (struct FileHandle *)BADDR(ctx->arg[0]._bptr);

         // Find or setup emulated object
         s_FileHandle *handle = s_FileHandle::create_from_real(real);

         if(handle != NULL) {
            ctx->cpu.d[0] = (handle->get_address() >> 2);
         }

         DEBUG(5) dprintf("  File handle (D0): 0x%x\n", ctx->cpu.d[0]);

         return HOOK_DONE;
      }
   }
}
