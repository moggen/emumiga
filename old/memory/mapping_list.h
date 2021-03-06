/*
memory/mapping_list.h - Skip-list collection for mappings, include file

Copyright (C) 2010  Magnus �berg

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

#ifndef _MEMORY_MAPPING_LIST_H_
#define _MEMORY_MAPPING_LIST_H_

#include <stdint.h>
#include <exec/types.h>

// Predecl for includes
namespace emumiga {
namespace memory {
class mapping_list;
} // namespace memory
} // namespace emumiga

#include "mapping.h"

namespace emumiga {
namespace memory {

#define MMU_SLIST_SIZE 10
#define MMU_SLIST_THRESHOLD 8
#define MMU_SLIST_SKIP 4

class mapping_list {
    public:

	mapping *findEnclosingVAddr(uint32_t vaddr);
	mapping *findEnclosingVAddrLimits(uint32_t vaddr, uint32_t *start, uint32_t *end);
	mapping *findTypeByVAddr(uint32_t type, uint32_t vaddr);
	mapping *findTypeByReal(uint32_t type, APTR real);

	int add(mapping *entry);
	int remove(mapping *entry);

    protected:

	class block {
	    public:
		block *prev, *next, *snext;
		int count;
		uint32_t vaddr[MMU_SLIST_SIZE];
		APTR real[MMU_SLIST_SIZE];
		mapping *entry[MMU_SLIST_SIZE];

		block();
	};

	class iterator {
	    public:
		iterator(block *_block, int _index);

		mapping *current();
		int moveNext();
		int movePrev();

	    protected:
		block *bl;
		int index;
	};

	block root_vaddr[MMU_PRIO_LEVELS];
	block root_real[MMU_PRIO_LEVELS];

	block *best_block_vaddr(block *root, uint32_t vaddr);
	block *best_block_real(block *root, APTR real);
	mapping *find_entry_prio(uint32_t vaddr, int prio);
	static void reindex(block *block);
	bool validate(mapping *entry);
};

} // namespace memory
} // namespace emumiga


#endif
