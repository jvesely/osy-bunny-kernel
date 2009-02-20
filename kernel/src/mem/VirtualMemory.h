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
 * @brief Virtual memory.
 *
 * Virtual memory map contains the tree where the VMAs are stored.
 */
class VirtualMemory: public IVirtualMemoryMap
{
public:
	int allocate(void** from, size_t size, unsigned int flags);
	int free(const void* from);

	int resize(const void* from, const size_t size);
	int remap(const void* from, const void* to);
	int merge(const void* area1, const void* area2);
	int split(const void* from, const void* split);

	// translate fnc
	bool translate(void*& address, Processor::PageSize& frameSize);

	/**
	 * Dump the tree of VMAs. This dump is called always when TLB asks
	 * for a non existent address translation.
	 *
	 * @note If VMA_NDEBUG is defined, dump is quiet.
	 */
	void dump();

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

