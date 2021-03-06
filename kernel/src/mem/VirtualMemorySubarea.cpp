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
 * @brief Member function definitions of the VirtualMemorySubarea class.
 */

#include "mem/VirtualMemorySubarea.h"

#include "mem/Memory.h"
#include "mem/FrameAllocator.h"

//#define VMA_DEBUG

#ifndef VMA_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...) \
  printf("[ VMA DEBUG ]: "); \
  printf(ARGS);
#endif


bool VirtualMemorySubarea::reduce(const size_t newSize)
{
	size_t oldSize = size();

	if ((newSize == 0) || (oldSize < newSize)) return false;
	if (oldSize == newSize) return true;

	// oldSize > newSize
	if (!Memory::isAligned(newSize, m_frameType)) {
		// get the biggest possible frame size the newSize is aligned for
		frameType(Memory::isAligned(newSize));
	}

	void* freeFrom = (void *)((size_t)m_physicalAddress + newSize);
	size_t freeCount = (oldSize - newSize) / frameSize();

	// update the subarea
	m_frameCount -= freeCount;

	// free the unnecessary frames in the end of the block
	FrameAllocator::instance().frameFree(freeFrom, freeCount, frameType());

	return true;
}

/* --------------------------------------------------------------------- */

VirtualMemorySubarea* VirtualMemorySubarea::split(const size_t newSize)
{
	size_t oldSize = size();

	ASSERT((newSize != 0) && (newSize < oldSize));

	if (!Memory::isAligned(newSize, m_frameType)) {
		// get the biggest possible frame size the newSize is aligned for
		frameType(Memory::isAligned(newSize));
	}

	// update the subarea
	m_frameCount = newSize / frameSize();

	// create the new subarea
	return new VirtualMemorySubarea(
		(void *)((size_t)m_physicalAddress + newSize),
		frameType(),
		(oldSize - newSize) / frameSize());
}

/* --------------------------------------------------------------------- */

void VirtualMemorySubarea::free()
{
	// free the physical memory
	bool res = FrameAllocator::instance().frameFree(m_physicalAddress, m_frameCount, m_frameType);

	ASSERT (res);
	PRINT_DEBUG("Subarea from %p of size %x (%d x %x) was%s freed.\n",
		m_physicalAddress, size(), m_frameCount, Memory::frameSize(m_frameType), res ? "" : " NOT");
	res = true;
}

/* --------------------------------------------------------------------- */

bool VirtualMemorySubarea::frameType(const Processor::PageSize newFrameType)
{
	if (newFrameType == m_frameType) return true;

	if (newFrameType < m_frameType) {
		m_frameCount = m_frameCount * (frameSize() / Memory::frameSize(newFrameType));
		m_frameType = newFrameType;
		return true;
	}

	if (newFrameType > m_frameType) {
		//TODO: checks - address and count alignment
		//TODO: take care of call from VMA::address() - setter
	}

	return false;
}

