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

Scheduler::Scheduler(): m_threadMap(61), m_currentThread(NULL)
{
	m_idle = new IdleThread(); // small stack should be enough
	//bool success = m_idle->isOK(); */
	m_idle->setId(0);
	m_threadMap.insert(0, NULL);
	assert(m_idle->status() == Thread::INITIALIZED && m_idle->id() == 0); // must have idle thread
	dprintf("Idle thread at address %p.\n", m_idle);
}
/*----------------------------------------------------------------------------*/
thread_t Scheduler::getId(Thread* newThread)
{
	//ostrich stuff, there shall always be free id
	thread_t id = m_nextThread++;
	while (m_threadMap.insert(id, newThread) == EINVAL) {
		//find free id
		id = m_nextThread++;
	}
	newThread->setId(id);
	++m_threadCount;

	return id;
}
/*----------------------------------------------------------------------------*/
void Scheduler::switchThread()
{
	//disable interrupts
	InterruptDisabler interrupts;

//	void* DUMMYSTACK = (void*)0xF00;
	if (   m_currentThread 
			&& m_currentThread->detached() 
			&& (  m_currentThread->status() == Thread::KILLED 
				 || m_currentThread->status() == Thread::FINISHED   )) {
		delete m_currentThread;
		m_currentThread = NULL;
	}

	void** old_stack = (m_currentThread?m_currentThread->stackTop():NULL);
//	m_currentThread->setStatus(Thread::READY);
//	m_currentThread = m_activeThreadList.getFront();
//	dprintf("Next thread %x.\n", m_currentThread);
	if (m_currentThread->status() == Thread::RUNNING)
		m_currentThread->setStatus(Thread::READY);

	if (!m_activeThreadList.size()) {
		m_currentThread = m_idle;
		dprintf("Nothing to do (waiting for %d threads) switching to the idle thread.\n", m_threadCount);
	} else {
		if (m_currentThread != m_activeThreadList.getFront()) {
			m_currentThread = m_activeThreadList.getFront();
		} else
			m_currentThread = *m_activeThreadList.rotate();
//		dprintf("Running thread %d of %d(%d).\n",m_currentThread->id(), m_threadCount, m_activeThreadList.size());
	}
	
	if (m_threadCount == 0) {
			assert(m_activeThreadList.size() == 0);
			dprintf("No more active threads, shutting down.\n");
			Kernel::halt();
	}

	m_currentThread->setStatus(Thread::RUNNING);
	void** new_stack = m_currentThread->stackTop();
//	dprintf("Switching stacks %x,%x\n", old_stack, new_stack);
	/*if (m_currentThread != m_idle)
		Kernel::instance().setTimeInterrupt(DEFAULT_QUATNUM);	
	else
		Kernel::instance().setTimeInterrupt(0);
	*/
	if (old_stack != new_stack)
		Processor::switch_cpu_context(old_stack, new_stack);

	if (m_currentThread != m_idle) {
		Timer::instance().plan(m_currentThread, Time(0, DEFAULT_QUATNUM) );
	}
	
}
/*----------------------------------------------------------------------------*/
void Scheduler::enqueue(Thread * thread)
{
	InterruptDisabler interrupts;
	
	dprintf("Scheduling thread %u(%u).\n", thread->id(), m_activeThreadList.size());

	thread->append(&m_activeThreadList);

	thread->setStatus(Thread::READY);
	
//	dprintf("Scheduled thread %u to run.\n", thread->id());
	
	if (m_currentThread == m_idle){
//		dprintf("IDLE PLAN\n");
		Timer::instance().plan(m_idle, Time(0, DEFAULT_QUATNUM));
	}
//			Kernel::instance().setTimeInterrupt(1); // plan to nearest slot
	
}
/*----------------------------------------------------------------------------*/
void Scheduler::dequeue(Thread* thread)
{
	InterruptDisabler interrupts;
	dprintf("Dequeueing thread %d of %d\n", thread->id(), m_threadCount);
	thread->remove();


	if ( (thread->status() == Thread::KILLED)
		|| (thread->status() == Thread::FINISHED) ) {
		dprintf("Decreasing thread count...(%d)\n", thread->status());
		thread->removeFromHeap();
		--m_threadCount; // remove dead
	}
	//dprintf("Thread %d dequeued (%d active remain).\n", thread->id(), m_threadCount);
}
/*----------------------------------------------------------------------------*/
