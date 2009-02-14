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
 * @brief List item template
 *
 *	Item for List template. Item has some basic functionality, but this
 *	should not be in most cases used by user, use List member functions
 *	instead.
 */

#pragma once

#include "types.h"
//#include "api.h"
//------------------------------------------------------------------------------
/** @brief element in doubly-linked list
*	@note type T must have defined: ctor, dtor, cpy-ctor, operator == and !=
*/
template <typename T> class ListItem
{
public:

	/**< @brief default constructor	*/
	inline ListItem()
	{ init(); }

	/**< @brief constructor with data initialisation */
	inline ListItem ( T data ) : m_data ( data )
	{ init(); }

	/** @brief destructor
	*
	*	Disconnects from list.
	*/
	virtual ~ListItem()	{ disconnect(); }

	/**< @brief const reference to data */
	inline const T& getData() const { return m_data; }
	/**< @brief const reference to data (wrapper)*/
	inline const T& data() const { return getData(); }
	/**< @brief reference to data */
	inline  T& getData()  { return m_data; }
	/**< @brief reference to data (wrapper)*/
	inline  T& data()  { return getData(); }

	/**< @brief next item in list */
	inline ListItem<T> * getNext() const { return m_next; }
	/**< @brief next item in list (wrapper)*/
	inline ListItem<T> * next() const { return getNext(); }

	/**< @brief previous item in list */
	inline ListItem<T> * getPrev() const { return m_prev; }
	/**< @brief previous item in list (wrapper)*/
	inline ListItem<T> * prev() const { return getPrev(); }

	/**< @brief set next item in list */
	inline void setNext ( ListItem<T> * next ) { m_next = next; }
	/**< @brief set next item in list (wrapper)*/
	inline void next ( ListItem<T> * nnext ) { setNext ( nnext ); }

	/**< @brief set previous item in list */
	inline void setPrev ( ListItem<T> * prev ) { m_prev = prev; }
	/**< @brief set previous item in list (wrapper)*/
	inline void prev ( ListItem<T> * nprev ) { setNext ( nprev ); }

	/**< @brief set data */
	inline void setData ( const T & data) {m_data = data;}
	/**< @brief set data (wrapper)*/
	inline void data(const T & data){ setData(data);}

	/** @brief append after prev
	*
	*	@param prev item after which should be item inserted
	*	@note prev->next() is ignored, even if it exists, so this function
	*		should be use only to add to the end of list
	*	@note checks whether prev is null
	*/
	void appendAfter ( ListItem<T> * prev )
	{
		if ( prev != NULL )
		{
			prev->setNext ( this );
		}
		setPrev ( prev );
	}


	/** @brief append before next
	*
	*	@param item before which should be item inserted
	*	@note next->prev() is ignored, even if it exists, so this function
	*		should be use only to add to the begin of list
	*	@note checks whether next is null
	*/
	void appendBefore ( ListItem<T> * next )
	{
		if ( next != NULL )
		{
			next->setPrev ( this );
		}
		setNext ( next );
	}

	/** @brief insert between item and item->prev() */
	void insertBefore ( ListItem<T> * item )
	{
		if ( item != NULL )
		{
			setPrev ( item->getPrev() );
			item->setPrev ( this );
			if ( m_prev != NULL )
			{
				m_prev->setNext ( this );
			}
		}
		else
		{
			setPrev ( NULL );
		}
		setNext ( item );
	}

	/** @brief insert between item and item->next() */
	void insertAfter ( ListItem<T> * item )
	{
		if ( item != NULL )
		{
			setNext ( item->getNext() );
			item->setNext ( this );
			if ( m_next != NULL )
			{
				m_next->setPrev ( this );
			}
		}
		else
		{
			setNext ( NULL );
		}
		setPrev ( item );
	}

	/** @brief disconnects from linked list
	*
	*	Only rewrites pointers to previous and next item and
	*	also pointers in next and previous items.
	*/
	inline void disconnect()
	{
		if ( m_prev != NULL )
		{
			m_prev->setNext ( m_next );
		}
		if ( m_next != NULL )
		{
			m_next->setPrev ( m_prev );
		}
		m_next = NULL;
		m_prev = NULL;
	}

private:

	/**< @brief data */
	T m_data;
	/**< @brief previous item in list */
	ListItem<T> * m_prev;
	/**< @brief next item in list */
	ListItem<T> * m_next;


	/** @brief initialisation
	*
	*	Pointers are set to null.
	*/
	inline void init()
	{
		m_prev = NULL;
		m_next = NULL;
	}

};

