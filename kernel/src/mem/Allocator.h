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
 * @brief Memory allocator class.
 *
 * Allocator class declaration, togather with required structures of Header
 * and Footer.
 */

#pragma once

#include "types.h"

/*! @class Allocator Allocator.h "src/mem/Allocator.h"
 * @brief heap allocator class
 *
 * Class initializes heap on the given part of the memory and distributes
 * it into smaller pieces keeping track of free and used parts.
 */
class Allocator {
	/*! @struct BlockHeader Allocator.h "mem/Allocator.h"
	 * @brief Header of memory blocks keeps availability, 
	 * size and some magic. 
	 */
	struct BlockHeader {
		bool free;  /*!< availability */
		size_t size;	/*!< size of the block */
		uint32_t magic; /*!< magic bytes for detecting corruption */
	};
	/*! @struct BlockFooter Allocator.h "mem/Allocator.h"
	 * @brief Footer of memory blocks keeps magic and size.
	 */
	struct BlockFooter {
		uint32_t magic; /*!< magic bytes for detecting corruption */
		size_t size;	/*!< size of the block */
	};

public:
	/*! @brief Initializes given chunk. 
	 *
	 * Initializes heap on given chunk of memory.
	 * Creates one big free block.
	 * @param from first byte of my chunk
	 * @param length size of the chunk
	 */
	void setup(const uintptr_t from, const size_t length);

	/*! @brief Allocates free block of size >= amount.
	 *
	 * Tries to find first big enough free block (first fit).
	 * State of the memory in the returned block is undefined.
	 * @param amount size of requested block
	 * @return pointer to allocated block, NULL on failure
	 */
	void* getMemory(const size_t amount) const;

	/*! @brief Returns used block to the heap.
	 *
	 * Marks Block as free and merges it with adjecent free blocks 
	 * (if there are any), checks whether address is from given chunk.
	 * @param address of the returned block
	 */
	void freeMemory(const void* address) const;

	/*! @brief Checks the status of all (used and free) blocks. */
	bool check();

private:

	/*! @brief starting address of my heap */
	uintptr_t m_start;

	/*! @brief end address of the heap (I can not use it) */
	uintptr_t m_end;

	static const uint8_t MAGIC = 0xAA; /*!< 1 byte magic */
	static const uint32_t BIG_MAGIC = 0xDEADBEEF; /*!< 4 byte magic */
	static const uint8_t ALIGMENT = 4; /*!< int aligment */

	/*! @brief Initializes block of given size and availability at given address.
	 *
	 * Creates header and footer structures representing block. 
	 * Sets size and MAGIC.
	 * @param start start of the block
	 * @param size of the block (including header and footer)
	 * @param free availability of the block
	 */
	void createBlock(
		const uintptr_t start, const size_t size, const bool free )  const;

	/*! @brief Checks the state of the block.
	 *
	 * Checks whether header size == footer size, and MAGIC in both structures.
	 * @param start pointer to the block
	 * @return block status
	 */
	bool checkBlock(const uintptr_t start) const;

	/*! @brief Aligns address to nearest bigger block.
	 * @param address address to be aligned
	 * @param factor align amount should be power of 2
	 * @return aligned address
	 */
	static inline unsigned int alignUp(
		const unsigned int address, const unsigned int factor)
		{ return  (address + (factor - 1) ) & ~(factor - 1); };

	/*! @brief Aligns address to nearest smaller block 
	 * @param address address to be aligned
	 * @param factor align amount should be power of 2
	 * @return aligned address
	 */
	static inline unsigned int alignDown(
		const unsigned int address, const unsigned int factor)
		{ return address & ~(factor - 1); }

};
