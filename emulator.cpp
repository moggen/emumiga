/*
emumlator.cpp - Emulation class

Copyright (C) 2009-2012  Magnus Öberg

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
#include "modules/exec/exec.h"
#include "modules/dos/dos.h"

namespace emumiga {

   struct SignalSemaphore emulator::semaphore;
   emulator *emulator::singleton = NULL;

   /* Include the bootstrap with signature:
    * uint8_t const emulator::bootstrap_data[]
    */
   #include "bootstrap.h"

   emulator::emulator():
      current_context(NULL), proc(NULL), port(NULL), creatorProc(NULL),
      creatorSigNo(0), bootstrap_mapping(NULL), int_module(0),
      exec_module(NULL), dos_module(NULL)
   {
      DEBUG(3) dprintf("emulator::emulator() called. this=%p\n", this);
   }

   emulator::~emulator()
   {
      DEBUG(3) dprintf("emulator::~emulator() called. this=%p\n", this);
   }

   void emulator::init()
   {
      DEBUG(3) dprintf("emulator::init() called.\n");
      InitSemaphore(&semaphore);
      singleton = NULL;
   }

   void emulator::cleanup()
   {
      DEBUG(3) dprintf("emulator::cleanup() called.\n");
      delete singleton;
   }

   // USER + EMULATOR
   emulator *emulator::getEmulator()
   {
//      DEBUG(3) dprintf("emulator::getEmulator() called.\n");

      if(singleton != NULL)
         return singleton;

      ObtainSemaphore(&semaphore);
    
      if(singleton == NULL) {
         singleton = new emulator();
         if(singleton != NULL) {
            int retc = singleton->setUpEmulator();
            if(retc) {
               DEBUG(1) dprintf("Warning: emulator::getEmulator(): setUpEmulator() returned code: %d.\n", retc);
            }
         }
      }
         
      ReleaseSemaphore(&semaphore);

      return singleton;
   }

   // USER
   int emulator::setUpEmulator()
   {
      DEBUG(3) dprintf("emulator::setUpEmulator() called. this=%p\n", this);

      int retc;

      // Internal module
      int_module = new internal_module();
      if(!int_module)
	 return ENOMEM;

      retc = int_module->init();
      if(retc)
         return retc;

      // Exec module
      exec_module = new exec::main();
      if(!exec_module)
	 return ENOMEM;

      retc = exec_module->init();
      if(retc)
	 return retc;

      // Dos module
      dos_module = new dos::main();
      if(!dos_module)
	 return ENOMEM;

      retc = dos_module->init();
      if(retc)
	 return retc;

      // Bootstrap
      bootstrap_mapping = new rom_mapping(sizeof(bootstrap_data), bootstrap_data);
      if(!bootstrap_mapping)
	 return ENOMEM;

      if(v_space.add_mapping(bootstrap_mapping) == 0xFFFFFFFF)
	 return ENOMEM;

      // Spawn emulation process
      char this_hex[17];
      sprintf(this_hex, "%p", this);

      // Set environment variable with "this" pointer.
      // It is inherited by the new process
      if(!SetVar("EMUMIGA_CREATOR", this_hex, -1, GVF_LOCAL_ONLY))
	 return ENOMEM;

      creatorProc = (struct Process *)FindTask(0);
      creatorSigNo = AllocSignal(-1);

      if(creatorSigNo == -1)
	 return ENOMEM;

      SetSignal(0, 1 << creatorSigNo);

      struct TagItem tags[] = {
	 { NP_Entry, (IPTR)processEntry },
	 { TAG_DONE, 0 }
      };

      struct Process *new_proc = CreateNewProcTagList(tags);
    
      if(!new_proc) {
	 FreeSignal(creatorSigNo);
	 return ENOMEM;
      }
    
      Wait(1 << creatorSigNo);
    
      FreeSignal(creatorSigNo);
    
      DeleteVar("EMUMIGA_CREATOR", GVF_LOCAL_ONLY);

      return 0;
   }

   // EMULATOR
   void emulator::processEntry()
   {
      DEBUG(3) dprintf("emulator::processEntry() called.\n");
 
      char this_hex[17];

      if(GetVar("EMUMIGA_CREATOR", this_hex, 17, GVF_LOCAL_ONLY) < 1)
	 return;

      emulator *emu;

      if(sscanf(this_hex, "%p", &emu) != 1)
	 return;

      int res = emu->setUpEmulator2();

      Signal((struct Task *)emu->creatorProc, 1 << emu->creatorSigNo);

      if(!res)
	emu->mainLoop();
   }

   // EMULATOR
   int emulator::setUpEmulator2()
   {
      DEBUG(3) dprintf("emulator::setUpEmulator2() called. this=%p\n", this);

      proc = (struct Process *)FindTask(0);
      port = CreateMsgPort();
      if(port == NULL)
	 return ENOMEM;

      return 0;
   }

   // EMULATOR
   void emulator::mainLoop()
   {
      DEBUG(3) dprintf("emulator::mainLoop() called. this=%p\n", this);

      struct Message *aros_msg;
      message *msg;
      context *ctx;

      while(1) {
	 while((aros_msg = GetMsg(port))){
	    msg = message::fromAros(aros_msg);
	    ctx = msg->getContext();

	    switch(msg->getCmd()) {
	       case message::RUN_CMD:
		  sched.pushToReadyQueue(ctx);
		  msg->free();
		  break;

	       case message::SYS_RETURN:
		  sched.remFromWaitQueue(ctx);
		  if(ctx->currentModule) {
                     current_context = ctx;
		     ctx->currentModule->syscall_hook_3(ctx);
                     current_context = NULL;
                  }
		  sched.pushToReadyQueue(ctx);
		  msg->free();
		  break;

	       case message::DUMP_MEM:
		  break;

	       default:
		  DEBUG(2)
		     dprintf("emulator::mainLoop(): Unknown message received, ignoring\n");
		  break;
	    }
	 }

	 ctx = sched.popFromReadyQueue();

	 if(ctx) {
            current_context = ctx;
	    context::run_status runStatus = ctx->runBlock();
	    switch(runStatus) {
	       case context::OK:
		  sched.pushToReadyQueue(ctx);
		  break;

	       case context::HOOK:
		  ctx->currentModule = module_reg.findByID((int)ctx->cpu.emu_module);
		  if(ctx->currentModule){
		     module::hook_status ret = ctx->currentModule->syscall_hook_1(ctx);
		     switch(ret) {
                        case module::HOOK_DONE:
                           sched.pushToReadyQueue(ctx);
                           break;

                        case module::HOOK_SYSCALL:
                           msg = ctx->allocMessage();
                           msg->setCmd(message::SYS_CALL);
                           msg->sendToUser();
                           sched.addToWaitQueue(ctx);
                           break;

                        case module::HOOK_ERROR:
                        case module::HOOK_EXIT:
                           msg = ctx->allocMessage();
                           msg->setCmd(message::RUN_EXIT);
                           msg->sendToUser();
                           break;

                        default:
                           DEBUG(2) dprintf("emulator::mainLoop(): Unknown return code from hook::syscall_1(), ignoring\n");
                           break;
		     }
		  }
		  break;

	       case context::EXIT:
		  msg = ctx->allocMessage();
		  msg->setCmd(message::RUN_EXIT);
		  msg->sendToUser();
		  break;

	       default:
                  DEBUG(2) dprintf("emulator::mainLoop(): Unknown return code from context::runBlock(), ignoring\n");
                  break;
                  
	    }
            current_context = NULL;

	 } else {
	    // Nothing in queue
	    DEBUG(5) dprintf("emulator::mainLoop(): Nothing to do, sleeping...\n");
	    WaitPort(port);
	    DEBUG(5) dprintf("emulator::mainLoop(): Woke up!\n");
	 }
      }
   }

   // USER
   context *emulator::getContext()
   {
      DEBUG(3) dprintf("emulator::getContext() called. this=%p\n", this);

      struct Process *process = (struct Process *)FindTask(0);

      context *ctx = getContextByTask(process);

      if(ctx)
	 return ctx;

      ctx = context::createContext();
      contexts.add(ctx);
      return ctx;
   }

   // USER
   context *emulator::getContextByTask(struct Process *process)
   {
      DEBUG(3) dprintf("emulator::getContextByTask() called. this=%p, process=%p\n", this, process);

      return contexts.findByTask(process);
   }

   // USER
   void emulator::deleteContext(context *ctx)
   {
      DEBUG(3) dprintf("emulator::deleteContext() called. this=%p, ctx=%p\n", this, ctx);

      if(!ctx)
	 return;

      contexts.rem(ctx);
      delete ctx;
   }

   // USER
   struct MsgPort *emulator::getPort()
   {
      return port;
   }

   // USER
   int emulator::run(char const *prg_name, char const *args, int *ret_ptr, int stack_size)
   {
      DEBUG(3) dprintf("emulator::run() called. this=%p, prg_name='%s', args='%s', ret_ptr=%p, stack_size=%d\n", this, prg_name, args, ret_ptr, stack_size);

      int retc;

      context *ctx = getContext();
      if(!ctx)
	 return ENOMEM;

      retc = ctx->init_run(prg_name, args, stack_size);
      if(retc) {
	 deleteContext(ctx);
	 return retc;
      }

      message *msg = new message(ctx);
      if(!msg) {
	 deleteContext(ctx);
	 return ENOMEM;
      }

      msg->setCmd(message::RUN_CMD);
      msg->sendToEmulator();

      doEvents(ctx);

      delete msg;

      retc = ctx->getRetVal();

      deleteContext(ctx);

      return retc;
   }

   // USER
   int emulator::doEvents(context *ctx)
   {
      DEBUG(3) dprintf("emulator::doEvents() called. this=%p, ctx=%p\n", this, ctx);

      struct Message *aros_msg;
      message *msg;

      while(1) {
	 while((aros_msg = GetMsg(ctx->port))){
	    msg = message::fromAros(aros_msg);

	    switch(msg->getCmd()) {
	       case message::RUN_EXIT:
		  msg->free();
		  return 0;

	       case message::SYS_CALL:
		  if(ctx->currentModule){
                     module::hook_status ret = ctx->currentModule->syscall_hook_2(ctx);
		     if(ret == module::HOOK_DONE){
			msg->setCmd(message::SYS_RETURN);
			msg->sendToEmulator();
		     } else if(ret == module::HOOK_CALLBACK) {
			msg->free();
			return 1;
		     } else {
			DEBUG(2)
			   dprintf("emulator::doEvents(): Unknown return code from hook::syscall_2(), ignoring\n");
			msg->free();
		     }
		  }
		  break;

	       default:
		  DEBUG(2)
		     dprintf("emulator::doEvents(): Unknown message received, ignoring\n");
		  msg->free();
		  break;
	    }
	 }
	 // Nothing in queue
	 DEBUG(5) dprintf("emulator::doEvents(): Nothing to do, sleeping...\n");
	 WaitPort(ctx->port);
	 DEBUG(5) dprintf("emulator::doEvents(): Woke up!\n");
      }

      return 0;
   }

   uint32_t emulator::get_bootstrap_address()
   {
      return bootstrap_mapping->get_address();
   }

} // namespace emumiga
