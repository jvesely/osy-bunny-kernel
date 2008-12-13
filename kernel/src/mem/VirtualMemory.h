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

#include "structures/Trees.h"
#include "mem/VirtualMemoryArea.h"


typedef Trees<VirtualMemoryArea>::SplayTree VirtualMemoryMap;
typedef SplayBinaryNode<VirtualMemoryArea> VirtualMemoryMapEntry;

/**
 * @class VirtualMemory VirtualMemory.h "mem/VirtualMemory.h"
 * @brief Virtual memory.
 *
 * Virtual memory.
 */
class VirtualMemory
{
public:
	VirtualMemory() : m_ownersCount(1) {}

	// VM sharing
	// - operator= & copy constructor
	// - delete
	// - owners count
	//VirtualMemory& operator=(const VirtualMemory& other);

	int allocate(void** from, size_t size, unsigned int flags);
	int free(const void* from);

	// translate fnc
	bool translate(void*& address, size_t& frameSize);

	static const size_t MAX_VIRTUAL_ADDRESS = 0x80000000;

protected:

	bool checkIfFree(const void* from, const size_t size);

	static inline bool checkAligned(const void* address, const size_t frameSize);
	static inline bool checkAligned(const size_t size, const size_t frameSize);

private:
	size_t m_ownersCount;

	VirtualMemoryMap m_virtualMemoryMap;

};

/* --------------------------------------------------------------------- */

inline bool VirtualMemory::checkAligned(const void* address, const size_t frameSize)
{
	return (((size_t)address % frameSize) == 0) && ((size_t)address < VirtualMemory::MAX_VIRTUAL_ADDRESS);
}

/* --------------------------------------------------------------------- */

inline bool VirtualMemory::checkAligned(const size_t size, const size_t frameSize)
{
	// less than MAX, because there is no allocation from address zero
	return ((size % frameSize) == 0) && (size < VirtualMemory::MAX_VIRTUAL_ADDRESS);
}



