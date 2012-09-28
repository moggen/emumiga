/*
68k.h - M68K emulation. Internal include file

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
#include "../emumiga.h"

//#ifndef CPU_DBGLEVEL
//#define CPU_DBGLEVEL(y) (cpu->flags & FLAG_DEBUG) && (cpu->dbg_level >= (y)))
//#endif

#ifndef DISASM
#define DISASM (cpu->flags & cpu68k::FLAG_DISASM)
#endif

#ifndef EMULATE
#define EMULATE (cpu->flags & cpu68k::FLAG_EMULATE)
#endif

#ifndef STRICT
#define STRICT (cpu->flags & cpu68k::FLAG_STRICT)
#endif

#define ASM_INSTR(...)                    \
   do {                                   \
      char buffer[2000];                  \
      sprintf(buffer,__VA_ARGS__);        \
      strcat(cpu->disasm_instr, buffer);  \
   } while(0)

#define ASM_ARG1(...)                     \
   do {                                   \
      char buffer[2000];                  \
      sprintf(buffer,__VA_ARGS__);        \
      strcat(cpu->disasm_arg1, buffer);   \
   } while(0)

#define ASM_ARG2(...)                     \
   do {                                   \
      char buffer[2000];                  \
      sprintf(buffer,__VA_ARGS__);        \
      strcat(cpu->disasm_arg2, buffer);   \
   } while(0)

#define READPC_8(v)                       \
      READMEM_8_I(cpu->pc+1, v);          \
      cpu->pc += 2;                       \
      if(DISASM)                          \
         cpu->disasm_bytes += 2;

#define READPC_16(v)                      \
      READMEM_16_I(cpu->pc, v);           \
      cpu->pc += 2;                       \
      if(DISASM)                          \
         cpu->disasm_bytes += 2;

#define READPC_32(v)                      \
      READMEM_32_I(cpu->pc, v);           \
      cpu->pc += 4;                       \
      if(DISASM)                          \
         cpu->disasm_bytes += 4;

#define LSB8(x) ((x) & 0x1)
#define LSB16(x) ((x) & 0x1)
#define LSB32(x) ((x) & 0x1)

#define MSB8(x) ((x) & 0x80)
#define MSB16(x) ((x) & 0x8000)
#define MSB32(x) ((x) & 0x80000000)

/* Bit values for mask parameter:
   0x0001   Dn
   0x0002   An
   0x0004   (An)
   0x0008   (An)+
   0x0010   -(An)
   0x0020   (d16,An)
   0x0040   (d8,An,Xn), (bd,An,Xn), ([bd,An,Xn],od), ([bd,An],Xn,od)
   0x0080   (xxx).W
   0x0100   (xxx).L
   0x0200   (d16,PC)
   0x0400   (d8,PC,Xn), (bd,PC,Xn), ([bd,PC,Xn],od), ([bd,PC],Xn,od)
   0x0800   #<data>
*/

// Predecls
namespace emumiga {
   namespace cpu_68k {

      // 68k_lut.cpp
      typedef cpu68k::returncode (*cpu68k_ip)(context *ctx, uint16_t iw);
      extern cpu68k_ip cpu_lut[];

      // 68k_i_help.cpp
      cpu68k::returncode calc_ea(context *ctx, uint32_t size, uint16_t mode, uint16_t reg, uint32_t *ea_p, uint32_t mask, int debug_argno);

      // 68k_i_1.cpp
      cpu68k::returncode i_add_ax(context *ctx, uint16_t iw);
      cpu68k::returncode i_addi(context *ctx, uint16_t iw);
      cpu68k::returncode i_addq(context *ctx, uint16_t iw);
      cpu68k::returncode i_sub_ax(context *ctx, uint16_t iw);
      cpu68k::returncode i_subi(context *ctx, uint16_t iw);
      cpu68k::returncode i_subq(context *ctx, uint16_t iw);
      cpu68k::returncode i_neg_x(context *ctx, uint16_t iw);

      // 68k_i_2.cpp
      cpu68k::returncode i_mul(context *ctx, uint16_t iw);
      cpu68k::returncode i_mull(context *ctx, uint16_t iw);
      cpu68k::returncode i_div(context *ctx, uint16_t iw);
      cpu68k::returncode i_divl(context *ctx, uint16_t iw);

      // 68k_i_3.cpp
      cpu68k::returncode i_and_abcd_exg(context *ctx, uint16_t iw);
      cpu68k::returncode i_andi(context *ctx, uint16_t iw);
      cpu68k::returncode i_or_sbcd_pack(context *ctx, uint16_t iw);
      cpu68k::returncode i_ori(context *ctx, uint16_t iw);
      cpu68k::returncode i_eor_cmpm(context *ctx, uint16_t iw);
      cpu68k::returncode i_eori(context *ctx, uint16_t iw);
      cpu68k::returncode i_not(context *ctx, uint16_t iw);

      // 68k_i_4.cpp
      cpu68k::returncode i_move(context *ctx, uint16_t iw);
      cpu68k::returncode i_move_sr_ccr(context *ctx, uint16_t iw);
      cpu68k::returncode i_movem_ext(context *ctx, uint16_t iw);
      cpu68k::returncode i_moveq(context *ctx, uint16_t iw);
      cpu68k::returncode i_moves(context *ctx, uint16_t iw);

      // 68k_i_5.cpp
      cpu68k::returncode i_sh(context *ctx, uint16_t iw);
      cpu68k::returncode i_sh_m(context *ctx, uint16_t iw);

      // 68k_i_6.cpp
      cpu68k::returncode i_jsr_jmp(context *ctx, uint16_t iw);
      cpu68k::returncode i_bra_bsr_bcc(context *ctx, uint16_t iw);
      cpu68k::returncode i_db_trap_s_cc(context *ctx, uint16_t iw);
      cpu68k::returncode i_callm_rtm(context *ctx, uint16_t iw);

      // 68k_i_7.cpp
      cpu68k::returncode i_bop_movep(context *ctx, uint16_t iw);
      cpu68k::returncode i_bopi(context *ctx, uint16_t iw);
      cpu68k::returncode i_bfop(context *ctx, uint16_t iw);

      // 68k_i_8.cpp
      cpu68k::returncode i_cmp_a(context *ctx, uint16_t iw);
      cpu68k::returncode i_cmpi(context *ctx, uint16_t iw);
      cpu68k::returncode i_chk(context *ctx, uint16_t iw);
      cpu68k::returncode i_cas_cas2(context *ctx, uint16_t iw);
      cpu68k::returncode i_cmp2_chk2(context *ctx, uint16_t iw);
      cpu68k::returncode i_tst_tas_ill(context *ctx, uint16_t iw);

      // 68k_i_9.cpp
      cpu68k::returncode i_lea_extb(context *ctx, uint16_t iw);
      cpu68k::returncode i_clr(context *ctx, uint16_t iw);
      cpu68k::returncode i_misc(context *ctx, uint16_t iw);
      cpu68k::returncode i_nbcd_linkl(context *ctx, uint16_t iw);
      cpu68k::returncode i_swap_bpkt_pea(context *ctx, uint16_t iw);

      // 68k_i_10.cpp
      cpu68k::returncode i_illegal(context *ctx, uint16_t iw);
      cpu68k::returncode i_line_a(context *ctx, uint16_t iw);
      cpu68k::returncode i_line_f(context *ctx, uint16_t iw);
      cpu68k::returncode i_emulator(context *ctx, uint16_t iw);

   } // namespace cpu68k
} // namespace emumiga

