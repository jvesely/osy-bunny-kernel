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

//#define BITSET_DEBUG

#ifndef BITSET_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...) \
	printf("[ BITSET DEBUG ]: "); \
	printf(ARGS);
#endif


const size_t Bitset::BITS = sizeof(unative_t) * 8;
const unative_t Bitset::MASK = ~0;
const unative_t Bitset::MOD_MASK = Bitset::BITS - 1;
const unative_t Bitset::HEAD = 1 << (Bitset::BITS - 1);

/* --------------------------------------------------------------------- */

Bitset::Bitset(const void* place, const size_t size) {
	ASSERT(size != 0);
	m_size = size;

	// the number of processor native elements (rounded up)
	size_t elements = (size / Bitset::BITS) + ((size & Bitset::MOD_MASK) > 0 ? 1 : 0);
	// store the number of elements
	m_elements = elements;

	// check if the given place pointer is aligned
	ASSERT(((int)place % sizeof(unative_t) == 0));

	unative_t* begin = (unative_t *)place;
	if (begin == NULL) {
		// if place is not given, just allocate some
		begin = new unative_t[elements];
	}

	m_begin = begin;

	// clear the container
	while (elements-- > 0) {
		*begin++ = 0;
	}

	PRINT_DEBUG("Bitset of %d bits created at %x. Built from %d native elements.\n",
		m_size, m_begin, m_elements);
}

/* --------------------------------------------------------------------- */

bool Bitset::bit(const size_t pos, const bool value) {
	// check range
	if (pos >= m_size) return false;

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

bool Bitset::bits(const size_t from, size_t count, const bool value) {
	// check range
	if ((from + count > m_size) || (count == 0)) return false;

	size_t modulo = from & Bitset::MOD_MASK;
	unative_t* ptr = m_begin + (from / Bitset::BITS);

	// if count is in the same native type (no iteration necessary)
	if (count <= (Bitset::BITS - modulo)) {
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

/* --------------------------------------------------------------------- */

size_t Bitset::empty(const size_t from, size_t enough) const {
	// check range
	if (from >= m_size) return 0;

	// if using default parameter for enough, set it to maximal possible
	if (enough == 0) {
		enough = m_size;
	}

	// initialize the result
	size_t res = 0;
	// mask used to cycle trough the bits of native type
	unative_t mask = Bitset::HEAD;

	// the pointer to the byte which contains the bit on position 'from'
	unative_t* ptr = m_begin + (from / Bitset::BITS);
	// the position in the byte from where to start the check
	const uint8_t bit = from & Bitset::MOD_MASK;

	// the number of bits in the last element
	const size_t lastBits = ((m_size & Bitset::MOD_MASK) == 0)
		? Bitset::BITS
		: (m_size & Bitset::MOD_MASK);

	// check the first element (from the beginnig we need to cut off some first bits)
	if ((*ptr & (Bitset::MASK >> bit)) != 0) {
		// if there is a 1 somewhere
		// check each bit (start from 'bit' - thus shift the mask)
		mask >>= bit;

		// check if the first element is not the last one
		if (ptr >= (m_begin + m_elements - 1)) {
			// if so, go from bit number to the end, but only till m_size ends
			size_t count = lastBits - bit;
			while (((*ptr & mask) == 0) && (count > 0)) {
				++res;
				mask >>= 1;
				--count;
			}
		} else {
			// loop till 1 is found in the element (mask check is optional)
			while (((*ptr & mask) == 0) && mask) {
				++res;
				mask >>= 1;
			}
		}

		// finally return the result (if the first checked element contains 1)
		return (res > enough) ? enough : res;
	} else {
		// if the first checked element was full of zeros

		// check if we are not on the end of the container
		if (ptr >= (m_begin + m_elements - 1)) {
			// add only till end of m_size without the bit number
			res += lastBits - bit;
			return (res > enough) ? enough : res;
		}

		// add clear bits from the first element (without the first some)
		res += Bitset::BITS - bit;
		// increment pointer to the container and continue in a loop
		++ptr;

		// cycle trough the container while zeros, not enough or container end
		while (*ptr == 0) {
			res += Bitset::BITS;

			// if end of the container reached
			if (ptr >= (m_begin + m_elements - 1)) {
				// number of bits stored in the last native type is smaller
				res -= (Bitset::BITS - lastBits);
				// return the count without the alignment bits
				return (res > enough) ? enough : res;
			}

			// if enough and not on the end of container, return enough
			if (res >= enough) {
				return enough;
			}

			// move to the next native element
			++ptr;
		}
	}

	// in ptr we have the first non zero value
	const unative_t value = *ptr;
	const size_t halfNative = Bitset::BITS / 2;

	// check if we are not in the end of the container
	if (ptr >= (m_begin + m_elements - 1)) {
		// if so, go from bit number to the end, but only till m_size ends
		size_t count = lastBits;
		// check if we could optimalize
		if ((count >= halfNative) && ((value & (Bitset::MASK << halfNative)) == 0)) {
			// if so, just jump over half of the native type
			res += halfNative;
			mask >>= halfNative;
			count -= halfNative;
		}
		// loop trough elements
		while (((value & mask) == 0) && (count > 0)) {
			++res;
			mask >>= 1;
			--count;
		}

		return (res > enough) ? enough : res;
	}

	// if not the last element of the container, count the empty bits in the whole element
	if ((value & (Bitset::MASK << halfNative)) == 0) {
		// if the first half was full of zeros, jump over the first half
		res += halfNative;
		mask >>= halfNative;
	}
	// loop till end
	while (((value & mask) == 0) && mask) {
		++res;
		mask >>= 1;
	}

	return (res > enough) ? enough : res;
}

/* --------------------------------------------------------------------- */

size_t Bitset::full(const size_t from, size_t enough) const {
	// check range
	if (from >= m_size) return 0;

	// if using default parameter for enough, set it to maximal possible
	if (enough == 0) {
		enough = m_size;
	}

	// initialize the result
	size_t res = 0;
	// the pointer to the byte which contains the bit on position 'from'
	unative_t* ptr = m_begin + (from / Bitset::BITS);
	// the position in the byte from where to start the check
	const uint8_t bit = from & Bitset::MOD_MASK;

	// mask used to cycle trough the bits of native type
	unative_t mask = Bitset::HEAD;
	// shift the mask to fit in the first element
	mask >>= bit;

	size_t count = Bitset::BITS;

	// if the first element is the last one
	if (ptr >= (m_begin + m_elements - 1)) {
		count = ((m_size & Bitset::MOD_MASK) == 0)
			? Bitset::BITS - bit
			: (m_size & Bitset::MOD_MASK) - bit;
	}

	while (true) {
		// if ptr is behind our container
		if (ptr > (m_begin + m_elements - 1)) break;

		while (mask) {
			if (((*ptr & mask) == 0) || (count == 0)) {
				// we found a cleared bit, return the result
				return (res > enough) ? enough : res;
			}
			mask >>= 1;
			++res;
			--count;
		}

		mask = Bitset::HEAD;
		++ptr;

		// on the end of container
		if (ptr == (m_begin + m_elements - 1)) {
			count = ((m_size & Bitset::MOD_MASK) == 0)
				? Bitset::BITS
				: (m_size & Bitset::MOD_MASK);
		} else {
			count = Bitset::BITS;
		}
	}

	return (res > enough) ? enough : res;
}

