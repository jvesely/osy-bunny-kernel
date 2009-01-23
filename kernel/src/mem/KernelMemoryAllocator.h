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

#include "Singleton.h"
#include "BasicMemoryAllocator.h"

/*! @class KernelMemoryAllocator KernelMemoryAllocator.h
 * "mem/KernelMemoryAllocator.h"
 *
 * @brief Memory allocator that gets aditional memory directly from
 * the FrameAllocator.
 *
 * KernelMemoryAllocator inherits handling of the allocated and free blocks
 * from the BasicMemoryAllocator, but gets new memory blocks directly from
 * the FrameAllocator.
 */
class KernelMemoryAllocator:
	public BasicMemoryAllocator, public Singleton<KernelMemoryAllocator>
{
public:
	virtual void* getMemory( size_t ammount );
	virtual void freeMemory( const void* address );

protected:
	/** @brief get brand new chunk of memory
	*
	*	Virtual function responsible for getting memory from either frame or vma allocator.
	*	@note does not insert new chunk to list, nor creates structures on new memory
	*	@param finalSize size of required memory (alligned to some pagesize)
	*	@return if successfull, returns address of new memory chunk, else NULL.
	*	Return address is virtual address.
	*/
	virtual void * getNewChunk(size_t finalSize);

	/** @brief extend existing chunk of memory
	*
	*	Virtual function responsible for extending existing memory chunk. Result should be,
	*	that new piece of usable(yet unformatted) memory is right behind old chunk of memory.
	*	@param oldChunk old chunk FOOTER pointer
	*	@param finalSize required size of new memory (alligned to some pagesize)
	*	@return if succesfull beginning of new memory piece, else NULL.
	*	Return address is virtual address.
	*/
	virtual void * extendExistingChunk(BlockFooter * oldChunk, size_t finalSize)
	{
		return NULL;
	}
	/** @brief return memory chunk to frame allocator
	*
	*	Returns whole chunk with given size to the frame allocator.
	*	@note Does not disconnect it from list of chunks.
	*	@param frontBorder front border of returned chunk
	*	@param finalSize size of chunk (must be alligned)
	*/
	virtual void returnChunk(BlockFooter * frontBorder, size_t finalSize);


	/** @brief shrink existing memory chunk to given size
	*
	*	Virtual function responsible for shrinking existing memory chunk, function
	*	should be called only if such shrink is possible.
	*	Function reduces chunk from the end.
	*	Function does not handle chunk borders and structures, only returns part of
	*	memory chunk!!! Also does not do any checks.
	*	@param frontBorder memory chunk front border pointer
	*	@param totalSize size which the remaining part of chunk should have.
	*	@return TRUE if succesful, FALSE otherwise (function might not be implemented)
	*/
	virtual bool reduceChunk(BlockFooter * frontBorder, size_t totalSize)
	{
		return false;
	}

};
