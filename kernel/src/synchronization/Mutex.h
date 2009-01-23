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
 * @brief Declaration and definition of a wrapper class on mutex locks providing
 * object oriented API.
 *
 * Mutex class holds one mutex lock and provides interface (member functions)
 * to lock() and unlock() it.
 */

#pragma once

#include "api.h"
#include "Time.h"
#include "synchronization/MutexManager.h"

/**
 * @class Mutex Mutex.h "Mutex.h"
 * @brief Class with C++ class interface for mutex struct.
 *
 * Class wich holds one mutex lock and provides interface (member functions)
 * to lock() and unlock() it. All member functions are defined as inline.
 */
class Mutex {
public:
	/** Initialize the mutex lock. */
	inline Mutex() {
		mutex_init(&m_mutex);
	}

	/** Destroy the mutex lock. */
	inline ~Mutex() {
		mutex_destroy(&m_mutex);
	}

	/** Lock the mutex. */
	inline void lock() {
		mutex_lock(&m_mutex);
	}

	/**
	 * Lock the mutex within a timelimit.
	 *
	 * @param time Timelimit for trying to lock the mutex.
	 * @return EOK on success (we locked the lock) and ETIMEDOUT if failed to lock it.
	 */
	inline int lockTimeout(const Time time) {
		return MutexManager::instance().mutex_lock_timeout(&m_mutex, time);
	}

	/** Unlock the mutex. */
	inline void unlock() {
		mutex_unlock(&m_mutex);
	}

private:
	/** The lock. */
	mutex_t m_mutex;
};

