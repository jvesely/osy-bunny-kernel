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
 * @brief Semaphore declaration and implementation of the inline member functions.
 *
 * Semaphore is a common synchronization technique using a counter, which
 * can be incremented or decremented. It never drops under zero and if
 * who will want to decrement under zero will be blocked till the counter
 * is not more than zero.
 */

#pragma once

#include "api.h"
#include "types.h"
#include "Time.h"
#include "structures/List.h"

class Thread;

typedef List<Thread*> ThreadList;

/**
 * @class Semaphore Semaphore.h "Semaphore.h"
 * @brief Semaphore synchronization.
 *
 * Semaphore is a common synchronization technique using a counter, which
 * can be incremented or decremented. It never drops under zero and if
 * who will want to decrement under zero will be blocked till the counter
 * is not more than zero.
 */
class Semaphore {
public:

	/**
	 * Constructor of Semaphore initializes the semaphore to the given value.
	 *
	 * @param value The initial value of the semaphore counter.
	 */
	inline Semaphore(unative_t value): m_counter(value) {}

	/**
	 * While destructing the Semaphore object, check if there are no other
	 * threads waiting for it. If somebody is still locked on this semaphore,
	 * panic.
	 */
	~Semaphore();

	/**
	 * Get the value of the semaphore counter. Using the fact, that reading
	 * an integer is atomic on MIPS. This is the only function which casts
	 * (converts) Semaphore to integer (no implicit cast operator).
	 *
	 * @return The value of the internal counter.
	 */
	inline unative_t get() const;

	/**
	 * Lift up the semaphore.
	 *
	 * @param number The value to lift (default is 1).
	 */
	void up(const unative_t number = 1);

	/**
	 * Push down the semaphore. If the semaphore counter is zero, this call blocks,
	 * till its not lifted up by someone else.
	 *
	 * @param number The value to substract from the semaphore (default is 1).
	 */
	void down(const unative_t number = 1);

	/**
	 * Try to lock the semaphore (push it down), but don't let it take more
	 * than the given timelimit.
	 *
	 * @param number The value to substract from the semaphore (default is 1).
	 * @param time Timelimit for trying to lock the semaphore (default is 0).
	 */
	int downTimeout(const unative_t number = 1, const Time time = Time(0, 0));

	/**
	 * Postfix increment operator is used to lift up the semaphore by one (wrapper
	 * to the up() member function).
	 *
	 * @note The operator doesn't return reference to the object itself, because
	 * nesting calls to Semaphore doesn't guarante atomicity, copying is denied
	 * and it is even a wrapper to a void function. If you don't like this
	 * behaviour, just use up(1).
	 */
	inline void operator++ (int);

	/**
	 * Postfix decrement operator is used to push down the semaphore by one (wrapper
	 * to the down() member function). If semaphore counter is zero, this call blocks
	 * till semaphore is not lifted up by someone else.
	 *
	 * @note The operator doesn't return reference to the object itself, because
	 * nesting calls to Semaphore doesn't guarante atomicity, copying is denied
	 * and it is even a wrapper to a void function. If you don't like this
	 * behaviour, just use up(1).
	 */
	inline void operator-- (int);

private:
	/**
	 * Variable holding the value of the Semaphore counter. Its size is native for the
	 * CPU (registers and memory).
	 */
	volatile unative_t m_counter;

	/** The list of blocked (waiting) threads on this semaphore. */
	ThreadList waitingList;

	/** Deny copy-constructor. */
	Semaphore(Semaphore&);

	/** Deny assignment. */
	Semaphore& operator=(const Semaphore&);

};

/* --------------------------------------------------------------------- */
/*
inline Semaphore::~Semaphore() {
	// if the waiting list is not empty, panic
	if (waitingList.size() != 0) {
		panic("Semaphore at %x being destroyed by thread %u while there are still locked threads waiting for it!\n",
			this, Thread::getCurrent()->id());
	}
}
*/
/* --------------------------------------------------------------------- */

inline unative_t Semaphore::get() const {
	return m_counter;
}

/* --------------------------------------------------------------------- */

inline void Semaphore::operator++ (int) {
	up(1);
}

/* --------------------------------------------------------------------- */

inline void Semaphore::operator-- (int) {
	down(1);
}

