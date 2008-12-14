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

#include "types.h"
#include "Object.h"
#include "Pointer.h"

class IVirtualMemoryMap: public Object
{
public:
	inline byte asid() { return m_asid; }
	inline byte setAsid( byte asid) { return m_asid = asid; }
	void switchTo();
	int copyTo(const void* src_addr, Pointer<IVirtualMemoryMap> dest_map, void* dst_addr, size_t size);
	virtual int allocate(void** from, size_t size, unsigned int flags)=0;
	virtual int free(const void* from)=0;
	virtual bool translate(void*& address, size_t& frameSize)=0;
	virtual ~IVirtualMemoryMap();

protected:
	void freed();

private:
	byte m_asid;
};
