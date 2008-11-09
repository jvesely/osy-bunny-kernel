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
 * @brief List implementation
 *
 * doubly linkes list template class
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

//------------------------------------------------------------------------------
/** @brief doubly-linked list
*	@note type T must have defined: ctor, dtor, cpy-ctor, operator == and !=
*/
template <typename T> class List
{
public:

	/**	@class Iterator
	*	@brief list iterator
	*	enables to iterate trough the List elements
	*	iterator stores a pointer to listItem, see method getItem()
	**/
	class Iterator
	{
	public:

		/** @brief default ctor
		*	m_item is set to NULL
		*/
		inline Iterator () : m_item ( NULL ) {}

		/** @brief ctor with initialisation value */
		inline Iterator ( ListItem<T> * item ) : m_item ( item ) {}

		/** @brief copy ctor */
		inline Iterator ( const Iterator & it ) : m_item ( it.getItem() ) {}

		inline const T& operator * () const { return getData(); }	/**< @brief const dereference operator	*/
		inline T& operator * () { return getData(); } 				/**< @brief dereference operator */

		inline const T * operator -> ( ) const { return &getData(); } 	/**< @brief const dereference postfix operator */
		inline T * operator -> ( ) { return &getData(); } 				/**< @brief dereference postfix operator */

		/** @brief get const data stored in item
		*	@return const reference to data
		*	@note is not NULL-safe (data must exist)
		*/
		inline const T& getData() const
		{
			/*if ( m_item == NULL )
				return T();
			else
				return m_item->getData(); - old null-safe implementation; */
			return m_item->getData();
		}

		/** @brief get data stored in item
		*	@return reference to data
		*	@note is not NULL-safe (data must exist)
		*/
		inline T& getData()
		{
			return m_item->getData();
		}

		inline const T & data() const { return getData(); } /**< @brief calls getData() */

		inline T & data() { return getData(); } /**< @brief calls getData() */

		/** @brief ++ prefix operator
		*	@return reference to self after increment
		*/
		Iterator& operator ++ ()
		{
			if ( m_item != NULL )
				m_item = m_item->getNext();
			return *this;
		}

		/** @brief postfix ++ operator
		*	@return copy of self before change
		*	increments self
		*/
		Iterator operator ++ ( int )
		{
			Iterator ret ( *this );
			if ( m_item != NULL )
				m_item = m_item->getNext();
			return ret;
		}

		/** @brief ++ prefix operator
		*	@return reference to self after decrement
		*/
		Iterator operator -- ()
		{
			if ( m_item != NULL )
				m_item = m_item->getPrev();
			return *this;
		}

		/** @brief postfix ++ operator
		*	@return copy of self before change
		*	decrements self
		*/

		Iterator operator -- ( int )
		{
			Iterator ret ( *this );
			if ( m_item != NULL )
				m_item = m_item->getPrev();
			return ret;
		}

		/** @brief compares iterators
		*	compares stored pointers
		*/
		inline bool operator == ( Iterator it ) const
		{
			return ( it.getItem() == m_item );
		}

		/** @brief compares iterators
		*	compares stored pointers
		*/

		inline bool operator != ( Iterator it ) const
		{
			return !operator== ( it );
		}

		/** @brief copies stored pointer */
		Iterator& operator = ( Iterator it )
		{
			m_item = it.getItem();
			return *this;
		}

		inline ListItem<T> * getItem() const { return m_item; } /**< @brief returns stored pointer to list item */
		inline ListItem<T> * item() const { return getItem(); } /**< @brief returns stored pointer to list item */

	protected:

		ListItem<T> * m_item; /**< @brief stored pointer to list item with data */

	private:
	};

	/** @brief default ctor
	*	creaets empty list
	*/
	inline List() {	init(); }

	/** @brief copy ctor */
	List ( const List<T> & oldList )
	{
		init();
		appendList ( oldList );
	}

	/** @brief dtor
	*	deletes list and all entries
	*/
	virtual ~List() { clear(); }

	/** @brief copy list
	*	clears data first
	*	implementation note: calls copyList()
	*/
	inline const List<T> & operator = ( const List<T> & oldList ) {
		copyList ( oldList );
		return *this;
	}

	inline bool empty() const 	{ return ( m_first == NULL ); }/**< @brief no comment :) */

	inline int size() const { return m_count; } /**< @brief count of stored items */

	inline int getSize() const { return size(); } /**< @brief count of stored items */

	/** @brief removes from front
	*	if empty() returns without changes
	*/
	void popFront()
	{
		//get first item
		if ( m_first == NULL ) return;
		assert ( m_last != NULL );
		ListItem<T> * item = m_first;

		//internal changes of pointers and count
		assert ( m_count > 0 );
		--m_count;
		m_first = item->getNext();
		assert ( ( m_first == NULL ) || ( item != m_last ) );//if m_fist != NULL => item is not last one in list
		if ( m_first == NULL )
			m_last = NULL;

		//delete item (disconnecting is in dtor of item)
		delete item;
	}

	/** @brief removes from back
	*	if empty() returns without changes
	*/
	void popBack()
	{
		//get last item
		if ( m_last == NULL ) return;
		assert ( m_first != NULL );
		ListItem<T> * item = m_last;

		//internal changes of pointers and count
		assert ( m_count > 0 );
		--m_count;
		m_last = item->getPrev();
		assert ( ( m_last == NULL ) || ( item != m_first ) );//if m_last != NULL => item is not last one in list
		if ( m_last == NULL )
			m_first = NULL;

		//delete item (disconnecting is in dtor of item)
		delete item;
	}

	/** @brief get data in first element
	*	@return no reference, copied data; if no first element exists, default T is returned
	*	@note recommended to use iterator begin(), rbegin() instead
	*/
	inline T getFront() const
	{
		if ( m_first == NULL ) { return T(); }
		else return m_first->getData();
	}

	/** @brief get data in last element
	*	@return no reference, copied data; if no last element exists, default T is returned
	*	@note recommended to use iterator begin(), rbegin() instead
	*/
	inline T getBack() const
	{
		if ( m_last == NULL ) { return T(); }
		else return m_last->getData();
	}

	/** @brief clear
	*	implementation note: uses popBack()
	*/
	inline void clear()
	{
		while ( !empty() ) {
			popBack();
		}
	}

	/** @brief add item to the end
	*	@return iterator to new element if succesfull; end() else
	*/
	inline Iterator pushBack ( const T & data )
	{
		//create item
		ListItem<T> * item = new ListItem<T> ( data );
		if(item ==NULL) return end();
		//integrate
		return pushBack ( item );
	}

	/** @brief add already created ListItem to the end
	*	implementation note: does not create anything new and does not use local variables (except parameters)
	*	@return iterator to new element
	*/
	Iterator pushBack ( ListItem<T> * item )
	{
		if ( item == NULL ) { return end(); }
		//integrate item
		item->appendAfter ( m_last );//is NULL safe
		assert ( item->next() == NULL );
		if ( m_last == NULL )
		{
			m_first = item;
			assert ( item->prev() == NULL );
		}
		m_last = item;
		assert ( ( m_count > 0 ) || ( m_first == m_last ) );
		assert ( ( m_count > 0 ) || ( item->prev() == NULL ) );

		++m_count ;
		return Iterator ( item );
	}

	/** @brief add item to head
	*	@return iterator to new element if succesfull; rend() else
	*/
	inline Iterator pushFront ( const T & data )
	{
		//create item
		ListItem<T> * item = new ListItem<T> ( data );
		if(item ==NULL) return rend();
		//integrate
		return pushFront ( item );
	}

	/** @brief add already created ListItem to head
	*	implementation note: does not create anything new and does not use local variables (except parameters)
	*	@return iterator to new element
	*/
	Iterator pushFront ( ListItem<T> * item )
	{
		if ( item == NULL ) { return rend(); }
		//integrate item
		item->appendBefore ( m_first );
		assert ( item->prev() == NULL );
		if ( m_last == NULL )
		{
			m_last = item;
			assert ( item->next() == NULL );
		}
		m_first = item;
		++m_count;
		return Iterator ( item );
	}

	/** @brief disconnect first from list
	*	does not delete the item, this is the difference against popFront
	*	implementation note: does not allocate anything
	*	@return pointer to disconnected element; if nothing in list, NULL is returned
	*/
	ListItem<T> * removeFront()
	{
		//get first item
		if ( m_first == NULL ) return NULL;
		ListItem<T> * item = m_first;
		assert ( m_last != NULL );
		assert ( m_count > 0 );
		//remove from list ptrs
		if ( m_first == m_last ) //only one item in list
		{
			m_last = NULL;
			m_first = NULL;
		}else{
			m_first = m_first->next();
		}
		//disconnecting & end
		--m_count;
		item->disconnect();
		return item;
	}

	/** @brief disconnect last from list
	*	does not delete the item, this is the difference against popBack
	*	implementation note: does not allocate anything
	*	@return pointer to disconnected element; if nothing in list, NULL is returned
	*/
	ListItem<T> * removeBack()
	{
		//get last item
		if ( m_last == NULL ) return NULL;
		ListItem<T> * item = m_last;
		assert ( m_first != NULL );
		assert ( m_count > 0 );
		//remove from list ptrs
		if ( m_first == m_last ) //only one item in list
		{
			m_last = NULL;
			m_first = NULL;
		}else{
			m_last = m_last->prev();
		}
		//disconnecting & end
		--m_count;
		item->disconnect();
		return item;
	}

	/** @brief disconects item from list
	*	does not delete the item, this is the difference against erase
	*	item is after this operation definitelly not conencted to this list
	*	(but can be connected in other, if method was called on bad object)
	*	@note does not check whether item is in 'this' list
	*	@param item removed list item
	*	@return pointer to removed item
	*/
	ListItem<T> * remove(ListItem<T> * item)
	{
		if(item == NULL) return item;
		if(m_count == 0) return item;//cannot be here
		//setting first/last pointer
		if(item == m_first)
		{
			m_first = item->getNext();
		}
		if(item == m_last)
		{
			m_last = item->getPrev();
		}
		//disconencting from list
		item->disconnect();
		//count
		--m_count;
		return item;
	}

	/** @brief disconects item from list - wrapper over remove(ListItem) method
	*	does not delete the item, this is the difference against erase
	*	item is after this operation definitelly not conencted to this list
	*	(but can be connected in other, if method was called on bad object)\n
	*	@note does not check whether item is in 'this' list. does not invalidate iterator it
	*	@param it iterator to item removed from list
	*	@return pointer to removed ListItem
	*/
	inline ListItem<T> * remove(Iterator it)
	{
		return remove(it.getItem());
	}

	/** @brief rotates list by 1 item forward
	*	moves first element behind the last
	*	implementation note: does not use any local variables and does not allocate or delete anything
	*	implementation note: calls ListItem->disconnect()
	*	result should be the same as pushBack( rotatedList.removeFront() );
	*	@return iterator to new \a first element
	*/
	inline Iterator rotate() {
		if ( m_first == NULL ) return begin();
		if ( m_first == m_last ) return begin();
		//connecting
		m_first->appendAfter ( m_last );
		//changing pointers
		m_first = m_first->next();
		m_last = m_last->next();
		//disconnecting
		m_first->setPrev ( NULL );
		m_last->setNext ( NULL );
		return begin();
	}

	/** @brief reverse-rotate
	*	rotates list by 1 item backwards
	*	moves last element before the first
	*	implementation note: does not use any local variables and does not allocate or delete anything
	*	implementation note: calls ListItem->disconnect()
	*	result should be the same as pushFront( rotatedList.removeBack() );
	*	@return iterator to new \a first element
	*/
	inline Iterator rrotate() {
		if ( m_first == NULL ) return rbegin();
		if ( m_first == m_last ) return rbegin();
		//connecting
		m_last->appendBefore ( m_first );
		//changing pointers
		m_first = m_first->prev();
		m_last = m_last->prev();
		//disconnecting
		m_first->setPrev ( NULL );
		m_last->setNext ( NULL );
		return begin();
	}

	/** @brief find
	*	searches for first matching item from start; uses defined == operator for type T
	*	searches towards front
	*	@return iterator to found item; end() if nothing has been found
	*/
	Iterator find ( const T& data, const Iterator& start ) const
	{
		Iterator it;
		for ( it = start; ( it != end() ) && ( ( *it ) != data ); ++it ) {};
		return it;
	}

	/** @brief reverse find
	*	searches for first matching item from rstart; uses defined == operator for type T
	*	searches towards back
	*	@return iterator to found item; rend() if nothing has been found
	*/

	Iterator rfind ( const T& data, const Iterator& rstart ) const
	{
		Iterator it;
		for ( it = rstart; ( it != rend() ) && ( ( *it ) != data ); --it ) {};
		return it;
	}

	/*! @brief gets ListItem containing value from the middle of the list
	 * @param value value to find
	 * @return pointer to listItem contaning value and detached from the list
	 * NULL on failure
	 */
	ListItem<T>* removeFind(const T& value)
	{
		Iterator it = find(value);
		if(it==end()) return NULL;
		remove(it);
		return it.getItem();


		/*if (!m_first) return NULL;
		if (m_first->data() == value)
			return removeFront();
		assert(m_last);
		if (m_last->data() == value)
			return removeBack();
		ListItem<T>* ptr = m_first->next();
		while (ptr) {
			if(ptr->data() == value) break;
			ptr = ptr->next();
		}
		if (ptr) {
			assert(ptr->next()); //it's not last
			ptr->next()->setPrev(ptr->prev());
			assert(ptr->prev()); //it'snot first
			ptr->prev()->setNext(ptr->next());
		}*/
//		return ptr;
	}


	/** @brief find from first item */
	inline Iterator find ( const T & data ) const { return find ( data, this->begin() ); }

	/** @brief rfind from last item	*/
	inline Iterator rfind ( const T& data ) const { return rfind ( data, this->rbegin() ); }

	/** @brief deletes forst matching
	*	searching from front to back
	*/
	inline void erase ( const T& data ) { erase ( find ( data ) ); }

	/** @brief erase by iterator
	*	@note iterator is invalidated by this operation, nobody knows what could happen
	*/
	void erase ( Iterator it )
	{
		if ( it.getItem() != NULL )
		{
			removeDelete ( it.getItem() );
		}//else nothing
	}

	inline Iterator begin() const { return Iterator ( m_first ); } /**< @brief iterator to first element */

	inline Iterator end()  const {return Iterator ( NULL );} /**< @brief invalid iterator */

	inline Iterator rbegin() const { return Iterator ( m_last ); } /**< @brief iterator to last element */

	/** @brief invalid pointer
	*	implementation note: iterator to reverse end() is the same as end() - both are iterators with NULL m_item
	*/
	inline Iterator rend()  const {return Iterator ( NULL );}

	/** @brief insert before iterator
	*	if it == end() element is inserted on the end of list
	*	@return iterator to new element; if fails, end() is returned
	*/
	Iterator insert ( const Iterator& it, const T& value )
	{
		if ( it == end() ) return pushBack ( value );

		ListItem<T> * item = it.getItem();
		ListItem<T> * newItem = new ListItem<T> ( value );

		return insert(item,newItem);
	}

	/** @brief insert after iterator
	*	if it == rend() element is inserted on the begin of list
	*	@return iterator to new element; if fails, rend() is returned
	*/

	Iterator rinsert ( const Iterator& it, const T& value )
	{
		if ( it == rend() ) return pushFront ( value );

		ListItem<T> * item = it.getItem();
		ListItem<T> * newItem = new ListItem<T> ( value );

		return rinsert(item,newItem);
	}

	/** @brief new-free insert before item
	*	if item == NULL, new item is inserted on the end of list
	*	@param item listItem before which will be insertedItem inserted
	*	@param insertedItem inserted item
	*	@return iterator to new element; if fails(insertedItem is NULL), end() is returned
	*
	*/
	Iterator insert ( ListItem<T> * item, ListItem<T> * insertedItem )
	{
		if ( insertedItem == NULL ) return Iterator ( NULL );
		if ( item == NULL ) return pushBack ( insertedItem );

		insertedItem->insertBefore ( item );
		if ( item == m_first )
			m_first = insertedItem;
		++m_count;
		return Iterator ( insertedItem );
	}

	/** @brief new-free insert before item
	*	if item == NULL, new item is inserted on the end of list
	*	@param item listItem before which will be insertedItem inserted; default NULL
	*	@param insertedItem inserted item
	*	@return iterator to new element; if fails(insertedItem is NULL), end() is returned
	*	wrapper for insert(item,insertedItem)
	*/
	inline Iterator insertBefore( ListItem<T> * insertedItem ,ListItem<T> * item = NULL){
		return insert(item,insertedItem);
	}

	/** @brief new-freeinsert after iterator
	*	if it == rend() element is inserted on the begin of list
	*	@param item listItem after which will be insertedItem inserted
	*	@param inertedItem inserted item
	*	@return iterator to new element; if fails(insertedItem is NULL), rend() is returned
	*/

	Iterator rinsert ( ListItem<T> * item, ListItem<T> * insertedItem )
	{
		if ( insertedItem == NULL ) return Iterator ( NULL );
		if ( item == NULL ) return pushFront ( insertedItem );

		insertedItem->insertAfter ( item );
		if ( item == m_last )
			m_last = insertedItem;
		++m_count;
		return Iterator ( insertedItem );
	}

	/** @brief new-freeinsert after iterator
	*	if it == rend() element is inserted on the begin of list
	*	@param item listItem after which will be insertedItem inserted; default NULL
	*	@param inertedItem inserted item
	*	@return iterator to new element; if fails(insertedItem is NULL), rend() is returned
	*	wrapper for rinsert(item,insertedItem)
	*/
	inline Iterator insertAfter ( ListItem<T> * insertedItem, ListItem<T> * item = NULL ){
		return rinsert(item,insertedItem);
	}


	/** @brief copies list
	*	clears list first
	*/
	inline void copyList ( const List<T> & oldList )
	{
		clear();
		appendList ( oldList );
	}

	/** @brief appends data from other list
	*	@return error message EOK or ENOMEM*/
	inline int appendList ( const List<T> & oldList )
	{
		Iterator it;
		int errmsg = EOK;
		for ( it = oldList.begin(); it != oldList.end(); ++it ) {
			if(pushBack ( *it ) == end()) errmsg = ENOMEM;
		}
		return errmsg;
	}

protected:

	/** @brief init of values
	*	iterators to NULL, count to 0
	*/
	void init()
	{
		m_first = NULL;
		m_last = NULL;
		m_count = 0;
	}

	/** @brief correctly removes and deletes item from list
	*	item is deleted
	*/
	void removeDelete ( ListItem<T> * item )
	{
		//special cases
		if ( item == NULL ) return;
		if ( item == m_first )
		{
			popFront();
			return;
		}
		if ( item == m_last )
		{
			popBack();
			return;
		}//else

		delete item;
		assert ( m_count > 0 );
		--m_count;
	}


	ListItem<T> * m_first; /**< @brief first element in list */
	ListItem<T> * m_last; /**< @brief lst element in list */

	int m_count; /**< @brief count of elements */

private:
};


