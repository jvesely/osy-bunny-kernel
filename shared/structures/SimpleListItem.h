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
 * @brief Simple list item base struture declaration
 *
 * 	Simple list item has slightly more functionality than normal list item,
 *	it is possible to disconnect self from SimpleList and to reconnect self.
 */

#pragma once

#include "types.h"

/** @brief base class for any simple list item
*
*	Because of simplicity of this class, everything in it is public.
*	Only reason why this is class(and not struct), is disconnect method,
*	which is better	here (and not in SimpleList), because there is no direct
*	(read easily computable) connection between SimpleList instance and
*	list item.
*	@note It is unsafe to use implicit operator = and copy constructor, be sure
*		you know what you are doing.
*/
class SimpleListItem
{
public:

	SimpleListItem * prev;
	SimpleListItem * next;

	/** @brief disconnects item from list
	*
	*	Correctly disconnects from previous and next list item and
	*	nulls next and prev pointer.
	*/
	inline void disconnect();

	/** @brief validates pointers in prev and next
	*
	*	If prev and next pointers are set, item is connected between these.
	*	Function might be used after using copy constructor or operator =,
	*	to connect new item (and disconnect old item).This will make
	*	copied item unreachable from list of course.
	*/
	inline void reconnect();


};

/* this code is intentionally obfuscated ;)
   __________________
   |               . \
   |                  \
   |                 \/
   |_________________/
      /\        /\
	 /  \      /  \
*/
inline void SimpleListItem::disconnect()
{
	(prev)&&(prev->next=next);
	(next)&&(next->prev=prev);
	prev=NULL;
	next=NULL;
}
//------------------------------------------------------------------------------
inline void SimpleListItem::reconnect()
{
	(prev)&&(prev->next=this);
	(next)&&(next->prev=this);
}













