/*
68k_1.cpp - M68K emulation. Instructions.

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
 * Function:       i_add_ax
 * Instructions:   ADD, ADDA, ADDX
 * Bit pattern(s): 1101 xxxx xx
 */
      cpu68k::returncode i_add_ax(context *ctx, uint16_t iw)
      {
	 uint16_t reg,mode,op,op2,reg2;
	 uint32_t ea,ea2;
	 uint8_t s8,d8,r8,x8;
	 uint16_t s16,d16,r16,x16;
	 uint32_t s32,d32,r32,x32;
	 int rc;
	 cpu68k::returncode ea_rc;

	 cpu68k *cpu = &(ctx->cpu);

	 reg2 = ( iw >> 9 ) & 0x7;
	 op2 = ( iw >> 8 ) & 0x1;
	 op = ( iw >> 6 ) & 0x3;
	 mode = ( iw >> 3 ) & 0x7;
	 reg = iw & 0x7;

	 if(op == 3) {
	    /* ADDA */
	    if(DISASM)
	       ASM_INSTR("ADDA");

	    if(op2 == 0) {
	       /* Word */
	       if(DISASM)
		  ASM_INSTR(".W");

	       if(mode==0) {
		  if(EMULATE)
		     s16 = (uint16_t)(cpu->d[reg] & 0xffff);

		  if(DISASM)
		     ASM_ARG1("D%d",reg);

	       } else if(mode==1) {
		  if(EMULATE)
		     s16 = (uint16_t)(cpu->a[reg] & 0xffff);

		  if(DISASM)
		     ASM_ARG1("A%d",reg);

	       } else if(mode==7 && reg==4) {
		  rc = READPC_16(&s16);
		  if(rc) return cpu68k::I_BUS_ERROR;

		  if(DISASM)
		     ASM_ARG1("#%u",(unsigned)s16);

	       } else {
		  /* EA, all (0xfff) */
		  ea_rc = calc_ea(ctx,2,mode,reg,&ea,0xfff,1);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     rc = READMEM_16_D(ea, &s16);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(DISASM)
		  ASM_ARG2("A%d",reg2);

	       if(EMULATE) {
		  s32 = (uint32_t)(int32_t)(int16_t)s16;
		  d32 = cpu->a[reg2];
		  r32 = s32 + d32;
		  cpu->a[reg2] = r32;

		  /* Increased SP? */
		  if(reg2 == 7 && !(s32 & 0x80000000)) {
		     CHECK_SP();
		  }
	       }
	    } else {
	       /* Long */
	       if(DISASM)
		  ASM_INSTR(".L");

	       if(mode==0) {
		  if(EMULATE)
		     s32 = cpu->d[reg];

		  if(DISASM)
		     ASM_ARG1("D%d",reg);

	       } else if(mode==1) {
		  if(EMULATE)
		     s32 = cpu->a[reg];

		  if(DISASM)
		     ASM_ARG1("A%d",reg);

	       } else if(mode==7 && reg==4) {
		  rc = READPC_32(&s32);
		  if(rc) return cpu68k::I_BUS_ERROR;

		  if(DISASM)
		     ASM_ARG1("#%u",(unsigned)s32);

	       } else {
		  /* EA, all (0xfff) */
		  ea_rc = calc_ea(ctx,4,mode,reg,&ea,0xfff,1);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     rc = READMEM_32_D(ea,&s32);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(DISASM)
		  ASM_ARG2("A%d",reg2);

	       if(EMULATE) {
		  d32 = cpu->a[reg2];
		  r32 = s32 + d32;
		  cpu->a[reg2] = r32;

		  /* Increased SP? */
		  if(reg2 == 7 && !(s32 & 0x80000000)) {
		     CHECK_SP();
		  }
	       }
	    }

	 } else if(op2==1 && mode<3) {
	    /* ADDX */
	    if(DISASM)
	       ASM_INSTR("ADDX");

	    if(op==0) {
	       /* Byte */
	       if(DISASM)
		  ASM_INSTR(".B");

	       if(mode==0) {	/* Dy + Dx + X -> Dx */
		  if(DISASM) {
		     ASM_ARG1("D%d",reg);
		     ASM_ARG2("D%d",reg2);
		  }

		  if(EMULATE) {
		     s8 = (uint8_t)(cpu->d[reg] & 0xff);
		     d8 = (uint8_t)(cpu->d[reg2] & 0xff);
		     x8 = (cpu->sr&0x10 ? 1 : 0);
		     r8 = s8 + d8 + x8;
		     cpu->d[reg2] &= 0xffffff00;
		     cpu->d[reg2] |= (uint32_t)r8;
		  }

	       } else {	   /* -(Ay) + -(Ax) + X -> (Ax) */
                  /* EA, only -(An) (0x10) */
		  ea_rc = calc_ea(ctx,1,4,reg,&ea,0x10,1);
		  if(ea_rc) return ea_rc;

		  /* EA, only -(An) (0x10) */
		  ea_rc = calc_ea(ctx,1,4,reg2,&ea2,0x10,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     rc = READMEM_8_D(ea, &s8);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     rc = READMEM_8_D(ea2, &d8);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     x8 = (cpu->sr&0x10 ? 1 : 0);
		     r8 = s8 + d8 + x8;
		     rc = WRITEMEM_8_D(r8,ea2);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  cpu->sr &= 0xffe4;  /* Clear XNVC */

		  if( (  MSB8(s8) &&  MSB8(d8) && !MSB8(r8) ) ||
		      ( !MSB8(s8) && !MSB8(d8) &&  MSB8(r8) ) )
		     cpu->sr |= 0x2;  /* V */

		  if( (  MSB8(s8) && MSB8(d8) ) ||
		      ( !MSB8(r8) && MSB8(d8) ) ||
		      ( !MSB8(r8) && MSB8(s8) ) )
		     cpu->sr |= 0x11;  /* XC */

		  if( MSB8(r8) )
		     cpu->sr |= 0x8;  /* N */

		  if( r8 )
		     cpu->sr &= 0xfffb;  /* !Z */
	       }

	    } else if(op == 1) {
	       /* Word */

	       if(DISASM)
		  ASM_INSTR(".W");

	       if(mode==0) {	/* Dy + Dx + X -> Dx */
		  if(DISASM) {
		     ASM_ARG1("D%d",reg);
		     ASM_ARG2("D%d",reg2);
		  }

		  if(EMULATE) {
		     s16 = (uint16_t)(cpu->d[reg] & 0xffff);
		     d16 = (uint16_t)(cpu->d[reg2] & 0xffff);
		     x16 = (cpu->sr&0x10 ? 1 : 0);
		     r16 = s16 + d16 + x16;
		     cpu->d[reg2] &= 0xffff0000;
		     cpu->d[reg2] |= (uint32_t)r16;
		  }

	       } else {	/* -(Ay) + -(Ax) + X -> (Ax) */
                  /* EA, only -(An) (0x10) */
		  ea_rc = calc_ea(ctx,2,4,reg,&ea,0x10,1);
		  if(ea_rc) return ea_rc;

		  /* EA, only -(An) (0x10) */
		  ea_rc = calc_ea(ctx,2,4,reg2,&ea2,0x10,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     rc = READMEM_16_D(ea, &s16);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     rc = READMEM_16_D(ea2, &d16);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     x16 = (cpu->sr&0x10 ? 1 : 0);
		     r16 = s16 + d16 + x16;
		     rc = WRITEMEM_16_D(r16,ea2);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  cpu->sr &= 0xffe4;  /* Clear XNVC */

		  if( (  MSB16(s16) &&  MSB16(d16) && !MSB16(r16) ) ||
		      ( !MSB16(s16) && !MSB16(d16) &&  MSB16(r16) ) )
		     cpu->sr |= 0x2;  /* V */

		  if( (  MSB16(s16) && MSB16(d16) ) ||
		      ( !MSB16(r16) && MSB16(d16) ) ||
		      ( !MSB16(r16) && MSB16(s16) ) )
		     cpu->sr |= 0x11;  /* XC */

		  if( MSB16(r16) )
		     cpu->sr |= 0x8;  /* N */

		  if( r16 )
		     cpu->sr &= 0xfffb;  /* !Z */
	       }

	    } else {
	       /* Long */

	       if(DISASM)
		  ASM_INSTR(".L");

	       if(mode==0) {	/* Dy + Dx + X -> Dx */
		  if(DISASM) {
		     ASM_ARG1("D%d",reg);
		     ASM_ARG2("D%d",reg2);
		  }

		  if(EMULATE) {
		     s32 = cpu->d[reg];
		     d32 = cpu->d[reg2];
		     x32 = (cpu->sr&0x10 ? 1 : 0);
		     r32 = s32 + d32 + x32;
		     cpu->d[reg2] = r32;
		  }

	       } else {	/* -(Ay) + -(Ax) + X -> (Ax) */
		  /* EA, only -(An) (0x10) */
		  ea_rc = calc_ea(ctx,4,4,reg,&ea,0x10,1);
		  if(ea_rc) return ea_rc;

		  /* EA, only -(An) (0x10) */
		  ea_rc = calc_ea(ctx,4,4,reg2,&ea2,0x10,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     rc = READMEM_32_D(ea, &s32);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     rc = READMEM_32_D(ea2, &d32);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     x32 = (cpu->sr&0x10 ? 1 : 0);
		     r32 = s32 + d32 + x32;
		     rc = WRITEMEM_32_D(r32,ea2);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  cpu->sr &= 0xffe4;  /* Clear XNVC */

		  if( (  MSB32(s32) &&  MSB32(d32) && !MSB32(r32) ) ||
		      ( !MSB32(s32) && !MSB32(d32) &&  MSB32(r32) ) )
		     cpu->sr |= 0x2;  /* V */

		  if( (  MSB32(s32) && MSB32(d32) ) ||
		      ( !MSB32(r32) && MSB32(d32) ) ||
		      ( !MSB32(r32) && MSB32(s32) ) )
		     cpu->sr |= 0x11;  /* XC */

		  if( MSB32(r32) )
		     cpu->sr |= 0x8;  /* N */

		  if( r32 )
		     cpu->sr &= 0xfffb;  /* !Z */
	       }
	    }

	 } else {
	    /* ADD */
	    if(DISASM)
	       ASM_INSTR("ADD");

	    if(EMULATE)
	       cpu->sr &= 0xffe0;  /* Clear XNZVC */

	    if(op==0) {
	       /* Byte */
	       if(DISASM)
		  ASM_INSTR(".B");

	       if(op2==0) {	/* <ea> + Dn -> Dn */
		  if(mode==0) {
		     if(EMULATE)
			s8 = (uint8_t)(cpu->d[reg] & 0xff);

		     if(DISASM)
			ASM_ARG1("D%d",reg);

		  } else if(mode==7 && reg==4) {
		     rc = READPC_8(&s8);
		     if(rc) return cpu68k::I_BUS_ERROR;

		     if(DISASM)
			ASM_ARG1("#%u",(unsigned)s8);

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
		     r8 = s8 + d8;
		     cpu->d[reg2] &= 0xffffff00;
		     cpu->d[reg2] |= (uint32_t)r8;
		  }

	       } else {	/* Dn + <ea> -> <ea> */
		  if(DISASM)
		     ASM_ARG1("D%d",reg2);

		  /* EA, only alterable memory (0x1fc) */
		  ea_rc = calc_ea(ctx,1,mode,reg,&ea,0x1fc,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     s8 = (uint8_t)(cpu->d[reg2] & 0xff);
		     rc = READMEM_8_D(ea, &d8);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     r8 = s8 + d8;
		     rc = WRITEMEM_8_D(r8,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  if( (  MSB8(s8) &&  MSB8(d8) && !MSB8(r8) ) ||
		      ( !MSB8(s8) && !MSB8(d8) &&  MSB8(r8) ) )
		     cpu->sr |= 0x2;  /* V */

		  if( (  MSB8(s8) && MSB8(d8) ) ||
		      ( !MSB8(r8) && MSB8(d8) ) ||
		      ( !MSB8(r8) && MSB8(s8) ) )
		     cpu->sr |= 0x11;  /* XC */

		  if( MSB8(r8) )
		     cpu->sr |= 0x8;  /* N */

		  if( !r8 )
		     cpu->sr |= 0x4;  /* Z */
	       }

	    } else if(op==1) {
	       /* Word */

	       if(DISASM)
		  ASM_INSTR(".W");

	       if(op2==0) {	/* <ea> + Dn -> Dn */
		  if(mode==0) {
		     if(EMULATE)
			s16 = (uint16_t)(cpu->d[reg] & 0xffff);

		     if(DISASM)
			ASM_ARG1("D%d",reg);

		  } else if(mode==1) {
		     if(EMULATE)
			s16 = (uint16_t)(cpu->a[reg] & 0xffff);

		     if(DISASM)
			ASM_ARG1("A%d",reg);

		  } else if(mode==7 && reg==4) {
		     rc = READPC_16(&s16);
		     if(rc) return cpu68k::I_BUS_ERROR;

		     if(DISASM)
			ASM_ARG1("#%u",(unsigned)s16);

		  } else {
		     /* EA, all (0xfff) */
		     ea_rc = calc_ea(ctx,2,mode,reg,&ea,0xff,1);
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
		     r16 = s16 + d16;
		     cpu->d[reg2] &= 0xffff0000;
		     cpu->d[reg2] |= (uint32_t)r16;
		  }

	       } else {	/* Dn + <ea> -> <ea> */
		  if(DISASM)
		     ASM_ARG1("D%d",reg2);

		  /* EA, only alterable memory (0x1fc) */
		  ea_rc = calc_ea(ctx,2,mode,reg,&ea,0x1fc,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     s16 = (uint16_t)(cpu->d[reg2] & 0xffff);
		     rc = READMEM_16_D(ea, &d16);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     r16 = s16 + d16;
		     rc = WRITEMEM_16_D(r16,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  if( (  MSB16(s16) &&  MSB16(d16) && !MSB16(r16) ) ||
		      ( !MSB16(s16) && !MSB16(d16) &&  MSB16(r16) ) )
		     cpu->sr |= 0x2;  /* V */

		  if( (  MSB16(s16) && MSB16(d16) ) ||
		      ( !MSB16(r16) && MSB16(d16) ) ||
		      ( !MSB16(r16) && MSB16(s16) ) )
		     cpu->sr |= 0x11;  /* XC */

		  if( MSB16(r16) )
		     cpu->sr |= 0x8;  /* N */

		  if( !r16 )
		     cpu->sr |= 0x4;  /* Z */
	       }

	    } else {
	       /* Long */

	       if(DISASM)
		  ASM_INSTR(".L");

	       if(op2==0) {	/* <ea> + Dn -> Dn */
		  if(mode==0) {
		     if(EMULATE)
			s32 = cpu->d[reg];

		     if(DISASM)
			ASM_ARG1("D%d",reg);

		  } else if(mode==1) {
		     if(EMULATE)
			s32 = cpu->a[reg];

		     if(DISASM)
			ASM_ARG1("A%d",reg);

		  } else if(mode==7 && reg==4) {
		     rc = READPC_32(&s32);
		     if(rc) return cpu68k::I_BUS_ERROR;

		     if(DISASM)
			ASM_ARG1("#%u",(unsigned)s32);

		  } else {
		     /* EA, all (0xfff) */
		     ea_rc = calc_ea(ctx,4,mode,reg,&ea,0xfff,1);
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
		     r32 = s32 + d32;
		     cpu->d[reg2] = r32;
		  }

	       } else {	/* Dn + <ea> -> <ea> */
		  if(DISASM)
		     ASM_ARG1("D%d",reg2);

		  /* EA, only alterable memory (0x1fc) */
		  ea_rc = calc_ea(ctx,4,mode,reg,&ea,0x1fc,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     s32 = cpu->d[reg2];
		     rc = READMEM_32_D(ea, &d32);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     r32 = s32 + d32;
		     rc = WRITEMEM_32_D(r32,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  if( (  MSB32(s32) &&  MSB32(d32) && !MSB32(r32) ) ||
		      ( !MSB32(s32) && !MSB32(d32) &&  MSB32(r32) ) )
		     cpu->sr |= 0x2;  /* V */

		  if( (  MSB32(s32) && MSB32(d32) ) ||
		      ( !MSB32(r32) && MSB32(d32) ) ||
		      ( !MSB32(r32) && MSB32(s32) ) )
		     cpu->sr |= 0x11;  /* XC */

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
 * Function:       i_addi
 * Instructions:   ADDI
 * Bit pattern(s): 0000 0110 00
 *                 0000 0110 01
 *                 0000 0110 10
 */
      cpu68k::returncode i_addi(context *ctx, uint16_t iw)
      {
	 uint16_t reg,mode,size;
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
	    ASM_INSTR("ADDI");

	 if(EMULATE)
	    cpu->sr &= 0xffe0;  /* Clear XNZVC */

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
		  r8 = s8 + d8;
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
		  r8 = s8 + d8;
		  rc = WRITEMEM_8_D(r8,ea);
		  if(rc) return cpu68k::I_BUS_ERROR;
	       }
	    }

	    if(EMULATE) {
	       if( (  MSB8(s8) &&  MSB8(d8) && !MSB8(r8) ) ||
	           ( !MSB8(s8) && !MSB8(d8) &&  MSB8(r8) ) )
		  cpu->sr |= 0x2;  /* V */

	       if( (  MSB8(s8) && MSB8(d8) ) ||
	           ( !MSB8(r8) && MSB8(d8) ) ||
	           ( !MSB8(r8) && MSB8(s8) ) )
		  cpu->sr |= 0x11;  /* XC */

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
		  r16 = s16 + d16;
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
		  r16 = s16 + d16;
		  rc = WRITEMEM_16_D(r16,ea);
		  if(rc) return cpu68k::I_BUS_ERROR;
	       }
	    }

	    if(EMULATE) {
	       if( (  MSB16(s16) &&  MSB16(d16) && !MSB16(r16) ) ||
	           ( !MSB16(s16) && !MSB16(d16) &&  MSB16(r16) ) )
		  cpu->sr |= 0x2;  /* V */

	       if( (  MSB16(s16) && MSB16(d16) ) ||
	           ( !MSB16(r16) && MSB16(d16) ) ||
	           ( !MSB16(r16) && MSB16(s16) ) )
		  cpu->sr |= 0x11;  /* XC */

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
		  r32 = s32 + d32;
		  cpu->d[reg] = r32;
	       }

	    } else {
	       /* EA, Dn + alterable memory (0x1fd) */
	       ea_rc = calc_ea(ctx,4,mode,reg,&ea,0x1fd,2);
	       if(ea_rc) return ea_rc;

	       if(EMULATE) {
		  rc = READMEM_32_D(ea, &d32);
		  if(rc) return cpu68k::I_BUS_ERROR;
		  r32 = s32 + d32;
		  rc = WRITEMEM_32_D(r32,ea);
		  if(rc) return cpu68k::I_BUS_ERROR;
	       }
	    }

	    if(EMULATE) {
	       if( (  MSB32(s32) &&  MSB32(d32) && !MSB32(r32) ) ||
	           ( !MSB32(s32) && !MSB32(d32) &&  MSB32(r32) ) )
		  cpu->sr |= 0x2;  /* V */

	       if( (  MSB32(s32) && MSB32(d32) ) ||
	           ( !MSB32(r32) && MSB32(d32) ) ||
	           ( !MSB32(r32) && MSB32(s32) ) )
		  cpu->sr |= 0x11;  /* XC */

	       if( MSB32(r32) )
		  cpu->sr |= 0x8;  /* N */

	       if( !r32 )
		  cpu->sr |= 0x4;  /* Z */
	    }
	 }
	 return cpu68k::I_OK;
      }

/*
 * Function:       i_addq
 * Instructions:   ADDQ
 * Bit pattern(s): 0101 xxx0 00
 *                 0101 xxx0 01
 *                 0101 xxx0 10
 */
      cpu68k::returncode i_addq(context *ctx, uint16_t iw)
      {
	 uint16_t reg,mode,size;
	 uint32_t data;
	 uint32_t ea;
	 uint8_t s8,d8,r8;
	 uint16_t s16,d16,r16=0;
	 uint32_t s32,d32,r32;
	 int rc;
	 cpu68k::returncode ea_rc;

	 cpu68k *cpu = &(ctx->cpu);

	 data = ( iw >> 9 ) & 0x7;
	 if(data==0) data=8;
	 size = ( iw >> 6 ) & 0x3;
	 mode = ( iw >> 3 ) & 0x7;
	 reg = iw & 0x7;

	 if(DISASM)
	    ASM_INSTR("ADDQ");

	 if(EMULATE) {
	    if(mode!=1)
	       cpu->sr &= 0xffe0;  /* Clear XNZVC */
	 }

	 if(size==0) {
	    /* Byte */
	    if(DISASM) {
	       ASM_INSTR(".B");
	       ASM_ARG1("#%u",(unsigned)data);
	    }

	    s8 = (uint8_t) data;

	    if(mode==0) {
	       if(DISASM)
		  ASM_ARG2("D%d",reg);

	       if(EMULATE) {
		  d8 = (uint8_t)(cpu->d[reg] & 0xff);
		  r8 = s8 + d8;
		  cpu->d[reg] &= 0xffffff00;
		  cpu->d[reg] |= (uint32_t)r8;
	       }

	    } else {
	       /* EA, alterable memory (0x1fc) */
	       ea_rc = calc_ea(ctx,1,mode,reg,&ea,0x1fd,2);
	       if(ea_rc) return ea_rc;

	       if(EMULATE) {
		  rc = READMEM_8_D(ea, &d8);
		  if(rc) return cpu68k::I_BUS_ERROR;
		  r8 = s8 + d8;
		  rc = WRITEMEM_8_D(r8,ea);
		  if(rc) return cpu68k::I_BUS_ERROR;
	       }
	    }

	    if(EMULATE) {
	       if( (  MSB8(s8) &&  MSB8(d8) && !MSB8(r8) ) ||
	           ( !MSB8(s8) && !MSB8(d8) &&  MSB8(r8) ) )
		  cpu->sr |= 0x2;  /* V */

	       if( (  MSB8(s8) && MSB8(d8) ) ||
	           ( !MSB8(r8) && MSB8(d8) ) ||
	           ( !MSB8(r8) && MSB8(s8) ) )
		  cpu->sr |= 0x11;  /* XC */

	       if( MSB8(r8) )
		  cpu->sr |= 0x8;  /* N */

	       if( !r8 )
		  cpu->sr |= 0x4;  /* Z */
	    }

	 } else if(size==1) {
	    /* Word */
	    if(DISASM) {
	       ASM_INSTR(".W");
	       ASM_ARG1("#%u",(unsigned)data);
	    }

	    s16 = (uint16_t) data;

	    if(mode==0) {
	       if(DISASM)
		  ASM_ARG2("D%d",reg);

	       if(EMULATE) {
		  d16 = (uint16_t)(cpu->d[reg] & 0xffff);
		  r16 = s16 + d16;
		  cpu->d[reg] &= 0xffff0000;
		  cpu->d[reg] |= (uint32_t)r16;
	       }

	    } else if(mode==1) {
	       if(DISASM)
		  ASM_ARG2("A%d",reg);

	       if(EMULATE) {
		  s32 = (uint32_t)(int32_t)(int16_t)s16;
		  d32 = cpu->a[reg];
		  r32 = s32 + d32;
		  cpu->a[reg] = r32;

		  /* Increased SP? */
		  if(reg == 7 && !(s32 & 0x80000000)) {
		     CHECK_SP();
		  }
	       }

	    } else {
	       /* EA, alterable memory (0x1fc) */
	       ea_rc = calc_ea(ctx,2,mode,reg,&ea,0x1fc,2);
	       if(ea_rc) return ea_rc;

	       if(EMULATE) {
		  rc = READMEM_16_D(ea, &d16);
		  if(rc) return cpu68k::I_BUS_ERROR;
		  r16 = s16 + d16;
		  rc = WRITEMEM_16_D(r16,ea);
		  if(rc) return cpu68k::I_BUS_ERROR;
	       }
	    }

	    if(EMULATE) {
	       if(mode != 1) {
		  if( (  MSB16(s16) &&  MSB16(d16) && !MSB16(r16) ) ||
		      ( !MSB16(s16) && !MSB16(d16) &&  MSB16(r16) ) )
		     cpu->sr |= 0x2;  /* V */

		  if( (  MSB16(s16) && MSB16(d16) ) ||
		      ( !MSB16(r16) && MSB16(d16) ) ||
		      ( !MSB16(r16) && MSB16(s16) ) )
		     cpu->sr |= 0x11;  /* XC */

		  if( MSB16(r16) )
		     cpu->sr |= 0x8;  /* N */

		  if( !r16 )
		     cpu->sr |= 0x4;  /* Z */
	       }
	    }

	 } else {
	    /* Long */
	    if(DISASM) {
	       ASM_INSTR(".L");
	       ASM_ARG1("#%u",(unsigned)data);
	    }

	    s32 = data;

	    if(mode==0) {
	       if(DISASM)
		  ASM_ARG2("D%d",reg);

	       if(EMULATE) {
		  d32 = cpu->d[reg];
		  r32 = s32 + d32;
		  cpu->d[reg] = r32;
	       }

	    } else if(mode==1) {
	       if(DISASM)
		  ASM_ARG2("A%d",reg);

	       if(EMULATE) {
		  d32 = cpu->a[reg];
		  r32 = s32 + d32;
		  cpu->a[reg] = r32;

		  /* Increased SP? */
		  if(reg == 7 && !(s32 & 0x80000000)) {
		     CHECK_SP();
		  }
	       }

	    } else {
	       /* EA, alterable memory (0x1fc) */
	       ea_rc = calc_ea(ctx,4,mode,reg,&ea,0x1fc,2);
	       if(ea_rc) return ea_rc;

	       if(EMULATE) {
		  rc = READMEM_32_D(ea, &d32);
		  if(rc) return cpu68k::I_BUS_ERROR;
		  r32 = s32 + d32;
		  rc = WRITEMEM_32_D(r32,ea);
		  if(rc) return cpu68k::I_BUS_ERROR;
	       }
	    }

	    if(EMULATE) {
	       if(mode != 1) {
		  if( (  MSB32(s32) &&  MSB32(d32) && !MSB32(r32) ) ||
		      ( !MSB32(s32) && !MSB32(d32) &&  MSB32(r32) ) )
		     cpu->sr |= 0x2;  /* V */

		  if( (  MSB32(s32) && MSB32(d32) ) ||
		      ( !MSB32(r32) && MSB32(d32) ) ||
		      ( !MSB32(r32) && MSB32(s32) ) )
		     cpu->sr |= 0x11;  /* XC */

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
 * Function:       i_sub_ax
 * Instructions:   SUB, SUBA, SUBX
 * Bit pattern(s): 1001 xxxx xx
 */
      cpu68k::returncode i_sub_ax(context *ctx, uint16_t iw)
      {
	 uint16_t reg,mode,op,op2,reg2;
	 uint32_t ea,ea2;
	 uint8_t s8,d8,r8,x8;
	 uint16_t s16,d16,r16,x16;
	 uint32_t s32,d32,r32,x32;
	 int rc;
	 cpu68k::returncode ea_rc;

	 cpu68k *cpu = &(ctx->cpu);

	 reg2 = ( iw >> 9 ) & 0x7;
	 op2 = ( iw >> 8 ) & 0x1;
	 op = ( iw >> 6 ) & 0x3;
	 mode = ( iw >> 3 ) & 0x7;
	 reg = iw & 0x7;

	 if(op == 3) {
	    /* SUBA */
	    if(DISASM)
	       ASM_INSTR("SUBA");

	    if(op2 == 0) {
	       /* Word */
	       if(DISASM)
		  ASM_INSTR(".W");

	       if(mode==0) {
		  if(EMULATE)
		     s16 = (uint16_t)(cpu->d[reg] & 0xffff);

		  if(DISASM)
		     ASM_ARG1("D%d",reg);

	       } else if(mode==1) {
		  if(EMULATE)
		     s16 = (uint16_t)(cpu->a[reg] & 0xffff);

		  if(DISASM)
		     ASM_ARG1("A%d",reg);

	       } else if(mode==7 && reg==4) {
		  rc = READPC_16(&s16);
		  if(rc) return cpu68k::I_BUS_ERROR;

		  if(DISASM)
		     ASM_ARG1("#%u",(unsigned)s16);

	       } else {
		  /* EA, all (0xfff) */
		  ea_rc = calc_ea(ctx,2,mode,reg,&ea,0xfff,1);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     rc = READMEM_16_D(ea, &s16);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(DISASM)
		  ASM_ARG2("A%d",reg2);

	       if(EMULATE) {
		  s32 = (uint32_t)(int32_t)(int16_t)s16;
		  d32 = cpu->a[reg2];
		  r32 = d32 - s32;
		  cpu->a[reg2] = r32;

		  /* Increased SP? */
		  if(reg2 == 7 && (s32 & 0x80000000)) {
		     CHECK_SP();
		  }
	       }

	    } else {
	       /* Long */
	       if(DISASM)
		  ASM_INSTR(".L");

	       if(mode==0) {
		  if(EMULATE)
		     s32 = cpu->d[reg];

		  if(DISASM)
		     ASM_ARG1("D%d",reg);

	       } else if(mode==1) {
		  if(EMULATE)
		     s32 = cpu->a[reg];

		  if(DISASM)
		     ASM_ARG1("A%d",reg);

	       } else if(mode==7 && reg==4) {
		  rc = READPC_32(&s32);
		  if(rc) return cpu68k::I_BUS_ERROR;

		  if(DISASM)
		     ASM_ARG1("#%u",(unsigned)s32);

	       } else {
		  /* EA, all (0xfff) */
		  ea_rc = calc_ea(ctx,4,mode,reg,&ea,0xfff,1);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     rc = READMEM_32_D(ea, &s32);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(DISASM)
		  ASM_ARG2("A%d",reg2);

	       if(EMULATE) {
		  d32 = cpu->a[reg2];
		  r32 = d32 - s32;
		  cpu->a[reg2] = r32;

		  /* Increased SP? */
		  if(reg2 == 7 && (s32 & 0x80000000)) {
		     CHECK_SP();
		  }
	       }
	    }

	 } else if(op2==1 && mode<3) {
	    /* SUBX */
	    if(DISASM)
	       ASM_INSTR("SUBX");

	    if(op==0) {
	       /* Byte */
	       if(DISASM)
		  ASM_INSTR(".B");

	       if(mode==0) {	/* Dx - Dy - X -> Dx */
		  if(DISASM) {
		     ASM_ARG1("D%d",reg);
		     ASM_ARG2("D%d",reg2);
		  }
		  if(EMULATE) {
		     s8 = (uint8_t)(cpu->d[reg] & 0xff);
		     d8 = (uint8_t)(cpu->d[reg2] & 0xff);
		     x8 = (cpu->sr&0x10 ? 1 : 0);
		     r8 = d8 - s8 - x8;
		     cpu->d[reg2] &= 0xffffff00;
		     cpu->d[reg2] |= (uint32_t)r8;
		  }

	       } else {	/* -(Ax) - -(Ay) - X -> (Ax) */
		  /* EA, only -(An) (0x10) */
		  ea_rc = calc_ea(ctx,1,4,reg,&ea,0x10,1);
		  if(ea_rc) return ea_rc;

		  /* EA, only -(An) (0x10) */
		  ea_rc = calc_ea(ctx,1,4,reg2,&ea2,0x10,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     rc = READMEM_8_D(ea, &s8);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     rc = READMEM_8_D(ea2, &d8);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     x8 = (cpu->sr&0x10 ? 1 : 0);
		     r8 = d8 - s8 - x8;
		     rc = WRITEMEM_8_D(r8,ea2);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  cpu->sr &= 0xffe4;  /* Clear XNVC */

		  if( ( !MSB8(s8) &&  MSB8(d8) && !MSB8(r8) ) ||
		      (  MSB8(s8) && !MSB8(d8) &&  MSB8(r8) ) )
		     cpu->sr |= 0x2;  /* V */

		  if( ( MSB8(s8) && !MSB8(d8) ) ||
		      ( MSB8(r8) && !MSB8(d8) ) ||
		      ( MSB8(s8) &&  MSB8(r8) ) )
		     cpu->sr |= 0x11;  /* XC */

		  if( MSB8(r8) )
		     cpu->sr |= 0x8;  /* N */

		  if( r8 )
		     cpu->sr &= 0xfffb;  /* !Z */
	       }

	    } else if(op==1) {
	       /* Word */
	       if(DISASM)
		  ASM_INSTR(".W");

	       if(mode==0) {	/* Dx - Dy - X -> Dx */
		  if(DISASM) {
		     ASM_ARG1("D%d",reg);
		     ASM_ARG2("D%d",reg2);
		  }

		  if(EMULATE) {
		     s16 = (uint16_t)(cpu->d[reg] & 0xffff);
		     d16 = (uint16_t)(cpu->d[reg2] & 0xffff);
		     x16 = (cpu->sr&0x10 ? 1 : 0);
		     r16 = d16 - s16 - x16;
		     cpu->d[reg2] &= 0xffff0000;
		     cpu->d[reg2] |= (uint32_t)r16;
		  }

	       } else {	/* -(Ax) - -(Ay) - X -> (Ax) */
 		  /* EA, only -(An) (0x10) */
		     ea_rc = calc_ea(ctx,2,4,reg,&ea,0x10,1);
		  if(ea_rc) return ea_rc;

		  /* EA, only -(An) (0x10) */
		  ea_rc = calc_ea(ctx,2,4,reg2,&ea2,0x10,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     rc = READMEM_16_D(ea, &s16);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     rc = READMEM_16_D(ea2, &d16);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     x16 = (cpu->sr&0x10 ? 1 : 0);
		     r16 = d16 - s16 - x16;
		     rc = WRITEMEM_16_D(r16,ea2);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  cpu->sr &= 0xffe4;  /* Clear XNVC */

		  if( ( !MSB16(s16) &&  MSB16(d16) && !MSB16(r16) ) ||
		      (  MSB16(s16) && !MSB16(d16) &&  MSB16(r16) ) )
		     cpu->sr |= 0x2;  /* V */

		  if( ( MSB16(s16) && !MSB16(d16) ) ||
		      ( MSB16(r16) && !MSB16(d16) ) ||
		      ( MSB16(s16) &&  MSB16(r16) ) )
		     cpu->sr |= 0x11;  /* XC */

		  if( MSB16(r16) )
		     cpu->sr |= 0x8;  /* N */

		  if( r16 )
		     cpu->sr &= 0xfffb;  /* !Z */
	       }

	    } else {
	       /* Long */
	       if(DISASM)
		  ASM_INSTR(".L");

	       if(mode==0) {	/* Dx - Dy - X -> Dx */
		  if(DISASM) {
		     ASM_ARG1("D%d",reg);
		     ASM_ARG2("D%d",reg2);
		  }

		  if(EMULATE) {
		     s32 = cpu->d[reg];
		     d32 = cpu->d[reg2];
		     x32 = (cpu->sr&0x10 ? 1 : 0);
		     r32 = d32 - s32 - x32;
		     cpu->d[reg2] = r32;
		  }

	       } else {	/* -(Ax) - -(Ay) - X -> (Ax) */
		  /* EA, only -(An) (0x10) */
		  ea_rc = calc_ea(ctx,4,4,reg,&ea,0x10,1);
		  if(ea_rc) return ea_rc;

		  /* EA, only -(An) (0x10) */
		  ea_rc = calc_ea(ctx,4,4,reg2,&ea2,0x10,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     rc = READMEM_32_D(ea, &s32);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     rc = READMEM_32_D(ea2, &d32);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     x32 = (cpu->sr&0x10 ? 1 : 0);
		     r32 = d32 - s32 - x32;
		     rc = WRITEMEM_32_D(r32,ea2);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  cpu->sr &= 0xffe4;  /* Clear XNVC */

		  if( ( !MSB32(s32) &&  MSB32(d32) && !MSB32(r32) ) ||
		      (  MSB32(s32) && !MSB32(d32) &&  MSB32(r32) ) )
		     cpu->sr |= 0x2;  /* V */

		  if( ( MSB32(s32) && !MSB32(d32) ) ||
		      ( MSB32(r32) && !MSB32(d32) ) ||
		      ( MSB32(s32) &&  MSB32(r32) ) )
		     cpu->sr |= 0x11;  /* XC */

		  if( MSB32(r32) )
		     cpu->sr |= 0x8;  /* N */

		  if( r32 )
		     cpu->sr &= 0xfffb;  /* !Z */
	       }
	    }

	 } else {
	    /* SUB */
	    if(DISASM)
	       ASM_INSTR("SUB");

	    if(EMULATE)
	       cpu->sr &= 0xffe0;  /* Clear XNZVC */

	    if(op==0) {
	       /* Byte */
	       if(DISASM)
		  ASM_INSTR(".B");

	       if(op2==0) {	/* Dn - <ea> -> Dn */
		  if(mode==0) {
		     if(EMULATE)
			s8 = (uint8_t)(cpu->d[reg] & 0xff);

		     if(DISASM)
			ASM_ARG1("D%d",reg);

		  } else if(mode==1) {
		     if(EMULATE)
			s8 = (uint8_t)(cpu->a[reg] & 0xff);

		     if(DISASM)
			ASM_ARG1("A%d",reg);

		  } else if(mode==7 && reg==4) {
		     rc = READPC_8(&s8);
		     if(rc) return cpu68k::I_BUS_ERROR;

		     if(DISASM)
			ASM_ARG1("#%u",(unsigned)s8);

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
		     r8 = d8 - s8;
		     cpu->d[reg2] &= 0xffffff00;
		     cpu->d[reg2] |= (uint32_t)r8;
		  }

	       } else {	/* <ea> - Dn -> <ea> */
		  if(DISASM)
		     ASM_ARG1("D%d",reg2);

		  /* EA, only alterable memory (0x1fc) */
		  ea_rc = calc_ea(ctx,1,mode,reg,&ea,0x1fc,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     s8 = (uint8_t)(cpu->d[reg2] & 0xff);
		     rc = READMEM_8_D(ea, &d8);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     r8 = d8 - s8;
		     rc = WRITEMEM_8_D(r8,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  if( ( !MSB8(s8) &&  MSB8(d8) && !MSB8(r8) ) ||
		      (  MSB8(s8) && !MSB8(d8) &&  MSB8(r8) ) )
		     cpu->sr |= 0x2;  /* V */

		  if( ( MSB8(s8) && !MSB8(d8) ) ||
		      ( MSB8(r8) && !MSB8(d8) ) ||
		      ( MSB8(s8) &&  MSB8(r8) ) )
		     cpu->sr |= 0x11;  /* XC */

		  if( MSB8(r8) )
		     cpu->sr |= 0x8;  /* N */

		  if( !r8 )
		     cpu->sr |= 0x4;  /* Z */
	       }

	    } else if(op==1) {
	       /* Word */
	       if(DISASM)
		  ASM_INSTR(".W");

	       if(op2==0) {	/* Dn - <ea> -> Dn */
		  if(mode==0) {
		     if(EMULATE)
			s16 = (uint16_t)(cpu->d[reg] & 0xffff);

		     if(DISASM)
			ASM_ARG1("D%d",reg);

		  } else if(mode==1) {
		     if(EMULATE)
			s16 = (uint16_t)(cpu->a[reg] & 0xffff);

		     if(DISASM)
			ASM_ARG1("A%d",reg);

		  } else if(mode==7 && reg==4) {
		     rc = READPC_16(&s16);
		     if(rc) return cpu68k::I_BUS_ERROR;

		     if(DISASM)
			ASM_ARG1("#%u",(unsigned)s16);

		  } else {
		     /* EA, all (0xfff) */
		     ea_rc = calc_ea(ctx,2,mode,reg,&ea,0xff,1);
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
		     r16 = d16 - s16;
		     cpu->d[reg2] &= 0xffff0000;
		     cpu->d[reg2] |= (uint32_t)r16;
		  }
	       } else {	/* <ea> - Dn -> <ea> */
		  if(DISASM)
		     ASM_ARG1("D%d",reg2);

		  /* EA, only alterable memory (0x1fc) */
		  ea_rc = calc_ea(ctx,2,mode,reg,&ea,0x1fc,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     s16 = (uint16_t)(cpu->d[reg2] & 0xffff);
		     rc = READMEM_16_D(ea, &s16);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     r16 = d16 - s16;
		     rc = WRITEMEM_16_D(r16,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  if( ( !MSB16(s16) &&  MSB16(d16) && !MSB16(r16) ) ||
		      (  MSB16(s16) && !MSB16(d16) &&  MSB16(r16) ) )
		     cpu->sr |= 0x2;  /* V */

		  if( ( MSB16(s16) && !MSB16(d16) ) ||
		      ( MSB16(r16) && !MSB16(d16) ) ||
		      ( MSB16(s16) &&  MSB16(r16) ) )
		     cpu->sr |= 0x11;  /* XC */

		  if( MSB16(r16) )
		     cpu->sr |= 0x8;  /* N */

		  if( !r16 )
		     cpu->sr |= 0x4;  /* Z */
	       }

	    } else {
	       /* Long */
	       if(DISASM)
		  ASM_INSTR(".L");

	       if(op2==0) {	/* Dn - <ea> -> Dn */
		  if(mode==0) {
		     if(EMULATE)
			s32 = cpu->d[reg];

		     if(DISASM)
			ASM_ARG1("D%d",reg);

		  } else if(mode==1) {
		     if(EMULATE)
			s32 = cpu->a[reg];

		     if(DISASM)
			ASM_ARG1("A%d",reg);

		  } else if(mode==7 && reg==4) {
		     rc = READPC_32(&s32);
		     if(rc) return cpu68k::I_BUS_ERROR;;

		     if(DISASM)
			ASM_ARG1("#%u",(unsigned)s32);

		  } else {
		     /* EA, all (0xfff) */
		     ea_rc = calc_ea(ctx,4,mode,reg,&ea,0xfff,1);
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
		     r32 = d32 - s32;
		     cpu->d[reg2] = r32;
		  }

	       } else {	/* <ea> - Dn -> <ea> */
		  if(DISASM)
		     ASM_ARG1("D%d,",reg2);

		  /* EA, only alterable memory (0x1fc) */
		  ea_rc = calc_ea(ctx,4,mode,reg,&ea,0x1fc,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     s32 = cpu->d[reg2];
		     rc = READMEM_32_D(ea, &d32);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     r32 = d32 - s32;
		     rc = WRITEMEM_32_D(r32,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  if( ( !MSB32(s32) &&  MSB32(d32) && !MSB32(r32) ) ||
		      (  MSB32(s32) && !MSB32(d32) &&  MSB32(r32) ) )
		     cpu->sr |= 0x2;  /* V */

		  if( ( MSB32(s32) && !MSB32(d32) ) ||
		      ( MSB32(r32) && !MSB32(d32) ) ||
		      ( MSB32(s32) &&  MSB32(r32) ) )
		     cpu->sr |= 0x11;  /* XC */

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
 * Function:       i_subi
 * Instructions:   SUBI
 * Bit pattern(s): 0000 0100 00
 *                 0000 0100 01
 *                 0000 0100 10
 */
      cpu68k::returncode i_subi(context *ctx, uint16_t iw)
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
	    ASM_INSTR("SUBI");

	 if(EMULATE)
	    cpu->sr &= 0xffe0;  /* Clear XNZVC */

	 if(size==0) {
	    /* Byte */
	    rc = READPC_8(&s8);
	    if(rc) return cpu68k::I_BUS_ERROR;

	    if(DISASM) {
	       ASM_INSTR(".B");
	       ASM_ARG1("#%u",(unsigned)s8);
	    }

	    if(mode==0) {
	       if(DISASM)
		  ASM_ARG2("D%d",reg);

	       if(EMULATE) {
		  d8 = (uint8_t)(cpu->d[reg] & 0xff);
		  r8 = d8 - s8;
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
		  r8 = d8 - s8;
		  rc = WRITEMEM_8_D(r8,ea);
		  if(rc) return cpu68k::I_BUS_ERROR;
	       }
	    }

	    if(EMULATE) {
	       if( ( !MSB8(s8) &&  MSB8(d8) && !MSB8(r8) ) ||
	           (  MSB8(s8) && !MSB8(d8) &&  MSB8(r8) ) )
		  cpu->sr |= 0x2;  /* V */

	       if( ( MSB8(s8) && !MSB8(d8) ) ||
	           ( MSB8(r8) && !MSB8(d8) ) ||
	           ( MSB8(s8) &&  MSB8(r8) ) )
		  cpu->sr |= 0x11;  /* XC */

	       if( MSB8(r8) )
		  cpu->sr |= 0x8;  /* N */

	       if( !r8 )
		  cpu->sr |= 0x4;  /* Z */
	    }

	 } else if(size==1) {
	    /* Word */
	    rc = READPC_16(&s16);
	    if(rc) return cpu68k::I_BUS_ERROR;

	    if(DISASM) {
	       ASM_INSTR(".W");
	       ASM_ARG1("#%u",(unsigned)s16);
	    }

	    if(mode==0) {
	       if(DISASM)
		  ASM_ARG2("D%d",reg);

	       if(EMULATE) {
		  d16 = (uint16_t)(cpu->d[reg] & 0xffff);
		  r16 = d16 - s16;
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
		  r16 = d16 - s16;
		  rc = WRITEMEM_16_D(r16,ea);
		  if(rc) return cpu68k::I_BUS_ERROR;
	       }
	    }

	    if(EMULATE) {
	       if( ( !MSB16(s16) &&  MSB16(d16) && !MSB16(r16) ) ||
	           (  MSB16(s16) && !MSB16(d16) &&  MSB16(r16) ) )
		  cpu->sr |= 0x2;  /* V */

	       if( ( MSB16(s16) && !MSB16(d16) ) ||
	           ( MSB16(r16) && !MSB16(d16) ) ||
	           ( MSB16(s16) &&  MSB16(r16) ) )
		  cpu->sr |= 0x11;  /* XC */

	       if( MSB16(r16) )
		  cpu->sr |= 0x8;  /* N */

	       if( !r16 )
		  cpu->sr |= 0x4;  /* Z */
	    }

	 } else {
	    /* Long */
	    rc = READPC_32(&s32);
	    if(rc) return cpu68k::I_BUS_ERROR;

	    if(DISASM) {
	       ASM_INSTR(".L");
	       ASM_ARG1("#%u",(unsigned)s32);
	    }

	    if(mode==0) {
	       if(DISASM)
		  ASM_ARG2("D%d",reg);

	       if(EMULATE) {
		  d32 = cpu->d[reg];
		  r32 = d32 - s32;
		  cpu->d[reg] = r32;
	       }

	    } else {
	       /* EA, Dn + alterable memory (0x1fd) */
	       ea_rc = calc_ea(ctx,4,mode,reg,&ea,0x1fd,2);
	       if(ea_rc) return ea_rc;

	       if(EMULATE) {
		  rc = READMEM_32_D(ea, &d32);
		  if(rc) return cpu68k::I_BUS_ERROR;
		  r32 = d32 - s32;
		  rc = WRITEMEM_32_D(r32,ea);
		  if(rc) return cpu68k::I_BUS_ERROR;
	       }
	    }

	    if(EMULATE) {
	       if( ( !MSB32(s32) &&  MSB32(d32) && !MSB32(r32) ) ||
	           (  MSB32(s32) && !MSB32(d32) &&  MSB32(r32) ) )
		  cpu->sr |= 0x2;  /* V */

	       if( ( MSB32(s32) && !MSB32(d32) ) ||
	           ( MSB32(r32) && !MSB32(d32) ) ||
	           ( MSB32(s32) &&  MSB32(r32) ) )
		  cpu->sr |= 0x11;  /* XC */

	       if( MSB32(r32) )
		  cpu->sr |= 0x8;  /* N */

	       if( !r32 )
		  cpu->sr |= 0x4;  /* Z */
	    }
	 }
	 return cpu68k::I_OK;
      }

/*
 * Function:      i_subq
 * Instructions:   SUBQ
 * Bit pattern(s): 0101 xxx1 00
 *                 0101 xxx1 01
 *                 0101 xxx1 10
 */
      cpu68k::returncode i_subq(context *ctx, uint16_t iw)
      {
	 uint16_t size,mode,reg;
	 uint32_t data;
	 uint32_t ea;
	 uint8_t s8,d8,r8;
	 uint16_t s16,d16,r16=0;
	 uint32_t s32,d32,r32;
	 int rc;
	 cpu68k::returncode ea_rc;

	 cpu68k *cpu = &(ctx->cpu);

	 data = ( iw >> 9 ) & 0x7;
	 if(data==0) data=8;
	 size = ( iw >> 6 ) & 0x3;
	 mode = ( iw >> 3 ) & 0x7;
	 reg = iw & 0x7;

	 if(DISASM)
	    ASM_INSTR("SUBQ");

	 if(EMULATE)
	    if(mode!=1)
	       cpu->sr &= 0xffe0;  /* Clear XNZVC */

	 if(size==0) {
	    /* Byte */
	    if(DISASM) {
	       ASM_INSTR(".B");
	       ASM_ARG1("#%u",(unsigned)data);
	    }

	    s8 = (uint8_t) data;

	    if(mode==0) {
	       if(DISASM)
		  ASM_ARG2("D%d",reg);

	       if(EMULATE) {
		  d8 = (uint8_t)(cpu->d[reg] & 0xff);
		  r8 = d8 - s8;
		  cpu->d[reg] &= 0xffffff00;
		  cpu->d[reg] |= (uint32_t)r8;
	       }

	    } else {
	       /* EA, alterable memory (0x1fc) */
	       ea_rc = calc_ea(ctx,1,mode,reg,&ea,0x1fc,2);
	       if(ea_rc) return ea_rc;

	       if(EMULATE) {
		  rc = READMEM_8_D(ea, &d8);
		  if(rc) return cpu68k::I_BUS_ERROR;
		  r8 = d8 - s8;
		  rc = WRITEMEM_8_D(r8,ea);
		  if(rc) return cpu68k::I_BUS_ERROR;
	       }
	    }

	    if(EMULATE) {
	       if( ( !MSB8(s8) &&  MSB8(d8) && !MSB8(r8) ) ||
	           (  MSB8(s8) && !MSB8(d8) &&  MSB8(r8) ) )
		  cpu->sr |= 0x2;  /* V */

	       if( ( MSB8(s8) && !MSB8(d8) ) ||
	           ( MSB8(r8) && !MSB8(d8) ) ||
	           ( MSB8(s8) &&  MSB8(r8) ) )
		  cpu->sr |= 0x11;  /* XC */

	       if( MSB8(r8) )
		  cpu->sr |= 0x8;  /* N */

	       if( !r8 )
		  cpu->sr |= 0x4;  /* Z */
	    }

	 } else if(size==1) {
	    /* Word */
	    if(DISASM) {
	       ASM_INSTR(".W");
	       ASM_ARG1("#%u",(unsigned)data);
	    }

	    s16 = (uint16_t) data;

	    if(mode==0) {
	       if(DISASM)
		  ASM_ARG2("D%d",reg);

	       if(EMULATE) {
		  d16 = (uint16_t)(cpu->d[reg] & 0xffff);
		  r16 = d16 - s16;
		  cpu->d[reg] &= 0xffff0000;
		  cpu->d[reg] |= (uint32_t)r16;
	       }

	    } else if(mode==1) {
	       if(DISASM)
		  ASM_ARG2("A%d",reg);

	       if(EMULATE) {
		  s32 = (uint32_t)(int32_t)(int16_t)s16;
		  d32 = cpu->a[reg];
		  r32 = d32 - s32;
		  cpu->a[reg] = r32;

		  /* Increased SP? */
		  if(reg == 7 && (s32 & 0x80000000)) {
		     CHECK_SP();
		  }
	       }

	    } else {
	       /* EA, alterable memory (0x1fc) */
	       ea_rc = calc_ea(ctx,2,mode,reg,&ea,0x1fc,2);
	       if(ea_rc) return ea_rc;

	       if(EMULATE) {
		  rc = READMEM_16_D(ea, &d16);
		  if(rc) return cpu68k::I_BUS_ERROR;
		  r16 = d16 - s16;
		  rc = WRITEMEM_16_D(r16,ea);
		  if(rc) return cpu68k::I_BUS_ERROR;
	       }
	    }

	    if(EMULATE) {
	       if(mode!=1) {
		  if( ( !MSB16(s16) &&  MSB16(d16) && !MSB16(r16) ) ||
		      (  MSB16(s16) && !MSB16(d16) &&  MSB16(r16) ) )
		     cpu->sr |= 0x2;  /* V */

		  if( (  MSB16(s16) && !MSB16(d16) ) ||
		      (  MSB16(r16) && !MSB16(d16) ) ||
		      (  MSB16(s16) &&  MSB16(r16) ) )
		     cpu->sr |= 0x11;  /* XC */

		  if( MSB16(r16) )
		     cpu->sr |= 0x8;  /* N */

		  if( !r16 )
		     cpu->sr |= 0x4;  /* Z */
	       }
	    }

	 } else {
	    /* Long */
	    if(DISASM) {
	       ASM_INSTR(".L");
	       ASM_ARG1("#%u",(unsigned)data);
	    }

	    s32 = data;

	    if(mode==0) {
	       if(DISASM)
		  ASM_ARG2("D%d",reg);

	       if(EMULATE) {
		  d32 = cpu->d[reg];
		  r32 = d32 - s32;
		  cpu->d[reg] = r32;
	       }

	    } else if(mode==1) {
	       if(DISASM)
		  ASM_ARG2("A%d",reg);

	       if(EMULATE) {
		  d32 = cpu->a[reg];
		  r32 = d32 - s32;
		  cpu->a[reg] = r32;

		  /* Increased SP? */
		  if(reg == 7 && (s32 & 0x80000000)) {
		     CHECK_SP();
		  }
	       }
	    } else {
	       /* EA, alterable memory (0x1fc) */
	       ea_rc = calc_ea(ctx,4,mode,reg,&ea,0x1fc,2);
	       if(ea_rc) return ea_rc;

	       if(EMULATE) {
		  rc = READMEM_32_D(ea, &d32);
		  if(rc) return cpu68k::I_BUS_ERROR;
		  r32 = d32 - s32;
		  rc = WRITEMEM_32_D(r32,ea);
		  if(rc) return cpu68k::I_BUS_ERROR;
	       }
	    }

	    if(EMULATE) {
	       if(mode!=1) {
		  if( ( !MSB32(s32) &&  MSB32(d32) && !MSB32(r32) ) ||
		      (  MSB32(s32) && !MSB32(d32) &&  MSB32(r32) ) )
		     cpu->sr |= 0x2;  /* V */

		  if( ( MSB32(s32) && !MSB32(d32) ) ||
		      ( MSB32(r32) && !MSB32(d32) ) ||
		      ( MSB32(s32) &&  MSB32(r32) ) )
		     cpu->sr |= 0x11;  /* XC */

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
 * Function:       i_neg_x
 * Instructions:   NEG, NEGX
 * Bit pattern(s): 0100 0x00 00
 *                 0100 0x00 01
 *                 0100 0x00 10
 */
      cpu68k::returncode i_neg_x(context *ctx, uint16_t iw)
      {
	 uint16_t size,mode,reg,xflg;
	 uint32_t ea;
	 uint8_t d8,r8,x8;
	 uint16_t d16,r16,x16;
	 uint32_t d32,r32,x32;
	 int rc;
	 cpu68k::returncode ea_rc;

	 cpu68k *cpu = &(ctx->cpu);

	 xflg = ( iw >> 10 ) & 0x1;
	 size = ( iw >> 6 ) & 0x3;
	 mode = ( iw >> 3 ) & 0x7;
	 reg = iw & 0x7;

	 if(xflg==1) {
	    /* NEG */
	    if(DISASM)
	       ASM_INSTR("NEG");

	    if(EMULATE)
	       cpu->sr &= 0xffe0;  /* Clear XNZVC */

	    if(size==0) {
	       /* Byte */
	       if(DISASM)
		  ASM_INSTR(".B");

	       if(mode==0) {
		  if(DISASM)
		     ASM_ARG1("D%d",reg);

		  if(EMULATE) {
		     d8 = (uint8_t)(cpu->d[reg] & 0xff);
		     r8 = - d8;
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
		     r8 = - d8;
		     rc = WRITEMEM_8_D(r8,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  if( MSB8(d8) && MSB8(r8) )
		     cpu->sr |= 0x2;  /* V */

		  if( MSB8(d8) || MSB8(r8) )
		     cpu->sr |= 0x11;  /* XC */

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
		     r16 = - d16;
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
		     r16 = - d16;
		     rc = WRITEMEM_16_D(r16,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  if( MSB16(d16) && MSB16(r16) )
		     cpu->sr |= 0x2;  /* V */

		  if( MSB16(d16) || MSB16(r16) )
		     cpu->sr |= 0x11;  /* XC */

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
		     r32 = - d32;
		     cpu->d[reg] = r32;
		  }

	       } else {
		  /* EA, Dn + alterable memory (0x1fd) */
		  ea_rc = calc_ea(ctx,4,mode,reg,&ea,0x1fd,1);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     rc = READMEM_32_D(ea, &d32);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     r32 = - d32;
		     rc = WRITEMEM_32_D(r32,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {

		  if( MSB32(d32) && MSB32(r32) )
		     cpu->sr |= 0x2;  /* V */

		  if( MSB32(d32) || MSB32(r32) )
		     cpu->sr |= 0x11;  /* XC */

		  if( MSB32(r32) )
		     cpu->sr |= 0x8;  /* N */

		  if( !r32 )
		     cpu->sr |= 0x4;  /* Z */
	       }
	    }

	 } else {
	    /* NEGX */
	    if(DISASM)
	       ASM_INSTR("NEGX");

	    if(size==0) {
	       /* Byte */
	       if(DISASM)
		  ASM_INSTR(".B");

	       if(mode==0) {	/* Dn */
		  if(DISASM)
		     ASM_ARG1("D%d",reg);

		  if(EMULATE) {
		     d8 = (uint8_t)(cpu->d[reg] & 0xff);
		     x8 = (cpu->sr&0x10 ? 1 : 0);
		     r8 = 0 - d8 - x8;
		     cpu->d[reg] &= 0xffffff00;
		     cpu->d[reg] |= (uint32_t)r8;
		  }
	       } else {
		  /* EA, Dn + alterable (0x1fb) */
		  ea_rc = calc_ea(ctx,1,mode,reg,&ea,0x1fb,1);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     rc = READMEM_8_D(ea, &d8);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     x8 = (cpu->sr&0x10 ? 1 : 0);
		     r8 = 0 - d8 - x8;
		     rc = WRITEMEM_8_D(r8,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  cpu->sr &= 0xffe4;  /* Clear XNVC */

		  if( MSB8(d8) && MSB8(r8) )
		     cpu->sr |= 0x2;  /* V */

		  if( MSB8(d8) || MSB8(r8) )
		     cpu->sr |= 0x11;  /* XC */

		  if( MSB8(r8) )
		     cpu->sr |= 0x8;  /* N */

		  if( r8 )
		     cpu->sr &= 0xfffb;  /* !Z */
	       }

	    } else if(size==1) {
	       /* Word */
	       if(DISASM)
		  ASM_INSTR(".W");

	       if(mode==0) {	/* Dn */
		  if(DISASM)
		     ASM_ARG1("D%d",reg);

		  if(EMULATE) {
		     d16 = (uint16_t)(cpu->d[reg] & 0xffff);
		     x16 = (cpu->sr&0x10 ? 1 : 0);
		     r16 = 0 - d16 - x16;
		     cpu->d[reg] &= 0xffff0000;
		     cpu->d[reg] |= (uint32_t)r16;
		  }

	       } else {
		  /* EA, Dn + alterable (0x1fb) */
		  ea_rc = calc_ea(ctx,2,mode,reg,&ea,0x1fb,1);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     rc = READMEM_16_D(ea, &d16);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     x16 = (cpu->sr&0x10 ? 1 : 0);
		     r16 = 0 - d16 - x16;
		     rc = WRITEMEM_16_D(r16,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  cpu->sr &= 0xffe4;  /* Clear XNVC */

		  if( MSB16(d16) && MSB16(r16) )
		     cpu->sr |= 0x2;  /* V */

		  if( MSB16(d16) || MSB16(r16) )
		     cpu->sr |= 0x11;  /* XC */

		  if( MSB16(r16) )
		     cpu->sr |= 0x8;  /* N */

		  if( r16 )
		     cpu->sr &= 0xfffb;  /* !Z */
	       }

	    } else if(size==2) {
	       /* Long */
	       if(DISASM)
		  ASM_INSTR(".L");

	       if(mode==0) {	/* Dn */
		  if(DISASM)
		     ASM_ARG1("D%d",reg);

		  if(EMULATE) {
		     d32 = cpu->d[reg];
		     x32 = (cpu->sr&0x10 ? 1 : 0);
		     r32 = 0 - d32 - x32;
		     cpu->d[reg] = r32;
		  }

	       } else {
		  /* EA, Dn + alterable (0x1fb) */
		  ea_rc = calc_ea(ctx,4,mode,reg,&ea,0x1fb,1);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     rc = READMEM_32_D(ea, &d32);
		     if(rc) return cpu68k::I_BUS_ERROR;
		     x32 = (cpu->sr&0x10 ? 1 : 0);
		     r32 = 0 - d32 - x32;
		     rc = WRITEMEM_32_D(r32,ea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  cpu->sr &= 0xffe4;  /* Clear XNVC */

		  if( MSB32(d32) && MSB32(r32) )
		     cpu->sr |= 0x2;  /* V */

		  if( MSB32(d32) || MSB32(r32) )
		     cpu->sr |= 0x11;  /* XC */

		  if( MSB32(r32) )
		     cpu->sr |= 0x8;  /* N */

		  if( r32 )
		     cpu->sr &= 0xfffb;  /* !Z */
	       }
	    }
	 }
	 return cpu68k::I_OK;
      }

   } // namespace cpu_68k
} // namespace emumiga
