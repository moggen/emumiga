/*
vmap_list.cpp - Virtual mapping list class

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

   vmap_list::vmap_list():
      list(NULL), capacity(0), length(0), current_dynamic(0x1000)
   {
   }

   vmap_list::~vmap_list()
   {
      if(list != NULL)
	 FreeVec(list);
   }

   int vmap_list::insert(virtual_mapping *vmap, uint32_t address)
   {
      uint32_t len = vmap->get_length();
      if(len == 0)
         return EINVAL;

      // Make sure we have place to insert
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

      if(length == 0) {
         // First entry!
         list[length].address = address;
         list[length].length = len;
         list[length].vmap = vmap;
         length++;
         return 0;
      }

      int i, j;

      // Iterate and find our place
      for(i=0 ; i<length ; i++) {
         if(address <= list[i].address) {
            // Ok, the list entry is beyond our entry
            if(i>0) {
               // Check previous so it does not overlap us
               if(list[i-1].address + list[i-1].length > address) {
                  return EADDRINUSE;
               }
            }
            // Check so we do not overlap the list entry
            if(address + len > list[i].address) {
               return EADDRINUSE;
            }

            // All ok
            break;
         }
      }

      if(i == length) {
         // Ok we are last, check the currently last so it does not overlap us
         if(list[i-1].address + list[i-1].length > address) {
            return EADDRINUSE;
         }
      }

      // Insert us before the current list[i]

      // Move the rest of the list up one step
      for(j=length ; j>i ; j--) {
         list[j].address = list[j-1].address;
         list[j].length  = list[j-1].length;
         list[j].vmap    = list[j-1].vmap;
      }

      list[i].address = address;
      list[i].length = len;
      list[i].vmap = vmap;

      length++;

      return 0;
   }

   uint32_t vmap_list::add(virtual_mapping *vmap)
   {
      if(vmap == NULL)
	 return 0xFFFFFFFF;

      // Don't add an already added mapping
      if(vmap->get_address() != 0xFFFFFFFF)
	 return 0xFFFFFFFF;

      uint32_t len = vmap->get_length();

      // Naïve approach, don't reuse addresses
      uint32_t new_address = current_dynamic;

      int retc = insert(vmap, new_address);

      if(retc) {
         return 0xFFFFFFFF;
      }

      current_dynamic += len;

      // Alignment padding to 4 bytes
      current_dynamic = (current_dynamic + 3) & ~3;

      vmap->set_address(new_address);

      return new_address;
   }

   uint32_t vmap_list::add_absolute(virtual_mapping *vmap, uint32_t address)
   {
      if(vmap == NULL)
	 return 0xFFFFFFFF;

      // Don't add an already added mapping
      if(vmap->get_address() != 0xFFFFFFFF)
	 return 0xFFFFFFFF;

      int retc = insert(vmap, address);

      if(retc) {
         return 0xFFFFFFFF;
      }

      vmap->set_address(address);

      return address;
   }

   int vmap_list::rem(virtual_mapping *vmap)
   {
      if(vmap == NULL)
	 return ENOENT;

      int i, j;
      for(i=0; i<length ; i++) {
	 if(list[i].vmap == vmap) {
            // Remove us by moving down the rest
            for(j=i+1 ; j<length; j++) {
               list[j-1].address = list[j].address;
               list[j-1].length = list[j].length;
               list[j-1].vmap = list[j].vmap;
            }
            length--;
            vmap->set_address(0xFFFFFFFF);
	    return 0;
	 }
      }

      // Not found
      return ENOENT;
   }

   virtual_mapping *vmap_list::find(uint32_t address)
   {
      for(int i=0; i<length ; i++) {

	 if(list[i].address == address)
	    return list[i].vmap;

	 if(list[i].address > address)
	    return NULL;             // They are in order
      }
      return NULL;
   }

   virtual_mapping *vmap_list::find_containing(uint32_t address)
   {
      for(int i=0; i<length ; i++) {
	 if(list[i].address > address)
	    return NULL;             // They are in order

	 if( list[i].address <= address && address < (list[i].address + list[i].length) )
	    return list[i].vmap;

      }
      return NULL;
   }

	 
} // namespace emumiga
