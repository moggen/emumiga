/*
cpu68k.h - M68K CPU emulation include file

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

#ifndef EMUMIGA_H
#error "Don't include individual headers, use emumiga.h"
#endif

namespace emumiga {

   class cpu68k {
     public:
      cpu68k();

      // Instance attributes

      // Registers
      uint32_t d[8];
      uint32_t a[8];
      uint32_t pc;
      uint16_t sr;

      uint32_t flags;
      static const uint32_t FLAG_DEBUG   = 0x1;
      static const uint32_t FLAG_EMULATE = 0x2;
      static const uint32_t FLAG_STRICT  = 0x4;
      static const uint32_t FLAG_DISASM  = 0x8;

      uint32_t type;
      static const uint32_t TYPE_AUTO = 0x01;
      static const uint32_t TYPE_000  = 0x02;
      static const uint32_t TYPE_010  = 0x04;
      static const uint32_t TYPE_020  = 0x08;
      static const uint32_t TYPE_030  = 0x10;
      static const uint32_t TYPE_040  = 0x20;
      static const uint32_t TYPE_060  = 0x40;
      static const uint32_t TYPE_ALL  = 0x7E;

      // Disassembler
      char disasm_instr[20];  // Instruction
      char disasm_arg1[40];   // First argument
      char disasm_arg2[40];   // Second argument
      uint32_t disasm_start;  // Start address of instruction
      int disasm_bytes;  // Total instruction length
      uint32_t disasm_ea1;    // Effective address for mem op
      uint32_t disasm_ea2;    // Secondary address, mem to mem op
      static const uint32_t EA_NONE = 0xFEDCBA98;

      // Hook
      uint16_t emu_module;  // Module
      int16_t emu_func;     // Function

      // MMU working space
      uint8_t mmu_work[4];

      // Caches
      //memory::cache *i_cache, *d_cache;

      // Instance methods
      void init();

      // Return codes from instruction execution
      enum returncode { I_OK=0, I_UNSUPPORTED, I_ILLEGAL, I_PRIVILEGE,
			I_HOOK, I_TRAP, I_DIV0, I_BUS_ERROR } ;

      returncode run_block(context *ctx);

      bool is_set(uint32_t);
      bool is_type(uint32_t);
   };

} // namespace emumiga
