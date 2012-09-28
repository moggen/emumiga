/*
memory/mapping.h - Memory mapping base class, include file

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

#ifndef _MEMORY_MAPPING_H_
#define _MEMORY_MAPPING_H_

#include <stdint.h>

// Predecl for the includes
namespace emumiga {
namespace memory {
class mapping;
} // namespace memory
} // namespace emumiga

#include "mapping_constants.h"
#include "mapping_list.h"
#include "symbol.h"

namespace emumiga {
namespace memory {

// Predecl of root_mapping
class root_mapping;

// Memory mapping base class
class mapping {
   public:
	uint32_t vaddr_start;
	uint32_t vaddr_length;
	APTR     real_start;
	int      real_length;
	uint32_t type;
	bool	 is_from_vmem;
	mapping	 *supermapping;

	symbol_list *symbols;

	virtual int read(uint32_t offset, uint32_t size, uint8_t *data) = 0;
	virtual int write(uint32_t offset, uint32_t size, uint8_t *data) = 0;

	virtual int dispose(bool copyback);

        static int init();
        static void cleanup();

	static mapping *findEnclosingVAddr(uint32_t vaddr);
        static mapping *findEnclosingVAddrLimits(uint32_t vaddr, uint32_t *start, uint32_t *end);
	static mapping *findTypeByVAddr(uint32_t type, uint32_t vaddr);
	static mapping *findTypeByReal(uint32_t type, APTR real);

    protected:

	// Protected constructor and destructor
	mapping();
	~mapping();

	int addLive();
	void removeLive();

	// Standard memory functions for use by read() and write()
	// Implemented in mapping_functions.cpp
	int fn_r_binary(uint32_t offset, uint32_t size, uint8_t *data);
	int fn_w_binary(uint32_t offset, uint32_t size, uint8_t *data);
	int fn_protected(uint32_t offset, uint32_t size, uint8_t *data);
	int fn_unmapped(uint32_t offset, uint32_t size, uint8_t *data);

//	static mmu_pool_t *pool;

    private:

	bool is_live;

	static bool is_init;
	static mapping_list *mappings;
	static root_mapping *root_entry;

};

/*
// pool.c
mapping *mmu_allocObj(mmu_pool_t *pool, void *real);
mapping *mmu_allocObjExtra(mmu_pool_t *pool, void *real, int extraSize);
void mmu_freeObj(mapping *entry);
mapping *mmu_allocObjVAddr(mmu_pool_t *pool, void *real, uint32_t vaddr);
mapping *mmu_allocObjVAddrExtra(mmu_pool_t *pool, void *real, uint32_t vaddr, int extraSize);
void mmu_freeObjVAddr(mapping *entry);
mapping *mmu_allocObjSubStruct(mmu_pool_t *pool, void *real, uint32_t vaddr);
mapping *mmu_allocObjSubStructExtra(mmu_pool_t *pool, void *real, uint32_t vaddr, int extraSize);
void mmu_freeObjSubStruct(mapping *entry);
*/

} // namespace memory
} // namespace emumiga

// 'root_mapping' inherits from 'mapping' so the header must be included
// after the complete definition of 'mapping'
#include "root_mapping.h"

#endif
