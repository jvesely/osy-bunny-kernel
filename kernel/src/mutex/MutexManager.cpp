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
 * locked mutexes. It dequeues/enques the threads in Scheduler.
 */

#include "MutexManager.h"

#include "cpp.h"
#include "Kernel.h"
#include "InterruptDisabler.h"
#include "proc/Scheduler.h"
#include "proc/Thread.h"
#include "structures/List.h"

//#define DEBUG_MUTEX
//#define DEBUG_MUTEX_ACTIONS

typedef List<Thread *> ThreadList;

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
		mtx, Scheduler::instance().activeThread()->id());
#endif
}

/* --------------------------------------------------------------------- */

void MutexManager::mutex_destroy(mutex_t *mtx) {
	ASSERT(mtx != NULL);
	if (!mtx) return;

	InterruptDisabler lock;

#ifdef DEBUG_MUTEX
	// check if being unlocked by the locking thread
	if (mtx->locked && mtx->locked != Scheduler::instance().activeThread()->id()) {
		panic("Lock at %x being destroyed by thread %u while still locked by thread %u!\n",
			mtx, Scheduler::instance().activeThread()->id(), mtx->locked);
	}
#endif

	// if there is a waiting list created, check if empty and remove it or panic if not empty
	if (((ThreadList *)mtx->waitingList)->size() != 0) {
		panic("Lock at %x being destroyed by thread %u while there are still locked threads waiting for it!\n",
			mtx, Scheduler::instance().activeThread()->id());
	}

#ifdef DEBUG_MUTEX_ACTIONS
	printf("[MUTEX] Destroy on mutex %p from thread %u.\n",
		mtx, Scheduler::instance().activeThread()->id());
#endif
}

/* --------------------------------------------------------------------- */

void MutexManager::mutex_lock(mutex_t *mtx) {
	ASSERT(mtx != NULL);
	if (!mtx) return;

#ifdef DEBUG_MUTEX_ACTIONS
	printf("[MUTEX] Lock on mutex %p from thread %u. Waitinglist size before lock: %u.\n",
		mtx, Scheduler::instance().activeThread()->id(), ((ThreadList *)mtx->waitingList)->size());
#endif

	// disable interrupts
	InterruptDisabler lock;

	// get the current thread info
	Thread* thread = Scheduler::instance().activeThread();

	if (mtx->locked != 0) {
		// if mutex locked already
		// unschedule and put the thread to mutex's waiting list
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
		mtx, Scheduler::instance().activeThread()->id(), ((ThreadList *)mtx->waitingList)->size());
#endif
}

/* --------------------------------------------------------------------- */

int MutexManager::mutex_lock_timeout(mutex_t *mtx, const unsigned int usec) {
	ASSERT(mtx != NULL);
	if (!mtx) return ETIMEDOUT;

	return ETIMEDOUT;
}

/* --------------------------------------------------------------------- */

void MutexManager::mutex_unlock(mutex_t *mtx) {
	ASSERT(mtx != NULL);
	if (!mtx || !mtx->locked) return;

#ifdef DEBUG_MUTEX_ACTIONS
	printf("[MUTEX] Unlock on mutex %p from thread %u. Waitinglist size before unlock: %u.\n",
		mtx, Scheduler::instance().activeThread()->id(), ((ThreadList *)mtx->waitingList)->size());
#endif

	InterruptDisabler lock;

#ifdef DEBUG_MUTEX
	// check if being unlocked by the locking thread
	if (mtx->locked != Scheduler::instance().activeThread()->id()) {
		panic("Lock at %x being unlocked by thread %u while still locked by thread %u!\n",
			mtx, Scheduler::instance().activeThread()->id(), mtx->locked);
	}
#endif

	if (((ThreadList *)mtx->waitingList)->size() != 0) {
		// if the waiting list is not empty, unblock the first waiting thread
		Thread* thread = ((ThreadList *)mtx->waitingList)->getFront();
		// lock the mutex with new id
		mtx->locked = thread->id();
		// enqueue the thread in Scheduler
		Scheduler::instance().enqueue(thread);
	} else {
		// if no waiting threads, just unlock
		mtx->locked = 0;
	}

#ifdef DEBUG_MUTEX_ACTIONS
	printf("[MUTEX] Unlock on mutex %p from thread %u. Waitinglist size after unlock: %u.\n",
		mtx, Scheduler::instance().activeThread()->id(), ((ThreadList *)mtx->waitingList)->size());
#endif
}

