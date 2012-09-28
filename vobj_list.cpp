/*
vobj_list.cpp - Virtual object list class

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

   vobj_list::vobj_list()
   {
      list = NULL;
      capacity = 0;
      length = 0;
   }

   vobj_list::~vobj_list()
   {
      if(list != NULL)
	 FreeVec(list);
   }

   int vobj_list::dispose_all()
   {
      // Special care must be taken in this method.
      // Virtual objects should remove themselves
      // from the virtual mapping as part of their
      // disposal. This will concurrently modify
      // the list we are looping through.

      int old_length;
      while(length > 0) {
         old_length = length;
         list[0].vobj->dispose();
         if(length == old_length) {
            // This object did not remove itself from us!!
            rem(list[0].vobj);
         }
      }
      return 0;
   }

   int vobj_list::add(virtual_object *vobj, bool no_clean)
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

      uint32_t address = vobj->get_address();
      uint32_t len = vobj->get_size();
      uint32_t prio = vobj->get_prio();

      int i;
      bool run_again;

      do {
         run_again = false;

         for( i=0 ; i<length ; i++ ) {

            if(list[i].address+list[i].length <= address)
               continue;  // completely before our object

            if(list[i].address >= address+len)
               break;   // completely after our object

            // So, we have overlap!
            if(no_clean) {
               // Overlap is ok
               if(list[i].address == address && list[i].prio > prio)
                  continue;  // If we have lower prio we should be after this entry

               // same or greater address, same or lower prio
               break;
            } else {
               // Overlap is not ok, we have to clean up the old entries
               DEBUG(5) dprintf("vobj_list::add(): Cleaning forced, virtual_object: %p\n", list[i].vobj);
               list[i].vobj->dispose();

               // This will modify the list we are iterating over, restart
               run_again = true;
               break;
            }
         }
      } while (run_again);

      // insert on position i
      for( int j=length; j>i; j-- )
	 list[j] = list[j-1];

      list[i].address = address;
      list[i].vobj = vobj;
      list[i].length = len;
      list[i].prio = prio;

      length++;

      return 0;
   }

   int vobj_list::rem(virtual_object *vobj)
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

   virtual_object *vobj_list::find_object(uint32_t address, object_type *type)
   {
      for(int i=0; i<length ; i++) {
	 if(list[i].address > address)
	    break;             // They are in order

	 if(list[i].address == address) {
	    if(type->equals(list[i].vobj->get_type()))
	       return list[i].vobj;
	 }
      }
      return NULL;
   }

   int vobj_list::count()
   {
      return length;
   }

   virtual_object *vobj_list::find_containing(uint32_t address, uint32_t *byte_count)
   {
      int best = -1;
      uint32_t best_prio = 0;

      for(int i=0; i<length ; i++) {
	 if(list[i].address > address)
	    break;             // They are in order

	 if( address < (list[i].address + list[i].length) ) {
	    if(best < 0) {
	       best = i;
	       best_prio = list[i].prio;
	    } else {
	       if(best_prio < list[i].prio) {
		  best = i;
		  best_prio = list[i].prio;
	       }
	    }
	 }
      }
      if(best < 0)
	 return NULL;

      virtual_object *vobj = list[best].vobj;

      if(byte_count) {
	 // Find how much we can read from vaddr and still remain in 
	 // the current vobject, or how much is acessable that is not
	 // overshadowed by higher prio objects
	 uint32_t end = vobj->get_address() + vobj->get_size();

	 for( int i=0; i<length; i++ ) {
	    if(list[i].address >= end)
	       break;             // They are in order
	    if(list[i].prio <= best_prio)
	       continue;
	    if(list[i].address > address && list[i].address < end)
	       end = list[i].address;
	 }

	 *byte_count = end-address;
      }

      return vobj;
   }

   bool vobj_list::contains_objects(uint32_t address, uint32_t length)
   {
      for(int i=0; i<this->length ; i++) {
         if(list[i].address+list[i].length <= address)
            continue;  // completely before our object

         if(list[i].address >= address+length)
            return false;   // completely after our object

         // So, we have overlap!
         return true;
      }
      return false;
   }

   
} // namespace emumiga
