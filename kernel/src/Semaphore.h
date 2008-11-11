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
 * @brief Semaphore implementation.
 * @see Atomic data type.
 *
 * Semaphore is a common synchronization technique using a counter, which
 * can be incremented or decremented. It never drops under zero and if
 * who will want to decrement under zero will be blocked till the counter
 * is not more than zero.
 * Largely inspired by atomic_t from Kalisto kernel, great thanks.
 */

#pragma once

#include "types.h"

/**
 * @class Semaphore Semaphore.h "Semaphore.h"
 * @brief Semaphore synchronization.
 *
 * Semaphore is a common synchronization technique using a counter, which
 * can be incremented or decremented. It never drops under zero and if
 * who will want to decrement under zero will be blocked till the counter
 * is not more than zero.
 * Largely inspired by atomic_t from Kalisto kernel, great thanks.
 */
class Semaphore {
public:

	/**
	 * Constructor of semaphore initializes the semaphore to the given value.
	 *
	 * @param value The initial value of the semaphore.
	 */
	inline Semaphore(unative_t value): m_value(value) {}

	/**
	 * Get the value of the semaphore counter. Using the fact, that reading
	 * an integer is atomic on MIPS. This is the only function which casts
	 * (converts) Semaphore to integer (no implicit cast operator).
	 *
	 * @return The value of the internal counter.
	 */
	inline unative_t get() const;

	/**
	 * Prefix increment operator used to lift up the semaphore. It could be faster
	 * than up().
	 */
	inline void operator++ ();

	/**
	 * Postfix increment operator is wrapper to the prefix increment operator.
	 */
	inline void operator++ (int);

	/**
	 * Prefix decrement operator is wrapper to operator -=.
	 */
	inline void operator-- ();

	/**
	 * Postfix decrement operator is wrapper to operator -=.
	 */
	inline void operator-- (int);

	/**
	 * Lift up the semaphore.
	 *
	 * @param number The value to lift.
	 * @return The atomic variable with its new value by reference.
	 */
	inline void operator+= (const unative_t number);

	/**
	 * Push down the semaphore. If the semaphore value is zero, this call blocks,
	 * till its not lifted up by someone else.
	 *
	 * @param number The value to substract from the semaphore.
	 */
	inline void operator-= (const unative_t number);

	/**
	 * Wrapper to operator +=.
	 */
	inline void up(const unative_t number);

	/**
	 * Wrapper to operator -=.
	 */
	inline void down(const unative_t number);

	/**
	 * Not finished timeout semaphore lock!
	 * Lock the semaphore (push it down), but don't let it take more
	 * than the given timelimit in microseconds.
	 *
	 * @param usec Timelimit in microseconds for trying to lock the semaphore.
	 */
	inline int downTimeout(const unative_t number, const unsigned int usec);

private:
	/**
	 * Variable holding the value of the Semaphore counter. Its size is native for the
	 * CPU (registers and memory).
	 */
	volatile unative_t m_value;

	/** Deny copy-constructor. */
	Semaphore(Semaphore&);

	/** Deny assignment. */
	Semaphore& operator=(const Semaphore&);

};

/* --------------------------------------------------------------------- */

inline unative_t Semaphore::get() const {
	return m_value;
}

/* --------------------------------------------------------------------- */

inline void Semaphore::operator++ () {
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
}

inline void Semaphore::operator++ (int) {
	this->operator++();
}

/* --------------------------------------------------------------------- */

inline void Semaphore::operator-- () {
	// no subiu (substract with immediate) instruction :-(
	return operator-=(1);
}

inline void Semaphore::operator-- (int) {
	// no subiu (substract with immediate) instruction :-(
	return operator-=(1);
}

/* --------------------------------------------------------------------- */

inline void Semaphore::operator+= (const unative_t number) {
	register unative_t temp, result;

	asm volatile (
		".set push\n"
		".set noreorder\n"

		"1:\n"
		"  ll %[temp], %[value]\n"
		"  addu %[result], %[temp], %[number]\n"
		"  sc %[result], %[value]\n"
		"  beqz %[result], 1b\n"
		"  nop\n"

		".set pop\n"
		: [temp] "=&r" (temp), [result] "=&r" (result), [value] "+m" (m_value)
		: [number] "Ir" (number)
		: "memory"
	);
}

/* --------------------------------------------------------------------- */

inline void Semaphore::operator-= (const unative_t number) {
	register unative_t temp, result;

	//TODO: make it not go under 0
	asm volatile (
		".set push\n"
		".set noreorder\n"

		"1:\n"
		"  ll %[temp], %[value]\n"
		"  subu %[result], %[temp], %[number]\n"
		"  sc %[result], %[value]\n"
		"  beqz %[result], 1b\n"
		"  nop\n"

		".set pop\n"
		: [temp] "=&r" (temp), [result] "=&r" (result), [value] "+m" (m_value)
		: [number] "Ir" (number)
		: "memory"
	);
}

/* --------------------------------------------------------------------- */

inline void Semaphore::up(const unative_t number) {
	this->operator+=(number);
}

inline void Semaphore::down(const unative_t number) {
	this->operator-=(number);
}

/* --------------------------------------------------------------------- */

inline int Semaphore::downTimeout(const unative_t number, const unsigned int usec) {
	return ETIMEDOUT;
}

