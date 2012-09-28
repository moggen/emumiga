/*
68k_help.cpp - M68K emulation. Help functions.

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

      cpu68k::returncode calc_ea(context *ctx, uint32_t size, uint16_t mode,
                                 uint16_t reg, uint32_t *ea_p, uint32_t mask,
                                 int debug_argno)
      {
	 uint32_t ea, tmp32=0;
	 uint16_t ext16, tmp16;
	 uint8_t tmp8;
	 int rc;

	 cpu68k *cpu = &(ctx->cpu);

	 ea = cpu68k::EA_NONE;
	 switch(mode) {
	    case 0:		/* Dn */
	       if(STRICT && !(mask & 0x0001) )
		  return cpu68k::I_ILLEGAL;
	       
	       if(DISASM) {
		  if(debug_argno == 1)
		     ASM_ARG1("D%d",reg);
		  else
		     ASM_ARG2("D%d",reg);
	       }
	       break;

	    case 1:		/* An */
	       if(STRICT && !(mask & 0x0002) )
		  return cpu68k::I_ILLEGAL;

	       if(DISASM) {
		  if(debug_argno == 1)
		     ASM_ARG1("A%d",reg);
		  else
		     ASM_ARG2("A%d",reg);
	       }
	       break;

	    case 2:		/* (An) */
	       if(STRICT && !(mask & 0x0004) )
		     return cpu68k::I_ILLEGAL;

	       if(DISASM) {
		  if(debug_argno == 1)
		     ASM_ARG1("(A%d)",reg);
		  else
		     ASM_ARG2("(A%d)",reg);
	       }
	       if(EMULATE)
		  ea = cpu->a[reg];
	       break;

	    case 3:		/* (An)+ */
	       if(STRICT && !(mask & 0x0008) )
		     return cpu68k::I_ILLEGAL;

	       if(DISASM) {
		  if(debug_argno == 1)
		     ASM_ARG1("(A%d)+",reg);
		  else
		     ASM_ARG2("(A%d)+",reg);
	       }

	       if(EMULATE) {
		  ea = cpu->a[reg];
		  if(reg==7 && size==1)
		     cpu->a[reg]+=2;
		  else
		     cpu->a[reg]+=size;

		  /* Check SP */
		  if(reg==7) {
		     CHECK_SP();
                  }
	       }
	       break;

	    case 4:		/* -(An) */
	       if(STRICT && !(mask & 0x0010) )
		     return cpu68k::I_ILLEGAL;

	       if(DISASM) {
		  if(debug_argno == 1)
		     ASM_ARG1("-(A%d)",reg);
		  else
		     ASM_ARG2("-(A%d)",reg);
	       }

	       if(EMULATE) {
		  if(reg==7 && size==1)
		     cpu->a[reg]-=2;
		  else
		     cpu->a[reg]-=size;

		  ea = cpu->a[reg];
	       }
	       break;

	    case 5:		/* (d16,An) */
	       if(STRICT && !(mask & 0x0020) )
		     return cpu68k::I_ILLEGAL;

	       rc = READPC_16(&tmp16);
	       if(rc) return cpu68k::I_BUS_ERROR;

	       ea = (uint32_t)(int32_t)(int16_t)tmp16;

	       if(DISASM) {
		  if(debug_argno == 1)
		     ASM_ARG1("(%u,A%d)",(unsigned)ea,reg);
		  else
		     ASM_ARG2("(%u,A%d)",(unsigned)ea,reg);
	       }

	       if(EMULATE)
		  ea += cpu->a[reg];

	       break;

	    case 6:		/* (d8,An,Xn.SIZE) */
	       if(STRICT && !(mask & 0x0040) )
		     return cpu68k::I_ILLEGAL;

	       rc = READPC_16(&ext16);
	       if(rc) return cpu68k::I_BUS_ERROR;

	       tmp16 = (ext16 & 0xff);
	       ea = (uint32_t)(int32_t)(int8_t)(uint8_t)tmp16;
	       tmp16 = (ext16>>12) & 0x07;

	       if(ext16 & 0x8000) {
		  /* Xn = An */
		  if( ext16 & 0x0800 ) {
		     /* SIZE = long */
		     if(DISASM) {
			if(debug_argno == 1)
			   ASM_ARG1("(%u,A%d,A%d.L)",(unsigned)ea,reg,tmp16);
			else
			   ASM_ARG2("(%u,A%d,A%d.L)",(unsigned)ea,reg,tmp16);
		     }

		     if(EMULATE)
			tmp32 = cpu->a[tmp16];

		  } else {
		     /* SIZE = word */
		     if(DISASM) {
			if(debug_argno == 1)
			   ASM_ARG1("(%u,A%d,A%d.W)",(unsigned)ea,reg,tmp16);
			else
			   ASM_ARG2("(%u,A%d,A%d.W)",(unsigned)ea,reg,tmp16);
		     }

		     if(EMULATE) {
			tmp32 = cpu->a[tmp16] & 0xffff;
			tmp32 = (uint32_t)(int32_t)(int16_t)(uint16_t)tmp32;
		     }
		  }
	       } else {
		  /* Xn = Dn */
		  if( ext16 & 0x0800 ) {
		     /* SIZE = long */
		     if(DISASM) {
			if(debug_argno == 1)
			   ASM_ARG1("(%u,A%d,D%d.L)",(unsigned)ea,reg,tmp16);
			else
			   ASM_ARG2("(%u,A%d,D%d.L)",(unsigned)ea,reg,tmp16);
		     }
		     if(EMULATE)
			tmp32 = cpu->d[tmp16];

		  } else {
		     /* SIZE = word */
		     if(DISASM) {
			if(debug_argno == 1)
			   ASM_ARG1("(%u,A%d,D%d.W)",(unsigned)ea,reg,tmp16);
			else
			   ASM_ARG2("(%u,A%d,D%d.W)",(unsigned)ea,reg,tmp16);
		     }
		     if(EMULATE) {
			tmp32 = cpu->d[tmp16] & 0xffff;
			tmp32 = (uint32_t)(int32_t)(int16_t)(uint16_t)tmp32;
		     }
		  }
	       }

	       if(EMULATE) {
		  ea += tmp32;
		  ea += cpu->a[reg];
	       }
	       break;

	    case 7:
	       if(reg==0) {		/* (xxx).W */
		  if(STRICT && !(mask & 0x0080) )
			return cpu68k::I_ILLEGAL;

		  rc = READPC_16(&tmp16);
		  if(rc) return cpu68k::I_BUS_ERROR;

		  ea = (uint32_t)(int32_t)(int16_t)tmp16;

		  if(DISASM) {
		     if(debug_argno == 1)
			ASM_ARG1("$%X.W",(unsigned int)ea);
		     else
			ASM_ARG2("$%X.W",(unsigned int)ea);
		  }

	       } else if(reg==1) {	/* (xxx).L */
		  if(STRICT && !(mask & 0x0100) )
			return cpu68k::I_ILLEGAL;

		  rc = READPC_32(&ea);
		  if(rc) return cpu68k::I_BUS_ERROR;

		  if(DISASM) {
		     if(debug_argno == 1)
			ASM_ARG1("$%X",(unsigned int)ea);
		     else
			ASM_ARG2("$%X",(unsigned int)ea);
		  }

	       } else if(reg==2) {	/* (d16,PC) */
		  if(STRICT && !(mask & 0x0200) )
			return cpu68k::I_ILLEGAL;

		  rc = READPC_16(&tmp16);
		  if(rc) return cpu68k::I_BUS_ERROR;

		  ea = (uint32_t)(int32_t)(int16_t)tmp16;

		  if(DISASM) {
		     if(debug_argno == 1)
			ASM_ARG1("(%u,PC)",(unsigned)ea);
		     else
			ASM_ARG2("(%u,PC)",(unsigned)ea);
		  }
		  if(EMULATE)
		     ea += cpu->pc - 2;

	       } else if(reg==3) {	/* (d8,PC,Xn.SIZE) */
		  if(STRICT && !(mask & 0x0400) )
			return cpu68k::I_ILLEGAL;

		  rc = READPC_16(&ext16);
		  if(rc) return cpu68k::I_BUS_ERROR;

		  tmp16 = (ext16 & 0xff);
		  ea = (uint32_t)(int32_t)(int8_t)(uint8_t)tmp16;
		  tmp16 = (ext16>>12) & 0x07;

		  if(ext16 & 0x8000) {
		     /* Xn = An */
		     if( ext16 & 0x0800 ) {
			/* SIZE = long */
			if(DISASM) {
			   if(debug_argno == 1)
			      ASM_ARG1("(%u,PC,A%d.L)",(unsigned)ea,tmp16);
			   else
			      ASM_ARG2("(%u,PC,A%d.L)",(unsigned)ea,tmp16);
			}
			if(EMULATE)
			   tmp32 = cpu->a[tmp16];

		     } else {
			/* SIZE = word */
			if(DISASM) {
			   if(debug_argno == 1)
			      ASM_ARG1("(%u,PC,A%d.W)",(unsigned)ea,tmp16);
			   else
			      ASM_ARG2("(%u,PC,A%d.W)",(unsigned)ea,tmp16);
			}
			if(EMULATE) {
			   tmp32 = cpu->a[tmp16] & 0xffff;
			   tmp32 = (uint32_t)(int32_t)(int16_t)(uint16_t)tmp32;
			}
		     }
		  } else {
		     /* Xn = Dn */
		     if( ext16 & 0x0800 ) {
			/* SIZE = long */
			if(DISASM) {
			   if(debug_argno == 1)
			      ASM_ARG1("(%u,PC,D%d.L)",(unsigned)ea,tmp16);
			   else
			      ASM_ARG2("(%u,PC,D%d.L)",(unsigned)ea,tmp16);
			}
			if(EMULATE)
			   tmp32 = cpu->d[tmp16];

		     } else {
			/* SIZE = word */
			if(DISASM) {
			   if(debug_argno == 1)
			      ASM_ARG1("(%u,PC,D%d.W)",(unsigned)ea,tmp16);
			   else
			      ASM_ARG2("(%u,PC,D%d.W)",(unsigned)ea,tmp16);
			}
			if(EMULATE) {
			   tmp32 = cpu->d[tmp16] & 0xffff;
			   tmp32 = (uint32_t)(int32_t)(int16_t)(uint16_t)tmp32;
			}
		     }
		  }

		  if(EMULATE)
		     ea += tmp32 + cpu->pc - 2;

	       } else if(reg==4) {	/* #imm */
		  if(STRICT && !(mask & 0x0800) )
			return cpu68k::I_ILLEGAL;

		  if(size==1) {
		     /* Byte */
		     rc = READPC_8(&tmp8);
		     if(rc) return cpu68k::I_BUS_ERROR;

		     ea = (uint32_t)(int32_t)(int8_t)tmp8;

		  } else if(size==2) {
		     /* Word */
		     rc = READPC_16(&tmp16);
		     if(rc) return cpu68k::I_BUS_ERROR;

		     ea = (uint32_t)(int32_t)(int16_t)tmp16;

		  } else {
		     /* Long */
		     rc = READPC_32(&ea);
		     if(rc) return cpu68k::I_BUS_ERROR;

		  }
		  if(debug_argno == 1)
		     ASM_ARG1("#%u",(unsigned)ea);
		  else
		     ASM_ARG2("#%u",(unsigned)ea);

	       } else {
		  if(STRICT)
		     return cpu68k::I_ILLEGAL;
	       }
	       break;
	 }
	 *ea_p = ea;

	 if(EMULATE && DISASM) {

	    /* Only makes sense if we are emulating
	       AND disassembling */

	    if(debug_argno == 1)
	       cpu->disasm_ea1 = ea;
	    else
	       cpu->disasm_ea2 = ea;
	 }

	 return cpu68k::I_OK;
      }

   } // namespace cpu
} // namespace emumiga
