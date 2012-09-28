/*
emulator.cpp - Emulation class

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
#include <proto/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>
#include "debug.h"
#include "emulator.h"
#include "hook.h"
#include "memory/vmem.h"
#include "memory/memory.h"
#include "memory/mapping.h"
#include "lib_dos/s_process.h"

namespace emumiga {

using namespace memory;

static int is_init = 0;

static struct Process *emulator_proc = NULL;
static struct SignalSemaphore *emulator_proc_sem;
static int emulator_end_signo;
static struct MsgPort *emulator_port;

static struct Process *starter_proc;
static int starter_signo;

static int proc_error = 0;

/* Scheduling queues */
#define SCHED_READY_SIZE 20
static lib_dos::s_process *sched_ready[SCHED_READY_SIZE];
static int sched_ready_head, sched_ready_tail;
#define SCHED_WAIT_SIZE 20
static lib_dos::s_process *sched_wait[SCHED_WAIT_SIZE];
static lib_dos::s_process *sched_current;

/* Bootcode */
/* static const uint8_t emulator_boot_main[] = { ... }; */
#include "boot_main.h"
static uint32_t boot_main_vaddr;

/* Predecls */
static int createProcess();
static void emulator();
static void handle_run(emumsg_run_t *recv);
static void handle_end(lib_dos::s_process *proc, int emu_status);
static void handle_syscall(emumsg_syscall_t *recv);

int emulator_init()
{
	int i;
	DEBUG(3) dprintf("emulator: emulator_init() called\n");

	emulator_proc_sem = (struct SignalSemaphore *)AllocVec(sizeof(struct SignalSemaphore), MEMF_PUBLIC|MEMF_CLEAR);
	if(emulator_proc_sem == NULL) {
		dprintf("Error: emulator: emulator_init: AllocVec() failed\n");
		return 1;
	}
	InitSemaphore(emulator_proc_sem);

	sched_ready_head = 0;
	sched_ready_tail = 0;
	for(i=0;i<SCHED_WAIT_SIZE;i++)
		sched_wait[i]=NULL;

        /* Set upp virtual memory for main bootstrap */
        boot_main_vaddr = vallocmem_rom(emulator_boot_main,sizeof(emulator_boot_main),"!bootstrap_main");
	if(boot_main_vaddr == 0) {
		dprintf("Error: emulator: emulator_init: vallocmem_rom() failed\n");
		FreeVec(emulator_proc_sem);
		return 2;
	}

	is_init = 1;
	return 0;
}

void emulator_cleanup()
{
	DEBUG(3) dprintf("emulator: emulator_cleanup() called\n");

	if( !is_init ) {
		DEBUG(2) dprintf("Warning: emulator: emulator_cleanup: Not initialized\n");
		return;
	}

	vfreemem_rom(boot_main_vaddr);

	FreeVec(emulator_proc_sem);

	is_init = 0;
}

// Exported library function, must be C name mangled
/* TODO: convert to taglist */
extern "C" int EM_run(char *prg_name, char *args, int *ret_ptr, int stack_size)
{
	int ret = 0;
	struct MsgPort *port;
	emumsg_run_t *runmsg;

	DEBUG(3) dprintf("emulator: EM_run() called. prg_name='%s', args='%s', ret_ptr=%p, stack_size=%d\n", prg_name, args, ret_ptr, stack_size);

	/* emulator_proc_sem protects variable emulator_proc */
	ObtainSemaphore(emulator_proc_sem);

	if(emulator_proc == NULL) {
		ret = createProcess();
	}

	ReleaseSemaphore(emulator_proc_sem);

	if(ret != 0)
		return ret;

	port = CreateMsgPort();
	if(port == NULL){
		dprintf("Error: emulator: EM_run: CreateMsgPort() failed\n");
		return 1;
	}

	runmsg = (emumsg_run_t *)AllocVec(sizeof(emumsg_run_t), MEMF_PUBLIC|MEMF_CLEAR);
	if(runmsg == NULL) {
		dprintf("Error: emulator: EM_run: AllocVec() failed\n");
		return 1;
	}

	/* Init message */
	runmsg->msg.mn_ReplyPort = port;
	runmsg->msg.mn_Length = sizeof(emumsg_run_t);
	runmsg->type = EMUMSGTYPE_RUN;
	runmsg->prg_name = prg_name;
	runmsg->args = args;
	runmsg->caller_proc = (struct Process *)FindTask(NULL);
	runmsg->syscall_port = port;
	runmsg->stack_size = stack_size;

	/* Send to emulator */
	PutMsg( emulator_port, (struct Message *)runmsg );

	/* Handle syscalls */
	ret = emulator_doEvents(port);

	if(ret != 0) {
		/* Not a normal end code.. */
		dprintf("Error: emulator: EM_run: Main event loop exitted abnormally\n");
	}

	/* Extract exit codes */
	ret = runmsg->emu_status;
	*ret_ptr = runmsg->exit_code;

	/* Clean up */
	FreeVec(runmsg);
	DeleteMsgPort(port);

	return ret;
}

int emulator_doEvents(struct MsgPort *port)
{
	int ret;
	emumsg_base_t *recv;

        /* Wait for and get the reply */
        while(1) {

                /* Wait for and get a message */
	        recv = (emumsg_base_t *)GetMsg(port);
	        while(!recv) {
	                WaitPort(port);
	                recv = (emumsg_base_t *)GetMsg(port);
	        }

                if( recv->type == EMUMSGTYPE_RUN ) {
                        /* OK, emulation has ended */
                        return 0;

                } else if( recv->type == EMUMSGTYPE_SYSCALL ) {
			/* Call library connector part 2 */
			ret = call_hook2((emumsg_syscall_t *)recv);
			if(ret == HOOK_DONE) {
				/* Normal, reply back and continue */
				ReplyMsg((struct Message *)recv);
			} else if(ret == HOOK_CALLBACK) {
				/* This was a return from a callback */
				return 1;
			} else {
				DEBUG(1) dprintf("Warning: emulator: mulator_doEvents: Unknown return code from call_hook2(), ignoring.\n");
			}
		} else {
			DEBUG(1) dprintf("Warning: emulator: emulator_doEvents: Unknown message received, ignoring.\n");
                }
        }
}

// Exported library function, must be C name mangled
extern "C" void EM_shutdown()
{
	DEBUG(3) dprintf("emulator: EM_shutdown() called\n");

	/* emulator_proc_sem protects variable emulator_proc */
	ObtainSemaphore(emulator_proc_sem);

	if(emulator_proc != NULL) {
		Signal((struct Task *)emulator_proc,1<<emulator_end_signo);
		emulator_proc = NULL;
	}

	ReleaseSemaphore(emulator_proc_sem);
}

static int createProcess()
{
        struct TagItem tags[] = {
                { NP_Entry, (IPTR)emulator },
                { TAG_DONE, 0 }
        };
	int signals;

	DEBUG(4) dprintf("emulator: createProcess() called\n");

	starter_proc = (struct Process *)FindTask(NULL);
	starter_signo = AllocSignal(-1);
	if(starter_signo==-1) {
		dprintf("Error: emulator: createProcess: AllocSignal() failed!\n");
		return 1;
	}
	SetSignal(0, 1<<starter_signo);

	DEBUG(5) dprintf("emulator: createProcess: Creating process\n");

	emulator_proc = CreateNewProcTagList(tags);
	if(emulator_proc == NULL) {
		dprintf("Error: emulator: createProcess: CreateNewProcTagList() failed!\n");
		FreeSignal(starter_signo);
		return 1;
	}

	DEBUG(5) dprintf("emulator: createProcess: Waiting for process to initialize...\n");

	signals = Wait(1<<starter_signo);
	FreeSignal(starter_signo);

	DEBUG(5) dprintf("emulator: createProcess: Process is initialized\n");

	return proc_error;
}

static void emulator()
{
	struct MsgPort *port;
	int port_signo;
	ULONG signals;
	emumsg_base_t *recv;
	int i, retc, retc2;

	DEBUG(3) dprintf("emulator: emulator: Process starting\n");

	DEBUG(5) dprintf("emulator: emulator: Creating message port\n");

	port = CreateMsgPort();
	if(port == NULL){
		dprintf("Error: emulator: emulator: CreateMsgPort() failed\n");
		proc_error = 1;
		Signal((struct Task *)starter_proc,1<<starter_signo);
		return;
	}
	emulator_port = port;
	port_signo = port->mp_SigBit;

	DEBUG(5) dprintf("emulator: emulator: Allocating end signal\n");

	emulator_end_signo = AllocSignal(-1);
	if(emulator_end_signo==-1) {
		dprintf("Error: emulator: emulator: AllocSignal() failed\n");
		DeleteMsgPort(port);
		proc_error = 1;
		Signal((struct Task *)starter_proc,1<<starter_signo);
		return;
	}
	SetSignal(0, 1<<emulator_end_signo);

	DEBUG(5) dprintf("emulator: emulator: Initialization done, signalling creator\n");

	Signal((struct Task *)starter_proc,1<<starter_signo);

	DEBUG(4) dprintf("emulator: emulator: Entering main loop\n");

	signals = 0;
	while(1) {
		while((recv = (emumsg_base_t *)GetMsg(port))){

			DEBUG(4) dprintf("emulator: emulator: Received message\n");

			if( recv->type == EMUMSGTYPE_RUN ) {
				handle_run((emumsg_run_t *)recv);
			}

			if( recv->type == EMUMSGTYPE_SYSCALL ) {
				handle_syscall((emumsg_syscall_t *)recv);
			}

			if( recv->type == EMUMSGTYPE_DUMP_MMAP ) {
				memory::dump();
				ReplyMsg((struct Message *)recv);
			}
		}
		if((signals & (1<<emulator_end_signo))){
			DEBUG(4) dprintf("emulator: emulator: End signal caught\n");
			break;
		}

		/* Run emulation */
		if(sched_ready_head != sched_ready_tail) {
			/* Dequeue first ready process */
			sched_current = sched_ready[sched_ready_head++];
			if(sched_ready_head == SCHED_READY_SIZE)
				sched_ready_head = 0;

	                /* Run the next instruction */
	                retc = sched_current->cpu->run_instruction();

	                if(retc == CPU_I_OK) {
				/* All ok, enqueue */
				i = sched_ready_tail+1;
				if(i==SCHED_READY_SIZE)
					i = 0;
				if(i==sched_ready_head) {
					dprintf("Error: emulator: emulator: sched_ready queue is full\n");
					handle_end(sched_current,11);
					sched_current = NULL;
					continue;
				}
				sched_ready[sched_ready_tail] = sched_current;
				sched_ready_tail = i;
				sched_current = NULL;

	                } else if(retc == CPU_I_EMULATOR_HOOK) {
	                        /* Handle emulator hook */

				/* Copy the hook parameters */
				sched_current->syscall_msg->mnum = sched_current->cpu->emu_module;
				sched_current->syscall_msg->func = sched_current->cpu->emu_func;

				/* Call library connector part 1 */
	                        retc2 = call_hook1(sched_current->syscall_msg);

	                        if(retc2 == HOOK_DONE) {
	                                /* Hook done, continue to execute */
					/* Put back in ready queue */
					i = sched_ready_tail+1;
					if(i==SCHED_READY_SIZE)
						i = 0;
					if(i==sched_ready_head) {
						dprintf("Error: emulator: emulator: sched_ready queue is full\n");
						handle_end(sched_current,12);
						sched_current = NULL;
						continue;
					}
					sched_ready[sched_ready_tail] = sched_current;
					sched_ready_tail = i;
					sched_current = NULL;

	                        } else if(retc2 == HOOK_SYSCALL) {
	                                /* A syscall to be done */

					/* Send the syscall message */
					PutMsg(sched_current->syscall_port, (struct Message *)sched_current->syscall_msg);

					/* Put in wait list */
					for(i=0;i<SCHED_WAIT_SIZE;i++) {
						if(sched_wait[i]==NULL) {
							sched_wait[i]=sched_current;
							break;
						}
					}
					if(i == SCHED_WAIT_SIZE) {
						dprintf("Error: emulator: emulator: sched_wait list is full\n");
						handle_end(sched_current,13);
						sched_current = NULL;
						continue;
					}
					sched_current = NULL;

	                        } else if(retc2 == HOOK_END_PROC) {
	                                /* Process ended */
					handle_end(sched_current,sched_current->syscall_msg->emu_status);
					sched_current = NULL;
	                                continue;
	                        } else {
	                                /* Unknown return code */
	                                dprintf("Error: emulator: emulator: Unknown return code from call_hook\n");
					handle_end(sched_current,14);
					sched_current = NULL;
					continue;
	                        }
	                } else if(retc != CPU_I_OK) {
	                        /* Something went bad */
	                        dprintf("Error: emulator: emulator: Unknown return code from cpu_run_instruction\n");
				handle_end(sched_current,15);
				sched_current = NULL;
				continue;
			}

		} else {
			/* No runnable processes? */
			DEBUG(5) dprintf("emulator: emulator: Sleeping...\n");
			signals = Wait( (1<<port_signo) | (1<<emulator_end_signo) );
			DEBUG(5) dprintf("emulator: emulator: ...Woke up\n");
		}
	}

	DEBUG(4) dprintf("emulator: emulator: Cleanup\n");

	/* emulator_proc_sem protects variable emulator_proc */
	ObtainSemaphore(emulator_proc_sem);
	emulator_proc = NULL;
	ReleaseSemaphore(emulator_proc_sem);

	FreeSignal(emulator_end_signo);
	DeleteMsgPort(emulator_port);

	DEBUG(3) dprintf("emulator: emulator: Process ending\n");
}

static void handle_run(emumsg_run_t *recv)
{
	lib_dos::s_process *proc;
	int i;
	mapping *entry;

	proc = lib_dos::s_process::get_real(recv->caller_proc);
	if(proc == NULL) {
		recv->emu_status = 1;
		ReplyMsg((struct Message *)recv);
		return;
	}
	if(proc->cpu != NULL) {
		DEBUG(1) dprintf("Warning: emulator: emulator: Attempt made to start multiple emulations from the same process\n");
		recv->emu_status = 2;
		ReplyMsg((struct Message *)recv);
		return;
	}

	/* Set upp an emulation context */
	proc->cpu = new cpu::cpu();
	if(proc->cpu == NULL) {
		dprintf("Error: emulator: emulator: Could not create cpu structure\n");
		handle_end(proc,3);
		return;
	}
	proc->cpu->init();

	DEBUG(6) {
		proc->cpu->flags |= CPU_FLAG_DISASM;
	}

	/* Init back reference */
	proc->cpu->proc = proc;

	/* Set up parameters for the bootstrap */
	proc->vaddr_prg_name = vallocmem_rom((uint8_t *)recv->prg_name,strlen(recv->prg_name)+1,"!cmdpath");
	if(proc->vaddr_prg_name == 0){
		dprintf("Error: emulator: emulator: Could not allocate rom memory for program name\n");
		handle_end(proc,4);
		return;
	}

	/* Set up and copy command line */
	proc->vaddr_args = vallocmem(strlen(recv->args)+1,"!cmdline");
	if(proc->vaddr_args == 0){
		dprintf("Error: emulator: emulator: Could not allocate memory for args\n");
		handle_end(proc,5);
		return;
	}
	memory::writeData(proc->vaddr_args, strlen(recv->args)+1, (uint8_t *)recv->args);

	/* Allocate a stack */
	proc->stacksize = recv->stack_size;
	proc->vaddr_stack = vallocmem(proc->stacksize,"!stack");
	if(proc->vaddr_stack == 0){
		dprintf("Error: emulator: emulator: Could not allocate memory for stack\n");
		handle_end(proc,6);
		return;
	}

	/* Prepare CPU */
	proc->cpu->pc = boot_main_vaddr;
	proc->cpu->a[7] = proc->vaddr_stack + proc->stacksize;
	proc->cpu->a[0] = proc->vaddr_args;
	proc->cpu->d[0] = strlen(recv->args)+1;
	proc->cpu->a[5] = proc->vaddr_prg_name;

	/* Add it to the ready queue */
	i = sched_ready_tail+1;
	if(i==SCHED_READY_SIZE)
		i = 0;
	if(i==sched_ready_head) {
		dprintf("Error: emulator: emulator: sched_ready queue is full\n");
		handle_end(proc,7);
		return;
	}
	sched_ready[sched_ready_tail] = proc;
	sched_ready_tail = i;

	/* Save run message for later */
	proc->run_msg = recv;

	/* Extract syscall port */
	proc->syscall_port = recv->syscall_port;

	/* Allocate memory for syscall memory */
	proc->syscall_msg = (emumsg_syscall_t *)AllocVec(sizeof(emumsg_syscall_t), MEMF_PUBLIC|MEMF_CLEAR);
	if(proc->syscall_msg == NULL) {
		dprintf("Error: emulator: emulator: Could not allocate memory for syscall message structure\n");
		handle_end(proc,8);
		return;
	}

	/* Init syscall message */
	proc->syscall_msg->msg.mn_ReplyPort = emulator_port;
	proc->syscall_msg->msg.mn_Length = sizeof(emumsg_syscall_t);
	proc->syscall_msg->type = EMUMSGTYPE_SYSCALL;
	proc->syscall_msg->proc = proc;
}

static void handle_end(lib_dos::s_process *proc, int emu_status)
{
	if(proc == NULL)
		return;

	/* Reply on the run message */
	if(proc->run_msg != NULL) {
		/* Extract process return code */
		proc->run_msg->exit_code = proc->cpu->d[0];
		/* Emulation staus code */
		proc->run_msg->emu_status = emu_status;
		ReplyMsg((struct Message *)proc->run_msg);
	}
	proc->run_msg = NULL;

	/* Free syscall msg, stack, args and cpu structure */
	if(proc->syscall_msg != NULL)
		FreeVec(proc->syscall_msg);
	proc->syscall_msg = NULL;
	proc->syscall_port = NULL;
	if(proc->vaddr_stack != 0)
		vfreemem(proc->vaddr_stack);
	proc->vaddr_stack = 0;
	if(proc->vaddr_args != 0)
		vfreemem(proc->vaddr_args);
	proc->vaddr_args = 0;
	if(proc->vaddr_prg_name != 0)
		vfreemem_rom(proc->vaddr_prg_name);
	proc->vaddr_prg_name = 0;
	if(proc->cpu != NULL)
		delete proc->cpu;
	proc->cpu = NULL;

}

static void handle_syscall(emumsg_syscall_t *recv)
{
	int i, retc;

	/* Get our process from the wait queue */
	for(i=0;i<SCHED_WAIT_SIZE;i++) {
		if(sched_wait[i]==recv->proc) {
			sched_wait[i]=NULL;
			break;
		}
	}
	if(i == SCHED_WAIT_SIZE) {
		/* Not found!! */
		dprintf("Error: emulator: handle_syscall: Could not find process in wait list\n");
		return;
	}

	/* Call library connector part 3 */
	retc = call_hook3(recv);

	if(retc == HOOK_DONE) {
		/* All ok, put back in run queue */
		i = sched_ready_tail+1;
		if(i==SCHED_READY_SIZE)
			i = 0;
		if(i==sched_ready_head) {
			dprintf("Error: emulator: handle_syscall: sched_ready queue is full\n");
			handle_end(recv->proc,9);
			return;
		}
		sched_ready[sched_ready_tail] = recv->proc;
		sched_ready_tail = i;

	} else if(retc == HOOK_END_PROC) {

		handle_end(recv->proc,0);

	} else {
		dprintf("Error: emulator: handle_syscall: Unknown return code from call_syshook\n");
		handle_end(recv->proc,10);
	}
}

lib_dos::s_process *emulator_current_process()
{
	DEBUG(4) dprintf("emulator: emulator_current_process() called\n");

	return sched_current;
}

// Exported library function, must be C name mangled
extern "C" void EM_dump_mmap()
{
	struct Process *proc;
	struct MsgPort *port;
	emumsg_base_t *msg;

	DEBUG(3) dprintf("emulator: EM_dump_mmap() called.\n");

	/* emulator_proc_sem protects variable emulator_proc */
	ObtainSemaphore(emulator_proc_sem);

	proc = emulator_proc;

	ReleaseSemaphore(emulator_proc_sem);

	if(proc == NULL)
		return;

	port = CreateMsgPort();
	if(port == NULL){
		dprintf("Error: emulator: EM_dump_mmap: CreateMsgPort() failed\n");
		return;
	}

	msg = (emumsg_base_t *)AllocVec(sizeof(emumsg_base_t), MEMF_PUBLIC|MEMF_CLEAR);
	if(msg == NULL) {
		dprintf("Error: emulator: EM_dump_mmap: AllocVec() failed\n");
		return;
	}

	/* Init message */
	msg->msg.mn_ReplyPort = port;
	msg->msg.mn_Length = sizeof(emumsg_base_t);
	msg->type = EMUMSGTYPE_DUMP_MMAP;

	/* Send to emulator */
	PutMsg( emulator_port, (struct Message *)msg );

	/* Wait for the reply */
	WaitPort( port );
	GetMsg( port );

	/* Clean up */
	FreeVec(msg);
	DeleteMsgPort(port);
}

void emulator_sp_check(cpu::cpu *cpu)
{
	uint32_t sp;
	lib_dos::s_process *proc;

	DEBUG(5) dprintf("emulator: emulator_sp_check() called\n");

	sp = cpu->a[7];
	proc = cpu->proc;

	/* Check if SP is inside the stack */
	if(
		proc->vaddr_stack <= sp &&
		sp < (proc->vaddr_stack + proc->stacksize)
	) {
		/* Yep, inside clear all up to SP */
		memory::clearRange(
			proc->vaddr_stack,
			(sp - proc->vaddr_stack)
		);
	}
}

} // namespace emumiga
