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

//#define TLB_DEBUG

#ifndef TLB_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...) \
  printf("[ TLB_DEBUG ]: "); \
  printf(ARGS);
#endif


Processor::PageSize TLB::suggestPageSize(
	size_t chunk_size, uint prefer_size, uint prefer_entries )
{
	using namespace Processor;
	return PAGE_4K;
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
	reg_write_pagemask( pages[PAGE_4K].mask );

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

	PRINT_DEBUG ("Clearing ASID: %d.\n", asid);

	using namespace Processor;

  for (uint i = 0; i < ENTRY_COUNT; ++i) {
    reg_write_index( i );
		TLB_read();	
		if ((reg_read_entryhi() & ASID_MASK) == asid) {
			reg_write_entryhi ( 0xff );
			reg_write_pagemask( pages[PAGE_4K].mask );
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

	reg_write_entryhi (((page << VPN2_SHIFT) & VPN2_MASK) | 0xff); // set address, ASID = 0xff


	unative_t reg_addr_value =
		((frame << PFN_SHIFT) & PFN_ADDR_MASK) | ENTRY_LO_VALID_MASK | ENTRY_LO_DIRTY_MASK | ENTRY_LO_GLOBAL_MASK;
	
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
	PRINT_DEBUG ("Selected ASID: %d.\n", new_asid);
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
	ASSERT (m_asidMap[asid]->asid() == asid);
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

	PRINT_DEBUG ("Mapping %p(%p) to %p(%p) using size %x for ASID %x.\n",
		page << 12, virtAddr, frame << 12, physAddr, pageSize, asid);
	reg_write_pagemask (page_mask); //set the right pageSize

	reg_write_entrylo0( global_flag );
	reg_write_entrylo1( global_flag );

	reg_write_entryhi( ((page << VPN2_SHIFT) & VPN2_MASK) | asid ); // set address, ASID = asid

	/* try find mapping */
	TLB_probe();

	/* read found position or any other if it was not found */
	const native_t index = reg_read_index();

	/* check the hit */
	const bool hit = !(index & PROBE_FAILURE);
	

	if (hit) {
		/* there is such entry */
		TLB_read();
		if (reg_read_pagemask() != (unative_t)pageSize){
			/* page size mismatch =>  invalidate */
			reg_write_entrylo0( global_flag );
			reg_write_entrylo1( global_flag );
			/* set the right pageSize */
			reg_write_pagemask (page_mask); 
		}
	} 

	/* construct PFN from given address */
	const unative_t reg_addr_value = 
		((frame << PFN_SHIFT) & PFN_ADDR_MASK) | ENTRY_LO_VALID_MASK | ENTRY_LO_DIRTY_MASK | global_flag;


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
}
/*----------------------------------------------------------------------------*/
bool TLB::refill(IVirtualMemoryMap* vmm, native_t bad_addr)
{
	PRINT_DEBUG ("Refilling virtual address %p.\n", bad_addr);
	ASSERT (vmm);

	byte asid = vmm->asid();

	ASSERT (asid);

	void* phys_addr = (void*)bad_addr;
	size_t size;
	bool success = vmm->translate( phys_addr, size );

	PRINT_DEBUG ("Virtual Address %p translated into %p %s.\n", bad_addr, phys_addr, success ? "OK" : "FAIL");

	if (!success) 
		return false;

	setMapping((uintptr_t)bad_addr, (uintptr_t)phys_addr, Processor::PAGE_4K, asid);
	return true;

}
