/*
cpu68k/cpu68k.cpp - CPU emulation main

Copyright (C) 2009-2011  Magnus Öberg

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

#include "68k.h"

namespace emumiga {

   cpu68k::cpu68k()
   {
      DEBUG(5) dprintf("cpu68k::cpu68k() called. this=%p\n", this);

      for(int i=0; i<8 ; i++) {
	 d[i] = 0;
	 a[i] = 0;
      }
      pc = 0;
      sr = 0;
      flags = FLAG_EMULATE;

      DEBUG(6)
	 flags |= FLAG_DISASM;

      type = TYPE_AUTO;
      disasm_instr[0] = '\0';
      disasm_arg1[0] = '\0';
      disasm_arg2[0] = '\0';
      disasm_start = 0;
      disasm_bytes = 0;
      disasm_ea1 = 0;
      disasm_ea2 = 0;
      emu_module = 0;
      emu_func = 0;
      for(int i=0 ; i<4 ; i++)
	 mmu_work[i] = 0;

      // i_cache = new cache();
      //d_cache = new cache();
   }

   void cpu68k::init()
   {
      DEBUG(5) dprintf("cpu68k::init() called. this=%p\n", this);
   }

   cpu68k::returncode cpu68k::run_block(context *ctx)
   {
      DEBUG(6) dprintf("cpu68k::run_block() called. this=%p, ctx=%p\n", this, ctx);
/*
      uint32_t oldpc;
      uint32_t i;
      mapping *entry;
      symbol *symbol;
      uint32_t diff;
      uint16_t data8;
*/
      cpu68k *cpu = this;

      DEBUG(8) {
	 dprintf("D0:%08x D1:%08x D2:%08x D3:%08x D4:%08x D5:%08x D6:%08x D7:%08x\n",
		 d[0],d[1],d[2],d[3],d[4],d[5],d[6],d[7]);
	 dprintf("A0:%08x A1:%08x A2:%08x A3:%08x A4:%08x A5:%08x A6:%08x A7:%08x\n",
		 a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7]);
	 dprintf("PC:%08x SR:%04x\n", pc, sr);
      }

      // Get next instruction
      uint16_t iw;
      int retc = READMEM_16_I(pc, &iw);
      if(retc)
	 return I_BUS_ERROR;

      // Get table index
      int idx = iw >> 6;

      DEBUG(6) {
	 if(DISASM) {
	    disasm_instr[0] = '\0';
	    disasm_arg1[0] = '\0';
	    disasm_arg2[0] = '\0';
	    disasm_start = pc;
	    disasm_bytes = 2;
	    if(EMULATE) {
	       disasm_ea1 = cpu68k::EA_NONE;
	       disasm_ea2 = cpu68k::EA_NONE;
	    }
	 }
      }

      cpu->pc += 2;

      // Call emulation via lookup-table
      returncode retcode = (cpu_68k::cpu_lut[idx])(ctx, iw);

      DEBUG(6) {
	 if(DISASM) {
	    dprintf("0x%08x:", disasm_start);
	    dprintf("  %-8s", disasm_instr);
	    char tmpstr[200];
	    strcpy(tmpstr, disasm_arg1);
	    if(disasm_arg2[0] != '\0')
	       strcat(tmpstr, ", ");
	    strcat(tmpstr, disasm_arg2);
	    dprintf(" %-20s  ; ", tmpstr);
	 
	    for(int i = 0 ; i < disasm_bytes ; i += 2) {
	       uint16_t data16;
	       READMEM_16_I(disasm_start + i, &data16);
	       dprintf("%04X", data16);
	    }

            char buffer[200];

            dprintf(" PC 0x%x", disasm_start);
            buffer[0] = '\0';
            ctx->print_symbol(disasm_start, buffer, sizeof(buffer));
	    dprintf("%s", buffer);

            if(EMULATE) {
               if(cpu->disasm_ea1 != cpu68k::EA_NONE) {
                  dprintf(" EA1 0x%x", disasm_ea1);
                  buffer[0] = '\0';
                  ctx->print_symbol(disasm_ea1, buffer, sizeof(buffer));
                  dprintf("%s", buffer);
               }
               if(cpu->disasm_ea2 != cpu68k::EA_NONE) {
                  dprintf(" EA2 0x%x", disasm_ea2);
                  buffer[0] = '\0';
                  ctx->print_symbol(disasm_ea2, buffer, sizeof(buffer));
                  dprintf("%s", buffer);
	       }
	    }
	 }
         dprintf("\n");
      }
      return retcode;
   }

   bool cpu68k::is_set(uint32_t f)
   {
      return (flags & f) != 0;
   }

   bool cpu68k::is_type(uint32_t t)
   {
      return (type & t) != 0;
   }

} // namespace emumiga
