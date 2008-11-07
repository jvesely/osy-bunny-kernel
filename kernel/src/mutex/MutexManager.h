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
 * @brief MutexManager declaration.
 *
 * MutexManager is a singleton class providing interface to init, lock, unlock and
 * destroy mutex locks. It manages the mutex struct and the waiting lists for the
 * locked mutexes. It dequeues/enques the threads in Scheduler.
 */

#pragma once

#include <api.h>

#include "cpp.h"
#include "Singleton.h"
#include "structures/List.h"
#include "proc/Thread.h"

/**
 * MutexManager is a singleton class providing interface to init, lock, unlock and
 * destroy mutex locks. It manages the mutex struct and the waiting lists for the
 * locked mutexes. It dequeues/enques the threads in Scheduler.
 */
class MutexManager: public Singleton<MutexManager> {
public:
	/**
	 * Initialize the given mutex struct (to unlocked state).
	 *
	 * @param mtx Mutex struct to initialize.
	 */
	void mutex_init(mutex_t *mtx);

	/**
	 * Destroy the given mutex struct. Remove it from all kernel structures.
	 * If there are any locked threads on this mutex, destroy will cause panic.
	 *
	 * @param mtx Mutex struct to destroy.
	 */
	void mutex_destroy(mutex_t *mtx);

	/**
	 * Lock the given mutex. If the mutex is already locked, blocks untill it is unlocked.
	 *
	 * @param mtx Mutex to lock.
	 */
	void mutex_lock(mutex_t *mtx);

	/**
	 * Lock the given mutex, but don't let it take more than the given timelimit in microseconds.
	 *
	 * @param mtx Mutex to lock within time limit.
	 * @param usec Timelimit in microseconds for trying to lock the mutex.
	 */
	int mutex_lock_timeout(mutex_t *mtx, const unsigned int usec);

	/**
	 * Unlock the mutex.
	 *
	 * @param mtx Mutex struct to unlock.
	 */
	void mutex_unlock(mutex_t *mtx);

};

