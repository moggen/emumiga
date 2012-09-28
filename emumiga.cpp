/*
emumiga.cpp - emumiga.library main code

Copyright (C) 2009-2011  Magnus Öberg

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

#include "emumiga.h"
/*
#include "memory/mapping.h"
#include "log.h"
#include "emulator.h"
#include "debug.h"
#include "hook.h"
#include "memory/vmem.h"
#include "support.h"
#include "lib_exec/s_execbase.h"
#include "lib_dos/s_doslibrary.h"
#include "lib_intuition/intuition.h"
#include "lib_graphics/graphics.h"
#include "lib_utility/utility.h"
#include "lib_gadtools/gadtools.h"
#include "lib_icon/icon.h"
#include "lib_layers/layers.h"
#include "lib_locale/locale.h"
#include "dev_timer/timer.h"
#include "dev_audio/audio.h"
*/

namespace emumiga {

/* Predecls */
   static void cleanup();

   static int init()
   {
      /* The semantic of the return code is reversed in this
	 library init function compared to other routines
	 in this project. Returning 0 means failure here */

      log::init();
      emulator::init();

      /*
	if( log_init() ) { cleanup(); return 0; }
	if( memory::mapping::init() ) { cleanup(); return 0; }
	if( vmem_init() ) { cleanup(); return 0; }
	if( hook_init() ) { cleanup(); return 0; }
	if( emulator_init() ) { cleanup(); return 0; }
	if( support_init() ) { cleanup(); return 0; }

	if( lib_exec::s_execbase::init() ) { cleanup(); return 0; }
	if( lib_dos::s_doslibrary::init() ) { cleanup(); return 0; }

	if( lib_intuition_init() ) { cleanup(); return 0; }
	if( lib_graphics_init() ) { cleanup(); return 0; }
	if( lib_utility_init() ) { cleanup(); return 0; }
	if( lib_gadtools_init() ) { cleanup(); return 0; }
	if( lib_icon_init() ) { cleanup(); return 0; }
	if( lib_layers_init() ) { cleanup(); return 0; }
	if( lib_locale_init() ) { cleanup(); return 0; }
	if( dev_timer_init() ) { cleanup(); return 0; }
	if( dev_audio_init() ) { cleanup(); return 0; }
      */
      return 1;
   }
   ADD2INIT(init,0);

   static void cleanup()
   {
      /*
	dev_audio_cleanup();
	dev_timer_cleanup();
	lib_locale_cleanup();
	lib_layers_cleanup();
	lib_icon_cleanup();
	lib_gadtools_cleanup();
	lib_utility_cleanup();
	lib_graphics_cleanup();
	lib_intuition_cleanup();
	lib_dos::s_doslibrary::cleanup();
	lib_exec::s_execbase::cleanup();

	support_cleanup();
	emulator_cleanup();
	hook_cleanup();
	vmem_cleanup();
	memory::mapping::cleanup();
	log_cleanup();
      */
      emulator::cleanup();
      log::cleanup();
   }
   ADD2EXIT(cleanup,0);

   static int openfunc(struct Library *lh __attribute__((unused)))
   {
      // DEBUG(3) dprintf("emumiga.library: OpenFunc() called.\n");
      return 1;
   }
   ADD2OPENLIB(openfunc,0);

   static void closefunc(struct Library *lh __attribute__((unused)))
   {
      // DEBUG(3) dprintf("emumiga.library: CloseFunc() called.\n");
   }
   ADD2CLOSELIB(closefunc,0);

   // Exported library function, must be C name mangled
   extern "C" int EM_run(char *prg_name, char *args, int *ret_ptr, int stack_size)
   {
      DEBUG(3) dprintf("EM_run() called. prg_name='%s', args='%s', ret_ptr=%p, stack_size=%d\n", prg_name, args, ret_ptr, stack_size);

      // TODO: convert to taglist
      emulator *emu = emulator::getEmulator();
      if(!emu)
	 return ENOMEM;

      return emu->run(prg_name, args, ret_ptr, stack_size);
   }

   // Exported library function, must be C name mangled
   extern "C" void EM_shutdown()
   {
   }

   // Exported library function, must be C name mangled
   extern "C" void EM_dump_mmap()
   {
   }

   // Exported library function, must be C name mangled
   extern "C" int EM_getlog(char *buf, int maxsize)
   {
      return log::get(buf, maxsize);
   }

   // Exported library function, must be C name mangled
   extern "C" void EM_putlog(char *msg)
   {
      log::put(msg);
   }

} // namespace emumiga

// We must provide operator new and delete for the
// memory allocation of instances. Must be done
// outside namespaces
void *operator new(size_t size)
{
   return AllocVec(size,MEMF_PUBLIC|MEMF_CLEAR);
}

void operator delete(void * ptr)
{
   FreeVec(ptr);
}

// Needed if pure virtual methods are defined
// It gets called if a pure virtual method is accidentally
// called. This is a fatal error.
extern "C" void __cxa_pure_virtual()
{
   while (1);   // Just die
}
