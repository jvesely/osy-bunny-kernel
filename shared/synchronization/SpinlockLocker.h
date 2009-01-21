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
 * @brief Declaration and definition of a wrapper class on the Spinlock class
 * providing an easy to use interface which automatically locks and unlocks
 * the Spinlock lock.
 *
 * SpinlockLocker locks the spinlock (given in constructor) on construction
 * and unlocks it on destruction (eg stack unwinding).
 */

#pragma once

#include "synchronization/Spinlock.h"

/**
 * @class SpinlockLocker Spinlock.h "Spinlock.h"
 * @brief Spinlock locking class used for locking a Spinlock in a block.
 *
 * SpinlockLocker locks the spinlock (given in constructor) on construction
 * and unlocks it on destruction (eg stack unwinding). Should be used as a
 * local variable (allocated on stack) to make the destructor do the work
 * automatically on function end. The member functions are defined as inline
 * functions.
 */
class SpinlockLocker {
public:
	/**
	 * Constructor locks the Spinlock given in parameter.
	 *
	 * @param Spinlock The lock passed by reference.
	 */
	inline SpinlockLocker(Spinlock& spinlock): m_spinlock(spinlock) {
		m_spinlock.lock();
	}

	/**
	 * Desctructor which unlocks the lock given in the constructor.
	 */
	inline ~SpinlockLocker() {
		m_spinlock.unlock();
	}

private:
	/** The reference to lock from the constructor. */
	Spinlock& m_spinlock;

	/** Deny copy-constructor. */
	SpinlockLocker(const SpinlockLocker&);
	
	/** Deny assignment. */
	SpinlockLocker& operator=(const SpinlockLocker&);

	/** Deny creation on heap, because it is useless for this class. */
	void* operator new(unsigned int);
};

