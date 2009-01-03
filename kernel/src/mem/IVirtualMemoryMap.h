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
 * @brief IVirtualMemoryMap declaration.
 *
 */

#pragma once

#include "types.h"
#include "Object.h"
#include "Pointer.h"
#include "drivers/Processor.h"

/*! @class IVirtualMemoryMap IVirtualMemoryMap.h "mem/IVirtualMemoryMap.h"
 *
 * @brief VirtualMemoryMap interface with same basic ASID handling.
 *
 * IVirtualMemoryMap is an abstract class providing interface for mapping, 
 * demapping and trnaslating of virtual adresses. IVirtualMemoryMap also handles
 * ASIDs, so that the map implementation does not need to care about it.
 */

class IVirtualMemoryMap: public Object
{
public:

	static Pointer<IVirtualMemoryMap>& getCurrent()
		{ static Pointer<IVirtualMemoryMap> current; return current; }

	/*! @brief Gets ASID assigned to this map.
	 * @return ASID assigned.
	 */
	inline byte asid() { return m_asid; }

	/*! @brief Assigns ASID to this map.
	 * @param asid ASID assigned.
	 * @return ASID assigned.
	 */
	inline byte setAsid( byte asid ) { return m_asid = asid; }

	/*! @brief Makes this Memory map active for translation.
	 *
	 * Sets current ASID to the assigned asid.
	 */
	void switchTo();

	/*! @brief InterMap memcpy.
	 *	@param src_addr Address to copy from.
	 *	@param dest_map Virtual map of the destination pointer.
	 *	@param dst_addr Address to copy to.
	 *	@param size number of bytes to copy.
	 *	@return EOK.
	 * Copies data from one virtual address space to another.
	 */
	int copyTo(const void* src_addr, Pointer<IVirtualMemoryMap> dest_map, void* dst_addr, size_t size);

	/*! @brief Interface for creating virtual memory area 
	 * @param from pointer to the location of the starting pointer,
	 * 	handling depends on flags
	 * @param size requested size of the VMA
	 * @param flags tunes the allocation.
	 * @return EOK on success, respective error code otherwise.
	 * @note See documentation of child class, that implements this function.
	 */
	virtual int allocate(void** from, size_t size, unsigned int flags) = 0;

	/*! @brief Destroys VMA.
	 * @param from The first byte of the VMA.
	 * @return EOK on succes, respective error code otherwise.
	 * @note See documentation of child class, that implements this function.
	 */
	virtual int free(const void* from) = 0;

	/*! @brief Translates Virtual address into physical.
	 * @param address Virtual address to translate, physical adress is returned
	 * 	in this param as well.
	 * @param frame_size size of the frame physical address resides in.
	 * @return @a true on success, @a false otherwise.
	 * @note See documentation of child class, that implements this function.
	 */
	virtual bool translate(void*& address, Processor::PageSize& frame_size) = 0;

	/*! @brief Returns used ASID. */
	virtual ~IVirtualMemoryMap();

protected:
	/*! @brief Makes sure that the free area is no longer accessible. */
	void freed();

private:
	byte m_asid; /*!< ASID used by this map, no other map can have same ASID. */
};
