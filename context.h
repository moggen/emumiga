/*
context.h - Emulation context class

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

#define READMEM_8(x,y) (ctx->emu->v_space.readmem_8((x),(y)))
#define READMEM_16(x,y) (ctx->emu->v_space.readmem_16((x),(y)))
#define READMEM_32(x,y) (ctx->emu->v_space.readmem_32((x),(y)))
#define WRITEMEM_8(x,y) (ctx->emu->v_space.writemem_8((x),(y)))
#define WRITEMEM_16(x,y) (ctx->emu->v_space.writemem_16((x),(y)))
#define WRITEMEM_32(x,y) (ctx->emu->v_space.writemem_32((x),(y)))

#define READMEM_8_I(x,y) READMEM_8(x,y)
#define READMEM_16_I(x,y) READMEM_16(x,y)
#define READMEM_32_I(x,y) READMEM_32(x,y)
#define WRITEMEM_8_I(x,y) WRITEMEM_8(x,y)
#define WRITEMEM_16_I(x,y) WRITEMEM_16(x,y)
#define WRITEMEM_32_I(x,y) WRITEMEM_32(x,y)

#define READMEM_8_D(x,y) READMEM_8(x,y)
#define READMEM_16_D(x,y) READMEM_16(x,y)
#define READMEM_32_D(x,y) READMEM_32(x,y)
#define WRITEMEM_8_D(x,y) WRITEMEM_8(x,y)
#define WRITEMEM_16_D(x,y) WRITEMEM_16(x,y)
#define WRITEMEM_32_D(x,y) WRITEMEM_32(x,y)

#define CHECK_SP() ;

namespace emumiga {

   class context {
     public:
      context(struct Process *my_proc, struct MsgPort *my_port);
      ~context();
    
      // Static methods
      static context *createContext();

      // Instance attributes
      struct Process *proc;
      struct MsgPort *port;
      emulator *emu;
      uint32_t run_name_address;
      uint32_t run_args_address;
      int run_stack;
      int run_retval;
      message *cached_message;
      module *currentModule;

      cpu68k cpu;

      user_mapping *stack_mapping;

      // Argument variables avalable for
      // syscall connectors
      union {
         APTR _aptr;
         IPTR _iptr;
         BPTR _bptr;
         STRPTR _strptr;
         DOUBLE _double;
         FLOAT _float;
         UQUAD _uquad;
         QUAD _quad;
         ULONG _ulong;
         LONG _long;
         UWORD _uword;
         WORD _word;
         UBYTE _ubyte;
         BYTE _byte;
         BOOL _bool;
         void *_voidptr;
      } arg[10];

      // Instance methods
      struct Process *getProcess();
      struct MsgPort *getPort();
      int init_run(char const *name, char const *args, int stack);
      int getRetVal();
      message *allocMessage();
      void freeMessage(message *msg);

      enum run_status {OK, HOOK, EXIT};
      run_status runBlock();

      uint32_t alloc_mem(uint32_t size);
      void free_mem(uint32_t address);
      uint8_t *mem_to_real(uint32_t address);

      int print_symbol(uint32_t address, char *str, int str_length);
   };

} // namespace emumiga
