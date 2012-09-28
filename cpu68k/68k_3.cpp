/*
68k_3.cpp - M68K emulation. Instructions.

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
 * Function:       i_and_abcd_exg
 * Instructions:   AND, ABCD, EXG
 * Bit pattern(s): 1100 xxxx 00
 *                 1100 xxxx 01
 *                 1100 xxxx 10
 */
      cpu68k::returncode i_and_abcd_exg(context *ctx, uint16_t iw)
      {
	 uint16_t reg2,opmode,mode,reg;
	 uint32_t ea;
	 uint8_t s8,d8,r8;
	 uint16_t s16,d16,r16;
	 uint32_t s32,d32,r32;
	 int rc;
	 cpu68k::returncode ea_rc;
	 uint32_t tmp;

	 cpu68k *cpu = &(ctx->cpu);

	 reg2 = ( iw >> 9 ) & 0x7;
	 opmode = ( iw >> 6 ) & 0x7;
	 mode = ( iw >> 3 ) & 0x7;
	 reg = iw & 0x7;

	 if(opmode==4 && mode<2) {
	    /* ABCD */

	    if(DISASM)
	       ASM_INSTR("ABCD");

	    return cpu68k::I_UNSUPPORTED;

	 } else if( (opmode==5 && mode<2) || (opmode==6 && mode==1) ) {
	    /* EXG */

	    /* reg = y-reg, reg2 = x-reg */

	    if(DISASM)
	       ASM_INSTR("EXG");

	    if(opmode==5 && mode==0) {		/* Dx <-> Dy */
	       if(EMULATE) {
		  tmp = cpu->d[reg2];
		  cpu->d[reg2] = cpu->d[reg];
		  cpu->d[reg] = tmp;
	       }

	       if(DISASM) {
		  ASM_ARG1("D%d",reg2);
		  ASM_ARG2("D%d",reg);
	       }

	    } else if(opmode==5 && mode==1) {	/* Ax <-> Ay */
	       if(EMULATE) {
		  tmp = cpu->a[reg2];
		  cpu->a[reg2] = cpu->a[reg];
		  cpu->a[reg] = tmp;

		  /* Changed SP? */
		  if(reg==7 || reg2==7) {
		     CHECK_SP();
		  }
	       }

	       if(DISASM) {
		  ASM_ARG1("A%d",reg2);
		  ASM_ARG2("A%d",reg);
	       }

	    } else {				/* Dx <-> Ay */
	       if(EMULATE) {
		  tmp = cpu->d[reg2];
		  cpu->d[reg2] = cpu->a[reg];
		  cpu->a[reg] = tmp;

		  /* Changed SP? */
		  if(reg==7) {
		     CHECK_SP();
		  }
	       }
	       if(DISASM) {
		  ASM_ARG1("D%d",reg2);
		  ASM_ARG2("A%d",reg);
	       }
	    }

	 } else {
	    /* AND */

	    if(EMULATE)
	       cpu->sr &= 0xfff0;  /* Clear NZVC */

	    if(DISASM)
	       ASM_INSTR("AND");

	    if(opmode==0 || opmode==4) {
	       /* Byte */
	       if(DISASM)
		  ASM_INSTR(".B");

	       if(opmode==0) {	/* <ea> & Dn -> Dn */
		  if(mode==0) {
		     if(EMULATE)
			s8 = (uint8_t)(cpu->d[reg] & 0xff);

		     if(DISASM)
			ASM_ARG1("D%d",reg);

		  } else if(mode==7 && reg==4) {
		     rc = READPC_8(&s8);
		     if(rc) return cpu68k::I_BUS_ERROR;

		     if(DISASM)
			ASM_ARG1("#%d",(unsigned)s8);

		  } else {
		     /* EA, all but An (0xffd) */
		     ea_rc = calc_ea(ctx,1,mode,reg,&ea,0xffd,1);
		     if(ea_rc) return ea_rc;

		     if(EMULATE) {
			rc = READMEM_8_D(ea, &s8);
			if(rc) return cpu68k::I_BUS_ERROR;
		     }
		  }

		  if(DISASM)
		     ASM_ARG2("D%d",reg2);

		  if(EMULATE) {
		     d8 = (uint8_t)(cpu->d[reg2] & 0xff);
		     r8 = s8 & d8;
		     cpu->d[reg2] &= 0xffffff00;
		     cpu->d[reg2] |= (uint32_t)r8;
		  }

	       } else {	/* Dn & <ea> -> <ea> */
		  if(DISASM)
		     ASM_ARG1("D%d",reg2);

		  /* EA, only alterable memory (0x1fc) */
		  ea_rc = calc_ea(ctx,1,mode,reg,&ea,0x1fc,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     s8 = (uint8_t)(cpu->d[reg2] & 0xff);
		     rc = READMEM_8_D(ea, &d8);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     r8 = s8 & d8;
		     rc = WRITEMEM_8_D(r8,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  if( MSB8(r8) )
		     cpu->sr |= 0x8;  /* N */

		  if( !r8 )
		     cpu->sr |= 0x4;  /* Z */
	       }

	    } else if(opmode==1 || opmode==5) {
	       /* Word */
	       if(DISASM)
		  ASM_INSTR(".W");

	       if(opmode==1) {	/* <ea> & Dn -> Dn */
		  if(mode==0) {
		     if(EMULATE)
			s16 = (uint16_t)(cpu->d[reg] & 0xffff);

		     if(DISASM)
			ASM_ARG1("D%d",reg);

		  } else if(mode==7 && reg==4) {
		     rc = READPC_16(&s16);
		     if(rc) return cpu68k::I_BUS_ERROR;

		     if(DISASM)
			ASM_ARG1("#%d",(unsigned)s16);

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
		     r16 = s16 & d16;
		     cpu->d[reg2] &= 0xffff0000;
		     cpu->d[reg2] |= (uint32_t)r16;
		  }

	       } else {	/* Dn & <ea> -> <ea> */
		  if(DISASM)
		     ASM_ARG1("D%d,",reg2);

		  /* EA, only alterable memory (0x1fc) */
		  ea_rc = calc_ea(ctx,2,mode,reg,&ea,0x1fc,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     s16 = (uint16_t)(cpu->d[reg2] & 0xffff);
		     rc = READMEM_16_D(ea, &d16);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     r16 = s16 & d16;
		     rc = WRITEMEM_16_D(r16,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  if( MSB16(r16) )
		     cpu->sr |= 0x8;  /* N */

		  if( !r16 )
		     cpu->sr |= 0x4;  /* Z */
	       }

	    } else {
	       /* Long */
	       if(DISASM)
		  ASM_INSTR(".L");

	       if(opmode==2) {	/* <ea> & Dn -> Dn */
		  if(mode==0) {
		     if(EMULATE)
			s32 = cpu->d[reg];

		     if(DISASM)
			ASM_ARG1("D%d",reg);

		  } else if(mode==7 && reg==4) {
		     rc = READPC_32(&s32);
		     if(rc) return cpu68k::I_BUS_ERROR;

		     if(DISASM)
			ASM_ARG1("#%u",(unsigned)s32);

		  } else {
		     /* EA, all but An (0xffd) */
		     ea_rc = calc_ea(ctx,4,mode,reg,&ea,0xffd,1);
		     if(ea_rc) return ea_rc;

		     if(EMULATE) {
			rc = READMEM_32_D(ea, &s32);
			if(rc) return cpu68k::I_BUS_ERROR;
		     }
		  }

		  if(DISASM)
		     ASM_ARG2("D%d",reg2);

		  if(EMULATE) {
		     d32 = cpu->d[reg2];
		     r32 = s32 & d32;
		     cpu->d[reg2] = r32;
		  }

	       } else {	/* Dn & <ea> -> <ea> */
		  if(DISASM)
		     ASM_ARG1("D%d",reg2);

		  /* EA, only alterable memory (0x1fc) */
		  ea_rc = calc_ea(ctx,4,mode,reg,&ea,0x1fc,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     s32 = cpu->d[reg2];
		     rc = READMEM_32_D(ea, &d32);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     r32 = s32 & d32;
		     rc = WRITEMEM_32_D(r32,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  if( MSB32(r32) )
		     cpu->sr |= 0x8;  /* N */

		  if( !r32 )
		     cpu->sr |= 0x4;  /* Z */
	       }
	    }
	 }
	 return cpu68k::I_OK;
      }


/*
 * Function:       i_andi
 * Instructions:   ANDI, ANDI_CCR, ANDI_SR
 * Bit pattern(s): 0000 0010 00
 *                 0000 0010 01
 *                 0000 0010 10
 */
      cpu68k::returncode i_andi(context *ctx, uint16_t iw)
      {
	 uint16_t size,mode,reg;
	 uint32_t ea;
	 uint8_t s8,d8,r8;
	 uint16_t s16,d16,r16;
	 uint32_t s32,d32,r32;
	 int rc;
	 cpu68k::returncode ea_rc;

	 cpu68k *cpu = &(ctx->cpu);

	 size = ( iw >> 6 ) & 0x3;
	 mode = ( iw >> 3 ) & 0x7;
	 reg = iw & 0x7;

	 if(DISASM)
	    ASM_INSTR("ANDI");

	 if(size==0 && mode==7 && reg==4) {
	    /* ANDI_CCR */
	    if(DISASM)
	       ASM_ARG2("CCR");

	    rc = READPC_16(&s16);
	    if(rc) return cpu68k::I_BUS_ERROR;

	    if(DISASM)
	       ASM_ARG1("#%u",(unsigned)s16);

	    if(EMULATE)
	       cpu->sr &= (s16 | 0xffe0);

	    return cpu68k::I_OK;

	 } else if(size==1 && mode==7 && reg==4) {
	    /* ANDI_SR */
	    if(DISASM)
	       ASM_ARG2("SR");

	    rc = READPC_16(&s16);
	    if(rc) return cpu68k::I_BUS_ERROR;

	    if(DISASM)
	       ASM_ARG1("#%u",(unsigned)s16);

	    if(EMULATE)
	       return cpu68k::I_PRIVILEGE;

	    return cpu68k::I_OK;

	 } else {
	    /* ANDI */
	    if(EMULATE)
	       cpu->sr &= 0xfff0;  /* Clear NZVC */

	    if(size==0) {
	       /* Byte */
	       if(DISASM)
		  ASM_INSTR(".B");

	       rc = READPC_8(&s8);
	       if(rc) return cpu68k::I_BUS_ERROR;

	       if(DISASM)
		  ASM_ARG1("#%u",(unsigned)s8);

	       if(mode==0) {
		  if(DISASM)
		     ASM_ARG2("D%d",reg);

		  if(EMULATE) {
		     d8 = (uint8_t)(cpu->d[reg] & 0xff);
		     r8 = s8 & d8;
		     cpu->d[reg] &= 0xffffff00;
		     cpu->d[reg] |= (uint32_t)r8;
		  }

	       } else {
		  /* EA, Dn + alterable memory (0x1fd) */
		  ea_rc = calc_ea(ctx,1,mode,reg,&ea,0x1fd,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     rc = READMEM_8_D(ea, &d8);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     r8 = s8 & d8;
		     rc = WRITEMEM_8_D(r8,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  if( MSB8(r8) )
		     cpu->sr |= 0x8;  /* N */

		  if( !r8 )
		     cpu->sr |= 0x4;  /* Z */
	       }

	    } else if(size==1) {
	       /* Word */
	       if(DISASM)
		  ASM_INSTR(".W");

	       rc = READPC_16(&s16);
	       if(rc) return cpu68k::I_BUS_ERROR;

	       if(DISASM)
		  ASM_ARG1("#%u",(unsigned)s16);

	       if(mode==0) {
		  if(DISASM)
		     ASM_ARG2("D%d",reg);

		  if(EMULATE) {
		     d16 = (uint16_t)(cpu->d[reg] & 0xffff);
		     r16 = s16 & d16;
		     cpu->d[reg] &= 0xffff0000;
		     cpu->d[reg] |= (uint32_t)r16;
		  }

	       } else {
		  /* EA, Dn + alterable memory (0x1fd) */
		  ea_rc = calc_ea(ctx,2,mode,reg,&ea,0x1fd,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     rc = READMEM_16_D(ea, &d16);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     r16 = s16 & d16;
		     rc = WRITEMEM_16_D(r16,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  if( MSB16(r16) )
		     cpu->sr |= 0x8;  /* N */

		  if( !r16 )
		     cpu->sr |= 0x4;  /* Z */
	       }

	    } else {
	       /* Long */
	       if(DISASM)
		  ASM_INSTR(".L");

	       rc = READPC_32(&s32);
	       if(rc) return cpu68k::I_BUS_ERROR;

	       if(DISASM)
		  ASM_ARG1("#%u",(unsigned)s32);

	       if(mode==0) {
		  if(DISASM)
		     ASM_ARG2("D%d",reg);

		  if(EMULATE) {
		     d32 = cpu->d[reg];
		     r32 = s32 & d32;
		     cpu->d[reg] = r32;
		  }

	       } else {
		  /* EA, Dn + alterable memory (0x1fd) */
		  ea_rc = calc_ea(ctx,4,mode,reg,&ea,0x1fd,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     rc = READMEM_32_D(ea, &d32);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     r32 = s32 & d32;
		     rc = WRITEMEM_32_D(r32,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  if( MSB32(r32) )
		     cpu->sr |= 0x8;  /* N */

		  if( !r32 )
		     cpu->sr |= 0x4;  /* Z */
	       }
	    }
	 }
	 return cpu68k::I_OK;
      }

/*
 * Function:       i_or_sbcd_pack
 * Instructions:   OR, SBCD, PACK, UNPK
 * Bit pattern(s): 1000 xxxx 00
 *                 1000 xxxx 01
 *                 1000 xxxx 10
 */
      cpu68k::returncode i_or_sbcd_pack(context *ctx, uint16_t iw)
      {
	 uint16_t reg2,opmode,mode,reg;
	 uint32_t ea;
	 uint8_t s8,d8,r8;
	 uint16_t s16,d16,r16;
	 uint32_t s32,d32,r32;
	 int rc;
	 cpu68k::returncode ea_rc;

	 cpu68k *cpu = &(ctx->cpu);

	 reg2 = ( iw >> 9 ) & 0x7;
	 opmode = ( iw >> 6 ) & 0x7;
	 mode = ( iw >> 3 ) & 0x7;
	 reg = iw & 0x7;

	 if(opmode==4 && mode<2) {
	    /* SBCD */
	    if(DISASM)
	       ASM_INSTR("SBCD");

	    return cpu68k::I_UNSUPPORTED;

	 } else if(opmode==5 && mode<2) {
	    /* PACK */
	    if(DISASM)
	       ASM_INSTR("PACK");

	    return cpu68k::I_UNSUPPORTED;

	 } else if(opmode==6 && mode<2) {
	    /* UNPK */
	    if(DISASM)
	       ASM_INSTR("UNPK");

	    return cpu68k::I_UNSUPPORTED;

	 } else {
	    /* OR */
	    if(DISASM)
	       ASM_INSTR("OR");

	    if(EMULATE)
	       cpu->sr &= 0xfff0;  /* Clear NZVC */

	    if(opmode==0 || opmode==4) {
	       /* Byte */
	       if(DISASM)
		  ASM_INSTR(".B");

	       if(opmode==0) {	/* <ea> | Dn -> Dn */
		  if(mode==0) {
		     if(EMULATE)
			s8 = (uint8_t)(cpu->d[reg] & 0xff);

		     if(DISASM)
			ASM_ARG1("D%d",reg);

		  } else if(mode==7 && reg==4) {
		     rc = READPC_8(&s8);
		     if(rc) return cpu68k::I_BUS_ERROR;

		     if(DISASM)
			ASM_ARG1("#%d",(unsigned)s8);

		  } else {
		     /* EA, all but An (0xffd) */
		     ea_rc = calc_ea(ctx,1,mode,reg,&ea,0xffd,1);
		     if(ea_rc) return ea_rc;

		     if(EMULATE) {
			rc = READMEM_8_D(ea, &s8);
			if(rc) return cpu68k::I_BUS_ERROR;
		     }
		  }

		  if(DISASM)
		     ASM_ARG2("D%d",reg2);

		  if(EMULATE) {
		     d8 = (uint8_t)(cpu->d[reg2] & 0xff);
		     r8 = s8 | d8;
		     cpu->d[reg2] &= 0xffffff00;
		     cpu->d[reg2] |= (uint32_t)r8;
		  }

	       } else {	/* Dn | <ea> -> <ea> */
		  if(DISASM)
		     ASM_ARG1("D%d",reg2);

		  /* EA, only alterable memory (0x1fc) */
		  ea_rc = calc_ea(ctx,1,mode,reg,&ea,0x1fc,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     s8 = (uint8_t)(cpu->d[reg2] & 0xff);
		     rc = READMEM_8_D(ea, &d8);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     r8 = s8 | d8;
		     rc = WRITEMEM_8_D(r8,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  if( MSB8(r8) )
		     cpu->sr |= 0x8;  /* N */

		  if( !r8 )
		     cpu->sr |= 0x4;  /* Z */
	       }

	    } else if(opmode==1 || opmode==5) {
	       /* Word */
	       if(DISASM)
		  ASM_INSTR(".W");

	       if(opmode==1) {	/* <ea> | Dn -> Dn */
		  if(mode==0) {
		     if(EMULATE)
			s16 = (uint16_t)(cpu->d[reg] & 0xffff);

		     if(DISASM)
			ASM_ARG1("D%d",reg);

		  } else if(mode==7 && reg==4) {
		     rc = READPC_16(&s16);
		     if(rc) return cpu68k::I_BUS_ERROR;

		     if(DISASM)
			ASM_ARG1("#%u",(unsigned)s16);

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
		     r16 = s16 | d16;
		     cpu->d[reg2] &= 0xffff0000;
		     cpu->d[reg2] |= (uint32_t)r16;
		  }

	       } else {	/* Dn | <ea> -> <ea> */
		  if(DISASM)
		     ASM_ARG1("D%d",reg2);

		  /* EA, only alterable memory (0x1fc) */
		  ea_rc = calc_ea(ctx,2,mode,reg,&ea,0x1fc,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     s16 = (uint16_t)(cpu->d[reg2] & 0xffff);
		     rc = READMEM_16_D(ea, &d16);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     r16 = s16 | d16;
		     rc = WRITEMEM_16_D(r16,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  if( MSB16(r16) )
		     cpu->sr |= 0x8;  /* N */

		  if( !r16 )
		     cpu->sr |= 0x4;  /* Z */
	       }

	    } else {
	       /* Long */
	       if(DISASM)
		  ASM_INSTR(".L");

	       if(opmode==2) {	/* <ea> | Dn -> Dn */
		  if(mode==0) {
		     if(EMULATE)
			s32 = cpu->d[reg];

		     if(DISASM)
			ASM_ARG1("D%d",reg);

		  } else if(mode==7 && reg==4) {
		     rc = READPC_32(&s32);
		     if(rc) return cpu68k::I_BUS_ERROR;

		     if(DISASM)
			ASM_ARG1("#%u",(unsigned)s32);

		  } else {
		     /* EA, all but An (0xffd) */
		     ea_rc = calc_ea(ctx,4,mode,reg,&ea,0xffd,1);
		     if(ea_rc) return ea_rc;

		     if(EMULATE) {
			rc = READMEM_32_D(ea, &s32);
			if(rc) return cpu68k::I_BUS_ERROR;
		     }
		  }

		  if(DISASM)
		     ASM_ARG2("D%d",reg2);

		  if(EMULATE) {
		     d32 = cpu->d[reg2];
		     r32 = s32 | d32;
		     cpu->d[reg2] = r32;
		  }

	       } else {	/* Dn | <ea> -> <ea> */
		  if(DISASM)
		     ASM_ARG1("D%d,",reg2);

		  /* EA, only alterable memory (0x1fc) */
		  ea_rc = calc_ea(ctx,4,mode,reg,&ea,0x1fc,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     s32 = cpu->d[reg2];
		     rc = READMEM_32_D(ea, &d32);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     r32 = s32 | d32;
		     rc = WRITEMEM_32_D(r32,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  if( MSB32(r32) )
		     cpu->sr |= 0x8;  /* N */

		  if( !r32 )
		     cpu->sr |= 0x4;  /* Z */
	       }
	    }
	 }
	 return cpu68k::I_OK;
      }

/*
 * Function:       i_ori
 * Instructions:   ORI, ORI_CCR, ORI_SR
 * Bit pattern(s): 0000 0000 00
 *                 0000 0000 01
 *                 0000 0000 10
 */
      cpu68k::returncode i_ori(context *ctx, uint16_t iw)
      {
	 uint16_t size,mode,reg;
	 uint32_t ea;
	 uint8_t s8,d8,r8;
	 uint16_t s16,d16,r16;
	 uint32_t s32,d32,r32;
	 int rc;
	 cpu68k::returncode ea_rc;

	 cpu68k *cpu = &(ctx->cpu);

	 size = ( iw >> 6 ) & 0x3;
	 mode = ( iw >> 3 ) & 0x7;
	 reg = iw & 0x7;

	 if(DISASM)
	    ASM_INSTR("ORI");

	 if(size==0 && mode==7 && reg==4) {
	    /* ORI_CCR */
	    if(DISASM)
	       ASM_ARG2("CCR");

	    rc = READPC_16(&s16);
	    if(rc) return cpu68k::I_BUS_ERROR;

	    if(DISASM)
	       ASM_ARG1("#%u",(unsigned)s16);

	    if(EMULATE)
	       cpu->sr |= (s16 & 0x1f);

	    return cpu68k::I_OK;

	 } else if(size==1 && mode==7 && reg==4) {
	    /* ORI_SR */
	    if(DISASM)
	       ASM_ARG2("SR");

	    rc = READPC_16(&s16);
	    if(rc) return cpu68k::I_BUS_ERROR;

	    if(DISASM)
	       ASM_ARG1("#%u",(unsigned)s16);

	    if(EMULATE)
	       return cpu68k::I_PRIVILEGE;

	    return cpu68k::I_OK;

	 } else {
	    /* ORI */
	    if(DISASM)
	       ASM_INSTR("ORI");

	    if(EMULATE)
	       cpu->sr &= 0xfff0;  /* Clear NZVC */

	    if(size==0) {
	       /* Byte */
	       if(DISASM)
		  ASM_INSTR(".B");

	       rc = READPC_8(&s8);
	       if(rc) return cpu68k::I_BUS_ERROR;

	       if(DISASM)
		  ASM_ARG1("#%u",(unsigned)s8);

	       if(mode==0) {
		  if(DISASM)
		     ASM_ARG2("D%d",reg);

		  if(EMULATE) {
		     d8 = (uint8_t)(cpu->d[reg] & 0xff);
		     r8 = s8 | d8;
		     cpu->d[reg] &= 0xffffff00;
		     cpu->d[reg] |= (uint32_t)r8;
		  }

	       } else {
		  /* EA, Dn + alterable memory (0x1fd) */
		  ea_rc = calc_ea(ctx,1,mode,reg,&ea,0x1fd,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     rc = READMEM_8_D(ea, &d8);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     r8 = s8 | d8;
		     rc = WRITEMEM_8_D(r8,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  if( MSB8(r8) )
		     cpu->sr |= 0x8;  /* N */

		  if( !r8 )
		     cpu->sr |= 0x4;  /* Z */
	       }

	    } else if(size==1) {
	       /* Word */
	       if(DISASM)
		  ASM_INSTR(".W");

	       rc = READPC_16(&s16);
	       if(rc) return cpu68k::I_BUS_ERROR;

	       if(DISASM)
		  ASM_ARG1("#%u",(unsigned)s16);

	       if(mode==0) {
		  if(DISASM)
		     ASM_ARG2("D%d",reg);

		  if(EMULATE) {
		     d16 = (uint16_t)(cpu->d[reg] & 0xffff);
		     r16 = s16 | d16;
		     cpu->d[reg] &= 0xffff0000;
		     cpu->d[reg] |= (uint32_t)r16;
		  }

	       } else {
		  /* EA, Dn + alterable memory (0x1fd) */
		  ea_rc = calc_ea(ctx,2,mode,reg,&ea,0x1fd,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     rc = READMEM_16_D(ea, &d16);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     r16 = s16 | d16;
		     rc = WRITEMEM_16_D(r16,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  if( MSB16(r16) )
		     cpu->sr |= 0x8;  /* N */

		  if( !r16 )
		     cpu->sr |= 0x4;  /* Z */
	       }

	    } else {
	       /* Long */
	       if(DISASM)
		  ASM_INSTR(".L");

	       rc = READPC_32(&s32);
	       if(rc) return cpu68k::I_BUS_ERROR;

	       if(DISASM)
		  ASM_ARG1("#%u",(unsigned)s32);

	       if(mode==0) {
		  if(DISASM)
		     ASM_ARG2("D%d",reg);

		  if(EMULATE) {
		     d32 = cpu->d[reg];
		     r32 = s32 | d32;
		     cpu->d[reg] = r32;
		  }

	       } else {
		  /* EA, Dn + alterable memory (0x1fd) */
		  ea_rc = calc_ea(ctx,4,mode,reg,&ea,0x1fd,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     rc = READMEM_32_D(ea, &d32);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     r32 = s32 | d32;
		     rc = WRITEMEM_32_D(r32,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  if( MSB32(r32) )
		     cpu->sr |= 0x8;  /* N */

		  if( !r32 )
		     cpu->sr |= 0x4;  /* Z */
	       }
	    }
	 }
	 return cpu68k::I_OK;
      }

/*
 * Function:       i_eor_cmpm
 * Instructions:   EOR, CMPM
 * Bit pattern(s): 1011 xxx1 00
 *                 1011 xxx1 01
 *                 1011 xxx1 10
 */
      cpu68k::returncode i_eor_cmpm(context *ctx, uint16_t iw)
      {
	 uint16_t reg2,opmode,mode,reg;
	 uint32_t ea,ea2;
	 uint8_t s8,d8,r8;
	 uint16_t s16,d16,r16;
	 uint32_t s32,d32,r32;
	 int rc;
	 cpu68k::returncode ea_rc;

	 cpu68k *cpu = &(ctx->cpu);

	 reg2 = ( iw >> 9 ) & 0x7;
	 opmode = ( iw >> 6 ) & 0x3;
	 mode = ( iw >> 3 ) & 0x7;
	 reg = iw & 0x7;

	 if(mode==1) {
	    /* CMPM */

	    /* reg = y-reg, reg2 = x-reg */

	    if(DISASM)
	       ASM_INSTR("CMPM");

	    if(EMULATE)
	       cpu->sr &= 0xfff0;  /* Clear NZVC */

	    if(opmode==4) {
	       /* Byte */
	       if(DISASM)
		  ASM_INSTR(".B");

	       /* EA, only (An)+ (0x8) */
	       ea_rc = calc_ea(ctx,1,3,reg,&ea,0x8,1);
	       if(ea_rc) return ea_rc;

	       if(EMULATE) {
		  rc = READMEM_8_D(ea, &s8);
		  if(rc) return cpu68k::I_BUS_ERROR;
	       }

	       /* EA, only (An)+ (0x8) */
	       ea_rc = calc_ea(ctx,1,3,reg2,&ea2,0x8,2);
	       if(ea_rc) return ea_rc;

	       if(EMULATE) {
		  rc = READMEM_8_D(ea2, &d8);
		  if(rc) return cpu68k::I_BUS_ERROR;

		  r8 = d8 - s8;

		  if( ( !MSB8(s8) &&  MSB8(d8) && !MSB8(r8) ) ||
		      (  MSB8(s8) && !MSB8(d8) &&  MSB8(r8) ) )
		     cpu->sr |= 0x2;  /* V */

		  if( (  MSB8(s8) && !MSB8(d8) ) ||
		      (  MSB8(r8) && !MSB8(d8) ) ||
		      (  MSB8(r8) &&  MSB8(s8) ) )
		     cpu->sr |= 0x1;  /* C */

		  if( MSB8(r8) )
		     cpu->sr |= 0x8;  /* N */

		  if( !r8 )
		     cpu->sr |= 0x4;  /* Z */
	       }

	    } else if(opmode==5) {
	       /* Word */
	       if(DISASM)
		  ASM_INSTR(".W");

	       /* EA, only (An)+ (0x8) */
	       ea_rc = calc_ea(ctx,2,3,reg,&ea,0x8,1);
	       if(ea_rc) return ea_rc;

	       if(EMULATE) {
		  rc = READMEM_16_D(ea, &s16);
		  if(rc) return cpu68k::I_BUS_ERROR;
	       }

	       /* EA, only (An)+ (0x8) */
	       ea_rc = calc_ea(ctx,2,3,reg2,&ea2,0x8,2);
	       if(ea_rc) return ea_rc;

	       if(EMULATE) {
		  rc = READMEM_16_D(ea2, &d16);
		  if(rc) return cpu68k::I_BUS_ERROR;

		  r16 = d16 - s16;

		  if( ( !MSB16(s16) &&  MSB16(d16) && !MSB16(r16) ) ||
		      (  MSB16(s16) && !MSB16(d16) &&  MSB16(r16) ) )
		     cpu->sr |= 0x2;  /* V */

		  if( (  MSB16(s16) && !MSB16(d16) ) ||
		      (  MSB16(r16) && !MSB16(d16) ) ||
		      (  MSB16(r16) &&  MSB16(s16) ) )
		     cpu->sr |= 0x1;  /* C */

		  if( MSB16(r16) )
		     cpu->sr |= 0x8;  /* N */

		  if( !r16 )
		     cpu->sr |= 0x4;  /* Z */
	       }

	    } else {
	       /* Long */
	       if(DISASM)
		  ASM_INSTR(".L");

	       /* EA, only (An)+ (0x8) */
	       ea_rc = calc_ea(ctx,4,3,reg,&ea,0x8,1);
	       if(ea_rc) return ea_rc;

	       if(EMULATE) {
		  rc = READMEM_32_D(ea, &s32);
		  if(rc) return cpu68k::I_BUS_ERROR;
	       }

	       /* EA, only (An)+ (0x8) */
	       ea_rc = calc_ea(ctx,4,3,reg2,&ea2,0x8,2);
	       if(ea_rc) return ea_rc;

	       if(EMULATE) {
		  rc = READMEM_32_D(ea2, &d32);
		  if(rc) return cpu68k::I_BUS_ERROR;

		  r32 = d32 - s32;

		  if( ( !MSB32(s32) &&  MSB32(d32) && !MSB32(r32) ) ||
		      ( !MSB32(s32) && !MSB32(d32) &&  MSB32(r32) ) )
		     cpu->sr |= 0x2;  /* V */

		  if( (  MSB32(s32) && !MSB32(d32) ) ||
		      (  MSB32(r32) && !MSB32(d32) ) ||
		      (  MSB32(r32) &&  MSB32(s32) ) )
		     cpu->sr |= 0x1;  /* C */

		  if( MSB32(r32) )
		     cpu->sr |= 0x8;  /* N */

		  if( !r32 )
		     cpu->sr |= 0x4;  /* Z */
	       }
	    }

	 } else {
	    /* EOR */
	    if(DISASM)
	       ASM_INSTR("EOR");

	    if(EMULATE)
	       cpu->sr &= 0xfff0;  /* Clear NZVC */

	    if(opmode==4) {
	       /* Byte */

	       /* Dn ^ <ea> -> <ea> */

	       if(DISASM) {
		  ASM_INSTR(".B");
		  ASM_ARG1("D%d",reg2);
	       }

	       if(mode==0) {
		  if(DISASM)
		     ASM_ARG2("D%d",reg);

		  if(EMULATE) {
		     s8 = (uint8_t)(cpu->d[reg2] & 0xff);
		     d8 = (uint8_t)(cpu->d[reg] & 0xff);
		     r8 = s8 ^ d8;
		     cpu->d[reg] &= 0xffffff00;
		     cpu->d[reg] |= (uint32_t)r8;
		  }

	       } else {
		  /* EA, only alterable memory (0x1fc) */
		  ea_rc = calc_ea(ctx,1,mode,reg,&ea,0x1fc,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     s8 = (uint8_t)(cpu->d[reg2] & 0xff);
		     rc = READMEM_8_D(ea, &d8);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     r8 = s8 ^ d8;
		     rc = WRITEMEM_8_D(r8,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  if( MSB8(r8) )
		     cpu->sr |= 0x8;  /* N */

		  if( !r8 )
		     cpu->sr |= 0x4;  /* Z */
	       }

	    } else if(opmode==5) {
	       /* Word */

	       /* Dn ^ <ea> -> <ea> */
	       if(DISASM) {
		  ASM_INSTR(".W");
		  ASM_ARG1("D%d",reg2);
	       }

	       if(mode==0) {
		  if(DISASM)
		     ASM_ARG2("D%d",reg);

		  if(EMULATE) {
		     s16 = (uint16_t)(cpu->d[reg2] & 0xffff);
		     d16 = (uint16_t)(cpu->d[reg] & 0xffff);
		     r16 = s16 ^ d16;
		     cpu->d[reg] &= 0xffff0000;
		     cpu->d[reg] |= (uint32_t)r16;
		  }

	       } else {
		  /* EA, only alterable memory (0x1fc) */
		  ea_rc = calc_ea(ctx,2,mode,reg,&ea,0x1fc,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     s16 = (uint16_t)(cpu->d[reg2] & 0xffff);
		     rc = READMEM_16_D(ea, &d16);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     r16 = s16 ^ d16;
		     rc = WRITEMEM_16_D(r16,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  if( MSB16(r16) )
		     cpu->sr |= 0x8;  /* N */

		  if( !r16 )
		     cpu->sr |= 0x4;  /* Z */
	       }

	    } else {
	       /* Long */

	       /* Dn ^ <ea> -> <ea> */

	       if(DISASM) {
		  ASM_INSTR(".L");
		  ASM_ARG1("D%d",reg2);
	       }

	       if(mode==0) {
		  if(DISASM)
		     ASM_ARG2("D%d",reg);

		  if(EMULATE) {
		     s32 = cpu->d[reg2];
		     d32 = cpu->d[reg];
		     r32 = s32 ^ d32;
		     cpu->d[reg] = r32;
		  }

	       } else {
		  /* EA, only alterable memory (0x1fc) */
		  ea_rc = calc_ea(ctx,4,mode,reg,&ea,0x1fc,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     s32 = cpu->d[reg2];
		     rc = READMEM_32_D(ea, &d32);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     r32 = s32 ^ d32;
		     rc = WRITEMEM_32_D(r32,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  if( MSB32(r32) )
		     cpu->sr |= 0x8;  /* N */

		  if( !r32 )
		     cpu->sr |= 0x4;  /* Z */
	       }
	    }
	 }
	 return cpu68k::I_OK;
      }

/*
 * Function:       i_eori
 * Instructions:   EORI, EORI_CCR, EORI_SR
 * Bit pattern(s): 0000 1010 00
 *                 0000 1010 01
 *                 0000 1010 10
 */
      cpu68k::returncode i_eori(context *ctx, uint16_t iw)
      {
	 uint16_t size,mode,reg;
	 uint32_t ea;
	 uint8_t s8,d8,r8;
	 uint16_t s16,d16,r16;
	 uint32_t s32,d32,r32;
	 int rc;
	 cpu68k::returncode ea_rc;

	 cpu68k *cpu = &(ctx->cpu);

	 size = ( iw >> 6 ) & 0x3;
	 mode = ( iw >> 3 ) & 0x7;
	 reg = iw & 0x7;

	 if(DISASM)
	    ASM_INSTR("EORI");

	 if(size==0 && mode==7 && reg==4) {
	    /* EORI_CCR */
	    if(DISASM)
	       ASM_ARG2("CCR");

	    rc = READPC_16(&s16);
	    if(rc) return cpu68k::I_BUS_ERROR;

	    if(DISASM)
	       ASM_ARG1("#%u",(unsigned)s16);

	    if(EMULATE)
	       cpu->sr ^= (s16 & 0x1f);

	    return cpu68k::I_OK;

	 } else if(size==1 && mode==7 && reg==4) {
	    /* EORI_SR */
	    if(DISASM)
	       ASM_ARG2("SR");

	    rc = READPC_16(&s16);
	    if(rc) return cpu68k::I_BUS_ERROR;

	    if(DISASM)
	       ASM_ARG1("#%u",(unsigned)s16);

	    if(EMULATE)
	       return cpu68k::I_PRIVILEGE;

	    return cpu68k::I_OK;

	 } else {
	    /* EORI */
	    if(EMULATE)
	       cpu->sr &= 0xfff0;  /* Clear NZVC */

	    if(size==0) {
	       /* Byte */
	       if(DISASM)
		  ASM_INSTR(".B");

	       rc = READPC_8(&s8);
	       if(rc) return cpu68k::I_BUS_ERROR;

	       if(DISASM)
		  ASM_ARG1("#%u",(unsigned)s8);

	       if(mode==0) {
		  if(DISASM)
		     ASM_ARG2("D%d",reg);

		  if(EMULATE) {
		     d8 = (uint8_t)(cpu->d[reg] & 0xff);
		     r8 = s8 ^ d8;
		     cpu->d[reg] &= 0xffffff00;
		     cpu->d[reg] |= (uint32_t)r8;
		  }

	       } else {
		  /* EA, Dn + alterable memory (0x1fd) */
		  ea_rc = calc_ea(ctx,1,mode,reg,&ea,0x1fd,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     rc = READMEM_8_D(ea, &d8);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     r8 = s8 ^ d8;
		     rc = WRITEMEM_8_D(r8,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  if( MSB8(r8) )
		     cpu->sr |= 0x8;  /* N */

		  if( !r8 )
		     cpu->sr |= 0x4;  /* Z */
	       }

	    } else if(size==1) {
	       /* Word */
	       if(DISASM)
		  ASM_INSTR(".W");

	       rc = READPC_16(&s16);
	       if(rc) return cpu68k::I_BUS_ERROR;

	       if(DISASM)
		  ASM_ARG1("#%u",(unsigned)s16);

	       if(mode==0) {
		  if(DISASM)
		     ASM_ARG2("D%d",reg);

		  if(EMULATE) {
		     d16 = (uint16_t)(cpu->d[reg] & 0xffff);
		     r16 = s16 ^ d16;
		     cpu->d[reg] &= 0xffff0000;
		     cpu->d[reg] |= (uint32_t)r16;
		  }

	       } else {
		  /* EA, Dn + alterable memory (0x1fd) */
		  ea_rc = calc_ea(ctx,2,mode,reg,&ea,0x1fd,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     rc = READMEM_16_D(ea, &d16);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     r16 = s16 ^ d16;
		     rc = WRITEMEM_16_D(r16,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  if( MSB16(r16) )
		     cpu->sr |= 0x8;  /* N */

		  if( !r16 )
		     cpu->sr |= 0x4;  /* Z */
	       }

	    } else {
	       /* Long */
	       if(DISASM)
		  ASM_INSTR(".L");

	       rc = READPC_32(&s32);
	       if(rc) return cpu68k::I_BUS_ERROR;

	       if(DISASM)
		  ASM_ARG1("#%u",(unsigned)s32);

	       if(mode==0) {
		  if(DISASM)
		     ASM_ARG2("D%d",reg);

		  if(EMULATE) {
		     d32 = cpu->d[reg];
		     r32 = s32 ^ d32;
		     cpu->d[reg] = r32;
		  }

	       } else {
		  /* EA, Dn + alterable memory (0x1fd) */
		  ea_rc = calc_ea(ctx,4,mode,reg,&ea,0x1fd,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     rc = READMEM_32_D(ea, &d32);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     r32 = s32 ^ d32;
		     rc = WRITEMEM_32_D(r32,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  if( MSB32(r32) )
		     cpu->sr |= 0x8;  /* N */

		  if( !r32 )
		     cpu->sr |= 0x4;  /* Z */
	       }
	    }
	 }
	 return cpu68k::I_OK;
      }

/*
 * Function:       i_not
 * Instructions:   NOT
 * Bit pattern(s): 0100 0110 00
 *                 0100 0110 01
 *                 0100 0110 10
 */
      cpu68k::returncode i_not(context *ctx, uint16_t iw)
      {
	 uint16_t size,mode,reg;
	 uint32_t ea;
	 uint8_t d8,r8;
	 uint16_t d16,r16;
	 uint32_t d32,r32;
	 int rc;
	 cpu68k::returncode ea_rc;

	 cpu68k *cpu = &(ctx->cpu);

	 size = ( iw >> 6 ) & 0x3;
	 mode = ( iw >> 3 ) & 0x7;
	 reg = iw & 0x7;

	 if(EMULATE)
	    cpu->sr &= 0xfff0;  /* Clear NZVC */

	 if(DISASM)
	    ASM_INSTR("NOT");

	 if(size==0) {
	    /* Byte */
	    if(DISASM)
	       ASM_INSTR(".B");

	    if(mode==0) {
	       if(DISASM)
		  ASM_ARG1("D%d",reg);

	       if(EMULATE) {
		  d8 = (uint8_t)(cpu->d[reg] & 0xff);
		  r8 = ~d8;
		  cpu->d[reg] &= 0xffffff00;
		  cpu->d[reg] |= (uint32_t)r8;
	       }

	    } else {
	       /* EA, Dn + alterable memory (0x1fd) */
	       ea_rc = calc_ea(ctx,1,mode,reg,&ea,0x1fd,1);
	       if(ea_rc) return ea_rc;

	       if(EMULATE) {
		  rc = READMEM_8_D(ea, &d8);
		  if(rc) return cpu68k::I_BUS_ERROR;
		  r8 = ~d8;
		  rc = WRITEMEM_8_D(r8,ea);
		  if(rc) return cpu68k::I_BUS_ERROR;
	       }
	    }

	    if(EMULATE) {
	       if( MSB8(r8) )
		  cpu->sr |= 0x8;  /* N */

	       if( !r8 )
		  cpu->sr |= 0x4;  /* Z */
	    }

	 } else if(size==1) {
	    /* Word */
	    if(DISASM)
	       ASM_INSTR(".W");

	    if(mode==0) {
	       if(DISASM)
		  ASM_ARG1("D%d",reg);

	       if(EMULATE) {
		  d16 = (uint16_t)(cpu->d[reg] & 0xffff);
		  r16 = ~d16;
		  cpu->d[reg] &= 0xffff0000;
		  cpu->d[reg] |= (uint32_t)r16;
	       }

	    } else {
	       /* EA, Dn + alterable memory (0x1fd) */
	       ea_rc = calc_ea(ctx,2,mode,reg,&ea,0x1fd,1);
	       if(ea_rc) return ea_rc;

	       if(EMULATE) {
		  rc = READMEM_16_D(ea, &d16);
		  if(rc) return cpu68k::I_BUS_ERROR;
		  r16 = ~d16;
		  rc = WRITEMEM_16_D(r16,ea);
		  if(rc) return cpu68k::I_BUS_ERROR;
	       }
	    }

	    if(EMULATE) {
	       if( MSB16(r16) )
		  cpu->sr |= 0x8;  /* N */

	       if( !r16 )
		  cpu->sr |= 0x4;  /* Z */
	    }

	 } else {
	    /* Long */
	    if(DISASM)
	       ASM_INSTR(".L");

	    if(mode==0) {
	       if(DISASM)
		  ASM_ARG1("D%d",reg);

	       if(EMULATE) {
		  d32 = cpu->d[reg];
		  r32 = ~d32;
		  cpu->d[reg] = r32;
	       }

	    } else {
	       /* EA, Dn + alterable memory (0x1fd) */
	       ea_rc = calc_ea(ctx,4,mode,reg,&ea,0x1fd,1);
	       if(ea_rc) return ea_rc;

	       if(EMULATE) {
		  rc = READMEM_32_D(ea, &d32);
		  if(rc) return cpu68k::I_BUS_ERROR;
		  r32 = ~d32;
		  rc = WRITEMEM_32_D(r32,ea);
		  if(rc) return cpu68k::I_BUS_ERROR;
	       }
	    }

	    if(EMULATE) {
	       if( MSB32(r32) )
		  cpu->sr |= 0x8;  /* N */

	       if( !r32 )
		  cpu->sr |= 0x4;  /* Z */
	    }
	 }
	 return cpu68k::I_OK;
      }

   } // namespace cpu_68k
} // namespace emumiga
