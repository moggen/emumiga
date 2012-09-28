/*
context.cpp - Emulation context class

Copyright (C) 2011  Magnus Öberg

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

namespace emumiga {

   context::context(struct Process *my_proc, struct MsgPort *my_port):
      proc(my_proc), port(my_port)
   {
      DEBUG(3) dprintf("context::context() called. this=%p, my_proc=%p, my_port=%p\n", this, my_proc, my_port);

      emu = emulator::getEmulator();
   }

   context::~context()
   {
      DEBUG(3) dprintf("context::~context() called. this=%p\n", this);

      DeleteMsgPort(port);
   }

   context *context::createContext()
   {
      DEBUG(3) dprintf("context::createContext() called.\n");

      struct Process *my_proc = (struct Process *)FindTask(0);
      struct MsgPort *my_port = CreateMsgPort();
      if(my_port == NULL)
	 return NULL;

      return new context(my_proc, my_port);
   }

   struct Process *context::getProcess()
   {
      return proc;
   }
  
   struct MsgPort *context::getPort()
   {
      return port;
   }

   int context::init_run(char const *name, char const *args, int stack)
   {
      DEBUG(3) dprintf("context::init_run() called. this=%p\n", this);

      int len;

      run_name_address = 0;
      if(name) {
         len = strlen(name);
         run_name_address = alloc_mem(len+1);
         if(run_name_address == 0)
            return ENOMEM;

         strcpy((char *)mem_to_real(run_name_address), name);

         // Special extension to bootstrap
         cpu.a[5] = run_name_address;
      }

      run_args_address = 0;
      if(args) {
         len = strlen(args);
         run_args_address = alloc_mem(len + 1);
         if(run_args_address == 0)
            return ENOMEM;

         strcpy((char *)mem_to_real(run_args_address), args);

         cpu.a[0] = run_args_address;
         cpu.d[0] = len + 1;
      }    

      run_stack = stack;

      stack_mapping = user_mapping::create(stack);
      if(stack_mapping == NULL)
	 return ENOMEM;

      cpu.a[7] = stack_mapping->get_address() + stack;

      cpu.pc = emu->get_bootstrap_address();

      return 0;
   }

   int context::getRetVal()
   {
      return run_retval;
   }

   message *context::allocMessage()
   {
      DEBUG(3) dprintf("context::allocMessage() called. this=%p\n", this);

      message *msg = cached_message;
      if(msg)
	 cached_message = NULL;
      else
	 msg = new message(this);

      return msg; 
   }

   void context::freeMessage(message *msg)
   {
      DEBUG(3) dprintf("context::freeMessage() called. this=%p\n", this);

      if(cached_message == NULL)
	 cached_message = msg;
      else
	 delete msg;
   }

   context::run_status context::runBlock()
   {
      DEBUG(3) dprintf("context::runBlock() called. this=%p\n", this);

      cpu68k::returncode retc = cpu.run_block(this);
      switch(retc) {
	 case cpu68k::I_OK:
	    return OK;

	 case cpu68k::I_HOOK:
	    return HOOK;

	 default:
	    dprintf("Error: context::runBlock(): CPU error raised, code %d\n", retc);
      }
      return EXIT;
   }

   uint32_t context::alloc_mem(uint32_t size)
   {
      user_mapping *mapping = user_mapping::create(size);
      if(mapping == NULL)
	 return 0;

      return mapping->get_address();
   }

   void context::free_mem(uint32_t address)
   {
      virtual_mapping *mapping = emu->v_space.find_mapping_exact(address);
      if(mapping == NULL)
	 return;

      if(mapping->type != virtual_mapping::TYPE_USER)
         return;

      mapping->dispose();
   }

   uint8_t *context::mem_to_real(uint32_t address)
   {
      virtual_mapping *mapping = emu->v_space.find_mapping(address);
      if(mapping == NULL)
	 return NULL;

      if(mapping->type != virtual_mapping::TYPE_USER)
         return NULL;

      user_mapping *umapping = static_cast<user_mapping *>(mapping);

      address -= umapping->get_address();

      return umapping->real_memory + address;
   }

   int context::print_symbol(uint32_t address, char *str, int str_length)
   {
      return emu->v_space.print_symbol(address, str, str_length);
   }

} // namespace emumiga
