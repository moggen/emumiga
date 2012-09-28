/*
vmem.c - Virtual memory support functions

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

#include <proto/exec.h>
#include "../debug.h"
#include "memory.h"
#include "vmem.h"

namespace emumiga {
namespace memory {
namespace vmem {

vmem_mapping::vmem_mapping(int size)
{
	vaddr_start = alloc_nomap(size);
	vaddr_length = size;
	type = MMU_TYPE_VALLOCMEM;

	native_start = (uint8_t *)AllocVec(size,MEMF_PUBLIC|MEMF_CLEAR);
	if(native_start == NULL){
		DEBUG(1) dprintf("Warning: vmem_mapping::vmem_mapping(): AllocVec() failed. Size: 0x%x\n",(unsigned int)size);
	} else {
		addLive();
	}
}

int vmem_mapping::read(uint32_t offset, uint32_t size, uint8_t *data)
{
	return fn_r_binary(offset, size, *data);
}

int vmem_mapping::write(uint32_t offset, uint32_t size, uint8_t *data)
{
	return fn_w_binary(offset, size, *data);
}
int vmem_mapping::dispose(bool copyback)
{
	
}

// Size of reserved memory region starting at address 0
static uint32_t nextFreeMem = 4096;

uint32_t alloc_nomap(uint32_t size)
{
	uint32_t vaddr;

	DEBUG(5) dprintf("vmem: alloc_nomap() called\n");
	DEBUG(5) dprintf("vmem: alloc_nomap: size: 0x%x\n",(unsigned int)size);

	vaddr = nextFreeMem;
	nextFreeMem += size;

	// Make sure the next block is aligned by 4 bytes
	if(nextFreeMem & 3)
		nextFreeMem = (nextFreeMem + 3) & 0xfffffffc;

	DEBUG(5) dprintf("vmem: alloc_nomap: vaddr: 0x%x\n",(unsigned int)vaddr);

	return vaddr;
}

void free_nomap(uint32_t vaddr)
{
	DEBUG(5) dprintf("vmem: free_nomap() called\n");
	DEBUG(5) dprintf("vmem: free_nomap: vaddr: 0x%x\n",(unsigned int)vaddr);
}

uint32_t alloc(uint32_t size, char const *name)
{
	uint32_t vaddr;
	vmem_mapping *entry;

	DEBUG(5) dprintf("vmem: alloc() called\n");
	DEBUG(5) dprintf("vmem: alloc: size: 0x%x, name: %s\n",(unsigned int)size, name);

	entry = new vmem_mapping(size);
	vaddr = entry->vaddr_start;
	DEBUG(3) {
		if(name != NULL) {
			entry->symbols->add(new symbol(vaddr, name));
		}
	}

	DEBUG(5) dprintf("vmem: alloc: vaddr: 0x%x\n",(unsigned int)vaddr);

	return vaddr;
}

void free(uint32_t vaddr)
{
	mapping *entry;
	void *mem;

	DEBUG(5) dprintf("vmem: free() called\n");
	DEBUG(5) dprintf("vmem: free: vaddr: 0x%x\n",(unsigned int)vaddr);

	entry = mmu_findEntryWithType(vaddr, MMU_TYPE_VALLOCMEM);
	if(!entry) {
		DEBUG(1) dprintf("Warning: vmem: free: No entry found at 0x%x\n",(unsigned int)vaddr);
		return;
	}
	if(entry->startAddr != vaddr) {
		DEBUG(1) dprintf("Warning: vmem: free: Address is not the start of the entry. VAddr: 0x%x, Entry start: 0x%x\n",(unsigned int)vaddr, (unsigned int)entry->startAddr);
	}
	mem = entry->nativePtr;
	mmu_delEntry(entry);
	FreeVec(mem);
}

uint32_t alloc_rom(uint8_t const *addr, uint32_t size, char const *name)
{
	uint32_t vaddr;
	mapping *entry;

	DEBUG(5) dprintf("vmem: alloc_rom() called\n");
	DEBUG(5) dprintf("vmem: alloc_rom: size: 0x%x, name: %s\n",(unsigned int)size, name);

	vaddr = alloc_nomap(size);

	entry = mmu_addEntry(vaddr, size, MMU_TYPE_VALLOCMEM_ROM, NULL);
	entry->read = mmu_fn_r_binary;
	entry->write = mmu_fn_protected;
	entry->nativePtr = (uint8_t *)addr;

	DEBUG(3) {
		if(name != NULL) {
			mmu_addSymbol(entry, vaddr, name);
		}
	}

	DEBUG(5) dprintf("vmem: alloc_rom: vaddr: 0x%x\n",(unsigned int)vaddr);

	return vaddr;
}

void free_rom(uint32_t vaddr)
{
	mapping *entry;
	void *mem;

	DEBUG(5) dprintf("vmem: free_rom() called\n");
	DEBUG(5) dprintf("vmem: free_rom: vaddr: 0x%x\n",(unsigned int)vaddr);

	entry = mmu_findEntryWithType(vaddr, MMU_TYPE_VALLOCMEM_ROM);
	if(!entry) {
		DEBUG(1) dprintf("Warning: vmem: free_rom: No entry found at 0x%x\n",(unsigned int)vaddr);
		return;
	}
	if(entry->startAddr != vaddr) {
		DEBUG(1) dprintf("Warning: vmem: free_rom: Address is not the start of the entry. VAddr: 0x%x, Entry start: 0x%x\n",(unsigned int)vaddr, (unsigned int)entry->startAddr);
	}
	mem = entry->nativePtr;
	mmu_delEntry(entry);
}

uint32_t alloc_ram(uint8_t *addr, uint32_t size, char const *name)
{
	uint32_t vaddr;
	mapping *entry;

	DEBUG(5) dprintf("vmem: alloc_ram() called\n");
	DEBUG(5) dprintf("vmem: alloc_ram: size: 0x%x, name: %s\n",(unsigned int)size, name);

	vaddr = alloc_nomap(size);

	entry = mmu_addEntry(vaddr, size, MMU_TYPE_VALLOCMEM_RAM, NULL);
	entry->read = mmu_fn_r_binary;
	entry->write = mmu_fn_w_binary;
	entry->nativePtr = (uint8_t *)addr;

	DEBUG(3) {
		if(name != NULL) {
			mmu_addSymbol(entry, vaddr, name);
		}
	}

	DEBUG(5) dprintf("vmem: alloc_ram: vaddr: 0x%x\n",(unsigned int)vaddr);

	return vaddr;
}

void vfreemem_ram(uint32_t vaddr)
{
	mapping *entry;
	void *mem;

	DEBUG(5) dprintf("mem: free_ram() called\n");
	DEBUG(5) dprintf("mem: free_ram: vaddr: 0x%x\n",(unsigned int)vaddr);

	entry = mmu_findEntryWithType(vaddr, MMU_TYPE_VALLOCMEM_RAM);
	if(!entry) {
		DEBUG(1) dprintf("Warning: vmem: free_ram: No entry found at 0x%x\n",(unsigned int)vaddr);
		return;
	}
	if(entry->startAddr != vaddr) {
		DEBUG(1) dprintf("Warning: vmem: free_ram: Address is not the start of the entry. VAddr: 0x%x, Entry start: 0x%x\n",(unsigned int)vaddr, (unsigned int)entry->startAddr);
	}
	mem = entry->nativePtr;
	mmu_delEntry(entry);
}

uint8_t *vptr(uint32_t addr)
{
	mapping *entry;

	DEBUG(5) dprintf("vmem: vptr() called\n");

	entry = mmu_findEntry(addr);
	if(entry == NULL)
		return NULL;

	return (uint8_t *)( (uint8_t *)entry->nativePtr + (addr - entry->startAddr) );
}


int read8(uint32_t addr, uint8_t *dptr)
{
	return readData(addr, 1, dptr);
}

int write8(uint8_t data, uint32_t addr)
{
	return writeData(addr, 1, &data);
}

int read16align(uint32_t addr, uint16_t *dptr)
{
	int retc;
	uint8_t workp[2];

	retc = readData(addr & 0xfffffffe, 2, workp);
	*dptr = ((uint16_t)workp[0] << 8) |
		((uint16_t)workp[1]);
	return retc;
}

int write16align(uint16_t data, uint32_t addr)
{
	uint8_t workp[2];

	workp[0] = (uint8_t)((data >> 8) & 0xff);
	workp[1] = (uint8_t)((data) & 0xff);
	return writeData(addr & 0xfffffffe, 2, workp);
}

int read32align(uint32_t addr, uint32_t *dptr)
{
	int retc;
	uint8_t workp[4];

	retc = readData(addr & 0xfffffffe, 4, workp);
	*dptr =	((uint32_t)workp[0] << 24) | 
	 	((uint32_t)workp[1] << 16) | 
	 	((uint32_t)workp[2] << 8) | 
		((uint32_t)workp[3]);
	return retc;
}

int write32align(uint32_t data, uint32_t addr)
{
	uint8_t workp[4];

	workp[0] = (uint8_t)((data >> 24) & 0xff);
	workp[1] = (uint8_t)((data >> 16) & 0xff);
	workp[2] = (uint8_t)((data >> 8) & 0xff);
	workp[3] = (uint8_t)((data) & 0xff);
	return writeData(addr & 0xfffffffe, 4, workp);
}


int read8_cached(uint32_t addr, uint8_t *dptr, cache *cache)
{
	return readData_cached(addr, 1, dptr, cache);
}

int write8_cached(uint8_t data, uint32_t addr, cache *cache)
{
	return writeData_cached(addr, 1, &data, cache);
}

int read16align_cached(uint32_t addr, uint16_t *dptr, cache *cache)
{
	int retc;
	uint8_t workp[2];

	retc = readData_cached(addr & 0xfffffffe, 2, workp, cache);
	*dptr = ((uint16_t)workp[0] << 8) |
		((uint16_t)workp[1]);
	return retc;
}

int write16align_cached(uint16_t data, uint32_t addr, cache *cache)
{
	uint8_t workp[2];

	workp[0] = (uint8_t)((data >> 8) & 0xff);
	workp[1] = (uint8_t)((data) & 0xff);
	return writeData_cached(addr & 0xfffffffe, 2, workp, cache);
}

int read32align_cached(uint32_t addr, uint32_t *dptr, cache *cache)
{
	int retc;
	uint8_t workp[4];

	retc = readData_cached(addr & 0xfffffffe, 4, workp, cache);
	*dptr =	((uint32_t)workp[0] << 24) | 
	 	((uint32_t)workp[1] << 16) | 
	 	((uint32_t)workp[2] << 8) | 
		((uint32_t)workp[3]);
	return retc;
}

int write32align_cached(uint32_t data, uint32_t addr, cache *cache)
{
	uint8_t workp[4];

	workp[0] = (uint8_t)((data >> 24) & 0xff);
	workp[1] = (uint8_t)((data >> 16) & 0xff);
	workp[2] = (uint8_t)((data >> 8) & 0xff);
	workp[3] = (uint8_t)((data) & 0xff);
	return writeData_cached(addr & 0xfffffffe, 4, workp, cache);
}

} // namespace vmem
} // namespace memory
} // namespace emumiga
