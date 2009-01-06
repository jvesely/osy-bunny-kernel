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
 * @brief KernelMemoryAllocator declaration.
 *
 */
#pragma once

#include "BasicMemoryAllocator.h"

/*! @class KernelMemoryAllocator KernelMemoryAllocator.h 
 * "mem/KernelMemoryAllocator.h"
 *
 * @brief Memory allocator that gets aditional memory directly from
 * the FrameAllocator.
 *
 * KernelMemoryAllocator inherits handling of the allocated and free blocks
 * from the BasicMemoryAllocator, but gets new memoyr blocks directly from
 * the FrameAllocator.
 */
class KernelMemoryAllocator: public BasicMemoryAllocator
{
public:
	virtual void* getMemory( size_t ammount );
	virtual void freeMemory( const void* address );
protected:
	/*! @brief Gets new block from the FrameAllocator.
	 * @param realSize size of the requested block
	 * @return Pointer to the header of the new block on success,
	 * 	NULL otherwise.
	 * Size of the returned block is >= requested size.
	 */
	BlockHeader * getBlock( size_t realSize );

	/*! @brief Returns no longer used block.
	 * @param header pointer to the header of the returned block.
	 */
	void returnBlock( BlockHeader* header );
};
