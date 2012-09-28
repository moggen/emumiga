/*
68k_5.cpp - M68K emulation. Instructions.

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
 * Function:       i_sh
 * Instructions:   ASR, ASL, LSR, LSL, ROXR, ROXL, ROR, ROL
 * Bit pattern(s): 1110 xxxx 00
 *                 1110 xxxx 01
 *                 1110 xxxx 10
 */
      cpu68k::returncode i_sh(context *ctx, uint16_t iw)
      {
	 uint16_t cntreg,size,ir,type,reg;
	 uint8_t s8,last8,x8,msb8;
	 uint16_t s16,last16,x16,msb16;
	 uint32_t s32,last32,x32,msb32;
	 int count, i, msbchanged;

	 cpu68k *cpu = &(ctx->cpu);

	 cntreg = ( iw >> 9 ) & 0x7;
	 size = ( iw >> 6 ) & 0x3;
	 ir = ( iw >> 5 ) & 0x1;
	 type = ( iw >> 3 ) & 0x23;
	 reg = iw & 0x7;

	 if(EMULATE)
	    cpu->sr &= 0xfff0;  /* Clear NZVC */

	 if(DISASM) {
	    switch(type) {
	       case 0x00: ASM_INSTR("ASR");  break;
	       case 0x20: ASM_INSTR("ASL");  break;
	       case 0x01: ASM_INSTR("LSR");  break;
	       case 0x21: ASM_INSTR("LSL");  break;
	       case 0x02: ASM_INSTR("ROXR"); break;
	       case 0x22: ASM_INSTR("ROXL"); break;
	       case 0x03: ASM_INSTR("ROR");  break;
	       default:   ASM_INSTR("ROL");  break;
	    }
	    ASM_ARG2("D%d",reg);
	 }

	 count=0;
	 if(ir==0) {	/* Count */
	    count=cntreg;
	    if(count==0) count=8;
	    if(DISASM)
	       ASM_ARG1("#%d",count);

	 } else {	/* Register */
	    if(DISASM)
	       ASM_ARG1("D%d",cntreg);

	    if(EMULATE) {
	       count=cpu->d[cntreg];
	       count %= 64;
	    }
	 }

	 if(size==0) {
	    /* Byte */
	    if(DISASM)
	       ASM_INSTR(".B");

	    if(EMULATE) {
	       s8 = (uint8_t)(cpu->d[reg] & 0xff);
	       last8 = 0;

	       if(type==0x00) {		/* ASR */

		  msb8 = MSB8(s8);

		  for(i=0;i<count;i++) {
		     last8 = LSB8(s8);
		     s8 = s8 >> 1;
		     s8 |= msb8;
		  }

		  if(count > 0) {
		     if(last8) {
			cpu->sr |= 0x11;  /* XC */
		     } else {
			cpu->sr &= 0xffee;  /* !X!C */
		     }
		  }

	       } else if(type==0x20) {		/* ASL */

		  msbchanged=0;

		  for(i=0;i<count;i++) {
		     last8 = MSB8(s8);
		     s8 = s8 << 1;
		     if(last8 != MSB8(s8))
			msbchanged=1;
		  }

		  if( msbchanged )
		     cpu->sr |= 0x2;  /* V */
	
		  if(count > 0) {
		     if(last8) {
			cpu->sr |= 0x11;  /* XC */
		     } else {
			cpu->sr &= 0xffee;  /* !X!C */
		     }
		  }

	       } else if(type==0x01) {		/* LSR */

		  for(i=0;i<count;i++) {
		     last8 = LSB8(s8);
		     s8 = s8 >> 1;
		  }

		  if(count > 0) {
		     if(last8) {
			cpu->sr |= 0x11;  /* XC */
		     } else {
			cpu->sr &= 0xffee;  /* !X!C */
		     }
		  }

	       } else if(type==0x21) {		/* LSL */

		  for(i=0;i<count;i++) {
		     last8 = MSB8(s8);
		     s8 = s8 << 1;
		  }
	
		  if(count > 0) {
		     if(last8) {
			cpu->sr |= 0x11;  /* XC */
		     } else {
			cpu->sr &= 0xffee;  /* !X!C */
		     }
		  }

	       } else if(type==0x02) {		/* ROXR */

		  x8 = ( cpu->sr&0x10 ? 1 : 0 );

		  for(i=0;i<count;i++) {
		     last8 = LSB8(s8);
		     s8 = s8 >> 1;
		     s8 |= ( x8 ? 0x80 : 0 );
		     x8 = last8;
		  }

		  if( x8 ) {
		     cpu->sr |= 0x11;  /* XC */
		  } else {
		     cpu->sr &= 0xffee;  /* !X!C */
		  }

	       } else if(type==0x22) {		/* ROXL */

		  x8 = ( cpu->sr&0x10 ? 1 : 0 );

		  for(i=0;i<count;i++) {
		     last8 = MSB8(s8);
		     s8 = s8 << 1;
		     s8 |= ( x8 ? 1 : 0 );
		     x8 = last8;
		  }

		  if( x8 ) {
		     cpu->sr |= 0x11;  /* XC */
		  } else {
		     cpu->sr &= 0xffee;  /* !X!C */
		  }

	       } else if(type==0x03) {		/* ROR */

		  for(i=0;i<count;i++) {
		     last8 = LSB8(s8);
		     s8 = s8 >> 1;
		     s8 |= ( last8 ? 0x80 : 0 );
		  }

		  if(last8) {
		     cpu->sr |= 0x1;  /* C */
		  }

	       } else {			/* ROL */

		  for(i=0;i<count;i++) {
		     last8 = MSB8(s8);
		     s8 = s8 << 1;
		     s8 |= ( last8 ? 1 : 0 );
		  }

		  if(last8) {
		     cpu->sr |= 0x1;  /* C */
		  }
	       }

	       cpu->d[reg] &= 0xffffff00;
	       cpu->d[reg] |= (uint32_t)s8;

	       if( MSB8(s8) )
		  cpu->sr |= 0x8;  /* N */

	       if( !s8 )
		  cpu->sr |= 0x4;  /* Z */

	    }

	 } else if(size==1) {
	    /* Word */
	    if(DISASM)
	       ASM_INSTR(".W");

	    if(EMULATE) {
	       s16 = (uint16_t)(cpu->d[reg] & 0xffff);
	       last16 = 0;

	       if(type==0x00) {		/* ASR */

		  msb16 = MSB16(s16);

		  for(i=0;i<count;i++) {
		     last16 = LSB16(s16);
		     s16 = s16 >> 1;
		     s16 |= msb16;
		  }

		  if(count > 0) {
		     if(last16) {
			cpu->sr |= 0x11;  /* XC */
		     } else {
			cpu->sr &= 0xffee;  /* !X!C */
		     }
		  }

	       } else if(type==0x20) {		/* ASL */

		  msbchanged=0;

		  for(i=0;i<count;i++) {
		     last16 = MSB16(s16);
		     s16 = s16 << 1;
		     if(last16 != MSB16(s16))
			msbchanged=1;
		  }

		  if( msbchanged )
		     cpu->sr |= 0x2;  /* V */

		  if(count > 0) {
		     if(last16) {
			cpu->sr |= 0x11;  /* XC */
		     } else {
			cpu->sr &= 0xffee;  /* !X!C */
		     }
		  }

	       } else if(type==0x01) {		/* LSR */

		  for(i=0;i<count;i++) {
		     last16 = LSB16(s16);
		     s16 = s16 >> 1;
		  }

		  if(count > 0) {
		     if(last16) {
			cpu->sr |= 0x11;  /* XC */
		     } else {
			cpu->sr &= 0xffee;  /* !X!C */
		     }
		  }

	       } else if(type==0x21) {		/* LSL */

		  for(i=0;i<count;i++) {
		     last16 = MSB16(s16);
		     s16 = s16 << 1;
		  }

		  if(count > 0) {
		     if(last16) {
			cpu->sr |= 0x11;  /* XC */
		     } else {
			cpu->sr &= 0xffee;  /* !X!C */
		     }
		  }

	       } else if(type==0x02) {		/* ROXR */

		  x16 = ( cpu->sr&0x10 ? 1 : 0 );

		  for(i=0;i<count;i++) {
		     last16 = LSB16(s16);
		     s16 = s16 >> 1;
		     s16 |= ( x16 ? 0x8000 : 0 );
		     x16 = last16;
		  }

		  if( x16 ) {
		     cpu->sr |= 0x11;  /* XC */
		  } else {
		     cpu->sr &= 0xffee;  /* !X!C */
		  }

	       } else if(type==0x22) {		/* ROXL */

		  x16 = ( cpu->sr&0x10 ? 1 : 0 );

		  for(i=0;i<count;i++) {
		     last16 = MSB16(s16);
		     s16 = s16 << 1;
		     s16 |= ( x16 ? 1 : 0 );
		     x16 = last16;
		  }

		  if( x16 ) {
		     cpu->sr |= 0x11;  /* XC */
		  } else {
		     cpu->sr &= 0xffee;  /* !X!C */
		  }

	       } else if(type==0x03) {		/* ROR */

		  for(i=0;i<count;i++) {
		     last16 = LSB16(s16);
		     s16 = s16 >> 1;
		     s16 |= ( last16 ? 0x8000 : 0 );
		  }

		  if(last16) {
		     cpu->sr |= 0x1;  /* C */
		  }

	       } else {			/* ROL */

		  for(i=0;i<count;i++) {
		     last16 = MSB16(s16);
		     s16 = s16 << 1;
		     s16 |= ( last16 ? 1 : 0 );
		  }

		  if(last16) {
		     cpu->sr |= 0x1;  /* C */
		  }
	       }

	       cpu->d[reg] &= 0xffff0000;
	       cpu->d[reg] |= (uint32_t)s16;

	       if( MSB16(s16) )
		  cpu->sr |= 0x8;  /* N */

	       if( !s16 )
		  cpu->sr |= 0x4;  /* Z */

	    }

	 } else {
	    /* Long */
	    if(DISASM)
	       ASM_INSTR(".L");

	    if(EMULATE) {
	       s32 = cpu->d[reg];
	       last32 = 0;

	       if(type==0x00) {		/* ASR */

		  msb32 = MSB32(s32);

		  for(i=0;i<count;i++) {
		     last32 = LSB32(s32);
		     s32 = s32 >> 1;
		     s32 |= msb32;
		  }

		  if(count > 0) {
		     if(last32) {
			cpu->sr |= 0x11;  /* XC */
		     } else {
			cpu->sr &= 0xffee;  /* !X!C */
		     }
		  }

	       } else if(type==0x20) {		/* ASL */

		  msbchanged=0;

		  for(i=0;i<count;i++) {
		     last32 = MSB32(s32);
		     s32 = s32 << 1;
		     if(last32 != MSB32(s32))
			msbchanged=1;
		  }

		  if( msbchanged )
		     cpu->sr |= 0x2;  /* V */

		  if(count > 0) {
		     if(last32) {
			cpu->sr |= 0x11;  /* XC */
		     } else {
			cpu->sr &= 0xffee;  /* !X!C */
		     }
		  }

	       } else if(type==0x01) {		/* LSR */

		  for(i=0;i<count;i++) {
		     last32 = LSB32(s32);
		     s32 = s32 >> 1;
		  }

		  if(count > 0) {
		     if(last32) {
			cpu->sr |= 0x11;  /* XC */
		     } else {
			cpu->sr &= 0xffee;  /* !X!C */
		     }
		  }

	       } else if(type==0x21) {		/* LSL */

		  for(i=0;i<count;i++) {
		     last32 = MSB32(s32);
		     s32 = s32 << 1;
		  }

		  if(count > 0) {
		     if(last32) {
			cpu->sr |= 0x11;  /* XC */
		     } else {
			cpu->sr &= 0xffee;  /* !X!C */
		     }
		  }

	       } else if(type==0x02) {		/* ROXR */

		  x32 = ( cpu->sr&0x10 ? 1 : 0 );

		  for(i=0;i<count;i++) {
		     last32 = LSB32(s32);
		     s32 = s32 >> 1;
		     s32 |= ( x32 ? 0x80000000 : 0 );
		     x32 = last32;
		  }

		  if( x32 ) {
		     cpu->sr |= 0x11;  /* XC */
		  } else {
		     cpu->sr &= 0xffee;  /* !X!C */
		  }

	       } else if(type==0x22) {		/* ROXL */

		  x32 = ( cpu->sr&0x10 ? 1 : 0 );

		  for(i=0;i<count;i++) {
		     last32 = MSB32(s32);
		     s32 = s32 << 1;
		     s32 |= ( x32 ? 1 : 0 );
		     x32 = last32;
		  }

		  if( x32 ) {
		     cpu->sr |= 0x11;  /* XC */
		  } else {
		     cpu->sr &= 0xffee;  /* !X!C */
		  }

	       } else if(type==0x03) {		/* ROR */

		  for(i=0;i<count;i++) {
		     last32 = LSB32(s32);
		     s32 = s32 >> 1;
		     s32 |= ( last32 ? 0x80000000 : 0 );
		  }

		  if(last32) {
		     cpu->sr |= 0x1;  /* C */
		  }

	       } else {			/* ROL */

		  for(i=0;i<count;i++) {
		     last32 = MSB32(s32);
		     s32 = s32 << 1;
		     s32 |= ( last32 ? 1 : 0 );
		  }

		  if(last32) {
		     cpu->sr |= 0x1;  /* C */
		  }
	       }

	       cpu->d[reg] = s32;

	       if( MSB32(s32) )
		  cpu->sr |= 0x8;  /* N */

	       if( !s32 )
		  cpu->sr |= 0x4;  /* Z */
	    }
	 }
	 return cpu68k::I_OK;
      }

/*
 * Function:       i_sh_m
 * Instructions:   ASR_M, ASL_M, LSR_M, LSL_M,
 *                 ROXR_M, ROXL_M, ROR_M, ROL_M
 * Bit pattern(s): 1110 0xxx 11
 */
      cpu68k::returncode i_sh_m(context *ctx, uint16_t iw)
      {
	 uint16_t mode,reg,type;
	 uint32_t ea;
	 uint16_t s16,last16,msb16,x16;
	 int rc;
	 cpu68k::returncode ea_rc;

	 cpu68k *cpu = &(ctx->cpu);

	 type = ( iw >> 8 ) & 0x7;
	 mode = ( iw >> 3 ) & 0x7;
	 reg = iw & 0x7;

	 if(EMULATE)
	    cpu->sr &= 0xffe0;  /* Clear NZVC */

	 if(DISASM) {
	    switch(type) {
	       case 0x0: ASM_INSTR("ASR");  break;
	       case 0x1: ASM_INSTR("ASL");  break;
	       case 0x2: ASM_INSTR("LSR");  break;
	       case 0x3: ASM_INSTR("LSL");  break;
	       case 0x4: ASM_INSTR("ROXR"); break;
	       case 0x5: ASM_INSTR("ROXL"); break;
	       case 0x6: ASM_INSTR("ROR");  break;
	       default:  ASM_INSTR("ROL");  break;
	    }
	 }

	 /* EA, only alterable memory (0x1fc) */
	 ea_rc = calc_ea(ctx,2,mode,reg,&ea,0x1fc,1);
	 if(ea_rc) return ea_rc;

	 if(EMULATE) {
	    rc =  READMEM_16_D(ea, &s16);
	    if(rc) return cpu68k::I_BUS_ERROR;

	    if(type==0x0) {				/* ASR */

	       msb16 = MSB16(s16);
	       last16 = LSB16(s16);
	       s16 = s16 >> 1;
	       s16 |= msb16;

	       if( last16 ) {
		  cpu->sr |= 0x11;  /* XC */
	       } else {
		  cpu->sr &= 0xffee;  /* !X!C */
	       }

	    } else if(type==0x1) {			/* ASL */

	       last16 = MSB16(s16);
	       s16 = s16 << 1;

	       if( last16 != MSB16(s16) )
		  cpu->sr |= 0x2;  /* V */

	       if( last16 ) {
		  cpu->sr |= 0x11;  /* XC */
	       } else {
		  cpu->sr &= 0xffee;  /* !X!C */
	       }

	    } else if(type==0x2) {			/* LSR */

	       last16 = LSB16(s16);
	       s16 = s16 >> 1;

	       if( last16 ) {
		  cpu->sr |= 0x11;  /* XC */
	       } else {
		  cpu->sr &= 0xffee;  /* !X!C */
	       }

	    } else if(type==0x3) {			/* LSL */

	       last16 = MSB16(s16);
	       s16 = s16 << 1;

	       if( last16 ) {
		  cpu->sr |= 0x11;  /* XC */
	       } else {
		  cpu->sr &= 0xffee;  /* !X!C */
	       }

	    } else if(type==0x4) {			/* ROXR */

	       x16 = ( cpu->sr&0x10 ? 1 : 0 );
	       last16 = LSB16(s16);
	       s16 = s16 >> 1;
	       s16 |= ( x16 ? 0x8000 : 0 );
	       x16 = last16;

	       if( x16 ) {
		  cpu->sr |= 0x11;  /* XC */
	       } else {
		  cpu->sr &= 0xffee;  /* !X!C */
	       }

	    } else if(type==0x5) {			/* ROXL */

	       x16 = ( cpu->sr&0x10 ? 1 : 0 );
	       last16 = MSB16(s16);
	       s16 = s16 << 1;
	       s16 |= ( x16 ? 1 : 0 );
	       x16 = last16;

	       if( x16 ) {
		  cpu->sr |= 0x11;  /* XC */
	       } else {
		  cpu->sr &= 0xffee;  /* !X!C */
	       }

	    } else if(type==0x6) {			/* ROR */

	       last16 = LSB16(s16);
	       s16 = s16 >> 1;
	       s16 |= ( last16 ? 0x8000 : 0 );

	       if( last16 ) {
		  cpu->sr |= 0x1;  /* C */
	       }

	    } else {				/* ROL */

	       last16 = MSB16(s16);
	       s16 = s16 << 1;
	       s16 |= ( last16 ? 1 : 0 );

	       if( last16 ) {
		  cpu->sr |= 0x1;  /* C */
	       }

	    }

	    rc = WRITEMEM_16_D(s16,ea);
	    if(rc) return cpu68k::I_BUS_ERROR;

	    if( MSB16(s16) )
	       cpu->sr |= 0x8;  /* N */

	    if( !s16 )
	       cpu->sr |= 0x4;  /* Z */
	 }
	 return cpu68k::I_OK;
      }

   } // namespace cpu_68k
} // namespace emumiga
