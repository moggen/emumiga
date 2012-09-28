/*
vmem.h - Virtual memory support include file

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

#ifndef _MEMORY_VMEM_H_
#define _MEMORY_VMEM_H_

#include <stdint.h>
#include "cache.h"

namespace emumiga {
namespace memory {
namespace vmem {

// Virtual mem mapping class
class vmem_mapping : public mapping {
   public:
        vmem_mapping(int size);

	virtual int read(uint32_t offset, uint32_t size, uint8_t *data);
	virtual int write(uint32_t offset, uint32_t size, uint8_t *data);

	virtual int dispose(bool copyback);

   protected:
        ~vmem_mapping();
};

// Virtual mem mapping class for RAM
class vmem_ram : public mapping {
   public:
        vmem_ram(int size, uint8_t *addr);

	virtual int read(uint32_t offset, uint32_t size, uint8_t *data);
	virtual int write(uint32_t offset, uint32_t size, uint8_t *data);

	virtual int dispose(bool copyback);

   protected:
        ~vmem_ram();
};

// Virtual mem mapping class for ROM
class vmem_rom : public mapping {
   public:
        vmem_rom(int size, uint8_t const *addr);

	virtual int read(uint32_t offset, uint32_t size, uint8_t *data);

	virtual int dispose(bool copyback);

   protected:
        ~vmem_rom();
};

// Memory management functions
uint32_t alloc_nomap(uint32_t size);
void free_nomap(uint32_t addr);
uint32_t alloc(uint32_t size, char const *name);
void free(uint32_t addr);
uint32_t alloc_rom(uint8_t const *addr, uint32_t size, char const *name);
void free_rom(uint32_t addr);
uint32_t alloc_ram(uint8_t *addr, uint32_t size, char const *name);
void free_ram(uint32_t addr);
uint8_t *vptr(uint32_t addr);

// Memory access functions
int read8(uint32_t addr, uint8_t *dptr);
int write8(uint8_t data, uint32_t addr);
int read16align(uint32_t addr, uint16_t *dptr);
int write16align(uint16_t data, uint32_t addr);
int read32align(uint32_t addr, uint32_t *dptr);
int write32align(uint32_t data, uint32_t addr);

int read8_cached(uint32_t addr, uint8_t *dptr, cache *cache);
int write8_cached(uint8_t data, uint32_t addr, cache *cache);
int read16align_cached(uint32_t addr, uint16_t *dptr, cache *cache);
int write16align_cached(uint16_t data, uint32_t addr, cache *cache);
int read32align_cached(uint32_t addr, uint32_t *dptr, cache *cache);
int write32align_cached(uint32_t data, uint32_t addr, cache *cache);

// Memory access macros, usable within namespace emumiga
#define READMEM_8(x,v) memory::vmem::read8(x,v)
#define WRITEMEM_8(v,x) memory::vmem::write8(v,x)
#define READMEM_16(x,v) memory::vmem::read16align(x,v)
#define WRITEMEM_16(v,x) memory::vmem::write16align(v,x)
#define READMEM_32(x,v) memory::vmem::read32align(x,v)
#define WRITEMEM_32(v,x) memory::vmem::write32align(v,x)

#define READMEM_8_I(x,v) memory::vmem::read8_cached(x,v,cpu->i_cache)
#define WRITEMEM_8_I(v,x) memory::vmem::write8_cached(v,x,cpu->i_cache)
#define READMEM_16_I(x,v) memory::vmem::read16align_cached(x,v,cpu->i_cache)
#define WRITEMEM_16_I(v,x) memory::vmem::write16align_cached(v,x,cpu->i_cache)
#define READMEM_32_I(x,v) memory::vmem::read32align_cached(x,v,cpu->i_cache)
#define WRITEMEM_32_I(v,x) memory::vmem::write32align_cached(v,x,cpu->i_cache)

#define READMEM_8_D(x,v) memory::vmem::read8_cached(x,v,cpu->d_cache)
#define WRITEMEM_8_D(v,x) memory::vmem::write8_cached(v,x,cpu->d_cache)
#define READMEM_16_D(x,v) memory::vmem::read16align_cached(x,v,cpu->d_cache)
#define WRITEMEM_16_D(v,x) memory::vmem::write16align_cached(v,x,cpu->d_cache)
#define READMEM_32_D(x,v) memory::vmem::read32align_cached(x,v,cpu->d_cache)
#define WRITEMEM_32_D(v,x) memory::vmem::write32align_cached(v,x,cpu->d_cache)

} // namespace vmem
} // namespace memory
} // namespace emumiga

#endif
