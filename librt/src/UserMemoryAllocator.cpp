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

#include "librt.h"
#include "UserMemoryAllocator.h"

#include "SysCall.h"
#include "synchronization/SpinlockLocker.h"


//debug messages for vma allocation
//#define ALLOCATOR_DEBUG_FRAME

//debug messages for vma resize
//#define ALLOCATOR_DEBUG_VMA_RESIZE


#ifndef ALLOCATOR_DEBUG_FRAME
#define PRINT_DEBUG_FRAME(...)
#else
#define PRINT_DEBUG_FRAME(ARGS...) \
	printf("[ ALLOCATOR_DEBUG_FRAME ]: "); \
	printf(ARGS);
#endif

#ifndef ALLOCATOR_DEBUG_VMA_RESIZE
#define PRINT_DEBUG_VMA_RESIZE(...)
#else
#define PRINT_DEBUG_VMA_RESIZE(ARGS...) \
	printf("[ ALLOCATOR_DEBUG_VMA_RESIZE ]: "); \
	printf(ARGS);
#endif


//------------------------------------------------------------------------------
UserMemoryAllocator::UserMemoryAllocator():
BasicMemoryAllocator()
{
	m_chunkResizingEnabled = true;
}
//------------------------------------------------------------------------------
void* UserMemoryAllocator::getMemory( size_t amount )
{
	SpinlockLocker locker(&m_lock);
	return this->BasicMemoryAllocator::getMemory( amount );
}
/*----------------------------------------------------------------------------*/
void UserMemoryAllocator::freeMemory( const void* address )
{
	SpinlockLocker locker(&m_lock);
	return this->BasicMemoryAllocator::freeMemory( address );
}
//------------------------------------------------------------------------------
void * UserMemoryAllocator::getNewChunk(size_t * finalSize)
{
	void * result = NULL;

	int success = SysCall::vma_alloc(&result,finalSize,((VF_AT_KUSEG << VF_AT_SHIFT) | (VF_VA_AUTO << VF_VA_SHIFT)));

	if (success!=EOK)
	{
		PRINT_DEBUG_FRAME("vma allocator did not return ok(%d) but %d\n",EOK,success);
		return NULL;
	}
	return (void*)result;
}

//------------------------------------------------------------------------------
void UserMemoryAllocator::returnChunk(BlockFooter * frontBorder, size_t finalSize)
{
	PRINT_DEBUG_FRAME("returning at %x, size %x \n",frontBorder,finalSize);
	int success = SysCall::vma_free(frontBorder);
	if(success!=EOK)
	{
		PRINT_DEBUG_FRAME("ERROR: returning memory to vma alocator not succesful \n");
	}
}
//------------------------------------------------------------------------------
bool UserMemoryAllocator::extendExistingChunk(BlockFooter * oldChunk, size_t * finalSize, size_t originalSize)
{
	PRINT_DEBUG_VMA_RESIZE("resizing chunk at %x, finalSize %x \n",oldChunk,*finalSize);

	int success = SysCall::vma_resize(oldChunk,finalSize);

	if (success!=EOK)
	{
		PRINT_DEBUG_VMA_RESIZE("vma allocator did not return ok(%d) but %d\n",EOK,success);
		return false;
	}
	PRINT_DEBUG_VMA_RESIZE("resized chunk at %x, finalSize %x \n",oldChunk,*finalSize);
	return true;
}
//------------------------------------------------------------------------------
bool UserMemoryAllocator::reduceChunk(BlockFooter * frontBorder, size_t * finalSize, size_t originalSize)
{
	PRINT_DEBUG_VMA_RESIZE("resizing chunk at %x, finalSize %x \n",frontBorder,*finalSize);
	int success = SysCall::vma_resize(frontBorder,finalSize);

	if (success!=EOK)
	{
		PRINT_DEBUG_VMA_RESIZE("Vma allocator was not able to reduce chunk. Something ba happened!\n");
		return false;
	}
	return true;
}








