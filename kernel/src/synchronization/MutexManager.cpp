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
 * @brief MutexManager member function definitions.
 *
 * MutexManager is a singleton class providing interface to init, lock, unlock and
 * destroy mutex locks. It manages the mutex struct and the waiting lists for the
 * locked mutexes..
 */

#include "MutexManager.h"

#include "cpp.h"
#include "InterruptDisabler.h"
#include "proc/Thread.h"
#include "structures/List.h"
#include "Time.h"

//#define DEBUG_MUTEX
//#define DEBUG_MUTEX_ACTIONS

void MutexManager::mutex_init(mutex_t *mtx) {
	ASSERT(mtx != NULL);
	if (!mtx) return;

	// check the size of the placeholder for ThreadList type
	ASSERT(sizeof(ThreadList) <= sizeof(mtx->waitingList));

	// initialize the mutex (to unlocked position)
	mtx->locked = 0;
	// call constructor of the List with placement new
	new ((void *)mtx->waitingList) ThreadList();

#ifdef DEBUG_MUTEX_ACTIONS
	printf("[MUTEX] Init on mutex %p from thread %u.\n",
		mtx, Thread::getCurrent()->id());
#endif
}

/* --------------------------------------------------------------------- */

void MutexManager::mutex_destroy(mutex_t *mtx) {
	ASSERT(mtx != NULL);
	if (!mtx) return;

	InterruptDisabler lock;

#ifdef DEBUG_MUTEX
	// check if being unlocked by the locking thread
	if (mtx->locked && mtx->locked != Thread::getCurrent()->id()) {
		panic("Lock at %x being destroyed by thread %u while still locked by thread %u!\n",
			mtx, Thread::getCurrent()->id(), mtx->locked);
	}
#endif

	// if there is a waiting list created, check if empty and remove it or panic if not empty
	if (((ThreadList *)mtx->waitingList)->size() != 0) {
		panic("Lock at %x being destroyed by thread %u while there are still locked threads waiting for it!\n",
			mtx, Thread::getCurrent()->id());
	}

#ifdef DEBUG_MUTEX_ACTIONS
	printf("[MUTEX] Destroy on mutex %p from thread %u.\n",
		mtx, Thread::getCurrent()->id());
#endif
}

/* --------------------------------------------------------------------- */

void MutexManager::mutex_lock(mutex_t *mtx) {
	ASSERT(mtx != NULL);
	if (!mtx) return;

#ifdef DEBUG_MUTEX_ACTIONS
	printf("[MUTEX] Lock on mutex %p from thread %u. Waitinglist size before lock: %u.\n",
		mtx, Thread::getCurrent()->id(), ((ThreadList *)mtx->waitingList)->size());
#endif

	// disable interrupts
	InterruptDisabler lock;

	// get the current thread info
	Thread* thread = Thread::getCurrent();

	if (mtx->locked != 0) {
		// if mutex is locked already
		// block the thread (this will remove it from the timer's heap and the scheduler)
		thread->block();
		// put the thread to mutex's waiting list (this will unschedule it)
		thread->append((ThreadList *)mtx->waitingList);
		// set the thread state to blocked
		thread->setStatus(Thread::BLOCKED);
		// switch to other thread
		thread->yield();
	} else {
		// if mutex not locked, just store the locking thread's id
		mtx->locked = thread->id();
	}

#ifdef DEBUG_MUTEX_ACTIONS
	printf("[MUTEX] Lock on mutex %p from thread %u. Waitinglist size after lock: %u.\n",
		mtx, Thread::getCurrent()->id(), ((ThreadList *)mtx->waitingList)->size());
#endif
}

/* --------------------------------------------------------------------- */

int MutexManager::mutex_lock_timeout(mutex_t *mtx, const Time time) {
	ASSERT(mtx != NULL);
	if (!mtx) return ETIMEDOUT;

#ifdef DEBUG_MUTEX_ACTIONS
	printf("[MUTEX] Timed lock called on mutex %p from thread %u, locked for %u. Waitinglist size before lock: %u.\n",
		mtx, Thread::getCurrent()->id(), mtx->locked, ((ThreadList *)mtx->waitingList)->size());
#endif

	// disable interrupts
	InterruptDisabler lock;

	// get the current thread info
	Thread* thread = Thread::getCurrent();

	if (!mtx->locked) {
		// if mutex not locked, just store the locking thread's id
		mtx->locked = thread->id();
		return EOK;
	}

	// if the sleep is 0, just return the lock state (like trylock)
	if ((time.getSecs() == 0) && (time.getUsecs() == 0)) {
		return ETIMEDOUT;
	}

	// store, which thread have the lock (for the mutex6 test)
	thread_t locked = mtx->locked;

	// if you got here, you are willing to sleep
	// plan to wake up after given time
	thread->alarm(time);
	// put the thread to mutex's waiting list (this will unschedule it)
	thread->append((ThreadList *)mtx->waitingList);
	// set the thread state to blocked
	thread->setStatus(Thread::BLOCKED);
	// switch to other thread
	thread->yield();

	return ((locked != mtx->locked) && (mtx->locked == thread->id()))
		? EOK
		: ETIMEDOUT;
}

/* --------------------------------------------------------------------- */

void MutexManager::mutex_unlock(mutex_t *mtx) {
	ASSERT(mtx != NULL);
	if (!mtx || !mtx->locked) return;

#ifdef DEBUG_MUTEX_ACTIONS
	printf("[MUTEX] Unlock on mutex %p from thread %u. Waitinglist size before unlock: %u.\n",
		mtx, Thread::getCurrent()->id(), ((ThreadList *)mtx->waitingList)->size());
#endif

	// disable interrupts
	InterruptDisabler lock;

#ifdef DEBUG_MUTEX
	// check if being unlocked by the locking thread
	if (mtx->locked != Thread::getCurrent()->id()) {
		panic("Lock at %x being unlocked by thread %u while still locked by thread %u!\n",
			mtx, Thread::getCurrent()->id(), mtx->locked);
	}
#endif

	if (((ThreadList *)mtx->waitingList)->size() != 0) {
		// if the waiting list is not empty, unblock the first waiting thread
		Thread* thread = ((ThreadList *)mtx->waitingList)->getFront();
		// lock the mutex with new id
		mtx->locked = thread->id();
		// remove from timer's heap (in case of timed lock)
		// and enqueue the thread in Scheduler
		thread->resume();
	} else {
		// if no waiting threads, just unlock
		mtx->locked = 0;
	}

#ifdef DEBUG_MUTEX_ACTIONS
	printf("[MUTEX] Unlock on mutex %p from thread %u. Waitinglist size after unlock: %u.\n",
		mtx, Thread::getCurrent()->id(), ((ThreadList *)mtx->waitingList)->size());
#endif
}

