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
#include "KernelMemoryAllocator.h"
#include "mem/FrameAllocator.h"
#include "InterruptDisabler.h"

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
/*----------------------------------------------------------------------------*/
BasicMemoryAllocator::BlockHeader * KernelMemoryAllocator::getBlock(size_t realSize)
{
  //expecting that realsize > 0 - otherwise it does not have sense
  assert(realSize);

  //this size will be in future provided by frame allocator
  size_t FRAME_SIZE = 4096;

  uintptr_t start = NULL;

  size_t finalSize = realSize + sizeof(BlockFooter) + sizeof(BlockHeader);

#ifndef BMA_DEBUG
  if (finalSize < DEFAULT_SIZE) finalSize = DEFAULT_SIZE;
#endif

  finalSize = roundUp(finalSize, FRAME_SIZE);
  uint frameCount = finalSize / FRAME_SIZE;
  void * physResult = NULL;

  //uint resultantCount = FrameAllocator<7>::instance().allocateAtKseg0(
  //        &physResult, frameCount, FRAME_SIZE);

  uint resultantCount = MyFrameAllocator::instance().allocateAtKseg0(
                            &physResult, frameCount, FRAME_SIZE);

  if (resultantCount != frameCount)
  {
//    PRINT_DEBUG_FRAME("frame allocator did not return enough\n");
//    PRINT_DEBUG_FRAME("expected %d frames, got %d \n", frameCount, resultantCount);
    return NULL;
  }

  start = ADDR_TO_KSEG0((uintptr_t)physResult);
  //end = start + finalSize;

  //BlockHeader * res = createBlock(start, end - start);
  BlockHeader * res = createBlock(start, finalSize);
  if (res)
  {
    m_freeSize += res->size() - sizeof(BlockHeader) - sizeof(BlockFooter);
    m_totalSize += res->size() + sizeof(BlockHeader) + sizeof(BlockFooter);
//    PRINT_DEBUG_FREE("free size increased to %x (new block)\n", m_freeSize);
//    PRINT_DEBUG_FRAME("total size increased to %x \n", m_totalSize);
    //m_firstUsable = end;//intentionally unsafe - to test proper creation
  };
  return res;
}
/*----------------------------------------------------------------------------*/
void KernelMemoryAllocator::returnBlock(BlockHeader * header)
{
//  PRINT_DEBUG_FRAME("returning frame \n");
  size_t FRAME_SIZE = 4096;
  size_t finalSize = header->size() + sizeof(BlockFooter) + sizeof(BlockHeader);
//  PRINT_DEBUG_SIZE("returning frame of size %x, internal block of size %x\n", finalSize, header->size());

  assert(finalSize == roundUp(finalSize, FRAME_SIZE));

  BlockFooter * frontBorder = (BlockFooter*)((uintptr_t)header - sizeof(BlockFooter));
  BlockHeader * backBorder = (BlockHeader*)((uintptr_t)(header->getFooter()) + sizeof(BlockFooter));

  assert(frontBorder->isBorder());
  assert(backBorder->isBorder());
  assert((size_t)finalSize == (size_t)((uintptr_t)backBorder - (uintptr_t)frontBorder + sizeof(BlockHeader)));
  assert(finalSize == backBorder->size());
//  PRINT_DEBUG_FRAME("checks passed\n");

  //how much
  uint frameCount = finalSize / FRAME_SIZE;
  //disconnecting and returning
  header->disconnect();
  //FrameAllocator<7>::instance().frameFree(frontBorder,frameCount,FRAME_SIZE);
  uintptr_t finalAddress = (uintptr_t)frontBorder-(uintptr_t)ADDR_PREFIX_KSEG0;
  //PRINT_DEBUG_FRAME("returning at %x, count %x, fsize %x \n",finalAddress,frameCount,FRAME_SIZE);
  MyFrameAllocator::instance().frameFree((void*)finalAddress, frameCount, FRAME_SIZE);
  m_totalSize -= finalSize;
//  PRINT_DEBUG_FRAME("total size decreased to %x \n", m_totalSize);

 // PRINT_DEBUG_FRAME("frame returned\n");
}
