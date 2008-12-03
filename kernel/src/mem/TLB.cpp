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

/*----------------------------------------------------------------------------*/
void TLB::flush(){
/* Entry is written from EntryHI, EntryL00, EntryL01
 * and position is in Index
 */
	using namespace Processor;

	/* size of page does not really matter at the beginning */
	reg_write_pagemask(PAGE_4K);

	/* map all to 0, stores pairs */
	reg_write_entrylo0(0);
	reg_write_entrylo1(0);

	/* mask is ff */
	reg_write_entryhi (ASID_MASK);

	/* copy to all */
	for(unsigned int i = 0; i < ENTRY_COUNT; ++i){
		reg_write_index(i);
		TLB_write_index();
	}

}
/*----------------------------------------------------------------------------*/
void TLB::setMapping(
	const uintptr_t virtAddr,	
	const uintptr_t physAddr,	
	const Processor::PageSize pageSize
	) 
{
	using namespace Processor;
	//dprintf("Mapping %x to %x\n", virtAddr, physAddr);
	reg_write_pagemask (pageSize); //set the right pageSize
	reg_write_entryhi (virtAddr & VPN2_MASK & ~pageSize); // set address, ASID = 0

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
			reg_write_entrylo0(0);
			reg_write_entrylo1(0);
			/* set the right pageSize */
			reg_write_pagemask (pageSize); 
		}
	}

	/* construct PFN from given address */
	const unative_t reg_addr_value = ( physAddr & ~(pageSize>>1) & PFN_ADDR_MASK) >> PFN_SHIFT;

	/* choose left/right in the pair, allow writing(Dirty) and set valid */
	if (! (virtAddr & VPN2_MASK & (1 << (MASK_SHIFT - 1) ) )) { //  ends with 1 or 0
		/* left/first */
		reg_write_entrylo0(reg_addr_value | ENTRY_LO_VALID_MASK | ENTRY_LO_DIRTY_MASK);
	} else {
		/* right/second */
		reg_write_entrylo1(reg_addr_value | ENTRY_LO_VALID_MASK | ENTRY_LO_DIRTY_MASK);
	}
	
	if (hit) {
		/* rewrite/update conflicting */
		TLB_write_index();
	}else{
		/* rewrite random as there was no previous */
		TLB_write_random();
	}
}
