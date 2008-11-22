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
 * @brief Active semaphore implementation (mainly MIPS assembler).
 * @see Atomic data type.
 *
 * Semaphore is a common synchronization technique using a counter, which
 * can be incremented or decremented. It never drops under zero and
 * who will want to decrement under zero will be blocked till the counter
 * is not more than zero.
 * Largely inspired by atomic_t from Kalisto kernel, great thanks.
 */

#pragma once

#include "types.h"

/**
 * @class ActiveSemaphore ActiveSemaphore.h "ActiveSemaphore.h"
 * @brief Active (blocking) semaphore synchronization.
 *
 * Semaphore is a common synchronization technique using a counter, which
 * can be incremented or decremented. It never drops under zero and
 * who will want to decrement under zero will be blocked till the counter
 * is not more than zero.
 * Largely inspired by atomic_t from Kalisto kernel, great thanks.
 */
class ActiveSemaphore
{
public:
	/**
	 * Constructor of semaphore initializes the semaphore to the given value.
	 *
	 * @param value The initial value of the semaphore.
	 */
	inline ActiveSemaphore(unative_t value): m_value(value) {}

	/**
	 * Get the value of the semaphore counter. Using the fact, that reading
	 * an integer is atomic on MIPS. This is the only function which casts
	 * (converts) ActiveSemaphore to integer (no implicit cast operator).
	 *
	 * @return The value of the internal counter.
	 */
	inline unative_t get() const;

	/**
	 * Lift up the semaphore.
	 *
	 * @param number The value to lift.
	 * @return The atomic variable with its new value by reference.
	 */
	inline void up(const unative_t number);

	/**
	 * Push down the semaphore. If the semaphore value is zero, this call blocks,
	 * till its not lifted up by someone else.
	 *
	 * @param number The value to substract from the semaphore.
	 */
	inline void down(const unative_t number);

	/**
	 * Postfix increment operator is used to lift up the semaphore by one.
	 *
	 * @note The operator doesn't return reference to the object itself, because
	 * nesting calls to semaphore doesn't guarante atomicity and order. If you
	 * don't like this behaviour, just use up(1).
	 */
	inline void operator++ (int);

	/**
	 * Postfix decrement operator is used to push down the semaphore by one.
	 * If semaphore value is zero, this call blocks till semaphore is not
	 * lifted up by someone else.
	 *
	 * @note The operator doesn't return reference to the object itself, because
	 * nesting calls to semaphore doesn't guarante atomicity and order. If you
	 * don't like this behaviour, just use down(1).
	 */
	inline void operator-- (int);

private:
	/**
	 * Variable holding the value of the semaphore counter. Its size is native for the
	 * CPU (registers and memory).
	 */
	volatile unative_t m_value;

	/** Deny copy-constructor. */
	ActiveSemaphore(ActiveSemaphore&);

	/** Deny assignment. */
	ActiveSemaphore& operator=(const ActiveSemaphore&);

};

/* --------------------------------------------------------------------- */

inline unative_t ActiveSemaphore::get() const {
	return m_value;
}

/* --------------------------------------------------------------------- */

inline void ActiveSemaphore::up(const unative_t number) {
	register unative_t temp;

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
}

/* --------------------------------------------------------------------- */

inline void ActiveSemaphore::down(const unative_t number) {
	register unative_t temp;

	asm volatile (
		".set push\n"
		".set noreorder\n"

		"1:\n"
		"  ll %[temp], %[value]\n"
		"  subu %[temp], %[temp], %[number]\n"
		"  bltz %[temp], 1b\n"
		"  nop\n"
		"  sc %[temp], %[value]\n"
		"  beqz %[temp], 1b\n"
		"  nop\n"

		".set pop\n"
		: [temp] "=&r" (temp), [value] "+m" (m_value)
		: [number] "Ir" (number)
		: "memory"
	);
}
/* --------------------------------------------------------------------- */

inline void ActiveSemaphore::operator++ (int) {
	register native_t temp;

	asm volatile (
		".set push\n"
		".set noreorder\n"

		"1:\n"
		"  ll %[temp], %[value]\n"
		"  addiu %[temp], %[temp], 1\n"
		"  sc %[temp], %[value]\n"
		"  beqz %[temp], 1b\n"
		"  nop\n"

		".set pop\n"
		: [temp] "=&r" (temp), [value] "+m" (m_value)
		:
		: "memory"
	);
}

/* --------------------------------------------------------------------- */

inline void ActiveSemaphore::operator-- (int) {
	register unative_t temp;

	asm volatile (
		".set push\n"
		".set noreorder\n"

		"1:\n"
		"  ll %[temp], %[value]\n"
		"  subu %[temp], %[temp], 1\n"
		"  bltz %[temp], 1b\n"
		"  nop\n"
		"  sc %[temp], %[value]\n"
		"  beqz %[temp], 1b\n"
		"  nop\n"

		".set pop\n"
		: [temp] "=&r" (temp), [value] "+m" (m_value)
		:
		: "memory"
	);
}

