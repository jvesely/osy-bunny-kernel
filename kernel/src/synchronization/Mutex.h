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
 * @brief Mutex lock - synchronization primitive for mutual exclusion.
 *
 * Mutex (mutual exclusion) is a common synchronization technique. The Mutex
 * class provides interface to init (constructor), lock, unlock and destroy
 * (destructor) a single mutex lock. It manages the waiting list of the threads
 * locked on the mutex. It dequeues/enques the threads in Scheduler.
 */

#pragma once

#include "api.h"

#include "Time.h"
#include "structures/List.h"

class Thread;
typedef List<Thread*> ThreadList;

/**
 * @class Mutex Mutex.h "Mutex.h"
 * @brief Mutex lock - synchronization primitive for mutual exclusion.
 *
 * Mutex (mutual exclusion) is a common synchronization technique. The Mutex
 * class provides interface to init (constructor), lock, unlock and destroy
 * (destructor) a single mutex lock. It manages the waiting list of the threads
 * locked on the mutex. It dequeues/enques the threads in Scheduler.
 */
class Mutex {
public:
	/**
	 * Initialize the mutex lock (to unlocked state).
	 */
	Mutex();

	/**
	 * Destroy the mutex lock. Remove it from all kernel structures.
	 * If there are any locked threads on this mutex, destroy will cause panic.
	 */
	virtual ~Mutex();

	/** 
	 * Lock the mutex. If the mutex is already locked, blocks untill it is unlocked.
	 */
	void lock();

	/**
	 * Lock the mutex, but don't let it take more than the given timelimit.
	 *
	 * @param time Timelimit for trying to lock the mutex.
	 * @return EOK on success (we locked the lock) and ETIMEDOUT if failed to lock it.
	 */
	int lockTimeout(const Time time);

	/**
	 * Unlock the mutex.
	 *
	 * @param safe Whether to check if the mutex is being unlocked by the owner
	 * (locking thread). Default is true.
	 */
	void unlock(bool safe = true);

private:
	/**
	 * Identification of the thread, for which is the lock locked. If 0 (zero)
	 * the lock is unlocked.
	 */
	volatile thread_t m_locked;

	/**
	 * The list of blocked (waiting) threads on this mutex.
	 */
	ThreadList m_waitingList;

};

