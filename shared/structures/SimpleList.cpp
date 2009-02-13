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
 * @brief Simple list implementation
 *
 */

#include "api.h"
#include "SimpleList.h"

//------------------------------------------------------------------------------
SimpleList::SimpleList()
{
	m_mainItem.prev = &m_mainItem;
	m_mainItem.next = &m_mainItem;
}


//------------------------------------------------------------------------------
/* this code is intentionally obfuscated ;)
   __________________
   |               . \
   |                  \
   |                 \/
   |_________________/
      /\        /\
	 /  \      /  \
*/
void SimpleList::insert(SimpleListItem * item)
{

	assert(item != &m_mainItem);
	item->prev = m_mainItem.prev;
	item->next = &m_mainItem;
	item->reconnect();

}
