/*
68k_lut.cpp - M68K emulation lookup table

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

      cpu68k_ip cpu_lut[] = {

	i_ori,		/* 0000 0000 00	 ORI.B, ORI_CCR			*/
	i_ori,		/* 0000 0000 01	 ORI.W, ORI_SR			*/
	i_ori,		/* 0000 0000 10	 ORI.L				*/
	i_cmp2_chk2,	/* 0000 0000 11	 CMP2.B, CHK2.B			*/

	i_bop_movep,	/* 0000 0001 00  BTST, MOVEP.W			*/
	i_bop_movep,	/* 0000 0001 01  BCHG, MOVEP.L			*/
	i_bop_movep,	/* 0000 0001 10  BCLR, MOVEP.W			*/
	i_bop_movep,	/* 0000 0001 11  BSET, MOVEP.L			*/

	i_andi,		/* 0000 0010 00	 ANDI.B, ANDI_CCR		*/
	i_andi,		/* 0000 0010 01	 ANDI.W, ANDI_SR		*/
	i_andi,		/* 0000 0010 10	 ANDI.L				*/
	i_cmp2_chk2,	/* 0000 0010 11  CMP2.W, CHK2.W			*/

	i_bop_movep,	/* 0000 0011 00  BTST, MOVEP.W			*/
	i_bop_movep,	/* 0000 0011 01  BCHG, MOVEP.L			*/
	i_bop_movep,	/* 0000 0011 10  BCLR, MOVEP.W			*/
	i_bop_movep,	/* 0000 0011 11  BSET, MOVEP.L			*/

	i_subi,		/* 0000 0100 00	 SUBI.B				*/
	i_subi,		/* 0000 0100 01	 SUBI.W				*/
	i_subi,		/* 0000 0100 10	 SUBI.L				*/
	i_cmp2_chk2,	/* 0000 0100 11  CMP2.L, CHK2.L			*/

	i_bop_movep,	/* 0000 0101 00  BTST, MOVEP.W			*/
	i_bop_movep,	/* 0000 0101 01  BCHG, MOVEP.L			*/
	i_bop_movep,	/* 0000 0101 10  BCLR, MOVEP.W			*/
	i_bop_movep,	/* 0000 0101 11  BSET, MOVEP.L			*/

	i_addi,		/* 0000 0110 00	 ADDI.B				*/
	i_addi,		/* 0000 0110 01	 ADDI.W				*/
	i_addi,		/* 0000 0110 10	 ADDI.L				*/
	i_callm_rtm,	/* 0000 0110 11  CALLM, RTM			*/

	i_bop_movep,	/* 0000 0111 00  BTST, MOVEP.W			*/
	i_bop_movep,	/* 0000 0111 01  BCHG, MOVEP.L			*/
	i_bop_movep,	/* 0000 0111 10  BCLR, MOVEP.W			*/
	i_bop_movep,	/* 0000 0111 11  BSET, MOVEP.L			*/

	i_bopi,		/* 0000 1000 00	 BTST_I				*/
	i_bopi,		/* 0000 1000 01	 BCHG_I				*/
	i_bopi,		/* 0000 1000 10	 BCLR_I				*/
	i_bopi,		/* 0000 1000 11	 BSET_I				*/

	i_bop_movep,	/* 0000 1001 00  BTST, MOVEP.W			*/
	i_bop_movep,	/* 0000 1001 01  BCHG, MOVEP.L			*/
	i_bop_movep,	/* 0000 1001 10  BCLR, MOVEP.W			*/
	i_bop_movep,	/* 0000 1001 11  BSET, MOVEP.L			*/

	i_eori,		/* 0000 1010 00	 EORI.B, EORI_CCR		*/
	i_eori,		/* 0000 1010 01	 EORI.W, EORI_SR		*/
	i_eori,		/* 0000 1010 10	 EORI.L				*/
	i_cas_cas2,	/* 0000 1010 11	 CAS.B, CAS2.B			*/

	i_bop_movep,	/* 0000 1011 00  BTST, MOVEP.W			*/
	i_bop_movep,	/* 0000 1011 01  BCHG, MOVEP.L			*/
	i_bop_movep,	/* 0000 1011 10  BCLR, MOVEP.W			*/
	i_bop_movep,	/* 0000 1011 11  BSET, MOVEP.L			*/

	i_cmpi,		/* 0000 1100 00	 CMPI.B				*/
	i_cmpi,		/* 0000 1100 01	 CMPI.W				*/
	i_cmpi,		/* 0000 1100 10	 CMPI.L				*/
	i_cas_cas2,	/* 0000 1100 11	 CAS.W, CAS2.W			*/

	i_bop_movep,	/* 0000 1101 00  BTST, MOVEP.W			*/
	i_bop_movep,	/* 0000 1101 01  BCHG, MOVEP.L			*/
	i_bop_movep,	/* 0000 1101 10  BCLR, MOVEP.W			*/
	i_bop_movep,	/* 0000 1101 11  BSET, MOVEP.L			*/

	i_moves,	/* 0000 1110 00	 MOVES.B			*/
	i_moves,	/* 0000 1110 01	 MOVES.W			*/
	i_moves,	/* 0000 1110 10	 MOVES.L			*/
	i_cas_cas2,	/* 0000 1110 11	 CAS.L, CAS2.L			*/

	i_bop_movep,	/* 0000 1111 00  BTST, MOVEP.W			*/
	i_bop_movep,	/* 0000 1111 01  BCHG, MOVEP.L			*/
	i_bop_movep,	/* 0000 1111 10  BCLR, MOVEP.W			*/
	i_bop_movep,	/* 0000 1111 11  BSET, MOVEP.L			*/

	i_move,		/* 0001 0000 00  MOVE.B				*/
	i_illegal,	/* 0001 0000 01  				*/
	i_move,		/* 0001 0000 10  MOVE.B				*/
	i_move,		/* 0001 0000 11  MOVE.B				*/

	i_move,		/* 0001 0001 00  MOVE.B				*/
	i_move,		/* 0001 0001 01  MOVE.B				*/
	i_move,		/* 0001 0001 10  MOVE.B				*/
	i_move,		/* 0001 0001 11  MOVE.B				*/

	i_move,		/* 0001 0010 00  MOVE.B				*/
	i_illegal,	/* 0001 0010 01  				*/
	i_move,		/* 0001 0010 10  MOVE.B				*/
	i_move,		/* 0001 0010 11  MOVE.B				*/

	i_move,		/* 0001 0011 00  MOVE.B				*/
	i_move,		/* 0001 0011 01  MOVE.B				*/
	i_move,		/* 0001 0011 10  MOVE.B				*/
	i_move,		/* 0001 0011 11  MOVE.B				*/

	i_move,		/* 0001 0100 00  MOVE.B				*/
	i_illegal,	/* 0001 0100 01  				*/
	i_move,		/* 0001 0100 10  MOVE.B				*/
	i_move,		/* 0001 0100 11  MOVE.B				*/

	i_move,		/* 0001 0101 00  MOVE.B				*/
	i_move,		/* 0001 0101 01  MOVE.B				*/
	i_move,		/* 0001 0101 10  MOVE.B				*/
	i_move,		/* 0001 0101 11  MOVE.B				*/

	i_move,		/* 0001 0110 00  MOVE.B				*/
	i_illegal,	/* 0001 0110 01  				*/
	i_move,		/* 0001 0110 10  MOVE.B				*/
	i_move,		/* 0001 0110 11  MOVE.B				*/

	i_move,		/* 0001 0111 00  MOVE.B				*/
	i_move,		/* 0001 0111 01  MOVE.B				*/
	i_move,		/* 0001 0111 10  MOVE.B				*/
	i_move,		/* 0001 0111 11  MOVE.B				*/

	i_move,		/* 0001 1000 00  MOVE.B				*/
	i_illegal,	/* 0001 1000 01  				*/
	i_move,		/* 0001 1000 10  MOVE.B				*/
	i_move,		/* 0001 1000 11  MOVE.B				*/

	i_move,		/* 0001 1001 00  MOVE.B				*/
	i_move,		/* 0001 1001 01  MOVE.B				*/
	i_move,		/* 0001 1001 10  MOVE.B				*/
	i_move,		/* 0001 1001 11  MOVE.B				*/

	i_move,		/* 0001 1010 00  MOVE.B				*/
	i_illegal,	/* 0001 1010 01  				*/
	i_move,		/* 0001 1010 10  MOVE.B				*/
	i_move,		/* 0001 1010 11  MOVE.B				*/

	i_move,		/* 0001 1011 00  MOVE.B				*/
	i_move,		/* 0001 1011 01  MOVE.B				*/
	i_move,		/* 0001 1011 10  MOVE.B				*/
	i_move,		/* 0001 1011 11  MOVE.B				*/

	i_move,		/* 0001 1100 00  MOVE.B				*/
	i_illegal,	/* 0001 1100 01  				*/
	i_move,		/* 0001 1100 10  MOVE.B				*/
	i_move,		/* 0001 1100 11  MOVE.B				*/

	i_move,		/* 0001 1101 00  MOVE.B				*/
	i_move,		/* 0001 1101 01  MOVE.B				*/
	i_move,		/* 0001 1101 10  MOVE.B				*/
	i_move,		/* 0001 1101 11  MOVE.B				*/

	i_move,		/* 0001 1110 00  MOVE.B				*/
	i_illegal,	/* 0001 1110 01  				*/
	i_move,		/* 0001 1110 10  MOVE.B				*/
	i_move,		/* 0001 1110 11  MOVE.B				*/

	i_move,		/* 0001 1111 00  MOVE.B				*/
	i_move,		/* 0001 1111 01  MOVE.B				*/
	i_move,		/* 0001 1111 10  MOVE.B				*/
	i_move,		/* 0001 1111 11  MOVE.B				*/

	i_move,		/* 0010 0000 00  MOVE.L				*/
	i_move,		/* 0010 0000 01  MOVEA.L			*/
	i_move,		/* 0010 0000 10  MOVE.L				*/
	i_move,		/* 0010 0000 11  MOVE.L				*/

	i_move,		/* 0010 0001 00  MOVE.L				*/
	i_move,		/* 0010 0001 01  MOVE.L				*/
	i_move,		/* 0010 0001 10  MOVE.L				*/
	i_move,		/* 0010 0001 11  MOVE.L				*/

	i_move,		/* 0010 0010 00  MOVE.L				*/
	i_move,		/* 0010 0010 01  MOVEA.L			*/
	i_move,		/* 0010 0010 10  MOVE.L				*/
	i_move,		/* 0010 0010 11  MOVE.L				*/

	i_move,		/* 0010 0011 00  MOVE.L				*/
	i_move,		/* 0010 0011 01  MOVE.L				*/
	i_move,		/* 0010 0011 10  MOVE.L				*/
	i_move,		/* 0010 0011 11  MOVE.L				*/

	i_move,		/* 0010 0100 00  MOVE.L				*/
	i_move,		/* 0010 0100 01  MOVEA.L			*/
	i_move,		/* 0010 0100 10  MOVE.L				*/
	i_move,		/* 0010 0100 11  MOVE.L				*/

	i_move,		/* 0010 0101 00  MOVE.L				*/
	i_move,		/* 0010 0101 01  MOVE.L				*/
	i_move,		/* 0010 0101 10  MOVE.L				*/
	i_move,		/* 0010 0101 11  MOVE.L				*/

	i_move,		/* 0010 0110 00  MOVE.L				*/
	i_move,		/* 0010 0110 01  MOVEA.L			*/
	i_move,		/* 0010 0110 10  MOVE.L				*/
	i_move,		/* 0010 0110 11  MOVE.L				*/

	i_move,		/* 0010 0111 00  MOVE.L				*/
	i_move,		/* 0010 0111 01  MOVE.L				*/
	i_move,		/* 0010 0111 10  MOVE.L				*/
	i_move,		/* 0010 0111 11  MOVE.L				*/

	i_move,		/* 0010 1000 00  MOVE.L				*/
	i_move,		/* 0010 1000 01  MOVEA.L			*/
	i_move,		/* 0010 1000 10  MOVE.L				*/
	i_move,		/* 0010 1000 11  MOVE.L				*/

	i_move,		/* 0010 1001 00  MOVE.L				*/
	i_move,		/* 0010 1001 01  MOVE.L				*/
	i_move,		/* 0010 1001 10  MOVE.L				*/
	i_move,		/* 0010 1001 11  MOVE.L				*/

	i_move,		/* 0010 1010 00  MOVE.L				*/
	i_move,		/* 0010 1010 01  MOVEA.L			*/
	i_move,		/* 0010 1010 10  MOVE.L				*/
	i_move,		/* 0010 1010 11  MOVE.L				*/

	i_move,		/* 0010 1011 00  MOVE.L				*/
	i_move,		/* 0010 1011 01  MOVE.L				*/
	i_move,		/* 0010 1011 10  MOVE.L				*/
	i_move,		/* 0010 1011 11  MOVE.L				*/

	i_move,		/* 0010 1100 00  MOVE.L				*/
	i_move,		/* 0010 1100 01  MOVEA.L			*/
	i_move,		/* 0010 1100 10  MOVE.L				*/
	i_move,		/* 0010 1100 11  MOVE.L				*/

	i_move,		/* 0010 1101 00  MOVE.L				*/
	i_move,		/* 0010 1101 01  MOVE.L				*/
	i_move,		/* 0010 1101 10  MOVE.L				*/
	i_move,		/* 0010 1101 11  MOVE.L				*/

	i_move,		/* 0010 1110 00  MOVE.L				*/
	i_move,		/* 0010 1110 01  MOVEA.L			*/
	i_move,		/* 0010 1110 10  MOVE.L				*/
	i_move,		/* 0010 1110 11  MOVE.L				*/

	i_move,		/* 0010 1111 00  MOVE.L				*/
	i_move,		/* 0010 1111 01  MOVE.L				*/
	i_move,		/* 0010 1111 10  MOVE.L				*/
	i_move,		/* 0010 1111 11  MOVE.L				*/

	i_move,		/* 0011 0000 00  MOVE.W				*/
	i_move,		/* 0011 0000 01  MOVEA.W			*/
	i_move,		/* 0011 0000 10  MOVE.W				*/
	i_move,		/* 0011 0000 11  MOVE.W				*/

	i_move,		/* 0011 0001 00  MOVE.W				*/
	i_move,		/* 0011 0001 01  MOVE.W				*/
	i_move,		/* 0011 0001 10  MOVE.W				*/
	i_move,		/* 0011 0001 11  MOVE.W				*/

	i_move,		/* 0011 0010 00  MOVE.W				*/
	i_move,		/* 0011 0010 01  MOVEA.W			*/
	i_move,		/* 0011 0010 10  MOVE.W				*/
	i_move,		/* 0011 0010 11  MOVE.W				*/

	i_move,		/* 0011 0011 00  MOVE.W				*/
	i_move,		/* 0011 0011 01  MOVE.W				*/
	i_move,		/* 0011 0011 10  MOVE.W				*/
	i_move,		/* 0011 0011 11  MOVE.W				*/

	i_move,		/* 0011 0100 00  MOVE.W				*/
	i_move,		/* 0011 0100 01  MOVEA.W			*/
	i_move,		/* 0011 0100 10  MOVE.W				*/
	i_move,		/* 0011 0100 11  MOVE.W				*/

	i_move,		/* 0011 0101 00  MOVE.W				*/
	i_move,		/* 0011 0101 01  MOVE.W				*/
	i_move,		/* 0011 0101 10  MOVE.W				*/
	i_move,		/* 0011 0101 11  MOVE.W				*/

	i_move,		/* 0011 0110 00  MOVE.W				*/
	i_move,		/* 0011 0110 01  MOVEA.W			*/
	i_move,		/* 0011 0110 10  MOVE.W				*/
	i_move,		/* 0011 0110 11  MOVE.W				*/

	i_move,		/* 0011 0111 00  MOVE.W				*/
	i_move,		/* 0011 0111 01  MOVE.W				*/
	i_move,		/* 0011 0111 10  MOVE.W				*/
	i_move,		/* 0011 0111 11  MOVE.W				*/

	i_move,		/* 0011 1000 00  MOVE.W				*/
	i_move,		/* 0011 1000 01  MOVEA.W			*/
	i_move,		/* 0011 1000 10  MOVE.W				*/
	i_move,		/* 0011 1000 11  MOVE.W				*/

	i_move,		/* 0011 1001 00  MOVE.W				*/
	i_move,		/* 0011 1001 01  MOVE.W				*/
	i_move,		/* 0011 1001 10  MOVE.W				*/
	i_move,		/* 0011 1001 11  MOVE.W				*/

	i_move,		/* 0011 1010 00  MOVE.W				*/
	i_move,		/* 0011 1010 01  MOVEA.W			*/
	i_move,		/* 0011 1010 10  MOVE.W				*/
	i_move,		/* 0011 1010 11  MOVE.W				*/

	i_move,		/* 0011 1011 00  MOVE.W				*/
	i_move,		/* 0011 1011 01  MOVE.W				*/
	i_move,		/* 0011 1011 10  MOVE.W				*/
	i_move,		/* 0011 1011 11  MOVE.W				*/

	i_move,		/* 0011 1100 00  MOVE.W				*/
	i_move,		/* 0011 1100 01  MOVEA.W			*/
	i_move,		/* 0011 1100 10  MOVE.W				*/
	i_move,		/* 0011 1100 11  MOVE.W				*/

	i_move,		/* 0011 1101 00  MOVE.W				*/
	i_move,		/* 0011 1101 01  MOVE.W				*/
	i_move,		/* 0011 1101 10  MOVE.W				*/
	i_move,		/* 0011 1101 11  MOVE.W				*/

	i_move,		/* 0011 1110 00  MOVE.W				*/
	i_move,		/* 0011 1110 01  MOVEA.W			*/
	i_move,		/* 0011 1110 10  MOVE.W				*/
	i_move,		/* 0011 1110 11  MOVE.W				*/

	i_move,		/* 0011 1111 00  MOVE.W				*/
	i_move,		/* 0011 1111 01  MOVE.W				*/
	i_move,		/* 0011 1111 10  MOVE.W				*/
	i_move,		/* 0011 1111 11  MOVE.W				*/

	i_neg_x,	/* 0100 0000 00  NEGX.B				*/
	i_neg_x,	/* 0100 0000 01  NEGX.W				*/
	i_neg_x,	/* 0100 0000 10  NEGX.L				*/
	i_move_sr_ccr,	/* 0100 0000 11  MOVE_f_SR			*/

	i_chk,		/* 0100 0001 00  CHK.L				*/
	i_illegal,	/* 0100 0001 01  				*/
	i_chk,		/* 0100 0001 10  CHK.W				*/
	i_lea_extb,	/* 0100 0001 11  LEA				*/

	i_clr,		/* 0100 0010 00  CLR.B				*/
	i_clr,		/* 0100 0010 01  CLR.W				*/
	i_clr,		/* 0100 0010 10  CLR.L				*/
	i_move_sr_ccr,	/* 0100 0010 11  MOVE_f_CCR			*/

	i_chk,		/* 0100 0011 00  CHK.L				*/
	i_illegal,	/* 0100 0011 01  				*/
	i_chk,		/* 0100 0011 10  CHK.W				*/
	i_lea_extb,	/* 0100 0011 11  LEA				*/

	i_neg_x,	/* 0100 0100 00  NEG.B				*/
	i_neg_x,	/* 0100 0100 01  NEG.W				*/
	i_neg_x,	/* 0100 0100 10  NEG.L				*/
	i_move_sr_ccr,	/* 0100 0100 11  MOVE_t_CCR			*/

	i_chk,		/* 0100 0101 00  CHK.L				*/
	i_illegal,	/* 0100 0101 01  				*/
	i_chk,		/* 0100 0101 10  CHK.W				*/
	i_lea_extb,	/* 0100 0101 11  LEA				*/

	i_not,		/* 0100 0110 00  NOT.B				*/
	i_not,		/* 0100 0110 01  NOT.W				*/
	i_not,		/* 0100 0110 10  NOT.L				*/
	i_move_sr_ccr,	/* 0100 0110 11  MOVE_t_SR			*/

	i_chk,		/* 0100 0111 00  CHK.L				*/
	i_illegal,	/* 0100 0111 01  				*/
	i_chk,		/* 0100 0111 10  CHK.W				*/
	i_lea_extb,	/* 0100 0111 11  LEA				*/

	i_nbcd_linkl,	/* 0100 1000 00  NBCD, LINK_L			*/
	i_swap_bpkt_pea,/* 0100 1000 01  SWAP, BPKT, PEA		*/
	i_movem_ext,	/* 0100 1000 10  MOVEM.W, EXT.W			*/
	i_movem_ext,	/* 0100 1000 11  MOVEM.L, EXT.L			*/

	i_chk,		/* 0100 1001 00  CHK.L				*/
	i_illegal,	/* 0100 1001 01  				*/
	i_chk,		/* 0100 1001 10  CHK.W				*/
	i_lea_extb,	/* 0100 1001 11  LEA, EXTB.L			*/

	i_tst_tas_ill,	/* 0100 1010 00  TST.B				*/
	i_tst_tas_ill,	/* 0100 1010 01  TST.W				*/
	i_tst_tas_ill,	/* 0100 1010 10  TST.L				*/
	i_tst_tas_ill,	/* 0100 1010 11  TAS, ILLEGAL			*/

	i_chk,		/* 0100 1011 00  CHK.L				*/
	i_illegal,	/* 0100 1011 01  				*/
	i_chk,		/* 0100 1011 10  CHK.W				*/
	i_lea_extb,	/* 0100 1011 11  LEA				*/

	i_mull,		/* 0100 1100 00  MULU_L, MULS_L			*/
	i_divl,		/* 0100 1100 01  DIVU_L, DIVS_L			*/
	i_movem_ext,	/* 0100 1100 10  MOVEM.W			*/
	i_movem_ext,	/* 0100 1100 11  MOVEM.L			*/

	i_chk,		/* 0100 1101 00  CHK.L				*/
	i_illegal,	/* 0100 1101 01  				*/
	i_chk,		/* 0100 1101 10  CHK.W				*/
	i_lea_extb,	/* 0100 1101 11  LEA				*/

	i_illegal,	/* 0100 1110 00  				*/
	i_misc,		/* 0100 1110 01  TRAP, LINK, UNLK, MOVE_USP,	*/
			/*               RESET, NOP, STOP, RTE, RTD,	*/
			/*               RTS, TRAPV, RTR, MOVEC		*/
	i_jsr_jmp,	/* 0100 1110 10  JSR				*/
	i_jsr_jmp,	/* 0100 1110 11  JMP				*/

	i_chk,		/* 0100 1111 00  CHK.L				*/
	i_illegal,	/* 0100 1111 01  				*/
	i_chk,		/* 0100 1111 10  CHK.W				*/
	i_lea_extb,	/* 0100 1111 11  LEA				*/

	i_addq,		/* 0101 0000 00  ADDQ.B				*/
	i_addq,		/* 0101 0000 01  ADDQ.W				*/
	i_addq,		/* 0101 0000 10  ADDQ.L				*/
	i_db_trap_s_cc,	/* 0101 0000 11  DBcc, TRAPcc, Scc		*/

	i_subq,		/* 0101 0001 00  SUBQ.B				*/
	i_subq,		/* 0101 0001 01  SUBQ.W				*/
	i_subq,		/* 0101 0001 10  SUBQ.L				*/
	i_db_trap_s_cc,	/* 0101 0001 11  DBcc, TRAPcc, Scc		*/

	i_addq,		/* 0101 0010 00  ADDQ.B				*/
	i_addq,		/* 0101 0010 01  ADDQ.W				*/
	i_addq,		/* 0101 0010 10  ADDQ.L				*/
	i_db_trap_s_cc,	/* 0101 0010 11  DBcc, TRAPcc, Scc		*/

	i_subq,		/* 0101 0011 00  SUBQ.B				*/
	i_subq,		/* 0101 0011 01  SUBQ.W				*/
	i_subq,		/* 0101 0011 10  SUBQ.L				*/
	i_db_trap_s_cc,	/* 0101 0011 11  DBcc, TRAPcc, Scc		*/

	i_addq,		/* 0101 0100 00  ADDQ.B				*/
	i_addq,		/* 0101 0100 01  ADDQ.W				*/
	i_addq,		/* 0101 0100 10  ADDQ.L				*/
	i_db_trap_s_cc,	/* 0101 0100 11  DBcc, TRAPcc, Scc		*/

	i_subq,		/* 0101 0101 00  SUBQ.B				*/
	i_subq,		/* 0101 0101 01  SUBQ.W				*/
	i_subq,		/* 0101 0101 10  SUBQ.L				*/
	i_db_trap_s_cc,	/* 0101 0101 11  DBcc, TRAPcc, Scc		*/

	i_addq,		/* 0101 0110 00  ADDQ.B				*/
	i_addq,		/* 0101 0110 01  ADDQ.W				*/
	i_addq,		/* 0101 0110 10  ADDQ.L				*/
	i_db_trap_s_cc,	/* 0101 0110 11  DBcc, TRAPcc, Scc		*/

	i_subq,		/* 0101 0111 00  SUBQ.B				*/
	i_subq,		/* 0101 0111 01  SUBQ.W				*/
	i_subq,		/* 0101 0111 10  SUBQ.L				*/
	i_db_trap_s_cc,	/* 0101 0111 11  DBcc, TRAPcc, Scc		*/

	i_addq,		/* 0101 1000 00  ADDQ.B				*/
	i_addq,		/* 0101 1000 01  ADDQ.W				*/
	i_addq,		/* 0101 1000 10  ADDQ.L				*/
	i_db_trap_s_cc,	/* 0101 1000 11  DBcc, TRAPcc, Scc		*/

	i_subq,		/* 0101 1001 00  SUBQ.B				*/
	i_subq,		/* 0101 1001 01  SUBQ.W				*/
	i_subq,		/* 0101 1001 10  SUBQ.L				*/
	i_db_trap_s_cc,	/* 0101 1001 11  DBcc, TRAPcc, Scc		*/

	i_addq,		/* 0101 1010 00  ADDQ.B				*/
	i_addq,		/* 0101 1010 01  ADDQ.W				*/
	i_addq,		/* 0101 1010 10  ADDQ.L				*/
	i_db_trap_s_cc,	/* 0101 1010 11  DBcc, TRAPcc, Scc		*/

	i_subq,		/* 0101 1011 00  SUBQ.B				*/
	i_subq,		/* 0101 1011 01  SUBQ.W				*/
	i_subq,		/* 0101 1011 10  SUBQ.L				*/
	i_db_trap_s_cc,	/* 0101 1011 11  DBcc, TRAPcc, Scc		*/

	i_addq,		/* 0101 1100 00  ADDQ.B				*/
	i_addq,		/* 0101 1100 01  ADDQ.W				*/
	i_addq,		/* 0101 1100 10  ADDQ.L				*/
	i_db_trap_s_cc,	/* 0101 1100 11  DBcc, TRAPcc, Scc		*/

	i_subq,		/* 0101 1101 00  SUBQ.B				*/
	i_subq,		/* 0101 1101 01  SUBQ.W				*/
	i_subq,		/* 0101 1101 10  SUBQ.L				*/
	i_db_trap_s_cc,	/* 0101 1101 11  DBcc, TRAPcc, Scc		*/

	i_addq,		/* 0101 1110 00  ADDQ.B				*/
	i_addq,		/* 0101 1110 01  ADDQ.W				*/
	i_addq,		/* 0101 1110 10  ADDQ.L				*/
	i_db_trap_s_cc,	/* 0101 1110 11  DBcc, TRAPcc, Scc		*/

	i_subq,		/* 0101 1111 00  SUBQ.B				*/
	i_subq,		/* 0101 1111 01  SUBQ.W				*/
	i_subq,		/* 0101 1111 10  SUBQ.L				*/
	i_db_trap_s_cc,	/* 0101 1111 11  DBcc, TRAPcc, Scc		*/

	i_bra_bsr_bcc,	/* 0110 0000 00  BRA				*/
	i_bra_bsr_bcc,	/* 0110 0000 01  BRA				*/
	i_bra_bsr_bcc,	/* 0110 0000 10  BRA				*/
	i_bra_bsr_bcc,	/* 0110 0000 11  BRA				*/

	i_bra_bsr_bcc,	/* 0110 0001 00  BSR				*/
	i_bra_bsr_bcc,	/* 0110 0001 01  BSR				*/
	i_bra_bsr_bcc,	/* 0110 0001 10  BSR				*/
	i_bra_bsr_bcc,	/* 0110 0001 11  BSR				*/

	i_bra_bsr_bcc,	/* 0110 0010 00  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 0010 01  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 0010 10  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 0010 11  Bcc				*/

	i_bra_bsr_bcc,	/* 0110 0011 00  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 0011 01  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 0011 10  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 0011 11  Bcc				*/

	i_bra_bsr_bcc,	/* 0110 0100 00  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 0100 01  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 0100 10  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 0100 11  Bcc				*/

	i_bra_bsr_bcc,	/* 0110 0101 00  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 0101 01  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 0101 10  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 0101 11  Bcc				*/

	i_bra_bsr_bcc,	/* 0110 0110 00  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 0110 01  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 0110 10  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 0110 11  Bcc				*/

	i_bra_bsr_bcc,	/* 0110 0111 00  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 0111 01  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 0111 10  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 0111 11  Bcc				*/

	i_bra_bsr_bcc,	/* 0110 1000 00  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 1000 01  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 1000 10  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 1000 11  Bcc				*/

	i_bra_bsr_bcc,	/* 0110 1001 00  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 1001 01  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 1001 10  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 1001 11  Bcc				*/

	i_bra_bsr_bcc,	/* 0110 1010 00  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 1010 01  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 1010 10  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 1010 11  Bcc				*/

	i_bra_bsr_bcc,	/* 0110 1011 00  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 1011 01  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 1011 10  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 1011 11  Bcc				*/

	i_bra_bsr_bcc,	/* 0110 1100 00  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 1100 01  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 1100 10  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 1100 11  Bcc				*/

	i_bra_bsr_bcc,	/* 0110 1101 00  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 1101 01  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 1101 10  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 1101 11  Bcc				*/

	i_bra_bsr_bcc,	/* 0110 1110 00  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 1110 01  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 1110 10  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 1110 11  Bcc				*/

	i_bra_bsr_bcc,	/* 0110 1111 00  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 1111 01  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 1111 10  Bcc				*/
	i_bra_bsr_bcc,	/* 0110 1111 11  Bcc				*/

	i_moveq,	/* 0111 0000 00  MOVEQ				*/
	i_moveq,	/* 0111 0000 01  MOVEQ				*/
	i_moveq,	/* 0111 0000 10  MOVEQ				*/
	i_moveq,	/* 0111 0000 11  MOVEQ				*/

	i_illegal,	/* 0111 0001 00  				*/
	i_illegal,	/* 0111 0001 01  				*/
	i_illegal,	/* 0111 0001 10  				*/
	i_illegal,	/* 0111 0001 11  				*/

	i_moveq,	/* 0111 0010 00  MOVEQ				*/
	i_moveq,	/* 0111 0010 01  MOVEQ				*/
	i_moveq,	/* 0111 0010 10  MOVEQ				*/
	i_moveq,	/* 0111 0010 11  MOVEQ				*/

	i_illegal,	/* 0111 0011 00  				*/
	i_illegal,	/* 0111 0011 01  				*/
	i_illegal,	/* 0111 0011 10  				*/
	i_illegal,	/* 0111 0011 11  				*/

	i_moveq,	/* 0111 0100 00  MOVEQ				*/
	i_moveq,	/* 0111 0100 01  MOVEQ				*/
	i_moveq,	/* 0111 0100 10  MOVEQ				*/
	i_moveq,	/* 0111 0100 11  MOVEQ				*/

	i_illegal,	/* 0111 0101 00  				*/
	i_illegal,	/* 0111 0101 01  				*/
	i_illegal,	/* 0111 0101 10  				*/
	i_illegal,	/* 0111 0101 11  				*/

	i_moveq,	/* 0111 0110 00  MOVEQ				*/
	i_moveq,	/* 0111 0110 01  MOVEQ				*/
	i_moveq,	/* 0111 0110 10  MOVEQ				*/
	i_moveq,	/* 0111 0110 11  MOVEQ				*/

	i_illegal,	/* 0111 0111 00  				*/
	i_illegal,	/* 0111 0111 01  				*/
	i_illegal,	/* 0111 0111 10  				*/
	i_illegal,	/* 0111 0111 11  				*/

	i_moveq,	/* 0111 1000 00  MOVEQ				*/
	i_moveq,	/* 0111 1000 01  MOVEQ				*/
	i_moveq,	/* 0111 1000 10  MOVEQ				*/
	i_moveq,	/* 0111 1000 11  MOVEQ				*/

	i_illegal,	/* 0111 1001 00  				*/
	i_illegal,	/* 0111 1001 01  				*/
	i_illegal,	/* 0111 1001 10  				*/
	i_illegal,	/* 0111 1001 11  				*/

	i_moveq,	/* 0111 1010 00  MOVEQ				*/
	i_moveq,	/* 0111 1010 01  MOVEQ				*/
	i_moveq,	/* 0111 1010 10  MOVEQ				*/
	i_moveq,	/* 0111 1010 11  MOVEQ				*/

	i_illegal,	/* 0111 1011 00  				*/
	i_illegal,	/* 0111 1011 01  				*/
	i_illegal,	/* 0111 1011 10  				*/
	i_illegal,	/* 0111 1011 11  				*/

	i_moveq,	/* 0111 1100 00  MOVEQ				*/
	i_moveq,	/* 0111 1100 01  MOVEQ				*/
	i_moveq,	/* 0111 1100 10  MOVEQ				*/
	i_moveq,	/* 0111 1100 11  MOVEQ				*/

	i_illegal,	/* 0111 1101 00  				*/
	i_illegal,	/* 0111 1101 01  				*/
	i_illegal,	/* 0111 1101 10  				*/
	i_illegal,	/* 0111 1101 11  				*/

	i_moveq,	/* 0111 1110 00  MOVEQ				*/
	i_moveq,	/* 0111 1110 01  MOVEQ				*/
	i_moveq,	/* 0111 1110 10  MOVEQ				*/
	i_moveq,	/* 0111 1110 11  MOVEQ				*/

	i_illegal,	/* 0111 1111 00  				*/
	i_illegal,	/* 0111 1111 01  				*/
	i_illegal,	/* 0111 1111 10  				*/
	i_illegal,	/* 0111 1111 11  				*/

	i_or_sbcd_pack,	/* 1000 0000 00  OR.B				*/
	i_or_sbcd_pack,	/* 1000 0000 01  OR.W				*/
	i_or_sbcd_pack,	/* 1000 0000 10  OR.L				*/
	i_div,		/* 1000 0000 11  DIVU				*/

	i_or_sbcd_pack,	/* 1000 0001 00  OR.B, SBCD			*/
	i_or_sbcd_pack,	/* 1000 0001 01  OR.W, PACK			*/
	i_or_sbcd_pack,	/* 1000 0001 10  OR.L, UNPK			*/
	i_div,		/* 1000 0001 11  DIVS				*/

	i_or_sbcd_pack,	/* 1000 0010 00  OR.B				*/
	i_or_sbcd_pack,	/* 1000 0010 01  OR.W				*/
	i_or_sbcd_pack,	/* 1000 0010 10  OR.L				*/
	i_div,		/* 1000 0010 11  DIVU				*/

	i_or_sbcd_pack,	/* 1000 0011 00  OR.B, SBCD			*/
	i_or_sbcd_pack,	/* 1000 0011 01  OR.W, PACK			*/
	i_or_sbcd_pack,	/* 1000 0011 10  OR.L, UNPK			*/
	i_div,		/* 1000 0011 11  DIVS				*/

	i_or_sbcd_pack,	/* 1000 0100 00  OR.B				*/
	i_or_sbcd_pack,	/* 1000 0100 01  OR.W				*/
	i_or_sbcd_pack,	/* 1000 0100 10  OR.L				*/
	i_div,		/* 1000 0100 11  DIVU				*/

	i_or_sbcd_pack,	/* 1000 0101 00  OR.B, SBCD			*/
	i_or_sbcd_pack,	/* 1000 0101 01  OR.W, PACK			*/
	i_or_sbcd_pack,	/* 1000 0101 10  OR.L, UNPK			*/
	i_div,		/* 1000 0101 11  DIVS				*/

	i_or_sbcd_pack,	/* 1000 0110 00  OR.B				*/
	i_or_sbcd_pack,	/* 1000 0110 01  OR.W				*/
	i_or_sbcd_pack,	/* 1000 0110 10  OR.L				*/
	i_div,		/* 1000 0110 11  DIVU				*/

	i_or_sbcd_pack,	/* 1000 0111 00  OR.B, SBCD			*/
	i_or_sbcd_pack,	/* 1000 0111 01  OR.W, PACK			*/
	i_or_sbcd_pack,	/* 1000 0111 10  OR.L, UNPK			*/
	i_div,		/* 1000 0111 11  DIVS				*/

	i_or_sbcd_pack,	/* 1000 1000 00  OR.B				*/
	i_or_sbcd_pack,	/* 1000 1000 01  OR.W				*/
	i_or_sbcd_pack,	/* 1000 1000 10  OR.L				*/
	i_div,		/* 1000 1000 11  DIVU				*/

	i_or_sbcd_pack,	/* 1000 1001 00  OR.B, SBCD			*/
	i_or_sbcd_pack,	/* 1000 1001 01  OR.W, PACK			*/
	i_or_sbcd_pack,	/* 1000 1001 10  OR.L, UNPK			*/
	i_div,		/* 1000 1001 11  DIVS				*/

	i_or_sbcd_pack,	/* 1000 1010 00  OR.B				*/
	i_or_sbcd_pack,	/* 1000 1010 01  OR.W				*/
	i_or_sbcd_pack,	/* 1000 1010 10  OR.L				*/
	i_div,		/* 1000 1010 11  DIVU				*/

	i_or_sbcd_pack,	/* 1000 1011 00  OR.B, SBCD			*/
	i_or_sbcd_pack,	/* 1000 1011 01  OR.W, PACK			*/
	i_or_sbcd_pack,	/* 1000 1011 10  OR.L, UNPK			*/
	i_div,		/* 1000 1011 11  DIVS				*/

	i_or_sbcd_pack,	/* 1000 1100 00  OR.B				*/
	i_or_sbcd_pack,	/* 1000 1100 01  OR.W				*/
	i_or_sbcd_pack,	/* 1000 1100 10  OR.L				*/
	i_div,		/* 1000 1100 11  DIVU				*/

	i_or_sbcd_pack,	/* 1000 1101 00  OR.B, SBCD			*/
	i_or_sbcd_pack,	/* 1000 1101 01  OR.W, PACK			*/
	i_or_sbcd_pack,	/* 1000 1101 10  OR.L, UNPK			*/
	i_div,		/* 1000 1101 11  DIVS				*/

	i_or_sbcd_pack,	/* 1000 1110 00  OR.B				*/
	i_or_sbcd_pack,	/* 1000 1110 01  OR.W				*/
	i_or_sbcd_pack,	/* 1000 1110 10  OR.L				*/
	i_div,		/* 1000 1110 11  DIVU				*/

	i_or_sbcd_pack,	/* 1000 1111 00  OR.B, SBCD			*/
	i_or_sbcd_pack,	/* 1000 1111 01  OR.W, PACK			*/
	i_or_sbcd_pack,	/* 1000 1111 10  OR.L, UNPK			*/
	i_div,		/* 1000 1111 11  DIVS				*/

	i_sub_ax,	/* 1001 0000 00  SUB.B				*/
	i_sub_ax,	/* 1001 0000 01  SUB.W				*/
	i_sub_ax,	/* 1001 0000 10  SUB.L				*/
	i_sub_ax,	/* 1001 0000 11  SUBA.W				*/

	i_sub_ax,	/* 1001 0001 00  SUB.B, SUBX.B			*/
	i_sub_ax,	/* 1001 0001 01  SUB.W, SUBX.W			*/
	i_sub_ax,	/* 1001 0001 10  SUB.L, SUBX.L			*/
	i_sub_ax,	/* 1001 0001 11  SUBA.L				*/

	i_sub_ax,	/* 1001 0010 00  SUB.B				*/
	i_sub_ax,	/* 1001 0010 01  SUB.W				*/
	i_sub_ax,	/* 1001 0010 10  SUB.L				*/
	i_sub_ax,	/* 1001 0010 11  SUBA.W				*/

	i_sub_ax,	/* 1001 0011 00  SUB.B, SUBX.B			*/
	i_sub_ax,	/* 1001 0011 01  SUB.W, SUBX.W			*/
	i_sub_ax,	/* 1001 0011 10  SUB.L, SUBX.L			*/
	i_sub_ax,	/* 1001 0011 11  SUBA.L				*/

	i_sub_ax,	/* 1001 0100 00  SUB.B				*/
	i_sub_ax,	/* 1001 0100 01  SUB.W				*/
	i_sub_ax,	/* 1001 0100 10  SUB.L				*/
	i_sub_ax,	/* 1001 0100 11  SUBA.W				*/

	i_sub_ax,	/* 1001 0101 00  SUB.B, SUBX.B			*/
	i_sub_ax,	/* 1001 0101 01  SUB.W, SUBX.W			*/
	i_sub_ax,	/* 1001 0101 10  SUB.L, SUBX.L			*/
	i_sub_ax,	/* 1001 0101 11  SUBA.L				*/

	i_sub_ax,	/* 1001 0110 00  SUB.B				*/
	i_sub_ax,	/* 1001 0110 01  SUB.W				*/
	i_sub_ax,	/* 1001 0110 10  SUB.L				*/
	i_sub_ax,	/* 1001 0110 11  SUBA.W				*/

	i_sub_ax,	/* 1001 0111 00  SUB.B, SUBX.B			*/
	i_sub_ax,	/* 1001 0111 01  SUB.W, SUBX.W			*/
	i_sub_ax,	/* 1001 0111 10  SUB.L, SUBX.L			*/
	i_sub_ax,	/* 1001 0111 11  SUBA.L				*/

	i_sub_ax,	/* 1001 1000 00  SUB.B				*/
	i_sub_ax,	/* 1001 1000 01  SUB.W				*/
	i_sub_ax,	/* 1001 1000 10  SUB.L				*/
	i_sub_ax,	/* 1001 1000 11  SUBA.W				*/

	i_sub_ax,	/* 1001 1001 00  SUB.B, SUBX.B			*/
	i_sub_ax,	/* 1001 1001 01  SUB.W, SUBX.W			*/
	i_sub_ax,	/* 1001 1001 10  SUB.L, SUBX.L			*/
	i_sub_ax,	/* 1001 1001 11  SUBA.L				*/

	i_sub_ax,	/* 1001 1010 00  SUB.B				*/
	i_sub_ax,	/* 1001 1010 01  SUB.W				*/
	i_sub_ax,	/* 1001 1010 10  SUB.L				*/
	i_sub_ax,	/* 1001 1010 11  SUBA.W				*/

	i_sub_ax,	/* 1001 1011 00  SUB.B, SUBX.B			*/
	i_sub_ax,	/* 1001 1011 01  SUB.W, SUBX.W			*/
	i_sub_ax,	/* 1001 1011 10  SUB.L, SUBX.L			*/
	i_sub_ax,	/* 1001 1011 11  SUBA.L				*/

	i_sub_ax,	/* 1001 1100 00  SUB.B				*/
	i_sub_ax,	/* 1001 1100 01  SUB.W				*/
	i_sub_ax,	/* 1001 1100 10  SUB.L				*/
	i_sub_ax,	/* 1001 1100 11  SUBA.W				*/

	i_sub_ax,	/* 1001 1101 00  SUB.B, SUBX.B			*/
	i_sub_ax,	/* 1001 1101 01  SUB.W, SUBX.W			*/
	i_sub_ax,	/* 1001 1101 10  SUB.L, SUBX.L			*/
	i_sub_ax,	/* 1001 1101 11  SUBA.L				*/

	i_sub_ax,	/* 1001 1110 00  SUB.B				*/
	i_sub_ax,	/* 1001 1110 01  SUB.W				*/
	i_sub_ax,	/* 1001 1110 10  SUB.L				*/
	i_sub_ax,	/* 1001 1110 11  SUBA.W				*/

	i_sub_ax,	/* 1001 1111 00  SUB.B, SUBX.B			*/
	i_sub_ax,	/* 1001 1111 01  SUB.W, SUBX.W			*/
	i_sub_ax,	/* 1001 1111 10  SUB.L, SUBX.L			*/
	i_sub_ax,	/* 1001 1111 11  SUBA.L				*/

	i_line_a,	/* 1010 0000 00  				*/
	i_line_a,	/* 1010 0000 01  				*/
	i_line_a,	/* 1010 0000 10  				*/
	i_line_a,	/* 1010 0000 11  				*/

	i_line_a,	/* 1010 0001 00  				*/
	i_line_a,	/* 1010 0001 01  				*/
	i_line_a,	/* 1010 0001 10  				*/
	i_line_a,	/* 1010 0001 11  				*/

	i_line_a,	/* 1010 0010 00  				*/
	i_line_a,	/* 1010 0010 01  				*/
	i_line_a,	/* 1010 0010 10  				*/
	i_line_a,	/* 1010 0010 11  				*/

	i_line_a,	/* 1010 0011 00  				*/
	i_line_a,	/* 1010 0011 01  				*/
	i_line_a,	/* 1010 0011 10  				*/
	i_line_a,	/* 1010 0011 11  				*/

	i_line_a,	/* 1010 0100 00  				*/
	i_line_a,	/* 1010 0100 01  				*/
	i_line_a,	/* 1010 0100 10  				*/
	i_line_a,	/* 1010 0100 11  				*/

	i_line_a,	/* 1010 0101 00  				*/
	i_line_a,	/* 1010 0101 01  				*/
	i_line_a,	/* 1010 0101 10  				*/
	i_line_a,	/* 1010 0101 11  				*/

	i_line_a,	/* 1010 0110 00  				*/
	i_line_a,	/* 1010 0110 01  				*/
	i_line_a,	/* 1010 0110 10  				*/
	i_line_a,	/* 1010 0110 11  				*/

	i_line_a,	/* 1010 0111 00  				*/
	i_line_a,	/* 1010 0111 01  				*/
	i_line_a,	/* 1010 0111 10  				*/
	i_line_a,	/* 1010 0111 11  				*/

	i_line_a,	/* 1010 1000 00  				*/
	i_line_a,	/* 1010 1000 01  				*/
	i_line_a,	/* 1010 1000 10  				*/
	i_line_a,	/* 1010 1000 11  				*/

	i_line_a,	/* 1010 1001 00  				*/
	i_line_a,	/* 1010 1001 01  				*/
	i_line_a,	/* 1010 1001 10  				*/
	i_line_a,	/* 1010 1001 11  				*/

	i_line_a,	/* 1010 1010 00  				*/
	i_line_a,	/* 1010 1010 01  				*/
	i_line_a,	/* 1010 1010 10  				*/
	i_line_a,	/* 1010 1010 11  				*/

	i_line_a,	/* 1010 1011 00  				*/
	i_line_a,	/* 1010 1011 01  				*/
	i_line_a,	/* 1010 1011 10  				*/
	i_line_a,	/* 1010 1011 11  				*/

	i_line_a,	/* 1010 1100 00  				*/
	i_line_a,	/* 1010 1100 01  				*/
	i_line_a,	/* 1010 1100 10  				*/
	i_line_a,	/* 1010 1100 11  				*/

	i_line_a,	/* 1010 1101 00  				*/
	i_line_a,	/* 1010 1101 01  				*/
	i_line_a,	/* 1010 1101 10  				*/
	i_line_a,	/* 1010 1101 11  				*/

	i_line_a,	/* 1010 1110 00  				*/
	i_line_a,	/* 1010 1110 01  				*/
	i_line_a,	/* 1010 1110 10  				*/
	i_line_a,	/* 1010 1110 11  				*/

	i_line_a,	/* 1010 1111 00  				*/
	i_line_a,	/* 1010 1111 01  				*/
	i_line_a,	/* 1010 1111 10  				*/
	i_line_a,	/* 1010 1111 11  				*/

	i_cmp_a,	/* 1011 0000 00  CMP.B				*/
	i_cmp_a,	/* 1011 0000 01  CMP.W				*/
	i_cmp_a,	/* 1011 0000 10  CMP.L				*/
	i_cmp_a,	/* 1011 0000 11  CMPA.W				*/

	i_eor_cmpm,	/* 1011 0001 00  EOR.B, CMPM.B			*/
	i_eor_cmpm,	/* 1011 0001 01  EOR.W, CMPM.W			*/
	i_eor_cmpm,	/* 1011 0001 10  EOR.L, CMPM.L			*/
	i_cmp_a,	/* 1011 0001 11  CMPA.L				*/

	i_cmp_a,	/* 1011 0010 00  CMP.B				*/
	i_cmp_a,	/* 1011 0010 01  CMP.W				*/
	i_cmp_a,	/* 1011 0010 10  CMP.L				*/
	i_cmp_a,	/* 1011 0010 11  CMPA.W				*/

	i_eor_cmpm,	/* 1011 0011 00  EOR.B, CMPM.B			*/
	i_eor_cmpm,	/* 1011 0011 01  EOR.W, CMPM.W			*/
	i_eor_cmpm,	/* 1011 0011 10  EOR.L, CMPM.L			*/
	i_cmp_a,	/* 1011 0011 11  CMPA.L				*/

	i_cmp_a,	/* 1011 0100 00  CMP.B				*/
	i_cmp_a,	/* 1011 0100 01  CMP.W				*/
	i_cmp_a,	/* 1011 0100 10  CMP.L				*/
	i_cmp_a,	/* 1011 0100 11  CMPA.W				*/

	i_eor_cmpm,	/* 1011 0101 00  EOR.B, CMPM.B			*/
	i_eor_cmpm,	/* 1011 0101 01  EOR.W, CMPM.W			*/
	i_eor_cmpm,	/* 1011 0101 10  EOR.L, CMPM.L			*/
	i_cmp_a,	/* 1011 0101 11  CMPA.L				*/

	i_cmp_a,	/* 1011 0110 00  CMP.B				*/
	i_cmp_a,	/* 1011 0110 01  CMP.W				*/
	i_cmp_a,	/* 1011 0110 10  CMP.L				*/
	i_cmp_a,	/* 1011 0110 11  CMPA.W				*/

	i_eor_cmpm,	/* 1011 0111 00  EOR.B, CMPM.B			*/
	i_eor_cmpm,	/* 1011 0111 01  EOR.W, CMPM.W			*/
	i_eor_cmpm,	/* 1011 0111 10  EOR.L, CMPM.L			*/
	i_cmp_a,	/* 1011 0111 11  CMPA.L				*/

	i_cmp_a,	/* 1011 1000 00  CMP.B				*/
	i_cmp_a,	/* 1011 1000 01  CMP.W				*/
	i_cmp_a,	/* 1011 1000 10  CMP.L				*/
	i_cmp_a,	/* 1011 1000 11  CMPA.W				*/

	i_eor_cmpm,	/* 1011 1001 00  EOR.B, CMPM.B			*/
	i_eor_cmpm,	/* 1011 1001 01  EOR.W, CMPM.W			*/
	i_eor_cmpm,	/* 1011 1001 10  EOR.L, CMPM.L			*/
	i_cmp_a,	/* 1011 1001 11  CMPA.L				*/

	i_cmp_a,	/* 1011 1010 00  CMP.B				*/
	i_cmp_a,	/* 1011 1010 01  CMP.W				*/
	i_cmp_a,	/* 1011 1010 10  CMP.L				*/
	i_cmp_a,	/* 1011 1010 11  CMPA.W				*/

	i_eor_cmpm,	/* 1011 1011 00  EOR.B, CMPM.B			*/
	i_eor_cmpm,	/* 1011 1011 01  EOR.W, CMPM.W			*/
	i_eor_cmpm,	/* 1011 1011 10  EOR.L, CMPM.L			*/
	i_cmp_a,	/* 1011 1011 11  CMPA.L				*/

	i_cmp_a,	/* 1011 1100 00  CMP.B				*/
	i_cmp_a,	/* 1011 1100 01  CMP.W				*/
	i_cmp_a,	/* 1011 1100 10  CMP.L				*/
	i_cmp_a,	/* 1011 1100 11  CMPA.W				*/

	i_eor_cmpm,	/* 1011 1101 00  EOR.B, CMPM.B			*/
	i_eor_cmpm,	/* 1011 1101 01  EOR.W, CMPM.W			*/
	i_eor_cmpm,	/* 1011 1101 10  EOR.L, CMPM.L			*/
	i_cmp_a,	/* 1011 1101 11  CMPA.L				*/

	i_cmp_a,	/* 1011 1110 00  CMP.B				*/
	i_cmp_a,	/* 1011 1110 01  CMP.W				*/
	i_cmp_a,	/* 1011 1110 10  CMP.L				*/
	i_cmp_a,	/* 1011 1110 11  CMPA.W				*/

	i_eor_cmpm,	/* 1011 1111 00  EOR.B, CMPM.B			*/
	i_eor_cmpm,	/* 1011 1111 01  EOR.W, CMPM.W			*/
	i_eor_cmpm,	/* 1011 1111 10  EOR.L, CMPM.L			*/
	i_cmp_a,	/* 1011 1111 11  CMPA.L				*/

	i_and_abcd_exg,	/* 1100 0000 00  AND.B				*/
	i_and_abcd_exg,	/* 1100 0000 01  AND.W				*/
	i_and_abcd_exg,	/* 1100 0000 10  AND.L				*/
	i_mul,		/* 1100 0000 11  MULU				*/

	i_and_abcd_exg,	/* 1100 0001 00  AND.B,	ABCD			*/
	i_and_abcd_exg,	/* 1100 0001 01  AND.W, EXG			*/
	i_and_abcd_exg,	/* 1100 0001 10  AND.L, EXG			*/
	i_mul,		/* 1100 0001 11  MULS				*/

	i_and_abcd_exg,	/* 1100 0010 00  AND.B				*/
	i_and_abcd_exg,	/* 1100 0010 01  AND.W				*/
	i_and_abcd_exg,	/* 1100 0010 10  AND.L				*/
	i_mul,		/* 1100 0010 11  MULU				*/

	i_and_abcd_exg,	/* 1100 0011 00  AND.B,	ABCD			*/
	i_and_abcd_exg,	/* 1100 0011 01  AND.W, EXG			*/
	i_and_abcd_exg,	/* 1100 0011 10  AND.L, EXG			*/
	i_mul,		/* 1100 0011 11  MULS				*/

	i_and_abcd_exg,	/* 1100 0100 00  AND.B				*/
	i_and_abcd_exg,	/* 1100 0100 01  AND.W				*/
	i_and_abcd_exg,	/* 1100 0100 10  AND.L				*/
	i_mul,		/* 1100 0100 11  MULU				*/

	i_and_abcd_exg,	/* 1100 0101 00  AND.B,	ABCD			*/
	i_and_abcd_exg,	/* 1100 0101 01  AND.W, EXG			*/
	i_and_abcd_exg,	/* 1100 0101 10  AND.L, EXG			*/
	i_mul,		/* 1100 0101 11  MULS				*/

	i_and_abcd_exg,	/* 1100 0110 00  AND.B				*/
	i_and_abcd_exg,	/* 1100 0110 01  AND.W				*/
	i_and_abcd_exg,	/* 1100 0110 10  AND.L				*/
	i_mul,		/* 1100 0110 11  MULU				*/

	i_and_abcd_exg,	/* 1100 0111 00  AND.B,	ABCD			*/
	i_and_abcd_exg,	/* 1100 0111 01  AND.W, EXG			*/
	i_and_abcd_exg,	/* 1100 0111 10  AND.L, EXG			*/
	i_mul,		/* 1100 0111 11  MULS				*/

	i_and_abcd_exg,	/* 1100 1000 00  AND.B				*/
	i_and_abcd_exg,	/* 1100 1000 01  AND.W				*/
	i_and_abcd_exg,	/* 1100 1000 10  AND.L				*/
	i_mul,		/* 1100 1000 11  MULU				*/

	i_and_abcd_exg,	/* 1100 1001 00  AND.B,	ABCD			*/
	i_and_abcd_exg,	/* 1100 1001 01  AND.W, EXG			*/
	i_and_abcd_exg,	/* 1100 1001 10  AND.L, EXG			*/
	i_mul,		/* 1100 1001 11  MULS				*/

	i_and_abcd_exg,	/* 1100 1010 00  AND.B				*/
	i_and_abcd_exg,	/* 1100 1010 01  AND.W				*/
	i_and_abcd_exg,	/* 1100 1010 10  AND.L				*/
	i_mul,		/* 1100 1010 11  MULU				*/

	i_and_abcd_exg,	/* 1100 1011 00  AND.B,	ABCD			*/
	i_and_abcd_exg,	/* 1100 1011 01  AND.W, EXG			*/
	i_and_abcd_exg,	/* 1100 1011 10  AND.L, EXG			*/
	i_mul,		/* 1100 1011 11  MULS				*/

	i_and_abcd_exg,	/* 1100 1100 00  AND.B				*/
	i_and_abcd_exg,	/* 1100 1100 01  AND.W				*/
	i_and_abcd_exg,	/* 1100 1100 10  AND.L				*/
	i_mul,		/* 1100 1100 11  MULU				*/

	i_and_abcd_exg,	/* 1100 1101 00  AND.B,	ABCD			*/
	i_and_abcd_exg,	/* 1100 1101 01  AND.W, EXG			*/
	i_and_abcd_exg,	/* 1100 1101 10  AND.L, EXG			*/
	i_mul,		/* 1100 1101 11  MULS				*/

	i_and_abcd_exg,	/* 1100 1110 00  AND.B				*/
	i_and_abcd_exg,	/* 1100 1110 01  AND.W				*/
	i_and_abcd_exg,	/* 1100 1110 10  AND.L				*/
	i_mul,		/* 1100 1110 11  MULU				*/

	i_and_abcd_exg,	/* 1100 1111 00  AND.B,	ABCD			*/
	i_and_abcd_exg,	/* 1100 1111 01  AND.W, EXG			*/
	i_and_abcd_exg,	/* 1100 1111 10  AND.L, EXG			*/
	i_mul,		/* 1100 1111 11  MULS				*/

	i_add_ax,	/* 1101 0000 00  ADD.B				*/
	i_add_ax,	/* 1101 0000 01  ADD.W				*/
	i_add_ax,	/* 1101 0000 10  ADD.L				*/
	i_add_ax,	/* 1101 0000 11  ADDA.W				*/

	i_add_ax,	/* 1101 0001 00  ADD.B, ADDX.B			*/
	i_add_ax,	/* 1101 0001 01  ADD.W, ADDX.W			*/
	i_add_ax,	/* 1101 0001 10  ADD.L, ADDX.L			*/
	i_add_ax,	/* 1101 0001 11  ADDA.L				*/

	i_add_ax,	/* 1101 0010 00  ADD.B				*/
	i_add_ax,	/* 1101 0010 01  ADD.W				*/
	i_add_ax,	/* 1101 0010 10  ADD.L				*/
	i_add_ax,	/* 1101 0010 11  ADDA.W				*/

	i_add_ax,	/* 1101 0011 00  ADD.B, ADDX.B			*/
	i_add_ax,	/* 1101 0011 01  ADD.W, ADDX.W			*/
	i_add_ax,	/* 1101 0011 10  ADD.L, ADDX.L			*/
	i_add_ax,	/* 1101 0011 11  ADDA.L				*/

	i_add_ax,	/* 1101 0100 00  ADD.B				*/
	i_add_ax,	/* 1101 0100 01  ADD.W				*/
	i_add_ax,	/* 1101 0100 10  ADD.L				*/
	i_add_ax,	/* 1101 0100 11  ADDA.W				*/

	i_add_ax,	/* 1101 0101 00  ADD.B, ADDX.B			*/
	i_add_ax,	/* 1101 0101 01  ADD.W, ADDX.W			*/
	i_add_ax,	/* 1101 0101 10  ADD.L, ADDX.L			*/
	i_add_ax,	/* 1101 0101 11  ADDA.L				*/

	i_add_ax,	/* 1101 0110 00  ADD.B				*/
	i_add_ax,	/* 1101 0110 01  ADD.W				*/
	i_add_ax,	/* 1101 0110 10  ADD.L				*/
	i_add_ax,	/* 1101 0110 11  ADDA.W				*/

	i_add_ax,	/* 1101 0111 00  ADD.B, ADDX.B			*/
	i_add_ax,	/* 1101 0111 01  ADD.W, ADDX.W			*/
	i_add_ax,	/* 1101 0111 10  ADD.L, ADDX.L			*/
	i_add_ax,	/* 1101 0111 11  ADDA.L				*/

	i_add_ax,	/* 1101 1000 00  ADD.B				*/
	i_add_ax,	/* 1101 1000 01  ADD.W				*/
	i_add_ax,	/* 1101 1000 10  ADD.L				*/
	i_add_ax,	/* 1101 1000 11  ADDA.W				*/

	i_add_ax,	/* 1101 1001 00  ADD.B, ADDX.B			*/
	i_add_ax,	/* 1101 1001 01  ADD.W, ADDX.W			*/
	i_add_ax,	/* 1101 1001 10  ADD.L, ADDX.L			*/
	i_add_ax,	/* 1101 1001 11  ADDA.L				*/

	i_add_ax,	/* 1101 1010 00  ADD.B				*/
	i_add_ax,	/* 1101 1010 01  ADD.W				*/
	i_add_ax,	/* 1101 1010 10  ADD.L				*/
	i_add_ax,	/* 1101 1010 11  ADDA.W				*/

	i_add_ax,	/* 1101 1011 00  ADD.B, ADDX.B			*/
	i_add_ax,	/* 1101 1011 01  ADD.W, ADDX.W			*/
	i_add_ax,	/* 1101 1011 10  ADD.L, ADDX.L			*/
	i_add_ax,	/* 1101 1011 11  ADDA.L				*/

	i_add_ax,	/* 1101 1100 00  ADD.B				*/
	i_add_ax,	/* 1101 1100 01  ADD.W				*/
	i_add_ax,	/* 1101 1100 10  ADD.L				*/
	i_add_ax,	/* 1101 1100 11  ADDA.W				*/

	i_add_ax,	/* 1101 1101 00  ADD.B, ADDX.B			*/
	i_add_ax,	/* 1101 1101 01  ADD.W, ADDX.W			*/
	i_add_ax,	/* 1101 1101 10  ADD.L, ADDX.L			*/
	i_add_ax,	/* 1101 1101 11  ADDA.L				*/

	i_add_ax,	/* 1101 1110 00  ADD.B				*/
	i_add_ax,	/* 1101 1110 01  ADD.W				*/
	i_add_ax,	/* 1101 1110 10  ADD.L				*/
	i_add_ax,	/* 1101 1110 11  ADDA.W				*/

	i_add_ax,	/* 1101 1111 00  ADD.B, ADDX.B			*/
	i_add_ax,	/* 1101 1111 01  ADD.W, ADDX.W			*/
	i_add_ax,	/* 1101 1111 10  ADD.L, ADDX.L			*/
	i_add_ax,	/* 1101 1111 11  ADDA.L				*/

	i_sh,		/* 1110 0000 00  ASR.B, LSR.B, ROR.B, ROXR.B	*/
	i_sh,		/* 1110 0000 01  ASR.W, LSR.W, ROR.W, ROXR.W	*/
	i_sh,		/* 1110 0000 10  ASR.L, LSR.L, ROR.L, ROXR.L	*/
	i_sh_m,		/* 1110 0000 11  ASR_M				*/

	i_sh,		/* 1110 0001 00  ASL.B, LSL.B, ROL.B, ROXL.B	*/
	i_sh,		/* 1110 0001 01  ASL.W, LSL.W, ROL.W, ROXL.W	*/
	i_sh,		/* 1110 0001 10  ASL.L, LSL.L, ROL.L, ROXL.L	*/
	i_sh_m,		/* 1110 0001 11  ASL_M				*/

	i_sh,		/* 1110 0010 00  ASR.B, LSR.B, ROR.B, ROXR.B	*/
	i_sh,		/* 1110 0010 01  ASR.W, LSR.W, ROR.W, ROXR.W	*/
	i_sh,		/* 1110 0010 10  ASR.L, LSR.L, ROR.L, ROXR.L	*/
	i_sh_m,		/* 1110 0010 11  LSR_M				*/

	i_sh,		/* 1110 0011 00  ASL.B, LSL.B, ROL.B, ROXL.B	*/
	i_sh,		/* 1110 0011 01  ASL.W, LSL.W, ROL.W, ROXL.W	*/
	i_sh,		/* 1110 0011 10  ASL.L, LSL.L, ROL.L, ROXL.L	*/
	i_sh_m,		/* 1110 0011 11  LSL_M				*/

	i_sh,		/* 1110 0100 00  ASR.B, LSR.B, ROR.B, ROXR.B	*/
	i_sh,		/* 1110 0100 01  ASR.W, LSR.W, ROR.W, ROXR.W	*/
	i_sh,		/* 1110 0100 10  ASR.L, LSR.L, ROR.L, ROXR.L	*/
	i_sh_m,		/* 1110 0100 11  ROXR_M				*/

	i_sh,		/* 1110 0101 00  ASL.B, LSL.B, ROL.B, ROXL.B	*/
	i_sh,		/* 1110 0101 01  ASL.W, LSL.W, ROL.W, ROXL.W	*/
	i_sh,		/* 1110 0101 10  ASL.L, LSL.L, ROL.L, ROXL.L	*/
	i_sh_m,		/* 1110 0101 11  ROXL_M				*/

	i_sh,		/* 1110 0110 00  ASR.B, LSR.B, ROR.B, ROXR.B	*/
	i_sh,		/* 1110 0110 01  ASR.W, LSR.W, ROR.W, ROXR.W	*/
	i_sh,		/* 1110 0110 10  ASR.L, LSR.L, ROR.L, ROXR.L	*/
	i_sh_m,		/* 1110 0110 11  ROR_M				*/

	i_sh,		/* 1110 0111 00  ASL.B, LSL.B, ROL.B, ROXL.B	*/
	i_sh,		/* 1110 0111 01  ASL.W, LSL.W, ROL.W, ROXL.W	*/
	i_sh,		/* 1110 0111 10  ASL.L, LSL.L, ROL.L, ROXL.L	*/
	i_sh_m,		/* 1110 0111 11  ROL_M				*/

	i_sh,		/* 1110 1000 00  ASR.B, LSR.B, ROR.B, ROXR.B	*/
	i_sh,		/* 1110 1000 01  ASR.W, LSR.W, ROR.W, ROXR.W	*/
	i_sh,		/* 1110 1000 10  ASR.L, LSR.L, ROR.L, ROXR.L	*/
	i_bfop,		/* 1110 1000 11  BFTST				*/

	i_sh,		/* 1110 1001 00  ASL.B, LSL.B, ROL.B, ROXL.B	*/
	i_sh,		/* 1110 1001 01  ASL.W, LSL.W, ROL.W, ROXL.W	*/
	i_sh,		/* 1110 1001 10  ASL.L, LSL.L, ROL.L, ROXL.L	*/
	i_bfop,		/* 1110 1001 11  BFEXTU				*/

	i_sh,		/* 1110 1010 00  ASR.B, LSR.B, ROR.B, ROXR.B	*/
	i_sh,		/* 1110 1010 01  ASR.W, LSR.W, ROR.W, ROXR.W	*/
	i_sh,		/* 1110 1010 10  ASR.L, LSR.L, ROR.L, ROXR.L	*/
	i_bfop,		/* 1110 1010 11  BFCHG				*/

	i_sh,		/* 1110 1011 00  ASL.B, LSL.B, ROL.B, ROXL.B	*/
	i_sh,		/* 1110 1011 01  ASL.W, LSL.W, ROL.W, ROXL.W	*/
	i_sh,		/* 1110 1011 10  ASL.L, LSL.L, ROL.L, ROXL.L	*/
	i_bfop,		/* 1110 1011 11  BXEXTS				*/

	i_sh,		/* 1110 1100 00  ASR.B, LSR.B, ROR.B, ROXR.B	*/
	i_sh,		/* 1110 1100 01  ASR.W, LSR.W, ROR.W, ROXR.W	*/
	i_sh,		/* 1110 1100 10  ASR.L, LSR.L, ROR.L, ROXR.L	*/
	i_bfop,		/* 1110 1100 11  BFCLR				*/

	i_sh,		/* 1110 1101 00  ASL.B, LSL.B, ROL.B, ROXL.B	*/
	i_sh,		/* 1110 1101 01  ASL.W, LSL.W, ROL.W, ROXL.W	*/
	i_sh,		/* 1110 1101 10  ASL.L, LSL.L, ROL.L, ROXL.L	*/
	i_bfop,		/* 1110 1101 11  BFFFO				*/

	i_sh,		/* 1110 1110 00  ASR.B, LSR.B, ROR.B, ROXR.B	*/
	i_sh,		/* 1110 1110 01  ASR.W, LSR.W, ROR.W, ROXR.W	*/
	i_sh,		/* 1110 1110 10  ASR.L, LSR.L, ROR.L, ROXR.L	*/
	i_bfop,		/* 1110 1110 11  BFSET				*/

	i_sh,		/* 1110 1111 00  ASL.B, LSL.B, ROL.B, ROXL.B	*/
	i_sh,		/* 1110 1111 01  ASL.W, LSL.W, ROL.W, ROXL.W	*/
	i_sh,		/* 1110 1111 10  ASL.L, LSL.L, ROL.L, ROXL.L	*/
	i_bfop,		/* 1110 1111 11  BFINS				*/

	i_line_f,	/* 1111 0000 00  				*/
	i_line_f,	/* 1111 0000 01  				*/
	i_line_f,	/* 1111 0000 10  				*/
	i_line_f,	/* 1111 0000 11  				*/

	i_line_f,	/* 1111 0001 00  				*/
	i_line_f,	/* 1111 0001 01  				*/
	i_line_f,	/* 1111 0001 10  				*/
	i_line_f,	/* 1111 0001 11  				*/

	i_line_f,	/* 1111 0010 00  				*/
	i_line_f,	/* 1111 0010 01  				*/
	i_line_f,	/* 1111 0010 10  				*/
	i_line_f,	/* 1111 0010 11  				*/

	i_line_f,	/* 1111 0011 00  				*/
	i_line_f,	/* 1111 0011 01  				*/
	i_line_f,	/* 1111 0011 10  				*/
	i_line_f,	/* 1111 0011 11  				*/

	i_line_f,	/* 1111 0100 00  				*/
	i_line_f,	/* 1111 0100 01  				*/
	i_line_f,	/* 1111 0100 10  				*/
	i_line_f,	/* 1111 0100 11  				*/

	i_line_f,	/* 1111 0101 00  				*/
	i_line_f,	/* 1111 0101 01  				*/
	i_line_f,	/* 1111 0101 10  				*/
	i_line_f,	/* 1111 0101 11  				*/

	i_line_f,	/* 1111 0110 00  				*/
	i_line_f,	/* 1111 0110 01  				*/
	i_line_f,	/* 1111 0110 10  				*/
	i_line_f,	/* 1111 0110 11  				*/

	i_line_f,	/* 1111 0111 00  				*/
	i_line_f,	/* 1111 0111 01  				*/
	i_line_f,	/* 1111 0111 10  				*/
	i_line_f,	/* 1111 0111 11  				*/

	i_line_f,	/* 1111 1000 00  				*/
	i_line_f,	/* 1111 1000 01  				*/
	i_line_f,	/* 1111 1000 10  				*/
	i_line_f,	/* 1111 1000 11  				*/

	i_line_f,	/* 1111 1001 00  				*/
	i_line_f,	/* 1111 1001 01  				*/
	i_line_f,	/* 1111 1001 10  				*/
	i_line_f,	/* 1111 1001 11  				*/

	i_line_f,	/* 1111 1010 00  				*/
	i_line_f,	/* 1111 1010 01  				*/
	i_line_f,	/* 1111 1010 10  				*/
	i_line_f,	/* 1111 1010 11  				*/

	i_line_f,	/* 1111 1011 00  				*/
	i_line_f,	/* 1111 1011 01  				*/
	i_line_f,	/* 1111 1011 10  				*/
	i_line_f,	/* 1111 1011 11  				*/

	i_line_f,	/* 1111 1100 00  				*/
	i_line_f,	/* 1111 1100 01  				*/
	i_line_f,	/* 1111 1100 10  				*/
	i_line_f,	/* 1111 1100 11  				*/

	i_line_f,	/* 1111 1101 00  				*/
	i_line_f,	/* 1111 1101 01  				*/
	i_emulator,	/* 1111 1101 10  				*/
	i_line_f,	/* 1111 1101 11  				*/

	i_line_f,	/* 1111 1110 00  				*/
	i_line_f,	/* 1111 1110 01  				*/
	i_line_f,	/* 1111 1110 10  				*/
	i_line_f,	/* 1111 1110 11  				*/

	i_line_f,	/* 1111 1111 00  				*/
	i_line_f,	/* 1111 1111 01  				*/
	i_line_f,	/* 1111 1111 10  				*/
	i_line_f	/* 1111 1111 11  				*/

      };
   } // namespace cpu
} // namespace emumiga
