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
 *   @version $Id: SimpleList.cpp 343 2008-12-08 21:12:55Z dekanek $
 *   @note
 *   Semestral work for Operating Systems course at MFF UK \n
 *   http://dsrg.mff.cuni.cz/~ceres/sch/osy/main.php
 *
 *   @date 2008-2009
 */

/*!
 * @file
 * @brief Simple list
 *
 */

#include "SimpleList.h"

//------------------------------------------------------------------------------
SimpleList::SimpleList()//:
//m_first(NULL),
//m_last(NULL)
{
	m_mainItem.prev = &m_mainItem;
	m_mainItem.next = &m_mainItem;
}


//------------------------------------------------------------------------------
/*
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
	/* //non-cyclical implementatio
	(!m_first)||(m_first=item);
	item->prev=m_last;
	item->next=NULL;
	m_last=item;
	*/
	//cyclical implementation
	assert(item != &m_mainItem);
	item->prev = m_mainItem.prev;
	item->next = &m_mainItem;
	item->reconnect();

}
//------------------------------------------------------------------------------
/*void SimpleList::reconnect(SimpleListItem * item)
{
	item->reconnect();

	if(!item->prev)
	{
		assert(item->next == m_first);
		m_first = item;
	}
	if(!item->next)
	{
		assert(item->prev == m_last);
		m_last = item;
	}
}*/
//------------------------------------------------------------------------------

/*void SimpleList::remove(SimpleListItem * item)
{
	if(item == m_first)
		m_first = item->next;

	if(item == m_last)
		m_last = item->prev;
	item->disconnect();
}
*/









