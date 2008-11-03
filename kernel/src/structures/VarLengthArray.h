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
 * @brief Contains both declaration and definition of a variable-length array template class.
 *  
 * This class is inspired by QVarLengthArray class from Qt framework by Trolltech.
 */
 
#pragma once

#include "api.h"

/*! 
 * @class VarLengthArray VarLengthArray "structures/VarLengthArray.h"
 * @brief Simple variable-length array.
 *
 * Template class.
 * @param T can by any type. 
 * @param Prealloc determines size of the array allocated on stack (see below).
 *
 * This class initially allocates some memory (@a Prealloc * @c sizeof(@a T)) 
 * on the stack and reallocates only when you need array with more than 
 * Prealloc items. Stack allocation is much faster than heap allocation, 
 * thus boosting the performance of the array when the initial size 
 * is suggested well.
 *
 * The class reallocates to use less memory only when the requested size
 * is less then @a Prealloc. In this case it throws away all the memory
 * allocated dynamically and switches to the prellocated memory.
 * The prellocated size of the array should thus be chosen carefully
 * as in the worst case the array will alocate and deallocate memory
 * too often.
 *
 * @note When creating an array of size larger than @a Prealloc
 *		 always check if it allocated enough memory using isReady().
 * @note Operations of appending an item and resizig the array
 *		 expect enough memory allocated.
 *		 You need to first check this using isAvailable(const int req).
 * @note Operator [] expects it's parameter to be inside the array index range.
 *		 Otherwise the results are undefined.
 * 
 * This class is inspired by QVarLengthArray class from Qt framework by Trolltech.
 */
template<class T, int Prealloc>
class VarLengthArray
{
public:
	/*!
	 * @brief Variable length array initialization.
	 *
	 * If needed, allocates required amount of memory.
	 * 
	 * @param size Required size of the array.
	 * If created an array of size @a size > 0, the elements will be uninitialized.
	 *
	 * @note If you created a variable length array A of size @a size, 
	 * next append will put the element to A[@a size] 
	 * leaving first @a size items uninitialized.
	 */
	inline VarLengthArray( const unsigned int size = 0 );

	/*! @brief Destroys the array, deleting all it's contents. */
	inline ~VarLengthArray();

	/*! @brief Copy constructor. */
	inline VarLengthArray( const VarLengthArray<T, Prealloc>& other );

	/*! @brief Checks if the array is empty. */
	inline bool isEmpty() const { return (m_size == 0); }

	/*! @brief Checks if the array was initialized successfuly. */
	inline bool isReady() const { return m_initialized; }

	/*! @brief Returns number of items in the array. */
	inline int size() const { return m_size; }

	/*! @brief Returns a const pointer pointing to the beginning of an array. */
	inline const T* constData() const { return m_ptr; }

	/*! @brief Returns a modifiable pointer to the beginning of the array. */
	inline T* data() { return m_ptr; }

	/*! @brief Returns the i-th item as a modifiable reference. */
	inline T& operator[]( const unsigned int i );

	/*! @brief Returns the i-th item as a const reference. */
	inline const T& operator[]( const unsigned int i ) const;

	/*! @brief Appends one item into the array. Requires enough memory allocated. 
	 * 
	 * When using this function, <b>always check</b> if there is enough 
	 * <b>memory available</b> using isAvailable( const int req ).
	 */
	inline void append( const T& item );

	/*! @brief Appends one item into the array. Requires enough memory allocated. 
	 * 
	 * When using this function, <b>always check</b> if there is enough 
	 * <b>memory available</b> using isAvailable( const int req ).
	 */
	void append( const T* items, const unsigned int size );

	/*! @brief Resizes the array to a given size. Requires enough memory allocated.
	 * 
	 * Never allocates more memory. In case the required size is less than
	 * @a Prealloc, it switches to the preallocated array.
	 * When using this function, <b>always check</b> if there is enough 
	 * <b>memory available</b> using isAvailable( const int req ).
	 */
	void resize( const unsigned int size );

	/*!
	 * @brief Determines whether there is enough available memory allocated 
	 * and tries to allocate more if necessary.
	 *
	 * @param req Number of elements required to add to the array.
	 * @retval True if there is enough memory,
	 * @retval False if not and the allocation failed.
	 */
	bool isAvailable( const unsigned int req );

	/*! @brief Clears the array. Does not change the allocated space. */
	inline void clear() { resize(0); }

private:
	/*! @brief Flag to indicate if there is enough space allocated. */
	bool m_initialized;
	/*! @brief Amount of memory allocated. */
	unsigned int m_allocated;
	/*! @brief Size of the array (count of elements in the array). */
	unsigned int m_size;
	/*! @brief Pointer denoting the first element of the array. */
	T* m_ptr;
	/*! @brief Static array used until it is large enough. */
	T m_array[Prealloc];

	/*! @brief Operator = */
	VarLengthArray<T, Prealloc>& operator=( const VarLengthArray<T, Prealloc>& other );

	/*!
	 * @brief Allocates more memory for the array.
	 * 
	 * @param size Required size of the array.
	 * @retval True if the allocation was successful,
	 * @retval False otherwise.
	 */
	bool reallocate( const unsigned int size );

	/*!
	 * @brief Throws away all dynamically allocated memory and restores
	 * the preallocated array.
	 *
	 * @param size New size of the array.
	 */
	void switchBack( const unsigned int size );
};

/*****************************************************************************/
/* DEFINITIONS ***************************************************************/
/*****************************************************************************/

template<class T, int Prealloc>
inline VarLengthArray<T, Prealloc>::VarLengthArray( 
	const unsigned int size )
: m_initialized(false), m_allocated(Prealloc), m_size(0), m_ptr(m_array)  
{
	if (isAvailable(size)) {
		m_size = size;
		m_initialized = true;
	}
}

/*****************************************************************************/

template<class T, int Prealloc>
inline VarLengthArray<T, Prealloc>::~VarLengthArray()
{
	if (m_ptr != m_array)
		delete(m_ptr);
}

/*****************************************************************************/

template<class T, int Prealloc>
inline VarLengthArray<T, Prealloc>::VarLengthArray( 
	const VarLengthArray<T, Prealloc> &other )
: m_ptr(m_array), m_size(0), m_initialized(false), m_allocated(Prealloc)
{
	if (isAvailable(other.size())) {
		append(other.constData(), other.size());
		m_initialized = true;
	}
}

/*****************************************************************************/

template<class T, int Prealloc>
inline T& VarLengthArray<T, Prealloc>::operator[]( const unsigned int i )
{
	assert(i < m_size);
	return m_ptr[i];
}

/*****************************************************************************/

template<class T, int Prealloc>
inline const T& VarLengthArray<T, Prealloc>::operator[]( 
	const unsigned int i ) const 
{
	assert(i < m_size);
	return m_ptr[i];
}

/*****************************************************************************/
/*
template<class T, int Prealloc>
VarLengthArray<T, Prealloc>& VarLengthArray<T, Prealloc>::operator =( const VarLengthArray<T, Prealloc> &other )
{
	if (this != &other) {
		clear();
		if (isAvailable()) {
			append(other.constData(), other.size());
			assert(m_size == other.size());
		}
	}
	return *this;
}
*/
/*****************************************************************************/

template<class T, int Prealloc>
inline void VarLengthArray<T, Prealloc>::append( const T& item ) 
{
	if (m_allocated > m_size)
		m_ptr[m_size++] = item;
}

/*****************************************************************************/

template<class T, int Prealloc>
void VarLengthArray<T, Prealloc>::append( 
	const T* items, const unsigned int size ) 
{
	assert(items);
	if (size <= 0)
		return;
	
	assert(m_allocated >= (m_size + size));

	if (m_allocated < (m_size + size))
		return;

	memcpy(&m_ptr[m_size], items, size * sizeof(T));
	m_size += size;
}

/*****************************************************************************/

template<class T, int Prealloc>
void VarLengthArray<T, Prealloc>::resize( const unsigned int size )
{
	// if there is not enough memory allocated
	if (size > m_allocated)
		return; // do nothing

	// if the size is less than the preallocated memory 
	// and we don't use the preallocated memory
	if (size <= Prealloc && m_ptr != m_array)
		switchBack(size);	// switch to the preallocated array

	m_size = size;	// in any case set the size to the new value
}

/*****************************************************************************/

template<class T, int Prealloc>
bool VarLengthArray<T, Prealloc>::isAvailable( const unsigned int req )
{
	if (m_size + req <= m_allocated)
		return true;

	// try to allocate enough memory and return the result
	return reallocate(m_size + req);
}

/*****************************************************************************/

template<class T, int Prealloc>
void VarLengthArray<T, Prealloc>::switchBack( const unsigned int size )
{
	// save the old pointer
	T* old = m_ptr;
	// switch to the preallocated arrays
	m_ptr = m_array;	
	// determine new size of the array
	unsigned int new_size = ((m_size < size) ? m_size : size);
	// if there should be some elements, copy them from the previous array
	if (new_size)
		memcpy((void*) m_ptr, (void*) old, new_size * sizeof(T));
	// set the allocated memory
	m_allocated = Prealloc;
	// and delete the old array, freeing the allocated memory
	delete(old);
}

/*****************************************************************************/

template<class T, int Prealloc>
bool VarLengthArray<T, Prealloc>::reallocate( const unsigned int size )
{
	// if the size is less than the preallocated memory
	if (size <= Prealloc) {
		if (m_ptr != m_array) // and we don't use the preallocated memory
			switchBack(size);
		return true;
	}

	// determine amount of memory to allocate as the nearest larger power of 2
	unsigned int alloc = (m_allocated) ? m_allocated : 2;	
	while (alloc < size)
		alloc *= 2;
	assert(alloc);

	if (alloc <= m_allocated)	
		return true;	// don't reallocate to use less memory!

	T* old = m_ptr;
	assert(old);
	assert(m_ptr);

	// alocate dynamicaly the required memory
	m_ptr = new T[alloc];
	// if allocation failed, fail
	if (!m_ptr) {
		m_ptr = old;
		return false;
	} 
	else {
		m_allocated = alloc;

		if (old == m_array && m_size)
			// if I used statically alocated array till now and there were
			// some old items, copy all of them into the new memory
			memcpy(m_ptr, old, m_size * sizeof(T));	
			// there is no need to delete the old array, as it's statically 
			// alocated and will be destroyed when the object is

		/*	
		The old version of this part distingushed between 1st time allocation
		and reallocation, when realloc should be used instead to save time:

		m_ptr = reinterpret_cast<T*>(realloc(m_ptr, alloc * sizeof(T)));	
		*/

		return true;
	}

}