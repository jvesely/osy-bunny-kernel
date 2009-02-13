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
 * @brief Mutex member function definitions.
 *
 * Mutex (mutual exclusion) is a common synchronization technique. The Mutex
 * class provides interface to init (constructor), lock, unlock and destroy
 * (destructor) a single mutex lock. It manages the waiting list of the threads
 * locked on the mutex. It dequeues/enques the threads in Scheduler.
 */

#include "Mutex.h"

#include "InterruptDisabler.h"

// To fobid unlocking by other thread than locked
//#define DEBUG_MUTEX

//#define MUTEX_DEBUG_PRINTS

#ifndef MUTEX_DEBUG_PRINTS
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...) \
  printf("[ MUTEX DEBUG ]: "); \
  printf(ARGS);
#endif


Mutex::Mutex()
{
	// initialize the mutex (to unlocked position)
	m_locked = 0;

	PRINT_DEBUG("[MUTEX] Init on mutex %p from thread %u.\n",
		this, Thread::getCurrent()->id());
}

/* --------------------------------------------------------------------- */

Mutex::~Mutex()
{
	InterruptDisabler lock;

#ifdef DEBUG_MUTEX
	// check if being unlocked by the locking thread
	if (m_locked && m_locked != Thread::getCurrent()->id()) {
		panic("Lock at %x being destroyed by thread %u while still locked by thread %u!\n",
			this, Thread::getCurrent()->id(), m_locked);
	}
#endif

	// if there is a waiting list created, check if empty and remove it or panic if not empty
	if (m_waitingList.size() != 0) {
		panic("Lock at %x being destroyed by thread %u while there are still locked threads waiting for it!\n",
			this, Thread::getCurrent()->id());
	}

	PRINT_DEBUG("[MUTEX] Destroy on mutex %p from thread %u.\n",
		this, Thread::getCurrent()->id());
}

/* --------------------------------------------------------------------- */

void Mutex::lock()
{
	PRINT_DEBUG("[MUTEX] Lock on mutex %p from thread %u. Waitinglist size before lock: %u.\n",
		this, Thread::getCurrent()->id(), m_waitingList.size());

	// disable interrupts
	InterruptDisabler lock;

	// get the current thread info
	Thread* thread = Thread::getCurrent();

	if (m_locked != 0) {
		// if mutex is locked already
		// block the thread (this will remove it from the timer's heap and the scheduler)
		thread->block();
		// put the thread to mutex's waiting list (this will unschedule it)
		thread->append(&m_waitingList); 
		// set the thread state to blocked
		thread->setStatus(Thread::BLOCKED);
		// switch to other thread
		thread->yield();
	} else {
		// if mutex not locked, just store the locking thread's id
		m_locked = thread->id();
	}

	PRINT_DEBUG("[MUTEX] Lock on mutex %p from thread %u. Waitinglist size after lock: %u.\n",
		this, Thread::getCurrent()->id(), m_waitingList.size());
}

/* --------------------------------------------------------------------- */

int Mutex::lockTimeout(const Time time)
{
	PRINT_DEBUG("[MUTEX] Timed lock called on mutex %p from thread %u, locked for %u. Waitinglist size before lock: %u.\n",
		this, Thread::getCurrent()->id(), m_locked, m_waitingList.size());

	// disable interrupts
	InterruptDisabler lock;

	// get the current thread info
	Thread* thread = Thread::getCurrent();

	if (!m_locked) {
		// if mutex not locked, just store the locking thread's id
		m_locked = thread->id();
		return EOK;
	}

	// if the sleep is 0, just return the lock state (like trylock)
	if ((time.getSecs() == 0) && (time.getUsecs() == 0)) {
		return ETIMEDOUT;
	}

	// store, which thread have the lock (for the mutex6 test)
	thread_t locked = m_locked;

	// if you got here, you are willing to sleep
	// plan to wake up after given time
	thread->alarm(time);
	// put the thread to mutex's waiting list (this will unschedule it)
	thread->append(&m_waitingList); 
	// set the thread state to blocked
	thread->setStatus(Thread::BLOCKED);
	// switch to other thread
	thread->yield();

	return ((locked != m_locked) && (m_locked == thread->id()))
		? EOK
		: ETIMEDOUT;
}

/* --------------------------------------------------------------------- */

void Mutex::unlock()
{
	PRINT_DEBUG("[MUTEX] Unlock on mutex %p from thread %u. Waitinglist size before unlock: %u.\n",
		this, Thread::getCurrent()->id(), m_waitingList.size());

	// disable interrupts
	InterruptDisabler lock;

#ifdef DEBUG_MUTEX
	// check if being unlocked by the locking thread
	if (m_locked != Thread::getCurrent()->id()) {
		panic("Lock at %x being unlocked by thread %u while still locked by thread %u!\n",
			this, Thread::getCurrent()->id(), m_locked);
	}
#endif

	if (m_waitingList.size() != 0) {
		// if the waiting list is not empty, unblock the first waiting thread
		Thread* thread = m_waitingList.getFront();
		// lock the mutex with new id
		m_locked = thread->id();
		// remove from timer's heap (in case of timed lock)
		// and enqueue the thread in Scheduler
		thread->resume();
	} else {
		// if no waiting threads, just unlock
		m_locked = 0;
	}

	PRINT_DEBUG("[MUTEX] Unlock on mutex %p from thread %u. Waitinglist size after unlock: %u.\n",
		this, Thread::getCurrent()->id(), m_waitingList.size());
}

