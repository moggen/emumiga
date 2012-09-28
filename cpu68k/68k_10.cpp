/*
68k_10.cpp - M68K emulation. Instructions.

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

#include "68k.h"

namespace emumiga {
   namespace cpu_68k {

/*
 * Function: i_illegal
 */
      cpu68k::returncode i_illegal(context *ctx, uint16_t iw __attribute__((unused)))
      {
	 cpu68k *cpu = &(ctx->cpu);

	 if(DISASM)
	    ASM_INSTR("(ILLEGAL)");

	 return cpu68k::I_ILLEGAL;
      }

/*
 * Function:       i_line_a
 * Bit pattern(s): 1010 xxxx xx
 */
      cpu68k::returncode i_line_a(context *ctx, uint16_t iw __attribute__((unused)))
      {
	 cpu68k *cpu = &(ctx->cpu);

	 if(DISASM)
	    ASM_INSTR("LINE-A");

	 return cpu68k::I_ILLEGAL;
      }

/*
 * Function:       i_line_f
 * Bit pattern(s): 1111 xxxx xx
 */
      cpu68k::returncode i_line_f(context *ctx, uint16_t iw __attribute__((unused)))
      {
	 cpu68k *cpu = &(ctx->cpu);

	 if(DISASM)
	    ASM_INSTR("LINE-F");

	 return cpu68k::I_ILLEGAL;
      }

/*
 * Function:       i_emulator
 * Bit pattern(s): 1111 1101 10
 */
      cpu68k::returncode i_emulator(context *ctx, uint16_t iw __attribute__((unused)))
      {
	 uint16_t tmp16;
	 int rc;

	 cpu68k *cpu = &(ctx->cpu);

	 if(DISASM)
	    ASM_INSTR("EMULATOR");

	 rc = READPC_16(&tmp16);
	 if(rc) return cpu68k::I_BUS_ERROR;
	 cpu->emu_module = tmp16;

	 rc = READPC_16(&tmp16);
	 if(rc) return cpu68k::I_BUS_ERROR;
	 cpu->emu_func = (int16_t)tmp16;

	 if(DISASM)
	    ASM_ARG1("%04X:%04X", cpu->emu_module, cpu->emu_func);

	 return cpu68k::I_HOOK;
      }

   } // namespace cpu_68k
} // namespace emumiga
