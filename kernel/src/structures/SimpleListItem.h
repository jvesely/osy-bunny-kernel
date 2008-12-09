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
 *   @version $Id: SimpleListItem.h 273 2008-11-27 18:34:19Z dekanek $
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
 * Long description. I would paste some Loren Ipsum rubbish here, but I'm afraid
 * It would stay that way. Not that this comment is by any means ingenious but
 * at least people can understand it.
 */

#pragma once
#include <api.h>

/** @brief base class for any simple list item
*
*	Because of simplicity of this class, everything in it is public.
*	Only reason why this is class is disconnect method, which is better
*	here (and not in SimpleList), because there is no direct (read easily
*	computable) connection between SimpleList instance and list item.
*	It is unsafe to use implicit operator = and copy constructor.
*/
class SimpleListItem
{
public:

	SimpleListItem * prev;
	SimpleListItem * next;

	/** @brief disconnects item from list
	*/
	inline void disconnect();

	/** @brief validates pointers in prev and next
	*
	*	Is used when operator = or copy constructor is applied. This will make
	*	copied item unreachable from list of course and copy correctly connected.
	*/
	inline void reconnect();


};

/*
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













