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
 * @brief Scheduler implementation.
 *
 * Contains some member functions' implementations both public and private.
 */

#include "Scheduler.h"
#include "Kernel.h"
#include "KernelThread.h"
#include "InterruptDisabler.h"
#include "timer/Timer.h"

//#define SCHEDULER_DEBUG

#ifndef SCHEDULER_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...) \
	printf("[ SCHEDULER_DEBUG ]: "); \
	printf(ARGS);
#endif

//const Time Scheduler::DEFAULT_QUANTUM(0, 20000);
/*----------------------------------------------------------------------------*/
Scheduler::Scheduler(): m_threadMap(61), m_currentThread(NULL)
{
	/* create idle thread */
	m_idle = new IdleThread();
	
	/* reserve thread_t 0 to be out of the reach for other threads */
	m_threadMap.insert(0, NULL);

	/* idle thread must be created successfully */
	ASSERT (m_idle->status() == Thread::INITIALIZED);
}
/*----------------------------------------------------------------------------*/
thread_t Scheduler::getId( Thread* newThread )
{
	/* there has to be free id becasue if all were occupied, than
	 * the system would have thread at every byt of adressable memory,
	 * but the next variable and the parameter take up 8 bytes 
	 * (and even more is used by the rest of the kernel), there must be a free id
	 */
	thread_t id = m_nextThread++;

	/* if it is taken repeat */
	while (m_threadMap.insert(id, newThread) == EINVAL) {
		PRINT_DEBUG("ID %u already in use getting new one.\n", id);
		id = m_nextThread++;
	}

	/* set the id to the thread */
	newThread->setId(id);

	/* count this thread as active */
	++m_threadCount;

	return id;
}
/*----------------------------------------------------------------------------*/
Thread* Scheduler::nextThread()
{
	/* disable interrupts, when mangling with scheduling queue */
  InterruptDisabler interrupts;

	if (m_activeThreadList.size() == 0 && m_threadCount > 0) {
		/* nothing to run but there arees till threads present */
		PRINT_DEBUG ("Next thread will be the idle thread.\n");
		return m_idle;
	}

	if (m_threadCount == 0) {
			/* Only timerManager might be running */
			ASSERT (m_activeThreadList.size() <= 1);
			PRINT_DEBUG ("No more threads to run.\n");
			return NULL;
	}
	
	/* if the running thread is not the first thread in the list 
	 * (is not in the list at all), then skip rotating and just plan
	 * the first thread.
	 */
	if (m_currentThread != m_activeThreadList.getFront()) {
		PRINT_DEBUG ("Active thread is not the first in the queue skipping rotation.\n");
		return m_activeThreadList.getFront();
	} else {
	//	PRINT_DEBUG ("Rotating queue.\n");
		return *m_activeThreadList.rotate();
	}
}
/*----------------------------------------------------------------------------*/
void Scheduler::enqueue(Thread * thread)
{
	/* disable interupts as all sheduling queue mangling functions */
	InterruptDisabler interrupts;

	ASSERT (thread);

	/* all threads in the queue can be scheduled to run so their status
	 * should be ready
	 */
	thread->append(&m_activeThreadList);
	PRINT_DEBUG ("Enqueued thread: %d.\n", thread->id());
	thread->setStatus( Thread::READY );
	
	/* if the idle thread is running and other thread became ready,
	 * idle thread is planned for switch as soon as possible
	 */
	if (m_currentThread == m_idle && m_activeThreadList.size() == 1) {
		PRINT_DEBUG("Ending IDLE thread reign.\n");
		thread->switchTo();
	}
	
}
/*----------------------------------------------------------------------------*/
void Scheduler::dequeue(Thread* thread)
{
	/* queue mangling needs interupts disabled */
	InterruptDisabler interrupts;

	thread->remove();
	PRINT_DEBUG("Removing thread %u.\n", thread->id());

	/* Decrease active threa count if it is never to be run again */
	if ( (thread->status() == Thread::KILLED)
		|| (thread->status() == Thread::FINISHED) ) {
		PRINT_DEBUG("Decreasing thread count.\n");
		--m_threadCount;
	}
}
/*----------------------------------------------------------------------------*/
