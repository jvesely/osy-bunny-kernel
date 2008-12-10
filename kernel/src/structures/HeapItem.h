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
 * @brief Contains both declaration and definition of class HeapItem.
 *
 * Class HeapItem represents one item in Heap.
 *
 * @note In contrast to class HeapInsertable, this class can be used separately
 * and be inserted into Heap.
 */

#pragma once

#include "api.h"

template <class T, int Children> class Heap;

/*! 
 * @class HeapItem HeapItem.h "structures/HeapItem.h"
 * @brief This class represents one item in Heap.
 *
 * Template class:
 * @param T	Type of the data to be stored in the heap.
 *			It is required to have operator < defined.
 * @param Children Number of item's children (i.e. Heap<@a T, @a Children> will then be a @a Children-ary heap). Should be a power of 2.
 *
 * @note In contrast to class HeapInsertable, this class can be used separately
 * and be inserted into Heap.
 */
template <class T, int Children>
class HeapItem
{
public:
	/*! 
	 * @brief Default constructor, initializes data to the given value.
	 * 
	 * Also initializes the internal structure (all pointers to NULL).
	 */
	inline HeapItem( const T& data = T() );

	/*! 
	 * @brief Default destructor. Does nothing. Virtual because of
	 * HeapInsertable derived from this class.
	 */
	virtual ~HeapItem() {};
	
	/*! 
	 * @brief Operator <. 
	 *
	 * Made virtual so class HeapInsertable and classes derived from it
	 * can redefine it.
	 * When called on HeapItem itself (thus not on a derived item), 
	 * it compares the data stored in item.
	 */
	virtual bool operator < ( const HeapItem<T, Children>& other ) const;

	/*! @brief Returns const reference to the data stored in item. */
	inline const T& data() const { return m_data; }

protected:
	/*! @brief Data stored in the heap item. */
	T m_data;

private:

	/* MEMBER DATA ----------------------------------------------------------*/
	
	/*! 
	 * @brief Pointer to the parent item in the heap. 
	 * NULL if this item is the root. 
	 */
	HeapItem<T, Children>* m_parent;
	
	/*! @brief  Array of pointers to this item's children. */
	HeapItem<T, Children>* m_children[Children];
	
	/*! 
	 * @brief Pointer to the next item in the heap.
	 * 
	 * In case I'm the last item in the heap it's NULL.
	 * In case, I'm the rightmost item in a row and am not the last item,
	 * this will point to the first child of my leftmost brother, 
	 * i.e. my first nephew.
	 */
	HeapItem<T, Children>* m_follower;
	
	/*! 
	 * @brief Pointer to the previous item in the heap.
	 * 
	 * In case I'm the root in the heap it's NULL.
	 * In case, I'm the leftmost item and am not the root,
	 * this will point to the leftmost brother of my parent, 
	 * i.e. my last uncle.
	 */
	HeapItem<T, Children>* m_previous;
	
	/*! @brief Number of children of this item. */
	unsigned short m_count;

	/* MEMBER FUNCTIONS -----------------------------------------------------*/

	/*! 
	 * @brief Tries to insert a new child to the item.
	 *
	 * @param child Item to be inserted as a child of this node.
	 *
	 * If there is a place in this item (i.e. it has less than @a Children)
	 * @a child will be inserted as a child of this node.
	 * If this item is full (thus having maximum number of children),
	 * @a child will be inserted to the @b follower of this item calling 
	 * the insertChild function on it.
	 *
	 * @return Pointer to the item where this child was inserted and
	 * where the next new child should be inserted as well.
	 */
	HeapItem<T, Children>* insertChild( HeapItem<T, Children>* child );

	/*! @brief Swaps item with any @a other given item from the heap. */
	void swapWithAny( HeapItem<T, Children>* other );

	/*! @brief Replaces my child @a old_child with @a new child. */
	void replaceChild( 
		HeapItem<T, Children>* old_child, HeapItem<T, Children>* new_child );

	/*! @brief Returns pointer to the pointer to the found child. */
	HeapItem<T, Children>** findChild( HeapItem<T, Children>* child ); 

	/*! @brief Swaps two pointers. */
	inline void swap( HeapItem<T, Children>** ptr1, HeapItem<T, Children>** ptr2 );

	/*! @brief Checks whether the item has no children. */
	inline bool isEmpty() const { return m_count == 0; }

	/*! @brief Checks whether the item has maximum children. */
	inline bool isFull() const { return m_count == Children; }

	/*! @brief Returns pointer to the last child of the item. */
	inline HeapItem<T, Children>* lastChild() const;

	/*! @brief Returns pointer to the minimum of the item's children. */
	HeapItem<T, Children>* minChild() const;

	/*! 
	 * @brief Checks the integrity of the item,
	 * i.e. whether the following, previous, parent and children
	 * items have their proper pointer pointed to me.
	 * E.g. my follower has to have me set as his previous item.
	 */
	bool checkItem() const;

	/*! @brief Prints node and calls itself recursively on it's children. */
	void printItem() const;

friend class Heap<T, Children>;

};

/*---------------------------------------------------------------------------*/
/* DEFINITIONS --------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

template <class T, int Children>
inline HeapItem<T, Children>::HeapItem( const T& data ) 
	: m_data(data), m_parent(NULL), m_follower(NULL), m_previous(NULL), 
	m_count(0)
{
	for (unsigned int i = 0; i < Children; ++i)
		m_children[i] = NULL;
}

/*---------------------------------------------------------------------------*/

template <class T, int Children>
bool HeapItem<T, Children>::operator<( 
	const HeapItem<T, Children>& other ) const
{
	return m_data < other.m_data;
}

/*---------------------------------------------------------------------------*/

template <class T, int Children>
HeapItem<T, Children>* HeapItem<T, Children>::insertChild( 
	HeapItem<T, Children>* child )
{
	if (isFull()) {
		lastChild()->m_follower = child;
		child->m_previous = lastChild();
		HeapItem<T, Children> *next_parent = m_follower->insertChild( child );
		ASSERT(next_parent == m_follower);
		next_parent = NULL;	// just to be used when NDEBUG is defined
		return m_follower;
	}

	if (!isEmpty()) {
		// I'm normal node with some children already
		// set the new child as follower of my last child
		lastChild()->m_follower = child;
		child->m_previous = lastChild();
	} 
	else if (!m_parent) {
		ASSERT(!m_follower);
		// I'm the root node, so set my first child as the next item
		m_follower = child;
		child->m_previous = this;
	}
	// else I'm not the root and I'm empty
	// nothing to do
	
	// I'm either the special case when a full node wants to insert
	// it's new child to me, or a normal node

	// set as my new last child
	m_children[m_count++] = child;
	// and set the parent of the new child
	child->m_parent = this;

	return this;
}

/*---------------------------------------------------------------------------*/

template <class T, int Children>
void HeapItem<T, Children>::swapWithAny( HeapItem<T, Children>* other )
{

	/*-------------------------------------------------------
	1)	change the properties of following and preceding nodes  
		change followers and previous items
	--------------------------------------------------------*/
	
	if (m_previous != other) {
		if (m_previous)
			m_previous->m_follower = other;
		if (other->m_follower)
			other->m_follower->m_previous = this;
	}

	if (m_follower != other) {
		if (m_follower)
			m_follower->m_previous = other;
		if (other->m_previous)
			other->m_previous->m_follower = this;
	}

	// if changing two consequent items and other is my follower
	if (m_follower == other) {
		// set other's previous to my previous
		m_follower->m_previous = m_previous;	
		// set my follower to the other's follower
		m_follower = other->m_follower;
		// connect on the other side (=> remain consequent)
		m_previous = other;
		other->m_follower = this;
	}
	// if changing two consequent items and other is my prev.
	else if (m_previous == other) {
		// set other's follower to my follower
		m_previous->m_follower = m_follower;	
		// and set my previous to other's previous
		m_previous = other->m_previous;			
		// connect on the other side (=> remain consequent)
		m_follower = other;
		other->m_previous = this;
	}
	// if changing two non-consequent items, can be any two in the heap
	else {	// (m_follower != other) && (m_previous != other)
		swap(&m_previous, &(other->m_previous));
		swap(&m_follower, &(other->m_follower));
	}

	/*-------------------------------------------------------
	2) parents
	--------------------------------------------------------*/

	if (m_parent != other->m_parent) {
		// replace me in my parent with other
		if (m_parent)
			m_parent->replaceChild(this, other);
		// replace other in it's parent with me
		if (other->m_parent)
			other->m_parent->replaceChild(other, this);
	} else {
		swap(m_parent->findChild(this), m_parent->findChild(other));
	}

	// I'm the parent of other
	if (other->m_parent == this) {
		other->m_parent = m_parent;
		m_parent = other;
	}
	// other is my parent
	else if (m_parent == other) {
		m_parent = other->m_parent;
		other->m_parent = this;
	}
	// we're not related
	else {	// (other->m_parent != this) && (m_parent != other)
		swap(&(other->m_parent), &m_parent);
	}

	/*-------------------------------------------------------
	3) swap children between nodes 
	   in the same time, change children's parent pointers
	   properly: my children will have parent "other" and 
	   other's children will have parent "this"
	--------------------------------------------------------*/

	for (unsigned int i = 0; i < Children; ++i) {
		swap( &(m_children[i]), &(other->m_children[i]) );
		if (m_children[i])
			m_children[i]->m_parent = this;
		if (other->m_children[i])
			other->m_children[i]->m_parent = other;
	}

	/*-------------------------------------------------------
	4) change count of my children with the count of other's
	--------------------------------------------------------*/

	unsigned short cnt = m_count;
	m_count = other->m_count;
	other->m_count = cnt;

	/*-------------------------------------------------------
	DONE
	--------------------------------------------------------*/
}

/*---------------------------------------------------------------------------*/

template <class T, int Children>
void HeapItem<T, Children>::replaceChild( 
	HeapItem<T, Children>* old_child, HeapItem<T, Children>* new_child )
{
	for (unsigned int i = 0; i < m_count; ++i) {
		if (m_children[i] == old_child) {
			m_children[i] = new_child;
			return;
		}
	}
	// should never reach this point
	ASSERT(false);
}

/*---------------------------------------------------------------------------*/

template <class T, int Children>
HeapItem<T, Children>** HeapItem<T, Children>::findChild( 
	HeapItem<T, Children>* child )
{
	for (unsigned int i = 0; i < m_count; ++i) {
		if (m_children[i] == child)
			return &m_children[i];
	}
	ASSERT(false);
	// should not reach this point but we must return something ;-)
	return NULL;
}

/*---------------------------------------------------------------------------*/

template <class T, int Children>
inline void HeapItem<T, Children>::swap( 
	HeapItem<T, Children>** ptr1, HeapItem<T, Children>** ptr2)
{
	HeapItem<T, Children>* p = *ptr1;
	*ptr1 = *ptr2;
	*ptr2 = p;
}

/*---------------------------------------------------------------------------*/

template <class T, int Children>
inline HeapItem<T, Children>* HeapItem<T, Children>::lastChild() const
{ 
	ASSERT(m_count);
	return m_children[m_count-1]; 
}

/*---------------------------------------------------------------------------*/

template <class T, int Children>
HeapItem<T, Children>* HeapItem<T, Children>::minChild() const
{
	ASSERT(m_count);

	HeapItem<T, Children>* min = m_children[0];
	for (unsigned int i = 1; i < m_count; ++i) {
		if (*(m_children[i]) < *min)
			min = m_children[i];
	}
	return min;
}

/*---------------------------------------------------------------------------*/

template <class T, int Children>
bool HeapItem<T, Children>::checkItem() const
{
	bool ok_part = false;
	if (m_parent) {
		for (unsigned short i = 0; i < Children; ++i) {
			if (m_parent->m_children[i] && (m_parent->m_children[i] == this) ) {
				ok_part = true;
				break;
			}
		}
	}

	if (m_parent && !ok_part) {
		printf("\nError: No such child in parent!! \n");
		ASSERT(false);
		return false;
	}

	if (m_previous) {
		if (m_previous->m_follower != this) {
			printf("\nError: Previous item not ok!! \n");
			ASSERT(false);
			return false;
		}
	}

	if (m_follower) {
		if (m_follower->m_previous != this) {
			printf("\nError: Following item not ok!! \n");
			ASSERT(false);
			return false;
		}
	}

	int count = 0;
	bool ok = true;
	for (unsigned short i = 0; i < Children; ++i) {
		if (m_children[i]) {
			count++;
			if (m_children[i]->m_parent != this) {
				ok = false;
				ASSERT(false);
			}
		}
	}

	if (count != m_count) {
		printf("\nError: Children count not ok!! \n");
		ASSERT(false);
		return false;
	}

	return ok;
}

/*---------------------------------------------------------------------------*/

template <class T, int Children>
void HeapItem<T, Children>::printItem() const
{
	printf("%x <%x>, ", this, m_parent);
	if (m_follower)
		m_follower->printItem();
}
