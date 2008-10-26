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
 * @brief Bitset implementation.
 *
 * Bitset is a data structure to efficiently store bit values like true/false
 * or 0/1. This class stores the bits in array of types, which are native
 * to the used processor. In our case it is MIPS with 32 bit unsigned integers.
 * The class allows to create an instance of it with preallocated or dynamically
 * allocated container. It also allows querying and setting each bit,
 * querying how many free spaces (zeros) are after a given bit and also setting
 * more bits to a specified value as fast as possible.
 */

#include "Bitset.h"

const size_t Bitset::BITS = sizeof(unative_t) * 8;
const unative_t Bitset::MASK = ~0;
const unative_t Bitset::MOD_MASK = Bitset::BITS - 1;
const unative_t Bitset::HEAD = 1 << (sizeof(unative_t) * 8 - 1);

/* --------------------------------------------------------------------- */

Bitset::Bitset(const void* place, const size_t size) {
	//TODO: add assert for size == 0
	m_size = size;

	// the number of processor native elements (rounded up)
	size_t elements = (size / Bitset::BITS) + ((size & Bitset::MOD_MASK) > 0 ? 1 : 0);
	// store the number of elements
	m_elements = elements;

	unative_t* begin = (unative_t *)place;
	if (begin == NULL) {
		//TODO: add new operator!
		//begin = new unative_t[elements];
	}

	m_begin = begin;

	// clear the container
	while (elements-- > 0) {
		*begin++ = 0;
	}
}

/* --------------------------------------------------------------------- */

bool Bitset::bit(const size_t pos, const bool value) {
	// check range
	if (pos > m_size) return false;

	if (value) {
		// set the bit
		m_begin[pos / Bitset::BITS] |= (Bitset::HEAD >> (pos & Bitset::MOD_MASK));
	} else {
		// clear the bit
		m_begin[pos / Bitset::BITS] &= ~(Bitset::HEAD >> (pos & Bitset::MOD_MASK));
	}

	return true;
}

/* --------------------------------------------------------------------- */

size_t Bitset::empty(const size_t from, size_t enough) const {
	// check range
	if (from > m_size) return 0;

	// if using default parameter for enough, set it to maximal possible
	if (enough == 0) {
		enough = m_size;
	}

	// initialize result
	size_t res = 0;
	// mask used to cycle trough the bits of native type
	unative_t mask = Bitset::HEAD;

	// the pointer to the byte which contains the bit on position 'from'
	unative_t* ptr = m_begin + (from / Bitset::BITS);
	// the position in the byte from where to start the check
	uint8_t bit = from & Bitset::MOD_MASK;

	if ((*ptr & (Bitset::MASK >> bit)) != 0) {
		// check each bit (start from 'bit')
		mask >>= bit;
		while ((*ptr & mask) == 0) {
			++res;
			mask >>= 1;
		}
		// return the result
		return res;
	} else {
		res += Bitset::BITS - bit;
		++ptr;

		// cycle trough the container while zeros, not enough or container end
		while (*ptr == 0) {
			res += Bitset::BITS;
			++ptr;

			// if end of the container reached
			if (ptr >= (m_begin + m_elements)) {
				// number of bits stored in the last native type
				for (size_t i = m_size & Bitset::MOD_MASK; i > 0; --i) {
					if ((*ptr & mask) == 0) {
						++res;
					}
				}
				// return only the real count of empty fields (without the alignment)
				return res;
			}

			// if enough and not on the end of container, return the number
			if (res >= enough) {
				return res;
			}
		}
	}

	// in ptr we have the first non zero value
	unative_t value = *ptr;

	// test the first half
	if ((value & (Bitset::MASK << (Bitset::BITS / 2))) != 0) {
		while ((value & mask) == 0) {
			++res;
			mask >>= 1;
			// security? if (mask == 0) break;
		}
	} else {
		res += Bitset::BITS / 2;
		mask >>= Bitset::BITS / 2;
		while ((value & mask) == 0) {
			++res;
			mask >>= 1;
			// security? if (mask == 0) break;
		}
	}

	return res;
}

/* --------------------------------------------------------------------- */

bool Bitset::bits(const size_t from, size_t count, const bool value) {
	// check range
	if ((from + count > m_size) || (count == 0)) return false;

	size_t modulo = from & Bitset::MOD_MASK;
	unative_t* ptr = m_begin + (from / Bitset::BITS);

	// if count is in the same native type (no iteration necessary)
	if (count < (Bitset::BITS - modulo)) {
		// creating the special mask
		size_t shift = Bitset::BITS - count;
		unative_t mask = (Bitset::MASK >> shift) << (shift - modulo);
		if (value) {
			// setting the bits
			*ptr |= mask;
		} else {
			// clearing the bits
			*ptr &= ~mask;
		}
	} else {
		// setting and clearing bits in the whole native type
		if (value) {
			// setting bits

			// first part
			*ptr |= Bitset::MASK >> modulo;
			++ptr;
			count -= Bitset::BITS - modulo;
			// middle
			while (count > Bitset::BITS) {
				*ptr |= Bitset::MASK;
				++ptr;
				count -= Bitset::BITS;
			}
			// last part
			*ptr |= Bitset::MASK << (Bitset::BITS - count);
		} else {
			// clearing bits

			// first part
			*ptr &= modulo ? (Bitset::MASK << (Bitset::BITS - modulo)) : 0;
			++ptr;
			count -= Bitset::BITS - modulo;
			// middle
			while (count >= Bitset::BITS) {
				*ptr &= 0;
				++ptr;
				count -= Bitset::BITS;
			}
			// last part
			*ptr &= Bitset::MASK >> count;
		}
	}

	return true;
}

