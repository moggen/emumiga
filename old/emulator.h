/*
emulator.h - Emulation process header

Copyright (C) 2009, 2010  Magnus Öberg

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

#ifndef _EMULATOR_H_
#define _EMULATOR_H_

#include <stdint.h>
#include <exec/types.h>
#include <exec/ports.h>
#include <dos/dosextens.h>
#include <dos/bptr.h>

namespace emumiga {

/* Predecls and typedefs */
struct emumsg_base;
typedef struct emumsg_base emumsg_base_t;
struct emumsg_run;
typedef struct emumsg_run emumsg_run_t;
struct emumsg_syscall;
typedef struct emumsg_syscall emumsg_syscall_t;

} // namespace emumiga

#include "cpu/cpu.h"
#include "lib_dos/s_process.h"

namespace emumiga {

/* Predecls */
int emulator_init();
void emulator_cleanup();
lib_dos::s_process *emulator_current_process();
int emulator_doEvents(struct MsgPort *port);
void emulator_sp_check(cpu::cpu *cpu);

/* Also emulation process end message */
struct emumsg_base {
	struct Message msg;
	int type;
};

/* Run emulation */
struct emumsg_run {
	struct Message msg;
	int type;
	/* params */
	char *prg_name;
	char *args;
	struct Process *caller_proc;
	struct MsgPort *syscall_port;
	int stack_size;
	/* return */
	uint32_t exit_code;
	int emu_status;
};

/* Syscall */
struct emumsg_syscall {
	/* Headers, must not be touched
	   by library connectors */
	struct Message msg;
	int type;
	lib_dos::s_process *proc;

	/* Argument variables avalable for
	   use by library connectors */
	union {
		APTR _aptr;
		IPTR _iptr;
		BPTR _bptr;
		STRPTR _strptr;
		DOUBLE _double;
		FLOAT _float;
		UQUAD _uquad;
		QUAD _quad;
		ULONG _ulong;
		LONG _long;
		UWORD _uword;
		WORD _word;
		UBYTE _ubyte;
		BYTE _byte;
		BOOL _bool;
	} arg[10];

	/* General data pointer for internal use by
           library connectors */
	void *internalPtr;

	/* Library module and function number */
	uint16_t mnum;
	int16_t func;
	int emu_status;
};

#define EMUMSGTYPE_RUN 		1
#define EMUMSGTYPE_SYSCALL	2
#define EMUMSGTYPE_DUMP_MMAP	3

} // namespace emumiga

#endif
