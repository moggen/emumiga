/*
68k_7.cpp - M68K emulation. Instructions.

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
 * Function:       i_bop_movep
 * Instructions:   BTST, BCHG, BCLR, BSET, MOVEP
 * Bit pattern(s): 0000 xxx1 xx
 */
      cpu68k::returncode i_bop_movep(context *ctx, uint16_t iw)
      {
	 uint16_t reg2,type,mode,reg;
	 uint32_t ea;
	 uint8_t s8,mask8;
	 uint32_t mask32;
	 int count;
	 int rc;
	 cpu68k::returncode ea_rc;

	 cpu68k *cpu = &(ctx->cpu); 

	 reg2 = ( iw >> 9 ) & 0x7;
	 type = ( iw >> 6 ) & 0x3;
	 mode = ( iw >> 3 ) & 0x7;
	 reg = iw & 0x7;

	 if(mode==1) {			/* MOVEP */

	    if(DISASM)
	       ASM_INSTR("MOVEP");

	    return cpu68k::I_UNSUPPORTED;

	 } else {			/* BTST/BCHG/BCLR/BSET */

	    if(EMULATE)
	       cpu->sr &= 0xfffb;  /* Clear Z */

	    if(DISASM) {
	       switch(type) {
		  case 0:  ASM_INSTR("BTST"); break;
		  case 1:  ASM_INSTR("BCHG"); break;
		  case 2:  ASM_INSTR("BCLR"); break;
		  default: ASM_INSTR("BSET"); break;
	       }
	       ASM_ARG1("D%d",reg2);
	    }

	    if(mode==0) {	/* Dn */
	       if(DISASM)
		  ASM_ARG2("D%d",reg);

	       if(EMULATE) {
		  count = cpu->d[reg2] % 32;
		  mask32 = 1 << count;

		  /* Test */
		  if( !(cpu->d[reg] & mask32) )
		     cpu->sr |= 0x4;  /* Z */

		  if(type==1) {			/* BCHG */
		     cpu->d[reg] ^= mask32;

		  } else if(type==2) {		/* BCLR */
		     cpu->d[reg] &= ~mask32;

		  } else if(type==3) {		/* BSET */
		     cpu->d[reg] |= mask32;

		  }
	       }

	    } else {
	       if(type==0) {
		  /* BTST */
		  /* EA, All except An and #<data> (0xdfd) */
		  ea_rc = calc_ea(ctx,1,mode,reg,&ea,0xdfd,2);
		  if(ea_rc) return ea_rc;

	       } else {
		  /* BCHG/BCLR/BSET */
		  /* EA, Dn + alterable data (0x1fd) */
		  ea_rc = calc_ea(ctx,1,mode,reg,&ea,0x1fd,2);
		  if(ea_rc) return ea_rc;
	       }

	       if(EMULATE) {
		  count = cpu->d[reg2] % 8;
		  mask8 = 1 << count;
		  rc = READMEM_8_D(ea, &s8);
		  if(rc) return cpu68k::I_BUS_ERROR;

		  /* Test */
		  if( !(s8 & mask8) )
		     cpu->sr |= 0x4;  /* Z */

		  if(type==1) {			/* BCHG */
		     s8 ^= mask8;
		     rc = WRITEMEM_8_D(s8,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;

		  } else if(type==2) {		/* BCLR */
		     s8 &= ~mask8;
		     rc = WRITEMEM_8_D(s8,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;

		  } else if(type==3) {		/* BSET */
		     s8 |= mask8;
		     rc = WRITEMEM_8_D(s8,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }
	    }
	 }
	 return cpu68k::I_OK;
      }

/*
 * Function:       i_bopi
 * Instructions:   BTST_I, BCHG_I, BCLR_I, BSET_I
 * Bit pattern(s): 0000 1000 xx
 */
      cpu68k::returncode i_bopi(context *ctx, uint16_t iw)
      {
	 uint16_t type,mode,reg;
	 uint32_t ea;
	 uint8_t s8,mask8,tmp8;
	 uint32_t mask32;
	 int count;
	 int rc;
	 cpu68k::returncode ea_rc;

	 cpu68k *cpu = &(ctx->cpu);

	 type = ( iw >> 6 ) & 0x3;
	 mode = ( iw >> 3 ) & 0x7;
	 reg = iw & 0x7;

	 rc = READPC_8(&tmp8);
	 if(rc) return cpu68k::I_BUS_ERROR;

	 count = (int)tmp8;

	 if(EMULATE)
	    cpu->sr &= 0xfffb;  /* Clear Z */

	 if(DISASM) {
	    switch(type) {
	       case 0:  ASM_INSTR("BTST"); break;
	       case 1:  ASM_INSTR("BCHG"); break;
	       case 2:  ASM_INSTR("BCLR"); break;
	       default: ASM_INSTR("BSET"); break;
	    }
	    ASM_ARG1("#%d,",count);
	 }

	 if(mode==0) {	/* Dn */
	    if(DISASM)
	       ASM_ARG1("D%d",reg);

	    if(EMULATE) {
	       count %= 32;
	       mask32 = 1 << count;

	       /* Test */
	       if( !(cpu->d[reg] & mask32) )
		  cpu->sr |= 0x4;  /* Z */

	       if(type==1) {			/* BCHG */
		  cpu->d[reg] ^= mask32;

	       } else if(type==2) {		/* BCLR */
		  cpu->d[reg] &= ~mask32;

	       } else if(type==3) {		/* BSET */
		  cpu->d[reg] |= mask32;
	       }
	    }

	 } else {
	    if(type==0) {		/* BTST */
	       /* EA, All except An and #<data> (0xdfd) */
	       ea_rc = calc_ea(ctx,1,mode,reg,&ea,0xdfd,2);
	       if(ea_rc) return ea_rc;

	    } else {		/* BCHG/BCLR/BSET */
	       /* EA, Dn + alterable data (0x1fd) */
	       ea_rc = calc_ea(ctx,1,mode,reg,&ea,0x1fd,2);
	       if(ea_rc) return ea_rc;
	    }

	    /*
	    if(CPU_IS_SET(cpu,CPU_FLAG_STRICT) && rc)
	       return rc;
	    */

	    if(EMULATE) {
	       count %= 8;
	       mask8 = 1 << count;
	       rc = READMEM_8_D(ea, &s8);
	       if(rc) return cpu68k::I_BUS_ERROR;

	       /* Test */
	       if( !(s8 & mask8) )
		  cpu->sr |= 0x4;  /* Z */

	       if(type==1) {			/* BCHG */
		  s8 ^= mask8;
		  rc = WRITEMEM_8_D(s8,ea);
		  if(rc) return cpu68k::I_BUS_ERROR;

	       } else if(type==2) {		/* BCLR */
		  s8 &= ~mask8;
		  rc = WRITEMEM_8_D(s8,ea);
		  if(rc) return cpu68k::I_BUS_ERROR;

	       } else if(type==3) {		/* BSET */
		  s8 |= mask8;
		  rc = WRITEMEM_8_D(s8,ea);
		  if(rc) return cpu68k::I_BUS_ERROR;
	       }
	    }
	 }
	 return cpu68k::I_OK;
      }

/*
 * Function:       i_bfop
 * Instructions:   BFTST, BFEXTU, BFCHG, BFEXTS, BFCLR, BFFFO,
 *                 BFSET, BFINS
 * Bit pattern(s): 1110 1xxx 11
 */
      cpu68k::returncode i_bfop(context *ctx, uint16_t iw __attribute__((unused)))
      {
	 cpu68k *cpu = &(ctx->cpu);

	 if(DISASM)
	    ASM_INSTR("BFTST/BFEXTU/BFCHG/BFEXTS/BFCLR/BFFFO/BFSET/BFINS");

	 return cpu68k::I_UNSUPPORTED;
      }

   } // namespace cpu_68k
} // namespace emumiga
