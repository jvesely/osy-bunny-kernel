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

#include "mem/VirtualMemoryArea.h"

//#define VMA_DEBUG
//#define VMA_OPERATOR_DEBUG

#ifndef VMA_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...) \
  printf("[ VMA DEBUG ]: "); \
  printf(ARGS);
#endif

#ifndef VMA_OPERATOR_DEBUG
#define PRINT_OP_DEBUG(...)
#else
#define PRINT_OP_DEBUG(ARGS...) \
  printf("[ SPLAY DEBUG ]: "); \
  printf(ARGS);
#endif

int VirtualMemoryArea::allocate(const unsigned int flags)
{
	if (m_subAreas == NULL) {
		// create the subarea container only if necessary
		m_subAreas = new VirtualMemorySubareaContainer();
	}

	//TODO get optimal frame size !from can be aligned for 4k if VF_VA_USER!
	size_t frameSize = 4096;

	if ((VF_ADDR_TYPE(flags) == VF_AT_KSEG0) || (VF_ADDR_TYPE(flags) == VF_AT_KSEG1)) {
		// VF_VA_USER here means it has to have the same physical address (KSEG0-1)

		m_address = (void *)ADDR_OFFSET((size_t)m_address);
		// calculate the frame count (ceil it)
		size_t count = (m_size / frameSize) + (m_size % frameSize ? 1 : 0);
		// allocate one piece of memory
		if (MyFrameAllocator::instance().allocateAtAddress(m_address, count, frameSize) < count) {
			return ENOMEM;
		}

		// change address to virtual 0x8* or 0xA*
		if (VF_ADDR_TYPE(flags) == VF_AT_KSEG0) {
			m_address = (void *)ADDR_TO_KSEG0((size_t)m_address);
		} else {
			m_address = (void *)ADDR_TO_KSEG1((size_t)m_address);
		}

		// save as subarea
		VirtualMemorySubarea* s = new VirtualMemorySubarea(m_address, frameSize, count);
		// add it to the list
		s->append(m_subAreas);
	} else {
		// how much we still need
		size_t allocate = m_size;

		// new and old values the frameAlloc function changes
		void* address = NULL;
		size_t newCount = 0, oldCount = 0;

		while (allocate > 0) {
			// if there was a unsuccesfull alloc, the old-new count differs
			if (newCount != oldCount) {
				oldCount = newCount;
			} else {
				// if they equals, calculate how much frames we need
				oldCount = (allocate / frameSize) + (allocate % frameSize ? 1 : 0);
			}

			// allocate
			newCount = MyFrameAllocator::instance().frameAlloc(&address, oldCount, frameSize, flags);

			// check for ENOMEM
			if (newCount == 0) {
				return ENOMEM;
			}

			// check success
			if (oldCount == newCount) {
				// create the subares
				VirtualMemorySubarea* s = new VirtualMemorySubarea(address, frameSize, newCount);
				// add it to the list
				s->append(m_subAreas);
				// decrease the amount of "still needed" memory
				allocate -= (frameSize * newCount);
				// clear the address pointer
				address = NULL;
			}
		}
	}

	return EOK;
}

/* --------------------------------------------------------------------- */

void VirtualMemoryArea::free()
{
	while (m_subAreas->size() != 0) {
		// free all the allocated subareas
		VirtualMemorySubarea* s = m_subAreas->getFront();
		s->free();
		delete s;
	}
	delete m_subAreas;
}

/* --------------------------------------------------------------------- */

bool VirtualMemoryArea::find(void*& address, size_t& frameSize) const
{
	PRINT_DEBUG("Searching address %p in VMA %p (size %x with %u subareas).\n",
		address, m_address, m_size, m_subAreas->size());

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
			// if so, set output parameters and return true
			address = (*subarea)->address((((size_t)va - (size_t)vaStart) / (*subarea)->frameSize()));
			frameSize = (*subarea)->frameSize();
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
	return ((other.m_address >= m_address) && (other.m_address < (void *)((size_t)m_address + m_size)))
		// check if this is inside other
		|| ((m_address >= other.m_address) && (m_address < (void *)((size_t)other.m_address + other.m_size)));
}

/* --------------------------------------------------------------------- */

bool VirtualMemoryArea::operator< (const VirtualMemoryArea& other) const
{
	PRINT_OP_DEBUG("Contrasting (<) %p (size %x) and %p (size %x).\n",
		m_address, m_size, other.m_address, other.m_size);

	return (m_address < other.m_address)
		&& ((void *)((size_t)m_address + m_size) <= other.m_address);
}

