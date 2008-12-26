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
 *   @version $Id: BasicMemoryAllocator.h 351 2008-12-09 23:29:15Z dekanek $
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
 * It would stay that way. Note that this comment is by any means ingenious but
 * at least people can understand it.
 *	\todo needs to be renamed to firstFitMemAllocator...
 */


#pragma once
#include <synchronization/Mutex.h>
#include <synchronization/MutexLocker.h>
#include <structures/SimpleList.h>
//------------------------------------------------------------------------------
/*	debug/no debug setting
*
*	if defined, debug ssettings will be used: only one frame at a time will be
*	allocated, unless bigger memory piece is needed, also wirtual 'lock' will
*	be used in memory allocation and deallocation, so that recursive allocation
*	problems will be revealed. This setting will not affect interrupt disabling
*	during allocation/deallocation.
*/
#define BMA_DEBUG


//------------------------------------------------------------------------------
/** @brief Class responsible for memory allocation
*
*	Allows use of basic memory allocation strategy:
*	first fit, next fit (maybe one best and worst fit).
*	Blocks allocated by this allocator should not be managed by other
*	allocators, because memory allocators may not be compatible.
*
*	It is possible to change allocation strategy during runtime. For this is
*	needed to call function setStrategy....().
*/
class BasicMemoryAllocator
{
public:
	/** @brief identifier of class used in memory blocks
	*
	*	See structure of memory block header.
	*/
	static const uint32_t IDENTIFIER = 0xBA51CAAA;

	/** @brief int aligment of blocks
	*
	*	This is minimal 'data' size in allocated block. Smaller blocks
	*	will be aligned to nearest ALIGMENT multiple.
	*/
	static const uint8_t ALIGMENT = 4;

	/** @brief deafuls size of newly allocated memory from frame allocator
	*
	*	1/2 MB
	*/
	static const size_t DEFAULT_SIZE = 1024 * 512;

	/** @brief block size which is taken from frame allocator
	*
	*	Now implemented allocator uses only 4kB frames.
	*/
	static const size_t DEFAULT_FRAME_SIZE = 4*1024;

	//forward declaration
	class BlockFooter;

	/*---------------- internal class BlockHeader-------------------------*/
	/** @brief class representing memory block header
	*
	*	Contains state, size and pointer on previous and next memory
	*	block(header). Memory blocks are in linked lists of non-fixed size.
	*	There are two lists of memory blocks: free and used.
	*	This will allow to dealloc all used memory, even if not all was
	*	freed by user.
	*	(note that next memory block is not allways neighbour block)
	*/
class BlockHeader: public SimpleListItem{
	public:
		/** @brief value representing 'free' state of block
		*
		*	Value as well stores 'magic' value identifying correct memory
		*	allocator.
		*/
		static const uint32_t FREE = IDENTIFIER;

		/** @brief value representing 'used' state of block
		*
		*	Value as well stores 'magic' value identifying correct memory
		*	allocator.
		*/
		static const uint32_t USED = IDENTIFIER + 1;

		/** @brief value representing 'free' state of border block
		*
		*	Value as well stores 'magic' value identifying correct memory
		*	allocator. If border value is in header, than there is no 'valid' memory
		*	right before this block.
		*/
		static const uint32_t BORDER = IDENTIFIER + 2;

		/** @brief getter for m_size*/
		inline size_t size() const{return m_size;}

		/** @brief setter for m_size*/
		inline void setSize(size_t size){m_size = size;}

		/** @brief free state indicator*/
		inline bool isFree(){return m_state == FREE;}

		/** @brief used state indicator*/
		inline bool isUsed(){return m_state == USED;}

		/** @brief border state indicator*/
		inline bool isBorder(){return m_state == BORDER;}

		/** @brief only changes m_state to FREE
		*
		*	Leaves block in inconsistent state
		*/
		inline void setFree(){m_state = FREE;}

		/** @brief only changes m_state to FREE
		*
		*	Leaves block in inconsistent state
		*/
		inline void setUsed(){m_state = USED;}

		/** @brief only changes m_state to BORDER
		*
		*	Leaves block in inconsistent state
		*/
		inline void setBorder(){m_state = BORDER;}

		/** @brief only changes m_state to NULL
		*
		*	Leaves block in inconsistent state
		*/
		inline void setUndefined(){m_state = NULL;}

		/** @brief get block footer*/
		inline BlockFooter * getFooter(){
			assert(!isBorder());
			return (BlockFooter*)
			       ((uintptr_t)this + m_size - sizeof(BlockFooter));
		}

	protected:
		/** @brief forbidden default constructor
		*
		*	has no reason to be used
		*/
		BlockHeader();

		/** @brief no comment
		*
		*	Size of whole block(with header and footer).
		*/
		size_t m_size;

		/** @brief state of block
		*
		*	Storing as well magic value identyfying correct memory allocator.
		*/
		uint32_t m_state;
	};

	/*---------------- internal class BlockFooter-------------------------*/
	/** @brief class representing block footer
	*
	*	Holds size and state value.
	*/
	class BlockFooter{
	public:
		/** @brief value representing 'free' state of block
		*
		*	Value as well stores 'magic' value identifying correct memory
		*	allocator.
		*/
		static const uint32_t FREE = IDENTIFIER;

		/** @brief value representing 'used' state of block
		*
		*	Value as well stores 'magic' value identifying correct memory
		*	allocator.
		*/
		static const uint32_t USED = IDENTIFIER + 1;

		/** @brief value representing 'free' state of border block
		*
		*	Value as well stores 'magic' value identifying correct memory
		*	allocator. If border value is in footer, than there is no 'valid' memory
		*	right after this block.
		*/
		static const uint32_t BORDER = IDENTIFIER + 2;

		/** @brief getter for m_size*/
		inline size_t size() const{return m_size;}

		/** @brief setter for m_size*/
		inline void setSize(size_t size){m_size = size;}

		/** @brief free state indicator*/
		inline bool isFree(){return m_state == FREE;}

		/** @brief used state indicator*/
		inline bool isUsed(){return m_state == USED;}

		/** @brief border state indicator*/
		inline bool isBorder(){return m_state == BORDER;}

		/** @brief only changes m_state to FREE
		*
		*	Leaves block in inconsistent state
		*/
		inline void setFree(){m_state = FREE;}

		/** @brief only changes m_state to FREE
		*
		*	Leaves block in inconsistent state
		*/
		inline void setUsed(){m_state = USED;}

		/** @brief only changes m_state to BORDER
		*
		*	Leaves block in inconsistent state
		*/
		inline void setBorder(){m_state = BORDER;}

		/** @brief only changes m_state to NULL
		*
		*	Leaves block in inconsistent state
		*/
		inline void setUndefined(){m_state = NULL;}

		/** @brief get block header	*/
		inline BlockHeader * getHeader()
		{
			assert(!isBorder());
			return (BlockHeader*)
			       ((uintptr_t)this - m_size + sizeof(BlockFooter));
		}


	protected:
		/** @brief forbidden constructor
		*/
		BlockFooter();

		/** @brief state of block
		*
		*	Storing as well magic value identyfying correct memory allocator.
		*/
		uint32_t m_state;

		/** @brief no comment
		*
		*	Size of whole block(with header and footer).
		*/
		size_t m_size;

	};
	/* -------------------- BasicMemoryAllocator methods ------------------ */

	/** @brief initialisator of values
	*
	*	Does not get memory from frame allocator. Sets free momeory from
	*	frame allocator to 0, first memory block pointer to null.
	*/
	BasicMemoryAllocator();

	/** @brief clears everything allocated
	*
	*	Does not return memory to frame allcator, because don`t know how...
	*/
	~BasicMemoryAllocator();

	/*! @brief returns free block of size >= amount (first fit)
	 *
	 * 	Finds free block big enough and uses it. If possible, divides it
	 *	into used and free remaining part.
	 * 	@param amount size of requested block
	 * 	@return pointer to allocated block, NULL on failure
	 */
	void* getMemory(size_t ammount);

	/*! @brief returns used block to the heap
	 *
	 * Marks Block as free and merges it with adjecent free blocks
	 * (if there are any), checks whether address is from given chunk.
	 * @param address of the returned block
	 */
	void freeMemory(const void* address);

	/** @brief frees all allocated memory and returns it to frame allocator
	*
	*	Should be called only at the end of application (process).
	*/
	void freeAll();

	/*! aligns adress to nearest bigger 4byte block.
	 * @param address address to be aligned
	 * @param align align amount
	 * @return aligned address
	 */
	static inline unsigned int alignUp(
	    const unsigned int number, const unsigned int factor)
	{ return  (number + (factor - 1) ) & ~(factor - 1); };

	/*! aligns address to nearest smaller 4byte block
	 * @param address address to be aligned
	 * @param align align amount
	 * @return aligned address
	 */
	static inline unsigned int alignDown(
	    const unsigned int number, const unsigned int factor)
	{ return number & ~(factor - 1); }

	/** @brief getter for m_freeSize
	*
	*	Debug function
	*/
	inline size_t getFreeSize() const {return m_freeSize;}


protected:
	/** @brief get some memory from frame allocator
	*
	*	Gets memory block and inserts it into list of free blocks. Allocates
	*	DEFAULT_SIZE sized block(plus block header and footer), except cases
	*	when larger block is needed.
	*	@return adress of new block (block header). If not successfull,
		NULL is returned.
	*	@param realSize minimum size of new block. Usually should not affect the
		size of new block, because default size is greater (1/2 MB).
	*/
	virtual BlockHeader * getBlock(size_t realSize) = 0;

	/** @brief return memory block to frame allocator
	*
	*	Both header after and footer before block must be border types, that means
	*	returned block is only one in chunk of memory given by frame allocator.
	*	Then it returns whole chunk to frame allocator and correctly disconnects block.
	*	Block can be free or used. Function does not handle freeSize, only totalSize.
	*	Block is correctly disconnected from list.
	*	@param header header of returned block
	*/
	virtual void returnBlock(BlockHeader * header) = 0;

	/** @brief return physical memory block to frame allocator if possible
	*
	*	Is only conditional wrapper to returnBlock member function.
	*	Checks whether block is only one in chunk of physical memory
	*	from frame allocator.
	*	@param header header of returned block
	*	@return true if block was returned, false otherwise
	*/
	inline bool returnBlockIfPossible(BlockHeader * header){
		assert(!header->isBorder());
		BlockFooter * frontBorder = (BlockFooter*)((uintptr_t)header - sizeof(BlockFooter));
		BlockHeader * backBorder = (BlockHeader*)((uintptr_t)(header->getFooter()) + sizeof(BlockFooter));
		if(!frontBorder->isBorder()) return false;
		if(!backBorder->isBorder()) return false;
		returnBlock(header);
		return true;
	}


	/** @brief creates used block in free block
	*
	*	Divides free block to used block and free block (or only to used block
	*	if remaining part is too small). Both parts are correctly connected to
	*	lists of used/free blocks.
	*	@return pointer to used block header
	*	@param header pointer on free block header
	*	@param realSize required size + block header and footer sizefreeUsedBlock
	*/
	BlockHeader * useFreeBlock(BlockHeader * header, size_t realSize);

	/** @brief marks used block as free, connects it or joins with neighbours
	*
	*	Given block is marked as free. If is neighbouring with another free block,
	*	they are joined. If header was change, pointers in list are changed. If
	*	there is no free neighbour, block is inserted to list of free blocks.
	*	(that means, that only one of increasing free list size or joining blocks
	*	may happen).
	*	@param header header of freed block
	*	@return header to the (joined) free block
	*/
	BlockHeader * freeUsedBlock(BlockHeader * header);

	/** @brief initializes chunk of memory
	*
	*	Creates one big FREE block PLUS footer on beginning an header
	*	on end with BORDER status (and size of block + size of header and footer),
	*	so that allocator will not try to use space outside created block.
	*	@param start beginning of new block
	*	@param size size of whole chunk of memory
			resultant block will have realSize = size - size of block header and footer
	*	@return block of header inside used chunk of memory
	*/
	BlockHeader * createBlock(
	    uintptr_t start, size_t size);

	/** @brief changes block`s state to used/free and connects it appropriately
	*
	*	Checks if state really has to be changed. If it will be changed, disconnects
	*	from old list and connects to new list of blocks. Also changes state of block
	*	(both in header and footer). If block has undefined state, it is not removed from
	*	list (is state-safe).
	*	No checking is performed. Expects that list is locked.
	*	@param header block header
	*	@param used specifies whether block will be used or free
	*/
	void setState(BlockHeader * header, bool free = false);

	/** @brief divides block into two, which are connected into appropriate lists
	*
	*	Block can be created with state used/free, free is default. Block is divided
	*	into block with size and the rest. No joining of neigbouring free block is
	*	performed, it should not be possible here. Does not change m_freeSize.
	*	@param header header of block to be divided
	*	@param realSize real size of first block
	*	@param free1 true == state of first block is free
	*	@param free2 true == state of second block is free
	*	@return pointer on block header of first block
	*/
	BlockHeader * divideBlock(
	    BlockHeader * header,
	    size_t realSize,
	    bool free1,
	    bool free2);

	/** @brief joins two free blocks
	*
	*	Joins two blocks, header information from 'valid' header is used.
	*	Invalid header is disconnected from list and lost.
	*	Function also expects, that second block is right behind first.
	*	Both block should be in consistent state, independently on firstIsValid parameter.
	*	@param first first block
	*	@param second second block, expected to be right after first one
	*	@param firstIsValid if true, first block header will be used, else second one
	*	@return header of joined block
	*/
	BlockHeader * joinFreeBlocks(BlockHeader * first, BlockHeader * second, bool firstIsValid);

	/** @brief initializes block and inserts it into correct list
	*
	*	Footer is 'created' as well. Should not be called more than once on
	*	one block. Block really should not be used in any list.
	*	@param header block header
	*	@param free true == state of block will be free (else used)(default true)
	*	@param realSize real size of initialised block
	*/
	inline void initBlock(BlockHeader * header, size_t realSize, bool free = true);

	/** @brief sets strategy to default
	*
	*	Changes pointer to functions only. Is not dependant on any free block order,
	*	therefore does not rearrange free blocks.
	*	Changes pointers getFreeBlockFunction, setSizeFunction and
	*	insertIntoFreeListFunction.
	*/
	inline void setStrategyDefault();

	/** @brief default memory allocation
	*
	*	Only searches list for first matching free block and returns it.
	*	No checking, no changes in structure.
	*	@note should be used only with default memory allocation strategy.
	*/
	BlockHeader * getFreeBlockDefault(size_t realSize) const;

	/** @brief logicaly resizes block
	*
	*	Does not really change size of block, only changes size value in block header,
	*	and on space, where block footer is supposed to be, sets size as well.
	*	Use with care, could destroy structure of list.
	*	Copies state as well (only free or used). Does not handle lists.
	*	@note Does not handle old footer. This might lead to loose of data.
	*/
	void setSizeDefault(BlockHeader * header, size_t realSize);

	/** @brief Inserts into list of free list in default order.
	*
	*	Default order is on the end of list. Header must be in state FREE, this function
	*	will not change it`s state.
	*/
	void insertIntoFreeListDefault(BlockHeader * header);

	/** @brief pointer to function which finds and uses block
	*
	*	Function is supposed to find free block big enough, divide it into two (if
	*	possible) mark it as used and insert it into list of used blocks. Please note,
	*	that some strategies such as best fit, requires to reintegrate divided block into
	*	list of free blocks, because of way they are ordered.
	*	@note This function changes according to used allocation strategy.
	*	Should be changed only via changeStrategy...() member function.
	*	Pointer to useMemoryFunction, freeBlockFunction, insertIntoFreeListFunction
	*	and setSizeFunction	and order of free blocks should be changed only trough this
	*	function.
	*	@return found and used memory block
	*/
	BlockHeader * (BasicMemoryAllocator::*getFreeBlockFunction) (size_t) const;

	/** @brief pointer to function which inserts into list of free blocks
	*
	*	This function is used everytime a free block is inserted into list of free blocks.
	*	This depends on allocation strategy: in default strategy, these blocks are not
	*	ordered in any way, byt first fit strategy orders them according to their adress.
	*	@note This function changes according to used allocation strategy.
	*	Should be changed only via changeStrategy...() member function.
	*	Pointer to useMemoryFunction, freeBlockFunction, insertIntoFreeListFunction
	*	and setSizeFunction	and order of free blocks should be changed only trough this
	*	function.
	*	@param block header with free state, not yet connected
	*/
	void (BasicMemoryAllocator::*insertIntoFreeListFunction) (BlockHeader*);

	/** @brief pointer to function which changes size of block
	*
	*	This function is used everytime a block changes it`s size.
	*	Function changes accoring to strategy, sometimes cahnge of size of free block
	*	requires reintegration of block into list of free blocks.
	*	@note This function changes according to used allocation strategy.
	*	Should be changed only via changeStrategy...() member function.
	*	Pointer to useMemoryFunction, freeBlockFunction, insertIntoFreeListFunction
	*	and setSizeFunction	and order of free blocks should be changed only trough this
	*	function.
	*/
	void (BasicMemoryAllocator::*setSizeFunction) (BlockHeader * , size_t );

	/** @brief size of free memory from frame allocator
	*
	*	If is less than size required in malloc operation, new frame is
	*	allocated (user-process manager will first try to enlarge memory block).
	*/
	size_t m_freeSize;

	/** @brief size of total memory from frame allocator
	*
	* 	Is used to decide, whether allocator has enough memory from frame allocator.
	*/
	size_t m_totalSize;

	/** @brief list of free blocks
	*/
	SimpleList m_freeBlocks;

	/** @brief list of used blocks
	*
	*	List is used only to deallocate all blocks at the end of process.
	*/
	SimpleList m_usedBloks;

#ifdef BMA_DEBUG
	/** @brief debug variable substituting lock
	*
	*	Is used to solve problem with program failing if printk used in
	*	insertIntoFreeListDefault function.
	*/
	int m_mylock;
#endif
};



//------------------------------------------------------------------------------
inline void BasicMemoryAllocator::initBlock
(BlockHeader * header, size_t realSize, bool free )
{
	assert(!header->isBorder());

	//setting undefined state
	header->setUndefined();

	//set size (=create footer)
	(this->*setSizeFunction)(header, realSize);

	//set state of block (which handless also footer state)
	setState(header, free);
}
//------------------------------------------------------------------------------
inline void BasicMemoryAllocator::setStrategyDefault()
{
	getFreeBlockFunction = &BasicMemoryAllocator::getFreeBlockDefault;
	insertIntoFreeListFunction = &BasicMemoryAllocator::insertIntoFreeListDefault;
	setSizeFunction = &BasicMemoryAllocator::setSizeDefault;
}













