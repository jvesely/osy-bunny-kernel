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

#include "drivers/Processor.h"
#include "structures/ListInsertable.h"

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
		const Processor::PageSize frameType, const size_t frameCount)
		: m_physicalAddress(physicalAddress), m_frameType(frameType), m_frameCount(frameCount)
	{}

	inline size_t size() const;
	inline size_t frameSize() const;
	inline void* address(size_t index = 0) const;

	inline Processor::PageSize frameType() const;

	bool reduce(const size_t newSize);
	VirtualMemorySubarea* split(const size_t newSize);

	void free();

	bool frameType(const Processor::PageSize newFrameType);

private:
	const void* m_physicalAddress;
	Processor::PageSize m_frameType;
	size_t m_frameCount;

};

/* --------------------------------------------------------------------- */

inline size_t VirtualMemorySubarea::size() const
{
	return frameSize() * m_frameCount;
}

/* --------------------------------------------------------------------- */

inline size_t VirtualMemorySubarea::frameSize() const
{
	return Processor::pages[m_frameType].size;
}

/* --------------------------------------------------------------------- */

inline Processor::PageSize VirtualMemorySubarea::frameType() const
{
	return m_frameType;
}

/* --------------------------------------------------------------------- */

inline void* VirtualMemorySubarea::address(size_t index) const
{
	return (void *)((size_t)m_physicalAddress + (index * frameSize()));
}


