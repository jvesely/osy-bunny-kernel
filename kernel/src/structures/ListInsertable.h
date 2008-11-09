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

#include "List.h"
/*! @class ListInsertable ListInsertable.h "structures/ListInsertable.h"
 * @brief Base class for all classes that wish to be inserted into List.
 *
 * All classes that inherit this base class can be inserted into List without
 * allocating memory. Class supports appending and prepending to the List 
 * as well as removing from the List. It is also delete safe 
 * (It removes from any list it may be in upon destruction).
 */

template <class T>
class ListInsertable: public ListItem<T*>
{
public:
	/* @brief Constructor initializes ListItem parent class.
	 *
	 * Pointer to self is stored in ListItem data field.
	 */
	inline ListInsertable()
		:ListItem<T*>(static_cast<T*> (this)), m_myList(NULL) {
			};
	
	/*! @brief Appends itself to the given List.
	 *
	 * Function expects that the input pointer is not NULL. If the object is 
	 * already present in another list, it is removed first.
	 * @param list List the object shall append itself to.
	 */
	inline void append( List<T*>* list );

	/*! @brief Prepends itself to the given List.
	 *
	 * Function expects that the pointer is not NULL. Object removes iteself 
	 * from any list it is presnet in and prepends itself to given one.
	 * @param list List the object shall prepend itself to.
	 */
	inline void prepend( List<T*>* list );

	/*! @brief Removes itself from any List it is present in. */
	inline void remove();

	/*! @brief Destruction removes self from the list it is in */
	virtual ~ListInsertable();

private:
	/*! @brief List the object is listed in. */
	List<T*>* m_myList;
};

/*----------------------------------------------------------------------------*/
template <class T>
inline void ListInsertable<T>::append( List<T*>* list )
{
	remove();
	assert(!m_myList);
	m_myList = list;

	m_myList->pushBack(this);
}
/*----------------------------------------------------------------------------*/
template <class T>
inline void ListInsertable<T>::prepend( List<T*>* list )
{
	assert(list);
	remove();
	assert(!m_myList);
	m_myList = list;
	m_myList->pushFront(this);
}
/*----------------------------------------------------------------------------*/
template <class T>
inline void ListInsertable<T>::remove()
{
	if (!m_myList)
		return;
	m_myList->removeFind(static_cast<T*>(this));
	m_myList = NULL;
}
/*----------------------------------------------------------------------------*/
template <class T>
ListInsertable<T>::~ListInsertable()
{
	remove();
}
