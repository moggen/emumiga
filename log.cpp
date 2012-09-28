/*
log.cpp - Logging class

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

namespace emumiga
{

   struct SignalSemaphore log::semaphore;
   char log::buffer[EMUMIGA_LOG_BUFSIZE];
   int log::headidx, log::tailidx;
   struct Task *log::task;
   int log::signo;

   void log::init()
   {
      InitSemaphore(&semaphore);
      headidx = 0;
      tailidx = 0;
      task = NULL;
      signo = -1;
   }

   void log::cleanup()
   {
   }

   int log::get(char *buf, int maxsize)
   {
      // We can not output any debug messages in these functions
      // because they are used for printing out debug messages!
      buf[0]='\0';

      // The semaphore protects the FIFO buffer
      ObtainSemaphore(&semaphore);

      // size counts the actual chars copied to the user buffer
      int size = 0;

      // Only allow one listener
      if(task != NULL) {
	 size = -1;
      }

      while(1) {
	 // Calculate if there is something in the buffer
	 if(headidx >= tailidx) {
	    size = headidx - tailidx;
	 } else {
	    size = EMUMIGA_LOG_BUFSIZE - tailidx;
	 }
	 if(size > 0) {
	    // Ok, copy
	    size = (size > maxsize) ? maxsize : size;
	    for(int i = 0 ; i < size ; i++) {
	       buf[i] = buffer[tailidx + i];
	    }
	    tailidx += size;
	    if(tailidx >= EMUMIGA_LOG_BUFSIZE)
	       tailidx = 0;

	    // Stop and quit
	    break;
	 }

	 // Nothing new in the buffer, we must go to sleep

	 // Allocate a signal
	 task = FindTask(NULL);
	 signo = AllocSignal(-1);
	 if(signo == -1) {
	    break;
	 }
	 SetSignal(0, 1<<signo);

	 // Must release the semaphore during the sleeping
	 ReleaseSemaphore(&semaphore);

	 // Wait for the signal or CTRL-C
	 ULONG signals = Wait((1<<signo) | SIGBREAKF_CTRL_C);

	 ObtainSemaphore(&semaphore);

	 // Free the signal
	 FreeSignal(signo);
	 signo = -1;
	 task = NULL;

	 // If CTRL-C was caught, make sure to set it again for
	 // the user program to handle
	 if(signals & SIGBREAKF_CTRL_C) {
	    SetSignal(SIGBREAKF_CTRL_C, SIGBREAKF_CTRL_C);
	    break;
	 }
      }
      ReleaseSemaphore(&semaphore);

      return size;
   }

   void log::put(char *msg)
   {
      // We can not output any debug messages in these functions
      // because they are used for printing out debug messages!

      // logsem protects the FIFO buffer
      ObtainSemaphore(&semaphore);

      // Copy to the buffer
      while(*msg != '\0') {
	 buffer[headidx++] = *msg++;
	 if(headidx >= EMUMIGA_LOG_BUFSIZE)
	    headidx = 0;
	 if(headidx == tailidx) {
	    // Oops, buffer overrun. Overwrite
	    tailidx++;
	    if(tailidx >= EMUMIGA_LOG_BUFSIZE)
	       tailidx = 0;
	 }
      }

      // Wake up any listener
      if(task != NULL) {
	 Signal(task, 1<<signo);
      }

      ReleaseSemaphore(&semaphore);
   }
}
