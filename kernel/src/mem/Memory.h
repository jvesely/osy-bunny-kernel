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

#pragma once

#include "api.h"
#include "address.h"
#include "tools.h"

#include "drivers/Processor.h"

/**
 * @class Memory Memory.h "mem/Memory.h"
 * @brief Function definition for memory related checks and operations.
 * @note This could be implemented as namespace instead of a class full
 *   of static member functions.
 */
class Memory
{
public:
	/**
	 * Alignment check for address.
	 *
	 * @note Wrapper to the same function with size as parameter.
	 * @param address The address to be checked.
	 * @param pageType The page size (enum value) to check the address alignment.
	 * @return Whether the address is aligned.
	 */
	static bool isAligned(const void* address, const Processor::PageSize pageType);

	/**
	 * Alignment check for size.
	 *
	 * @param size The size to be checked.
	 * @param pageType The page size (enum value) to check the size alignment.
	 * @return Whether the size is aligned.
	 */
	static bool isAligned(const size_t size, const Processor::PageSize pageType);

	/**
	 * Calculate address alignment.
	 *
	 * @note Wrapper to the same function with size as parameter.
	 * @param address Address to examine.
	 * @return The largest page size (enum value) the address is aligned to.
	 */
	static Processor::PageSize isAligned(const void* address);

	/**
	 * Calculate size alignment.
	 *
	 * @param size Size to examine.
	 * @return The largest page size (enum value) the size is aligned to.
	 */
	static Processor::PageSize isAligned(const size_t size);

	/**
	 * Align the address to a bigger page size than it is aligned (align up).
	 *
	 * @note Uses the general alignUp() function.
	 * @param address Reference to the address to be aligned (in/out parameter).
	 * @param pageType The page size (enum value) to align the address to.
	 * @return Whether the alignment was successfull. It can fail, if the new
	 *   address is in a different memory segment.
	 */
	static bool alignUp(void*& address, const Processor::PageSize pageType);

	/**
	 * Return the largest possible frame size that is smaller than the given size.
	 *
	 * @param size The size to be checked.
	 * @return The largest page size (enum value) smaller than the block.
	 */
	static Processor::PageSize getBiggestPage(const size_t size);

	/**
	 * Check if the given address is in the given range.
	 *
	 * @param address Any address.
	 * @param from The lower bound of the range to be checked.
	 * @param size The size of the range. The upper bound is calculated as from + size.
	 * @return Whether the address is greater or equals than from and is less than from + size.
	 */
	static bool isInRange(const void* address, const void* from, const size_t size);

	/**
	 * Get the frame (page) size in bytes. Conversion from the PageSize enum to bytes.
	 *
	 * @param pageType The page size enum value.
	 * @return The real size in bytes.
	 */
	static size_t frameSize(Processor::PageSize pageType);

	/**
	 * Check if the size fits to the segment.
	 *
	 * @param size Size to check the segment for.
	 * @param segment Segment identification (defined as VF_AT_* in flags.h).
	 * @return Whether the segment is big enough.
	 */
	static bool checkSizeInSegment(const size_t size, const unsigned int segment);

	/**
	 * Check if the given block (range) starts and ends in the same segment.
	 *
	 * @param address The starting address of the block.
	 * @param size The size of the block.
	 * @return Whether the block stays in one segment.
	 */
	static bool checkSegment(const void* address, const size_t size);

	/**
	 * Return the segment the given address belongs to.
	 *
	 * @param address The address to check.
	 * @return Segment identifier (defined as VF_AT_* in flags.h) in which is the address.
	 */
	static unsigned int getSegment(const void* address);

	/**
	 * Returns the first usable address in the given segment.
	 *
	 * @note It is not always the starting address of the segment due to convetion
	 *   never start VMA at address 0 if not requested so.
	 * @param segment Segment identification (defined as VF_AT_* in flags.h).
	 * @return The first usable address in the segment.
	 */
	static void* getAddressInSegment(const unsigned int segment);
};

/* --------------------------------------------------------------------- */

inline bool Memory::isAligned(const void* address, const Processor::PageSize pageType)
{
	return isAligned((size_t)address, pageType);
}

/* --------------------------------------------------------------------- */

inline bool Memory::isAligned(const size_t size, const Processor::PageSize pageType)
{
	return ((size % frameSize(pageType)) == 0);
}

/* --------------------------------------------------------------------- */

inline Processor::PageSize Memory::isAligned(const void* address)
{
	return isAligned((size_t)address);
}

/* --------------------------------------------------------------------- */

inline Processor::PageSize Memory::isAligned(const size_t size)
{
	Processor::PageSize page = Processor::PAGE_MAX;

	while (!isAligned(size, page)) {
		if (--page == Processor::PAGE_MIN) break;
	}

	return page;
}

/* --------------------------------------------------------------------- */

inline bool Memory::alignUp(void*& address, const Processor::PageSize pageType)
{
	void* newAddress = (void *)::alignUp((size_t)address, frameSize(pageType));

	if (getSegment(address) != getSegment(newAddress)) return false;

	address = newAddress;
	return true;
}

/* --------------------------------------------------------------------- */

inline Processor::PageSize Memory::getBiggestPage(const size_t size)
{
	if (size <= frameSize(Processor::PAGE_MIN)) return Processor::PAGE_MIN;

	Processor::PageSize ps = Processor::PAGE_MAX;
	while (size < frameSize(ps)) {
		--ps;
	}

	return ps;
}

/* --------------------------------------------------------------------- */

inline bool Memory::isInRange(const void* address, const void* from, const size_t size)
{
	// address equals to from or is bigger than from
	bool result = (size_t)address >= (size_t)from;
	// calculate the ending address
	size_t to = (size_t)from + size;
	// check overflow
	if ((size != 0) && (to <= (size_t)from)) {
		// in case of overflow address always smaller than end
		return result;
	} else {
		// if no overflow, check if address is smaller than the range end
		return result && ((size_t)address < to);
	}
}

/* --------------------------------------------------------------------- */

inline size_t Memory::frameSize(Processor::PageSize pageType)
{
	return Processor::pages[pageType].size;
}

/* --------------------------------------------------------------------- */

inline bool Memory::checkSizeInSegment(const size_t size, const unsigned int segment)
{
	switch (segment) {
		case VF_AT_KUSEG:
			return size <= ADDR_PREFIX_KSEG0;

		case VF_AT_KSEG0:
			return size < (ADDR_PREFIX_KSEG1 - ADDR_PREFIX_KSEG0);

		case VF_AT_KSEG1:
			return size < (ADDR_PREFIX_KSSEG - ADDR_PREFIX_KSEG1);

		case VF_AT_KSSEG:
			return size < (ADDR_PREFIX_KSEG3 - ADDR_PREFIX_KSSEG);

		case VF_AT_KSEG3:
			return size < (~0 - ADDR_PREFIX_KSEG3);

		default:
			ASSERT(!"Bad segment identifier!");
			return false;
	}
}

/* --------------------------------------------------------------------- */

inline bool Memory::checkSegment(const void* address, const size_t size)
{
	// get the segment of the starting address
	size_t startsInSegment = ADDR_PREFIX((size_t)address);
	// get the segment of the last usable byte (without -1 it could be in other segment)
	size_t endsInSegment = ADDR_PREFIX((size_t)address + size - 1);

	// return whether the start & end address is in the same segment
	return (startsInSegment == endsInSegment)
		// or both in USEG which is under KSEG0
		|| ((startsInSegment < ADDR_PREFIX_KSEG0) && (endsInSegment < ADDR_PREFIX_KSEG0));
}

/* --------------------------------------------------------------------- */

inline unsigned int Memory::getSegment(const void* address)
{
	switch (ADDR_PREFIX((size_t)address)) {
		case ADDR_PREFIX_KSEG0:
			return VF_AT_KSEG0;

		case ADDR_PREFIX_KSEG1:
			return VF_AT_KSEG1;

		case ADDR_PREFIX_KSSEG:
			return VF_AT_KSSEG;

		case ADDR_PREFIX_KSEG3:
			return VF_AT_KSEG3;

		default:
			return VF_AT_KUSEG;
	}
}

/* --------------------------------------------------------------------- */

inline void* Memory::getAddressInSegment(const unsigned int segment)
{
	switch (segment) {
		case VF_AT_KUSEG:
			// the lowest non zero virtual memory which is aligned for any frame size
			return (void *)Memory::frameSize(Processor::PAGE_MAX);

		case VF_AT_KSEG0:
			return (void *)ADDR_PREFIX_KSEG0;

		case VF_AT_KSEG1:
			return (void *)ADDR_PREFIX_KSEG1;

		case VF_AT_KSSEG:
			return (void *)ADDR_PREFIX_KSSEG;

		case VF_AT_KSEG3:
			return (void *)ADDR_PREFIX_KSEG3;

		default:
			ASSERT(!"Bad segment identifier!");
			return NULL;
	}
}

