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
#include "mem/IVirtualMemoryMap.h"
#include "drivers/Processor.h"

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

	int resize(const void* from, const size_t size);
	int remap(const void* from, const void* to);
	int merge(const void* area1, const void* area2);
	int split(const void* from, const void* split);

	// translate fnc
	bool translate(void*& address, Processor::PageSize& frameSize);

	void dump();

protected:
	bool isFree(const void* from, const size_t size);
	void getFreeAddress(void*& from, const size_t size, Processor::PageSize frameType);

private:
	VirtualMemoryMap m_virtualMemoryMap;

};

