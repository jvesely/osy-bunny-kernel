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
#include <api.h>
//------------------------------------------------------------------------------
/** @brief element in doubly-linked list
*	@note type T must have defined: ctor, dtor, cpy-ctor, operator == and !=
*/
template <typename T> class ListItem
{
public:

	inline ListItem() { init(); } /**< @brief default ctor	*/

	inline ListItem ( T data ) : m_data ( data ) { init();	} /**< @brief ctor with data initialisation */

	/** @brief dtor
	*	disconnects from list
	*	see disconnect()
	*/
	virtual ~ListItem()	{ disconnect(); }

	inline const T& getData() const { return m_data; } /**< @brief const reference to data */
	inline const T& data() const { return getData(); } /**< @brief const reference to data */
	inline  T& getData()  { return m_data; } /**< @brief reference to data */
	inline  T& data()  { return getData(); } /**< @brief reference to data */

	inline ListItem<T> * getNext() const { return m_next; } /**< @brief next item in list */
	inline ListItem<T> * next() const { return getNext(); } /**< @brief next item in list */

	inline ListItem<T> * getPrev() const { return m_prev; } /**< @brief previous item in list */
	inline ListItem<T> * prev() const { return getPrev(); } /**< @brief previous item in list */

	inline void setNext ( ListItem<T> * next ) { m_next = next; } 	/**< @brief set next item in list */
	inline void next ( ListItem<T> * nnext ) { setNext ( nnext ); }	/**< @brief set next item in list */

	inline void setPrev ( ListItem<T> * prev ) { m_prev = prev; }	/**< @brief set previous item in list */
	inline void prev ( ListItem<T> * nprev ) { setNext ( nprev ); }	/**< @brief set previous item in list */

	inline void setData ( const T & data) {m_data = data;} 	/**< @brief set data */
	inline void data(const T & data){ setData(data);}			/**< @brief wrapper over setData */

	/** @brief append after \a prev
	*	@note prev->next() is ignored, even if exists
	*	@note checks whether \a prev is null
	*/
	void appendAfter ( ListItem<T> * prev )
	{
		if ( prev != NULL )
		{
			prev->setNext ( this );
		}
		setPrev ( prev );
	}


	/** @brief append before \a next
	*	@note next->prev() is ignored, even if exists
	*	@note checks whether \a next is null
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

	/** @brief disconnects from linked - list
	*	only rewrites pointers
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

	T m_data;/**< @brief data */
	ListItem<T> * m_prev; /**< @brief previous item in list */
	ListItem<T> * m_next; /**< @brief next item in list */


	/** @brief initialisation
	*	pointers are set to null
	*/
	inline void init()
	{
		m_prev = NULL;
		m_next = NULL;
	}

};

