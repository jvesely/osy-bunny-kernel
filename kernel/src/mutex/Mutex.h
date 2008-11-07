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
 * @brief Declaration and definition of two wrapper classes on mutex locks providing
 * object oriented API.
 *
 * Mutex class holds one mutex lock and provides interface (member functions)
 * to lock() and unlock() it.
 * MutexLocker locks the mutex lock (given in constructor) on construction
 * and unlocks it on destruction (eg stack unwinding).
 */

#pragma once

#include "api.h"

/**
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

	/** Unlock the mutex. */
	inline void unlock() {
		mutex_unlock(&m_mutex);
	}

private:
	/** The lock. */
	mutex m_mutex;
};

/* --------------------------------------------------------------------- */

/**
 * MutexLocker locks the mutex lock (given in constructor) on construction
 * and unlocks it on destruction (eg stack unwinding). Should be used as a
 * local variable (allocated on stack) to make the destructor do the work
 * automatically on function end. The member functions are defined as inline
 * functions.
 */
class MutexLocker {
public:
	/**
	 * Constructor locks the mutex lock given in parameter.
	 *
	 * @param mutex The lock passed by reference.
	 */
	inline MutexLocker(Mutex& mutex): m_mutex(mutex) {
		m_mutex.lock();
	}

	/**
	 * Desctructor which unlocks the lock given in the constructor.
	 */
	inline ~MutexLocker() {
		m_mutex.unlock();
	}

private:
	Mutex& m_mutex;
};

