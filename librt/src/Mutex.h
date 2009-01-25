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
 * @brief Mutex class declaration.
 */

#pragma once

#include "types.h"

class Time;

/*! @class Mutex Mutex.h "Mutex.h"
 * @brief Userspace Mutex class. 
 *
 * Uses kernel class Event via syscalls to maintain the queue of waiting threads.
 */
class Mutex
{
public:

	/*! @brief Default constructor. */
	inline Mutex() {};

	~Mutex();

	/*! 
	 * @brief Initialize the Mutex structures. 
	 *
	 * @note Must be called before calling other member functions.
	 *
	 * @retval EOK on success.
	 * @retval ENOMEM on lack of memory.
	 */
	int init();

	/*! 
	 * @brief Correctly destroys the mutex. 
	 * 
	 * If there are any locked threads on this mutex, the currently running thread
	 * is stopped.
	 */
	void destroy();

	/*! 
	 * @brief Locks the given mutex. If the mutex is already locked, blocks the
	 * thread until it is unlocked.
	 *
	 * In case the mutex is unlocked, this function does not need to do a syscall.
	 */
	int lock();

	/*! 
	 * @brief Locks the given mutex. If the mutex is already locked, blocks the
	 * thread until it is unlocked, but not more than the given timeout.
	 */
	int lockTimeout( const Time timeout );

	/*!
	 * @brief Unlocks the mutex. Unblocks the first thread waiting for this mutex.
	 *
	 * In case there are no threads waiting for this mutex, this function does not
	 * need to do a syscall.
	 */
	int unlock();
	
	/*!
	 * @brief Same as unlock() and in case this function is called by other thread 
	 * than the one that locked the mutex, the current thread is killed. 
	 */
	int unlockCheck();


private:
	/*! @brief Mutex status. (0 = unlocked, 1 = locked). */
	volatile native_t m_locked;
	
	/*! @brief Id of the thread owning this mutex. */
	volatile thread_t m_owner;

	/*! 
	 * @brief Number of blocked (waiting) threads on this mutex. 
	 *
	 * Used to avoid some unneccessary syscalls.
	 */
	volatile uint m_waiting;

	/*! @brief Id of the kernel space Event class instance. */
	event_t m_event;

};
