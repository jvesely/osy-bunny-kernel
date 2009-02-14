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

#include "api.h"
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

/*----------------------------------------------------------------------------*/
Scheduler::Scheduler():ThreadMap( 61 )
{
	m_idle = &KERNEL;
	m_currentThread = &KERNEL;
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
