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
#include "InterruptDisabler.h"

Scheduler::Scheduler(): m_threadMap(61), m_currentThread(NULL)
{
	m_idle = new Thread(idleThread, (void*)NULL, 0, 512); // small stack should be enough
	//bool success = m_idle->isOK();
	m_idle->setId(0);
	m_threadMap.insert(0, NULL);
	assert(m_idle->m_status == Thread::INITIALIZED); // must have odle thread
}
/*----------------------------------------------------------------------------*/
thread_t Scheduler::getId(Thread* newThread)
{
	//ostrich stuff, there shall always be free id
	thread_t id = m_nextThread++;
	while (m_threadMap.insert(id, newThread) == -1) {
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

	void* DUMMYSTACK = (void*)0xF00;
	void** old_stack = (m_currentThread?m_currentThread->stackTop():&DUMMYSTACK);
//	m_currentThread->setStatus(Thread::READY);
//	m_currentThread = m_activeThreadList.getFront();
	//dprintf("Next thread %x.\n", m_currentThread);
	if (m_currentThread->status() == Thread::RUNNING)
		m_currentThread->setStatus(Thread::READY);

	if (!m_activeThreadList.size()) {
		m_currentThread = m_idle;
		dprintf("Nothing to do switching to the idle thread.\n");
	} else {
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
	if (m_currentThread != m_idle)
		Kernel::instance().setTimeInterrupt(DEFAULT_QUATNUM);	
	else
		Kernel::instance().setTimeInterrupt(0);
	if (old_stack != new_stack)
		Processor::switch_cpu_context(old_stack, new_stack);
	
}
/*----------------------------------------------------------------------------*/
void Scheduler::enqueue(Thread * thread)
{
	InterruptDisabler interrupts;
//	assert( Kernel::instance().pool().reserved() );
	
	//ListItem<Thread*>* item = Kernel::instance().pool().get();
	//item->data() = thread;
//	m_activeThreadList.pushBack(item);
	thread->append(&m_activeThreadList);

//	if (thread->status() == Thread::INITIALIZED) 
//		++m_threadCount; // new thread
	
	thread->setStatus(Thread::READY);
	
	dprintf("Scheduled thread %u to run.\n", thread->id());
	
	if (m_currentThread == m_idle)
			Kernel::instance().setTimeInterrupt(1); // plan to nearest slot
	
}
/*----------------------------------------------------------------------------*/
void Scheduler::dequeue(Thread* thread)
{
	InterruptDisabler interrupts;
	thread->remove();

//	ListItem<Thread*>* ptr = m_activeThreadList.removeFind(thread);
//	if (!ptr) {
//		return;  // not in the list
//	}

	if ( (thread->status() == Thread::KILLED)
		|| (thread->status() == Thread::FINISHED) ) {
		--m_threadCount; // remove dead
	}
//	Kernel::instance().pool().put(ptr);
	//dprintf("Returning listitem %x.\n", ptr);
	dprintf("Thread %d dequeued.\n", thread->m_id);
}
/*----------------------------------------------------------------------------*/
void * idleThread(void*) { asm volatile ( "wait" ); return NULL; }
