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
 *   @version $Id: Mutex.cpp 605 2009-01-22 23:58:59Z slovak $
 *   @note
 *   Semestral work for Operating Systems course at MFF UK \n
 *   http://dsrg.mff.cuni.cz/~ceres/sch/osy/main.php
 *   
 *   @date 2008-2009
 */

/*!
 * @file 
 * @brief Userspace Mutex class implementation.
 */

#include "Mutex.h"
#include "atomic.h"
#include "assert.h"
#include "librt.h"
#include "SysCall.h"

#define USER_MUTEX_DEBUG

#ifndef USER_MUTEX_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...) \
	printf("[ USER_MUTEX_DEBUG ]: "); \
	printf(ARGS);
#endif

/*----------------------------------------------------------------------------*/

Mutex::~Mutex()
{
	ASSERT(m_waiting);
}

/*----------------------------------------------------------------------------*/

int Mutex::init()
{
	m_locked = 0;
	m_owner = 0;
	m_waiting = 0;

	return SysCall::event_init(&m_event);;
}

/*----------------------------------------------------------------------------*/

void Mutex::destroy()
{
	if (m_waiting)
		thread_exit(NULL);

	SysCall::event_destroy(m_event);
}

/*----------------------------------------------------------------------------*/

int Mutex::lock()
{
	PRINT_DEBUG("Mutex::lock(%u) started...\n", m_event);
	while (swap(m_locked, 1) != 0) {
		++m_waiting;
		PRINT_DEBUG("Calling syscall event_wait with event id %d and locked pointer: %p\n", 
			m_event, &m_locked);
		SysCall::event_wait(m_event, &m_locked);
		--m_waiting;
	}

	// save the id of the thread which owns the mutex
	m_owner = thread_self();

	PRINT_DEBUG("Mutex::lock(%u): Mutex successfully locked..\n", m_event);
	return EOK;
}

/*----------------------------------------------------------------------------*/

int Mutex::lockTimeout( const Time timeout )
{
	PRINT_DEBUG("Mutex::lockTimeout(%u) started...\n", m_event);
	while (swap(m_locked, 1) != 0) {
		++m_waiting;
		PRINT_DEBUG("Calling syscall event_wait_timeout with event id %d, locked pointer: %p and timeout pointer %p\n", 
			m_event, &m_locked, &timeout);
		PRINT_DEBUG("Timeout: %u\n", timeout.toUsecs());
		if (SysCall::event_wait_timeout(m_event, &timeout, &m_locked) == ETIMEDOUT) {
			PRINT_DEBUG("Returned ETIMEDOUT from syscall, current timeout: %u\n", timeout.toUsecs());
			--m_waiting;
			return ETIMEDOUT;
		}
		--m_waiting;
	}

	// save the id of the thread which owns the mutex
	m_owner = thread_self();

	PRINT_DEBUG("Mutex::lockTimeout(%u): Mutex successfully locked..\n", m_event);
	return EOK;
}

/*----------------------------------------------------------------------------*/

int Mutex::unlock()
{
	if (swap(m_locked, 0) == 0)
		return EINVAL;
	PRINT_DEBUG("Mutex::unlock(%u): Mutex successfully unlocked..\n", m_event);
	// unblock waiting threads (if any)
	if (m_waiting)
		SysCall::event_fire(m_event);

	return EOK;
}

/*----------------------------------------------------------------------------*/

int Mutex::unlockCheck()
{
	thread_t current = thread_self();
	if (current != m_owner)
		thread_exit(NULL);

	if (swap(m_locked, 0) == 0)
		return EINVAL;

	// unblock waiting threads (if any)
	if (m_waiting)
		SysCall::event_fire(m_event);

	return EOK;
}
