/*
hook.c - Emulator hook handling

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

#include <string.h>
#include <proto/exec.h>
#include "debug.h"
#include "emulator.h"
#include "hook.h"

namespace emumiga {

/* Array of simulated objects */
#define HOOK_CSIZE 20
static char const *objects_name[HOOK_CSIZE];
static hookptr_t objects_hook1[HOOK_CSIZE];
static hookptr_t objects_hook2[HOOK_CSIZE];
static hookptr_t objects_hook3[HOOK_CSIZE];
static int is_init=0;

/* TODO: Convert array to dynamic container */

/* Global init */
int hook_init()
{
	DEBUG(3) dprintf("hook: hook_init() called\n");

        memset(objects_name, 0, sizeof(objects_name));
        memset(objects_hook1, 0, sizeof(objects_hook1));
        memset(objects_hook2, 0, sizeof(objects_hook2));
        memset(objects_hook3, 0, sizeof(objects_hook3));
	is_init = 1;
	return 0;
}

/* Global cleanup */
void hook_cleanup()
{
        int i;

	DEBUG(4) dprintf("hook: hook_cleanup() called\n");

	if(!is_init)
		return;

        for(i=0;i<HOOK_CSIZE;i++)
        {
                if(objects_hook1[i] != NULL)
                {
                        DEBUG(2) dprintf("Warning: hook: hook_cleanup: Objects remain!\n");
                }
        }
	is_init = 0;
}

/* Get an empty hook from the array */
uint16_t allocate_hook(char const *name, hookptr_t hook1, hookptr_t hook2, hookptr_t hook3)
{
        uint16_t i;

	DEBUG(4) dprintf("hook: allocate_hook() called. name '%s'\n", name);

        /* Find an empty slot in the instance array */
        for(i=0;i<HOOK_CSIZE;i++) {
                if(objects_hook1[i] == NULL) {
			/* Set it*/
			objects_name[i] = name;
			objects_hook1[i] = hook1;
			objects_hook2[i] = hook2;
			objects_hook3[i] = hook3;
                        return i;
		}
        }
        dprintf("Error: hook: allocate_hook: Instance array is full.\n");
        return -1;
}

/* Release a hook */
void free_hook(uint16_t mnum)
{
	DEBUG(4) dprintf("hook: free_hook() called\n");

	if(objects_hook1[mnum] == NULL) {
	        DEBUG(1) dprintf("Warning: hook: free_hook: Freeing an already free hook.\n");
	}

	/* Mark as free */
        objects_name[mnum] = NULL;
        objects_hook1[mnum] = NULL;
        objects_hook2[mnum] = NULL;
        objects_hook3[mnum] = NULL;
}

/* Call hook1 */
int call_hook1(emumsg_syscall_t *msg)
{
	uint16_t mnum;
	int16_t func;

	mnum = msg->mnum;
	func = msg->func;

	DEBUG(4) dprintf("hook: call_hook1() called. mnum %u (%s), func %d\n", mnum, objects_name[mnum], func);

	if(objects_hook1[mnum] == NULL) {
	        dprintf("Error: hook: call_hook1: Call on unallocated module numer.\n");
		return 1;
	}

	return objects_hook1[mnum](msg);
}

/* Call hook2 */
int call_hook2(emumsg_syscall_t *msg)
{
	uint16_t mnum;
	int16_t func;

	mnum = msg->mnum;
	func = msg->func;

	DEBUG(4) dprintf("hook: call_hook2() called. msg->mnum %u (%s), msg->func %d\n", mnum, objects_name[mnum], func);

	if(objects_hook2[mnum] == NULL) {
	        dprintf("Error: hook: call_hook2: Call on unallocated module numer.\n");
		return 1;
	}

	return objects_hook2[mnum](msg);
}

/* Call hook3 */
int call_hook3(emumsg_syscall_t *msg)
{
	uint16_t mnum;
	int16_t func;

	mnum = msg->mnum;
	func = msg->func;

	DEBUG(4) dprintf("hook: call_hook3() called. msg->mnum %u (%s), msg->func %d\n", mnum, objects_name[mnum], func);

	if(objects_hook3[mnum] == NULL) {
	        dprintf("Error: hook: call_hook3: Call on unallocated module numer.\n");
		return 1;
	}

	return objects_hook3[mnum](msg);
}

} // namespace emumiga
