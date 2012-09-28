/*
memory/mapping.c - Memory mapping functions

Copyright (C) 2007, 2008, 2009, 2010  Magnus Öberg

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

#include "../debug.h"
#include "mapping.h"
#include "root_mapping.h"

//#include "slist.h"
//#include "cache.h"

namespace emumiga {
namespace memory {

/*
#define MMU_TABLESIZE 1000

struct mmu_table_struct {
        mapping entry[MMU_TABLESIZE];
//      struct mmu_table_struct *next;
};
typedef struct mmu_table_struct mmu_table_t;

static mmu_table_t *mmu_table;
static mapping *mmu_rootEntry;
*/

// Init static members
bool mapping::is_init = 0;
mapping_list *mapping::mappings = NULL;
root_mapping *mapping::root_entry = NULL;

int mapping::init()
{
	DEBUG(3) dprintf("mapping: init() called\n");

	mappings = new mapping_list();

	root_entry = new root_mapping();

	mappings->add(root_entry);

//	mmu_addSymbol(mmu_rootEntry, 0, "$$_MMU_Root_$$");

	is_init = 1;

	return 0;
}

void mapping::cleanup()
{

	DEBUG(3) dprintf("mapping: cleanup() called\n");

	if( !is_init )
		return;

	mappings->remove(root_entry);

	delete root_entry;

	delete mappings;

	is_init = 0;
}

mapping::mapping()
{
	vaddr_start = 0;
	vaddr_length = 0;
	real_start = NULL;
	real_length = 0;

	type = MMU_TYPE_UNINITIALIZED;

	is_from_vmem = 0;
	supermapping = NULL;

	symbols = new symbol_list();

	is_live = 0;
}

mapping::~mapping()
{
	delete symbols;
}

int mapping::dispose(bool copyback __attribute__((unused)))
{
	if(supermapping)
		return supermapping->dispose();

	removeLive();
	delete this;
	return 0;
}

int mapping::read(uint32_t offset, uint32_t size, uint8_t *data)
{
	return fn_unmapped(offset, size, data);
}

int mapping::write(uint32_t offset, uint32_t size, uint8_t *data)
{
	return fn_unmapped(offset, size, data);
}

int mapping::addLive()
{
	if(is_live) return 1;

	if(!mappings->add(this)) return 1;

	is_live = 1;

	return 0;
}

void mapping::removeLive()
{
	if(!is_live) return;

	mappings->remove(this);

	is_live = 0;

/*
	// Invalidate all caches
	mmu_cache_up_serial();
*/
}

mapping *mapping::findEnclosingVAddr(uint32_t vaddr)
{
	return mappings->findEnclosingVAddr(vaddr);
}

mapping *mapping::findEnclosingVAddrLimits(uint32_t vaddr, uint32_t *start, uint32_t *end)
{
	return mappings->findEnclosingVAddrLimits(vaddr, start, end);
}

mapping *mapping::findTypeByVAddr(uint32_t type, uint32_t vaddr)
{
	return mappings->findTypeByVAddr(type, vaddr);
}

mapping *mapping::findTypeByReal(uint32_t type, APTR real)
{
	return mappings->findTypeByReal(type, real);
}

} // namespace memory
} // namespace emumiga
