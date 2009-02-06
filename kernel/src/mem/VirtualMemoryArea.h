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
#include "drivers/Processor.h"

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
		: m_address(address), m_size(size), m_subAreas(NULL)
	{}

	inline const void* address() const;
	void address(const void* newAddress);
	inline size_t size() const;

	int resize(const size_t size);
	void merge(VirtualMemoryArea& area);
	VirtualMemoryArea split(const void* split);

	int allocate(const unsigned int flags);
	void free();

	bool find(void*& address, Processor::PageSize& frameType) const;

	bool operator== (const VirtualMemoryArea& other) const;
	bool operator< (const VirtualMemoryArea& other) const;

private:
	int allocateAtKSegAddr(const void* address, const size_t size);
	int allocateAtKSegAuto();
	int allocateAtKUSeg(const void* virtualAddress, const size_t size);

	const void* m_address;
	size_t m_size;

	VirtualMemorySubareaContainer* m_subAreas;

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

