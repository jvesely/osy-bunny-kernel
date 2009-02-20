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
 * @brief TLB class implementation.
 *
 * Implementing member functions for class TLB. For class description 
 * look in header file
 */
#include "TLB.h"
#include "api.h"
#include "InterruptDisabler.h"
#include "tools.h"
#include "mem/IVirtualMemoryMap.h"

//#define TLB_DEBUG

#ifndef TLB_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...) \
  printf("[ TLB DEBUG ]: "); \
  printf(ARGS);
#endif


Processor::PageSize TLB::suggestPageSize(
	size_t chunk_size, uint prefer_size, uint prefer_entries )
{
	PRINT_DEBUG ("Analyzing %u B memory chunk.\n", chunk_size);
	using namespace Processor;
	uint min_res = -1;
	PageSize victor = PAGE_MIN;
	for (PageSize page = PAGE_MIN; ; ++page)
	{
		size_t aligned = roundUp(chunk_size, pages[page].size);
		uint loss = ( (aligned - chunk_size) * 100) / aligned;
		uint count = (aligned / pages[page].size) ;
		uint result = loss * prefer_size + count * prefer_entries;
		PRINT_DEBUG ("%u: %u => %u (%u,%u) %u.\n",
			pages[page].size, chunk_size, aligned, loss, count, result);
		if (result < min_res) {
			min_res = result;
			victor = page;
		}
		if (page == PAGE_MAX) break;
	}
	PRINT_DEBUG ("Suggest pages of size: %u.\n", pages[victor].size);
	return victor;
}
/*----------------------------------------------------------------------------*/
TLB::TLB()
{
	flush();

	m_asidMap[0] = NULL;
	m_asidMap[ASID_COUNT -1] = NULL;
	for ( byte i = 1; i < ASID_COUNT - 1; ++i) {
		m_asidMap[i] = NULL;
		m_freeAsids.append( i );
	}
}
/*----------------------------------------------------------------------------*/
void TLB::flush()
{
	/* Entry is written from EntryHI, EntryL00, EntryL01
 	 * and position is in Index
   */
	using namespace Processor;
	InterruptDisabler interrupts;

	/* size of page does not really matter at the beginning */
	reg_write_pagemask( pages[PAGE_MIN].mask );

	/* map all to 0, stores pairs */
	reg_write_entrylo0( 0 );
	reg_write_entrylo1( 0 );

	/* mask is ff */
	reg_write_entryhi ( 0xff );

	/* copy to all */
	for (uint i = reg_read_wired(); i < ENTRY_COUNT; ++i) {
		reg_write_index( i );
		TLB_write_index();
	}
}
/*----------------------------------------------------------------------------*/
void TLB::clearAsid( const byte asid )
{
	InterruptDisabler interrupts;

	ASSERT (asid < 255);

	PRINT_DEBUG ("Clearing ASID: %d.\n", asid);

	using namespace Processor;

	native_t old_asid = reg_read_entryhi();
  
	for (uint i = 0; i < ENTRY_COUNT; ++i) {
    reg_write_index( i );
		TLB_read();	
		if ((reg_read_entryhi() & ASID_MASK) == asid) {
			reg_write_entryhi ( 0xff );
			reg_write_pagemask( pages[PAGE_MIN].mask );
			reg_write_entrylo0( 0 );
			reg_write_entrylo1( 0 );
    	TLB_write_index();
		}
  }
	reg_write_entryhi( old_asid );
}
/*----------------------------------------------------------------------------*/
void TLB::switchAsid( byte asid )
{
	PRINT_DEBUG ("Switching ASID to %u.\n", asid);
	Processor::reg_write_entryhi( asid & Processor::ASID_MASK );
}
/*----------------------------------------------------------------------------*/
void TLB::mapDevices( uintptr_t physical_address, uintptr_t virtual_address, Processor::PageSize page_size )
{
	using namespace Processor;

	InterruptDisabler interrupts;

	reg_write_index( 0 );
	reg_write_pagemask( pages[page_size].mask << PAGE_MASK_SHIFT );
	reg_write_entryhi (addrToEntryHi( virtual_address, page_size, 0xff ));

	const byte flags = ENTRY_LO_VALID_MASK | ENTRY_LO_DIRTY_MASK | ENTRY_LO_GLOBAL_MASK;

	reg_write_entrylo0( addrToEntryLo( physical_address, page_size, flags, false ) );
	reg_write_entrylo1( addrToEntryLo( physical_address, page_size, flags, true ) );

	TLB_write_index();
	reg_write_wired( 1 );
}
/*----------------------------------------------------------------------------*/
byte TLB::getAsid( IVirtualMemoryMap* map )
{
	PRINT_DEBUG ("Getting new ASID for VMM: %p.\n", map);
	byte new_asid;
	if (m_freeAsids.count()) {
		new_asid = m_freeAsids.getFirst();
		ASSERT (m_asidMap[new_asid] == NULL);

	} else {
	/* 0 and 255 are reserved */
		new_asid = (Processor::reg_read_count() % 254) +1;
		ASSERT (m_asidMap[new_asid] != NULL);
		m_asidMap[new_asid]->setAsid( 0 );
		clearAsid( new_asid );
	}
	PRINT_DEBUG ("VMM: %p got ASID: %d.\n", map, new_asid);
	m_asidMap[new_asid] = map;
	map->setAsid( new_asid );
	
	return new_asid;
}
/*----------------------------------------------------------------------------*/
void TLB::returnAsid( const byte asid )
{
	PRINT_DEBUG ("Returning ASID: %d.\n", asid);
	clearAsid( asid );
	m_freeAsids.append( asid );
	ASSERT (m_asidMap[asid]);
	ASSERT (m_asidMap[asid]->asid() == asid);
	m_asidMap[asid] = NULL;
}
/*----------------------------------------------------------------------------*/
bool  TLB::handleException( Processor::Context* registers )
{
	ASSERT (!"Should never happen.\n");
	return false;
}
/*----------------------------------------------------------------------------*/
void TLB::setMapping(
	const uintptr_t virtual_address, const uintptr_t physical_address,
	const Processor::PageSize page_size, const byte asid
	) 
{
	using namespace Processor;

	const byte old_asid = reg_read_entryhi();

	reg_write_pagemask( pages[page_size].mask << PAGE_MASK_SHIFT );
	reg_write_entryhi (addrToEntryHi( virtual_address, page_size, asid ));

	const byte flags = ENTRY_LO_VALID_MASK | ENTRY_LO_DIRTY_MASK;

	reg_write_entrylo0( addrToEntryLo( physical_address, page_size, flags, false ) );
	reg_write_entrylo1( addrToEntryLo( physical_address, page_size, flags, true ) );

	TLB_write_random();

	reg_write_entryhi( old_asid );
}
/*----------------------------------------------------------------------------*/
bool TLB::refill( Pointer<IVirtualMemoryMap> vmm, native_t bad_addr )
{
#ifdef TLB_DEBUG
	const unative_t start_count = Processor::reg_read_count();	
#endif
	ASSERT (vmm);

	byte asid = vmm->asid();

	ASSERT (asid);
	if (asid == BAD_ASID) return false;

	PRINT_DEBUG ("Refilling virtual address %p ASID: %u.\n", bad_addr, asid);
	
	void* phys_addr = (void*)bad_addr;
	Processor::PageSize page_size;
#ifdef TLB_DEBUG
	const unative_t translate_start = Processor::reg_read_count();
#endif

	bool success = vmm->translate( phys_addr, page_size );

#ifdef TLB_DEBUG
	const unative_t translate_end = Processor::reg_read_count();
	PRINT_DEBUG ("Translation took: %u.\n",
		translate_end - translate_start);
#endif
	PRINT_DEBUG ("Virtual Address %p translated into %p using size %d %s.\n",
		bad_addr, phys_addr, page_size, success ? "OK" : "FAIL");

	if (!success) 
		return false;
#ifdef TLB_DEBUG
	const unative_t map_start = Processor::reg_read_count();
#endif

	setMapping((uintptr_t)bad_addr, (uintptr_t)phys_addr, page_size, asid);
#ifdef TLB_DEBUG
	const unative_t map_end = Processor::reg_read_count();
	PRINT_DEBUG ("Mapping took: %u.\n",
		map_end - map_start);
#endif

#ifdef TLB_DEBUG
	const unative_t end_count = Processor::reg_read_count();
	PRINT_DEBUG ("Start count: %u end count: %u, used cycles: %u.\n",
		start_count, end_count, end_count - start_count);
#endif
	return true;
}
