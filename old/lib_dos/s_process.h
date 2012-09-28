/*
lib_dos/s_process.h - Header file for dos.library
	struct Process

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

#ifndef _LIB_DOS_S_PROCESS_H_
#define _LIB_DOS_S_PROCESS_H_

#include <stdint.h>
#include <dos/dosextens.h>

// Predecl for the includes
namespace emumiga {
namespace lib_dos {
class s_process;
} // namespace lib_dos
} // namespace emumiga

#include "../cpu/cpu.h"
#include "../memory/mapping.h"
#include "../emulator.h"

namespace emumiga {
namespace lib_dos {

// Kickstart 2.0
#define LIB_DOS_S_PROCESS_SIZE 228

// Structure for simulated object
class s_process : public memory::mapping {
    public:
	cpu::cpu *cpu;
	int stacksize;
	uint32_t vaddr_stack;
	uint32_t vaddr_prg_name;
	uint32_t vaddr_args;
	emumsg_run_t *run_msg;
	struct MsgPort *syscall_port;
	emumsg_syscall_t *syscall_msg;

	static int init();
	static void cleanup();

	static s_process *get_real(struct Process *proc);
	static s_process *get_vaddr(uint32_t vaddr);

	// Override dispose
	virtual int dispose();

    protected:
	// Current process index
	static int currentProcessIdx;

	// VAddr to instruction that ends a process
	static uint32_t endInstruction;

	// Init flag
	static int is_init;

/*
	lib_dos_s_commandlineinterface_t *cli;
	lib_dos_s_filehandle_t *cis, *cos, *ces;
	lib_dos_s_filelock_t *currentdir;
	lib_exec_s_msgport_t *msgport;
	uint32_t sp_lower;
	uint32_t sigalloc, sigwait, sigrecvd;
*/
};

} // namespace lib_dos
} // namespace emumiga

#endif
