/*
68k_8.cpp - M68K emulation. Instructions.

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
 * Function:       i_cmp_a
 * Instructions:   CMP, CMPA
 * Bit pattern(s): 1011 xxx0 xx
 *                 1011 xxx1 11
 */
      cpu68k::returncode i_cmp_a(context *ctx, uint16_t iw)
      {
	 uint16_t opmode,reg2,mode,reg;
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

	 if(EMULATE)
	    cpu->sr &= 0xfff0;  /* Clear NZVC */

	 if(DISASM) {
	    if(opmode>2)
	       ASM_INSTR("CMPA");
	    else
	       ASM_INSTR("CMP");
	 }

	 if(opmode==0) {
	    /* CMP.B */
	    if(DISASM)
	       ASM_INSTR(".B");

	    if(mode==0) {
	       if(DISASM)
		  ASM_ARG1("D%d",reg);

	       if(EMULATE)
		  s8 = (uint8_t)(cpu->d[reg] & 0xff);

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

	 } else if(opmode==1) {
	    /* CMP.W */
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
		  ASM_ARG1("#%d",(unsigned)s16);

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
	       ASM_ARG2("D%d",reg2);

	    if(EMULATE) {
	       d16 = (uint16_t)(cpu->d[reg2] & 0xffff);
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

	 } else if(opmode==2) {
	    /* CMP.L */
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
	       ASM_ARG2("D%d",reg2);

	    if(EMULATE) {
	       d32 = cpu->d[reg2];
	       r32 = d32 - s32;

	       if( ( !MSB32(s32) &&  MSB32(d32) && !MSB32(r32) ) ||
	           (  MSB32(s32) && !MSB32(d32) &&  MSB32(r32) ) )
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

	 } else if(opmode==3) {
	    /* CMPA.W */
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

	       if( ( !MSB32(s32) &&  MSB32(d32) && !MSB32(r32) ) ||
	           (  MSB32(s32) && !MSB32(d32) &&  MSB32(r32) ) )
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

	 } else {
	    /* CMPA.L */
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
	       rc = READPC_32( &s32);
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

	       if( ( !MSB32(s32) &&  MSB32(d32) && !MSB32(r32) ) ||
	           (  MSB32(s32) && !MSB32(d32) &&  MSB32(r32) ) )
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
	 return cpu68k::I_OK;
      }

/*
 * Function:       i_cmpi
 * Instructions:   CMPI
 * Bit pattern(s): 0000 1100 00
 *                 0000 1100 01
 *                 0000 1100 10
 */
      cpu68k::returncode i_cmpi(context *ctx, uint16_t iw)
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

	 if(EMULATE)
	    cpu->sr &= 0xfff0;  /* Clear NZVC */

	 if(DISASM)
	    ASM_INSTR("CMPI");

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
		  r8 = d8 - s8;
	       }

	    } else {
	       /* EA, all but #<data> and An (0x7fd) */
	       ea_rc = calc_ea(ctx,1,mode,reg,&ea,0x7fd,2);
	       if(ea_rc) return ea_rc;

	       if(EMULATE) {
		  rc = READMEM_8_D(ea, &d8);
		  if(rc) return cpu68k::I_BUS_ERROR;
		  r8 = d8 - s8;
	       }
	    }

	    if(EMULATE) {
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
		  r16 = d16 - s16;
	       }

	    } else {
	       /* EA, all but #<data> and An (0x7fd) */
	       ea_rc = calc_ea(ctx,2,mode,reg,&ea,0x7fd,2);
	       if(ea_rc) return ea_rc;

	       if(EMULATE) {
		  rc = READMEM_16_D(ea, &d16);
		  if(rc) return cpu68k::I_BUS_ERROR;
		  r16 = d16 - s16;
	       }
	    }

	    if(EMULATE) {
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

	    rc = READPC_32(&s32);
	    if(rc) return cpu68k::I_BUS_ERROR;

	    if(DISASM)
	       ASM_ARG1("#%u",(unsigned)s32);

	    if(mode==0) {
	       if(DISASM)
		  ASM_ARG2("D%d",reg);

	       if(EMULATE) {
		  d32 = cpu->d[reg];
		  r32 = d32 - s32;
	       }

	    } else {
	       /* EA, all but #<data> and An (0x7fd) */
	       ea_rc = calc_ea(ctx,4,mode,reg,&ea,0x7fd,2);
	       if(ea_rc) return ea_rc;

	       if(EMULATE) {
		  rc = READMEM_32_D(ea, &d32);
		  if(rc) return cpu68k::I_BUS_ERROR;
		  r32 = d32 - s32;
	       }
	    }

	    if(EMULATE) {
	       if( ( !MSB32(s32) &&  MSB32(d32) && !MSB32(r32) ) ||
	           (  MSB32(s32) && !MSB32(d32) &&  MSB32(r32) ) )
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

	 return cpu68k::I_OK;
      }

/*
 * Function:       i_chk
 * Instructions:   CHK
 * Bit pattern(s): 0100 xxx1 x0
 */
      cpu68k::returncode i_chk(context *ctx, uint16_t iw __attribute__((unused)))
      {
	 cpu68k *cpu = &(ctx->cpu);

	 if(DISASM)
	    ASM_INSTR("CHK");

	 return cpu68k::I_UNSUPPORTED;
      }

/*
 * Function:       i_cas_cas2
 * Instructions:   CAS, CAS2
 * Bit pattern(s): 0000 1010 11
 *                 0000 1100 11
 *                 0000 1110 11
 */
      cpu68k::returncode i_cas_cas2(context *ctx, uint16_t iw __attribute__((unused)))
      {
	 cpu68k *cpu = &(ctx->cpu);

	 if(DISASM)
	    ASM_INSTR("CAS/CAS2");

	 return cpu68k::I_UNSUPPORTED;
      }

/*
 * Function:       i_cmp2_chk2
 * Instructions:   CMP2, CHK2
 * Bit pattern(s): 0000 0000 11
 *                 0000 0010 11
 *                 0000 0100 11
 */
      cpu68k::returncode i_cmp2_chk2(context *ctx, uint16_t iw __attribute__((unused)))
      {
	 cpu68k *cpu = &(ctx->cpu);

	 if(DISASM)
	    ASM_INSTR("CMP2/CHK2");

	 return cpu68k::I_UNSUPPORTED;
      }

/*
 * Function:       i_tst_tas_ill
 * Instructions:   TST, TAS, ILLEGAL
 * Bit pattern(s): 0100 1010 xx
 */
      cpu68k::returncode i_tst_tas_ill(context *ctx, uint16_t iw)
      {
	 uint16_t size,mode,reg;
	 uint32_t ea;
	 uint8_t s8;
	 uint16_t s16;
	 uint32_t s32;
	 int rc;
	 cpu68k::returncode ea_rc;

	 cpu68k *cpu = &(ctx->cpu);

	 size = ( iw >> 6 ) & 0x3;
	 mode = ( iw >> 3 ) & 0x7;
	 reg = iw & 0x7;

	 if(EMULATE)
	    cpu->sr &= 0xfff0;  /* Clear NZVC */

	 if(DISASM) {
	    if(size==3) {
	       if(mode==7 && reg==4)
		  ASM_INSTR("ILLEGAL");
	       else
		  ASM_INSTR("TAS");
	    } else {
	       ASM_INSTR("TST");
	    }
	 }

	 if(size==0) {
	    /* TST.B */
	    if(DISASM)
	       ASM_INSTR(".B");

	    if(mode==0) {
	       if(DISASM)
		  ASM_ARG1("D%d",reg);

	       if(EMULATE)
		  s8 = (uint8_t)(cpu->d[reg] & 0xff);

	    } else if(mode==7 && reg==4) {

	       /*
	       if( !CPU_IS_TYPE(cpu,CPU_TYPE_020) &&
	       !CPU_IS_TYPE(cpu,CPU_TYPE_030) &&
	       !CPU_IS_TYPE(cpu,CPU_TYPE_040) &&
	       !CPU_IS_TYPE(cpu,CPU_TYPE_060)    ) {

		  if(CPU_IS_TYPE(cpu,CPU_TYPE_AUTO)) {
		     cpu->type |= CPU_TYPE_020;
		  } else {
		     return CPU_I_ILLEGAL_FOR_CPU;
		  }
	       }
	       */

	       rc = READPC_8(&s8);
	       if(rc) return cpu68k::I_BUS_ERROR;

	       if(DISASM)
		  ASM_ARG1("#%d",(unsigned)s8);

	    } else {

	       /*
	       if(mode==7 && (reg==2 || reg==3) ) {
		  if( !CPU_IS_TYPE(cpu,CPU_TYPE_020) &&
		  !CPU_IS_TYPE(cpu,CPU_TYPE_030) &&
		  !CPU_IS_TYPE(cpu,CPU_TYPE_040) &&
		  !CPU_IS_TYPE(cpu,CPU_TYPE_060)    ) {

		     if(CPU_IS_TYPE(cpu,CPU_TYPE_AUTO)) {
			cpu->type |= CPU_TYPE_020;
		     } else {
			return CPU_I_ILLEGAL_FOR_CPU;
		     }
		  }
	       }
	       */

	       /* EA, all but An (0xffd) */
	       ea_rc = calc_ea(ctx,1,mode,reg,&ea,0xffd,1);
	       if(ea_rc) return ea_rc;

	       if(EMULATE) {
		  rc = READMEM_8_D(ea, &s8);
		  if(rc) return cpu68k::I_BUS_ERROR;
	       }
	    }

	    if(EMULATE) {
	       if( MSB8(s8) )
		  cpu->sr |= 0x8;  /* N */

	       if( !s8 )
		  cpu->sr |= 0x4;  /* Z */
	    }

	 } else if(size==1) {
	    /* TST.W */
	    if(DISASM)
	       ASM_INSTR(".W");

	    if(mode==0) {
	       if(DISASM)
		  ASM_ARG1("D%d",reg);

	       if(EMULATE)
		  s16 = (uint16_t)(cpu->d[reg] & 0xffff);

	    } else if(mode==1) {

	       /*
	       if( !CPU_IS_TYPE(cpu,CPU_TYPE_020) &&
	       !CPU_IS_TYPE(cpu,CPU_TYPE_030) &&
	       !CPU_IS_TYPE(cpu,CPU_TYPE_040) &&
	       !CPU_IS_TYPE(cpu,CPU_TYPE_060)    ) {

		  if(CPU_IS_TYPE(cpu,CPU_TYPE_AUTO)) {
		     cpu->type |= CPU_TYPE_020;
		  } else {
		     return CPU_I_ILLEGAL_FOR_CPU;
		  }
	       }
	       */

	       if(DISASM)
		  ASM_ARG1("A%d",reg);

	       if(EMULATE)
		  s16 = (uint16_t)(cpu->a[reg] & 0xffff);

	    } else if(mode==7 && reg==4) {

	       /*
	       if( !CPU_IS_TYPE(cpu,CPU_TYPE_020) &&
	       !CPU_IS_TYPE(cpu,CPU_TYPE_030) &&
	       !CPU_IS_TYPE(cpu,CPU_TYPE_040) &&
	       !CPU_IS_TYPE(cpu,CPU_TYPE_060)    ) {

		  if(CPU_IS_TYPE(cpu,CPU_TYPE_AUTO)) {
		     cpu->type |= CPU_TYPE_020;
		  } else {
		     return CPU_I_ILLEGAL_FOR_CPU;
		  }
	       }
	       */

	       rc = READPC_16(&s16);
	       if(rc) return cpu68k::I_BUS_ERROR;

	       if(DISASM)
		  ASM_ARG1("#%u",(unsigned)s16);

	    } else {

	       /*
	       if(mode==7 && (reg==2 || reg==3) ) {
		  if( !CPU_IS_TYPE(cpu,CPU_TYPE_020) &&
		  !CPU_IS_TYPE(cpu,CPU_TYPE_030) &&
		  !CPU_IS_TYPE(cpu,CPU_TYPE_040) &&
		  !CPU_IS_TYPE(cpu,CPU_TYPE_060)    ) {

		     if(CPU_IS_TYPE(cpu,CPU_TYPE_AUTO)) {
			cpu->type |= CPU_TYPE_020;
		     } else {
			return CPU_I_ILLEGAL_FOR_CPU;
		     }
		  }
	       }
	       */

	       /* EA, all (0xfff) */
	       ea_rc = calc_ea(ctx,2,mode,reg,&ea,0xfff,1);
	       if(ea_rc) return ea_rc;

	       if(EMULATE) {
		  rc = READMEM_16_D(ea, &s16);
		  if(rc) return cpu68k::I_BUS_ERROR;
	       }
	    }

	    if(EMULATE) {
	       if( MSB16(s16) )
		  cpu->sr |= 0x8;  /* N */

	       if( !s16 )
		  cpu->sr |= 0x4;  /* Z */
	    }

	 } else if(size==2) {
	    /* TST.L */
	    if(DISASM)
	       ASM_INSTR(".L");

	    if(mode==0) {
	       if(DISASM)
		  ASM_ARG1("D%d",reg);

	       if(EMULATE)
		  s32 = cpu->d[reg];

	    } else if(mode==1) {

	       /*
	       if( !CPU_IS_TYPE(cpu,CPU_TYPE_020) &&
	       !CPU_IS_TYPE(cpu,CPU_TYPE_030) &&
	       !CPU_IS_TYPE(cpu,CPU_TYPE_040) &&
	       !CPU_IS_TYPE(cpu,CPU_TYPE_060)    ) {

		  if(CPU_IS_TYPE(cpu,CPU_TYPE_AUTO)) {
		     cpu->type |= CPU_TYPE_020;
		  } else {
		     return CPU_I_ILLEGAL_FOR_CPU;
		  }
	       }
	       */

	       if(DISASM)
		  ASM_ARG1("A%d",reg);

	       if(EMULATE)
		  s32 = cpu->a[reg];

	    } else if(mode==7 && reg==4) {

	       /*
	       if( !CPU_IS_TYPE(cpu,CPU_TYPE_020) &&
	       !CPU_IS_TYPE(cpu,CPU_TYPE_030) &&
	       !CPU_IS_TYPE(cpu,CPU_TYPE_040) &&
	       !CPU_IS_TYPE(cpu,CPU_TYPE_060)    ) {

		  if(CPU_IS_TYPE(cpu,CPU_TYPE_AUTO)) {
		     cpu->type |= CPU_TYPE_020;
		  } else {
		     return CPU_I_ILLEGAL_FOR_CPU;
		  }
	       }
	       */

	       rc = READPC_32(&s32);
	       if(rc) return cpu68k::I_BUS_ERROR;

	       if(DISASM)
		  ASM_ARG1("#%u",(unsigned)s32);

	    } else {

	       /*
	       if(mode==7 && (reg==2 || reg==3) ) {
		  if( !CPU_IS_TYPE(cpu,CPU_TYPE_020) &&
		  !CPU_IS_TYPE(cpu,CPU_TYPE_030) &&
		  !CPU_IS_TYPE(cpu,CPU_TYPE_040) &&
		  !CPU_IS_TYPE(cpu,CPU_TYPE_060)    ) {

		     if(CPU_IS_TYPE(cpu,CPU_TYPE_AUTO)) {
			cpu->type |= CPU_TYPE_020;
		     } else {
			return CPU_I_ILLEGAL_FOR_CPU;
		     }
		  }
	       }
	       */

	       /* EA, all (0xfff) */
	       ea_rc = calc_ea(ctx,4,mode,reg,&ea,0xfff,1);
	       if(ea_rc) return ea_rc;

	       if(EMULATE) {
		  rc = READMEM_32_D(ea, &s32);
		  if(rc) return cpu68k::I_BUS_ERROR;
	       }
	    }

	    if(EMULATE) {
	       if( MSB32(s32) )
		  cpu->sr |= 0x8;  /* N */

	       if( !s32 )
		  cpu->sr |= 0x4;  /* Z */
	    }

	 } else {
	    /* TAS/ILLEGAL */
	    if(mode==7 && reg==4)
	       /* ILLEGAL */
	       return cpu68k::I_ILLEGAL;

	    else
	       /* TAS */
	       return cpu68k::I_UNSUPPORTED;
	 }
	 return cpu68k::I_OK;
      }

   } // namespace cpu_68k
} // namespace emumiga
