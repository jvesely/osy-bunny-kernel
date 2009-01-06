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

#include "flags.h"
#include "VirtualMemory.h"

#define VMA_DEBUG

#ifndef VMA_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...) \
  printf("[ VMA DEBUG ]: "); \
  printf(ARGS);
#endif

int VirtualMemory::allocate(void **from, size_t size, unsigned int flags)
{
	//TODO add new param with frame size or get the optimal size (check how is from aligned)
	size_t frameSize = 4096;

	// check if size is aligned and not zero
	if (!VirtualMemory::checkAligned(size, frameSize)) {
		PRINT_DEBUG("Size %u not aligned to frame size %x.\n", size, frameSize);
		return EINVAL;
	}

	// check if in the given size is possible in the given segment (for area1 test)
	if (!VirtualMemory::checkSizeInSegment(size, VF_ADDR_TYPE(flags))) {
		PRINT_DEBUG("Segment %u is smaller than requested size %x.\n", VF_ADDR_TYPE(flags), size);
		return ENOMEM;
	}

	if (VF_VIRT_ADDR(flags) == VF_VA_AUTO) {
		// automatically assign the virtual address

		// get the right address in segment (KSEG0/1 will be ignored later)
		*from = getAddressInSegment(VF_ADDR_TYPE(flags));

		if (m_virtualMemoryMap.count() != 0) {
			getFreeAddress(*from, size);
			//TODO align from to the frameSize
		}

		PRINT_DEBUG("Address %p choosen for the new block of size %x.\n", *from, size);
	} else {
		// VF_VA_USER means the from is important for virtual address

		// check if address is aliged
		if (!VirtualMemory::checkAligned(*from, frameSize)) {
			PRINT_DEBUG("Address %p not aligned to frame size %x.\n", *from, frameSize);
			return EINVAL;
		}
		// check if from/size is free
		if (!checkIfFree(*from, size)) {
			PRINT_DEBUG("Address %p with size %x (or its part) is not free.\n", *from, size);
			return EINVAL;
		}

		// for test area1
		// calculate the missing or wrong segment
		flags &= ~VF_AT_MASK;
		flags |= (VirtualMemory::getSegment(*from) << VF_AT_SHIFT);
		//XXX printf("FLAGS: %x - seg %u, type %u\n", flags, VF_ADDR_TYPE(flags), VF_VIRT_ADDR(flags));
	}

	// check if the whole  virtual block is in the right segment (KSEG0/1 will be checked later)
	if ((VirtualMemory::getSegment(*from) != VF_ADDR_TYPE(flags)) || !checkSegment(*from, size)) {
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

	VirtualMemoryArea vma(*from, size);
	PRINT_DEBUG("Allocating physical memory for VMA at %p with size %x.\n", *from, size);
	int res = vma.allocate(flags);

	if (res == ENOMEM) {
		PRINT_DEBUG("Not enough memory for %p with size %x, freeing already allocated parts.\n",
			*from, size);
		// on error free the allocated memory and return
		vma.free();
		return ENOMEM;
	}

	// add vma to the virtual memory map
	PRINT_DEBUG("Adding VMA at %p with size %x to the virtual memory map (%u).\n",
		*from, size, m_virtualMemoryMap.count());
	m_virtualMemoryMap.insert(vma);

	return EOK;
}

/* --------------------------------------------------------------------- */

int VirtualMemory::free(const void *from)
{
	// search for the address and get the VMA
	const VirtualMemoryMapEntry* entry = m_virtualMemoryMap.findItem(
		VirtualMemoryMapEntry(VirtualMemoryArea(from)));

	// if it doesn't exists or doesn't begin on the given address - fail
	if ((entry == NULL) || (entry->data().address() != from)) return EINVAL;

	// free all subareas
	const_cast<VirtualMemoryArea&>(entry->data()).free();

	// delete the entry from the tree
	delete entry;

	freed();

	return EOK;
}

/* --------------------------------------------------------------------- */

bool VirtualMemory::translate(void*& address, Processor::PageSize& frameSize)
{
	PRINT_DEBUG("Virtual memory map tree size %u.\n", m_virtualMemoryMap.count());

	// search for the address and get the VMA
	const VirtualMemoryMapEntry* entry = 
		m_virtualMemoryMap.findItem( VirtualMemoryArea(address) );

	// if VMA not found, address is not allocated
	if (entry == NULL) {
		PRINT_DEBUG("Address %p is not in the tree.\n", address);
		return false;
	}

	// find the address translation on the found VMA
	frameSize = Processor::PAGE_MIN;
	return entry->data().find(address, frameSize);
}

/* --------------------------------------------------------------------- */

bool VirtualMemory::checkIfFree(const void* from, const size_t size)
{
	// if we have not allocated yet, everything is free (except NULL)
	if (m_virtualMemoryMap.count() == 0) return true;

	// check if address not in other block
	if (m_virtualMemoryMap.findItem(VirtualMemoryArea(from)) != NULL) {
		PRINT_DEBUG("Address %p was found in the virtual memory map.\n", from);
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

void VirtualMemory::getFreeAddress(void*& from, const size_t size)
{
	// start from beginning (min() - so count check was done before call)
	VirtualMemoryMapEntry *next, *low = &m_virtualMemoryMap.min();

	// loop to the right segment (so from already contains address in the right segment)
	while (VirtualMemory::getSegment(from) != VirtualMemory::getSegment(low->data().address())) {
		if ((low = (VirtualMemoryMapEntry *)low->next()) == NULL) break;
	}

	if (low == NULL) {
		// there was no allocation in the requested segment, so from is set to a good value
		return;
	}

	// from is set to the minimal automatically assigned address in segment
	if ((from < low->data().address()) && (checkIfFree(from, size))) {
		// from is lower than the first allocated block and there is enough free space
		return;
	}

	size_t freeSize;
	// loop trough the allocated block, to find a suitable place for the new block
	do {
		// the next free pointer
		from = (void *)((size_t)(low->data().address()) + low->data().size());
		// the next used pointer (TODO: next() should return my ptr)
		if ((next = (VirtualMemoryMapEntry *)low->next()) == NULL) break;
		// if running out of segment
		if (VirtualMemory::getSegment(from) != VirtualMemory::getSegment(next->data().address())) break;
		// free space between the two pointers
		freeSize = (size_t)(next->data().address()) - (size_t)(from);
		// store next as low for the next iteration
		low = next;
		// loop while the space is not enough
	} while (freeSize < size);
}

