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

#include "structures/Trees.h"
#include "mem/VirtualMemoryArea.h"
#include "mem/IVirtualMemoryMap.h"

typedef Trees<VirtualMemoryArea>::SplayTree VirtualMemoryMap;
typedef SplayBinaryNode<VirtualMemoryArea> VirtualMemoryMapEntry;

/**
 * @class VirtualMemory VirtualMemory.h "mem/VirtualMemory.h"
 * @brief Virtual memory.
 *
 * Virtual memory.
 */
class VirtualMemory: public IVirtualMemoryMap
{
public:
	VirtualMemory() {}

	int allocate(void** from, size_t size, unsigned int flags);
	int free(const void* from);

	// translate fnc
	bool translate(void*& address, size_t& frameSize);

	static const size_t MAX_VIRTUAL_ADDRESS = 0x80000000;

protected:
	bool checkIfFree(const void* from, const size_t size);

	static inline bool checkAligned(const void* address, const size_t frameSize);
	static inline bool checkAligned(const size_t size, const size_t frameSize);
	static inline bool checkSizeInSegment(const size_t size, const unsigned int segment);
	static inline bool checkSegment(const void* address, const size_t size);
	static inline unsigned int getSegment(const void* address);
	static inline void* getAddressInSegment(const unsigned int segment);

private:
	VirtualMemoryMap m_virtualMemoryMap;

};

/* --------------------------------------------------------------------- */

inline bool VirtualMemory::checkAligned(const void* address, const size_t frameSize)
{
	return (((size_t)address % frameSize) == 0);
}

/* --------------------------------------------------------------------- */

inline bool VirtualMemory::checkAligned(const size_t size, const size_t frameSize)
{
	// less than MAX, because there is no allocation from address zero
	return ((size % frameSize) == 0) && (size > 0);
}

/* --------------------------------------------------------------------- */

inline bool VirtualMemory::checkSizeInSegment(const size_t size, const unsigned int segment)
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
			ASSERT(NULL == "Bad segment identifier!");
			return false;
	}
}

/* --------------------------------------------------------------------- */

inline bool VirtualMemory::checkSegment(const void* address, const size_t size)
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

inline unsigned int VirtualMemory::getSegment(const void* address)
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

inline void* VirtualMemory::getAddressInSegment(const unsigned int segment)
{
	switch (segment) {
		case VF_AT_KUSEG:
			// FrameAllocator::MAX_FRAME_SIZE which is private
			// the lowest virtual memory (it should be aligned for the used frame size)
			//TODO: choose better way to get this number
			return (void *)16777216;

		case VF_AT_KSEG0:
			return (void *)ADDR_PREFIX_KSEG0;

		case VF_AT_KSEG1:
			return (void *)ADDR_PREFIX_KSEG1;

		case VF_AT_KSSEG:
			return (void *)ADDR_PREFIX_KSSEG;

		case VF_AT_KSEG3:
			return (void *)ADDR_PREFIX_KSEG3;

		default:
			ASSERT(NULL == "Bad segment identifier!");
			return NULL;
	}
}


