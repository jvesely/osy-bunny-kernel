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
 * @brief Contains both declaration and definition class Heap.
 *
 * Class Heap is an implementation of a d-ary dynamic min-heap 
 * (class Heap<@a T, @a Children>).
 *
 * @note This class never allocates memory, just inserts and removes 
 * prepared items. These are of type HeapItem<@a T, @a Children>.
 */

#pragma once

#include "HeapItem.h"

/*! @class Heap Heap "structures/Heap.h"
 * @brief Implementation of a d-ary dynamic min-heap.
 * 
 * Template class:
 * @param T Type of the data to be stored in the heap.
 *			It is required to have operator < defined
 * @param Children Number of item's children. Should be a power of 2.
 * 
 * @note This class never allocates memory, just inserts and removes 
 * prepared items. These are of type HeapItem<@a T, @a Children>.
 */
template <class T, int Children> 
class Heap
{
public:
	/*! @brief Default constructor. Initializes root pointer @a m_root to NULL,
	 * @a m_last_parent pointer to NULL and size of the array @a m_size to 0. 
	 */
	inline Heap();
	/*! @brief Default destructor. Does nothing yet.
	 * @todo Remove all items from the heap.
	 */
	inline ~Heap() {};

	/*! @brief Inserts one prepared item into the heap. */
	void insert( HeapItem<T, Children>* item );

	/*! @brief Removes item beign pointed to by @a item, from the heap. */
	void remove( HeapItem<T, Children>* item );

	/*! @brief Returns the first (i.e. the smallest) item in the heap.
	 * as a pointer to the proper HeapItem.
	 */
	inline HeapItem<T, Children>* topItem() const;
	
	/*! @brief Returns the first (i.e. the smallest) item in the heap.
	 * as a const reference to the item value.
	 */
	inline const T& top() const;

	/*! @brief Returns the first (i.e. the smallest) element of the heap 
	 * and deletes it from the heap. 
	 */
	inline HeapItem<T, Children>* getTop();

	/*! @brief Returns the number of items in the heap. */
	unsigned int size() const { return m_size; }

	/*! @brief Checks if heap satisfies the heap property. 
	 * 
	 * Uses isHeapPart() to check the root of the heap.
	 */
	bool isHeap() const;

	/*! @brief Checks heap integrity.
	 *
	 * Uses checkHeapPart() to check the root of the heap.
	 */
	bool checkHeap() const;

	/*! @brief Prints whole heap. */
	void printHeap() const;

private:

	/*! @brief Pointer to the first (i.e. the smallest) item in the heap. */
	HeapItem<T, Children>* m_root;

	/*! @brief Pointer to the parent whose child will be the next item inserted. 
	 * @note This is IMHO the best way how to determine where to 
	 * insert a new item and in the same time to simply change all pointers
	 * that have to be changed during the insertion.
	 */
	HeapItem<T, Children>* m_last_parent;

	/*! @brief The actual count of items in the heap. */
	unsigned int m_size;

	/*! @brief Default copy constructor. 
	 * Made private in order not to allow copying the heap.
	 */
	Heap( const Heap<T, Children>& other );

	/*! @brief Operator =. Made private in order not to allow copying the heap.
	 */
	Heap<T, Children>& operator=( const Heap<T, Children>& other);

	/*! @brief Percolates the given item down down the heap. */
	void percolateDown( HeapItem<T, Children>* item );
	
	/*! @brief Percolates the given item down up the heap. */
	void percolateUp( HeapItem<T, Children>* item );

	/*! @brief Returns the first (i.e. the smallest) item in the heap.
	 * as a pointer to the proper HeapItem.
	 */
	inline HeapItem<T, Children>* findMinItem() const { return m_root; }

	/*! @brief Returns the first (i.e. the smallest) item in the heap. */
	inline const T& findMin() const { return m_root->m_data; }
	
	/*! @brief Returns the first (i.e. the smallest) element of the heap 
	 * and deletes it from the heap. 
	 */
	HeapItem<T, Children>* getMinItem();

	/*! @brief Swaps two items in the heap. 
	 *
	 * Uses HeapItem<@a T>::swapWithAny() to swap two different items.
	 */
	void swapItems( HeapItem<T, Children>* item1, HeapItem<T, Children>* item2 );

	/*! @brief Checks heap node for integrity. */
	bool checkHeapPart( HeapItem<T, Children>* item ) const;

	/*! @brief Checks if heap node satisfies the heap property. */
	bool isHeapPart( HeapItem<T, Children>* item ) const;

	/*! @brief Checks if the item is initialized as a new HeapItem. */
	bool isNew( HeapItem<T, Children>* item ) const;
};

/*---------------------------------------------------------------------------*/
/* DEFINITIONS --------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

template <class T, int Children>
inline Heap<T, Children>::Heap()
	: m_root(NULL), m_last_parent(NULL), m_size(0) 
{}

template <class T, int Children>
inline void Heap<T, Children>::insert( HeapItem<T, Children>* item )
{
	ASSERT(isNew(item));

	/*------------------------------------------------
	1) insert the item as the last node of the heap  
	--------------------------------------------------*/

	if (!m_root) {	// if there is no root
		ASSERT(!m_last_parent);
		m_last_parent = m_root = item;
		ASSERT(m_root->isEmpty() && !m_root->m_parent && !m_root->m_follower);
		return;
	}	// OK
	
	ASSERT(m_root && m_last_parent);
	
	m_last_parent = m_last_parent->insertChild(item);

	/*------------------------------------------------
	2) percolate the inserted child up  
	--------------------------------------------------*/
	percolateUp(item);	
}

/*---------------------------------------------------------------------------*/

template <class T, int Children>
inline void Heap<T, Children>::remove( HeapItem<T, Children>* item )
{
	if (!item)
		return;

	if ((m_last_parent == m_root) && (m_last_parent->isEmpty()) ) {
		// there is no other item in the heap than the root
		ASSERT(item == m_root);
		m_last_parent = m_root = NULL;
		return;
	}

	/*---------------------------------------------------
	1) swap removed item and last item in the heap
	-----------------------------------------------------*/
	HeapItem<T, Children>* last = m_last_parent->lastChild();

	bool removing_last = (last == item);

	if (!removing_last) {
		// if we're removing the root
		if (item == m_root)
			// set the root to the last child as this will be switched with root
			m_root = last;	

		// if we're removing the item which is the last parent
		if (item == m_last_parent)
			// set the last parent to the last item as this will be switched
			m_last_parent = last;

		swapItems(item, last);
		// set item to the item which we'll need to percolate down
		item = last;
		// set last againg to the last item of the heap (previously item)
		last = m_last_parent->lastChild();
	}
	// if we're removing the last item, we don't have to swap anything

	/*---------------------------------------------------
	2) disconnect the last item from the heap
	-----------------------------------------------------*/	
	// there must be some previous
	ASSERT(last->m_previous);
	last->m_previous->m_follower = NULL;
	last->m_previous = NULL;

	// follower must be NULL
	ASSERT(!last->m_follower);

	// parent
	ASSERT(last->m_parent && last->m_parent == m_last_parent);
	last->m_parent->replaceChild(last, NULL);
	last->m_parent->m_count -= 1;
	if ( (!last->m_parent->m_count) && (last->m_parent != m_root) ) {
		m_last_parent = last->m_parent->m_previous;
	} else {
		ASSERT(m_last_parent == last->m_parent);
	}
	last->m_parent = NULL;

	if (!removing_last) {
		/*---------------------------------------------------
		3) percolate the previously last item in the heap
		   to the direction where the heap property isn't satisfied
		-----------------------------------------------------*/
		if ( item->m_parent && (*(item) < *(item->m_parent)) ) {
			percolateUp(item);
		} else {
			percolateDown(item);
		}
	}
	
}

/*---------------------------------------------------------------------------*/

template <class T, int Children>
void Heap<T, Children>::percolateDown( HeapItem<T, Children>* item )
{
	// while item is not a leaf
	while (/*item->m_follower && */item->m_count) {
		HeapItem<T, Children>* min = item->minChild();

		if (*min < *item) {
			if (item == m_root)
				m_root = min;

			if (min == m_last_parent)
				m_last_parent = item;
			else if (item == m_last_parent)
				m_last_parent = min;

			swapItems(min, item);
		} else {
			break;
		}
	}
}

/*---------------------------------------------------------------------------*/

template <class T, int Children>
void Heap<T, Children>::percolateUp( HeapItem<T, Children>* item )
{
	while (item->m_parent && ((*item) < *(item->m_parent)) ) {

		if (item->m_parent == m_root)
			m_root = item;

		// if the swap will influence the item to which m_last_parent points,
		// we must change that pointer 

		if (item->m_parent == m_last_parent)
			m_last_parent = item;
		else if (item == m_last_parent)
			m_last_parent = item->m_parent;

		swapItems( item, item->m_parent );
	}
}

/*---------------------------------------------------------------------------*/

template <class T, int Children>
inline HeapItem<T, Children>* Heap<T, Children>::topItem() const 
{ 
	return findMinItem(); 
}

/*---------------------------------------------------------------------------*/

template <class T, int Children>
inline const T& Heap<T, Children>::top() const 
{ 
	return findMin(); 
}

/*---------------------------------------------------------------------------*/

template <class T, int Children>
inline HeapItem<T, Children>* Heap<T, Children>::getTop() 
{ 
	return getMinItem(); 
}

/*---------------------------------------------------------------------------*/

template <class T, int Children>
HeapItem<T, Children>* Heap<T, Children>::getMinItem()
{
	HeapItem<T, Children>* ptr = m_root;
	remove(m_root);
	return ptr;
}

/*---------------------------------------------------------------------------*/

template <class T, int Children>
void Heap<T, Children>::swapItems( 
	HeapItem<T, Children>* item1, HeapItem<T, Children>* item2 )
{
	item1->swapWithAny(item2);
}

/*---------------------------------------------------------------------------*/

template <class T, int Children>
bool Heap<T, Children>::isHeap() const
{
	return isHeapPart( m_root );
}

/*---------------------------------------------------------------------------*/

template <class T, int Children>
bool Heap<T, Children>::isHeapPart( HeapItem<T, Children>* item ) const
{
	bool is_heap = true;
	unsigned int i = 0;

	while ( is_heap && (i < item->m_count) ) {
		if (*(item->m_children[i]) < (*item)) {
			is_heap = false;
			ASSERT(false);
			break;
		} else {
			is_heap = isHeapPart( item->m_children[i] );
		}
		++i;
	}

	return is_heap;
}

/*---------------------------------------------------------------------------*/

template <class T, int Children>
bool Heap<T, Children>::checkHeap() const
{
	return checkHeapPart( m_root );
}

/*---------------------------------------------------------------------------*/

template <class T, int Children>
bool Heap<T, Children>::checkHeapPart( HeapItem<T, Children>* item ) const
{
	bool is_ok = true;
	unsigned int i = 0;

	while ( is_ok && (i < item->m_count) ) {
		if (!item->checkItem()) {
			is_ok = false;
			break;
		} else {
			is_ok = checkHeapPart( item->m_children[i] );
		}
		++i;
	}

	return is_ok;
}

/*---------------------------------------------------------------------------*/

template <class T, int Children>
bool Heap<T, Children>::isNew( HeapItem<T, Children>* item ) const
{	
	if (item->m_previous)
		return false;
	if (item->m_follower)
		return false;
	if (item->m_parent)
		return false;
	if (item->m_count)
		return false;
	
	bool ok = true;
	for (unsigned int i = 0; i < Children; ++i)
		if (item->m_children[i]) {
			ok = false;
			break;
		}
	return ok;
}

/*---------------------------------------------------------------------------*/

template <class T, int Children>
void Heap<T, Children>::printHeap() const
{
	m_root->printItem();
}
