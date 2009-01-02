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
 * @brief Semaphore implementation.
 *
 * Semaphore is a common synchronization technique using a counter, which
 * can be incremented or decremented. It never drops under zero and if
 * who will want to decrement under zero will be blocked till the counter
 * is not more than zero.
 */

#include "Semaphore.h"

void Semaphore::up(const unative_t number) {
	// disable interrupts
	InterruptDisabler lock;

	m_counter += number;

	Thread* thread;
	// while the waiting list is not empty
	while (waitingList.size() != 0) {
		// unlock the waiting threads
		thread = waitingList.getFront();
		//TODO: don't remove from timer's heap (in case of timed lock)
		thread->resume();
	}
}

/* --------------------------------------------------------------------- */

void Semaphore::down(const unative_t number) {
	// disable interrupts
	InterruptDisabler lock;

	// get the current thread info
	Thread* thread = Thread::getCurrent();

	while (m_counter < number) {
		// while the semaphore is not high enough block the thread
		thread->block();
		// put the thread to the waiting list (this will unschedule it)
		thread->append(&waitingList);
		// set the thread state to blocked
		thread->setStatus(Thread::BLOCKED);
		// switch to other thread
		thread->yield();
	}

	// here we can be sure m_counter >= number
	m_counter -= number;
}

/* --------------------------------------------------------------------- */

int Semaphore::downTimeout(const unative_t number, const Time time) {
	// disable interrupts
	InterruptDisabler lock;

	// if time equals to 0 and we can't lover the semaphore, don't block
	if ((time.getSecs() == 0) && (time.getUsecs() == 0) && (m_counter < number)) {
		return ETIMEDOUT;
	}

	Time now = Time::getCurrentTime();
	Time waitUntill = now + time;

	// get the current thread info
	Thread* thread = Thread::getCurrent();

	while ((m_counter < number) && (now < waitUntill)) {
		// plan to wake up
		thread->alarm(waitUntill - now);
		// put the thread to the waiting list
		thread->append(&waitingList); 
		// set the thread state to blocked
		thread->setStatus(Thread::BLOCKED);
		// switch to other thread
		thread->yield();
		// after waking up remember the current time for the cycle condition
		now = Time::getCurrentTime();
	}

	return (m_counter >= number)
		? m_counter -= number, EOK
		: ETIMEDOUT;
}

