/*
object_type_registry.cpp - Collection class for object_types

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

   object_type_registry::object_type_registry():
      entry_array(NULL), entry_length(0)
   {
      DEBUG(3) dprintf("object_type_registry::object_type_registry() called. this=%p\n", this);
   }
   
   object_type_registry::~object_type_registry()
   {
      DEBUG(3) dprintf("object_type_registry::~object_type_registry() called. this=%p\n", this);

      if(entry_array != NULL) {
         for(int i=0; i<entry_length; i++) {
            if(entry_array[i].object_type_array != NULL) {
               FreeVec(entry_array[i].object_type_array);
            }
         }
         FreeVec(entry_array);
      }
   }
   
   int object_type_registry::register_type(object_type *type)
   {
      DEBUG(3) dprintf("object_type_registry::register_type() called. this=%p, type=%p (%s)\n", this, type, type->get_name());

      if(type == NULL)
         return EINVAL;

      module *mod = type->get_module();
      int i;
      entry *ent = NULL;

      // Find the module
      for(i = 0 ; i < entry_length ; i++) {
	 if(entry_array[i].mod == mod) {
            ent = &entry_array[i];
	    break;
         }
      }

      if(ent == NULL) {
         // Nothing found, make a new module entry

         for(i = 0 ; i < entry_length ; i++) {
            if(entry_array[i].mod == NULL)
	    break;
         }
         if(i == entry_length) {
            int new_length = 2*entry_length ?: 20;
            entry *new_array = (entry *)AllocVec( new_length * sizeof(entry), MEMF_PUBLIC|MEMF_CLEAR);
            if(new_array == NULL)
               return ENOMEM;

            if(entry_array != NULL) {
               CopyMem(entry_array, new_array, entry_length * sizeof(entry));
               FreeVec(entry_array);
            }
            entry_array = new_array;
            entry_length = new_length;
         }
         entry_array[i].mod = mod;
         ent = &entry_array[i];
      }

      // Ok, module entry in ent

      // Add it, search for an empty place in object_type_array
      for(i = 0 ; i < ent->array_length ; i++) {
	 if(ent->object_type_array[i] == NULL)
	    break;
      }

      if(i == ent->array_length) {
	 int new_length = 2*ent->array_length ?: 20;
	 object_type **new_array = (object_type **)AllocVec( new_length * sizeof(object_type *), MEMF_PUBLIC|MEMF_CLEAR);
	 if(new_array == NULL)
	    return ENOMEM;

	 if(ent->object_type_array != NULL) {
	    CopyMem(ent->object_type_array, new_array, ent->array_length * sizeof(object_type *));
	    FreeVec(ent->object_type_array);
	 }
	 ent->object_type_array = new_array;
	 ent->array_length = new_length;
      }

      ent->object_type_array[i] = type;

      return 0;
   }

   int object_type_registry::unregister_type(object_type *type)
   {
      DEBUG(3) dprintf("object_type_registry::unregister_type() called. this=%p, type=%p (%s)\n", this, type, type->get_name());

      if(type == NULL)
         return EINVAL;

      module *mod = type->get_module();
      int i;
      entry *ent = NULL;

      // Find the module
      for(i = 0 ; i < entry_length ; i++) {
	 if(entry_array[i].mod == mod) {
            ent = &entry_array[i];
	    break;
         }
      }

      if(ent == NULL)
         return ENOENT;

      // Find the type
      for(i = 0 ; i < ent->array_length ; i++) {
         if(ent->object_type_array[i] != NULL) {
            if(type->equals(ent->object_type_array[i])) {
               // Found it
               ent->object_type_array[i]=NULL;
               return 0;
            }
         }
      }

      return ENOENT;
   }

   object_type *object_type_registry::find_by_module_and_id(module *mod, int id)
   {
      DEBUG(3) dprintf("object_type_registry::find_by_module_and_id() called. this=%p, mod=%p, id=%d\n", this, mod, id);

      int i;
      entry *ent = NULL;

      // Find the module entry
      for(i = 0 ; i < entry_length ; i++) {
	 if(entry_array[i].mod == mod) {
            ent = &entry_array[i];
	    break;
         }
      }

      if(ent == NULL)
         return NULL;

      // Find the type
      for(i = 0 ; i < ent->array_length ; i++) {
         if(ent->object_type_array[i] != NULL) {
            if(ent->object_type_array[i]->get_id() == id) {
               // Found it
               return ent->object_type_array[i];
            }
         }
      }

      return NULL;
   }

   object_type *object_type_registry::find_by_module_and_name(module *mod, char const *name)
   {
      DEBUG(3) dprintf("object_type_registry::find_by_module_and_id() called. this=%p, mod=%p, name=%s\n", this, mod, name);

      int i;
      entry *ent = NULL;

      // Find the module entry
      for(i = 0 ; i < entry_length ; i++) {
	 if(entry_array[i].mod == mod) {
            ent = &entry_array[i];
	    break;
         }
      }

      if(ent == NULL)
         return NULL;

      // Find the type
      for(i = 0 ; i < ent->array_length ; i++) {
         if(ent->object_type_array[i] != NULL) {
            if(strcmp(ent->object_type_array[i]->get_name(),name)==0) {
               // Found it
               return ent->object_type_array[i];
            }
         }
      }

      return NULL;
   }

} // namespace emumiga
