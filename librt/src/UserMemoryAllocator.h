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

#include "Singleton.h"
#include "BasicMemoryAllocator.h"
#include "YieldingSpinLock.h"

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
class UserMemoryAllocator:
	public BasicMemoryAllocator, public Singleton<UserMemoryAllocator>
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
	*	@param finalSize reference to size of required memory. Function alligns this value according to
	*		page size (and thus returns final size of chunk via reference).
	*	Return address is virtual.
	*/
	virtual void * getNewChunk(size_t * finalSize);

	/** @brief extend existing chunk of memory
	*
	*	Virtual function responsible for extending existing memory chunk. Result should be,
	*	that new piece of usable(yet unformatted) memory is right behind old chunk of memory.
	*	@param oldChunk old chunk FOOTER pointer
	*	@param finalSize reference to required size of new memory.
	*	@return if succesfull beginning of new memory piece, else NULL.
	*		Return address is virtual.
	*	@note this is NOT final implementation (dummy implementation has no sense :) )
	*	@note there is almost no way to return finalSize
	*/
	virtual bool extendExistingChunk(BlockFooter * oldChunk, size_t & finalSize, size_t originalSize)
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
	void returnChunk(BlockFooter * frontBorder, size_t finalSize);


	/** @brief shrink existing memory chunk to given size
	*
	*	Virtual function responsible for shrinking existing memory chunk, function
	*	should be called only if such shrink is possible.
	*	Function reduces chunk from the end.
	*	Function does not handle chunk borders and structures, only returns part of
	*	memory chunk!!! Also does not do any checks.
	*	@param frontBorder memory chunk front border pointer
	*	@param finalSize reference to size, which should remaining chunk have. This value is
	*		alligned(up) to page size and thus returned via reference.
	*	@return TRUE if succesful, FALSE otherwise (function might not be implemented)
	*	@note this is NOT final implementation (dummy implementation has no sense :) )
	*/
	virtual bool reduceChunk(BlockFooter * frontBorder, size_t & finalSize, size_t originalSize)
	{
		return false;
	}

	/** @brief lock for allocator synchronisation
	*
	*	Used for malloc and free. These functions must
	*/
	YieldingSpinLock m_lock;


};
