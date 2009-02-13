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
#include "api.h"
#include "KernelMemoryAllocator.h"
#include "mem/FrameAllocator.h"
#include "InterruptDisabler.h"
#include "drivers/Processor.h"

//debug messages for frame allocation
//#define ALLOCATOR_DEBUG_FRAME

#ifndef ALLOCATOR_DEBUG_FRAME
#define PRINT_DEBUG_FRAME(...)
#else
#define PRINT_DEBUG_FRAME(ARGS...) \
	printf("[ ALLOCATOR_DEBUG_FRAME ]: "); \
	printf(ARGS);
#endif


void* KernelMemoryAllocator::getMemory( size_t amount )
{
	InterruptDisabler inter;
	return this->BasicMemoryAllocator::getMemory( amount );
}
/*----------------------------------------------------------------------------*/
void KernelMemoryAllocator::freeMemory( const void* address )
{
	InterruptDisabler inter;
	return this->BasicMemoryAllocator::freeMemory( address );
}

//------------------------------------------------------------------------------
void * KernelMemoryAllocator::getNewChunk(size_t* finalSize)
{
	size_t MIN_FRAME_SIZE = Processor::pages[Processor::PAGE_MIN].size;
	//allignment
	(*finalSize) = roundUp( *finalSize, MIN_FRAME_SIZE);

	void * physResult = NULL;
	uint frameCount = (*finalSize) / MIN_FRAME_SIZE;

	uint resultantCount = MyFrameAllocator::instance().allocateAtKseg0(
	                          &physResult, frameCount, MIN_FRAME_SIZE);
	if (resultantCount != frameCount)
	{
		PRINT_DEBUG_FRAME("Frame allocator did not return enough\n");
		PRINT_DEBUG_FRAME("Expected %d frames, got %d \n", frameCount, resultantCount);
		return NULL;
	}
	return (void*)ADDR_TO_KSEG0((uintptr_t)physResult);
}

//------------------------------------------------------------------------------
void KernelMemoryAllocator::returnChunk(BlockFooter * frontBorder, size_t finalSize)
{
	size_t MIN_FRAME_SIZE = Processor::pages[Processor::PAGE_MIN].size;
	//how much
	uint frameCount = finalSize / MIN_FRAME_SIZE;
	uintptr_t finalAddress = (uintptr_t)frontBorder - (uintptr_t)ADDR_PREFIX_KSEG0;

	PRINT_DEBUG_FRAME("Returning at %x, count %x, fsize %x \n", finalAddress, frameCount, MIN_FRAME_SIZE);
	MyFrameAllocator::instance().frameFree((void*)finalAddress, frameCount, MIN_FRAME_SIZE);
}

