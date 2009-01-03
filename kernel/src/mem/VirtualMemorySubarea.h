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

#include "structures/ListInsertable.h"
#include "mem/FrameAllocator.h"

/**
 * @class VirtualMemorySubarea VirtualMemorySubarea.h "mem/VirtualMemorySubarea.h"
 * @brief Virtual memory subarea.
 *
 * Virtual memory subarea is a part of virtual memory area.
 */
class VirtualMemorySubarea : public ListInsertable<VirtualMemorySubarea>
{
public:
	VirtualMemorySubarea(const void* physicalAddress, 
		const Processor::PageSize frameSize, const size_t frameCount)
		: m_physicalAddress(physicalAddress), m_frameSize(frameSize), m_frameCount(frameCount)
	{}

	inline size_t size() const;
	inline Processor::PageSize frameSize() const;
	inline void* address(size_t index) const;

	inline void free();

private:
	const void* m_physicalAddress;
	Processor::PageSize m_frameSize;
	size_t m_frameCount;

};

/* --------------------------------------------------------------------- */

inline size_t VirtualMemorySubarea::size() const
{
	return Processor::pages[m_frameSize].size * m_frameCount;
}

/* --------------------------------------------------------------------- */

inline Processor::PageSize VirtualMemorySubarea::frameSize() const
{
	return m_frameSize;
}

/* --------------------------------------------------------------------- */

inline void* VirtualMemorySubarea::address(size_t index) const
{
	return (void *)((size_t)m_physicalAddress + (index * Processor::pages[m_frameSize].size));
}

/* --------------------------------------------------------------------- */

inline void VirtualMemorySubarea::free()
{
	// free the physical memory
	MyFrameAllocator::instance().frameFree(m_physicalAddress, m_frameCount, 
		Processor::pages[m_frameSize].size);
}

