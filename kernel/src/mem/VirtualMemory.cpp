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

int VirtualMemory::allocate(void **from, size_t size, unsigned int flags)
{
	//TODO add new param with frame size or get the optimal size (check how is from aligned)
	size_t frameSize = 4096;

	// check if size is aligned
	if (!VirtualMemory::checkAligned(size, frameSize)) return EINVAL;

	if (VF_VIRT_ADDR(flags) == VF_VA_AUTO) {
		// automatically assign the virtual address
		if (m_virtualMemoryMap.count() == 0) {
			// FrameAllocator::MAX_FRAME_SIZE which is private
			// the lowest virtual memory (it should be aligned for the used frame size)
			//TODO: choose better way to get this number
			*from = (void *)16777216;
		} else {
			// if we already allocated something, find the next free big enough pointer
			size_t freeSize;
			// start from beginning
			VirtualMemoryMapEntry *next, *low = &m_virtualMemoryMap.min();

			do {
				// the next free pointer
				*from = (void *)((size_t)(low->data().address()) + low->data().size());
				// the next used pointer (TODO: next() should return my ptr)
				if ((next = (VirtualMemoryMapEntry *)low->next()) == NULL) break;
				// free space between the two pointers
				freeSize = (size_t)(next->data().address()) - (size_t)(*from);
				// loop while the space is not enough
			} while (freeSize >= size);

			//TODO align from to the frameSize

			// check if not on the end of the virtual space
			if (((size_t)*from + size) >= VirtualMemory::MAX_VIRTUAL_ADDRESS) {
				return ENOMEM;
			}
		}
	} else {
		// VF_VA_USER means the from is important for virtual address

		// check if address is aliged
		if (!VirtualMemory::checkAligned(*from, frameSize)) return EINVAL;
		// check if from/size is free
		if (!checkIfFree(*from, size)) return EINVAL;

		// VF_AT_KSEG0 and VF_AT_KSEG1 means VF_VA_USER is important also for frame allocator
		if ((VF_ADDR_TYPE(flags) != VF_AT_KSEG0) && (VF_ADDR_TYPE(flags) != VF_AT_KSEG1)) {
			// other segments are trough TLB, so clear the VF_VA_USER flag
			flags &= ~VF_VA_MASK;
			flags |= (VF_VA_AUTO << VF_VA_SHIFT);
		}
	}

	VirtualMemoryArea vma(*from, size);
	int res = vma.allocate(flags);

	if (res == ENOMEM) {
		// on error free the allocated memory and return
		vma.free();
		return ENOMEM;
	}

	// add vma to the virtual memory map
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

	return EOK;
}

/* --------------------------------------------------------------------- */

bool VirtualMemory::translate(void*& address, size_t& frameSize)
{
	// search for the address and get the VMA
	const VirtualMemoryMapEntry* entry = m_virtualMemoryMap.findItem(
		VirtualMemoryMapEntry(VirtualMemoryArea(address)));

	// if VMA not found, address is not allocated
	if (entry == NULL) return false;

	// find the address translation on the found VMA
	return entry->data().find(address, frameSize);
}

/* --------------------------------------------------------------------- */

bool VirtualMemory::checkIfFree(const void* from, const size_t size)
{
	// never return NULL as free space
	if (from == NULL) return false;

	// if we have not allocated yet, everything is free (except NULL)
	if (m_virtualMemoryMap.count() == 0) return true;

	// check if address not in other block
	if (m_virtualMemoryMap.findItem(VirtualMemoryMapEntry(VirtualMemoryArea(from))) != NULL) {
		return false;
	}

	// check if the space after from is free too

	// start from beginning
	const VirtualMemoryMapEntry& checkedAddress = VirtualMemoryMapEntry(VirtualMemoryArea(from, size));
	VirtualMemoryMapEntry *lower, *upper = &m_virtualMemoryMap.min();
	do {
		lower = upper;
		upper = (VirtualMemoryMapEntry *)lower->next();
	} while ((upper != NULL) && (*upper < checkedAddress));

	if (lower->data().address() > from) {
		// check if there is enough space after from to the first allocated block
		return ((size_t)from + size) <= (size_t)(lower->data().address());
	}

	if (upper == NULL) {
		// no block with higher address then from
		return (size_t)from > ((size_t)(lower->data().address()) + lower->data().size());
	}

	// check if from is after lower+size
	return ((size_t)from >= ((size_t)(lower->data().address()) + lower->data().size()))
		// and if from+size ends before the next allocated block (in upper)
		&& ((size_t)from + size) <= (size_t)(upper->data().address());
}

