/*
robj_list.cpp - Real object list class

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

   robj_list::robj_list()
   {
      list = NULL;
      capacity = 0;
      length = 0;
   }

   robj_list::~robj_list()
   {
      if(list != NULL)
	 FreeVec(list);
   }

   int robj_list::add(virtual_object *vobj)
   {
      if(vobj == NULL)
	 return EINVAL;

      if(length == capacity) {

	 int new_capacity = 2*capacity ?: 20;

	 tuple *new_list = (tuple *)AllocVec(sizeof(tuple)*new_capacity, MEMF_PUBLIC|MEMF_CLEAR);

	 if(new_list == NULL)
	    return ENOMEM;

	 if(list != NULL) {
	    CopyMem(list, new_list, sizeof(tuple)*capacity);
	    FreeVec(list);
	 }

	 list = new_list;
	 capacity = new_capacity;
      }

      void *address = vobj->get_real_address();
      int len = vobj->get_real_size();

      int i;
      for( i=0 ; i<length ; i++ ) {
	 if(list[i].address < address)
	    continue;

	 // same or greater address
	 break;
      }

      // insert on position i
      for( int j=length; j>i; j-- )
	 list[j] = list[j-1];

      list[i].address = address;
      list[i].vobj = vobj;
      list[i].length = len;

      length++;

      return 0;
   }

   int robj_list::rem(virtual_object *vobj)
   {
      if(vobj == NULL)
	 return EINVAL;

      for(int i=0; i<length ; i++) {
	 if(list[i].vobj == vobj) {
	    // Move everything above down one step
	    for( ++i ; i<length ; i++ )
	       list[i-1] = list[i];
	    length--;
	    return 0;
	 }
      }

      // Not found
      return ENOENT;
   }

   virtual_object *robj_list::find_object(void *real_address, object_type *type)
   {
      for(int i=0; i<length ; i++) {
	 if(list[i].address > real_address)
	    break;             // They are in order

	 if(list[i].address == real_address) {
	    if(type->equals(list[i].vobj->get_type()))
	       return list[i].vobj;
	 }
      }
      return NULL;
   }

} // namespace emumiga
