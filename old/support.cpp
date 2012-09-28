/*
support.c - Helper module

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


#include "debug.h"
#include "hook.h"
#include "emulator.h"

namespace emumiga {

static int is_init = 0;

// Predecls
static int hook1(emumsg_syscall_t *msg);
static int hook2(emumsg_syscall_t *msg);
static int hook3(emumsg_syscall_t *msg);

int support_init()
{
	DEBUG(3) dprintf("support: support_init() called\n");

	// Allocate hook number. First caller so it is always 0
	allocate_hook("support", hook1, hook2, hook3);

	is_init = 1;
	return 0;
}

void support_cleanup()
{
	DEBUG(3) dprintf("support: support_cleanup() called\n");

	if( !is_init ) {
		DEBUG(2) dprintf("Warning: support: support_cleanup: Not initialized.\n");
		return;
	}

	// Free hook module number
	free_hook(0);

	is_init = 0;
}

static int hook1(emumsg_syscall_t *msg)
{
	switch(msg->func){
		case 0:
			// End current process
			DEBUG(5) dprintf("support: hook1: End process\n");
			return HOOK_END_PROC;

		case 1:
			// Error 1 in bootstrap
			dprintf("Error: support: hook1: Bootstrap could not open dos.library\n");
			msg->emu_status=1;
			return HOOK_END_PROC;

		case 2:
			// Error 2 in bootstrap
			dprintf("Error: support: hook1: Bootstrap could not load hunk file\n");
			msg->emu_status=2;
			return HOOK_END_PROC;
	}
	dprintf("Error: support: hook1: Unimplemented function %u\n", msg->func);
	msg->emu_status=3;
	return HOOK_END_PROC;
}

static int hook2(emumsg_syscall_t *msg)
{
	dprintf("Error: support: hook2: Unimplemented function %u\n", msg->func);
	msg->emu_status=4;
	return HOOK_END_PROC;
}

static int hook3(emumsg_syscall_t *msg)
{
	dprintf("Error: support: hook3: Unimplemented function %u\n", msg->func);
	msg->emu_status=5;
	return HOOK_END_PROC;
}

} // namespace emumiga
