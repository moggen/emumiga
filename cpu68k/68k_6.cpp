/*
68k_6.cpp - M68K emulation. Instructions.

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
 * Function:       i_jsr_jmp
 * Instructions:   JSR, JMP
 * Bit pattern(s): 0100 1110 1x
 */
      cpu68k::returncode i_jsr_jmp(context *ctx, uint16_t iw)
      {
	 uint16_t type,mode,reg;
	 uint32_t ea;
	 int rc;
	 cpu68k::returncode ea_rc;

	 cpu68k *cpu = &(ctx->cpu);

	 type = ( iw >> 6 ) & 0x1;
	 mode = ( iw >> 3 ) & 0x7;
	 reg = iw & 0x7;

	 if(DISASM) {
	    if(type==0)
	       ASM_INSTR("JSR");
	    else
	       ASM_INSTR("JMP");
	 }

	 /* EA, all but Dn,An,(An)+,-(An),#<data> (0x7e4) */
	 ea_rc = calc_ea(ctx,4,mode,reg,&ea,0x7e4,1);
	 if(ea_rc) return ea_rc;

	 if(EMULATE) {
	    if(type==0) {		/* JSR */
	       cpu->a[7] -= 4;
	       rc = WRITEMEM_32_D(cpu->pc,cpu->a[7]);
	       if(rc) return cpu68k::I_BUS_ERROR;
	    }

	    cpu->pc = ea;
	 }

	 return cpu68k::I_OK;
      }

/*
 * Function:       i_bra_bsr_bcc
 * Instructions:   BRA, BSR, Bcc
 * Bit pattern(s): 0110 xxxx xx
 */
      cpu68k::returncode i_bra_bsr_bcc(context *ctx, uint16_t iw)
      {
	 uint16_t cond,disp16;
	 uint8_t disp8;
	 uint32_t disp32;
	 int val;
	 int rc;

	 cpu68k *cpu = &(ctx->cpu);

	 cond = ( iw >> 8 ) & 0xf;
	 disp8 = (uint8_t)( iw & 0xff );

	 if(DISASM) {
	    switch(cond) {
	       case 0:  ASM_INSTR("BRA"); break;
	       case 1:  ASM_INSTR("BSR"); break;
	       case 2:  ASM_INSTR("BHI"); break;
	       case 3:  ASM_INSTR("BLS"); break;
	       case 4:  ASM_INSTR("BCC"); break;
	       case 5:  ASM_INSTR("BCS"); break;
	       case 6:  ASM_INSTR("BNE"); break;
	       case 7:  ASM_INSTR("BEQ"); break;
	       case 8:  ASM_INSTR("BVC"); break;
	       case 9:  ASM_INSTR("BVS"); break;
	       case 10: ASM_INSTR("BPL"); break;
	       case 11: ASM_INSTR("BMI"); break;
	       case 12: ASM_INSTR("BGE"); break;
	       case 13: ASM_INSTR("BLT"); break;
	       case 14: ASM_INSTR("BGT"); break;
	       default: ASM_INSTR("BLE"); break;
	    }
	 }

	 if(disp8 == 0) {
	    /* 16 bit, Bxx.W */
	    if(DISASM)
	       ASM_INSTR(".W");

	    rc = READPC_16(&disp16);
	    if(rc) return cpu68k::I_BUS_ERROR;

	    disp32 = (uint32_t)(int32_t)(int16_t)disp16 - 2;

	 } else if(disp8 == 0xff) {
	    /* 32 bit, Bxx.L */
	    if(DISASM)
	       ASM_INSTR(".L");

	    /*
	    if( !cpu68k::IS_TYPE(cpu,CPU_TYPE_020) &&
	    !cpu68k::IS_TYPE(cpu,CPU_TYPE_030) &&
	    !cpu68k::IS_TYPE(cpu,CPU_TYPE_040) &&
	    !cpu68k::IS_TYPE(cpu,CPU_TYPE_060)    ) {

	       if(cpu68k::IS_TYPE(cpu,CPU_TYPE_AUTO)) {
		  cpu->type |= CPU_TYPE_020;
	       } else {
		  return cpu68k::I_ILLEGAL_FOR_CPU;
	       }
	    }
	    */

	    rc = READPC_32(&disp32);
	    if(rc) return cpu68k::I_BUS_ERROR;

	    disp32 -= 4;

	 } else {
	    /* 8 bit, Bxx.B */
	    if(DISASM)
	       ASM_INSTR(".B");

	    disp32 = (uint32_t)(int32_t)(int8_t)disp8;
	 }

	 if(DISASM)
	    ASM_ARG1("0x%X",(unsigned)(cpu->pc+disp32));

	 if(EMULATE) {
	    val=0;
	    switch(cond) {
	       case 0:  /* BRA */
		  val = 1;
		  break;

	       case 1:  /* BSR */
		  cpu->a[7] -= 4;
		  rc = WRITEMEM_32_D(cpu->pc,cpu->a[7]);
		  if(rc) return cpu68k::I_BUS_ERROR;
		  val = 1;
		  break;

	       case 2:  /* BHI: !C & !Z = !(C | Z) */
		  val = !(cpu->sr & 0x05);
		  break;

	       case 3:  /* BLS: C | Z */
		  val = (cpu->sr & 0x05);
		  break;

	       case 4:  /* BCC: !C */
		  val = !(cpu->sr & 0x01);
		  break;

	       case 5:  /* BCS: C */
		  val = (cpu->sr & 0x01);
		  break;

	       case 6:  /* BNE: !Z */
		  val = !(cpu->sr & 0x04);
		  break;

	       case 7:  /* BEQ: Z */
		  val = (cpu->sr & 0x04);
		  break;

	       case 8:  /* BVC: !V */
		  val = !(cpu->sr & 0x02);
		  break;

	       case 9:  /* BVS: V */
		  val = (cpu->sr & 0x02);
		  break;

	       case 10: /* BPL: !N */
		  val = !(cpu->sr & 0x08);
		  break;

	       case 11: /* BMI: N */
		  val = (cpu->sr & 0x08);
		  break;

	       case 12: /* BGE: N & V | !N & !V */
		  val = (cpu->sr & 0x0a) == 0x0a ||
		     (cpu->sr & 0x0a) == 0x00;
		  break;

	       case 13: /* BLT: N & !V | !N & V */
		  val = (cpu->sr & 0x0a) == 0x08 ||
		     (cpu->sr & 0x0a) == 0x02;
		  break;

	       case 14: /* BGT: N & V & !Z | !N & !V & !Z */
		  val = (cpu->sr & 0x0e) == 0x0a ||
		     (cpu->sr & 0x0e) == 0x00;
		  break;

	       default: /* BLE: Z | N & !V | !N & V */
		  val = (cpu->sr & 0x04) ||
		     (cpu->sr & 0x0a) == 0x08 ||
		     (cpu->sr & 0x0a) == 0x02;
		  break;
	    }
	    if(val) {
	       cpu->pc += disp32;
	    }
	 }

	 return cpu68k::I_OK;
      }

/*
 * Function:       i_db_trap_s_cc
 * Instructions:   DBcc, TRAPcc, Scc
 * Bit pattern(s): 0101 xxxx 11
 */
      cpu68k::returncode i_db_trap_s_cc(context *ctx, uint16_t iw)
      {
	 uint16_t cond,mode,reg;
	 uint16_t disp16,s16,data16;
	 uint32_t disp32,ea,data32;
	 int val=0;
	 int rc;
	 cpu68k::returncode ea_rc;

	 cpu68k *cpu = &(ctx->cpu);

	 cond = ( iw >> 8 ) & 0xf;
	 mode = ( iw >> 3 ) & 0x7;
	 reg = iw & 0x7;

	 if(DISASM) {
	    if(mode==1) {
	       switch(cond) {
		  case 0:  ASM_INSTR("DBT");  break;
		  case 1:  ASM_INSTR("DBF");  break;
		  case 2:  ASM_INSTR("DBHI"); break;
		  case 3:  ASM_INSTR("DBLS"); break;
		  case 4:  ASM_INSTR("DBCC"); break;
		  case 5:  ASM_INSTR("DBCS"); break;
		  case 6:  ASM_INSTR("DBNE"); break;
		  case 7:  ASM_INSTR("DBEQ"); break;
		  case 8:  ASM_INSTR("DBVC"); break;
		  case 9:  ASM_INSTR("DBVS"); break;
		  case 10: ASM_INSTR("DBPL"); break;
		  case 11: ASM_INSTR("DBMI"); break;
		  case 12: ASM_INSTR("DBGE"); break;
		  case 13: ASM_INSTR("DBLT"); break;
		  case 14: ASM_INSTR("DBGT"); break;
		  default: ASM_INSTR("DBLE"); break;
	       }

	    } else if(mode==7 && reg>1 && reg<5) {
	       switch(cond) {
		  case 0:  ASM_INSTR("TRAPT");  break;
		  case 1:  ASM_INSTR("TRAPF");  break;
		  case 2:  ASM_INSTR("TRAPHI"); break;
		  case 3:  ASM_INSTR("TRAPLS"); break;
		  case 4:  ASM_INSTR("TRAPCC"); break;
		  case 5:  ASM_INSTR("TRAPCS"); break;
		  case 6:  ASM_INSTR("TRAPNE"); break;
		  case 7:  ASM_INSTR("TRAPEQ"); break;
		  case 8:  ASM_INSTR("TRAPVC"); break;
		  case 9:  ASM_INSTR("TRAPVS"); break;
		  case 10: ASM_INSTR("TRAPPL"); break;
		  case 11: ASM_INSTR("TRAPMI"); break;
		  case 12: ASM_INSTR("TRAPGE"); break;
		  case 13: ASM_INSTR("TRAPLT"); break;
		  case 14: ASM_INSTR("TRAPGT"); break;
		  default: ASM_INSTR("TRAPLE"); break;
	       }

	    } else {
	       switch(cond) {
		  case 0:  ASM_INSTR("ST");  break;
		  case 1:  ASM_INSTR("SF");  break;
		  case 2:  ASM_INSTR("SHI"); break;
		  case 3:  ASM_INSTR("SLS"); break;
		  case 4:  ASM_INSTR("SCC"); break;
		  case 5:  ASM_INSTR("SCS"); break;
		  case 6:  ASM_INSTR("SNE"); break;
		  case 7:  ASM_INSTR("SEQ"); break;
		  case 8:  ASM_INSTR("SVC"); break;
		  case 9:  ASM_INSTR("SVS"); break;
		  case 10: ASM_INSTR("SPL"); break;
		  case 11: ASM_INSTR("SMI"); break;
		  case 12: ASM_INSTR("SGE"); break;
		  case 13: ASM_INSTR("SLT"); break;
		  case 14: ASM_INSTR("SGT"); break;
		  default: ASM_INSTR("SLE"); break;
	       }
	    }
	 }

	 if(EMULATE) {
	    switch(cond) {
	       case 0:  /* DBT, ST, TRAPT: True */
		  val = 1;
		  break;

	       case 1:  /* DBF, SF, TRAPF: False */
		  val = 0;
		  break;

	       case 2:  /* DBHI, SHI, TRAPHI: !C & !Z = !(C | Z) */
		  val = !(cpu->sr & 0x05);
		  break;

	       case 3:  /* DBLS, SLS, TRAPLS: C | Z */
		  val = (cpu->sr & 0x05);
		  break;

	       case 4:  /* DBCC, SCC, TRAPCC: !C */
		  val = !(cpu->sr & 0x01);
		  break;

	       case 5:  /* DBCS, SCS, TRAPCS: C */
		  val = (cpu->sr & 0x01);
		  break;

	       case 6:  /* DBNE, SNE, TRAPNE: !Z */
		  val = !(cpu->sr & 0x04);
		  break;

	       case 7:  /* DBEQ, SEQ, TRAPEQ: Z */
		  val = (cpu->sr & 0x04);
		  break;

	       case 8:  /* DBVC, SVC, TRAPVC: !V */
		  val = !(cpu->sr & 0x02);
		  break;

	       case 9:  /* DBVS, SVS, TRAPVS: V */
		  val = (cpu->sr & 0x02);
		  break;

	       case 10: /* DBPL, SPL, TRAPPL: !N */
		  val = !(cpu->sr & 0x08);
		  break;

	       case 11: /* DBMI, SMI, TRAPMI: N */
		  val = (cpu->sr & 0x08);
		  break;

	       case 12: /* DBGE, SGE, TRAPGE: N & V | !N & !V */
		  val = (cpu->sr & 0x0a) == 0x0a ||
		     (cpu->sr & 0x0a) == 0x00;
		  break;

	       case 13: /* DBLT, SLT, TRAPLT: N & !V | !N & V */
		  val = (cpu->sr & 0x0a) == 0x08 ||
		     (cpu->sr & 0x0a) == 0x02;
		  break;

	       case 14: /* DBGT, SGT, TRAPGT: N & V & !Z | !N & !V & !Z */
		  val = (cpu->sr & 0x0e) == 0x0a ||
		     (cpu->sr & 0x0e) == 0x00;
		  break;

	       default: /* DBLE, SLE, TRAPLE: Z | N & !V | !N & V */
		  val = (cpu->sr & 0x04) ||
		     (cpu->sr & 0x0a) == 0x08 ||
		     (cpu->sr & 0x0a) == 0x02;
		  break;
	    }
	 }

	 if(mode==1) {
	    /* DBcc */
	    rc = READPC_16(&disp16);
	    if(rc) return cpu68k::I_BUS_ERROR;

	    disp32 = (uint32_t)(int32_t)(int16_t)disp16 - 2;

	    if(DISASM)
	       ASM_ARG1("0x%X",(unsigned)(cpu->pc+disp32));

	    if(EMULATE) {
	       if(!val) {
		  s16 = (uint16_t)(cpu->d[reg] & 0xffff);
		  s16--;
		  cpu->d[reg] &= 0xffff0000;
		  cpu->d[reg] |= (uint32_t)s16;

		  if( s16 != 0xffff )
		     cpu->pc += disp32;
	       }
	    }

	 } else if(mode==7 && reg==2) {
	    /* TRAPcc.W */
	    rc = READPC_16(&data16);
	    if(rc) return cpu68k::I_BUS_ERROR;

	    if(DISASM) {
	       ASM_INSTR(".W");
	       ASM_ARG1("#%u",(unsigned)data16);
	    }
	    return cpu68k::I_UNSUPPORTED;

	 } else if(mode==7 && reg==3) {
	    /* TRAPcc.L */
	    rc = READPC_32(&data32);
	    if(rc) return cpu68k::I_BUS_ERROR;

	    if(DISASM) {
	       ASM_INSTR(".L");
	       ASM_ARG1("#%u",(unsigned)data32);
	    }
	    return cpu68k::I_UNSUPPORTED;

	 } else if(mode==7 && reg==4) {
	    /* TRAPcc */
	    return cpu68k::I_UNSUPPORTED;

	 } else {
	    /* Scc */
	    if(mode==0) {
	       if(DISASM)
		  ASM_ARG1("D%d",reg);

	       if(EMULATE) {
		  cpu->d[reg] &= 0xffffff00;
		  cpu->d[reg] |= ( val ? 0xff : 0 );
	       }

	    } else {
	       /* EA, Dn + alterable data (0x1fd) */
	       ea_rc = calc_ea(ctx,1,mode,reg,&ea,0x1fd,1);
	       if(ea_rc) return ea_rc;

	       if(EMULATE) {
		  rc = WRITEMEM_8_D( ( val ? 0xff : 0 ), ea );
		  if(rc) return cpu68k::I_BUS_ERROR;
	       }
	    }
	 }
	 return cpu68k::I_OK;
      }

/*
 * Function:       i_callm_rtm
 * Instructions:   CALLM, RTM
 * Bit pattern(s): 0000 0110 11
 */
      cpu68k::returncode i_callm_rtm(context *ctx, uint16_t iw __attribute__((unused)))
      {
	 cpu68k *cpu = &(ctx->cpu);

	 if(DISASM)
	    ASM_INSTR("CALLM/RTM");

	 return cpu68k::I_UNSUPPORTED;
      }

   } // namespace cpu_68k
} // namespace emumiga
