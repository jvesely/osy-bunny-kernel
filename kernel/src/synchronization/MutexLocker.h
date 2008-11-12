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
 * @brief Declaration and definition of a wrapper class on the Mutex class
 * providing an easy to use interface which automatically locks and unlocks
 * the Mutex lock.
 *
 * MutexLocker locks the mutex lock (given in constructor) on construction
 * and unlocks it on destruction (eg stack unwinding).
 */

#pragma once

#include "Mutex.h"

/**
 * @class MutexLocker Mutex.h "Mutex.h"
 * @brief Mutex locking class used for locking a Mutex in a block.
 *
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
	/** The reference to lock from the constructor. */
	Mutex& m_mutex;

	/** Deny copy-constructor. */
	MutexLocker(const MutexLocker&);
	
	/** Deny assignment. */
	MutexLocker& operator=(const MutexLocker&);

	/** Deny creation on heap, because it is useless for this class. */
	void* operator new(unsigned int);
};

