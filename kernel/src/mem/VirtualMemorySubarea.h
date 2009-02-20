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
 * @brief Virtual memory subarea representation:.
 *
 * Virtual memory subarea is a part of virtual memory area.
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
	/**
	 * Create and initialize the virtual memory subarea.
	 *
	 * @param physicalAddress Physical address of the subarea.
	 * @param frameType Frame (page) size (enum value).
	 * @param frameCount Count of the subsequent frames.
	 */
	VirtualMemorySubarea(const void* physicalAddress, 
		const Processor::PageSize frameType, const size_t frameCount)
		: m_physicalAddress(physicalAddress), m_frameType(frameType), m_frameCount(frameCount)
	{}

	/**
	 * Get the size of the subarea in bytes.
	 *
	 * @return Size in bytes.
	 */
	inline size_t size() const;

	/**
	 * Get the frame size used in the subarea in bytes.
	 *
	 * @return Size of the used frames in bytes.
	 */
	inline size_t frameSize() const;

	/**
	 * Get the physical address of the subarea.
	 *
	 * @param index Offset. Address of which block we want (default is 0).
	 * @return The address of the physical block.
	 */
	inline void* address(size_t index = 0) const;

	/**
	 * Get the frame size (enum value) used in the subarea.
	 *
	 * @return Type of the used frames.
	 */
	inline Processor::PageSize frameType() const;

	/**
	 * Reduce the subarea to the given size.
	 *
	 * @param newSize The new size of the subarea.
	 * @return Whether the reduction was successful.
	 */
	bool reduce(const size_t newSize);

	/**
	 * Cut of the end of the subarea to stay the given size.
	 *
	 * @param newSize The new size of the subarea. After this size will be the
	 *   rest cut off.
	 * @return The rest of the subarea (part after the given size).
	 */
	VirtualMemorySubarea* split(const size_t newSize);

	/**
	 * Free the physical memory.
	 */
	void free();

	/**
	 * Change used frame type in the subarea and recalculate the count.
	 *
	 * @param newFrameType The new frame (page) type (enum value).
	 * @return Whether the conversion was successful.
	 */
	bool frameType(const Processor::PageSize newFrameType);

private:
	/** Physical address. */
	const void* m_physicalAddress;

	/** Frame (page) type used in the subarea. */
	Processor::PageSize m_frameType;

	/** Count of the subsequent frames. */
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


