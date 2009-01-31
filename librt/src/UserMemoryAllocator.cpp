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

#include "UserMemoryAllocator.h"
/// \todo synchronisation
//#include "synchronization/StupidSpinlockLocker.h"

#include "SysCall.h"
#include "syscallcodes.h"
#include "api.h"

//debug messages for frame allocation
//#define ALLOCATOR_DEBUG_FRAME

#ifndef ALLOCATOR_DEBUG_FRAME
#define PRINT_DEBUG_FRAME(...)
#else
#define PRINT_DEBUG_FRAME(ARGS...) \
	printf("[ ALLOCATOR_DEBUG_FRAME ]: "); \
	printf(ARGS);
#endif


void* UserMemoryAllocator::getMemory( size_t amount )
{
	/// \todo synchronisation
	//StupidSpinlockLocker locker(m_lock);
	return this->BasicMemoryAllocator::getMemory( amount );
}
/*----------------------------------------------------------------------------*/
void UserMemoryAllocator::freeMemory( const void* address )
{
	/// \todo synchronisation
	//StupidSpinlockLocker locker(m_lock);
	return this->BasicMemoryAllocator::freeMemory( address );
}

//------------------------------------------------------------------------------
void * UserMemoryAllocator::getNewChunk(size_t * finalSize)
{
	void * result = NULL;

	//printf("sending parameters: result at %x, finalsize at %x, flag %x \n",&result,finalSize,((VF_AT_KUSEG << VF_AT_SHIFT) | (VF_VA_AUTO << VF_VA_SHIFT)));
	int success = SysCall::vma_alloc(&result,finalSize,((VF_AT_KUSEG << VF_AT_SHIFT) | (VF_VA_AUTO << VF_VA_SHIFT)));
	//printf("result is %d at %x, size %x\n",success,result,*finalSize);//debug

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

