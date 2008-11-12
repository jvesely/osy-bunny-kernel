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
 */

#pragma once

#include "asm/atomic.h"
#include "proc/Scheduler.h"

class Spinlock
{
public:
	Spinlock(): m_locked(0), m_ieStatus(~STATUS_IE_MASK) {}

	inline void lock();

	inline void unlock();

	inline void lockActive();

	inline void lockYield();

private:
	/**
	 * Variable holding the value of the Spinlock. Its size is native for the
	 * CPU (registers and memory).
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
	m_ieStatus = Processor::save_and_disable_interrupts();
}

/* --------------------------------------------------------------------- */

inline void Spinlock::unlock() {
	swap(lock, 0);
}

/* --------------------------------------------------------------------- */

inline void Spinlock::lockActive() {
	while (swap(m_locked, 1)) {}
}

/* --------------------------------------------------------------------- */

inline void Spinlock::lockYield() {
	while (swap(m_lock, 1)) {
		Scheduler::instance().switchThread();
	}
}

