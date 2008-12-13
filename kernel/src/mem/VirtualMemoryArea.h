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

#pragma once

#include "api.h"

#include "mem/FrameAllocator.h"
#include "mem/VirtualMemorySubarea.h"

typedef List<VirtualMemorySubarea *> VirtualMemorySubareaContainer;
typedef List<VirtualMemorySubarea *>::Iterator VirtualMemorySubareaIterator;

/**
 * @class VirtualMemoryArea VirtualMemoryArea.h "mem/VirtualMemoryArea.h"
 * @brief Virtual memory subarea.
 *
 * Virtual memory subarea is a part of virtual memory area.
 */
class VirtualMemoryArea
{
public:
	VirtualMemoryArea(const void* address, const size_t size = 0)
		: m_address(address), m_size(size), m_subAreas()
	{
		printf("VMA constructed\n");
	}

	~VirtualMemoryArea() {
		printf("VMA destructed\n");
	}

	inline const void* address() const;
	inline size_t size() const;

	inline int allocate(const unsigned int flags);
	inline void free();

	bool find(void*& address, size_t& frameSize) const;

	bool operator== (const VirtualMemoryArea& other) const;
	bool operator< (const VirtualMemoryArea& other) const;

private:

	const void* m_address;
	size_t m_size;

	VirtualMemorySubareaContainer m_subAreas;

};

/* --------------------------------------------------------------------- */

inline const void* VirtualMemoryArea::address() const
{
	return m_address;
}

/* --------------------------------------------------------------------- */

inline size_t VirtualMemoryArea::size() const
{
	return m_size;
}

/* --------------------------------------------------------------------- */

inline int VirtualMemoryArea::allocate(const unsigned int flags)
{
	//TODO get optimal frame size !from can be aligned for 4k if VF_VA_USER!
	size_t frameSize = 4096;

	if (VF_VIRT_ADDR(flags) == VF_VA_USER) {
		// VF_VA_USER here means it has to have the same physical address (KSEG0-1)
		// calculate the frame count (ceil it)
		size_t count = (m_size / frameSize) + (m_size % frameSize ? 1 : 0);
		// allocate one piece of memory
		if (MyFrameAllocator::instance().allocateAtAddress(m_address, count, frameSize) < count) {
			return ENOMEM;
		}
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
				s->append(&m_subAreas);
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

inline void VirtualMemoryArea::free()
{
	while (m_subAreas.size() != 0) {
		// free all the allocated subareas
		VirtualMemorySubarea* s = m_subAreas.getFront();
		s->free();
		delete s;
	}
}

/* --------------------------------------------------------------------- */

bool VirtualMemoryArea::find(void*& address, size_t& frameSize) const
{
	// virtualAddress we are searching
	const void *va = address;
	// virtual start and end addresses of the subarea
	void *vaEnd, *vaStart = const_cast<void *>(m_address);

	// get the first subarea (expect there is at least one)
	VirtualMemorySubareaIterator subarea = m_subAreas.begin();

	do {
		// set the end of the subarea
		vaEnd = (void *)((size_t)vaStart + (*subarea)->size());
		// check if the virtual address is in the range (in subarea)
		if ((va > vaStart) && (va < vaEnd)) {
			// if so, set output parameters and return true
			address = (*subarea)->address((((size_t)va - (size_t)vaStart) / (*subarea)->frameSize()));
			frameSize = (*subarea)->frameSize();
			return true;
		}
		// set new start for the next subarea
		vaStart = (void *)((size_t)vaStart + (*subarea)->size());
	} while (++subarea != m_subAreas.end());

	// if not returned yet, we have not found the address
	return false;
}

/* --------------------------------------------------------------------- */

bool VirtualMemoryArea::operator== (const VirtualMemoryArea& other) const
{
	return (m_address <= other.m_address)
		&& (other.m_address < (void *)((int)m_address + m_size));
}

/* --------------------------------------------------------------------- */

bool VirtualMemoryArea::operator< (const VirtualMemoryArea& other) const
{
	return m_address < other.m_address;
}


