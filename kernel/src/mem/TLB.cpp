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
	PageSize victor = PAGE_4K;
	for (PageSize page = PAGE_4K; page <= PAGE_16M; page = (PageSize)(page + 1) )
	{
		size_t aligned = roundUp(chunk_size, pages[page].size);
		uint loss = ( (aligned - chunk_size) * 100) / aligned;
		uint count = (aligned / pages[page].size) ;
		uint result = loss * prefer_size + count * prefer_entries;
		PRINT_DEBUG ("%u: %u => %u (%u,%u) %u.\n", pages[page].size, chunk_size, aligned,
			loss, count, result);
		if (result < min_res) {
			min_res = result;
			victor = page;
		}
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
	reg_write_pagemask( PAGE_4K );

	/* map all to 0, stores pairs */
	reg_write_entrylo0( 0 );
	reg_write_entrylo1( 0 );

	/* mask is ff */
	reg_write_entryhi ( 0xff );

	/* copy to all */
	for (uint i = 0; i < ENTRY_COUNT; ++i) {
		reg_write_index( i );
		TLB_write_index();
	}
}
/*----------------------------------------------------------------------------*/
void TLB::clearAsid( const byte asid )
{
	InterruptDisabler interrupts;

	ASSERT (asid < 255);

	using namespace Processor;

  for (uint i = 0; i < ENTRY_COUNT; ++i) {
    reg_write_index( i );
		TLB_read();	
		if ((reg_read_entryhi() & ASID_MASK) == asid) {
			reg_write_entryhi ( 0xff );
			reg_write_pagemask( PAGE_4K );
			reg_write_entrylo0( 0 );
			reg_write_entrylo1( 0 );
    	TLB_write_index();
		}
  }	
}
/*----------------------------------------------------------------------------*/
void TLB::mapDevices( uintptr_t physical_address, uintptr_t virtual_address, Processor::PageSize page_size )
{
	using namespace Processor;

	InterruptDisabler interrupts;

	reg_write_index( 0 );
	reg_write_pagemask( pages[page_size].mask << PAGE_MASK_SHIFT );

	unative_t page        = addrToPage( virtual_address,  page_size);
	unative_t frame       = addrToPage( physical_address, page_size);

	reg_write_entryhi (pageToVPN2( page, 0xff )); // set address, ASID = 0xff

	unative_t reg_addr_value = 
		frameToPFN( frame, ENTRY_LO_VALID_MASK | ENTRY_LO_DIRTY_MASK | ENTRY_LO_GLOBAL_MASK );
	
	if ( isEven(page, page_size) ) {
		reg_write_entrylo0( ENTRY_LO_GLOBAL_MASK );
		reg_write_entrylo1( reg_addr_value );
	} else {
		reg_write_entrylo0( ENTRY_LO_GLOBAL_MASK );
    reg_write_entrylo1( reg_addr_value );
	}

	TLB_write_index();
	reg_write_wired( 1 );
	
}
/*----------------------------------------------------------------------------*/
byte TLB::getAsid()
{
	if (m_freeAsids.count())
		return m_freeAsids.getFirst();
	
	/* 0 and 255 are reserved */
	byte asid = (Processor::reg_read_count() % 254) +1;

	/* take asid from someone */
	// here thre should be taking asid from vmm
	
	/* clear tlb of the old entries */
	clearAsid( asid );
	return asid;
}
/*----------------------------------------------------------------------------*/
void TLB::returnAsid( const byte asid )
{
	clearAsid( asid );
	m_freeAsids.append( asid );
	m_asidMap[asid] = NULL;
}
/*----------------------------------------------------------------------------*/
void TLB::setMapping(
	const uintptr_t virtAddr,	
	const uintptr_t physAddr,	
	const Processor::PageSize pageSize,
	const byte asid,
	bool global
	) 
{
	using namespace Processor;

	unative_t page        = addrToPage( virtAddr, pageSize);
	unative_t frame       = addrToPage( physAddr, pageSize);
	unative_t page_mask   = pages[pageSize].mask << PAGE_MASK_SHIFT;
	unative_t global_flag = global ? ENTRY_LO_GLOBAL_MASK : 0;

	unative_t old_mask    = reg_read_pagemask();

	PRINT_DEBUG ("Mapping %p(%p) to %p(%p) using size %x for ASID %x.\n",
		page << 12, virtAddr, frame << 12, physAddr, pageSize, asid);
	reg_write_pagemask (page_mask); //set the right pageSize

	reg_write_entrylo0( global_flag );
	reg_write_entrylo1( global_flag );

	reg_write_entryhi( pageToVPN2( page, asid ) ); // set address, ASID = asid

	/* try find mapping */
	TLB_probe();

	/* check the hit */
	const bool hit = !(reg_read_index() & PROBE_FAILURE);

	if (hit) {
		/* there is such entry */
		TLB_read();
		if (reg_read_pagemask() != pages[pageSize].mask){
			/* page size mismatch =>  invalidate */
			reg_write_entrylo0( global_flag );
			reg_write_entrylo1( global_flag );
			/* set the right pageSize */
			reg_write_pagemask (page_mask); 
		}
	} 

	/* construct PFN from given address */
	const unative_t reg_addr_value = 
		frameToPFN( frame, ENTRY_LO_VALID_MASK | ENTRY_LO_DIRTY_MASK | global_flag);


	/* choose left/right in the pair, allow writing(Dirty) and set valid */
	if ( isEven(page, pageSize) ) { //  ends with 1 or 0
		/* left/first */
		reg_write_entrylo0( reg_addr_value );
		PRINT_DEBUG ("Writing left entry.\n");
	} else {
		/* right/second */
		reg_write_entrylo1( reg_addr_value );
		PRINT_DEBUG ("Writing right entry.\n");
	}
	
	if (hit) {
		/* rewrite/update conflicting */
		TLB_write_index();
		PRINT_DEBUG ("Rewriting existing at position %u.\n", reg_read_index());
	} else {
		/* rewrite random as there was no previous */
		TLB_write_random();
		PRINT_DEBUG ("Adding entry at position.\n");
	}

	reg_write_pagemask( old_mask );
}
