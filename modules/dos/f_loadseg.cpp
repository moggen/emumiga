/*
modules/dos/f_loadseg.cpp - dos.library emulation of LoadSeg and related functions.

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

#include "dos.h"

namespace emumiga
{
   namespace dos
   {

      // Helper function, loads 32 bits
      static uint32_t read32(BPTR f)
      {
         uint32_t i;

         i = ((uint32_t)FGetC(f) & 0xff) << 24;
         i |= ((uint32_t)FGetC(f) & 0xff) << 16;
         i |= ((uint32_t)FGetC(f) & 0xff) << 8;
         i |= ((uint32_t)FGetC(f) & 0xff);
         return i;
      }

      module::hook_status main::f_LoadSeg_1(context *ctx)
      {
         //****************************
         //  Function: LoadSeg
         //    Vector: 25
         //    Offset: -150
         // Arguments: D1.L pointer to file name string
         //   Returns: D0.L BCPL pointer to first segment or 0 if failure
         //****************************

         DEBUG(4) dprintf("dos::main::f_LoadSeg_1() called. this=%p, ctx=%p\n", this, ctx);

         DEBUG(5) dprintf("  File name (D1): 0x%x\n", ctx->cpu.d[1]);
     
         char filename[300];
         if(strlcpy_from_vspace(ctx, filename, ctx->cpu.d[1], sizeof(filename))) {
            dprintf("Error: dos::main::f_LoadSeg(): File name is not readable\n");
            return HOOK_ERROR;
         }

         DEBUG(5) dprintf("  File name: %s\n", filename);

         ctx->cpu.d[0] = 0;

         BPTR hunk_file = Open((STRPTR)filename, MODE_OLDFILE);
         if(hunk_file == NULL) {
            DEBUG(5) dprintf("  Couldn't open file\n");
            return HOOK_DONE;
         }

         uint32_t tmp = read32(hunk_file);

         // TODO: Detect and relay AROS ELF loads

         if(tmp != 0x3f3){
            // Must begin with a HUNK_HEADER
            DEBUG(5) dprintf("  No HUNK_HEADER first in file\n");
            Close(hunk_file);
            return HOOK_DONE;
         }

         DEBUG(5) dprintf("  HUNK_HEADER (0x3F3)\n");

         tmp = read32(hunk_file);
         if(tmp != 0){
            // Hunk names are not handled
            DEBUG(1) dprintf("Warning: dos::main::f_LoadSeg_1(): Hunk names are not supported\n");
            Close(hunk_file);
            return HOOK_DONE;
         }

         // Number of hunks
         uint32_t hunk_count = read32(hunk_file);
         DEBUG(5) dprintf("  Hunk count: %u\n", hunk_count);

         // First hunk to load
         tmp = read32(hunk_file);
         DEBUG(5) dprintf("  First hunk: %u (ignored)\n", tmp);

         // Last hunk to load
         tmp = read32(hunk_file);
         DEBUG(5) dprintf("  Last hunk: %u (ignored)\n", tmp);

         // Read hunk sizes
         uint32_t i;
         uint32_t hunk_sizes[100], hunk_address[100];
         uint8_t *hunk_address_real[100];
         int rc;

         for(i=0 ; i<hunk_count ; i++) {

            tmp = read32(hunk_file);
            hunk_sizes[i] = (tmp & 0xffffff) << 2;

            DEBUG(5) dprintf("  Hunk %2d: size %u\n", i, hunk_sizes[i]);

            // Special flags are not handled
            DEBUG(2) {
               if(tmp & 0xff000000)
                  dprintf("Notice: dos::main::f_LoadSeg_1(): Hunk flags are not supported, ignoring\n");
            }

            // Allocate memory, add 4 bytes for segment header
            hunk_address[i] = ctx->alloc_mem(hunk_sizes[i] + 4);
            if(hunk_address[i] == 0) {
               dprintf("Error: dos::main::f_LoadSeg_1(): Out of memory\n");
               Close(hunk_file);
               return HOOK_ERROR;
            }

            hunk_address_real[i] = ctx->mem_to_real(hunk_address[i]);
            if(hunk_address_real[i] == NULL) {
               dprintf("Error: dos::main::f_LoadSeg_1(): Memory error\n");
               Close(hunk_file);
               return HOOK_ERROR;
            }

/* symbols TBD
            if(CPU_IS_SET(cpu,CPU_FLAG_DISASM)) {
               // Add basic symbols
               entry = mmu_findEntry(hunk_address[i]);
               sprintf(tmps,"%%Hunk_%d_header",(unsigned int)i);
               mmu_addSymbol(entry,hunk_address[i],tmps);
               sprintf(tmps,"%%Hunk_%d",(unsigned int)i);
               mmu_addSymbol(entry,hunk_address[i]+4,tmps);
            }
*/
         }

         // Go through all hunks and set next pointer
         for(i=0 ; i<hunk_count ; i++) {
            if(i == hunk_count-1) {
               // Last one, set to NULL
               rc = WRITEMEM_32(0, hunk_address[i]);
               if(rc) {
                  dprintf("Error: dos::main::f_LoadSeg_1(): Memory access error\n");
                  Close(hunk_file);
                  return HOOK_ERROR;
               }
            } else {
               // BCPL pointer to the next segment
               rc = WRITEMEM_32(hunk_address[i+1]>>2, hunk_address[i]);
               if(rc) {
                  dprintf("Error: dos::main::f_LoadSeg_1(): Memory access error\n");
                  Close(hunk_file);
                  return HOOK_ERROR;
               }
            }
         }

         // Load and process hunks
         int hunk_num = 0;
         tmp = read32(hunk_file);

         while(FGetC(hunk_file) >= 0) {
            UnGetC(hunk_file, -1);

            // Special flags are not handled
            DEBUG(2) {
               if(tmp & 0xff000000)
                  dprintf("Notice: dos::main::f_LoadSeg_1(): Hunk flags are not supported, ignoring\n");
            }

            uint32_t reloc_count;

            switch(tmp & 0xffffff) {

               case 0x3e9: // HUNK_CODE

                  tmp = read32(hunk_file) << 2; // Size in 32 bit words
                  DEBUG(5) {
                     dprintf("  Hunk %2d: HUNK_CODE (0x3E9), Size: %d\n", hunk_num, tmp);
                  }
                  FRead(hunk_file, hunk_address_real[hunk_num] + 4, tmp, 1);
                  hunk_num++;
                  break;

               case 0x3ea: // HUNK_DATA

                  tmp = read32(hunk_file) << 2; // Size in 32 bit words
                  DEBUG(5) {
                     dprintf("  Hunk %2d: HUNK_DATA (0x3EA), Size: %d\n", hunk_num, tmp);
                  }
                  FRead(hunk_file, hunk_address_real[hunk_num] + 4, tmp, 1);
                  hunk_num++;
                  break;

               case 0x3eb: // HUNK_BSS

                  tmp = read32(hunk_file) << 2; // Size in 32 bit words
                  DEBUG(5) {
                     dprintf("  Hunk %2d: HUNK_BSS (0x3EB), Size: %d\n", hunk_num, tmp);
                  }
                  hunk_num++;
                  break;

               case 0x3ec: // HUNK_RELOC32

                  DEBUG(5) {
                     dprintf("  Hunk %2d: HUNK_RELOC32 (0x3EB)\n", hunk_num-1);
                  }

                  reloc_count = read32(hunk_file);
                  while(reloc_count > 0) {

                     uint32_t referring_hunk = read32(hunk_file);
                     DEBUG(5) {
                        dprintf("    Referring hunk %u: %u patching points\n", referring_hunk, reloc_count);
                     }
                     for(i=0 ; i<reloc_count ; i++) {
                        // Offset into this hunk
                        uint32_t offset = read32(hunk_file);
                        DEBUG(5) {
                           dprintf("      Patching offset %u\n", offset);
                        }
                        rc = READMEM_32(hunk_address[hunk_num-1]+4+offset, &tmp);
                        if(rc) {
                           dprintf("Error: dos::main::f_LoadSeg(): Memory access error\n");
                           Close(hunk_file);
                           return HOOK_ERROR;
                        }
                        tmp += hunk_address[referring_hunk] + 4;
                        rc = WRITEMEM_32(tmp,hunk_address[hunk_num-1]+4+offset);
                        if(rc) {
                           dprintf("Error: dos::main::f_LoadSeg(): Memory access error\n");
                           Close(hunk_file);
                           return HOOK_ERROR;
                        }
                     }
                     reloc_count = read32(hunk_file);
                  }
                  break;

               case 0x3f0: // HUNK_SYMBOL

                  tmp = read32(hunk_file) << 2; // Size in 32 bit words
                  DEBUG(5) {
                     dprintf("  Hunk %2d: HUNK_SYMBOL (0x3F0), Size: %d\n", hunk_num-1, tmp);
                  }

                  // Just ignore for now
                  while(tmp > 0) {
                     read32(hunk_file);
                     tmp -= 4;
                  }
/*
                  while(tmp!=0){
                     // Read data
                     FRead(hunk_file,symbolName,4,tmp);
                     // Make sure it is null terminated
                     symbolName[4*tmp]='\0';
                     // Read address
                     tmp3 = read32(hunk_file);

                     DEBUG(5){
                        dprintf("dos.library: LoadSeg():       Symbol: \"%s\", value 0x%x\n",symbolName,(unsigned int)tmp3);
                     }

                     if(CPU_IS_SET(cpu,CPU_FLAG_DISASM)) {
                        // Add the symbol
                        mmu_addSymbol(entry,hunk_vaddr[hunk_num-1]+4+tmp3,symbolName);
                     }

                     // Read next length
                     tmp = read32(hunk_file);
                  }
*/
                  break;

               case 0x3f1:

                  tmp = read32(hunk_file) << 2; // Size in 32 bit words
                  DEBUG(5) {
                     dprintf("  Hunk %2d: HUNK_DEBUG (0x3F1), Size: %d\n", hunk_num-1, tmp);
                  }

                  // Just ignore for now
                  while(tmp > 0) {
                     read32(hunk_file);
                     tmp -= 4;
                  }
                  break;

               case 0x3f2:

                  DEBUG(5) {
                     dprintf("  Hunk %2d: HUNK_END (0x3F2)\n", hunk_num-1);
                  }
                  break;

               default:
                  DEBUG(1) {
                     dprintf("Error: dos::main::f_LoadSeg(): Hunk type 0x%x is not supported.\n", tmp);
                  }
                  break;

            }
            tmp = read32(hunk_file);
         }
         Close(hunk_file);

/*
         DEBUG(8){
            dprintf("First segment: %u\n",(unsigned int)hunk_vaddr[0]);
            for(i=0;i<(hunk_sizes[0]+4);i++) {
               rc = READMEM_8(hunk_vaddr[0]+i, &tmp8);
               if(rc) {
                  dprintf("Error: dos::main::f_LoadSeg(): Memory access error\n");
                  Close(hunk_file);
                  return HOOK_ERROR;
               }
               if(rc) return rc;
               dprintf(" %02x",tmp8);
            }
            dprintf("\n");
         }
*/
         // All ok, return BCPL ptr to first segment
         ctx->cpu.d[0] = hunk_address[0] >> 2;

         return HOOK_DONE;
      }
   }
}
