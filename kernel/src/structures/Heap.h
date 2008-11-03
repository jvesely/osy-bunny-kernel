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
 * @brief Contains both declaration and definition of a d-ary min-heap.
 */

#pragma once

#include "structures/VarLengthArray.h"

/*! 
 * @class Heap Heap "structures/Heap.h"
 * @brief Implementation of a min-heap.
 * 
 * This class uses VarLengthArray to store elements of the heap.
 * Template class. 
 * @param T Type of items in the heap. 
 *		  It is required to have operator < implemented. 
 * @param Children Number of node's children.
 * @param Prealloc Initial size of the variable-length array.
 *
 * @note Operation of inserting a new item to the heap expects <b>enough
 * memory</b> to be allocated. <b>Always check</b> this using isAvailable().
 */
template<class T, int Children, int Prealloc>
class Heap
{
public:
	/*! @brief Default constructor. */
	inline Heap() {};

	/*! @brief Default destructor. */
	inline ~Heap() {};

	/*!
	 * @brief Determines whether there is enough available memory allocated 
	 * for one new item and tries to allocate more if necessary.
	 *
	 * @retval True if there is enough memory,
	 * @retval False if not and the allocation failed.
	 */
	inline bool isAvailable();

	/*! @brief Appends one item into the array. Requires enough memory allocated. 
	 * 
	 * When using this function, <b>always check</b> if there is enough 
	 * <b>memory available</b> using isAvailable().
	 */
	void insert( const T& item );

	/*! @brief Returns the first (i.e. the smallest) element of the heap. */
	inline const T& findMin() const;

	/*! @brief Returns the first (i.e. the smallest) element of the heap
	 * and deletes it from the heap.
	 */
	T getMin();

	/*! @brief Returns number of items in the heap. */
	inline int size() const { return m_array.size(); }

private:
	/*! @brief Variable-length array for storing the heap. */
	VarLengthArray<T, Prealloc> m_array;

	/*! @brief Copy constructor. */
	Heap( const Heap<T, Children, Prealloc>& other );
	
	/*! @brief Operator = */
	Heap<T, Children, Prealloc>& operator=( const Heap<T, Children, Prealloc>& other );

	/*! @brief Returns index of the parent element of the i-th item. */
	inline int parent( const unsigned int i ) const;
	/*! @brief Returns index of the first child of the i-th item. */
	inline int firstChild( const unsigned int i ) const;
	/*! @brief Returns index of the n-th child of the i-th item. */
	inline int nthChild( const unsigned int n, const unsigned int i ) const;
	/*! @brief Returns index of the last child of the i-th item. */
	inline int lastChild( const unsigned int i ) const;
	/*! @brief Returns index of the smallest of @a n children of the @a i-th item. */
	inline int minChild( const unsigned int n, const unsigned int i ) const;
};

/*****************************************************************************/
/* DEFINITIONS ***************************************************************/
/*****************************************************************************/

template<class T, int Children, int Prealloc>
inline bool Heap<T, Children, Prealloc>::isAvailable()
{
	return m_array.isAvailable(1);
}

/*****************************************************************************/

template<class T, int Children, int Prealloc>
void Heap<T, Children, Prealloc>::insert( const T& item )
{
	// reserve space in the array for the new item
	m_array.resize(m_array.size() + 1);

	int hole = m_array.size() - 1;
	// percolate up
	while (hole > 0) {
		if (m_array[parent(hole)] < item) {
			break;
		} else {
			m_array[hole] = m_array[parent(hole)];
			hole = parent(hole);
		}
	}
	m_array[hole] = item;
}

/*****************************************************************************/

template<class T, int Children, int Prealloc>
inline const T& Heap<T, Children, Prealloc>::findMin() const
{
	return m_array[0];
}

/*****************************************************************************/

template<class T, int Children, int Prealloc>
T Heap<T, Children, Prealloc>::getMin() 
{
	T return_value = m_array[0];
	int hole = 0;
	// while there are both children in the heap
	while (hole < m_array.size() - 1) {
		// compare the last element with the smaller from two children of the hole
		int smallestChild;
		if (lastChild(hole) < m_array.size()) {
			// if there are all children
			smallestChild = minChild(Children, hole);
		} else if (firstChild(hole) < m_array.size()) {
			// if there is at least one child
			smallestChild = minChild(m_array.size() - firstChild(hole), hole);
		} else {
			// if the there are no children, we can't go further
			break;
		}

		if (m_array[m_array.size() - 1] < m_array[smallestChild]) {
			break;
		} else {
			// if it's larger, switch the smaller child and the hole
			m_array[hole] = m_array[smallestChild];
			hole = smallestChild;
		}
	}
	// fill the hole with the formerly last element of the heap
	m_array[hole] = m_array[m_array.size() - 1];
	m_array.resize(m_array.size() - 1);

	return return_value;
}

/*****************************************************************************/

template<class T, int Children, int Prealloc>
inline int Heap<T, Children, Prealloc>::parent( const unsigned int i ) const
{
	return (i - 1) / Children;
}

/*****************************************************************************/

template<class T, int Children, int Prealloc>
inline int Heap<T, Children, Prealloc>::firstChild( 
	const unsigned int i ) const 
{
	return (Children * i) + 1;
}

/*****************************************************************************/

template<class T, int Children, int Prealloc>
inline int Heap<T, Children, Prealloc>::lastChild( const unsigned int i ) const 
{
	return (Children * i) + Children;
}

/*****************************************************************************/

template<class T, int Children, int Prealloc>
inline int Heap<T, Children, Prealloc>::nthChild( 
	const unsigned int n, const unsigned int i ) const 
{
	return (Children * i) + n;
}

/*****************************************************************************/

template<class T, int Children, int Prealloc>
int Heap<T, Children, Prealloc>::minChild( 
	const unsigned int n, unsigned const int i ) const 
{
	int min = nthChild(1, i);
	for (unsigned int j = 2; j <= n; ++j)
		if (m_array[nthChild(j, i)] < m_array[min])
			min = nthChild(j, i);
	return min;
}