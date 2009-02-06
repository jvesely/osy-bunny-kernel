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
 * @brief Base class of memory allocator
 *
 * Long description. I would paste some Loren Ipsum rubbish here, but I'm afraid
 * It would stay that way. Not that this comment is by any means ingenious but
 * at least people can understand it.
 */

#include "BasicMemoryAllocator.h"
#include <api.h>
#include <tools.h>
#include "address.h"

/*
naming convention for memory sizes:
size, ammount...: just required size
realSize : required size + blockHeader and BlockFooter (size of block)
totalSize : realsize + size of borders ( = + blokHeader + blockFooter)
finalSize : totalSize aligned to min page size
*/

//------------------------------------------------------------------------------
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

#ifdef ALLOCATOR_DEBUG_ALL
#define ALLOCATOR_DEBUG_DIVIDE
#define ALLOCATOR_DEBUG_FRAME
#define ALLOCATOR_DEBUG_FREE
#define ALLOCATOR_DEBUG_JOIN
#define ALLOCATOR_DEBUG_OTHER
#define ALLOCATOR_DEBUG_SIZE
#define ALLOCATOR_DEBUG_STRATEGY
#endif

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
//------------------------------------------------------------------------------

BasicMemoryAllocator::BasicMemoryAllocator():
		m_freeSize( 0 )
{
#ifdef BMA_DEBUG
	m_mylock = 0;
#endif
	m_freeSize = 0;
	m_totalSize = 0;
	m_chunkResizingEnabled = false;

	setStrategyDefault();
	//setStrategyFirstFit();
	//setStrategyNextFit();
	//setStrategyBestFit();
	//setStrategyWorstFit();
}
//------------------------------------------------------------------------------
BasicMemoryAllocator::~BasicMemoryAllocator()
{
	freeAll();
}
//------------------------------------------------------------------------------

void * BasicMemoryAllocator::getMemory(size_t ammount)
{
#ifdef BMA_DEBUG
	if ((m_mylock)) //debug
	{
		printf("The way is shut. You cannot pass!\n");
	}
	while (m_mylock){};
	m_mylock = 1;
#endif
	//init
	const size_t size = alignUp(ammount, ALIGMENT);
	const size_t realSize = size + sizeof(BlockHeader) + sizeof(BlockFooter);

	PRINT_DEBUG_SIZE("need real size block: %x B \n", realSize);

	BlockHeader * resHeader = NULL;
	//check
	if ( size <= m_freeSize )
	{
		//finding free block using actual allocation strategy
		resHeader = (this->*getFreeBlockFunction)(realSize);
	}

	//now res points either to NULL or to header of free block large enough
	if (resHeader == NULL)//will need new block
	{
		PRINT_DEBUG_FRAME("must get new memory piece \n");
		resHeader = getBlock( realSize ); //= real size of minimal allocated block
		if (resHeader == NULL)
		{
			printf("------------OUT OF MEMORY------------\n");
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
	void * res = (void*)((uintptr_t)resHeader + sizeof(BlockHeader));
	PRINT_DEBUG_OTHER("res       = %x\n", res);
#ifdef BMA_DEBUG
	m_mylock = 0;//debug
#endif
	return res;
}

//------------------------------------------------------------------------------
BasicMemoryAllocator::BlockHeader * BasicMemoryAllocator::getBlock(size_t realSize)
{
	//expecting that realsize > 0 - otherwise it does not have sense
	assert(realSize);

	//this size will be in future provided by frame allocator
	size_t totalSize = realSize + sizeof(BlockFooter) + sizeof(BlockHeader);
	BlockHeader * res;

#ifndef BMA_DEBUG
	if (totalSize < DEFAULT_SIZE) totalSize = DEFAULT_SIZE;
#endif

	//try to extend some existing memory chunk
	if (( res = tryExpandSomeChunk(totalSize) ) != NULL)
	{
		return res;
	}
	//else try to get brand new memory chunk
	return tryToGetNewChunk(totalSize);
}

//------------------------------------------------------------------------------
void BasicMemoryAllocator::freeMemory(const void * address)
{
	if (!address) return;//according to specification, this should do nothing
//	InterruptDisabler lock;

#ifdef BMA_DEBUG
	if ((m_mylock))
	{
		printf("The way is shut. You cannot pass!\n");
	}
	while (m_mylock){};
	m_mylock = 1;
#endif

	BlockHeader * header = (BlockHeader*)((uintptr_t)address - sizeof(BlockHeader));

	assert(header);
	assert(header->isUsed());

	if (!returnChunkWithBlockIfNeeded(header))
	{
		//did not return whole chunk
		//must return block and try to reduce chunk if possible
		header = freeUsedBlock(header);
		reduceChunkWithBlockIfNeeded(header);
	}

#ifdef BMA_DEBUG
	m_mylock = 0;//debug
#endif
}

//------------------------------------------------------------------------------
void BasicMemoryAllocator::freeAll()
{
	//InterruptDisabler lock;
	BlockHeader * header;
	BlockFooter * frontBorder;
	/*	while (!m_usedBloks.empty())
		{
			header = (BlockHeader*)(m_usedBloks.getMainItem()->next);
			freeUsedBlock(header);
			returnChunkIfPossible(header);
		}*/
	while (!m_chunks.empty())
	{
		header = (BlockHeader*)(m_chunks.getMainItem()->next);
		frontBorder = (BlockFooter*)(
		                  (uintptr_t)header - header->size() + sizeof(BlockHeader));
		assert(frontBorder->isBorder());
		returnChunk(frontBorder, frontBorder->size());
	}
}
//------------------------------------------------------------------------------
BasicMemoryAllocator::BlockHeader *
BasicMemoryAllocator::tryToGetNewChunk(size_t totalSize)
{
	//size_t FRAME_SIZE = Processor::pages[0].size;
	PRINT_DEBUG_FRAME("trying to get new chunk \n");
	size_t finalSize = totalSize;// roundUp(totalSize, FRAME_SIZE);

	//also get real finalSize
	void * start = getNewChunk(&finalSize);
	PRINT_DEBUG_FRAME("got chunk at %x \n", start);
	if (!start) return NULL;//no memory

	BlockHeader * res = initChunk(start, finalSize);
	m_freeSize += res->size() - sizeof(BlockHeader) - sizeof(BlockFooter);
	m_totalSize += finalSize;//=res->size + sizeof blockheader + blockfooter
	return res;
}

//------------------------------------------------------------------------------
BasicMemoryAllocator::BlockHeader * BasicMemoryAllocator::tryExpandSomeChunk(size_t totalSize)
{
	if (!m_chunkResizingEnabled) return NULL;
	//alignment
	//size_t FRAME_SIZE = Processor::pages[0].size;
	size_t finalSize = totalSize;// roundUp(totalSize, FRAME_SIZE);

	//bear in mind that chunk has footer at the front and header at the end
	BlockFooter * frontBorder = NULL;
	BlockHeader * backBorder = (BlockHeader*)(m_chunks.getMainItem()->next);
	bool success = false;

	//trying to find some chunk to resize
	while ((!success) && (backBorder != m_chunks.getMainItem()))
	{
		frontBorder = (BlockFooter*)((uintptr_t)backBorder - backBorder->size() + sizeof(BlockHeader));

		assert(frontBorder->isBorder());

		//also get real finalSize
		success = extendExistingChunk(frontBorder, &finalSize, frontBorder->size());
		backBorder = (BlockHeader*) backBorder->next;
	}
	//else success = bad and NULL is returned

	if (success)
	{
		//expanding was succesful - joining expanded piece
		PRINT_DEBUG_FRAME("expanding existing chunk was possible\n");
		backBorder = (BlockHeader*) backBorder->prev;
		return joinChunk(frontBorder, backBorder, finalSize);
	}
	//else
	PRINT_DEBUG_FRAME("expanding existing chunk was NOT possible\n");
	return NULL;
}

//------------------------------------------------------------------------------
BasicMemoryAllocator::BlockHeader * BasicMemoryAllocator::initChunk(
    void * start, size_t size)
{
	assert(start);
	assert(size > 2*(sizeof(BlockHeader) + sizeof(BlockFooter)));
	//real size of free block
	const size_t realSize = size - sizeof(BlockHeader) - sizeof(BlockFooter);
	//init what is where

	BlockFooter * frontBorder = (BlockFooter*) start;
	BlockHeader * result = (BlockHeader*)((uintptr_t)start + sizeof(BlockFooter));
	BlockHeader * backBorder = (BlockHeader*)((uintptr_t)start + size - sizeof(BlockHeader));
	//setting borders
	//their state and total size of memory frame
	frontBorder->setBorder();
	frontBorder->setSize(size);

	backBorder->setBorder();
	backBorder->setSize(size);

	//insert into chunk list
	m_chunks.insert(backBorder);

	//setting resultant block
	PRINT_DEBUG_FRAME("init block with real size %x \n", realSize);

	result->setUndefined();
	initBlock(result, realSize, true);
	return result;
}
//------------------------------------------------------------------------------
bool BasicMemoryAllocator::returnChunkWithBlockIfNeeded(BlockHeader * header)
{
	BlockFooter * frontBorder = ((BlockFooter*)header) - 1;
	BlockHeader * backBorder = (BlockHeader*)(header->getFooter() + 1);

	if (!frontBorder->isBorder()) return false;
	if (!backBorder->isBorder()) return false;

	size_t reduceSize = header->size() - sizeof(BlockHeader) - sizeof(BlockFooter);

	//here comes condition, whether to return block or not
	if (m_freeSize > MIN_FREE_SIZE)
	{
		//invalidate structures on memory chunk
		header->disconnect();
		backBorder->disconnect();
		//return chunk
		returnChunk(frontBorder, frontBorder->size());
		//set values
		size_t reduction = reduceSize + 2 * (sizeof(BlockFooter) + sizeof(BlockHeader));
		m_totalSize -= reduction;
		PRINT_DEBUG_FREE("free size unchanged on value %x (returning chunk)\n", m_freeSize);
		PRINT_DEBUG_FRAME("memory chunk was returned. his size was %x \n", reduction);
	}
	else
	{
		//only free the block
		freeUsedBlock(header);
	}
	return true;
}
//------------------------------------------------------------------------------
bool BasicMemoryAllocator::reduceChunkWithBlockIfNeeded(BlockHeader * header)
{
	if (!m_chunkResizingEnabled) return false;

	BlockHeader * backBorder = (BlockHeader*)(header->getFooter() + 1);
	//checking
	if (!backBorder->isBorder()) return false;
	if (!header->isFree()) return false;

	BlockFooter * frontBorder = (BlockFooter*)
	                            ((uintptr_t)backBorder - backBorder->size() + sizeof(BlockHeader));
	//to reduce memory chunk is required:
	//	-enough free memory
	//	-enough free space in reduced chunk to reduce it (has something to do with alignment of adresses)
	size_t expectedPageAlignment = 4096;
	size_t reduceSize = header->size() - sizeof(BlockHeader) - sizeof(BlockFooter);
	if (((m_freeSize - reduceSize) > MIN_FREE_SIZE) &&
	        (header->size() > 2*expectedPageAlignment))
	{
		//aligning new size of memory chunk - we want some free space left
		/*		size_t finalSize  = alignUp(
				                        frontBorder->size() - header->size() + pageAlignment,
				                        pageAlignment);*/
		//this is required, so that there will be reasonable free space left
		size_t finalSize = frontBorder->size() - header->size() + expectedPageAlignment;

		//storing potential new backBorder
		BlockHeader storedBorder(*backBorder);//implicit copy ctor
		//reducing memory chunk - new size returned in finalSize
		if (reduceChunk(frontBorder, &finalSize, frontBorder->size()))
		{
			//if it worked, then structures must be restored
			//setting potential new backBorder - surely is in given block
			backBorder = (BlockHeader*)((uintptr_t)(frontBorder) + finalSize - sizeof(BlockHeader));
			(*backBorder) = storedBorder;//implicit operator =
			size_t reduction = frontBorder->size() - finalSize;
			//chunkend is now valid
			frontBorder->setSize(finalSize);
			backBorder->setSize(finalSize);
			backBorder->reconnect();
			//set shrinked free block to new reduced size
			//(reduction is the same for this block and memory chunk)
			(this->*setSizeFunction)(header, header->size() - reduction);
			m_freeSize -= reduction;
			m_totalSize -= reduction;
			PRINT_DEBUG_FREE("free size decreased to %x (reducing chunk)\n", m_freeSize);
			PRINT_DEBUG_FRAME("memory chunk was reduced by size %x \n", reduction);
			return true;
		}//else nothing to do, old structures remain valid
		return false;
	}//else nothing
	return false;
}

//------------------------------------------------------------------------------
BasicMemoryAllocator::BlockHeader * BasicMemoryAllocator::joinChunk(
    BlockFooter * frontBorder, BlockHeader * backBorder, size_t totalSize)
{
	assert(totalSize > 0);
	assert(frontBorder->isBorder());
	assert(backBorder->isBorder());

	size_t finalSize = frontBorder->size() + totalSize;

	BlockHeader * finalBackBorder = (BlockHeader*)((uintptr_t)(backBorder) + totalSize);
	(*finalBackBorder) = (*backBorder);

	assert(finalBackBorder->isBorder());

	//connecting new header into chunk list
	finalBackBorder->reconnect();
	//setting borders size value
	frontBorder->setSize(finalSize);
	finalBackBorder->setSize(finalSize);

	//creating new free block (resultant) with header on place of backBorder
	//From now on consider backBorder as a BlockHeader of new free block!
	backBorder->setUndefined();
	initBlock(backBorder, totalSize);//free=true

	//extending free and total size allocator is working with
	m_totalSize += totalSize;
	m_freeSize += totalSize - sizeof(BlockHeader) - sizeof(BlockFooter);
	//if possible, join it with neighbour block (= previously last block in chunk)
	BlockFooter * oldLastFooter = (BlockFooter*)((uintptr_t)backBorder - sizeof(BlockFooter));

	//oldLastFooter CANNOT be border or undefined
	assert((oldLastFooter->isFree()) || (oldLastFooter->isUsed()));

	if (oldLastFooter->isFree())
	{
		backBorder = joinFreeBlocks(oldLastFooter->getHeader(), backBorder, true);
		m_freeSize += sizeof(BlockHeader) + sizeof(BlockFooter);
	}
	return backBorder;
}

//------------------------------------------------------------------------------
BasicMemoryAllocator::BlockHeader *
BasicMemoryAllocator::getFreeBlockDefault(size_t realSize)
const
{
	PRINT_DEBUG_STRATEGY("getFreeBlockDefault\n");
	//search initialisation
	if (m_freeBlocks.empty()) return NULL;
	//BlockHeader* resHeader = NULL;
	BlockHeader* header = (BlockHeader*)(m_freeBlocks.getMainItem()->next);

	assert(header != m_freeBlocks.getMainItem());

	//while not found and there is still something to find
	while (header != m_freeBlocks.getMainItem())
	{
		PRINT_DEBUG_OTHER("Testing block at %x, size %d\n", header, header->size());
		assert(header->isFree());

		if (header->size() >= realSize)
		{
			// first fit
			return header;
		}
		// next block
		header = ( BlockHeader* )( header->next );
	}

	//now res points either to NULL or to header of free block large enough
	return NULL;
}

//------------------------------------------------------------------------------
BasicMemoryAllocator::BlockHeader *
BasicMemoryAllocator::getFreeBlockNextFit(size_t realSize)
const
{
	PRINT_DEBUG_STRATEGY("getFreeBlockNextFit\n");

	//find first
	BlockHeader * header = getFreeBlockDefault(realSize);
	if (!header) return NULL;

	BlockHeader * secondHeader = ( BlockHeader* )( header->next );

	//find second :)
	while (secondHeader != m_freeBlocks.getMainItem())
	{
		assert(secondHeader->isFree());
		if (secondHeader->size() >= realSize)
		{
			return secondHeader;
		}
		secondHeader = (BlockHeader*)secondHeader->next;
	}
	//if second suitable block has not been found yet, then there is only one suitable
	return header;
}

//------------------------------------------------------------------------------
BasicMemoryAllocator::BlockHeader *
BasicMemoryAllocator::getFreeBlockWorstFit(size_t realSize)
const
{
	PRINT_DEBUG_STRATEGY("getFreeBlockWorstFit\n");

	if (m_freeBlocks.empty()) return NULL;

	BlockHeader * header = (BlockHeader*)m_freeBlocks.getMainItem()->prev;

	if (header->size() >= realSize)
	{
		return header;
	}
	//else //there is no block big enough
	return NULL;
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
	{
		//block will be divided into used and the rest
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
	assert((uint32_t)afterHeader > (uint32_t)beforeFooter);

	//increasing free size by ammount of memory inside of block(without header/footer
	m_freeSize += header->size() - sizeof(BlockHeader) - sizeof(BlockFooter);
	PRINT_DEBUG_SIZE("freeing block with size %x \n", header->size());

	//handling block after
	//if there is any...
	if (afterHeader->isFree())//is border safe
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
	assert(beforeFooter);
	if (beforeFooter->isFree())//is border safe
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
	*	If header if free and 1st block will be free,
	*	then resize with reintegration of block is executed.
	*	Else it is needed only to change size:
	*		reintegration in used blocks list is not needed
	*		reintegration if block will be moved to used blocks is not needed
	*		changing state from used to free also integrates block correctly
	*/
	if ((header->isFree()) && (free1))
	{
		PRINT_DEBUG_DIVIDE("changing size of free block\n");
		(this->*setSizeFunction)(header, realSize);
	}
	else
	{
		PRINT_DEBUG_DIVIDE("changing size without reintegration\n");
		setSizeDefault( header, realSize );
	}

	//changing state of first block
	setState(header, free1);

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
	assert( (BlockHeader*)(first->getFooter() + 1) == second);

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
		//disconnecting valid header, which will be no more valid
		second->disconnect();
		//connecting new header, which will be now valid again
		first->reconnect();
	}
	else
	{
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
	//and connecting

	if (free)//will be free
	{
		header->setFree();
		header->getFooter()->setFree();
		(this->*insertIntoFreeListFunction)(header);//strategy-dependant insert
	}
	else//will be used
	{
		header->setUsed();
		header->getFooter()->setUsed();
		m_usedBloks.insert(header);//this is not strategy-dependant function
	}

}
//strategy functions (most)
//------------------------------------------------------------------------------
void BasicMemoryAllocator::setStrategyFirstFit()
{
	PRINT_DEBUG_STRATEGY("setStrategyFirstFit\n");
	sortFreeAddress();

	//set function pointers
	getFreeBlockFunction = &BasicMemoryAllocator::getFreeBlockDefault;
	insertIntoFreeListFunction = &BasicMemoryAllocator::insertIntoFreeListFirstFit;
	setSizeFunction = &BasicMemoryAllocator::setSizeDefault;
}

//------------------------------------------------------------------------------
void BasicMemoryAllocator::setStrategyNextFit()
{
	PRINT_DEBUG_STRATEGY("setStrategyFirstFit\n");
	sortFreeAddress();

	//set function pointers
	getFreeBlockFunction = &BasicMemoryAllocator::getFreeBlockNextFit;
	insertIntoFreeListFunction = &BasicMemoryAllocator::insertIntoFreeListFirstFit;
	setSizeFunction = &BasicMemoryAllocator::setSizeDefault;
}

//------------------------------------------------------------------------------
void BasicMemoryAllocator::setStrategyBestFit()
{
	PRINT_DEBUG_STRATEGY("setStrategyBestFit\n");
	sortFreeSize();

	//set function pointers
	getFreeBlockFunction = &BasicMemoryAllocator::getFreeBlockDefault;
	insertIntoFreeListFunction = &BasicMemoryAllocator::insertIntoFreeListBestFit;
	setSizeFunction = &BasicMemoryAllocator::setSizeBestFit;
}

//------------------------------------------------------------------------------
void BasicMemoryAllocator::setStrategyWorstFit()
{
	PRINT_DEBUG_STRATEGY("setStrategyWorstFit\n");
	sortFreeSize();

	//set function pointers
	getFreeBlockFunction = &BasicMemoryAllocator::getFreeBlockWorstFit;
	insertIntoFreeListFunction = &BasicMemoryAllocator::insertIntoFreeListBestFit;
	setSizeFunction = &BasicMemoryAllocator::setSizeBestFit;
}


//------------------------------------------------------------------------------
void BasicMemoryAllocator::insertIntoFreeListDefault
(BasicMemoryAllocator::BlockHeader * header)
{
	PRINT_DEBUG_STRATEGY("insertIntoFreeListDefault\n");
	assert(header);
	assert(!header->isBorder());

	//only inserts at the end of list
	m_freeBlocks.insert(header);
}
//------------------------------------------------------------------------------
void BasicMemoryAllocator::insertIntoFreeListFirstFit
(BasicMemoryAllocator::BlockHeader * header)
{
	PRINT_DEBUG_STRATEGY("insertIntoFreeListFirstFit\n");
	assert(header);
	assert(!header->isBorder());

	if (m_freeBlocks.empty())
	{
		//printf("only insert\n");//debug
		//only inserts
		m_freeBlocks.insert(header);
		return;
	}

	//if it is to be first or last...
	if ((uintptr_t)(m_freeBlocks.getMainItem()->next) > (uintptr_t)header)
	{
		//printf("on begin\n");//debug
		header->prev = m_freeBlocks.getMainItem();
		header->next = m_freeBlocks.getMainItem()->next;
		header->reconnect();
		return;
	}

	if ((uintptr_t)(m_freeBlocks.getMainItem()->prev) < (uintptr_t)header)
	{
		//printf("on end\n");//debug
		header->next = m_freeBlocks.getMainItem();
		header->prev = m_freeBlocks.getMainItem()->prev;
		header->reconnect();
		return;
	}

	//what to do first? find nearest free block or search in list?

	//try to find nearest free block
	/*	keep in mind, that this method is called allways with header, that
	*	might have yet undefinet front neighbour, his back neighbour should be
	*	defined though.
	*/
	BlockHeader * afterHeader = (BlockHeader*)(header->getFooter() + 1);
	while (afterHeader->isUsed())
	{
		afterHeader = (BlockHeader*)(afterHeader->getFooter() + 1);
	}

	assert((afterHeader->isFree()) || (afterHeader->isBorder()));

	if (afterHeader->isFree())
	{
		//printf("found nearest\n");//debug
		header->next = afterHeader;
		header->prev = afterHeader->prev;
		header->reconnect();
		return;
	}

	//find place in free blocks list
	//printf("searching in list\n");//debug
	afterHeader = (BlockHeader*)m_freeBlocks.getMainItem()->next;
	while ((uintptr_t)afterHeader < (uintptr_t)header)
	{
		afterHeader = (BlockHeader*) afterHeader->next;
	}
	//afterHeader must be found : otherwise something bad happened
	assert(afterHeader->isFree());
	header->next = afterHeader;
	header->prev = afterHeader->prev;
	header->reconnect();
}

//------------------------------------------------------------------------------
void BasicMemoryAllocator::insertIntoFreeListBestFit
(BasicMemoryAllocator::BlockHeader * header)
{
	PRINT_DEBUG_STRATEGY("insertIntoFreeListBestFit\n");
	assert(header);
	assert(!header->isBorder());
	BlockHeader * header2;

	if (m_freeBlocks.empty())
	{
		//only inserts at the end of list
		PRINT_DEBUG_OTHER("inserting blocks to empty free list\n");
		m_freeBlocks.insert(header);
		return;
	}

	header2 = (BlockHeader*)m_freeBlocks.getMainItem()->next;
	if (header->size() <= header2->size())
	{
		//at to the begin
		PRINT_DEBUG_OTHER("inserting blocks to begin of free list\n");
		header->prev = m_freeBlocks.getMainItem();
		header->next = header2;
		header->reconnect();
		return;
	}

	header2 = (BlockHeader*)m_freeBlocks.getMainItem()->prev;
	if (header->size() >= header2->size())
	{
		//at to the end
		PRINT_DEBUG_OTHER("inserting blocks to the end of free list\n");
		header->next = m_freeBlocks.getMainItem();
		header->prev = header2;
		header->reconnect();
		return;
	}
	//otherwise, search trough blocks...
	header2 = (BlockHeader*)m_freeBlocks.getMainItem()->prev;
	while (header2->size() > header->size())
	{
		header2 = (BlockHeader*) header2->prev;
	}
	assert(header2->isFree());
	header->prev = header2;
	header->next = header2->next;
	header->reconnect();
}


//------------------------------------------------------------------------------
void BasicMemoryAllocator::setSizeDefault
(BasicMemoryAllocator::BlockHeader * header, size_t realSize)
{
	PRINT_DEBUG_STRATEGY("setSizeDefault\n");
	assert(!header->isBorder());

	//setting size value
	header->setSize(realSize);
	header->getFooter()->setSize(realSize);
	//set state
	if (header->isFree())
	{
		header->getFooter()->setFree();
	}
	else
	{
		header->getFooter()->setUsed();
	}
}
//------------------------------------------------------------------------------
void BasicMemoryAllocator::setSizeBestFit
(BasicMemoryAllocator::BlockHeader * header, size_t realSize)
{
	PRINT_DEBUG_STRATEGY("setSizeBestFit\n");
	assert(!header->isBorder());

	//setting size value
	header->setSize(realSize);
	header->getFooter()->setSize(realSize);
	//set state
	if (header->isFree())
	{
		header->getFooter()->setFree();
		//reintegrate into sorted list
		header->disconnect();
		insertIntoFreeListBestFit(header);
	}
	else
	{
		header->getFooter()->setUsed();
	}
}


//------------------------------------------------------------------------------
void BasicMemoryAllocator::sortFreeAddress()
{
	PRINT_DEBUG_STRATEGY("sorting free list(address)");
	if (m_freeBlocks.empty()) return;
	//checking of blocks
	SimpleList unsorted;
	BlockHeader * header = (BlockHeader*)m_freeBlocks.getMainItem()->next;
	BlockHeader * prevHeader = header;
	header = (BlockHeader*) header->next;
	while (header != m_freeBlocks.getMainItem())
	{
		assert(header->isFree());
		if ((uintptr_t)header < (uintptr_t)prevHeader)
		{
			header->disconnect();
			unsorted.insert(header);
		}
		else
		{
			prevHeader = header;
		}
		header = (BlockHeader*) prevHeader->next;
	}
	//now all sorted blocks should be in m_freeBlocks and unsorted in unsorted...
	/*	all unsorted blocks will be inserted into m_freeBlocks via
	*	insertIntoFreeListFirtFit function, since this does insertin according to address
	*/
	while (!unsorted.empty())
	{
		header = (BlockHeader*)unsorted.getMainItem()->next;
		header->disconnect();
		insertIntoFreeListFirstFit(header);
	}
	//now everything should be sorted..
}

//------------------------------------------------------------------------------
void BasicMemoryAllocator::sortFreeSize()
{
	PRINT_DEBUG_STRATEGY("sorting free list(size)");
	if (m_freeBlocks.empty()) return;
	//checking of blocks
	SimpleList unsorted;
	BlockHeader * header = (BlockHeader*)m_freeBlocks.getMainItem()->next;
	BlockHeader * prevHeader = header;
	header = (BlockHeader*) header->next;
	while (header != m_freeBlocks.getMainItem())
	{
		assert(header->isFree());
		if (header->size() < prevHeader->size())
		{
			header->disconnect();
			unsorted.insert(header);
		}
		else
		{
			prevHeader = header;
		}
		header = (BlockHeader*) prevHeader->next;
	}
	//now all sorted blocks should be in m_freeBlocks and unsorted in unsorted...
	/*	all unsorted blocks will be inserted into m_freeBlocks via
	*	insertIntoFreeListFirtFit function, since this does insertin according to address
	*/
	while (!unsorted.empty())
	{
		header = (BlockHeader*)unsorted.getMainItem()->next;
		header->disconnect();
		insertIntoFreeListBestFit(header);
	}
	//now everything should be sorted..
}



