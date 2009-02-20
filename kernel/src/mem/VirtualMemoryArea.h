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
 * @brief Virtual memory area.
 *
 * Virtual memory area is a block of virtual memory mapped to one or more
 * virtual memory subareas. The VMA is stored in a tree in the VirtualMemory class.
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
 * @brief Representation of one virtual memory area (containing subareas).
 *
 * Virtual memory area is a block of virtual memory mapped to one or more
 * virtual memory subareas. The VMA is stored in a tree in the VirtualMemory class.
 */
class VirtualMemoryArea
{
public:
	/**
	 * Create and initialize VMA.
	 *
	 * @param address The virtual address of the VMA.
	 * @param size Size of the VMA.
	 */
	VirtualMemoryArea(const void* address, const size_t size = 0)
		: m_address(address), m_size(size), m_subAreas(NULL)
	{}

	/**
	 * Get the address of VMA.
	 * @return The virtual address.
	 */
	inline const void* address() const;

	/**
	 * Set the virtual address of the VMA.
	 * @param newAddress The new address of the VMA.
	 */
	void address(const void* newAddress);

	/**
	 * Get the size of the VMA in bytes.
	 * @return Size of the VMA in bytes.
	 */
	inline size_t size() const;

	/**
	 * Resize VMA to the given size.
	 *
	 * @param size New size.
	 * @return EOK, ENOMEM, EINVAL.
	 */
	int resize(const size_t size);

	/**
	 * Merge this VMA with the given VMA.
	 * @param area The second area to merge.
	 */
	void merge(VirtualMemoryArea& area);

	/**
	 * Split VMA at the given address.
	 *
	 * @param split The address to split the VMA. This address should be inside
	 *   the VMA.
	 * @return VMA cut off from the end of the old sized VMA.
	 */
	VirtualMemoryArea split(const void* split);

	/**
	 * Allocate the VMA.
	 *
	 * @param flags Flags to check if the allocation will be done in KSEG0/1
	 *   and if the address could be automatically assigned or is user defined.
	 * @return EOK, ENOMEM, EINVAL.
	 */
	int allocate(const unsigned int flags);

	/**
	 * Free the VMA.
	 */
	void free();

	/**
	 * Search for the given address.
	 *
	 * @param address The virtual address to be searched for, output is
	 *   the found physical address (in/out parameter).
	 * @param frameType Output parameter for the frame size of the found block.
	 * @return Whether the address was found.
	 */
	bool find(void*& address, Processor::PageSize& frameType) const;

	/**
	 * Operator equals is used to compare elements in the splay tree.
	 *
	 * @param other The other element to compare to.
	 * @return Whether the two elements are the same.
	 */
	bool operator== (const VirtualMemoryArea& other) const;

	/**
	 * Operator less than is used to compare elements in the splay tree.
	 *
	 * @param other The other element to compare to.
	 * @return Whether this element is smaller than the other one.
	 */
	bool operator< (const VirtualMemoryArea& other) const;

private:
	/**
	 * Allocate at KSEG0 or KSEG1.
	 *
	 * @param address Address to allocate at (always converted to physical address).
	 * @param size Size to allocate.
	 * @return EOK, ENOMEM, EINVAL.
	 */
	int allocateAtKSegAddr(const void* address, const size_t size);

	/**
	 * Allocate at KSEG0 or KSEG1 depending on the m_address and m_size.
	 *
	 * @return EOK, ENOMEM, EINVAL.
	 */
	int allocateAtKSegAuto();

	/**
	 * Allocation at the user segment.
	 *
	 * @param virtualAddress Allocation starts at the given virtual address.
	 * @param size The requested size of the VMA.
	 * @return EOK, ENOMEM, EINVAL.
	 */
	int allocateAtKUSeg(const void* virtualAddress, const size_t size);

	/** Virtual address of the VMA. */
	const void* m_address;

	/** Size of the VMA. */
	size_t m_size;

	/** Subarea container. */
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

