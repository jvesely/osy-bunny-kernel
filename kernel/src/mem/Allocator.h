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
 * Long description. I would paste some Loren Ipsum rubbish here, but I'm afraid
 * It would stay that way. Not that this comment is by any means ingenious but 
 * at least people can understand it. 
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
	/*! Header of the memory block keeps availability, size and some magic */
	struct BlockHeader {
		bool free;
		size_t size;
		uint32_t magic;
	};
	/*! Footer of the memory block keeps magic and size */
	struct BlockFooter {
		uint32_t magic;
		size_t size;
	};
public:
	/*! @brief initializes given chunk */
	void setup(const uintptr_t from, const size_t length);

	/*! @brief returns free block of size >= amount */
	void* getMemory(size_t amount) const;

	/*! @brief returns block to the heap */
	void freeMemory(void* address) const;

private:
	/*! @brief starting address of my heap */
	uintptr_t m_start;
	/*! @brief end address of the heap (I can not use it) */
	uintptr_t m_end;
	static const uint8_t MAGIC = 0xAA; /*!< 1 byte magic */
	static const uint32_t BIG_MAGIC = 0xDEADBEEF; /*!< 4 byte magic */
	static const uint8_t ALIGMENT = 4; /*!< int aligment */

	/*! @brief initializes block of given size and availability at given address */
	void createBlock(
		const uintptr_t start, const size_t size, const bool free) const;
	/*! @brief checks state of the block */
	bool checkBlock(const uintptr_t start) const;

	/*! aligns adress to nearest bigger 4byte block */
	inline uintptr_t alignUp(uintptr_t  address, uint8_t align)
		{ return  (address + (align - 1) ) & ~(align - 1); };
	/*! aligns address to nearest smaller 4byte block */
	inline uintptr_t alignDown(uintptr_t address, uint8_t align)
		{ return address & ~(align - 1); }
};
