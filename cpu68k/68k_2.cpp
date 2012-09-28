/*
68k_2.cpp - M68K emulation. Instructions.

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
 * Function:       i_mul
 * Instructions:   MULU, MULS
 * Bit pattern(s): 1100 xxxx 11
 */
      cpu68k::returncode i_mul(context *ctx, uint16_t iw)
      {
	 uint16_t reg2,mode,reg,sign;
	 uint32_t ea;
	 uint16_t s16,d16;
	 uint32_t s32,d32,r32;
	 int32_t ss32,ds32,rs32;
	 int rc;
	 cpu68k::returncode ea_rc;

	 cpu68k *cpu = &(ctx->cpu);

	 reg2 = ( iw >> 9 ) & 0x7;
	 sign = ( iw >> 8 ) & 0x1;
	 mode = ( iw >> 3 ) & 0x7;
	 reg = iw & 0x7;

	 if(EMULATE)
	    cpu->sr &= 0xfff0;  /* Clear NZVC */

	 if(DISASM) {
	    if(sign==0)
	       ASM_INSTR("MULU");
	    else
	       ASM_INSTR("MULS");
	 }

	 if(mode==0) {
	    if(DISASM)
	       ASM_ARG1("D%d",reg);

	    if(EMULATE)
	       s16 = (uint16_t)(cpu->d[reg] & 0xffff);

	 } else if(mode==7 && reg==4) {
	    rc = READPC_16(&s16);
	    if(rc) return cpu68k::I_BUS_ERROR;

	    if(DISASM) {
	       if(sign==0)
		  ASM_ARG1("#%u",(unsigned)s16);
	       else
		  ASM_ARG1("#%u",(unsigned)s16);
	    }

	 } else {
	    /* EA, all but An (0xffd) */
	    ea_rc = calc_ea(ctx,2,mode,reg,&ea,0xffd,1);
	    if(ea_rc) return ea_rc;

	    if(EMULATE) {
	       rc = READMEM_16_D(ea, &s16);
	       if(rc) return cpu68k::I_BUS_ERROR;
	    }
	 }

	 if(DISASM)
	    ASM_ARG2("D%d",reg2);

	 if(EMULATE) {
	    d16 = (uint16_t)(cpu->d[reg2] & 0xffff);

	    if(sign==0) {
	       s32 = (uint32_t)s16;
	       d32 = (uint32_t)d16;
	       r32 = d32 * s32;
	    } else {
	       ss32 = (int32_t)(int16_t)s16;
	       ds32 = (int32_t)(int16_t)d16;
	       rs32 = ds32 * ss32;
	       r32 = (uint32_t)rs32;
	    }
	    cpu->d[reg2] = r32;

	    if( MSB32(r32) )
	       cpu->sr |= 0x8;  /* N */

	    if( !r32 )
	       cpu->sr |= 0x4;  /* Z */
	 }
	 return cpu68k::I_OK;
      }

/*
 * Function:       i_mull
 * Instructions:   MULU_L, MULS_L
 * Bit pattern(s): 0100 1100 00
 */
      cpu68k::returncode i_mull(context *ctx, uint16_t iw __attribute__((unused)))
      {
	 cpu68k *cpu = &(ctx->cpu);

	 if(DISASM)
	    ASM_INSTR("MULU_L/MULS_L");

	 return cpu68k::I_UNSUPPORTED;
      }

/*
 * Function:       i_div
 * Instructions:   DIVU, DIVS
 * Bit pattern(s): 1000 xxxx 11
 */
      cpu68k::returncode i_div(context *ctx, uint16_t iw)
      {
	 uint16_t reg2,mode,reg,sign;
	 uint32_t ea,oldpc=0;
	 uint16_t s16,r16,re16;
	 uint32_t s32,d32,r32,re32,t32;
	 int32_t ss32,ds32,rs32,res32,ts32;
	 int rc;
	 cpu68k::returncode ea_rc;

	 cpu68k *cpu = &(ctx->cpu);

	 reg2 = ( iw >> 9 ) & 0x7;
	 sign = ( iw >> 8 ) & 0x1;
	 mode = ( iw >> 3 ) & 0x7;
	 reg = iw & 0x7;

	 if(EMULATE) {
	    cpu->sr &= 0xfff0;  /* Clear NZVC */
	    oldpc = cpu->pc;
	 }

	 if(DISASM) {
	    if(sign==0)
	       ASM_INSTR("DIVU");
	    else
	       ASM_INSTR("DIVS");
	 }

	 if(mode==0) {
	    if(DISASM)
	       ASM_ARG1("D%d",reg);

	    if(EMULATE)
	       s16 = (uint16_t)(cpu->d[reg] & 0xffff);

	 } else if(mode==7 && reg==4) {
	    rc = READPC_16(&s16);
	    if(rc) return cpu68k::I_BUS_ERROR;

	    if(DISASM) {
	       if(sign==0)
		  ASM_ARG1("#%u",(unsigned)s16);
	       else
		  ASM_ARG1("#%u",(unsigned)s16);
	    }

	 } else {
	    /* EA, all but An (0xffd) */
	    ea_rc = calc_ea(ctx,2,mode,reg,&ea,0xffd,1);
	    if(ea_rc) return ea_rc;

	    if(EMULATE) {
	       rc = READMEM_16_D(ea, &s16);
	       if(rc) return cpu68k::I_BUS_ERROR;
	    }
	 }

	 if(DISASM)
	    ASM_ARG2("D%d",reg2);

	 if(EMULATE) {
	    if(!s16) {
	       cpu->pc = oldpc;
	       return cpu68k::I_DIV0;
	    }

	    if(sign==0) {

	       s32 = (uint32_t)s16;
	       d32 = cpu->d[reg2];

	       r32 = d32 / s32;
	       re32 = d32 % s32;

	       r16 = 0xffff & r32;
	       re16 = 0xffff & re32;

	       t32 = (uint32_t)r16;

	       if( r32 != t32 ) {
		  cpu->sr |= 0x2;  /* V */
		  return cpu68k::I_OK;
	       }

	    } else {

	       ss32 = (int32_t)(int16_t)s16;
	       ds32 = (int32_t)cpu->d[reg2];

	       rs32 = ds32 / ss32;
	       res32 = ds32 % ss32;

	       r16 = 0xffff & (uint32_t)rs32;
	       re16 = 0xffff & (uint32_t)res32;

	       ts32 = (int32_t)(int16_t)r16;

	       if( rs32 != ts32 ) {
		  cpu->sr |= 0x2;  /* V */
		  return cpu68k::I_OK;
	       }

	    }

	    cpu->d[reg2] = ((uint32_t)re16 << 16) | (uint32_t)r16;

	    if( MSB16(r16) )
	       cpu->sr |= 0x8;  /* N */

	    if( !r16 )
	       cpu->sr |= 0x4;  /* Z */
	 }
	 return cpu68k::I_OK;
      }

/*
 * Function:       i_divl
 * Instructions:   DIVUL, DIVSL, DIVU_L, DIVS_L
 * Bit pattern(s): 0100 1100 01
 */
      cpu68k::returncode i_divl(context *ctx, uint16_t iw __attribute__((unused)))
      {
	 cpu68k *cpu = &(ctx->cpu);

	 if(DISASM)
	    ASM_INSTR("DIVUL/DIVSL/DIVU_L/DIVS_L");

	 return cpu68k::I_UNSUPPORTED;
      }

   } // namespace cpu_68k
} // namespace emumiga
