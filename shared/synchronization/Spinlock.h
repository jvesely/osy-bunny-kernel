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
 * @brief Spinlock implementation.
 *
 * Spinlock is a form of fast binary semaphore (mutex lock) with a very small
 * overhead. Waiting for a locked spinlock is active or with yield(), so it
 * is usefull if we expect the waiting time to be very little. There is no
 * waiting list, so the threads remains in the Scheduler. The implementation
 * is with atomic swap() (exchange) operation written in assembler.
 */

#pragma once

#include "atomic.h"

/**
 * @class Spinlock Spinlock.h "Spinlock.h"
 * @brief Spinlock is a fast binary semaphore (mutex with no owner checks).
 *
 * Spinlock with interface (member functions) to lock() and unlock() it. All
 * member functions are defined as inline.
 * Spinlock is a form of fast binary semaphore (mutex lock) with a very small
 * overhead. Waiting for a locked spinlock is active or with yield(), so it
 * is usefull if we expect the waiting time to be very little. There is no
 * waiting list, so the threads remains in the Scheduler. The implementation
 * is with atomic swap() (exchange) operation written in assembler.
 */
class Spinlock
{
public:
	/** After construction is the spinlock unlocked. */
	Spinlock(): m_locked(0) {}

	/**
	 * Lock the spinlock with active or passive waiting, depending on the
	 * processor count. The decision is done by compile-time depending on MULTICPU
	 * define, so there is no runtime overhead.
	 */
	inline void lock();

	/** Unlock the spinlock. */
	inline void unlock();

private:
	/*! This decides on the waiting style, default si spinning (nothing). */
	virtual void wait() {};

	/**
	 * Variable holding the state of the Spinlock. Its size is native for the
	 * CPU (registers and memory). 1 means locked and 0 means unlocked.
	 */
	volatile native_t m_locked;

	/** Deny copy-constructor. */
	Spinlock(Spinlock&);

	/** Deny assignment. */
	Spinlock& operator=(const Spinlock&);

};
/* --------------------------------------------------------------------- */
inline void Spinlock::lock()
{
	while (swap(m_locked, 1)) {
		wait();
	}
}
/* --------------------------------------------------------------------- */
inline void Spinlock::unlock()
{
	swap(m_locked, 0);
}
/* --------------------------------------------------------------------- */
