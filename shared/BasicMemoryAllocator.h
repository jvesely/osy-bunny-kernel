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
 * 	This is base class, from which are derived KernelMemoryAllocator and
 *	UserMemoryAllocator. Class should implement at least four basic memory allocation
 *	strategies, enable to switch them at runtime and resize existing memory.
 *	Allocator now formats memory as a bunch of free/used blocks, which are
 *	organized in (sorted) list.
 */


#pragma once
#include <structures/SimpleList.h>
#include "types.h"
//------------------------------------------------------------------------------
/*	debug/no debug setting
*
*	if defined, debug ssettings will be used: only one frame at a time will be
*	allocated, unless bigger memory piece is needed, also wirtual 'lock' will
*	be used in memory allocation and deallocation, so that recursive allocation
*	problems will be revealed. This setting will not affect interrupt disabling
*	during allocation/deallocation.
*/
//#define BMA_DEBUG


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
	*	See the structure of memory block header.
	*/
	static const uint32_t IDENTIFIER = 0xBA51CAAA;

	/** @brief int aligment of blocks
	*
	*	This is minimal 'data' size in allocated block. Smaller blocks
	*	will be aligned to nearest ALIGMENT multiple.
	*/
	static const uint8_t ALIGMENT = sizeof(native_t);

	/** @brief default size of newly allocated memory from frame allocator
	*
	*	1/2 MB
	*/
	static const size_t DEFAULT_SIZE = 1024 * 512;

	/** @brief minimal ammount of free memory
	*
	*	Allocator will not return physical/vitual memory to frame/vma allocator,
	*	if size of free blocks is less than this value.
	*
	*	1/4 MB
	*/
	static const size_t MIN_FREE_SIZE = 1024 * 256;

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
	*	@note default copy constructor and operator '=' are allowed, but
	*	copied header is not connected to list (must be either normally added to list
	*	or reconnected - for more see SimpleList and SimpleListItem).
	*/
class BlockHeader: public SimpleListItem
	{
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
		inline size_t size() const { return m_size; }

		/** @brief setter for m_size*/
		inline void setSize(size_t size) { m_size = size; }

		/** @brief free state indicator*/
		inline bool isFree() { return m_state == FREE; }

		/** @brief used state indicator*/
		inline bool isUsed() { return m_state == USED; }

		/** @brief border state indicator*/
		inline bool isBorder() { return m_state == BORDER; }

		/** @brief only changes m_state to FREE
		*
		*	Leaves block in inconsistent state
		*/
		inline void setFree() { m_state = FREE; }

		/** @brief only changes m_state to FREE
		*
		*	Leaves block in inconsistent state
		*/
		inline void setUsed() { m_state = USED; }

		/** @brief only changes m_state to BORDER
		*
		*	Leaves block in inconsistent state
		*/
		inline void setBorder() { m_state = BORDER; }

		/** @brief only changes m_state to NULL
		*
		*	Leaves block in inconsistent state
		*/
		inline void setUndefined() { m_state = NULL; }

		/** @brief get block footer*/
		inline BlockFooter * getFooter()
		{
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

		/** @brief no comment (guess :) )
		*
		*	Size of whole block(with header and footer) or whole memory chunk
		*	(if it is border header).
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
	class BlockFooter
	{
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
		inline size_t size() const { return m_size; }

		/** @brief setter for m_size*/
		inline void setSize(size_t size) { m_size = size; }

		/** @brief free state indicator*/
		inline bool isFree() { return m_state == FREE; }

		/** @brief used state indicator*/
		inline bool isUsed() { return m_state == USED; }

		/** @brief border state indicator*/
		inline bool isBorder() { return m_state == BORDER; }

		/** @brief only changes m_state to FREE
		*
		*	Leaves block in inconsistent state
		*/
		inline void setFree() { m_state = FREE; }

		/** @brief only changes m_state to FREE
		*
		*	Leaves block in inconsistent state
		*/
		inline void setUsed() { m_state = USED; }

		/** @brief only changes m_state to BORDER
		*
		*	Leaves block in inconsistent state
		*/
		inline void setBorder() { m_state = BORDER; }

		/** @brief only changes m_state to NULL
		*
		*	Leaves block in inconsistent state
		*/
		inline void setUndefined() { m_state = 0; }

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
		*	Size of whole block(with header and footer) or whole memory chunk
		*	(if it is border header).
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
	virtual void* getMemory( size_t amount );

	/*! @brief returns used block to the heap
	 *
	 * Marks Block as free and merges it with adjecent free blocks
	 * (if there are any), checks whether address is from given chunk.
	 * @param address of the returned block
	 */
	virtual void freeMemory( const void* address );

	/** @brief frees all allocated memory and returns it to frame allocator
	*
	*	Should be called only at the end of application (process).
	*/
	void freeAll();

	/** @brief getter for m_freeSize
	*
	*	Debug function
	*/
	inline size_t getFreeSize() const { return m_freeSize; }

		/** @brief sets strategy to default
	*
	*	Changes pointer to functions only. Is not dependant on any free block order,
	*	therefore does not rearrange free blocks.
	*	Changes pointers getFreeBlockFunction, setSizeFunction and
	*	insertIntoFreeListFunction.
	*/
	inline void setStrategyDefault();

	/** @brief set strategy to FirstFit
	*
	*	@todo change : must use sorted lists
	*	Changes pointers getFreeBlockFunction, setSizeFunction and
	*	insertIntoFreeListFunction.
	*	For setSizeFunction is enough to use setSizeDefault.
	*	Inserting into list of free blocks has no effect on allocator work,
	*	so it is enough to use default implementation.
	*	getFreeBlockFunction must search blocks according to their addresses.
	*	This strategy does not use blockLists at all, it uses list of memory chunks
	*	and iterates trough blocks sequentially as they are stored in memory chunk.
	*	On the other hand, it must maintains consistency of free/used memory blocks lists.
	*/
	void setStrategyFirstFit();

	/** @brief set strategy to next fit
	*
	*	@todo change : must use sorted lists
	*	Changes pointers getFreeBlockFunction, setSizeFunction and
	*	insertIntoFreeListFunction according to next fit strategy.
	*	Next fit strategy has the same principle as the first it strategy,
	*	except that it finds 'second' fit.
	*	This strategy again sequentialy iterates trough blocks in chunks and therefore does
	*	not use memory blocks lists, but must maintain them.
	*	Inserting into list of free blocks has no effect on allocator work,
	*	so it is enough to use default implementation.
	*	getFreeBlockFunction must iterate blocks according to their addresses and if
	*	possible, must return second suitable block.
	*	For setSizeFunction is enough to use setSizeDefault.
	*/
	void setStrategyNextFit();

	/** @brief set strategy to best fit
	*
	*	Changes pointers getFreeBlockFunction, setSizeFunction and
	*	insertIntoFreeListFunction according to best fit strategy and sorts
	*	list of free blocks.
	*	Strategy searches for smallest block bigger or equal to required size.
	*	Iterating trough blocks is the same as in first fit or default strategy,
	*	blocks are sorted according to their sizes. Resized blocks must be reintegrated
	*	into list.
	*	@todo
	*/
	void setStrategyBestFit();

	/** @brief set strategy to worst fit
	*
	*	Changes pointers getFreeBlockFunction, setSizeFunction and
	*	insertIntoFreeListFunction according to best fit strategy and sorts
	*	list of free blocks.
	*	Iterating trough blocks is reverse to the iterating in first fit or default strategy,
	*	blocks are sorted according to their sizes. Resized blocks must be reintegrated
	*	into list.
	*/
	void setStrategyWorstFit();

protected:
	/** @brief get some memory from frame allocator
	*
	*	Gets memory block and inserts it into list of free blocks. Allocates
	*	DEFAULT_SIZE sized block(plus block header and footer), except cases
	*	when larger block is needed.
	*	@return adress of new block (block header). If not successfull,
	*	NULL is returned.
	*	@param realSize minimum size of new block. Usually should not affect the
	*	size of new block, because default size is greater (1/2 MB).
	*/
	BlockHeader * getBlock(size_t realSize);

	/** @brief get brand new chunk of memory
	*
	*	Virtual function responsible for getting memory from either frame or vma allocator.
	*	@note does not insert new chunk to list, nor creates structures on new memory
	*	@param finalSize pointer to size of required memory. Function alligns this value according to
	*		page size (and returns final size of chunk via pointer).
	*	@return if successfull, returns address of new memory chunk, else NULL.
	*	Return address is virtual(not HW) address.
	*/
	virtual void * getNewChunk(size_t * finalSize) = 0;

	/** @brief extend existing chunk of memory
	*
	*	Virtual function responsible for extending existing memory chunk. Result should be,
	*	that new piece of usable(yet unformatted) memory is right behind old chunk of memory.
	*	@param oldChunk old chunk FOOTER pointer
	*	@param finalSize pointer to required size of new memory.Function alligns this value according to
	*		page size (and returns final size of chunk via pointer).
	*	@return true if success, false otherwise
	*	@note Although there is almost no way to get resultant finalSize (alligned to
	*	actual page size), this can be ignored and structures will be correctly
	*	created.
	*/
	virtual bool extendExistingChunk(BlockFooter * oldChunk, size_t * finalSize, size_t originalSize)
	{
		return NULL;
	}

	/** @brief return memory chunk around given block if needed
	*
	*	If condition for returning whole memory chunk is met (like there will be left enough
	*	free memory etc.) then correctly disconnects only one block in chunk from block list,
	*	removes chunk from chunk list, reduces free and total size values and does all the
	*	other logic behind such operation. Calls returnChunk member function.
	*
	*	If such condition is not met, only ensures, that block is free.
	*	If block is not the only one in memory chunk, does nothing.
	*	@param header blockheader, block should be only one in his memory chunk
	*	@return TRUE if any operation was performed (block was freed, or chunk returned),
	*		FALSE otherwise(block was not the only one in chunk)
	*/
	bool returnChunkWithBlockIfNeeded(BlockHeader * header);

	/** @brief reduce memory chunk with all the logic behind if needed
	*
	*	If conditions for shrinking memory chunk is met, then correctly resizes memory chunk,
	*	in which is given block. Block is expected to be free, and last one in chunk.
	*	Calls reduceChunk function.
	*	If such condition is not met, or such operation was not succesfull, does nothing.
	*	If block is not the last one in chunk, nothing happens as well, and the same pays if
	*	block is not free.
	*	@param header blockheader
	*	@return TRUE if chunk was shrinked, FALSE otherwise
	*/
	bool reduceChunkWithBlockIfNeeded(BlockHeader * header);

	/** @brief return memory chunk to frame/vma allocator
	*
	*	Returns whole chunk to the frame/vma allocator. No work with allocator
	*	structures.
	*	@note Does not disconnect it from list of chunks.
	*	@param frontBorder front border of returned chunk
	*	@param finalSize size of chunk
	*/
	virtual void returnChunk(BlockFooter * frontBorder, size_t finalSize) = 0;

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
	*	@note Although there is almost no way to get resultant finalSize (alligned to
	*	actual page size), this can be ignored and structures will be correctly
	*	created.
	*/
	virtual bool reduceChunk(BlockFooter * frontBorder, size_t * finalSize, size_t originalSize)
	{
		return false;
	}

	/** @brief return physical memory chunk to frame allocator if possible
	*
	*	Is only conditional wrapper to returnBlock member function.
	*	Checks whether block is only one in chunk of physical memory
	*	from frame allocator.
	*	@param header header of returned block
	*	@return true if block was returned, false otherwise
	*/
	inline bool returnChunkIfPossible(BlockHeader * header)
	{
		assert(!header->isBorder());

		BlockFooter * frontBorder = ((BlockFooter*)header) - 1;
		if (!frontBorder->isBorder()) return false;

		BlockHeader * backBorder = (BlockHeader*)(header->getFooter() + 1);
		if (!backBorder->isBorder()) return false;

		returnChunk(frontBorder,frontBorder->size());
		return true;
	}

	/** @brief try to expand some memory chunk and create correct structures
	*
	*	Tries to expand some existing memory chunk. If it is possible, also creates correct
	*	structures on that chunk and handles it`s integrity with chunk list.
	*	@param totalSize required size. Memory extension will be at least totalSize.
	*	@return newly created/expanded free block if succesfull, NULL otherwise
	*/
	BlockHeader * tryExpandSomeChunk(size_t totalSize);

	/** @brief try to get brand new memory chunk and create correct structures
	*
	*	Tries to get brand new memory chunk. If it is possible, also creates correct structures
	*	on new chunk and inserts it into chunk list.
	*	@param totalSize required size, must be aligned to minimum page size.
	*		Memory extension will be exactly totalSize
	*	@return newly created free block if succesfull, NULL otherwise
	*/
	BlockHeader * tryToGetNewChunk(size_t totalSize);

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
	*	Creates one big FREE block PLUS footer on beginning and header
	*	on end with BORDER status (and size of block + size of header and footer),
	*	so that allocator will not try to use space outside created block.
	*	@param start beginning of new block
	*	@param size size of whole chunk of memory
			resultant block will have realSize = size - size of block header and footer
	*	@return block of header inside used chunk of memory
	*/
	BlockHeader * initChunk(
	    void * start, size_t size);

	/** @brief integrates chunk of memory to neighbouring chunk
	*
	*	Pushes back border of chunk to new position, and reintegrates it in chunk list.
	*	Former back border will be header of newly created free block, which will, if
	*	possible, join with previously last block in memory chunk.
	*	Function expects correct parameters, that means, that behind given memory chunk really
	*	is expected available memory from frame/vma allocator.
	*	@param frontBorder front border of memory chunk
	*	@param backBorder old back border of chunk
	*	@param totalSize size of memory chunk extension (NOT new memory chunk size)
	*	@return pointer to newly created/joined free block
	*/
	BlockHeader * joinChunk(
	    BlockFooter * frontBorder, BlockHeader * backBorder, size_t totalSize);

	/** @brief changes block`s state to used/free and connects it appropriately
	*
	*	Checks if state really has to be changed. If it will be changed, disconnects
	*	from old list and connects to new list of blocks. Also changes state of block
	*	(both in header and footer). If block has undefined state, it is not removed from
	*	list (is state-safe).
	*	No checking is performed. Expects that list is locked.
	*
	*	Keep in ming, that some strategies requires, that after each correct blockfooter
	*	follows correct header!!
	*
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

	/** @brief receives front chunk border from information in back border
	*
	*	Uses position of back border and size information. No checking is performed.
	*	@note Chunk front border is of type BlockFooter and back border of type
	*	BlockHeader.
	*/
	inline BlockFooter * frontFromBackBorder(BlockHeader * backBorder) const{
		return (BlockFooter*)(
			(uintptr_t)backBorder - backBorder->size() + sizeof(BlockHeader));
	}

	//--------------strategy dependant functions-----

	/** @brief default memory allocation
	*
	*	Only searches list for first matching free block and returns it.
	*	No checking, no changes in structure.
	*/
	BlockHeader * getFreeBlockDefault(size_t realSize) const;

	/** @brief default memory allocation
	*
	*	Allways takes the last block in list (that means the biggest). If it is
	*	big enough, returns it.
	*/
	BlockHeader * getFreeBlockWorstFit(size_t realSize) const;

	/** @brief next fit memory allocation
	*
	*	Searches trough all memory chunks (if needed). Each chunk is linearly
	*	searched for free block big enough. Second suitable block is returned.
	*	If nothing is found, returns NULL.
	*	@note should be used only with NextFit memory allocation strategy.
	*/
	BlockHeader * getFreeBlockNextFit(size_t realSize) const;

	/** @brief logicaly resizes block
	*
	*	Does not really change size of block, only changes size value in block header,
	*	and on space, where block footer is supposed to be, sets size as well.
	*	Use with care, could destroy structure of list.
	*	Copies state as well (only free or used). Does not handle lists.
	*	@note Does not handle old footer. This might lead to loose of data.
	*/
	void setSizeDefault(BlockHeader * header, size_t realSize);

	/** @brief logically resizes block and reintegrates it into list
	*
	*	Does not really change size of block, only changes size value in block header,
	*	and on space, where block footer is supposed to be, sets size as well.
	*	Use with care, could destroy structure of list.
	*	Copies state as well (only free or used).
	*	Also reintegrates block, if needed, to list of free blocks, so this list is
	*	sorted by block`s size.
	*	@note Does not handle old footer. This might lead to loose of data.
	*/
	void setSizeBestFit(BlockHeader * header, size_t realSize);

	/** @brief Inserts into list of free blocks in default order.
	*
	*	Default order is on the end of list. Header must be in state FREE, this function
	*	will not change it`s state.
	*/
	void insertIntoFreeListDefault(BlockHeader * header);

	/** @brief Inserts into list of free blocks sorted according to address.
	*
	*	Function will insert block into list of free blocks. With first fit and next fit strategy,
	*	this list must be sorted according to address. Header must be in state FREE, this function
	*	will not change it`s state.
	*/
	void insertIntoFreeListFirstFit(BlockHeader * header);

	/** @brief Inserts into list of free blocks sorted according to size.
	*
	*	Function will insert block into list of free blocks. With best fit and worst fit strategy,
	*	this list must be sorted according to size. Header must be in state FREE, this function
	*	will not change it`s state.
	*/
	void insertIntoFreeListBestFit(BlockHeader * header);

	/** @brief sort free blocks list according to address
	*
	*	Resorts list of free blocks according to addresses, in ascending order.
	*	Tries to optimize the process, so in good conditions, it may have linear complexity,
	*	in general case it has quadratic complexity.
	*/
	void sortFreeAddress();

	/** @brief sort free blocks according to their sizes
	*
	*	Resorts list of free blocks according to sizes, in ascending order.
	*	Tries to optimize the process, so in good conditions, it may have linear complexity,
	*	in general case it has quadratic complexity.
	*/
	void sortFreeSize();

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
	*	Function should correctly create BlockFooter for block and ignore old
	*	BlockFooter. If needed, reintegrates it into free blocks list.
	*	Function changes accoring to strategy, sometimes change of size of free block
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

	/** @brief size of total memory from frame/vma allocator
	*
	* 	Is used to decide, whether allocator has enough memory from frame/vma allocator.
	*/
	size_t m_totalSize;

	/** @brief value indicating whether chunk resizing is supported
	*
	*	If false, then allocator will not try to resize memory chunks when returning or
	*	getting new memory from either frame or vma allocator.
	*/
	bool m_chunkResizingEnabled;

	/** @brief list of free blocks
	*/
	SimpleList m_freeBlocks;

	/** @brief list of used blocks
	*
	*	List is used only to deallocate all blocks at the end of process.
	*/
	SimpleList m_usedBloks;

	/** @brief list of chunks of memory
	*
	*	List is used to store memory chunks (in which are stored memory blocks).
	*/
	SimpleList m_chunks;


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
	//(this->*setSizeFunction)(header, realSize);
	setSizeDefault(header,realSize);

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













