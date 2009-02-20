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
 * @brief Virtual memory.
 *
 * Virtual memory map contains the tree where the VMAs are stored.
 */

#pragma once

#include "api.h"

#include "structures/Trees.h"
#include "mem/VirtualMemoryArea.h"
#include "mem/IVirtualMemoryMap.h"
#include "drivers/Processor.h"

/**
 * @typedef
 * @brief Virtual memory map is implemented as splay tree of VMAs.
 */
typedef Trees<VirtualMemoryArea>::SplayTree VirtualMemoryMap;

/**
 * @typedef
 * @brief Virtual memory map entry is a splay binary node of VMAs.
 */
typedef SplayBinaryNode<VirtualMemoryArea> VirtualMemoryMapEntry;

/**
 * @class VirtualMemory VirtualMemory.h "mem/VirtualMemory.h"
 * @brief Virtual memory representation (virtual memory map).
 *
 * Virtual memory map contains the tree where the VMAs are stored.
 */
class VirtualMemory: public IVirtualMemoryMap
{
public:
	/**
	 * Create (allocate) a new virtual memory area and store it in the virtual memory map.
	 *
	 * @param[in,out] from Optionally (if VF_VA_USER flag set) it is the user defined
	 *   address of the new VMA (starting address). As output parameter it says the
	 *   address where the new VMA starts (if allocation successful).
	 * @param[in] size Requested size of the new VMA.
	 * @param[in] flags Bit array of segment (where to allocate) and the way how to
	 *   choose the address. (See flags.h for details.)
	 * @return EOK, ENOMEM or EINVAL
	 */
	int allocate(void** from, size_t size, unsigned int flags);

	/**
	 * Free one virtual memory area at the given address.
	 *
	 * @param from The starting address of the VMA (first byte).
	 * @return EOK or EINVAL if correspoding VMA was not found.
	 */
	int free(const void* from);

	/**
	 * Resize the given virtual memory area.
	 *
	 * @param from The address of a VMA (first byte).
	 * @param size The new size of the VMA (non zero value).
	 * @return EOK, ENOMEM or EINVAL
	 */
	int resize(const void* from, const size_t size);
	
	/**
	 * Remap virtual memory area to a new virtual address. The new virtual address
	 * can be (as it is not defined in the assignment and the tests want this feature)
	 * in other segment.
	 *
	 * @param from The address of the VMA (first byte)
	 * @param to The new starting address of the VMA.
	 * @return EOK or EINVAL.
	 */
	int remap(const void* from, const void* to);
	
	/**
	 * Merge two consequent virtual memory areas to one bigger block.
	 *
	 * @param area1 The address of a VMA (first byte). The order doesn't matter.
	 * @param area2 The address of a VMA (first byte). The order doesn't matter.
	 * @return EOK or EINVAL.
	 */
	int merge(const void* area1, const void* area2);

	/**
	 * Split one virtual memory area to two pieces.
	 *
	 * @param from The address of the VMA (first byte).
	 * @param split The address where to split, start the second piece.
	 * @return EOK or EINVAL.
	 */
	int split(const void* from, const void* split);

	/**
	 * Translate virtual address to physical address.
	 *
	 * @param[in,out] address As input parameter the virtual address to be translated and
	 *   as output the physical address (only if the translation was successful).
	 * @param[out] frameSize Output parameter, the page size (enum value) for the
	 *   physical block (what mask will be required for the physical address in TLB).
	 * @return Whether the translation was successful.
	 */
	bool translate(void*& address, Processor::PageSize& frameSize);

	/**
	 * Dump the tree of VMAs. This dump is called always when TLB asks
	 * for a non existent address translation (and when a process ends).
	 *
	 * @note If VMA_NDEBUG is defined, dump is quiet.
	 */
	void dump();

	/**
	 * Clean up and free all memory which was left in the tree.
	 */
	virtual ~VirtualMemory();

protected:
	/**
	 * Check if the requested block (range) is free.
	 *
	 * @param from Address where to start the check.
	 * @param size How many bytes do we want to check.
	 * @return Wheter the given block is free.
	 */
	bool isFree(const void* from, const size_t size);

	/**
	 * Calculate the first free address aligned to frameType of the given size after
	 * the given address from.
	 *
	 * @param from The address where to start the search (in/out parameter).
	 * @param size The requested size of the free block.
	 * @param frameType Alignment request for the found address.
	 */
	void getFreeAddress(void*& from, const size_t size, Processor::PageSize frameType);

private:
	/** Tree of the virtual memory map. */
	VirtualMemoryMap m_virtualMemoryMap;

};

