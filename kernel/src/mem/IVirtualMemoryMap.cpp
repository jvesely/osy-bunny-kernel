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

#include "IVirtualMemoryMap.h"
#include "Kernel.h"

//#define IVMM_DEBUG

#ifndef IVMM_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...)\
  puts("[ VMM DEBUG ]: ");\
	  printf(ARGS);
#endif



void IVirtualMemoryMap::freed()
{
	ASSERT (m_asid);
	Kernel::instance().tlb().clearAsid( m_asid );
}
/*----------------------------------------------------------------------------*/
void IVirtualMemoryMap::switchTo()
{

	if (!m_asid) {
		m_asid = Kernel::instance().tlb().getAsid( this );
	}
	PRINT_DEBUG ("Switching to VMM %p with ASID: %u", this, m_asid);
	Kernel::instance().tlb().switchAsid( m_asid );
}
/*----------------------------------------------------------------------------*/
IVirtualMemoryMap::~IVirtualMemoryMap()
{
	if (m_asid) Kernel::instance().tlb().returnAsid(m_asid);
}
