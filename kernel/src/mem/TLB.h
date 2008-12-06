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
 * @brief TLB class declaration.
 *
 * Header file for TLB class, this class handles all stuff concerning TLB.
 */
#pragma once
#include "drivers/Processor.h"
#include "structures/Buffer.h"

/*!
 * @class TLB mem/TLB.h "mem/TLB.h"
 * @brief TLB handling class.
 *
 * This class should hanle all operations done on TLB, not much so far,
 * but stuff will eventually come.
 */
class TLB
{
public:

	static const uint ASID_COUNT = 256;

	/*! @brief Prepares the TLB, by @a flushing it. */
	TLB();


	/*! @brief setMapping inserts record into TLB.
	 *
	 * Given virtual address will map onto given physical address using
	 * request page size.
	 * @param virtualAddress this will be sirtual part of the mapping pair
	 * @param physicalAddress here it should map to
	 * @param pageSize will use page of this size
	 * @param asid create entry using this ASID
	 */
	void setMapping(
		const uintptr_t virtualAddress, 
		const uintptr_t physicalAddress, 
		const Processor::PageSize pageSize,
		const byte asid,
		const bool global = false
		);

	void clearAsid( const byte asid );

	byte getAsid();

	void returnAsid( const byte asid );

	void mapDevices( uintptr_t physical_address, uintptr_t virtual_address,
		Processor::PageSize page_size );

private:

	/*! @brief Removes all entries from the TLB
	 * Resets whole TLB with invalid 0->0 4KB ASID:ff invalid entries
	 * Inspired by Kalisto
	 */
	void flush();

	inline unative_t addrToPage( uintptr_t address, Processor::PageSize page_size )
		{ return address >> Processor::pages[Processor::PAGE_4K].shift & ~Processor::pages[page_size].mask; }
	
	inline bool isEven( unative_t page, Processor::PageSize page_size )
		{ return !((page >> (Processor::pages[page_size].shift - Processor::pages[Processor::PAGE_4K].shift)) & 1); }

	Buffer<byte, ASID_COUNT> m_freeAsids;
	void* m_asidMap[ASID_COUNT];
	
};
