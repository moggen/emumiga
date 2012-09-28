/*
lib_dos/s_commandlineinterface.c - dos.library struct CommandLineInterface

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
#include <string.h>

#include <proto/exec.h>

#include "../debug.h"
#include "../cpu/cpu.h"
#include "../mmu/mmu.h"
#include "../vmem.h"
#include "../macros.h"
#include "s_commandlineinterface.h"
#include "s_process.h"

/* Predecl of destructor */
static void destructor(mmu_entry_t *entry);

/* Object pool */
static mmu_pool_t *pool;

/* Init flag */
static int is_init = 0;

/* Read map function */
static int map_r(mmu_entry_t *entry, uint32_t offset, uint32_t size,
        uint8_t *data )
{
	uint32_t i;
	lib_dos_s_commandlineinterface_t *cli;
	uint32_t defaultstack = 0;
	int slen;
	uint8_t *addr;
	uint32_t vaddr;

	DEBUG(7) dprintf("dos.library: struct CommandLineInterface: map_r() called. VAddr: 0x%x, Size: 0x%x\n", (unsigned int)(entry->startAddr+offset), (unsigned int)size);

	/* Get our object */
	cli = (lib_dos_s_commandlineinterface_t *)entry->nativePtr;

	for(i=offset;i<(size+offset);i++) {
		switch(i) {

			/* cli_CommandName */
			case 16: case 17: case 18: case 19:
				if(cli->commandname_vaddr == 0) {
					if(cli->proc != NULL) {
						/* Special, simulated process */
						slen = strlen(cli->proc->run_msg->prg_name);
						addr = (uint8_t *)AllocVec(slen+1, MEMF_PUBLIC);
						addr[0] = (uint8_t)slen;
						strncpy((char *)&addr[1],cli->proc->run_msg->prg_name,slen);
					} else {

						/* No simulation */
						/* TODO: Move BSTR handling into its own class? */
						slen = AROS_BSTR_strlen(cli->real->cli_CommandName);
						if(slen>255) {
							slen = 255;
							DEBUG(1) dprintf("Warning: Mapping of string with length>255 to a BSTR. The string was truncated.\n");
						}
						addr = (uint8_t *)AllocVec(slen+1, MEMF_PUBLIC);
						addr[0] = (uint8_t)slen;
						strncpy((char *)&addr[1], (char *)AROS_BSTR_ADDR(cli->real->cli_CommandName),slen);
					}
					vaddr = vallocmem_rom(addr,slen+1,NULL);
					cli->commandname = addr;
					cli->commandname_vaddr = vaddr >> 2;
				}
				i = 19;
				break;

			/* cli_DefaultStack */
			case 52: case 53: case 54: case 55:
				if(cli->proc != NULL) {
					/* Special, simulated process */
					defaultstack = (cli->proc->stacksize >> 2);
				} else {
					/* No simulation */
					defaultstack = cli->real->cli_DefaultStack;
				}
				i = 55;
				break;

			default:
				/* Nothing found, signal error */
			        dprintf("Error: dos.library: struct CommandLineInterface: Read is not allowed. Offset=%u\n", (unsigned int)offset);
			        return 1;
		}
	}

	for(i=offset;i<(size+offset);i++) {
		switch(i) {

			/* cli_CommandLine */
			case 16:
				*data++ = (cli->commandname_vaddr >> 24) & 0xff;
				break;
			case 17:
				*data++ = (cli->commandname_vaddr >> 16) & 0xff;
				break;
			case 18:
				*data++ = (cli->commandname_vaddr >> 8) & 0xff;
				break;
			case 19:
				*data++ = (cli->commandname_vaddr) & 0xff;
				break;

			/* cli_DefaultStack */
			case 52:
				*data++ = (defaultstack >> 24) & 0xff;
				break;
			case 53:
				*data++ = (defaultstack >> 16) & 0xff;
				break;
			case 54:
				*data++ = (defaultstack >> 8) & 0xff;
				break;
			case 55:
				*data++ = (defaultstack) & 0xff;
				break;

			default:
				/* Nothing found, signal error */
			        dprintf("Error: dos.library: struct CommandLineInterface: Read is not allowed. Offset=%u\n", (unsigned int)offset);
			        return 1;
		}
	}
	return 0;
}

/* Global init */
int lib_dos_s_commandlineinterface_init()
{
	DEBUG(4) dprintf("dos.library: lib_dos_s_commandlineinterface_init() called\n");

        pool = mmu_pool_create(
                "dos.s.CommandLineInterface",
                MMU_TYPE_DOS_S_COMMANDLINEINTERFACE,
                LIB_DOS_S_COMMANDLINEINTERFACE_SIZE,
                sizeof(lib_dos_s_commandlineinterface_t)
        );

        if(pool == NULL)
                return 1;

	is_init = 1;
	return 0;
}

/* Global cleanup */
void lib_dos_s_commandlineinterface_cleanup()
{
	DEBUG(4) dprintf("dos.library: lib_dos_s_commandlineinterface_cleanup() called\n");

	if( !is_init ) {
		DEBUG(2) dprintf("Warning: dos.library: lib_dos_s_commandlineinterface_cleanup: Not initialized\n");
		return;
	}

	mmu_pool_destroy(pool);

	is_init = 0;
}

/* Get an instance */
lib_dos_s_commandlineinterface_t *lib_dos_s_commandlineinterface_get_real(struct CommandLineInterface *real)
{
	mmu_entry_t *entry;
        lib_dos_s_commandlineinterface_t *obj;
	uint32_t vaddr;
	char symbol[200];

	DEBUG(5) dprintf("dos.library: lib_dos_s_commandlineinterface_get_real() called\n");

	if(real == NULL)
		return NULL;

        /* Check to find if it already exists */
        entry = mmu_findEntryWithTypeReal(real, MMU_TYPE_DOS_S_COMMANDLINEINTERFACE);
        if(entry != NULL) {

                /* Yes, return object */
                return (lib_dos_s_commandlineinterface_t *)entry->nativePtr;
        }

        /* No, set up an object */
        entry = mmu_allocObj(pool, real);
        if(entry == NULL)
                 return NULL;

        /* Extract object */
        obj = (lib_dos_s_commandlineinterface_t *)entry->nativePtr;

        /* Init structure */
	entry->read = map_r;
        entry->destructor = destructor;
        obj->entry = entry;
        obj->real = real;

        DEBUG(3) {
                /* Add extra symbols */
		vaddr = entry->startAddr;
                sprintf(symbol, "!%08x.%s.CommandName", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+16, symbol);
                sprintf(symbol, "!%08x.%s.DefaultStack", vaddr, pool->name);
                mmu_addSymbol(entry, vaddr+52, symbol);
	}

        return obj;
}

/* Get an instance, special handling for simulated processes */
lib_dos_s_commandlineinterface_t *lib_dos_s_commandlineinterface_get_real_sim(struct CommandLineInterface *cli, lib_dos_s_process_t *proc)
{
	lib_dos_s_commandlineinterface_t *obj;

	obj = lib_dos_s_commandlineinterface_get_real(cli);
	if(obj != NULL) {
		obj->proc = proc;
	}
	return obj;
}

/* Get an instance */
lib_dos_s_commandlineinterface_t *lib_dos_s_commandlineinterface_get_vaddr(uint32_t vaddr)
{
        mmu_entry_t *entry;

        DEBUG(5) dprintf("dos.library: lib_dos_s_commandlineinterface_get_vaddr() called\n");

        if(vaddr == 0)
                return NULL;

        /* Search for an existing object */
        entry = mmu_findEntryWithType(vaddr, MMU_TYPE_DOS_S_COMMANDLINEINTERFACE);

        /* This structure should not be created from user space */
        if(entry == NULL)
                return NULL;

        return (lib_dos_s_commandlineinterface_t *)entry->nativePtr;
}

/* Destructor of mapping */
static void destructor(mmu_entry_t *entry)
{
	lib_dos_s_commandlineinterface_t *obj;

	DEBUG(5) dprintf("dos.library: lib_dos_s_commandlineinterface: destructor() called\n");

	obj = (lib_dos_s_commandlineinterface_t *)entry->nativePtr;

	/* CommandName created? */
	if(obj->commandname_vaddr) {
		vfreemem_rom(obj->commandname_vaddr);
	}
	if(obj->commandname) {
		FreeVec(obj->commandname);
	}

	mmu_freeObj(entry);
}
