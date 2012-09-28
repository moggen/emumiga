
/*
emumiga.h - emumiga main header file

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

#ifndef EMUMIGA_H
#define EMUMIGA_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <exec/memory.h>
#include <aros/symbolsets.h>

// Predecls
namespace emumiga
{
  class context;
  class context_collection;
  class emulator;
  class message;
  class module;
  class module_registry;
  class internal_module;
  class scheduler;
  class symbol;
  class vmap_list;
  class virtual_space;
  class vobj_list;
  class virtual_mapping;
  class virtual_object;
  class robj_list;
  class real_space;
  class user_mapping;
  class sys_mapping;
  class rom_mapping;
  class library_mapping;
  class object_type;
  class object_type_registry;

  namespace exec
  {
     class main;
  }
  namespace dos
  {
     class main;
  }
}

#include "cpu68k.h"
#include "context.h"
#include "context_collection.h"
#include "scheduler.h"
#include "module_registry.h"
#include "vmap_list.h"
#include "virtual_space.h"
#include "robj_list.h"
#include "real_space.h"
#include "emulator.h"
#include "message.h"
#include "log.h"
#include "module.h"
#include "internal_module.h"
#include "symbol.h"
#include "vobj_list.h"
#include "virtual_mapping.h"
#include "virtual_object.h"
#include "user_mapping.h"
#include "sys_mapping.h"
#include "rom_mapping.h"
#include "library_mapping.h"
#include "utility.h"
#include "object_type.h"
#include "object_type_registry.h"

#endif // EMUMIGA_H
