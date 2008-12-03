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
	/*! @brief Prepares the TLB, by @a flushing it. */
	TLB(){ flush(); }

	/*! @brief Removes all entries from the TLB
	 * Resets whole TLB with invalid 0->0 4KB ASID:ff invalid entries
	 * Inspired by Kalisto
	 */
	void flush();

	/*! @brief setMapping inserts record into TLB.
	 *
	 * Given virtual address will map onto given physical address using
	 * request page size.
	 * @param virtualAddress this will be sirtual part of the mapping pair
	 * @param physicalAddress here it should map to
	 * @param pageSize will use page of this size
	 */
	void setMapping(
		const uintptr_t virtualAddress, 
		const uintptr_t physicalAddress, 
		const Processor::PageSize pageSize
		);
};
