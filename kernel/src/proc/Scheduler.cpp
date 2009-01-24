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
#include "Thread.h"
#include "InterruptDisabler.h"
#include "Kernel.h"

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
	m_idle = &KERNEL; //new IdleThread();
	m_currentThread = &KERNEL;
	/* reserve thread_t 0 to be out of the reach for other threads */
	m_threadMap.insert(0, NULL);

	/* idle thread must be created successfully */
	ASSERT (m_idle->status() == Thread::INITIALIZED);
}
/*----------------------------------------------------------------------------*/
thread_t Scheduler::getId( Thread* newThread )
{
	/* there has to be free id becasue if all were occupied, than
	 * the system would have thread at every byte of adressable memory,
	 * but the next variable and the parameter take up 8 bytes 
	 * (and even more is used by the rest of the kernel), 
	 * thus there must be a free id
	 */
	thread_t id = m_nextThreadId++;

	/* if it is taken repeat */
	int result;
	while ( !id || (result = m_threadMap.insert(id, newThread)) == EINVAL) {
		PRINT_DEBUG("ID %u already in use getting new one.\n", id);
		id = m_nextThreadId++;
	}

	if (result == ENOMEM)
		return 0;

	/* set the id to the thread */
	newThread->setId(id);

	/* count this thread as active */
	//++m_threadCount;

	return id;
}
/*----------------------------------------------------------------------------*/
Thread* Scheduler::nextThread()
{
	/* disable interrupts, when mangling with scheduling queue */
  InterruptDisabler interrupts;

	if (m_activeThreadList.empty()) {
		/* nothing to run but there arees till threads present */
		PRINT_DEBUG ("Next thread will be the idle thread.\n");
		return m_idle;
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
void Scheduler::enqueue( Thread* thread )
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
	if (m_currentThread == m_idle) {
		PRINT_DEBUG("Ending IDLE thread reign.\n");
		m_shouldSwitch = true;
	}
	
}
/*----------------------------------------------------------------------------*/
void Scheduler::dequeue( Thread* thread )
{
	/* queue mangling needs interupts disabled */
	InterruptDisabler interrupts;

	thread->remove();
	PRINT_DEBUG("Dequeuing thread %u.\n", thread->id());
}
/*----------------------------------------------------------------------------*/
