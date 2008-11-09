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
 * @brief Contains implementation of HeapInsertable template class.
 *
 * HeapInsertable is a base class of all objects that can be inserted into Heap.
 * All classes that inherit this base class can be inserted into Heap 
 * without allocating memory.
 *
 * @note This class must not be used separately, use it only to derive
 * your class from it.
 */

#pragma once

#include "Heap.h"

/*! @class HeapInsertable HeapInsertable "structures/HeapInsertable.h"
 * @brief This is a base class of all objects that can be inserted into Heap.
 * 
 * All classes that inherit this base class can be inserted into Heap 
 * without allocating memory. 
 *
 * When class MyClass is derived from HeapInsertable, the Heap must have
 * MyClass* as it's first template argument.
 *
 * Usage example:
 *
 * @code 
 * class MyClass: public HeapInsertable<MyClass, int, 8>
 * {
 *	...
 * }
 *
 * Heap<MyClass*, 8> my_heap;
 * MyClass* my_item = new MyClass;
 * int key = 5;
 * 
 * my_item->insertIntoHeap(m_heap, key)
 * my_item->removeFromHeap();
 *
 * @endcode
 * 
 * Template class:
 * @param T Class that is derived from HeapInsertable.
 * @param Key Key used for comparing items during heap operations.
 *			  It must have operator < defined.
 * @param Children Number of children of the heap's item. Should be a power of 2.
 *		  (i.e. Heap<@a T, @a Children> will then be a @a Children-ary heap).
 *
 * @note This class must not be used separately, use it only to derive
 * your class from it.
 */
template <class T, typename Key, int Children>
class HeapInsertable: public HeapItem<T*, Children>
{
public:
	/*! @brief Default constructor initializes the HeapItem with a pointer 
	 * to itself and sets itself as not inserted into a heap.
	 */
	inline HeapInsertable() 
		: HeapItem<T*, Children>( static_cast<T*> (this) ), m_owner(NULL) {};
	
	/*! @brief Destructor removes self from the heap it was inserted into. */
	inline ~HeapInsertable();

	/*! @brief Operator < compares the keys of two HeapInsertable objects. 
	 *
	 * Can be called only when tie object was inserted into heap,
	 * otherwise the results are undefined.
	 */
	bool operator<( const HeapItem<T*, Children>& other ) const;
	
	/*! @brief Returns const reference to the key. */
	inline const Key& key() const { return m_key; }
	
	/*! @brief Returns the data stored in the HeapItem (i.e. a pointer
	 * to myself).
	 */
	inline const T* value() const { return static_cast<T*>(this->m_data); }

	/*! @brief Inserts itself into @a heap, initializing it's key to @a key. 
	 *
	 * Does nothing in case the object has already been inserted into a heap.
	 */
	void insertIntoHeap(Heap<T*, Children>* heap, const Key &key);
	
	/*! @brief Removes itself from the heap it was inserted into. 
	 *
	 * Does nothing in case the object was not yet inserted into a heap.
	 */
	void removeFromHeap();

private:
	/*! @brief Pointer to the heap where it's inserted. */
	Heap<T*, Children> *m_owner;

	/*! @brief The key of this item, used for heap operations. */
	Key m_key;
};

/*---------------------------------------------------------------------------*/
/* DEFINITIONS --------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

template <class T, typename Key, int Children>
inline HeapInsertable<T, Key, Children>::~HeapInsertable()
{
	if (m_owner)
		m_owner->remove(this);
}

/*---------------------------------------------------------------------------*/

template <class T, typename Key, int Children>
bool HeapInsertable<T, Key, Children>::operator<( 
	const HeapItem<T*, Children>&other ) const
{
	return m_key < static_cast<const HeapInsertable<T, Key, Children>*>
					(&other)->m_key;
}

/*---------------------------------------------------------------------------*/

template <class T, typename Key, int Children>
void HeapInsertable<T, Key, Children>::insertIntoHeap(Heap<T*, Children>* heap, const Key &key)
{
	assert(!m_owner);
	if (!m_owner) {
		m_key = key;
		(m_owner = heap)->insert(this);
	}
}

/*---------------------------------------------------------------------------*/

template <class T, typename Key, int Children>
void HeapInsertable<T, Key, Children>::removeFromHeap()
{
	if (m_owner) {
		m_owner->remove(this);
		m_owner = NULL;
	}
}
