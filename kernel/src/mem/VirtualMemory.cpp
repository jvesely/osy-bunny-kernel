/*
 *          _     _
 *          \`\ /`/
 *           \ V /
 *           /. .\            Bunny Kernel for MIPS
 *          =\ T /=
 *           / ^ \
 *        {}/\\ //\
 *        __\ " " /__
 *   jgs (____/^\____)
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
/*! 	 
 *   @author Matus Dekanek, Tomas Petrusek, Lubos Slovak, Jan Vesely
 *   @par "SVN Repository"
 *   svn://aiya.ms.mff.cuni.cz/osy0809-depeslve
 *   
 *   @version $Id$
 *   @note
 *   Semestral work for Operating Systems course at MFF UK \n
 *   http://dsrg.mff.cuni.cz/~ceres/sch/osy/main.php
 *   
 *   @date 2008-2009
 */

/*!
 * @file 
 * @brief Short description.
 *
 * Long description. I would paste some Loren Ipsum rubbish here, but I'm afraid
 * It would stay that way. Not that this comment is by any means ingenious but 
 * at least people can understand it. 
 */

#include "VirtualMemory.h"

#include "flags.h"
#include "Memory.h"

// Make dump() function quiet.
//#define VMA_NDEBUG

//#define VMA_DEBUG
//#define VMA_TLB_DEBUG

#ifndef VMA_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...) \
  printf("[ VMA DEBUG ]: "); \
  printf(ARGS);
#endif

#ifndef VMA_TLB_DEBUG
#define PRINT_TLB_DEBUG(...)
#else
#define PRINT_TLB_DEBUG(ARGS...) \
  printf("[ VMA TLB DEBUG ]: "); \
  printf(ARGS);
#endif

using namespace Processor;

int VirtualMemory::allocate(void** from, size_t size, unsigned int flags)
{
	// the global minimal allowed page size (only for alignment checks)
	const PageSize frameType = PAGE_MIN;

	// check if size is aligned and not zero
	if (!Memory::isAligned(size, frameType) || (size == 0)) {
		PRINT_DEBUG("Size %u is 0 or is not aligned to frame size %x.\n",
			size, Memory::frameSize(frameType));
		return EINVAL;
	}

	// check if in the given size is possible in the given segment (for area1 test)
	if (!Memory::checkSizeInSegment(size, VF_ADDR_TYPE(flags))) {
		PRINT_DEBUG("Segment %u is smaller than requested size %x.\n",
			VF_ADDR_TYPE(flags), size);
		return ENOMEM;
	}

	if (VF_VIRT_ADDR(flags) == VF_VA_AUTO) {
		// automatically assign the virtual address

		// get the right address in segment (if KSEG0/1 from will be calculated later)
		*from = Memory::getAddressInSegment(VF_ADDR_TYPE(flags));

		if (m_virtualMemoryMap.count() != 0) {
			// get a free address aligned to the biggest usable page size
			getFreeAddress(*from, size, Memory::getBiggestPage(size));
		}

		PRINT_DEBUG("Address %p choosen for the new block of size %x.\n", *from, size);
	} else {
		// VF_VA_USER means the from is important for virtual address

		// check if address is aligned
		if (!Memory::isAligned(*from, frameType)) {
			PRINT_DEBUG("Address %p is not aligned to frame size %x.\n",
				*from, Memory::frameSize(frameType));
			return EINVAL;
		}
		// check if from/size is free
		if (!isFree(*from, size)) {
			PRINT_DEBUG("Address %p with size %x (or its part) is not free.\n", *from, size);
			return EINVAL;
		}

		// calculate the missing or wrong segment (the address contains the information)
		flags &= ~VF_AT_MASK;
		flags |= (Memory::getSegment(*from) << VF_AT_SHIFT);
	}

	// check if the whole virtual block is in the right segment (KSEG0/1 will be checked later)
	if ((Memory::getSegment(*from) != VF_ADDR_TYPE(flags)) || !Memory::checkSegment(*from, size)) {
		if (VF_VIRT_ADDR(flags) == VF_VA_AUTO) {
			PRINT_DEBUG("Address %p with size %x is overflowing segment %u.\n",
				*from, size, VF_ADDR_TYPE(flags));
			return ENOMEM;
		} else {
			PRINT_DEBUG("Address %p with size %x is overflowing segment %u.\n",
				*from, size, VF_ADDR_TYPE(flags));
			return EINVAL;
		}
	}

	// VF_AT_KSEG0 and VF_AT_KSEG1 means VF_VA_USER is important also for frame allocator
	if ((VF_VIRT_ADDR(flags) == VF_VA_USER) && !VF_SEG_NOTLB(VF_ADDR_TYPE(flags))) {
		// other segments are trough TLB, so clear the VF_VA_USER flag
		flags &= ~VF_VA_MASK;
		flags |= (VF_VA_AUTO << VF_VA_SHIFT);
	}

	// create VM Area
	VirtualMemoryArea vma(*from, size);

	PRINT_DEBUG("Allocating physical memory for VMA at %p with size %x.\n", *from, size);
	// allocate the physical memory trough VMA
	int res = vma.allocate(flags);

	if (res == ENOMEM) {
		PRINT_DEBUG("Not enough memory for %p with size %x, freeing already allocated parts.\n",
			*from, size);
		// on error free the allocated memory and return
		vma.free();
		return ENOMEM;
	}

	PRINT_DEBUG("Adding VMA at %p with size %x to the virtual memory map (%u).\n",
		*from, size, m_virtualMemoryMap.count());
	// add vma to the virtual memory map
	m_virtualMemoryMap.insert(vma);

	return EOK;
}

/* --------------------------------------------------------------------- */

int VirtualMemory::free(const void* from)
{
	// search for the address and get the VMA
	const VirtualMemoryMapEntry* entry = m_virtualMemoryMap.findItem(
		VirtualMemoryMapEntry(VirtualMemoryArea(from)));

	// if it doesn't exists or doesn't begin on the given address - fail
	if ((entry == NULL) || (entry->data().address() != from)) {
		PRINT_DEBUG("No VMA starts at %p.\n", from);
		return EINVAL;
	}

	// free all subareas
	const_cast<VirtualMemoryArea&>(entry->data()).free();

	// delete the entry from the tree
	delete entry;

	// clear the TLB
	freed();

	return EOK;
}

/* --------------------------------------------------------------------- */

int VirtualMemory::resize(const void* from, const size_t size)
{
	// check if size is aligned and not zero
	if (!Memory::isAligned(size, PAGE_MIN) || (size == 0)) {
		PRINT_DEBUG("Size %x is zero or is not aligned.\n", size);
		return EINVAL;
	}

	// search for the address and get the VMA
	const VirtualMemoryMapEntry* entry =
		m_virtualMemoryMap.findItem(VirtualMemoryArea(from));

	// if it doesn't exists or doesn't begin on the given address - fail
	if ((entry == NULL) || (entry->data().address() != from)) {
		PRINT_DEBUG("No VMA starts at %p.\n", from);
		return EINVAL;
	}

	// actual size of the selected VMA
	const size_t actualSize = const_cast<VirtualMemoryArea&>(entry->data()).size();

	if (size > actualSize) {
		// if enlarging, check if the VMA stays in one segment
		if (!Memory::checkSegment(from, size)) {
			PRINT_DEBUG("Area %p (%x) would overflow to another segment after resizing to %x.\n",
				from, actualSize, size);
			return EINVAL;
		}

		// if enlarging, check if there is enough space behind the VMA
		if (!isFree((void *)((size_t)from + actualSize), size - actualSize)) {
			PRINT_DEBUG("Not enough free space after %p to resize from %x to %x.\n",
				from, actualSize, size);
			return EINVAL;
		}
	}

	// call resize on the specific VM Area
	int result = const_cast<VirtualMemoryArea&>(entry->data()).resize(size);

	// clear the TLB
	freed();

	return result;
}

/* --------------------------------------------------------------------- */

int VirtualMemory::remap(const void* from, const void* to)
{
	if (from == to) return EOK;

	// check if address is aligned
	if (!Memory::isAligned(to, PAGE_MIN)) {
		PRINT_DEBUG("Address %p is not aligned.\n", to);
		return EINVAL;
	}

	// search for the address and get the VMA
	VirtualMemoryMapEntry* entry =
		m_virtualMemoryMap.findItem(VirtualMemoryArea(from));

	// if it doesn't exists or doesn't begin on the given address - fail
	if ((entry == NULL) || (entry->data().address() != from)) {
		PRINT_DEBUG("No VMA starts at %p.\n", from);
		return EINVAL;
	}

	// size of the selected VMA
	const size_t size = entry->data().size();

	//TODO !!! is it allowed to have to in a different segment???
	//TODO !!! maybe at least check if to is not in KSEG0/1 (no TLB)
	// check if from and to are in the same segment
	//if (Memory::getSegment(from) != Memory::getSegment(to)) {
	//	PRINT_DEBUG("Address %p (from) is in different segment than %p (to).\n",
	//		from, to);
	//	return EINVAL;
	//}

	// check if to+size fits to the segment
	if (!Memory::checkSegment(to, size)) {
		PRINT_DEBUG("Area %p (%x) would overflow to another segment after mapping to %p.\n",
			from, size, to);
		return EINVAL;
	}

	const void* checkAddr = to;
	size_t checkSize = size;

	// check if to is not in the range from+size
	if (Memory::isInRange(to, from, size)) {
		checkAddr = (void *)((size_t)to + size);
		checkSize = (size_t)to - (size_t)from;
	}

	// check if from is not in the range to+size
	if (Memory::isInRange(from, to, size)) {
		checkSize = (size_t)from - (size_t)to;
	}

	// check if the to, to+size range is free
	if (!isFree(checkAddr, checkSize)) {
		PRINT_DEBUG("Mapping %p (%x) to %p (%x) would overlap another area.\n",
			from, size, to, size);
		return EINVAL;
	}

	// save the vma from the virtual memory map
	VirtualMemoryArea vma = entry->data();
	// remove the old value
	m_virtualMemoryMap.remove(entry);
	// change the address of VMA
	vma.address(to);
	// insert it to the virtual memory map
	m_virtualMemoryMap.insert(vma);

	// clear the TLB
	freed();

	return EOK;
}

/* --------------------------------------------------------------------- */

int VirtualMemory::merge(const void* area1, const void* area2)
{
	if (Memory::getSegment(area1) != Memory::getSegment(area2)) {
		PRINT_DEBUG("Areas %p and %p to merge are in different segments.\n", area1, area2);
		return EINVAL;
	}

	// search for the first area
	VirtualMemoryMapEntry* entry1 =
		m_virtualMemoryMap.findItem(VirtualMemoryArea(area1));

	// if it doesn't exists or doesn't begin on the given address - fail
	if ((entry1 == NULL) || (entry1->data().address() != area1)) {
		PRINT_DEBUG("No VMA starts at %p.\n", area1);
		return EINVAL;
	}

	// search for the second area
	VirtualMemoryMapEntry* entry2 =
		m_virtualMemoryMap.findItem(VirtualMemoryArea(area2));

	// if it doesn't exists or doesn't begin on the given address - fail
	if ((entry2 == NULL) || (entry2->data().address() != area2)) {
		PRINT_DEBUG("No VMA starts at %p.\n", area2);
		return EINVAL;
	}

	if (area2 == (void *)((size_t)area1 + entry1->data().size())) {
		// on the first area call merge
		const_cast<VirtualMemoryArea&>(entry1->data()).merge(
			const_cast<VirtualMemoryArea&>(entry2->data()));
		// remove area2 from the virtual memory map
		m_virtualMemoryMap.remove(entry2);
	} else if (area1 == (void *)((size_t)area2 + entry2->data().size())) {
		// on the first area call merge
		const_cast<VirtualMemoryArea&>(entry2->data()).merge(
			const_cast<VirtualMemoryArea&>(entry1->data()));
		// remove area1 from the virtual memory map
		m_virtualMemoryMap.remove(entry1);
	} else {
		PRINT_DEBUG("Areas %p (%x) and %p (%x) are not adjacent.\n",
			area1, entry1->data().size(), area2, entry2->data().size());
		return EINVAL;
	}

	// clear the TLB
	freed();

	return EOK;
}

/* --------------------------------------------------------------------- */

int VirtualMemory::split(const void* from, const void* split)
{
	// check if split address is aligned
	if (!Memory::isAligned(split, PAGE_MIN)) {
		PRINT_DEBUG("Address %p is not aligned.\n", split);
		return EINVAL;
	}

	// search for the area
	VirtualMemoryMapEntry* entry =
		m_virtualMemoryMap.findItem(VirtualMemoryArea(from));

	// if it doesn't exists or doesn't begin on the given address - fail
	if ((entry == NULL) || (entry->data().address() != from)) {
		PRINT_DEBUG("No VMA starts at %p.\n", from);
		return EINVAL;
	}

	size_t size = entry->data().size();

	// check if split is inside the selected VMA
	if ((split == from) || !Memory::isInRange(split, from, size)) {
		PRINT_DEBUG("Address %p is not inside the area %p (%x).\n", split, from, size);
		return EINVAL;
	}

	VirtualMemoryArea vma = const_cast<VirtualMemoryArea&>(entry->data()).split(split);
	if (vma.size() == 0) {
		PRINT_DEBUG("No memory left to create new VM Area in split().\n");
		return ENOMEM;
	}

	PRINT_DEBUG("Adding VMA at %p with size %x to the virtual memory map (%u).\n",
		vma.address(), vma.size(), m_virtualMemoryMap.count());
	// add vma to the virtual memory map
	m_virtualMemoryMap.insert(vma);

	// clear the TLB
	freed();

	return EOK;
}

/* --------------------------------------------------------------------- */

bool VirtualMemory::translate(void*& address, Processor::PageSize& frameSize)
{
	PRINT_TLB_DEBUG("Virtual memory map tree size %u, TLB is looking for %p.\n",
		m_virtualMemoryMap.count(), address);

	// search for the address and get the VMA
	const VirtualMemoryMapEntry* entry = 
		m_virtualMemoryMap.findItem( VirtualMemoryArea(address) );

	// if VMA not found, address is not allocated
	if (entry == NULL) {
		PRINT_TLB_DEBUG("Address %p is not in the tree.\n", address);
		dump();
		return false;
	}

	//XXX
	//if (address == (void*)0xc01a4000) msim_stop();

	// find the address translation on the found VMA
	return entry->data().find(address, frameSize);
}

/* --------------------------------------------------------------------- */

bool VirtualMemory::isFree(const void* from, const size_t size)
{
	// if we have not allocated yet, everything is free (except NULL)
	if (m_virtualMemoryMap.count() == 0) return true;

	// check if address not in other block
	if (m_virtualMemoryMap.findItem(VirtualMemoryArea(from)) != NULL) {
		PRINT_DEBUG("Address %p is already taken.\n", from);
		return false;
	}

	// check if the space after from is free too
	// start from beginning
	const VirtualMemoryMapEntry& checkedAddress(VirtualMemoryArea(from, size));
	VirtualMemoryMapEntry *lower, *upper = &m_virtualMemoryMap.min();
	do {
		lower = upper;
		upper = (VirtualMemoryMapEntry *)lower->next();
	} while ((upper != NULL) && (*upper < checkedAddress));

	if (lower->data().address() > from) {
		// check if there is enough space after from to the first allocated block
		PRINT_DEBUG("Checking space (size %x) after %p.\n", size, from);
		return ((size_t)from + size) <= (size_t)(lower->data().address());
	}

	if (upper == NULL) {
		// no block with higher address then from
		PRINT_DEBUG("Checking block length before %p.\n", from);
		return (size_t)from >= ((size_t)(lower->data().address()) + lower->data().size());
	}

	PRINT_DEBUG("Checking if %p with size %x suits between %p (%x) and %p (%x).\n",
		from, size, lower->data().address(), lower->data().size(), upper->data().address(), upper->data().size());
	// check if from is after lower+size
	return ((size_t)from >= ((size_t)(lower->data().address()) + lower->data().size()))
		// and if from+size ends before the next allocated block (in upper)
		&& ((size_t)from + size) <= (size_t)(upper->data().address());
}

/* --------------------------------------------------------------------- */

void VirtualMemory::getFreeAddress(void*& from, const size_t size, Processor::PageSize frameType)
{
	// start from the beginning (min() - count check was done before this call)
	VirtualMemoryMapEntry* segmentLow = &m_virtualMemoryMap.min();

	// loop to the right segment (so from already contains address in the right segment)
	while (Memory::getSegment(from) != Memory::getSegment(segmentLow->data().address())) {
		if ((segmentLow = (VirtualMemoryMapEntry *)segmentLow->next()) == NULL) break;
	}

	// there was no allocation in the requested segment, so from is set to a good value
	if (segmentLow == NULL) return;

	// from is set to the minimal automatically assigned address in segment
	if ((from < segmentLow->data().address()) && (isFree(from, size))) {
		// from is lower than the first allocated block and there is enough free space
		return;
	}

	VirtualMemoryMapEntry *next, *low;
	size_t freeSize;

	while (1) {
		// intialize the variables
		low = segmentLow;
		freeSize = 0;

		// loop trough the allocated blocks, to find a suitable place for the new block
		do {
			set_from:
			// the next free pointer
			from = (void *)((size_t)(low->data().address()) + low->data().size());
			// the next used pointer
			next = (VirtualMemoryMapEntry *)low->next();

			//PRINT_DEBUG(" trying OLD %p as from \n", from);
			// align from to the requested pagesize, if it's not possible segment ends there
			if (!Memory::alignUp(from, frameType)) break;
			//PRINT_DEBUG(" trying NEW %p as from \n", from);

			// if from is aligned up, be sure next is after from (overflow risk)
			while ((next != NULL) && ((size_t)(next->data().address()) < (size_t)from)) {
				// check if from is not already taken
				if (Memory::isInRange(from, next->data().address(), next->data().size())) {
					low = next;
					goto set_from;
				}
				// move to the next area
				next = (VirtualMemoryMapEntry *)next->next();
			}
			// if next is NULL, memory after from is free
			if (next == NULL) break;

			//PRINT_DEBUG(" comparing from %p to next %p \n", from, next ? next->data().address() : (void *)0xDEAD0BED);

			// if running out of segment (the last free address could be OK)
			if (Memory::getSegment(from) != Memory::getSegment(next->data().address())) break;

			// free space between the two pointers (area addresses)
			freeSize = (size_t)(next->data().address()) - (size_t)(from);
			// store next as low for the next iteration
			low = next;
			// loop while the space is not enough
		} while (freeSize < size);

		if (Memory::checkSegment(from, size) || (frameType == PAGE_MIN)) {
			// everything OK or we used the smallest possible page size
			return;
		} else {
			// the from+size overflows the segment, we try smaller pagesize
			--frameType;
		}
	}
}

/* --------------------------------------------------------------------- */

void VirtualMemory::dump()
{
#ifndef VMA_NDEBUG
	if (m_virtualMemoryMap.count() == 0) {
		printf("[ VMA DEBUG ]: VM Map is empty.\n");
		return;
	}

	printf("\n[ VMA DEBUG ]: ============= VMM DUMP ===========\n");

	VirtualMemoryMapEntry *x = &m_virtualMemoryMap.min();
	do {
		printf("[ VMA DEBUG ]: VM Area is at %p (%x).\n",
			x->data().address(), x->data().size());

		x = (VirtualMemoryMapEntry *)x->next();
	} while (x != NULL);
	printf("[ VMA DEBUG ]: ==================================\n");
#endif
}


