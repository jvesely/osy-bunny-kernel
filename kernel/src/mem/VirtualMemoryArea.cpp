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

#include "VirtualMemoryArea.h"

#include "mem/Memory.h"
#include "mem/TLB.h"

//#define VMA_DEBUG
//#define VMA_TLB_DEBUG
//#define VMA_OPERATOR_DEBUG

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

#ifndef VMA_OPERATOR_DEBUG
#define PRINT_OP_DEBUG(...)
#else
#define PRINT_OP_DEBUG(ARGS...) \
  printf("[ SPLAY DEBUG ]: "); \
  printf(ARGS);
#endif

using namespace Processor;

/* --------------------------------------------------------------------- */

void VirtualMemoryArea::address(const void* newAddress)
{
	PageSize oldPS = Memory::isAligned(m_address);
	PageSize newPS = Memory::isAligned(newAddress);
	// save the new address
	m_address = newAddress;

	// if the new address is aligned only to smaller page size, all subareas
	// need to be crashed to smaller pieces
	if (newPS < oldPS) {
		// get the first subarea (expect there is at least one)
		VirtualMemorySubareaIterator subarea = m_subAreas->begin();
		do {
			// change the frame type of all subareas to the smaller one
			// if it will be changed to a bigger one, it can't hurt (setter is correct)
			(*subarea)->frameType(newPS);
		} while (++subarea != m_subAreas->end());
	}
}

/* --------------------------------------------------------------------- */

int VirtualMemoryArea::allocate(const unsigned int flags)
{
	if (m_subAreas == NULL) {
		// create the subarea container only if necessary
		if ((m_subAreas = new VirtualMemorySubareaContainer()) == NULL) {
			return ENOMEM;
		}
	}

	if (VF_SEG_NOTLB(VF_ADDR_TYPE(flags))) {
		// KSEG0 and KSEG1

		// change address to virtual 0x8* or 0xA*
		if (VF_ADDR_TYPE(flags) == VF_AT_KSEG0) {
			m_address = (void *)ADDR_TO_KSEG0((size_t)m_address);
		} else {
			m_address = (void *)ADDR_TO_KSEG1((size_t)m_address);
		}

		// VF_VA_USER here means it has to have the same physical address (KSEG0-1)
		if (VF_VIRT_ADDR(flags) == VF_VA_USER) {
			return allocateAtKSegAuto();
		} else {
			return allocateAtKSegAddr(m_address, m_size);
		}
	} else {
		// User segments
		return allocateAtKUSeg(m_address, m_size);
	}
}

/* --------------------------------------------------------------------- */

int VirtualMemoryArea::allocateAtKSegAddr(const void* address, const size_t size)
{
	// KSEG0 and KSEG1 are without TLB, so don't bother about page size
	Processor::PageSize frameType = Processor::PAGE_MIN;
	// calculate the frame count
	size_t count = size / Memory::frameSize(frameType);

	// allocate one piece of memory at address
	if (FrameAllocator::instance().allocateAtAddress(
		(void *)ADDR_OFFSET((size_t)address), count, frameType
		) < count)
	{
		return ENOMEM;
	}

	// save as subarea
	VirtualMemorySubarea* s = new VirtualMemorySubarea(address, frameType, count);
	PRINT_DEBUG("Subarea created at %p physical memory, build of %d frames of size %x.\n",
		address, count, Memory::frameSize(frameType));
	// add it to the list
	s->append(m_subAreas);

	return EOK;
}

/* --------------------------------------------------------------------- */

int VirtualMemoryArea::allocateAtKSegAuto()
{
	// KSEG0 and KSEG1 are without TLB, so don't bother about page size
	Processor::PageSize frameType = Processor::PAGE_MIN;
	// address without the KSEG0/1 offset
	void * address = (void *)ADDR_OFFSET((size_t)m_address);
	// calculate the frame count
	size_t count = m_size / Memory::frameSize(frameType);

	// allocate one piece of memory (anywhere)
	if (FrameAllocator::instance().allocateAtKseg0(
		&address, count, frameType) < count) {
		return ENOMEM;
	}

	// change the address back to virtual 0x8* or 0xA*
	m_address = (void *)(ADDR_PREFIX((size_t)m_address) | (size_t)address);

	// save as subarea
	VirtualMemorySubarea* s = new VirtualMemorySubarea(m_address, frameType, count);
	PRINT_DEBUG("Subarea created at %p physical memory, build of %d frames of size %x.\n",
		m_address, count, frameType);
	// add it to the list
	s->append(m_subAreas);

	return EOK;
}

/* --------------------------------------------------------------------- */

int VirtualMemoryArea::allocateAtKUSeg(const void* virtualAddress, const size_t size)
{
	// temporary list for newly allocated subareas
	VirtualMemorySubareaContainer newSubareas;
	// how much we still need
	size_t allocate = size;

	//TODO get optimal frame size !from can be aligned for 4k if VF_VA_USER!
	PageSize frameType = TLB::suggestPageSize(m_size, 100, 1);
	PageSize addressAlignedFor = Memory::isAligned(virtualAddress);

	// if address is not aligned for the suggested page size, fallback
	if (frameType > addressAlignedFor) {
		frameType = addressAlignedFor;
	}

	// force 4K pages
	//frameType = PAGE_MIN;

	// new and old values the frameAlloc function changes
	void* address = NULL;
	size_t newCount = 0, oldCount = 0;

	while (allocate > 0) {
		// if there was a unsuccesfull alloc, the old-new count differs
		if ((newCount != oldCount) && (newCount != 0)) {
			oldCount = newCount;
		} else {
			// if they equals, calculate how much frames we need
			oldCount = (allocate / Memory::frameSize(frameType)) +
				(allocate % Memory::frameSize(frameType) ? 1 : 0);
		}

		// allocate
		newCount = FrameAllocator::instance().allocateAtKuseg(
			&address, oldCount, frameType);

		// check for no free frames
		if (newCount == 0) {
			if (frameType == PAGE_MIN) {
				// clear the temporary subarea container
				VirtualMemorySubarea* s = NULL;
				while (newSubareas.size() != 0) {
					s = newSubareas.getFront();
					s->free();
					delete s;
				}
				return ENOMEM;
			} else {
				--frameType;
				continue;
			}
		}

		// check success
		if (oldCount == newCount) {
			// create the subares
			VirtualMemorySubarea* s = new VirtualMemorySubarea(address, frameType, newCount);
			PRINT_DEBUG("Subarea created at %p physical memory, build of %d frames of size %x.\n",
				address, newCount, Memory::frameSize(frameType));
			// add it to the list
			s->append(&newSubareas);
			// decrease the amount of "still needed" memory
			allocate -= (Memory::frameSize(frameType) * newCount);
			// clear the address pointer
			address = NULL;
		}
	}

	// put all the newly allocated subareas to the m_subAreas container
	VirtualMemorySubarea* s = NULL;
	while (newSubareas.size() != 0) {
		s = newSubareas.getFront();
		s->append(m_subAreas);
	}

	return EOK;
}

/* --------------------------------------------------------------------- */

void VirtualMemoryArea::free()
{
	if (m_subAreas != NULL) {
		VirtualMemorySubarea* s = NULL;
		while (m_subAreas->size() != 0) {
			// free all the allocated subareas
			s = m_subAreas->getFront();
			s->free();
			delete s;
		}
		delete m_subAreas;
	}
}

/* --------------------------------------------------------------------- */

int VirtualMemoryArea::resize(const size_t size)
{
	if (size == m_size) return EOK;

	int result = EOK;

	if (size < m_size) {
		// reduce the VMA
		size_t partialSize = 0;
		void* address = NULL;

		// get the first subarea (expect there is at least one)
		VirtualMemorySubareaIterator subarea = m_subAreas->begin();

		do {
			partialSize += (*subarea)->size();
			if (partialSize >= size) {
				address = (*subarea)->address();

				PRINT_DEBUG("Reducing subarea at %p to %x from %x.\n",
					address, size - (partialSize - (*subarea)->size()), (*subarea)->size());
				// reduce the subarea to a new size
				if (!(*subarea)->reduce(size - (partialSize - (*subarea)->size()))) {
					return EINVAL;
				}
				break;
			}
		} while (++subarea != m_subAreas->end());

		ASSERT(subarea != m_subAreas->end());

		// free all the subareas after the last (reduced) block
		VirtualMemorySubarea* s = m_subAreas->getBack();
		while (s->address() != address) {
			s->free();
			delete s;
			s = m_subAreas->getBack();
		}
	} else {
		size_t allocate = size - m_size;
		void* address = (void *)((size_t)m_address + m_size);

		// enlarge the VMA
		if (VF_SEG_NOTLB(Memory::getSegment(m_address))) {
			// if in KSEG0/1 the new allocation have to be placed just after the block
			result = allocateAtKSegAddr(address, allocate);
		} else {
			// if in User segments, allocate some place anywhere
			result = allocateAtKUSeg(address, allocate);
		}

		if (result != EOK) return result;
	}

	m_size = size;
	return EOK;
}

/* --------------------------------------------------------------------- */

void VirtualMemoryArea::merge(VirtualMemoryArea& area)
{
	// update the size
	m_size += area.size();

	// steal all the subareas from the given area
	VirtualMemorySubarea* s = NULL;
	while (area.m_subAreas->size() != 0) {
		s = area.m_subAreas->getFront();
		s->append(m_subAreas);
	}
}

/* --------------------------------------------------------------------- */

VirtualMemoryArea VirtualMemoryArea::split(const void* split)
{
	size_t oldSize = size();
	m_size = (size_t)split - (size_t)address();

	// create the new area
	VirtualMemoryArea vma(split, oldSize - m_size);
	// create the subarea container in the new area
	vma.m_subAreas = new VirtualMemorySubareaContainer();
	if (vma.m_subAreas == NULL) return VirtualMemoryArea(0, 0);

	// get the first subarea (expect there is at least one)
	VirtualMemorySubarea* s = NULL;

	size_t transferred = 0;
	do {
		s = m_subAreas->getBack();
		if ((transferred + s->size()) > vma.size()) {
			// if the s would be more than wanted, split it
			s = s->split(transferred + s->size() - vma.size());
		}
		s->append(vma.m_subAreas);
		transferred += s->size();
	} while (transferred != vma.size());

	return vma;
}

/* --------------------------------------------------------------------- */

bool VirtualMemoryArea::find(void*& address, Processor::PageSize& frameType) const
{
	if (m_subAreas == NULL) return false;

	PRINT_TLB_DEBUG("Searching address %p in VMA %p (size %x with %u subareas).\n",
		address, m_address, m_size, m_subAreas->size());

	/* Skip searching if it is not in my range. */
	if (address < m_address || address >= (void*)((uintptr_t)m_address + m_size))
		return false;

	// virtualAddress we are searching
	const void *va = address;
	// virtual start and end addresses of the subarea
	void *vaEnd, *vaStart = const_cast<void *>(m_address);

	// get the first subarea (expect there is at least one)
	VirtualMemorySubareaIterator subarea = m_subAreas->begin();

	do {
		// set the end of the subarea
		vaEnd = (void *)((size_t)vaStart + (*subarea)->size());
		// check if the virtual address is in the range (in subarea)
		if ((va >= vaStart) && (va < vaEnd)) {
			PRINT_TLB_DEBUG("Searching in subarea from %p to %p.\n", vaStart, vaEnd);
			// if so, set output parameters and return true
			address = (*subarea)->address((((size_t)va - (size_t)vaStart) / (*subarea)->frameSize()));
			frameType = (*subarea)->frameType();
			PRINT_TLB_DEBUG("Physical address found at %p with frame size %x.\n",
				address, Memory::frameSize(frameType));
			return true;
		}
		// set new start for the next subarea
		vaStart = (void *)((size_t)vaStart + (*subarea)->size());
	} while (++subarea != m_subAreas->end());

	// if not returned yet, we have not found the address
	return false;
}

/* --------------------------------------------------------------------- */

bool VirtualMemoryArea::operator== (const VirtualMemoryArea& other) const
{
	PRINT_OP_DEBUG("Comparing (==) %p (size %x) and %p (size %x).\n",
		m_address, m_size, other.m_address, other.m_size);

	// check if other is inside this
	return Memory::isInRange(other.m_address, m_address, m_size)
		// check if this is inside other
		|| Memory::isInRange(m_address, other.m_address, other.m_size);
}

/* --------------------------------------------------------------------- */

bool VirtualMemoryArea::operator< (const VirtualMemoryArea& other) const
{
	PRINT_OP_DEBUG("Contrasting (<) %p (size %x) and %p (size %x).\n",
		m_address, m_size, other.m_address, other.m_size);

	return (m_address < other.m_address)
		&& ((void *)((size_t)m_address + m_size) <= other.m_address);
}

