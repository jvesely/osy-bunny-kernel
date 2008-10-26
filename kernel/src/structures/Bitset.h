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
	 * @param place Allocated space, where bitset's container can be placed. If null, it will allocate space dynamically.
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
	 * Bit getter.
	 *
	 * @param pos The posision of the bit we want to query.
	 * @return Whether the bit was set or not. (True for set.)
	 */
	bool bit(const size_t pos) const __attribute__((always_inline));

	/**
	 * Bit setter.
	 *
	 * @param pos The posision of the bit we want to set or clear.
	 * @param value The value we want the bit to have. True is to set the bit and false is to clear the bit.
	 * @return Whether the operation was successful.
	 */
	bool bit(const size_t pos, const bool value);

	/**
	 * Query how many bits are clear after the given position.
	 *
	 * @param from The position from where to start counting zeros.
	 * @param enough The number of bits that is enough. Use it to get results faster.
	 * @return The number of free (cleared) bits from the given position (including the positions bit).
	 */
	size_t empty(const size_t from, size_t enough = 0) const;

	/**
	 * Set or clear more bits in a row.
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

	/** The CPU native type with all bits set except the first on. Used for modulo, since the native type in bits is power of 2. */
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

/*
 * Small exmple of Bitset usage.
 * Compile like: g++ -Wall Bitset.cpp main.cxx
 *
 * #include <iostream>
 * #include <iomanip>
 * 
 * #include "Bitset.h"
 * 
 * using namespace std;
 * 
 * const size_t size = 11;
 *
 * void set_bit(Bitset* b, size_t pos) {
 * 	cerr << "Setting bit " << setw(10) << dec << pos << " : " << setw(15) << (b->bit(pos, true) ? "done" : "failed") << endl;
 * }
 * 
 * void dump(Bitset* b) {
 * 	for (size_t i = 0; i < 2 * size; i++) {
 * 		for (int j = 0; j < 16; j++) {
 * 			cerr << setw(5) << (i * 16 + j) << " : " << b->bit(i * 16 + j) << ((j == 7) ? "  |" : "");
 * 		}
 * 		cerr << endl;
 * 		if (i % 2) cerr << endl;
 * 	}
 * }
 * 
 * void dump(unative_t* ptr) {
 * 	for (size_t i = 0; i < size; i++) {
 * 		cerr << setw(3) << setfill('0') << dec << i << " 0x" << hex << *ptr++ << setfill(' ') << dec << endl;
 * 	}
 * }
 * 
 * int main() {
 * 
 * 	unative_t *arr = new unative_t[size];
 * 	unative_t *ptr = arr;
 * 	Bitset *b = new Bitset(arr, size * 32);
 * 
 * 	cerr << "The container size will be " << Bitset::getContainerSize(size * 32) << " bytes for " << (size * 32) << " bool values." << endl << endl;
 * 
 * 	size_t i = 0;
 * 
 * 	// set each byte to 00000001b 
 * 	set_bit(b, i + 7);
 * 	set_bit(b, i + 15);
 * 	set_bit(b, i + 23);
 * 	set_bit(b, i + 31);
 * 	i += 32;
 * 
 * 	// set bytes to 1 2 3 4
 * 	set_bit(b, i + 7);  // 1
 * 	set_bit(b, i + 14); // 2
 * 	set_bit(b, i + 22); // 3
 * 	set_bit(b, i + 23); // 3
 * 	set_bit(b, i + 29); // 4
 * 	i += 32;
 * 
 * 	// write DEAD BEEF
 * 	#define I set_bit(b, i++);
 * 	#define O i++;
 * 	// on little endian it has to be from back EF'BE'AD'DE (1 byte is 2hex chars)
 * 	// but because of the Bitset is working with native type (uint32), the byte
 * 	// ordering is applied also while inserting it
 * 	I I O I  I I I O // 0xDE
 * 	I O I O  I I O I // 0xAD
 * 	I O I I  I I I O // 0xBE
 * 	I I I O  I I I I // 0xEF
 * 
 * 	// trying out of range (returns false)
 * 	set_bit(b, 452425);
 * 
 * 	cerr << endl << "Dumping the data structure:" << endl;
 * 	dump(b);
 * 	dump(arr);
 * 
 * 	cerr << endl;
 * 
 * 	cerr << "Cleared bits from  0 till end: " << dec << b->empty(0) << endl;
 * 	cerr << "Cleared bits from 50 till end: " << dec << b->empty(50) << endl;
 * 	cerr << "Cleared bits from 'out of range': " << dec << b->empty(12314350) << endl;
 * 
 * 	cerr << endl;
 * 
 * 	cerr << "Set 20 bits from 96: " << (b->bits(96, 20, true) ? "done" : "failed") << endl;
 * 	cerr << "Set 7 bits from 125: " << (b->bits(125, 7, true) ? "done" : "failed") << endl;
 * 	cerr << "Set 0 bits from 135: " << (b->bits(135, 0, true) ? "done" : "failed") << endl;
 * 	cerr << "Set 1 bit from 135: " << (b->bits(135, 1, true) ? "done" : "failed") << endl;
 * 	cerr << "Set 150 bits from 138: " << (b->bits(138, 150, true) ? "done" : "failed") << endl;
 * 
 * 	cerr << endl << "Dumping the data structure:" << endl;
 * 	dump(b);
 * 	dump(arr);
 * 
 * 	cerr << "Clear 48 bits from 0: " << (b->bits(0, 48, false) ? "done" : "failed") << endl;
 * 	cerr << "Clear 20 bits from 96: " << (b->bits(96, 20, false) ? "done" : "failed") << endl;
 * 	cerr << "Clear 7 bits from 125: " << (b->bits(125, 7, false) ? "done" : "failed") << endl;
 * 	cerr << "Clear 0 bits from 135: " << (b->bits(135, 0, false) ? "done" : "failed") << endl;
 * 	cerr << "Clear 1 bit from 135: " << (b->bits(135, 1, false) ? "done" : "failed") << endl;
 * 	cerr << "Set 150 bits from 138: " << (b->bits(138, 150, false) ? "done" : "failed") << endl;
 * 
 * 	cerr << endl << "Dumping the data structure:" << endl;
 * 	dump(b);
 * 	dump(arr);
 * 
 * 	delete b;
 * 	delete ptr;
 * 
 * 	return 0;
 * }
 *
 */


