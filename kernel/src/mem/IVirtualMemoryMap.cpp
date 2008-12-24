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
#include "InterruptDisabler.h"
#include "tools.h"

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
	PRINT_DEBUG ("Switching to VMM %p with ASID: %u\n", this, m_asid);
	Kernel::instance().tlb().switchAsid( m_asid );
}
/*----------------------------------------------------------------------------*/
IVirtualMemoryMap::~IVirtualMemoryMap()
{
	if (m_asid) Kernel::instance().tlb().returnAsid(m_asid);
}
/*----------------------------------------------------------------------------*/
int IVirtualMemoryMap::copyTo(const void* src_addr, Pointer<IVirtualMemoryMap> dest_map, void* dst_addr, size_t size)
{
		
	
	PRINT_DEBUG ("Copying from VMM: %p to %p. addr: %p toaddr %p, count: %u asid: %u asid:%u .\n",
		this, dest_map.data(), src_addr, dst_addr, size, m_asid, dest_map->asid());

	byte old_asid = Kernel::instance().tlb().currentAsid();
	const size_t BUFFER_SIZE = 512;
	byte buffer[BUFFER_SIZE];
	char* dest = (char*)dst_addr;
	const char* src  = (const char*)src_addr;

	ASSERT (m_asid);

	while (size) {
		InterruptDisabler inter;
		Kernel::instance().tlb().switchAsid( m_asid );
		size_t count = min(BUFFER_SIZE, size);
		PRINT_DEBUG ("First 4B to copy: %x.\n", *src);
		memcpy((void*)buffer, (void*)src, count);
		PRINT_DEBUG ("Copying %uB data %x vs. %x.\n", count, *(uint*)src,*(uint*) buffer);
		PRINT_DEBUG ("Copied from %p to buffer %p count %u.\n", src, buffer, count);
//		Kernel::instance().stop();
		Kernel::instance().tlb().switchAsid( dest_map->asid() );
		memcpy((void*)dest, (void*)buffer, count);
//		Kernel::instance().stop();
		src  += count;
		dest += count;
		size -= count;
	}


	Kernel::instance().tlb().switchAsid( old_asid );
	PRINT_DEBUG ("Thread copy complete, copied %u B of data.\n", dest - (char*)dst_addr);
//	Kernel::instance().stop();
	return EOK;
}
