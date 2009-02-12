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
	*	@param finalSize pointer to size of required memory. Function alligns this value according to
	*		page size (and returns final size of chunk via pointer).
	*	@return if successfull, returns address of new memory chunk, else NULL.
	*		Return address is address from kseg0.
	*/
	virtual void * getNewChunk(size_t * finalSize);

		/** @brief extend existing chunk of memory
	*
	*	Virtual function responsible for extending existing memory chunk. Result should be,
	*	that new piece of usable(yet unformatted) memory is right behind old chunk of memory.
	*	@param oldChunk old chunk FOOTER pointer
	*	@param finalSize pointer to required size of new memory.Function alligns this value according to
	*		page size (and returns final size of chunk via pointer).
	*	@return true if success, false otherwise
	*	@note this is NOT final implementation (dummy implementation has no sense :) )
	*/
	virtual bool extendExistingChunk(BlockFooter * oldChunk, size_t * finalSize, size_t originalSize)
	{
		return NULL;
	}

	/** @brief return memory chunk to frame allocator
	*
	*	Returns whole chunk with given size to the frame allocator.
	*	@note Does not disconnect it from list of chunks.
	*	@param frontBorder front border of returned chunk
	*	@param finalSize size of chunk
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
	*	@param finalSize pointer to size, which should remaining chunk have.Function alligns this value according to
	*		page size (and returns final size of chunk via pointer).
	*	@param originalSize original size of chunk
	*	@return TRUE if succesful, FALSE otherwise (function might not be implemented)
	*	@note this is NOT final implementation (dummy implementation has no sense :) )
	*/
	virtual bool reduceChunk(BlockFooter * frontBorder, size_t * finalSize, size_t originalSize)
	{
		return false;
	}

};
