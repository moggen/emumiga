/*
68k_9.cpp - M68K emulation. Instructions.

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
 * Function:       i_lea_extb
 * Instructions:   LEA, EXTB
 * Bit pattern(s): 0100 xxx1 11
 */
      cpu68k::returncode i_lea_extb(context *ctx, uint16_t iw)
      {
	 uint16_t mode,reg,reg2;
	 uint8_t r8;
	 uint32_t ea;
	 cpu68k::returncode ea_rc;

	 cpu68k *cpu = &(ctx->cpu);

	 reg2 = ( iw >> 9 ) & 0x7;
	 mode = ( iw >> 3 ) & 0x7;
	 reg = iw & 0x7;

	 if(mode==0) {
	    /* EXTB */
	    if(reg2!=4) {
	       if(DISASM)
		  ASM_INSTR("(ILLEGAL)");

	       return cpu68k::I_ILLEGAL;
	    }

	    if(DISASM) {
	       ASM_INSTR("EXTB.L");
	       ASM_ARG1("D%d",reg);
	    }

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

	    if(EMULATE) {
	       cpu->sr &= 0xfff0;  /* Clear NZVC */

	       r8 = (uint8_t)(cpu->d[reg] & 0xff);
	       cpu->d[reg] = (uint32_t)(int32_t)(int8_t)r8;

	       if( MSB8(r8) )
		  cpu->sr |= 0x8;  /* N */

	       if( !r8 )
		  cpu->sr |= 0x4;  /* Z */
	    }

	 } else {
	    /* LEA */
	    if(DISASM) {
	       ASM_INSTR("LEA");
	       ASM_ARG2("A%d",reg2);
	    }

	    /* EA, all but Dn,An,(An)+,-(An),#<data> (0x7e4) */
	    ea_rc = calc_ea(ctx,4,mode,reg,&ea,0x7e4,1);
	    if(ea_rc) return ea_rc;

	    if(EMULATE) {
	       cpu->a[reg2] = ea;

	       /* Changed SP? */
	       if(reg2==7) {
		  CHECK_SP();
	       }
	    }
	 }
	 return cpu68k::I_OK;
      }

/*
 * Function:       i_clr
 * Instructions:   CLR
 * Bit pattern(s): 0100 0010 00
 *                 0100 0010 01
 *                 0100 0010 10
 */
      cpu68k::returncode i_clr(context *ctx, uint16_t iw)
      {
	 uint16_t size,mode,reg;
	 uint32_t ea;
	 int rc;
	 cpu68k::returncode ea_rc;

	 cpu68k *cpu = &(ctx->cpu);

	 size = ( iw >> 6 ) & 0x3;
	 mode = ( iw >> 3 ) & 0x7;
	 reg = iw & 0x7;

	 if(EMULATE) {
	    cpu->sr &= 0xfff0;  /* Clear NZVC */
	    cpu->sr |= 0x0004;  /* Sätt Z */
	 }

	 if(DISASM)
	    ASM_INSTR("CLR");

	 if(size==0) {
	    /* Byte */
	    if(DISASM)
	       ASM_INSTR(".B");

	    if(mode==0) {
	       if(EMULATE)
		  cpu->d[reg] &= 0xffffff00;

	       if(DISASM)
		  ASM_ARG1("D%d",reg);

	    } else {
	       /* EA, Dn + alterable memory (0x1fd) */
	       ea_rc = calc_ea(ctx,1,mode,reg,&ea,0x1fd,1);
	       if(ea_rc) return ea_rc;

	       if(EMULATE) {
		  rc = WRITEMEM_8_D(0,ea);
		  if(rc) return cpu68k::I_BUS_ERROR;
	       }
	    }

	 } else if(size==1) {
	    /* Word */
	    if(DISASM)
	       ASM_INSTR(".W");

	    if(mode==0) {
	       if(EMULATE)
		  cpu->d[reg] &= 0xffff0000;

	       if(DISASM)
		  ASM_ARG1("D%d",reg);

	    } else {
	       /* EA, Dn + alterable memory (0x1fd) */
	       ea_rc = calc_ea(ctx,2,mode,reg,&ea,0x1fd,1);
	       if(ea_rc) return ea_rc;

	       if(EMULATE) {
		  rc = WRITEMEM_16_D(0,ea);
		  if(rc) return cpu68k::I_BUS_ERROR;
	       }
	    }

	 } else {
	    /* Long */
	    if(DISASM)
	       ASM_INSTR(".L");

	    if(mode==0) {
	       if(EMULATE)
		  cpu->d[reg] = 0;

	       if(DISASM)
		  ASM_ARG1("D%d",reg);

	    } else {
	       /* EA, Dn + alterable memory (0x1fd) */
	       ea_rc = calc_ea(ctx,4,mode,reg,&ea,0x1fd,1);
	       if(ea_rc) return ea_rc;

	       if(EMULATE) {
		  rc = WRITEMEM_32_D(0,ea);
		  if(rc) return cpu68k::I_BUS_ERROR;
	       }
	    }
	 }
	 return cpu68k::I_OK;
      }

/*
 * Function:       i_misc
 * Instructions:   TRAP, LINK, UNLK, MOVE_USP, RESET, NOP, STOP,
 *                 RTE, RTD, RTS, TRAPV, RTR, MOVEC
 * Bit pattern(s): 0100 1110 01
*/
      cpu68k::returncode i_misc(context *ctx, uint16_t iw)
      {
	 uint16_t upper,vector,reg,disp,ccr;
	 uint32_t disp32,tmp32;
	 int rc;

	 cpu68k *cpu = &(ctx->cpu);

	 upper = ( iw >> 4 ) & 0x3;
	 vector = iw & 0xf;
	 reg = iw & 0x7;

	 if(upper==0) {
	    /* TRAP */
	    if(DISASM)
	       ASM_INSTR("TRAP");

	    return cpu68k::I_UNSUPPORTED;

	 } else if(upper==1) {
	    /* LINK/UNLK */
	    if(iw & 0x8) {
	       /* UNLK */
	       if(DISASM) {
		  ASM_INSTR("UNLK");
		  ASM_ARG1("A%d",reg);
	       }

	       if(EMULATE) {
		  cpu->a[7] = cpu->a[reg];
		  rc = READMEM_32_D(cpu->a[7], &tmp32);
		  if(rc) return cpu68k::I_BUS_ERROR;
		  cpu->a[reg] = tmp32;
		  cpu->a[7] += 4;

		  /* SP is increased */
		  CHECK_SP();
	       }

	    } else {
	       /* LINK */
	       if(DISASM) {
		  ASM_INSTR("LINK");
		  ASM_ARG1("A%d",reg);
	       }

	       rc = READPC_16(&disp);
	       if(rc) return cpu68k::I_BUS_ERROR;

	       disp32 = (uint32_t)(int32_t)(int16_t)disp;

	       if(DISASM)
		  ASM_ARG2("#%u",(unsigned)disp32);

	       if(EMULATE) {
		  cpu->a[7] -= 4;
		  rc = WRITEMEM_32_D(cpu->a[reg],cpu->a[7]);
		  if(rc) return cpu68k::I_BUS_ERROR;
		  cpu->a[reg] = cpu->a[7];
		  cpu->a[7] += disp32;
	       }
	    }

	 } else if(upper==2) {
	    /* MOVE_USP */
	    if(DISASM)
	       ASM_INSTR("MOVE_USP");

	    return cpu68k::I_UNSUPPORTED;

	 } else {
	    if(vector==0) {
	       /* RESET */
	       if(DISASM)
		  ASM_INSTR("RESET");

	       return cpu68k::I_UNSUPPORTED;

	    } else if(vector==1) {
	       /* NOP */
	       if(DISASM)
		  ASM_INSTR("NOP");

	    } else if(vector==2) {
	       /* STOP */
	       if(DISASM)
		  ASM_INSTR("STOP");

	       return cpu68k::I_UNSUPPORTED;

	    } else if(vector==3) {
	       /* RTE */
	       if(DISASM)
		  ASM_INSTR("RTE");

	       return cpu68k::I_UNSUPPORTED;

	    } else if(vector==4) {
	       /* RTD */
	       if(DISASM)
		  ASM_INSTR("RTD");

	       return cpu68k::I_UNSUPPORTED;

	    } else if(vector==5) {
	       /* RTS */
	       if(DISASM)
		  ASM_INSTR("RTS");

	       if(EMULATE) {
		  rc = READMEM_32_D(cpu->a[7], &tmp32);
		  if(rc) return cpu68k::I_BUS_ERROR;
		  cpu->pc = tmp32;
		  cpu->a[7] += 4;

		  /* SP is increased, but it should be
		     safe not to clear mappings here */
		  /* emulator_sp_check(cpu); */
	       }

	    } else if(vector==6) {
	       /* TRAPV */
	       if(DISASM)
		  ASM_INSTR("TRAPV");

	       return cpu68k::I_UNSUPPORTED;

	    } else if(vector==7) {
	       /* RTR */
	       if(DISASM)
		  ASM_INSTR("RTR");

	       if(EMULATE) {
		  rc = READMEM_16_D(cpu->a[7], &ccr);
		  if(rc) return cpu68k::I_BUS_ERROR;
		  cpu->a[7] += 2;
		  ccr &= 0x1f;
		  cpu->sr &= 0xffe0;
		  cpu->sr |= ccr;

		  rc = READMEM_32_D(cpu->a[7], &tmp32);
		  if(rc) return cpu68k::I_BUS_ERROR;
		  cpu->pc = tmp32;
		  cpu->a[7] += 4;
	       }

	    } else if(vector==10 || vector==11) {
	       /* MOVEC */
	       if(DISASM)
		  ASM_INSTR("MOVEC");

	       return cpu68k::I_UNSUPPORTED;

	    } else {
	       if(DISASM)
		  ASM_INSTR("(ILLEGAL)");

	       return cpu68k::I_ILLEGAL;
	    }
	 }
	 return cpu68k::I_OK;
      }

/*
 * Function:       i_nbcd_linkl
 * Instructions:   NBCD, LINK_L
 * Bit pattern(s): 0100 1000 00
 */
      cpu68k::returncode i_nbcd_linkl(context *ctx, uint16_t iw __attribute__((unused)))
      {
	 cpu68k *cpu = &(ctx->cpu);
	 
	 if(DISASM)
	    ASM_INSTR("NBCD/LINK_L");

	 return cpu68k::I_UNSUPPORTED;
      }

/*
 * Function:       i_swap_bpkt_pea
 * Instructions:   SWAP, BPKT, PEA
 * Bit pattern(s): 0100 1000 01
 */
      cpu68k::returncode i_swap_bpkt_pea(context *ctx, uint16_t iw)
      {
	 uint16_t mode,reg;
	 uint32_t u32,l32;
	 uint32_t ea;
	 int rc;
	 cpu68k::returncode ea_rc;

	 cpu68k *cpu = &(ctx->cpu);

	 mode = ( iw >> 3 ) & 0x7;
	 reg = iw & 0x7;

	 if(mode==0) {
	    /* SWAP */

	    if(EMULATE)
	       cpu->sr &= 0xfff0;  /* Clear NZVC */

	    if(DISASM) {
	       ASM_INSTR("SWAP");
	       ASM_ARG1("D%d",reg);
	    }

	    if(EMULATE) {
	       u32 = cpu->d[reg];
	       l32 = u32 & 0xffff;
	       u32 = u32 >> 16;
	       u32 |= (l32 << 16);
	       cpu->d[reg] = u32;

	       if( MSB32(u32) )
		  cpu->sr |= 0x8;  /* N */

	       if( !u32 )
		  cpu->sr |= 0x4;  /* Z */
	    }

	 } else if(mode==1) {
	    /* BPKT */
	    if(DISASM)
	       ASM_INSTR("BPKT");

	    return cpu68k::I_UNSUPPORTED;

	 } else {
	    /* PEA */
	    if(DISASM)
	       ASM_INSTR("PEA");

	    /* EA, all but Dn,An,(An)+,-(An),#<data> (0x7e4) */
	    ea_rc = calc_ea(ctx,4,mode,reg,&ea,0x7e4,1);
	    if(ea_rc) return ea_rc;

	    if(EMULATE) {
	       cpu->a[7] -= 4;
	       rc = WRITEMEM_32_D(ea,cpu->a[7]);
	       if(rc) return cpu68k::I_BUS_ERROR;
	    }
	 }
	 return cpu68k::I_OK;
      }

   } // namespace cpu_68k
} // namespace emumiga
