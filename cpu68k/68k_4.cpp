/*
68k_4.cpp - M68K emulation. Instructions.

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
 * Function:       i_move
 * Instructions:   MOVE, MOVEA
 * Bit pattern(s): 0001 xxx0 00
 *                 0001 xxx0 1x
 *                 0001 xxx1 xx
 *                 001x xxxx xx 
 */
      cpu68k::returncode i_move(context *ctx, uint16_t iw)
      {
	 uint16_t size,dmode,dreg,smode,sreg;
	 uint32_t dea,sea;
	 uint8_t s8;
	 uint16_t s16;
	 uint32_t s32;
	 int rc;
	 cpu68k::returncode ea_rc;

	 cpu68k *cpu = &(ctx->cpu);

	 size = ( iw >> 12 ) & 0x3;
	 dreg = ( iw >> 9 ) & 0x7;
	 dmode = ( iw >> 6 ) & 0x7;
	 smode = ( iw >> 3 ) & 0x7;
	 sreg = iw & 0x7;

	 if(dmode!=1) {
	    /* MOVE */
	    if(DISASM)
	       ASM_INSTR("MOVE");

	    if(EMULATE)
	       cpu->sr &= 0xfff0;  /* Clear NZVC */

	    if(size==1) {
	       /* Byte */
	       if(DISASM)
		  ASM_INSTR(".B");

	       if(smode==0) {
		  if(EMULATE)
		     s8 = (uint8_t)(cpu->d[sreg] & 0xff);

		  if(DISASM)
		     ASM_ARG1("D%d",sreg);

	       } else if(smode==1) {
		  if(EMULATE)
		     s8 = (uint8_t)(cpu->a[sreg] & 0xff);

		  if(DISASM)
		     ASM_ARG1("A%d",sreg);

	       } else if(smode==7 && sreg==4) {
		  rc = READPC_8(&s8);
		  if(rc) return cpu68k::I_BUS_ERROR;

		  if(DISASM)
		     ASM_ARG1("#%u",(unsigned)s8);

	       } else {
		  /* EA, all (0xfff) */
		  ea_rc = calc_ea(ctx,1,smode,sreg,&sea,0xfff,1);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     rc = READMEM_8_D(sea, &s8);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(dmode==0) {
		  if(EMULATE) {
		     cpu->d[dreg] &= 0xffffff00;
		     cpu->d[dreg] |= (uint32_t)s8;
		  }

		  if(DISASM)
		     ASM_ARG2("D%d",dreg);

	       } else {
		  /* EA, Dn + alterable memory (0x1fd) */
		  ea_rc = calc_ea(ctx,1,dmode,dreg,&dea,0x1fd,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     rc = WRITEMEM_8_D(s8,dea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  if( MSB8(s8) )
		     cpu->sr |= 0x8;  /* N */

		  if( !s8 )
		     cpu->sr |= 0x4;  /* Z */
	       }

	    } else if(size==3) {
	       /* Word */
	       if(DISASM)
		  ASM_INSTR(".W");

	       if(smode==0) {
		  if(EMULATE)
		     s16 = (uint16_t)(cpu->d[sreg] & 0xffff);

		  if(DISASM)
		     ASM_ARG1("D%d",sreg);

	       } else if(smode==1) {
		  if(EMULATE)
		     s16 = (uint16_t)(cpu->a[sreg] & 0xffff);

		  if(DISASM)
		     ASM_ARG1("A%d",sreg);

	       } else if(smode==7 && sreg==4) {
		  rc = READPC_16(&s16);
		  if(rc) return cpu68k::I_BUS_ERROR;

		  if(DISASM)
		     ASM_ARG1("#%u",(unsigned)(int16_t)s16);

	       } else {
		  /* EA, all (0xfff) */
		  ea_rc = calc_ea(ctx,2,smode,sreg,&sea,0xfff,1);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     rc = READMEM_16_D(sea, &s16);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(dmode==0) {
		  if(EMULATE) {
		     cpu->d[dreg] &= 0xffff0000;
		     cpu->d[dreg] |= (uint32_t)s16;
		  }

		  if(DISASM)
		     ASM_ARG2("D%d",dreg);

	       } else {
		  /* EA, Dn + alterable memory (0x1fd) */
		  ea_rc = calc_ea(ctx,2,dmode,dreg,&dea,0x1fd,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     rc = WRITEMEM_16_D(s16,dea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  if( MSB16(s16) )
		     cpu->sr |= 0x8;  /* N */

		  if( !s16 )
		     cpu->sr |= 0x4;  /* Z */
	       }

	    } else {
	       /* Long */
	       if(DISASM)
		  ASM_INSTR(".L");

	       if(smode==0) {
		  if(EMULATE)
		     s32 = cpu->d[sreg];

		  if(DISASM)
		     ASM_ARG1("D%d",sreg);

	       } else if(smode==1) {
		  if(EMULATE)
		     s32 = cpu->a[sreg];

		  if(DISASM)
		     ASM_ARG1("A%d",sreg);

	       } else if(smode==7 && sreg==4) {
		  rc = READPC_32(&s32);
		  if(rc) return cpu68k::I_BUS_ERROR;

		  if(DISASM)
		     ASM_ARG1("#%u",(unsigned)s32);

	       } else {
		  /* EA, all (0xfff) */
		  ea_rc = calc_ea(ctx,4,smode,sreg,&sea,0xfff,1);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     rc = READMEM_32_D(sea, &s32);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(dmode==0) {
		  if(EMULATE)
		     cpu->d[dreg] = s32;

		  if(DISASM)
		     ASM_ARG2("D%d",dreg);

	       } else {
		  /* EA, Dn + alterable memory (0x1fd) */
		  ea_rc = calc_ea(ctx,4,dmode,dreg,&dea,0x1fd,2);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     rc = WRITEMEM_32_D(s32,dea);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(EMULATE) {
		  if( MSB32(s32) )
		     cpu->sr |= 0x8;  /* N */

		  if( !s32 )
		     cpu->sr |= 0x4;  /* Z */
	       }
	    }

	 } else {
	    /* MOVEA */
	    if(DISASM)
	       ASM_INSTR("MOVEA");

	    if(size==3) {
	       /* Word */
	       if(DISASM)
		  ASM_INSTR(".W");

	       if(smode==0) {
		  if(EMULATE)
		     s16 = (uint16_t)(cpu->d[sreg] & 0xffff);

		  if(DISASM)
		     ASM_ARG1("D%d",sreg);

	       } else if(smode==1) {
		  if(EMULATE)
		     s16 = (uint16_t)(cpu->a[sreg] & 0xffff);

		  if(DISASM)
		     ASM_ARG1("A%d",sreg);

	       } else if(smode==7 && sreg==4) {
		  rc = READPC_16(&s16);
		  if(rc) return cpu68k::I_BUS_ERROR;

		  if(DISASM)
		     ASM_ARG1("#%u",(unsigned)s16);

	       } else {
		  /* EA, all (0xfff) */
		  ea_rc = calc_ea(ctx,2,smode,sreg,&sea,0xfff,1);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     rc = READMEM_16_D(sea, &s16);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(DISASM)
		  ASM_ARG2("A%d",dreg);

	       if(EMULATE) {
		  cpu->a[dreg] = (uint32_t)(int32_t)(int16_t)s16;

		  /* Changed SP? */
		  if(dreg==7) {
		     CHECK_SP();
		  }
	       }

	    } else {
	       /* Long */
	       if(DISASM)
		  ASM_INSTR(".L");

	       if(smode==0) {
		  if(EMULATE)
		     s32 = cpu->d[sreg];

		  if(DISASM)
		     ASM_ARG1("D%d",sreg);

	       } else if(smode==1) {
		  if(EMULATE)
		     s32 = cpu->a[sreg];

		  if(DISASM)
		     ASM_ARG1("A%d",sreg);

	       } else if(smode==7 && sreg==4) {
		  rc = READPC_32(&s32);
		  if(rc) return cpu68k::I_BUS_ERROR;

		  if(DISASM)
		     ASM_ARG1("#%u",(unsigned)s32);

	       } else {
		  /* EA, all (0xfff) */
		  ea_rc = calc_ea(ctx,4,smode,sreg,&sea,0xfff,1);
		  if(ea_rc) return ea_rc;

		  if(EMULATE) {
		     rc = READMEM_32_D(sea, &s32);
		     if(rc) return cpu68k::I_BUS_ERROR;
		  }
	       }

	       if(DISASM)
		  ASM_ARG2("A%d",dreg);

	       if(EMULATE) {
		  cpu->a[dreg] = s32;

		  /* Changed SP? */
		  if(dreg==7) {
		     CHECK_SP();
		  }
	       }
	    }
	 }
	 return cpu68k::I_OK;
      }

/*
 * Function:       i_move_sr_ccr
 * Instructions:   MOVE_f_SR, MOVE_t_SR, MOVE_f_CCR, MOVE_t_CCR
 * Bit pattern(s): 0100 0xx0 11
 */
      cpu68k::returncode i_move_sr_ccr(context *ctx, uint16_t iw)
      {
	 uint16_t type,mode,reg;
	 uint32_t ea;
	 uint16_t s16;
	 int rc;
	 cpu68k::returncode ea_rc;

	 cpu68k *cpu = &(ctx->cpu);

	 type = ( iw >> 9 ) & 0x3;
	 mode = ( iw >> 3 ) & 0x7;
	 reg =  iw & 0x7;

	 if(DISASM)
	    ASM_INSTR("MOVE");

	 if(type == 0) {
	    /* MOVE from SR */
	    if(DISASM)
	       ASM_ARG1("SR");

	    /*
	    if( !cpu68k::IS_TYPE(cpu,CPU_TYPE_000) ) {
	       if(cpu68k::IS_TYPE(cpu,CPU_TYPE_AUTO)) {
		  cpu->type |= CPU_TYPE_000;
	       } else {
		  return cpu68k::I_PRIVILEGE_VIOLATION;
	       }
	    }
	    */

	    if(mode==0) {
	       if(EMULATE) {
		  cpu->d[reg] &= 0xffff0000;
		  cpu->d[reg] |= (uint32_t)cpu->sr;
	       }
	       if(DISASM)
		  ASM_ARG2("D%d",reg);

	    } else {
	       /* EA, Dn + alterable memory (0x1fd) */
	       ea_rc = calc_ea(ctx,2,mode,reg,&ea,0x1fd,2);
	       if(ea_rc) return ea_rc;

	       if(EMULATE) {
		  rc = WRITEMEM_16_D(cpu->sr,ea);
		  if(rc) return cpu68k::I_BUS_ERROR;
	       }
	    }

	 } else if(type == 1) {
	    /* MOVE from CCR */
	    if(DISASM)
	       ASM_ARG1("CCR");

	    /*
	    if( !cpu68k::IS_TYPE(cpu,CPU_TYPE_010) &&
	    !cpu68k::IS_TYPE(cpu,CPU_TYPE_020) &&
	    !cpu68k::IS_TYPE(cpu,CPU_TYPE_030) &&
	    !cpu68k::IS_TYPE(cpu,CPU_TYPE_040) &&
	    !cpu68k::IS_TYPE(cpu,CPU_TYPE_060)    ) {

	       if(cpu68k::IS_TYPE(cpu,CPU_TYPE_AUTO)) {
		  cpu->type |= CPU_TYPE_010;
	       } else {
		  return cpu68k::I_ILLEGAL_FOR_CPU;
	       }
	    }
	    */

	    if(mode==0) {
	       if(EMULATE) {
		  cpu->d[reg] &= 0xffff0000;
		  cpu->d[reg] |= (uint32_t)(cpu->sr & 0x1f);
	       }
	       if(DISASM)
		  ASM_ARG2("D%d",reg);

	    } else {
	       /* EA, Dn + alterable memory (0x1fd) */
	       ea_rc = calc_ea(ctx,2,mode,reg,&ea,0x1fd,2);
	       if(ea_rc) return ea_rc;

	       if(EMULATE) {
		  rc = WRITEMEM_16_D((cpu->sr & 0x1f),ea);
		  if(rc) return cpu68k::I_BUS_ERROR;
	       }
	    }

	 } else if(type == 2) {
	    /* MOVE to CCR */
	    if(DISASM)
	       ASM_ARG2("CCR");

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

	    if(EMULATE) {
	       cpu->sr &= 0xffe0;
	       cpu->sr |= (s16 & 0x1f);
	    }

	 } else {
	    /* MOVE to SR */
	    return cpu68k::I_PRIVILEGE;

	 }
	 return cpu68k::I_OK;
      }

/*
 * Function:       i_movem_ext
 * Instructions:   MOVEM, EXT
 * Bit pattern(s): 0100 1x00 1x
 */
      cpu68k::returncode i_movem_ext(context *ctx, uint16_t iw)
      {
	 uint16_t mask,dir,size,mode,reg;
	 uint32_t ea,tmp32;
	 uint8_t r8;
	 uint16_t r16,tmp16;
	 int rc;
	 cpu68k::returncode ea_rc;

	 cpu68k *cpu = &(ctx->cpu);

	 dir = ( iw >> 10 ) & 0x1;
	 size = ( iw >> 6 ) & 0x1;
	 mode = ( iw >> 3 ) & 0x7;
	 reg =  iw & 0x7;

	 if(dir==0 && mode==0) {
	    /* EXT */
	    if(DISASM)
	       ASM_INSTR("EXT");

	    if(EMULATE)
	       cpu->sr &= 0xfff0;  /* Clear NZVC */

	    if(size==0) {
	       /* Byte -> Word */
	       if(DISASM) {
		  ASM_INSTR(".W");
		  ASM_ARG1("D%d",reg);
	       }

	       if(EMULATE) {
		  r8 = (uint8_t)(cpu->d[reg] & 0xff);
		  cpu->d[reg] &= 0xffff0000;
		  cpu->d[reg] |= (uint32_t)(uint16_t)(int16_t)(int8_t)r8;

		  if( MSB8(r8) )
		     cpu->sr |= 0x8;  /* N */

		  if( !r8 )
		     cpu->sr |= 0x4;  /* Z */
	       }

	    } else {
	       /* Word -> Long */
	       if(DISASM) {
		  ASM_INSTR(".L");
		  ASM_ARG1("D%d",reg);
	       }

	       if(EMULATE) {
		  r16 = (uint16_t)(cpu->d[reg] & 0xffff);
		  cpu->d[reg] = (uint32_t)(int32_t)(int16_t)r16;

		  if( MSB16(r16) )
		     cpu->sr |= 0x8;  /* N */

		  if( !r16 )
		     cpu->sr |= 0x4;  /* Z */
	       }
	    }

	 } else {
	    /* MOVEM */
	    if(DISASM) {
	       if(size==0)
		  ASM_INSTR("MOVEM.W");
	       else
		  ASM_INSTR("MOVEM.L");
	    }

	    rc = READPC_16(&mask);
	    if(rc) return cpu68k::I_BUS_ERROR;

	    if(dir==0) {	/* Registers -> Memory */

	       if(mode==4) {	/* -(An) */
		  /* -(An) but size=0 will not modify the reg */
		  ea_rc = calc_ea(ctx,0,4,reg,&ea,0x4,2);
		  if(ea_rc) return ea_rc;

		  if(mask & 0x0001) {   /* A7 */
		     if(DISASM)
			ASM_ARG1("A7/");

		     if(EMULATE) {
			if(size==0) {   /* Word */
			   ea-=2;
			   rc = WRITEMEM_16_D((uint16_t)(cpu->a[7]&0xffff),ea);
			   if(rc) return cpu68k::I_BUS_ERROR;

			} else {        /* Long */
			   ea-=4;
			   rc = WRITEMEM_32_D(cpu->a[7],ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			}
		     }
		  }

		  if(mask & 0x0002) {   /* A6 */
		     if(DISASM)
			ASM_ARG1("A6/");

		     if(EMULATE) {
			if(size==0) {   /* Word */
			   ea-=2;
			   rc = WRITEMEM_16_D((uint16_t)(cpu->a[6]&0xffff),ea);
			   if(rc) return cpu68k::I_BUS_ERROR;

			} else {        /* Long */
			   ea-=4;
			   rc = WRITEMEM_32_D(cpu->a[6],ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			}
		     }
		  }

		  if(mask & 0x0004) {   /* A5 */
		     if(DISASM)
			ASM_ARG1("A5/");

		     if(EMULATE) {
			if(size==0) {   /* Word */
			   ea-=2;
			   rc = WRITEMEM_16_D((uint16_t)(cpu->a[5]&0xffff),ea);
			   if(rc) return cpu68k::I_BUS_ERROR;

			} else {        /* Long */
			   ea-=4;
			   rc = WRITEMEM_32_D(cpu->a[5],ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			}
		     }
		  }

		  if(mask & 0x0008) {   /* A4 */
		     if(DISASM)
			ASM_ARG1("A4/");

		     if(EMULATE) {
			if(size==0) {   /* Word */
			   ea-=2;
			   rc = WRITEMEM_16_D((uint16_t)(cpu->a[4]&0xffff),ea);
			   if(rc) return cpu68k::I_BUS_ERROR;

			} else {        /* Long */
			   ea-=4;
			   rc = WRITEMEM_32_D(cpu->a[4],ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			}
		     }
		  }

		  if(mask & 0x0010) {   /* A3 */
		     if(DISASM)
			ASM_ARG1("A3/");

		     if(EMULATE) {
			if(size==0) {   /* Word */
			   ea-=2;
			   rc = WRITEMEM_16_D((uint16_t)(cpu->a[3]&0xffff),ea);
			   if(rc) return cpu68k::I_BUS_ERROR;

			} else {        /* Long */
			   ea-=4;
			   rc = WRITEMEM_32_D(cpu->a[3],ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			}
		     }
		  }

		  if(mask & 0x0020) {   /* A2 */
		     if(DISASM)
			ASM_ARG1("A2/");

		     if(EMULATE) {
			if(size==0) {   /* Word */
			   ea-=2;
			   rc = WRITEMEM_16_D((uint16_t)(cpu->a[2]&0xffff),ea);
			   if(rc) return cpu68k::I_BUS_ERROR;

			} else {        /* Long */
			   ea-=4;
			   rc = WRITEMEM_32_D(cpu->a[2],ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			}
		     }
		  }

		  if(mask & 0x0040) {   /* A1 */
		     if(DISASM)
			ASM_ARG1("A1/");

		     if(EMULATE) {
			if(size==0) {   /* Word */
			   ea-=2;
			   rc = WRITEMEM_16_D((uint16_t)(cpu->a[1]&0xffff),ea);
			   if(rc) return cpu68k::I_BUS_ERROR;

			} else {        /* Long */
			   ea-=4;
			   rc = WRITEMEM_32_D(cpu->a[1],ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			}
		     }
		  }

		  if(mask & 0x0080) {   /* A0 */
		     if(DISASM)
			ASM_ARG1("A0/");

		     if(EMULATE) {
			if(size==0) {   /* Word */
			   ea-=2;
			   rc = WRITEMEM_16_D((uint16_t)(cpu->a[0]&0xffff),ea);
			   if(rc) return cpu68k::I_BUS_ERROR;

			} else {        /* Long */
			   ea-=4;
			   rc = WRITEMEM_32_D(cpu->a[0],ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			}
		     }
		  }

		  if(mask & 0x0100) {   /* D7 */
		     if(DISASM)
			ASM_ARG1("D7/");

		     if(EMULATE) {
			if(size==0) {   /* Word */
			   ea-=2;
			   rc = WRITEMEM_16_D((uint16_t)(cpu->d[7]&0xffff),ea);
			   if(rc) return cpu68k::I_BUS_ERROR;

			} else {        /* Long */
			   ea-=4;
			   rc = WRITEMEM_32_D(cpu->d[7],ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			}
		     }
		  }

		  if(mask & 0x0200) {   /* D6 */
		     if(DISASM)
			ASM_ARG1("D6/");

		     if(EMULATE) {
			if(size==0) {   /* Word */
			   ea-=2;
			   rc = WRITEMEM_16_D((uint16_t)(cpu->d[6]&0xffff),ea);
			   if(rc) return cpu68k::I_BUS_ERROR;

			} else {        /* Long */
			   ea-=4;
			   rc = WRITEMEM_32_D(cpu->d[6],ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			}
		     }
		  }

		  if(mask & 0x0400) {   /* D5 */
		     if(DISASM)
			ASM_ARG1("D5/");

		     if(EMULATE) {
			if(size==0) {   /* Word */
			   ea-=2;
			   rc = WRITEMEM_16_D((uint16_t)(cpu->d[5]&0xffff),ea);
			   if(rc) return cpu68k::I_BUS_ERROR;

			} else {        /* Long */
			   ea-=4;
			   rc = WRITEMEM_32_D(cpu->d[5],ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			}
		     }
		  }

		  if(mask & 0x0800) {   /* D4 */
		     if(DISASM)
			ASM_ARG1("D4/");

		     if(EMULATE) {
			if(size==0) {   /* Word */
			   ea-=2;
			   rc = WRITEMEM_16_D((uint16_t)(cpu->d[4]&0xffff),ea);
			   if(rc) return cpu68k::I_BUS_ERROR;

			} else {        /* Long */
			   ea-=4;
			   rc = WRITEMEM_32_D(cpu->d[4],ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			}
		     }
		  }

		  if(mask & 0x1000) {   /* D3 */
		     if(DISASM)
			ASM_ARG1("D3/");

		     if(EMULATE) {
			if(size==0) {   /* Word */
			   ea-=2;
			   rc = WRITEMEM_16_D((uint16_t)(cpu->d[3]&0xffff),ea);
			   if(rc) return cpu68k::I_BUS_ERROR;

			} else {        /* Long */
			   ea-=4;
			   rc = WRITEMEM_32_D(cpu->d[3],ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			}
		     }
		  }

		  if(mask & 0x2000) {   /* D2 */
		     if(DISASM)
			ASM_ARG1("D2/");

		     if(EMULATE) {
			if(size==0) {   /* Word */
			   ea-=2;
			   rc = WRITEMEM_16_D((uint16_t)(cpu->d[2]&0xffff),ea);
			   if(rc) return cpu68k::I_BUS_ERROR;

			} else {        /* Long */
			   ea-=4;
			   rc = WRITEMEM_32_D(cpu->d[2],ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			}
		     }
		  }

		  if(mask & 0x4000) {   /* D1 */
		     if(DISASM)
			ASM_ARG1("D1/");

		     if(EMULATE) {
			if(size==0) {   /* Word */
			   ea-=2;
			   rc = WRITEMEM_16_D((uint16_t)(cpu->d[1]&0xffff),ea);
			   if(rc) return cpu68k::I_BUS_ERROR;

			} else {        /* Long */
			   ea-=4;
			   rc = WRITEMEM_32_D(cpu->d[1],ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			}
		     }
		  }

		  if(mask & 0x8000) {   /* D0 */
		     if(DISASM)
			ASM_ARG1("D0/");

		     if(EMULATE) {
			if(size==0) {   /* Word */
			   ea-=2;
			   rc = WRITEMEM_16_D((uint16_t)(cpu->d[0]&0xffff),ea);
			   if(rc) return cpu68k::I_BUS_ERROR;

			} else {        /* Long */
			   ea-=4;
			   rc = WRITEMEM_32_D(cpu->d[0],ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			}
		     }
		  }

		  /* Update A-reg */
		  if(EMULATE)
		     cpu->a[reg] = ea;

	       } else {
		  /* EA, alterable memory but (An)+ (0x1f4) */
		  ea_rc = calc_ea(ctx,2,mode,reg,&ea,0x1f4,2);
		  if(ea_rc) return ea_rc;


		  if(mask & 0x0001) {   /* D0 */
		     if(DISASM)
			ASM_ARG1("D0/");

		     if(EMULATE) {
			if(size==0) {   /* Word */
			   rc = WRITEMEM_16_D((uint16_t)(cpu->d[0]&0xffff),ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			   ea+=2;

			} else {        /* Long */
			   rc = WRITEMEM_32_D(cpu->d[0],ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			   ea+=4;
			}
		     }
		  }

		  if(mask & 0x0002) {   /* D1 */
		     if(DISASM)
			ASM_ARG1("D1/");

		     if(EMULATE) {
			if(size==0) {   /* Word */
			   rc = WRITEMEM_16_D((uint16_t)(cpu->d[1]&0xffff),ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			   ea+=2;

			} else {        /* Long */
			   rc = WRITEMEM_32_D(cpu->d[1],ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			   ea+=4;
			}
		     }
		  }

		  if(mask & 0x0004) {   /* D2 */
		     if(DISASM)
			ASM_ARG1("D2/");

		     if(EMULATE) {
			if(size==0) {   /* Word */
			   rc = WRITEMEM_16_D((uint16_t)(cpu->d[2]&0xffff),ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			   ea+=2;

			} else {        /* Long */
			   rc = WRITEMEM_32_D(cpu->d[2],ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			   ea+=4;
			}
		     }
		  }

		  if(mask & 0x0008) {   /* D3 */
		     if(DISASM)
			ASM_ARG1("D3/");

		     if(EMULATE) {
			if(size==0) {   /* Word */
			   rc = WRITEMEM_16_D((uint16_t)(cpu->d[3]&0xffff),ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			   ea+=2;

			} else {        /* Long */
			   rc = WRITEMEM_32_D(cpu->d[3],ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			   ea+=4;
			}
		     }
		  }

		  if(mask & 0x0010) {   /* D4 */
		     if(DISASM)
			ASM_ARG1("D4/");

		     if(EMULATE) {
			if(size==0) {   /* Word */
			   rc = WRITEMEM_16_D((uint16_t)(cpu->d[4]&0xffff),ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			   ea+=2;

			} else {        /* Long */
			   rc = WRITEMEM_32_D(cpu->d[4],ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			   ea+=4;
			}
		     }
		  }

		  if(mask & 0x0020) {   /* D5 */
		     if(DISASM)
			ASM_ARG1("D5/");

		     if(EMULATE) {
			if(size==0) {   /* Word */
			   rc = WRITEMEM_16_D((uint16_t)(cpu->d[5]&0xffff),ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			   ea+=2;

			} else {        /* Long */
			   rc = WRITEMEM_32_D(cpu->d[5],ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			   ea+=4;
			}
		     }
		  }

		  if(mask & 0x0040) {   /* D6 */
		     if(DISASM)
			ASM_ARG1("D6/");

		     if(EMULATE) {
			if(size==0) {   /* Word */
			   rc = WRITEMEM_16_D((uint16_t)(cpu->d[6]&0xffff),ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			   ea+=2;

			} else {        /* Long */
			   rc = WRITEMEM_32_D(cpu->d[6],ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			   ea+=4;
			}
		     }
		  }

		  if(mask & 0x0080) {   /* D7 */
		     if(DISASM)
			ASM_ARG1("D7/");

		     if(EMULATE) {
			if(size==0) {   /* Word */
			   rc = WRITEMEM_16_D((uint16_t)(cpu->d[7]&0xffff),ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			   ea+=2;

			} else {        /* Long */
			   rc = WRITEMEM_32_D(cpu->d[7],ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			   ea+=4;
			}
		     }
		  }

		  if(mask & 0x0100) {   /* A0 */
		     if(DISASM)
			ASM_ARG1("A0/");

		     if(EMULATE) {
			if(size==0) {   /* Word */
			   rc = WRITEMEM_16_D((uint16_t)(cpu->a[0]&0xffff),ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			   ea+=2;

			} else {        /* Long */
			   rc = WRITEMEM_32_D(cpu->a[0],ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			   ea+=4;
			}
		     }
		  }

		  if(mask & 0x0200) {   /* A1 */
		     if(DISASM)
			ASM_ARG1("A1/");

		     if(EMULATE) {
			if(size==0) {   /* Word */
			   rc = WRITEMEM_16_D((uint16_t)(cpu->a[1]&0xffff),ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			   ea+=2;

			} else {        /* Long */
			   rc = WRITEMEM_32_D(cpu->a[1],ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			   ea+=4;
			}
		     }
		  }

		  if(mask & 0x0400) {   /* A2 */
		     if(DISASM)
			ASM_ARG1("A2/");

		     if(EMULATE) {
			if(size==0) {   /* Word */
			   rc = WRITEMEM_16_D((uint16_t)(cpu->a[2]&0xffff),ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			   ea+=2;

			} else {        /* Long */
			   rc = WRITEMEM_32_D(cpu->a[2],ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			   ea+=4;
			}
		     }
		  }

		  if(mask & 0x0800) {   /* A3 */
		     if(DISASM)
			ASM_ARG1("A3/");

		     if(EMULATE) {
			if(size==0) {   /* Word */
			   rc = WRITEMEM_16_D((uint16_t)(cpu->a[3]&0xffff),ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			   ea+=2;

			} else {        /* Long */
			   rc = WRITEMEM_32_D(cpu->a[3],ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			   ea+=4;
			}
		     }
		  }

		  if(mask & 0x1000) {   /* A4 */
		     if(DISASM)
			ASM_ARG1("A4/");

		     if(EMULATE) {
			if(size==0) {   /* Word */
			   rc = WRITEMEM_16_D((uint16_t)(cpu->a[4]&0xffff),ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			   ea+=2;

			} else {        /* Long */
			   rc = WRITEMEM_32_D(cpu->a[4],ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			   ea+=4;
			}
		     }
		  }

		  if(mask & 0x2000) {   /* A5 */
		     if(DISASM)
			ASM_ARG1("A5/");

		     if(EMULATE) {
			if(size==0) {   /* Word */
			   rc = WRITEMEM_16_D((uint16_t)(cpu->a[5]&0xffff),ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			   ea+=2;

			} else {        /* Long */
			   rc = WRITEMEM_32_D(cpu->a[5],ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			   ea+=4;
			}
		     }
		  }

		  if(mask & 0x4000) {   /* A6 */
		     if(DISASM)
			ASM_ARG1("A6/");

		     if(EMULATE) {
			if(size==0) {   /* Word */
			   rc = WRITEMEM_16_D((uint16_t)(cpu->a[6]&0xffff),ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			   ea+=2;

			} else {        /* Long */
			   rc = WRITEMEM_32_D(cpu->a[6],ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			   ea+=4;
			}
		     }
		  }

		  if(mask & 0x8000) {   /* A7 */
		     if(DISASM)
			ASM_ARG1("A7/");

		     if(EMULATE) {
			if(size==0) {   /* Word */
			   rc = WRITEMEM_16_D((uint16_t)(cpu->a[7]&0xffff),ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			   ea+=2;

			} else {        /* Long */
			   rc = WRITEMEM_32_D(cpu->a[7],ea);
			   if(rc) return cpu68k::I_BUS_ERROR;
			   ea+=4;
			}
		     }
		  }
	       }

	       /* Fix ARG1, remove last slash */
	       if(DISASM) {
		  int len = strlen(cpu->disasm_arg1);
		  if(len > 0)
		     cpu->disasm_arg1[len-1] = '\0';
	       }

	    } else {	/* Memory -> Registers */

	       /* EA, all but Dn,An,-(An),#<data> (0x7ec) */
	       /* (An)+ is valid but size=0 will not modify the reg */
	       ea_rc = calc_ea(ctx,0,mode,reg,&ea,0x7ec,1);
	       if(ea_rc) return ea_rc;


	       if(mask & 0x0001) {   /* D0 */
		  if(DISASM)
		     ASM_ARG2("D0/");

		  if(EMULATE) {
		     if(size==0) {   /* Word */
			rc = READMEM_16_D(ea, &tmp16);
			if(rc) return cpu68k::I_BUS_ERROR;
			cpu->d[0]=(uint32_t)(int32_t)(int16_t)tmp16;
			ea+=2;

		     } else {        /* Long */
			rc = READMEM_32_D(ea, &tmp32);
			if(rc) return cpu68k::I_BUS_ERROR;
			cpu->d[0]=tmp32;
			ea+=4;
		     }
		  }
	       }

	       if(mask & 0x0002) {   /* D1 */
		  if(DISASM)
		     ASM_ARG2("D1/");

		  if(EMULATE) {
		     if(size==0) {   /* Word */
			rc = READMEM_16_D(ea, &tmp16);
			if(rc) return cpu68k::I_BUS_ERROR;
			cpu->d[1]=(uint32_t)(int32_t)(int16_t)tmp16;
			ea+=2;

		     } else {        /* Long */
			rc = READMEM_32_D(ea, &tmp32);
			if(rc) return cpu68k::I_BUS_ERROR;
			cpu->d[1]=tmp32;
			ea+=4;
		     }
		  }
	       }

	       if(mask & 0x0004) {   /* D2 */
		  if(DISASM)
		     ASM_ARG2("D2/");

		  if(EMULATE) {
		     if(size==0) {   /* Word */
			rc = READMEM_16_D(ea, &tmp16);
			if(rc) return cpu68k::I_BUS_ERROR;
			cpu->d[2]=(uint32_t)(int32_t)(int16_t)tmp16;
			ea+=2;

		     } else {        /* Long */
			rc = READMEM_32_D(ea, &tmp32);
			if(rc) return cpu68k::I_BUS_ERROR;
			cpu->d[2]=tmp32;
			ea+=4;
		     }
		  }
	       }

	       if(mask & 0x0008) {   /* D3 */
		  if(DISASM)
		     ASM_ARG2("D3/");

		  if(EMULATE) {
		     if(size==0) {   /* Word */
			rc = READMEM_16_D(ea, &tmp16);
			if(rc) return cpu68k::I_BUS_ERROR;
			cpu->d[3]=(uint32_t)(int32_t)(int16_t)tmp16;
			ea+=2;

		     } else {        /* Long */
			rc = READMEM_32_D(ea, &tmp32);
			if(rc) return cpu68k::I_BUS_ERROR;
			cpu->d[3]=tmp32;
			ea+=4;
		     }
		  }
	       }

	       if(mask & 0x0010) {   /* D4 */
		  if(DISASM)
		     ASM_ARG2("D4/");

		  if(EMULATE) {
		     if(size==0) {   /* Word */
			rc = READMEM_16_D(ea, &tmp16);
			if(rc) return cpu68k::I_BUS_ERROR;
			cpu->d[4]=(uint32_t)(int32_t)(int16_t)tmp16;
			ea+=2;

		     } else {        /* Long */
			rc = READMEM_32_D(ea, &tmp32);
			if(rc) return cpu68k::I_BUS_ERROR;
			cpu->d[4]=tmp32;
			ea+=4;
		     }
		  }
	       }

	       if(mask & 0x0020) {   /* D5 */
		  if(DISASM)
		     ASM_ARG2("D5/");

		  if(EMULATE) {
		     if(size==0) {   /* Word */
			rc = READMEM_16_D(ea, &tmp16);
			if(rc) return cpu68k::I_BUS_ERROR;
			cpu->d[5]=(uint32_t)(int32_t)(int16_t)tmp16;
			ea+=2;

		     } else {        /* Long */
			rc = READMEM_32_D(ea, &tmp32);
			if(rc) return cpu68k::I_BUS_ERROR;
			cpu->d[5]=tmp32;
			ea+=4;
		     }
		  }
	       }

	       if(mask & 0x0040) {   /* D6 */
		  if(DISASM)
		     ASM_ARG2("D6/");

		  if(EMULATE) {
		     if(size==0) {   /* Word */
			rc = READMEM_16_D(ea, &tmp16);
			if(rc) return cpu68k::I_BUS_ERROR;
			cpu->d[6]=(uint32_t)(int32_t)(int16_t)tmp16;
			ea+=2;

		     } else {        /* Long */
			rc = READMEM_32_D(ea, &tmp32);
			if(rc) return cpu68k::I_BUS_ERROR;
			cpu->d[6]=tmp32;
			ea+=4;
		     }
		  }
	       }

	       if(mask & 0x0080) {   /* D7 */
		  if(DISASM)
		     ASM_ARG2("D7/");

		  if(EMULATE) {
		     if(size==0) {   /* Word */
			rc = READMEM_16_D(ea, &tmp16);
			if(rc) return cpu68k::I_BUS_ERROR;
			cpu->d[7]=(uint32_t)(int32_t)(int16_t)tmp16;
			ea+=2;

		     } else {        /* Long */
			rc = READMEM_32_D(ea, &tmp32);
			if(rc) return cpu68k::I_BUS_ERROR;
			cpu->d[7]=tmp32;
			ea+=4;
		     }
		  }
	       }

	       if(mask & 0x0100) {   /* A0 */
		  if(DISASM)
		     ASM_ARG2("A0/");

		  if(EMULATE) {
		     if(size==0) {   /* Word */
			rc = READMEM_16_D(ea, &tmp16);
			if(rc) return cpu68k::I_BUS_ERROR;
			cpu->a[0]=(uint32_t)(int32_t)(int16_t)tmp16;
			ea+=2;

		     } else {        /* Long */
			rc = READMEM_32_D(ea, &tmp32);
			if(rc) return cpu68k::I_BUS_ERROR;
			cpu->a[0]=tmp32;
			ea+=4;
		     }
		  }
	       }

	       if(mask & 0x0200) {   /* A1 */
		  if(DISASM)
		     ASM_ARG2("A1/");

		  if(EMULATE) {
		     if(size==0) {   /* Word */
			rc = READMEM_16_D(ea, &tmp16);
			if(rc) return cpu68k::I_BUS_ERROR;
			cpu->a[1]=(uint32_t)(int32_t)(int16_t)tmp16;
			ea+=2;

		     } else {        /* Long */
			rc = READMEM_32_D(ea, &tmp32);
			if(rc) return cpu68k::I_BUS_ERROR;
			cpu->a[1]=tmp32;
			ea+=4;
		     }
		  }
	       }

	       if(mask & 0x0400) {   /* A2 */
		  if(DISASM)
		     ASM_ARG2("A2/");

		  if(EMULATE) {
		     if(size==0) {   /* Word */
			rc = READMEM_16_D(ea, &tmp16);
			if(rc) return cpu68k::I_BUS_ERROR;
			cpu->a[2]=(uint32_t)(int32_t)(int16_t)tmp16;
			ea+=2;

		     } else {        /* Long */
			rc = READMEM_32_D(ea, &tmp32);
			if(rc) return cpu68k::I_BUS_ERROR;
			cpu->a[2]=tmp32;
			ea+=4;
		     }
		  }
	       }

	       if(mask & 0x0800) {   /* A3 */
		  if(DISASM)
		     ASM_ARG2("A3/");

		  if(EMULATE) {
		     if(size==0) {   /* Word */
			rc = READMEM_16_D(ea, &tmp16);
			if(rc) return cpu68k::I_BUS_ERROR;
			cpu->a[3]=(uint32_t)(int32_t)(int16_t)tmp16;
			ea+=2;

		     } else {        /* Long */
			rc = READMEM_32_D(ea, &tmp32);
			if(rc) return cpu68k::I_BUS_ERROR;
			cpu->a[3]=tmp32;
			ea+=4;
		     }
		  }
	       }

	       if(mask & 0x1000) {   /* A4 */
		  if(DISASM)
		     ASM_ARG2("A4/");

		  if(EMULATE) {
		     if(size==0) {   /* Word */
			rc = READMEM_16_D(ea, &tmp16);
			if(rc) return cpu68k::I_BUS_ERROR;
			cpu->a[4]=(uint32_t)(int32_t)(int16_t)tmp16;
			ea+=2;

		     } else {        /* Long */
			rc = READMEM_32_D(ea, &tmp32);
			if(rc) return cpu68k::I_BUS_ERROR;
			cpu->a[4]=tmp32;
			ea+=4;
		     }
		  }
	       }

	       if(mask & 0x2000) {   /* A5 */
		  if(DISASM)
		     ASM_ARG2("A5/");

		  if(EMULATE) {
		     if(size==0) {   /* Word */
			rc = READMEM_16_D(ea, &tmp16);
			if(rc) return cpu68k::I_BUS_ERROR;
			cpu->a[5]=(uint32_t)(int32_t)(int16_t)tmp16;
			ea+=2;

		     } else {        /* Long */
			rc = READMEM_32_D(ea, &tmp32);
			if(rc) return cpu68k::I_BUS_ERROR;
			cpu->a[5]=tmp32;
			ea+=4;
		     }
		  }
	       }

	       if(mask & 0x4000) {   /* A6 */
		  if(DISASM)
		     ASM_ARG2("A6/");

		  if(EMULATE) {
		     if(size==0) {   /* Word */
			rc = READMEM_16_D(ea, &tmp16);
			if(rc) return cpu68k::I_BUS_ERROR;
			cpu->a[6]=(uint32_t)(int32_t)(int16_t)tmp16;
			ea+=2;

		     } else {        /* Long */
			rc = READMEM_32_D(ea, &tmp32);
			if(rc) return cpu68k::I_BUS_ERROR;
			cpu->a[6]=tmp32;
			ea+=4;
		     }
		  }
	       }

	       if(mask & 0x8000) {   /* A7 */
		  if(DISASM)
		     ASM_ARG2("A7/");

		  if(EMULATE) {
		     if(size==0) {   /* Word */
			rc = READMEM_16_D(ea, &tmp16);
			if(rc) return cpu68k::I_BUS_ERROR;
			cpu->a[7]=(uint32_t)(int32_t)(int16_t)tmp16;
			ea+=2;

		     } else {        /* Long */
			rc = READMEM_32_D(ea, &tmp32);
			if(rc) return cpu68k::I_BUS_ERROR;
			cpu->a[7]=tmp32;
			ea+=4;
		     }
		  }
	       }

	       if(EMULATE) {
		  if(mode==3) {
		     /* Update A-reg if (An+) */
		     cpu->a[reg]=ea;
		  }
	       }

	       /* Fix ARG2, remove last slash */
	       if(DISASM) {
		  int len = strlen(cpu->disasm_arg2);
		  if(len > 0)
		     cpu->disasm_arg2[len-1] = '\0';
	       }
	    }
	    /* SP is changed and/or increased */
	    CHECK_SP();
	 }

	 return cpu68k::I_OK;
      }

/*
 * Function:       i_moveq
 * Instructions:   MOVEQ
 * Bit pattern(s): 0111 xxx0 xx
 */
      cpu68k::returncode i_moveq(context *ctx, uint16_t iw)
      {
	 uint16_t reg;
	 uint32_t s32;

	 cpu68k *cpu = &(ctx->cpu);

	 reg = ( iw >> 9 ) & 0x7;
	 s32 = (uint32_t)(int32_t)(int8_t)(uint8_t)(iw & 0xff);

	 if(DISASM) {
	    ASM_INSTR("MOVEQ");
	    ASM_ARG1("#%d",(unsigned)s32);
	    ASM_ARG2("D%d",reg);
	 }

	 if(EMULATE) {
	    cpu->sr &= 0xfff0;  /* Clear NZVC */
	    cpu->d[reg] = s32;

	    if( MSB32(s32) )
	       cpu->sr |= 0x8;  /* N */

	    if( !s32 )
	       cpu->sr |= 0x4;  /* Z */
	 }

	 return cpu68k::I_OK;
      }

/*
 * Function:       i_moves
 * Instructions:   MOVES
 * Bit pattern(s): 0000 1110 00
 *                 0000 1110 01
 *                 0000 1110 10
 */
      cpu68k::returncode i_moves(context *ctx, uint16_t iw __attribute__((unused)))
      {
	 cpu68k *cpu = &(ctx->cpu);

	 if(DISASM)
	    ASM_INSTR("MOVES");

	 return cpu68k::I_UNSUPPORTED;
      }

   } // namespace cpu_68k
} // namespace emumiga
