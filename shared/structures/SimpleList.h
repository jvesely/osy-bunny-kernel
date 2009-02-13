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
 * @brief Simple list declaration
 *
 * Simple list is cyclical and has far less functionalit than normal list.
 * On the other hand, list items can be disconnected and reconnected to list
 * (see documentation of SimpleListItem).
 */
#pragma once
#include "SimpleListItem.h"


/** @brief minimal cyclical list
*
*	Only pointer to main item and basic methods. No count method,
*	no creation/deletion. Basic insertion to the end.
*	Main item is item which exists even in empty list. Tis simplifies some
*	list operations.
*/
class SimpleList
{
public:
	/** @brief only sets first and last
	*/
	SimpleList();

	/** @brief inserts new item (to the end)
	*
	*	Does not allocate anything.
	*/
	void insert(SimpleListItem * item);

	/** @brief get item connecting head and tail of list
	*
	*	Main item is the only one item which is allways in the list.
	*/
	inline const SimpleListItem * getMainItem() const{
		return & m_mainItem;
	}

	/** @brief get item connecting head and tail of list
	*
	*	Non-const version of function.
	*	Main item is the only one item which is allways in the list.
	*/
	inline SimpleListItem * getMainItem(){
		return & m_mainItem;
	}


	/** @brief
	*
	*	SimpleList is empty if only main item is in list.
	*/
	inline bool empty()const{return m_mainItem.next == getMainItem();}



protected:
	/** @brief item connecting head and tail
	*
	*	Must allways remain in list.
	*/
	SimpleListItem m_mainItem;

};




















