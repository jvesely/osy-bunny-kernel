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
 * @brief Atomic data type (inspired by Kalisto).
 *
 * Basic primitives for atomic variables. The implementation here uses the
 * LL (load linked) and SC (store conditional) instruction pair to achieve
 * atomicity of certain operations. 
 * Largely inspired by Kalisto kernel, great thanks.
 */

#pragma once

#include "types.h"

/**
 * @class Atomic atomic.h "atomic.h"
 * @brief Atomic variable.
 *
 * The atomic data type is opaque to the user to prevent
 * access by other than atomic operations. It is also small enough to be
 * passed by value (one CPU native variable and no virtual member functions).
 * All the functions implement an optimistic algorithm that keeps trying
 * to add/subtract, inc/dec until the LL and SC pair of instructions
 * succeeds, which means there was no other access to the same variable
 * in the meantime.
 */
class Atomic {
public:

	inline Atomic(native_t value = 0): m_value(value) {}

	/**
	 * Get the value of an atomic variable. Using the fact, that reading
	 * an integer is atomic on MIPS. This is the only function which casts
	 * Atomic variable to integer (no implicit cast operator).
	 *
	 * @return The value.
	 */
	inline native_t get() const;

	/**
	 * Set the value of the atomic variable. Using the fact, that writing
	 * an integer is atomic on MIPS.
	 *
	 * @param value The value to write.
	 */
	inline void set(const native_t value);

	/**
	 * Prefix increment operator.
	 *
	 * @return
	 */
	inline Atomic& operator++ ();

	/**
	 * Prefix decrement operator.
	 *
	 * @return
	 */
	inline Atomic& operator-- ();

	/**
	 * Add to the value of the atomic variable.
	 *
	 * @param number The value to add.
	 * @return The atomic variable with its new value by reference.
	 */
	inline Atomic& operator+= (const native_t number);

	/**
	 * Subtract from the value of an atomic variable.
	 *
	 * @param number The value to substract.
	 * @return The atomic variable with its new value by reference.
	 */
	inline Atomic& operator-= (const native_t number);

private:
	/**
	 * Variable holding the value of the Atomic type. Its size is native for the
	 * CPU (registers and memory).
	 */
	volatile native_t m_value;

	/** Deny copy-constructor. */
	Atomic(Atomic&);

	/** Deny assignment. */
	Atomic& operator=(const Atomic&);

	/** Deny postfix increment. */
	Atomic operator++ (int);

	/** Deny postfix decrement. */
	Atomic operator-- (int);
};

/* --------------------------------------------------------------------- */

/**
 * @typedef Atomic atomic_t
 * @brief C-style name for Atomic.
 *
 * Atomic is a C++ name for a class, while this class is just a cover of one
 * simple integer type.
 */
typedef Atomic atomic_t;

/* --------------------------------------------------------------------- */

inline native_t Atomic::get() const {
	return m_value;
}

/* --------------------------------------------------------------------- */

inline void Atomic::set(const native_t value) {
	m_value = value;
}

/* --------------------------------------------------------------------- */

inline Atomic& Atomic::operator++ () {
	register native_t temp, result;

	asm volatile (
		".set push\n"
		".set noreorder\n"

		"1:\n"
		"  ll %[temp], %[value]\n"
		"  addiu %[result], %[temp], 1\n"
		"  sc %[result], %[value]\n"
		"  beqz %[result], 1b\n"
		"  nop\n"

		".set pop\n"
		: [temp] "=&r" (temp), [result] "=&r" (result), [value] "+m" (m_value)
		:
		: "memory"
	);

	return *this;
}

/* --------------------------------------------------------------------- */

inline Atomic& Atomic::operator-- () {
	register native_t temp;

	asm volatile (
		".set push\n"
		".set noreorder\n"

		"1:\n"
		"  ll %[temp], %[value]\n"
		"  subu %[temp], %[temp], 1\n"
		"  sc %[temp], %[value]\n"
		"  beqz %[temp], 1b\n"
		"  nop\n"

		".set pop\n"
		: [temp] "=&r" (temp), [value] "+m" (m_value)
		:
		: "memory"
	);

	return *this;
}

/* --------------------------------------------------------------------- */

inline Atomic& Atomic::operator+= (const native_t number) {
	register native_t temp;

	asm volatile (
		".set push\n"
		".set noreorder\n"

		"1:\n"
		"  ll %[temp], %[value]\n"
		"  addu %[temp], %[temp], %[number]\n"
		"  sc %[temp], %[value]\n"
		"  beqz %[temp], 1b\n"
		"  nop\n"

		".set pop\n"
		: [temp] "=&r" (temp), [value] "+m" (m_value)
		: [number] "Ir" (number)
		: "memory"
	);

	return *this;
}

/* --------------------------------------------------------------------- */

inline Atomic& Atomic::operator-= (const native_t number) {
	register native_t temp;

	asm volatile (
		".set push\n"
		".set noreorder\n"

		"1:\n"
		"  ll %[temp], %[value]\n"
		"  subu %[temp], %[temp], %[number]\n"
		"  sc %[temp], %[value]\n"
		"  beqz %[temp], 1b\n"
		"  nop\n"

		".set pop\n"
		: [temp] "=&r" (temp), [value] "+m" (m_value)
		: [number] "Ir" (number)
		: "memory"
	);

	return *this;
}

