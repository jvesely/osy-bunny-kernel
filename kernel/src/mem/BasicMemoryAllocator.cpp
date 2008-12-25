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
 *   @version $Id: BasicMemoryAllocator.cpp 351 2008-12-09 23:29:15Z dekanek $
 *   @note
 *   Semestral work for Operating Systems course at MFF UK \n
 *   http://dsrg.mff.cuni.cz/~ceres/sch/osy/main.php
 *
 *   @date 2008-2009
 */

/*!
 * @file
 * @brief Basic memory allocator implementation.
 *
 * Long description. I would paste some Loren Ipsum rubbish here, but I'm afraid
 * It would stay that way. Not that this comment is by any means ingenious but
 * at least people can understand it.
 */

#include <mem/BasicMemoryAllocator.h>
#include <api.h>
#include <Kernel.h>
#include <InterruptDisabler.h>
#include <tools.h>
#include <../../common/address.h>
#include "mem/FrameAllocator.h"

//------------------------------------------------------------------------------
//debug reports
//all debug messsages
//#define ALLOCATOR_DEBUG_ALL

//debug messages for dividing block
//#define ALLOCATOR_DEBUG_DIVIDE

//debug messages for frame allocation
//#define ALLOCATOR_DEBUG_FRAME

//debug messages about free frame-allocated memory
//#define ALLOCATOR_DEBUG_FREE

//debug messages for joining blocks
//#define ALLOCATOR_DEBUG_JOIN

//other debug messages
//#define ALLOCATOR_DEBUG_OTHER

//information about size
//#define ALLOCATOR_DEBUG_SIZE

//debug messages about used allocation strategy
//#define ALLOCATOR_DEBUG_STRATEGY

#ifndef ALLOCATOR_DEBUG_ALL

#ifndef ALLOCATOR_DEBUG_FREE
#define PRINT_DEBUG_FREE(...)
#else
#define PRINT_DEBUG_FREE(ARGS...) \
	printf("[ ALLOCATOR_DEBUG_FREE ]: "); \
	printf(ARGS);
#endif

#ifndef ALLOCATOR_DEBUG_FRAME
#define PRINT_DEBUG_FRAME(...)
#else
#define PRINT_DEBUG_FRAME(ARGS...) \
	printf("[ ALLOCATOR_DEBUG_FRAME ]: "); \
	printf(ARGS);
#endif

#ifndef ALLOCATOR_DEBUG_JOIN
#define PRINT_DEBUG_JOIN(...)
#else
#define PRINT_DEBUG_JOIN(ARGS...) \
	printf("[ ALLOCATOR_DEBUG_JOIN ]: "); \
	printf(ARGS);
#endif

#ifndef ALLOCATOR_DEBUG_DIVIDE
#define PRINT_DEBUG_DIVIDE(...)
#else
#define PRINT_DEBUG_DIVIDE(ARGS...) \
	printf("[ ALLOCATOR_DEBUG_DIVIDE ]: "); \
	printf(ARGS);
#endif

#ifndef ALLOCATOR_DEBUG_STRATEGY
#define PRINT_DEBUG_STRATEGY(...)
#else
#define PRINT_DEBUG_STRATEGY(ARGS...) \
	printf("[ ALLOCATOR_DEBUG_STRATEGY ]: "); \
	printf(ARGS);
#endif

#ifndef ALLOCATOR_DEBUG_SIZE
#define PRINT_DEBUG_SIZE(...)
#else
#define PRINT_DEBUG_SIZE(ARGS...) \
	printf("[ ALLOCATOR_DEBUG_SIZE ]: "); \
	printf(ARGS);
#endif

#ifndef ALLOCATOR_DEBUG_OTHER
#define PRINT_DEBUG_OTHER(...)
#else
#define PRINT_DEBUG_OTHER(ARGS...) \
	printf("[ ALLOCATOR_DEBUG ]: "); \
	printf(ARGS);
#endif

//if defined debug all
#else
#define PRINT_DEBUG_FREE(ARGS...) \
  printf("[ ALLOCATOR_DEBUG_FREE ]: "); \
  printf(ARGS);

#define PRINT_DEBUG_JOIN(ARGS...) \
  printf("[ ALLOCATOR_DEBUG_JOIN ]: "); \
  printf(ARGS);

#define PRINT_DEBUG_DIVIDE(ARGS...) \
  printf("[ ALLOCATOR_DEBUG_DIVIDE ]: "); \
  printf(ARGS);

#define PRINT_DEBUG_FRAME(ARGS...) \
  printf("[ ALLOCATOR_DEBUG_FRAME ]: "); \
  printf(ARGS);

#define PRINT_DEBUG_SIZE(ARGS...) \
	printf("[ ALLOCATOR_DEBUG_SIZE ]: "); \
	printf(ARGS);

#define PRINT_DEBUG_STRATEGY(ARGS...) \
  printf("[ ALLOCATOR_DEBUG_STRATEGY ]: "); \
  printf(ARGS);

#define PRINT_DEBUG_OTHER(ARGS...) \
  printf("[ ALLOCATOR_DEBUG ]: "); \
  printf(ARGS);
#endif
//------------------------------------------------------------------------------

BasicMemoryAllocator::BasicMemoryAllocator():
		m_freeSize( 0 )
{
#ifdef BMA_DEBUG
	m_mylock = 0;
#endif
	m_freeSize = 0;
	m_totalSize = 0;
	//default allocation strategy
	setStrategyDefault();
}
//------------------------------------------------------------------------------
BasicMemoryAllocator::~BasicMemoryAllocator()
{
	freeAll();
}
//------------------------------------------------------------------------------
void * BasicMemoryAllocator::getMemory(size_t ammount)
{
	//locking structure of memory manager
	InterruptDisabler lock;
#ifdef BMA_DEBUG
	if ((m_mylock)){//debug
		printk("The way is shut. You cannot pass!\n");
	}
	while (m_mylock){};
	m_mylock = 1;
#endif

	//init
	size_t size = alignUp(ammount, ALIGMENT);
	const size_t realSize = size + sizeof(BlockHeader) + sizeof(BlockFooter);

	PRINT_DEBUG_SIZE("need real size block: %x B \n", realSize);

	void * res = NULL;
	BlockHeader * resHeader = NULL;
	//check
	if ( size > m_freeSize ){
		PRINT_DEBUG_FRAME("must get new frame \n");
		//get new frame from frame allocator (in case of user allocator
		//enlarging block).
		//now it is clear that new used block will be at address got from frame
		//allocator: allocator will not search trough other blocks
		resHeader = getBlock( realSize/*= real size of minimal allocated block*/ );
		if ( resHeader == NULL )
		{
			printk( "------------OUT OF MEMORY(TOTAL)------------\n" );
#ifdef BMA_DEBUG
			m_mylock = 0;//debug
#endif
			return NULL;
		}
	}

	//finding free block using actual allocation strategy
	resHeader = (this->*getFreeBlockFunction)(realSize);
	//resHeader = getFreeBlockDefault(realSize);

	//now res points either to NULL or to header of free block large enough
	if (resHeader == NULL)//will need new block
	{
		PRINT_DEBUG_FRAME("must get new frame \n");
		resHeader = getBlock( realSize/*= real size of minimal allocated block*/ );
		if (resHeader == NULL)
		{
			printk("------------OUT OF MEMORY------------\n");
#ifdef BMA_DEBUG
			m_mylock = 0;//debug
#endif
			return NULL;
		}
	}

	//now res points only to valid free memory block large enough
	//creating used block in free block (and reconnecting free remaining part)
	resHeader = useFreeBlock(resHeader, realSize);
	PRINT_DEBUG_OTHER("resHeader = %x\n", resHeader);
	res = (void*)((uintptr_t)resHeader + sizeof(BlockHeader));
	PRINT_DEBUG_OTHER("res       = %x\n", res);

#ifdef BMA_DEBUG
	m_mylock = 0;//debug
#endif
	return res;
}


//------------------------------------------------------------------------------
void BasicMemoryAllocator::freeMemory(const void * address)
{
	if(!address) return;//according to specification, this should do nothing
	InterruptDisabler lock;

#ifdef BMA_DEBUG
	if ((m_mylock)){
		printk("The way is shut. You cannot pass!\n");
	}
	while (m_mylock){};
	m_mylock = 1;
#endif

	BlockHeader * header = (BlockHeader*)((uintptr_t)address - sizeof(BlockHeader));
	assert(header);
	assert(header->isUsed());

	//if it is only one block in physical memory chunk return it directly
	if (!returnBlockIfPossible(header))
	{
		header = freeUsedBlock(header);
		size_t blockSize = header->size();
		//still there might be possibility to return the block to frame allocator
		if (returnBlockIfPossible(header))
		{//in such case return it and decrease free memory size
			m_freeSize -= blockSize - sizeof(BlockHeader) - sizeof(BlockFooter);
			PRINT_DEBUG_FREE("free size decreased to %x (returning block)\n", m_freeSize);
		}
	}else{
		PRINT_DEBUG_FREE("free size unchanged on value %x (returning used block)\n", m_freeSize);
	}

#ifdef BMA_DEBUG
	m_mylock = 0;//debug
#endif
}
//------------------------------------------------------------------------------
void BasicMemoryAllocator::freeAll()
{
	InterruptDisabler lock;
	BlockHeader * header;
	while (!m_usedBloks.empty())
	{
		header = (BlockHeader*)(m_usedBloks.getMainItem()->next);
		freeUsedBlock(header);
		returnBlockIfPossible(header);
	}
}
//------------------------------------------------------------------------------
BasicMemoryAllocator::BlockHeader * BasicMemoryAllocator::getBlock(size_t realSize)
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
	//				&physResult, frameCount, FRAME_SIZE);

	uint resultantCount = MyFrameAllocator::instance().allocateAtKseg0(
	                          &physResult, frameCount, FRAME_SIZE);

	if (resultantCount != frameCount)
	{
		PRINT_DEBUG_FRAME("frame allocator did not return enough\n");
		PRINT_DEBUG_FRAME("expected %d frames, got %d \n", frameCount, resultantCount);
		return NULL;
	}

	start = ADDR_TO_KSEG0((uintptr_t)physResult);
	//end = start + finalSize;

	//BlockHeader * res =	createBlock(start, end - start);
	BlockHeader * res =	createBlock(start, finalSize);
	if (res)
	{
		m_freeSize += res->size() - sizeof(BlockHeader) - sizeof(BlockFooter);
		m_totalSize += res->size() + sizeof(BlockHeader) + sizeof(BlockFooter);
		PRINT_DEBUG_FREE("free size increased to %x (new block)\n", m_freeSize);
		PRINT_DEBUG_FRAME("total size increased to %x \n", m_totalSize);
		//m_firstUsable = end;//intentionally unsafe - to test proper creation
	};
	return res;
}
//------------------------------------------------------------------------------
void BasicMemoryAllocator::returnBlock(BlockHeader * header)
{
	PRINT_DEBUG_FRAME("returning frame \n");
	size_t FRAME_SIZE = 4096;
	size_t finalSize = header->size() + sizeof(BlockFooter) + sizeof(BlockHeader);
	PRINT_DEBUG_SIZE("returning frame of size %x, internal block of size %x\n", finalSize, header->size());

	assert(finalSize == roundUp(finalSize, FRAME_SIZE));

	BlockFooter * frontBorder = (BlockFooter*)((uintptr_t)header - sizeof(BlockFooter));
	BlockHeader * backBorder = (BlockHeader*)((uintptr_t)(header->getFooter()) + sizeof(BlockFooter));

	assert(frontBorder->isBorder());
	assert(backBorder->isBorder());
	assert((size_t)finalSize == (size_t)((uintptr_t)backBorder - (uintptr_t)frontBorder + sizeof(BlockHeader)));
	assert(finalSize == backBorder->size());
	PRINT_DEBUG_FRAME("checks passed\n");

	//how much
	uint frameCount = finalSize / FRAME_SIZE;
	//disconnecting and returning
	header->disconnect();
	//FrameAllocator<7>::instance().frameFree(frontBorder,frameCount,FRAME_SIZE);
	uintptr_t finalAddress = (uintptr_t)frontBorder-(uintptr_t)ADDR_PREFIX_KSEG0;
	//PRINT_DEBUG_FRAME("returning at %x, count %x, fsize %x \n",finalAddress,frameCount,FRAME_SIZE);
	MyFrameAllocator::instance().frameFree((void*)finalAddress, frameCount, FRAME_SIZE);
	m_totalSize -= finalSize;
	PRINT_DEBUG_FRAME("total size decreased to %x \n", m_totalSize);

	PRINT_DEBUG_FRAME("frame returned\n");
}
//------------------------------------------------------------------------------
BasicMemoryAllocator::BlockHeader * BasicMemoryAllocator::createBlock(
    uintptr_t start, size_t size)
{
	assert(size > 2*(sizeof(BlockHeader) + sizeof(BlockFooter)));
	//real size of free block
	size_t realSize = size - sizeof(BlockHeader) - sizeof(BlockFooter);
	//init what is where
	BlockFooter * frontBorder = (BlockFooter*) start;
	BlockHeader * result = (BlockHeader*)(start + sizeof(BlockFooter));
	BlockHeader * backBorder = (BlockHeader*) (start + size - sizeof(BlockHeader));
	//setting borders
	//their state and total size of memory frame
	frontBorder->setBorder();
	frontBorder->setSize(size);
	backBorder->setBorder();
	backBorder->setSize(size);
	//setting resultant block
	PRINT_DEBUG_FRAME("init block with real size %x \n", realSize);
	result->setUndefined();
	initBlock(result, realSize, true);
	assert(result->isFree());
	return result;
}
//------------------------------------------------------------------------------
BasicMemoryAllocator::BlockHeader *
BasicMemoryAllocator::getFreeBlockDefault(size_t realSize)
const
{
	PRINT_DEBUG_STRATEGY("getFreeBlockDefault\n");
	//search initialisation
	BlockHeader* resHeader = NULL;
	BlockHeader* header = (BlockHeader*)(m_freeBlocks.getMainItem()->next);

	//while not found and there is still something to find
	while ((resHeader == NULL) && (header))
	{
		PRINT_DEBUG_OTHER("Testing block at %x, size %d\n", header, header->size());

		if (header->size() >= realSize)
		{ // first fit
			resHeader = header;
		}
		// next block
		header = ( BlockHeader* )( header->next );
		//if header is header of first block (and therefore we cycled trough list)
		//set it to null
		if (((SimpleListItem*)header) == (m_freeBlocks.getMainItem()))
		{
			header = NULL;
		}
	}

	//now res points either to NULL or to header of free block large enough
	return resHeader;
}



//------------------------------------------------------------------------------
BasicMemoryAllocator::BlockHeader * BasicMemoryAllocator::useFreeBlock(
    BasicMemoryAllocator::BlockHeader * header,
    size_t realSize)
{
	assert(header->size() >= realSize);
	assert(header->isFree());

	//if block is too small to create used and free block
	if (header->size() < (realSize + sizeof(BlockHeader) + sizeof(BlockFooter)))
	{
		PRINT_DEBUG_OTHER("using whole block\n");
		setState(header, false);//change state of block to (not free)
		assert(m_freeSize >= header->size() - sizeof(BlockFooter) - sizeof(BlockHeader));
		m_freeSize -= header->size() - sizeof(BlockFooter) - sizeof(BlockHeader) ;
		PRINT_DEBUG_FREE("free size decreased to %x (using block)\n", m_freeSize);
	}
	else
	{//block will be divided into used and the rest
		PRINT_DEBUG_OTHER("dividing block\n");
		divideBlock(header, realSize, false, true);
		assert(m_freeSize >= realSize);
		m_freeSize -= realSize;
		PRINT_DEBUG_FREE("free size decreased to %x (using block)\n", m_freeSize);
	}

	return header;
}
//------------------------------------------------------------------------------
BasicMemoryAllocator::BlockHeader * BasicMemoryAllocator::freeUsedBlock
( BlockHeader * header )
{

	BlockFooter * beforeFooter = (BlockFooter*)((uintptr_t)header - sizeof(BlockFooter));
	BlockHeader * afterHeader =	(BlockHeader*)((uintptr_t)header->getFooter() + sizeof(BlockFooter));

	//increasing free size by ammount of memory inside of block(without header/footer
	m_freeSize += header->size() - sizeof(BlockHeader) - sizeof(BlockFooter);
	PRINT_DEBUG_SIZE("freeing block with size %x \n", header->size());

	//handling block after
	//if there is any
	if ((afterHeader) && (afterHeader->isFree()))//is border safe
	{
		//join blocks: second block is valid
		PRINT_DEBUG_JOIN("after header size = %x \n", afterHeader->size());
		PRINT_DEBUG_JOIN("after footer =      %x \n", afterHeader);
		PRINT_DEBUG_JOIN("after header =      %x \n", afterHeader->getFooter());

		header = joinFreeBlocks( header, afterHeader, false );
		m_freeSize += sizeof(BlockHeader) + sizeof(BlockFooter);
	}

	//handling block before
	//if there is any block before
	if ((beforeFooter) && (beforeFooter->isFree()))//is border safe
	{
		//join blocks: first block is valid
		PRINT_DEBUG_JOIN("before footer size = %x \n", beforeFooter->size());
		PRINT_DEBUG_JOIN("before footer =      %x \n", beforeFooter);
		PRINT_DEBUG_JOIN("before header =      %x \n", beforeFooter->getHeader());

		header = joinFreeBlocks(beforeFooter->getHeader(), header, true);
		m_freeSize += sizeof(BlockHeader) + sizeof(BlockFooter);
	}
	PRINT_DEBUG_FREE("free size increased to %x (freeing block)\n", m_freeSize);

	//if neither of those : insert into list as new block
	if (header->isUsed())
	{
		setState(header, true);
	}
	return header;
}

//------------------------------------------------------------------------------
BasicMemoryAllocator::BlockHeader * BasicMemoryAllocator::divideBlock(
    BasicMemoryAllocator::BlockHeader * header,
    size_t realSize,
    bool free1,
    bool free2 )
{
	//checking
	assert(header);
	assert(header->size() >= (realSize + sizeof(BlockHeader) + sizeof(BlockFooter)));

	PRINT_DEBUG_DIVIDE("Dividing block of size %x to one with size %x \n", header->size(), realSize);

	//creating 2nd block
	BlockHeader * newHeader = (BlockHeader*)((uintptr_t)header + realSize);
	newHeader->setUndefined();
	initBlock(newHeader, header->size() - realSize, free2);

	//setting 1st block
	/*	changing size of 1st block (still has old size):
	*	If header if free and is changed to free block, then resize with reintegration of block
	*	is executed
	*	else it is needed only to change size (reintegration in used blocks list
	*	is not needed, reintegration if block will be moved to used blocks is not needed
	*	and changing state from used to free also integrates block correctly).
	*/
	if ((header->isFree()) && (free1)){
		PRINT_DEBUG_DIVIDE("changing size of free block\n");
		(this->*setSizeFunction)(header, realSize);
	}else{
		PRINT_DEBUG_DIVIDE("changing size without reintegration\n");
		setSizeDefault( header, realSize );
	}

	//changing state of first block
	setState( header, free1 );

	return header;

}
//------------------------------------------------------------------------------
BasicMemoryAllocator::BlockHeader * BasicMemoryAllocator::joinFreeBlocks(
    BasicMemoryAllocator::BlockHeader * first,
    BasicMemoryAllocator::BlockHeader * second,
    bool firstIsValid)
{
	assert(first && second);
	assert(!first->isBorder());
	assert(!second->isBorder());
	assert(first != second);

	PRINT_DEBUG_JOIN("Joining two blocks\n");
	PRINT_DEBUG_SIZE("size of first %x B\n", first->size());
	PRINT_DEBUG_SIZE("size of second %x B\n", second->size());

	//header settings
	if (!firstIsValid)
	{
		assert(second->isFree());
		//disconnecting not valid header
		first->disconnect();
		//copying to new header from valid header
		first->next = second->next;
		first->prev = second->prev;
		first->setFree();
		//disconnecting valid header, which is no more valid
		second->disconnect();
		//connecting new header, which is now valid
		first->reconnect();
	}else{
		assert(first->isFree());
		//disconnect header, which is no more valid
		second->disconnect();
	}
	//join them de jure :)
	//this will also reintegrate block if needed
	(this->*setSizeFunction)(first, first->size() + second->size());
	assert(first->isFree());
	PRINT_DEBUG_SIZE("size of result %x B\n", first->size());
	return first;
}

//------------------------------------------------------------------------------
void BasicMemoryAllocator::setState( BasicMemoryAllocator::BlockHeader * header, bool free )
{
	assert(header);
	assert(!header->isBorder());
	//checking need to change state
	if (free && (header->isFree())) return;
	if (!free && (header->isUsed())) return;

	//disconnect if possible
	if ((header->isFree()) || (header->isUsed()))
		header->disconnect();

	//set whole block state, considering border values as well
	//connecting as well
	if (free)//will be free
	{
		header->setFree();
		header->getFooter()->setFree();
		(this->*insertIntoFreeListFunction)(header);//strategy-dependant insert
	}else//will be used
	{
		header->setUsed();
		header->getFooter()->setUsed();
		m_usedBloks.insert(header);//this is not strategy-dependant function
	}

}
//------------------------------------------------------------------------------
void BasicMemoryAllocator::insertIntoFreeListDefault
(BasicMemoryAllocator::BlockHeader * header)
{
	//for yet unknown reason, any print in this function causes fail in
	//mallocator01 test. do not use any debug print here.
	assert(header);
	assert(!header->isBorder());
	//PRINT_DEBUG_STRATEGY("insertIntoFreeListDefault\n");

	//only inserts at the end of list
	m_freeBlocks.insert(header);
	//printk("experiment print\n");
}

//------------------------------------------------------------------------------
inline void BasicMemoryAllocator::setSizeDefault
(BasicMemoryAllocator::BlockHeader * header, size_t realSize)
{
	PRINT_DEBUG_STRATEGY("setSizeDefault\n");
	assert(!header->isBorder());

	//setting size value
	header->setSize(realSize);
	header->getFooter()->setSize(realSize);
	//set state
	if (header->isFree()){
		header->getFooter()->setFree();
	}else{
		header->getFooter()->setUsed();
	}
}





