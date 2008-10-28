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
 * @brief ListItem Pool declaration.
 *
 * ListItem pool, preallocates List items to be used in Thread operations 
 * that cannot fail.
 */
#pragma once

#include "structures/List.h"
#include "proc/Thread.h"

/*!
 * @class ItemPool ItemPool.h "structures/ItemPool.h"
 * @brief ListItem pool.
 *
 * Class reserves and sotres ListItem instances that can be used in List
 * without need to call malloc(new). IF possible this class should be replaced
 * by some kind of SLAB allocator.
 */
class ItemPool
{
public:
	/*! @brief Allocates and stores n ListItems
	 * @param n number of items to create and store
	 * @return number of created items
	 */
	uint32_t reserve( uint32_t n = 1 );
	
	/*! @brief Returns number of available ListItems.
	 * @return number of stored preallocated items
	 */
	inline uint32_t reserved() { return m_pool.size(); };

	/*! @brief Gets one preallocated items.
	 * @return pointer to one allocated item
	 * @note SHould only be used one pool that contains at least 1 item
	 */
	inline ListItem<Thread*>* get()
		{ return m_pool.removeFront(); };
	
	/*! @brief Returns no longer used Item
	 * @param pointer to returnd item
	 */
	inline void put(ListItem<Thread*>* item)
		{ m_pool.pushBack(item) != m_pool.end(); };

	/*! @brief Destroys one allocated item
	 */
	inline void free()
		{ m_pool.popBack(); };

private:
	List<Thread*> m_pool; /*!< List that stores items */

};
