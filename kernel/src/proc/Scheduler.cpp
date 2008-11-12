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

#define SCHEDULER_DEBUG

#ifndef SCHEDULER_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...) \
	printf("[ SCHEDULER_DEBUG ]: "); \
	printf(ARGS);
#endif

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
void Scheduler::switchThread()
{
	/* disable interrupts, when mangling with scheduling queue */
	InterruptDisabler interrupts;
	
	/* check and delete if current thread was detached and has ended */
	if (   m_currentThread 
		&& m_currentThread->detached() 
		&& ( m_currentThread->status() == Thread::KILLED 
		  || m_currentThread->status() == Thread::FINISHED )) {
		PRINT_DEBUG("Detached thread has ended...deleting: %u.\n", m_currentThread->id());
		delete m_currentThread;
		m_currentThread = NULL;
	}

	/* old_stack points to the old thread stackpointerr, 
	 * if it's NULL saving of the context is skipped
	 */
	void** old_stack = (m_currentThread ? m_currentThread->stackTop() : NULL);

	/* change status if it remains in the queue */
	if (m_currentThread->status() == Thread::RUNNING)
		m_currentThread->setStatus(Thread::READY);

	/* nothing to run */
	if (!m_activeThreadList.size()) {
		PRINT_DEBUG("Nothing to do switching to the idle thread.\n");
		m_currentThread = m_idle;
	}

	/* if the running thread is not the first thread in the list 
	 * (i.e is not in the list at all), then skip rotating and just plan
	 * the first thread
	 */
	if (m_currentThread != m_activeThreadList.getFront()) {
		PRINT_DEBUG("Active thread was lost skipping rotation.\n");
		m_currentThread = m_activeThreadList.getFront();
	} else {
		PRINT_DEBUG("Rotating queue.\n");
		m_currentThread = *m_activeThreadList.rotate();
	}

	/* no more threads can be scheduled => shutdown */
	if (m_threadCount == 0) {
			assert(m_activeThreadList.size() == 0);
			printf("[ KERNEL SHUTDOWN ] No more active threads, shutting down.\n");
			Kernel::halt();
	}
	
	PRINT_DEBUG("New active thread will be: %u.\n", m_currentThread->id());

	/* set running on the chosen thread */
	m_currentThread->setStatus(Thread::RUNNING);
	void** new_stack = m_currentThread->stackTop();

	/* plan it's switch before it's run */
	if (m_currentThread != m_idle) {
		Timer::instance().plan(m_currentThread, Time(0, DEFAULT_QUANTUM) );
		PRINT_DEBUG("Planning preemptive strike for thread %u.\n",
			m_currentThread->id());
	}

	/* the actual context switch */
	if (old_stack != new_stack) {
		PRINT_DEBUG("Switching stacks.\n");
		Processor::switch_cpu_context(old_stack, new_stack);
	}
}
/*----------------------------------------------------------------------------*/
void Scheduler::enqueue(Thread * thread)
{
	/* disable interupts as all sheduling queue mangling functions */
	InterruptDisabler interrupts;

	/* all threads in the queue can be scheduled to run so their status
	 * should be ready
	 */
	thread->append(&m_activeThreadList);
	PRINT_DEBUG("Enqueued thread: %d.\n", thread->id());
	thread->setStatus(Thread::READY);
	
	/* if the idle thread is running and other thread became ready,
	 * idle thread is planned for switch as soon as possible
	 */
	if (m_currentThread == m_idle){
		PRINT_DEBUG("Ending IDLE thread reign.\n");
		Timer::instance().plan(m_idle, Time(0, 1));
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
