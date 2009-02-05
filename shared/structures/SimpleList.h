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
#include "SimpleListItem.h"


/** @brief minimal cyclical list
*
*	Only pointer to first and last item and basic methods. No count method,
*	no creation/deletion. Basic insertion to the end.
*/
class SimpleList
{
public:
	/** @brief only sets first and last
	*/
	SimpleList();

	/** @brief inserts new item (to the end)
	*/
	void insert(SimpleListItem * item);

	/** @brief correctly reconnects item
	*
	*	if item.prev == NULL and item.next == first,
	*	item will be new first.
	*	if item.next == NULL and item.prev == last,
	*	item will be new second.
	*	If both prev and nextare not null, only reconnects item.
	*	If some logical error occurrs stops on assert.
	*/
	//void reconnect(SimpleListItem * item);

	/** @brief correctly removes from list, does not delete item
	*
	*	Disconnects item and if needed changes first/last.
	*/
	//void remove(SimpleListItem * item);


	/** @brief get item connecting head and tail of list
	*/
	inline const SimpleListItem * getMainItem() const{
		return & m_mainItem;
	}

	/** @brief get item connecting head and tail of list
	*
	*	Non-const function.
	*/
	inline SimpleListItem * getMainItem(){
		return & m_mainItem;
	}


	/** @brief no comment
	*/
	//inline bool empty(){return m_first==NULL;}
	inline bool empty()const{return m_mainItem.next == getMainItem();}



protected:
	/** @brief
	*
	*	Null if empty.
	*/
	//SimpleListItem * m_first;

	/** @brief
	*
	*	Null if empty.
	*/
	//SimpleListItem * m_last;

	/** @brief item connecting head and tail
	*
	*	Must allways remain in list
	*/
	SimpleListItem m_mainItem;

};




















