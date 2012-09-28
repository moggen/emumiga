/*
lib_dos/f_loadseg.c - dos.library LoadSeg function

Copyright (C) 2007, 2008, 2009, 2010  Magnus Öberg

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

#include <stdio.h>
#include <stdlib.h>

#include <proto/dos.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../hook.h"
#include "../emulator.h"

/****************************
 Function: LoadSeg
   Vector: 25
   Offset: -150
Arguments: D1.L pointer to file name string
  Returns: D0.L BCPL pointer to first segment or 0 if failure
****************************/

/* Helper function, loads 32 bits */
static uint32_t read32(BPTR f)
{
	uint32_t i;

	i = ((uint32_t)FGetC(f) & 0xff) << 24;
	i |= ((uint32_t)FGetC(f) & 0xff) << 16;
	i |= ((uint32_t)FGetC(f) & 0xff) << 8;
	i |= ((uint32_t)FGetC(f) & 0xff);
	return i;
}

int lib_dos_f_LoadSeg(emumsg_syscall_t *msg)
{
	cpu_t *cpu;
	BPTR hfile;
	char ch, filename[300], *cur, tmps[200], symbolName[200];
	uint32_t tmp1, tmp2, tmp3, tmp4, tmp5;
	uint32_t hunkc, hunk_sizes[100], hunk_vaddr[100];
	uint32_t i, hunknum, vaddr;
	mmu_entry_t *entry;
	uint8_t tmp8;
	int rc;

	DEBUG(4) dprintf("dos.library: LoadSeg() called\n");

	cpu = msg->proc->cpu;

	cur=filename;
	vaddr=cpu->d[1];
	do {
		rc = READMEM_8(vaddr++, &tmp8);
		if(rc) return rc;
		ch = (char)tmp8;
		*cur++ = ch;
	} while(ch != '\0');

	hfile=Open((STRPTR)filename,MODE_OLDFILE);
	if(hfile==NULL){
		DEBUG(5){
			dprintf("dos.library: LoadSeg(): Couldn't open file\n");
		}
		cpu->d[0]=0;
		return HOOK_DONE;
	}

	tmp1 = read32(hfile);

	/* TODO: Detect and relay AROS ELF loads */

	if(tmp1 != 0x3f3){
		/* Must begin with a HUNK_HEADER */
		DEBUG(5){
			dprintf("dos.library: LoadSeg(): No HUNK_HEADER first in file\n");
		}
		Close(hfile);
		cpu->d[0]=0;
		return HOOK_DONE;
	}

	DEBUG(5){
		dprintf("dos.library: LoadSeg(): HUNK_HEADER (0x3F3)\n");
	}

	tmp1 = read32(hfile);
	if(tmp1 != 0x0){
		/* Hunk names are not handled */
		DEBUG(1){
			dprintf("dos.library: LoadSeg(): Hunk names are not supported\n");
		}
		Close(hfile);
		cpu->d[0]=0;
		return HOOK_DONE;
	}

	/* Number of hunks */
	hunkc = read32(hfile);

	/* First hunk to load */
	tmp1 = read32(hfile);

	/* Last hunk to load */
	tmp2 = read32(hfile);

	DEBUG(5){
		dprintf("dos.library: LoadSeg(): Hunks: %u, first: %u, last: %u\n",(unsigned int)hunkc,(unsigned int)tmp1,(unsigned int)tmp2);
	}

	/* Read hunk sizes */
	for(i=0;i<hunkc;i++){

		tmp1 = read32(hfile);
		hunk_sizes[i] = (tmp1 & 0xffffff) << 2;

		DEBUG(5){
			dprintf("dos.library: LoadSeg(): Hunk %d size: %u\n",(unsigned int)i,(unsigned int)hunk_sizes[i]);
		}
		if(tmp1 & 0xff000000) {
			/* Special flags are not handled */
			DEBUG(2){
				dprintf("Warning: dos.library: LoadSeg(): Hunk flags are not supported.\n");
			}
		}

		/* Allocate memory, add 4 bytes for segment header */
		hunk_vaddr[i] = vallocmem(hunk_sizes[i] + 4, NULL);

		if(CPU_IS_SET(cpu,CPU_FLAG_DISASM)) {
			/* Add basic symbols */
			entry = mmu_findEntry(hunk_vaddr[i]);
			sprintf(tmps,"%%Hunk_%d_header",(unsigned int)i);
			mmu_addSymbol(entry,hunk_vaddr[i],tmps);
			sprintf(tmps,"%%Hunk_%d",(unsigned int)i);
			mmu_addSymbol(entry,hunk_vaddr[i]+4,tmps);
		}
	}

	/* Go through all hunks and set next pointer */
	for(i=0;i<hunkc;i++){
		if(i == hunkc-1) {
			/* Last one, set to NULL */
			rc = WRITEMEM_32(0,hunk_vaddr[i]);
			if(rc) return rc;
		} else {
			/* BCPL pointer to the next segment */
			rc = WRITEMEM_32(hunk_vaddr[i+1]>>2,hunk_vaddr[i]);
			if(rc) return rc;
		}
	}

	/* Load and process hunks */
	hunknum=0;
	tmp1 = read32(hfile);
	while(FGetC(hfile)>=0){
		UnGetC(hfile,-1);
		if(tmp1 & 0xff000000) {
			/* Special flags are not handled */
			DEBUG(2){
				dprintf("Warning: dos.library: LoadSeg(): Hunk flags are not supported.\n");
			}
		}

		switch(tmp1 & 0xffffff){
			case 0x3e9:
				/* HUNK_CODE */

				/* Size of data */
				tmp2 = read32(hfile) << 2;
				DEBUG(5){
					dprintf("dos.library: LoadSeg(): %2u: HUNK_CODE (0x3E9), Size: %u\n",(unsigned int)hunknum,(unsigned int)tmp2);
				}
				FRead(hfile,vptr(hunk_vaddr[hunknum]+4),
					 tmp2, 1);
				hunknum++;
				break;

			case 0x3ea:

				/* Size of data */
				tmp2 = read32(hfile) << 2;
				DEBUG(5){
					dprintf("dos.library: LoadSeg(): %2u: HUNK_DATA (0x3EA), Size: %u\n",(unsigned int)hunknum,(unsigned int)tmp2);
				}
				FRead(hfile,vptr(hunk_vaddr[hunknum]+4),
					 tmp2, 1);
				hunknum++;
				break;

			case 0x3eb:

				/* Size of data */
				tmp2 = read32(hfile) << 2;
				DEBUG(5){
					dprintf("dos.library: LoadSeg(): %2u: HUNK_BSS (0x3EB), Size: %u\n",(unsigned int)hunknum,(unsigned int)tmp2);
				}
				hunknum++;
				break;

			case 0x3ec:

				DEBUG(5){
					dprintf("dos.library: LoadSeg(): %2u: HUNK_RELOC32 (0x3EC)\n",(unsigned int)hunknum-1);
				}
				/* Number of relocations */
				tmp2 = read32(hfile);
				while(tmp2!=0){
					/* Refering hunk */
					tmp3 = read32(hfile);
					DEBUG(5){
						dprintf("dos.library: LoadSeg():       Referencing hunk %u, number of relocations: %u\n",(unsigned int)tmp3,(unsigned int)tmp2);
					}
					for(i=0;i<tmp2;i++) {
						/* Offset into this hunk */
						tmp4 = read32(hfile);
						DEBUG(5){
							dprintf("dos.library: LoadSeg():       Relocating offset %u = memory %u\n",(unsigned int)tmp4,(unsigned int)(hunk_vaddr[hunknum-1]+4+tmp4));
						}
						rc = READMEM_32(hunk_vaddr[hunknum-1]+4+tmp4, &tmp5);
						if(rc) return rc;
						tmp5 += hunk_vaddr[tmp3] + 4;
						rc = WRITEMEM_32(tmp5,hunk_vaddr[hunknum-1]+4+tmp4);
						if(rc) return rc;
					}
					tmp2 = read32(hfile);
				}
				break;

			case 0x3f0:

				DEBUG(5){
					dprintf("dos.library: LoadSeg(): %2u: HUNK_SYMBOL (0x3F0)\n",(unsigned int)hunknum-1);
				}

				/* Get MMU entry of referenced hunk */
				entry = mmu_findEntry(hunk_vaddr[hunknum-1]);

				/* Read length of symbol unit, number of longwords */
				tmp2 = read32(hfile);
				while(tmp2!=0){
					/* Read data */
					FRead(hfile,symbolName,4,tmp2);
					/* Make sure it is null terminated */
					symbolName[4*tmp2]='\0';
					/* Read value */
					tmp3 = read32(hfile);

					DEBUG(5){
						dprintf("dos.library: LoadSeg():       Symbol: \"%s\", value 0x%x\n",symbolName,(unsigned int)tmp3);
					}

					if(CPU_IS_SET(cpu,CPU_FLAG_DISASM)) {
						/* Add the symbol */
						mmu_addSymbol(entry,hunk_vaddr[hunknum-1]+4+tmp3,symbolName);
					}

					/* Read next length */
					tmp2 = read32(hfile);
				}
				break;

			case 0x3f1:

				DEBUG(5){
					dprintf("dos.library: LoadSeg():     HUNK_DEBUG (0x3F1)\n");
				}
				/* Read length of data, number of longwords */
				tmp2 = read32(hfile);

				DEBUG(5){
					dprintf("dos.library: LoadSeg():       %u bytes of debug data\n",(unsigned int)tmp2*4);
				}

				while(tmp2--){
					/* Read data */
					tmp3 = read32(hfile);
				}
				break;

			case 0x3f2:

				DEBUG(5){
					dprintf("dos.library: LoadSeg(): %2u: HUNK_END (0x3F2)\n",(unsigned int)hunknum-1);
				}
				break;

			default:
				DEBUG(1){
					dprintf("Warning: dos.library: LoadSeg(): Hunk type 0x%x is not supported.\n",(unsigned int)tmp1);
				}
				break;

		}
		tmp1 = read32(hfile);
	}
	Close(hfile);

	DEBUG(8){
		dprintf("First segment: %u\n",(unsigned int)hunk_vaddr[0]);
		for(i=0;i<(hunk_sizes[0]+4);i++) {
			rc = READMEM_8(hunk_vaddr[0]+i, &tmp8);
			if(rc) return rc;
			dprintf(" %02x",tmp8);
		}
		dprintf("\n");
	}

	/* All ok, return BCPL ptr to first segment */
	cpu->d[0] = hunk_vaddr[0] >> 2;

	return HOOK_DONE;
}
