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
#include "Singleton.h"
#include "ExceptionHandler.h"
#include "Pointer.h"

class IVirtualMemoryMap;

/*!
 * @class TLB mem/TLB.h "mem/TLB.h"
 * @brief TLB handling class.
 *
 * This class should hanle all operations done on TLB, not much so far,
 * but stuff will eventually come.
 */
class TLB:public Singleton<TLB>, public ExceptionHandler
{
public:

	static const uint ASID_COUNT = 256;

	static const uint BAD_ASID   = 255;

	/*! @brief Counts best suited page size for the given size. */
	static Processor::PageSize suggestPageSize( 
		size_t chunk_size, uint prefer_entries, uint prefer_size );


	/*! @brief Prepares the TLB, by @a flushing it. */
	TLB();

	bool handleException( Processor::Context* registers );

	/*! @brief Uses input memory map and address to insert its translation.
	 *
	 * Given address is looked up in the VirtualMemoryMap and translation is
	 * inserted into the TLB, if sucessfull.
	 * @param vmm VirtualMemoryMap to search the address in.
	 * @param bad_addr Address to refill.
	 * @return @a true if translation was found and inserrted into the TLB
	 * 		@a false otherwise.
	 */
	bool refill( Pointer<IVirtualMemoryMap> vmm, native_t bad_addr, const bool hit );

	/*! @brief Creates mapping from virtual to physical memory in the TLB..
	 *
	 * Given virtual address will map onto given physical address using
	 * requested page size and asid.
	 * @param virtual_address The page conatining this virtual address will be
	 * 	mapped.
	 * @param physical_address The frame containing this physical address will be 
	 * 	the destination.
	 * @param page_size Use page of this size.
	 * @param asid Create entry using this ASID.
	 * @param global Say true if you wish to make the mapping accessible to all.
	 */
	void setMapping(
		const uintptr_t virtual_address, const uintptr_t physical_address, 
		const Processor::PageSize page_size, const byte asid
	);

	/*!
	 * @brief Gets currently used ASID.
	 * @return Currently used ASID.
	 */
	inline byte currentAsid()
		{ return Processor::reg_read_entryhi() & Processor::ASID_MASK; }
	
	/*!
	 * @brief Clears all entries with the corresponding ASID from the TLB.
	 * @param asid ASID to clear.
	 */
	void clearAsid( const byte asid );

	/*! 
	 * @brief Gets free ASID to use.
	 * @return Free ASID.
	 * If all ASIDs are used one (random) is confiscated and reused.
	 */
	byte getAsid( IVirtualMemoryMap* map);

	/*!
	 * @brief Sets @a asid as free.
	 * @param asid ASID to free.
	 */
	void returnAsid( const byte asid );

	/*!
	 * @brief Switches currently used ASID.
	 * @param asid ASID to switch to.
	 */
	void switchAsid( const byte asid );

	/*!
	 * @brief Maps page with the location of hardware devices.
	 * @param physical_address start of the frame in which 
	 * 	the devices are present.
	 * @param virtual_address start of the page to which the devices are mapped.
	 * @param page_size Size of the used page/frame.
	 */
	void mapDevices( uintptr_t physical_address, uintptr_t virtual_address,
		Processor::PageSize page_size );


	/*! @brief Removes all entries from the TLB
	 * Resets whole TLB with invalid 0->0 4KB ASID:ff invalid entries
	 * Inspired by Kalisto
	 */
	void flush();

private:
	static inline unative_t addrToEntryLo( uintptr_t addr, Processor::PageSize size, byte flags, bool odd)
		{
			return (((addr >> Processor::pages[size].shift) << 1 | odd) << (Processor::pages[size].shift - Processor::pages[Processor::PAGE_MIN].shift) << Processor::PFN_SHIFT) | flags;
		}

	static inline unative_t addrToEntryHi( uintptr_t addr, Processor::PageSize size, byte asid )
		{
			return (addr >> Processor::pages[size].shift) << Processor::pages[size].shift | asid;
		}

	/*! @brief Stores asids that could be assigned to VMM. */
	Buffer<byte, ASID_COUNT> m_freeAsids;

	/*! @brief Map of Assigned ASIDs. */
	IVirtualMemoryMap* m_asidMap[ASID_COUNT];
	
};
