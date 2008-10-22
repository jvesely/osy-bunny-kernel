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
 * @brief Buffer template class.
 *
 * Contains implementation of simple fixed size buffer.
 */
#pragma once

#include "api.h"

/*! @class Buffer Buffer.h "structures/Buffer.h"
 * @brief Simple fixed size buffer.
 *
 * Template using type T and int SIZE. Type T is required to have operator= impelmented.
 * Stores up to SIZE items of type T.
 * Inserting into full buffer will ignore inserted item. 
 * Pulling expects not empty buffer, this needs to be checked before read is called,
 * otherwise are results undefined. (uses assert)
 */
template<typename T, int SIZE>
class Buffer
{
public:
	/*! constructor zeros indexes and count */
	inline Buffer():m_head(0), m_tail(0), m_count(0) {};

	/*! inserts item into internal array
	 * @param item item to be inserted
	 * @return number of items inserted (0/1)
	 */
	size_t insert(const T& item)
	{
		if (m_count == SIZE) // buffer is full
			return 0;
		m_buffer[m_tail] = item; 
		m_tail = (m_tail + 1) % SIZE;
		++m_count;
		assert(m_count <= SIZE);
		return 1;
	};

	/*! reads the first item in the buffer without removing it.
	 * Should only be called on non-empty buffer.
	 * @return const refernce to the first item.
	 */
	inline const T& read() const
	/* expects non-empty buffer, needs to be checked before using it */
	{
		assert(m_count > 0);
		return m_buffer[m_head];
	};

	/*! reads and removes te first item from the array.
	 * Should only be called on non-empty buffer.
	 * @return first item in the array
	 */
	inline T get()
	/* expects non-empty buffer, needs to be checked before using */
	{
		assert(m_count > 0);
		T value =  m_buffer[m_head++];
		m_head %= SIZE;
		--m_count;
		return value;
	};

	/*! @return number of items in the buffer */
	inline size_t count() const	{ return m_count;	};

private:
	/*! index of the first item */
	size_t m_head;
	/*! index of the last item */
	size_t m_tail;
	/*! number of items in te array */
	size_t m_count;
	/*! storage structure */
	T m_buffer[SIZE];

};
