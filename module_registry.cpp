/*
module_registry.cpp - Collection class for modules

Copyright (C) 2011, 2012  Magnus Öberg

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
#include "modules/dos/dos.h"

namespace emumiga {

   module_registry::module_registry():
      module_array(NULL), array_length(0)
   {
      DEBUG(3) dprintf("module_registry::module_registry() called. this=%p\n", this);
   }
   
   module_registry::~module_registry()
   {
      DEBUG(3) dprintf("module_registry::~module_registry() called. this=%p\n", this);

      if(module_array != NULL)
	 FreeVec(module_array);
   }
   
   int module_registry::registerModule(module *mymodule, const char *name)
   {
      DEBUG(3) dprintf("module_registry::registerModule() called. this=%p, mymodule=%p, name=%s\n", this, mymodule, name);

      if(mymodule == NULL)
         return 0;

      int i;
      for(i = 0 ; i < array_length ; i++) {
	 if(module_array[i].mod == NULL)
	    break;
      }

      if(i == array_length) {
	 int new_length = 2*array_length ?: 20;
	 module_entry *new_array = (module_entry *)AllocVec( new_length * sizeof(module_entry), MEMF_PUBLIC|MEMF_CLEAR);
	 if(new_array == NULL)
	    return 0;

	 if(module_array != NULL) {
	    CopyMem(module_array, new_array, array_length * sizeof(module_entry));
	    FreeVec(module_array);
	 }
	 module_array = new_array;
	 array_length = new_length;
      }

      char *new_name = NULL;
      if(name != NULL) {
         new_name = (char *)AllocVec( strlen(name)+1, MEMF_PUBLIC|MEMF_CLEAR);
         if(new_name == NULL)
            return 0;

         strcpy(new_name, name);
      }
              
      module_array[i].mod = mymodule;
      module_array[i].name = new_name;

      mymodule->setID(i+1);

      return i+1;
   }

   int module_registry::unRegisterModule(module *mymodule)
   {
      DEBUG(3) dprintf("module_registry::unRegisterModule() called. this=%p, mymodule=%p\n", this, mymodule);

      if(mymodule == NULL)
         return 0;

      for(int i = 0 ; i < array_length ; i++) {
	 if(module_array[i].mod == mymodule) {
	    module_array[i].mod = NULL;
            if(module_array[i].name != NULL) {
               FreeVec(module_array[i].name);
               module_array[i].name = NULL;
            }
	    return 0;
	 }
      }

      mymodule->setID(0);

      return ENOENT;
   }

   module *module_registry::findByID(int id)
   {
//      DEBUG(3) dprintf("module_registry::findByID() called. this=%p, id=%d\n", this, id);

      id--;

      if(id < 0 || id >= array_length)
	 return NULL;

      return module_array[id].mod;
   }

   module *module_registry::get_module(const char *name)
   {
      DEBUG(3) dprintf("module_registry::get_module() called. this=%p, name=%s\n", this, name);

      if(name == NULL)
         return NULL;

      // Find it in the registry
      for(int i = 0 ; i < array_length ; i++) {
	 if(module_array[i].mod != NULL && module_array[i].name != NULL) {
            if(strcmp(module_array[i].name, name) == 0) {
               return module_array[i].mod;
            }
         }
      }

      // TODO: Add logic for dynamic loading here

      return NULL;
   }

} // namespace emumiga
