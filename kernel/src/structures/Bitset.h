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
 * @brief Bitset declaration and inline function definitions.
 *
 * Bitset is a data structure to efficiently store bit values like true/false
 * or 0/1. This class stores the bits in array of types, which are native
 * to the used processor. In our case it is MIPS with 32 bit unsigned integers.
 * The class allows to create an instance of it with preallocated or dynamically
 * allocated container. It also allows querying and setting each bit,
 * querying how many free spaces (zeros) are after a given bit and also setting
 * more bits to a specified value as fast as possible.
 */

#pragma once

#include <api.h>
#include <types.h>

/**
 * @class Bitset Bitset.h "Bitset.h"
 * @brief Container for bit values.
 *
 * Bitset is a data structure to efficiently store bit values like true/false
 * or 0/1. This class stores the bits in array of types, which are native
 * to the used processor. In our case it is MIPS with 32 bit unsigned integers.
 * The class allows to create an instance of it with preallocated or dynamically
 * allocated container. It also allows querying and setting each bit,
 * querying how many free spaces (zeros) are after a given bit and also setting
 * more bits to a specified value as fast as possible.
 */
class Bitset {
public:
	/**
	 * Construct and initialize the bitset.
	 *
	 * @param place Allocated space, where bitset's container can be placed
	 *        (place have to be aligned!). If NULL, it will allocate space dynamically.
	 * @param size The number of bits to store.
	 */
	Bitset(const void* place, const size_t size);

	/**
	 * Calculate the space necessary to store 'size' bits in the memory.
	 *
	 * @param size Number of bits to store.
	 * @return The number of bytes (rounded up) the Bitset needs.
	 */
	static size_t getContainerSize(const size_t size) __attribute__((always_inline));

	/**
	 * Bit getter. The bit position is counted from 0.
	 *
	 * @param pos The posision of the bit we want to query.
	 * @return Whether the bit was set or not. (True for set.)
	 */
	bool bit(const size_t pos) const __attribute__((always_inline));

	/**
	 * Bit setter. The bit position is counted from 0.
	 *
	 * @param pos The posision of the bit we want to set or clear.
	 * @param value The value we want the bit to have. True is to set the bit and false is to clear the bit.
	 * @return Whether the operation was successful.
	 */
	bool bit(const size_t pos, const bool value);

	/**
	 * Query how many bits are clear after the given position. The bit position is counted from 0.
	 *
	 * @param from The position from where to start counting zeros.
	 * @param enough The number of bits that is enough. Use it to get results faster.
	 * @return The number of free (cleared) bits from the given position (including the positions bit).
	 */
	size_t empty(const size_t from, size_t enough = 0) const;

	/**
	 * Set or clear more bits in a row. The bit position is counted from 0.
	 *
	 * @param from The position from where to start setting or clearing bits.
	 * @param count The number of bits we want to set (including the starting bit).
	 * @param value Whether we are setting or clearing the bits. True is to set and false is to clear the bits.
	 * @return Whether the operation was successful.
	 */
	bool bits(const size_t from, size_t count, const bool value);

protected:

private:
	/** Pointer to the container of bits. */
	unative_t* m_begin;

	/** The container size from outside. Used for range checks. */
	size_t m_size;

	/** Number of processor native elements. */
	size_t m_elements;

	/** Size of the CPU native type in bits. */
	static const size_t BITS;

	/** The CPU native type with all bits set. Mask full of ones. */
	static const unative_t MASK;

	/**
	 * The CPU native type with all bits set except the first on.
	 * Used for modulo, since the native type in bits is power of 2.
	 */
	static const unative_t MOD_MASK;

	/** The CPU native type with only one bit set on the first position. */
	static const unative_t HEAD;

};

/* --------------------------------------------------------------------- */

inline size_t Bitset::getContainerSize(const size_t size) {
	return ((size / Bitset::BITS) + ((size & Bitset::MOD_MASK) > 0 ? 1 : 0)) * sizeof(unative_t);
}

/* --------------------------------------------------------------------- */

inline bool Bitset::bit(const size_t pos) const {
	return *(m_begin + (pos / Bitset::BITS)) & (Bitset::HEAD >> (pos & Bitset::MOD_MASK));
}

