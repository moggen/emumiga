/*
context_collection.cpp - Collection class for emulation contexts

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

   context_collection::context_collection()
   {
      DEBUG(3) dprintf("context_collection::context_collection() called. this=%p\n", this);

      array_length = 20;
      context_array = (context **)AllocVec(array_length * sizeof(context *), MEMF_PUBLIC|MEMF_CLEAR);
      if(context_array == NULL)
	 array_length = 0;
   }

   context_collection::~context_collection()
   {
      DEBUG(3) dprintf("context_collection::~context_collection() called. this=%p\n", this);
      if(context_array != NULL)
	 FreeVec(context_array);
   }

   int context_collection::add(context *ctx)
   {
      DEBUG(3) dprintf("context_collection::add() called. this=%p, ctx=%p\n", this, ctx);
      int i;
      for(i = 0 ; i < array_length ; i++) {
	 if(context_array[i] == NULL)
	    break;
      }

      if(i == array_length) {
	 context **new_array = (context **)AllocVec(2 * array_length * sizeof(context *), MEMF_PUBLIC|MEMF_CLEAR);
	 if(new_array == NULL)
	    return ENOMEM;

	 CopyMem(context_array, new_array, array_length * sizeof(context *));
	 FreeVec(context_array);
	 context_array = new_array;
	 array_length *= 2;
      }

      context_array[i] = ctx;
      
      return 0;
   }

   int context_collection::rem(context *ctx)
   {
      DEBUG(3) dprintf("context_collection::rem) called. this=%p, ctx=%p\n", this, ctx);
      for(int i = 0 ; i < array_length ; i++) {
	 if(context_array[i] == ctx) {
	    context_array[i] = NULL;
	    return 0;
	 }
      }
      return ENOENT;
   }

   int context_collection::contains(context *ctx)
   {
      DEBUG(3) dprintf("context_collection::contains() called. this=%p, ctx=%p\n", this, ctx);

      for(int i = 0 ; i < array_length ; i++) {
	 if(context_array[i] == ctx) {
	    return 1;
	 }
      }
      return 0;
   }

   context *context_collection::findByTask(struct Process *proc)
   {
      DEBUG(3) dprintf("context_collection::findByTask() called. this=%p, proc=%p\n", this, proc);

      context *ctx;

      for(int i = 0 ; i < array_length ; i++) {
	 ctx = context_array[i];
	 if(ctx != NULL) {
	    if(ctx->getProcess() == proc)
	       return ctx;
	 }
      }
      return NULL;
   }

} // namespace emumiga
