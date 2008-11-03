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
 * Contains some method implementations both public and private.
 */
#include "Scheduler.h"
#include "Kernel.h"
#include "drivers/Processor.h"

Scheduler::Scheduler(): m_threadMap(61), m_currentThread(NULL)
{
	m_idle = new Thread(idleThread, (void*)NULL, 0, 512); // small stack should be enough
	//bool success = m_idle->isOK();
	assert(m_idle->m_stack); // must have odle thread
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

	return id;
}
/*----------------------------------------------------------------------------*/
void Scheduler::switchThread()
{
	//disable interupts
	const ipl_t state = Processor::save_and_disable_interupts();

	void* DUMMYSTACK = (void*)0xF00;
	void** old_stack = (m_currentThread?m_currentThread->stackTop():&DUMMYSTACK);
//	m_currentThread->setStatus(Thread::READY);
//	m_currentThread = m_activeThreadList.getFront();
	//dprintf("Next thread %x.\n", m_currentThread);
	if (!m_activeThreadList.size()) {
		m_currentThread = m_idle;
	//		dprintf("Nothing to do switching to the idle thread.\n");
	} else {
		m_currentThread = *m_activeThreadList.rotate();
	}
	
	if (m_threadCount == 0) {
			assert(m_activeThreadList.size() == 0);
			dprintf("No more active threads, shutting down.\n");
			Kernel::halt();
	}

	m_currentThread->setStatus(Thread::RUNNING);
	void** new_stack = m_currentThread->stackTop();
	dprintf("Switching stacks %x,%x\n", old_stack, new_stack);
	if (m_currentThread != m_idle)
		Kernel::instance().setTimeInterupt(DEFAULT_QUATNUM);	
	else
		Kernel::instance().setTimeInterupt(0);
//	if (*old)
	Processor::switch_cpu_context(old_stack, new_stack);
	//enable interupts
	Processor::revert_interupt_state(state);
}
/*----------------------------------------------------------------------------*/
void Scheduler::enqueue(Thread * thread)
{
	ipl_t status = Processor::save_and_disable_interupts();
	assert( Kernel::instance().pool().reserved() );
	
	ListItem<Thread*>* item = Kernel::instance().pool().get();
	item->data() = thread;
	m_activeThreadList.pushBack(item);

	if (thread->status() == Thread::INITIALIZED) 
		++m_threadCount; // new thread

	
	thread->setStatus(Thread::READY);
	
	dprintf("Scheduled thread %u to run.\n", thread->id());
	
	if (m_currentThread == m_idle)
			Kernel::instance().setTimeInterupt(1); // plan to nearest slot
	
	Processor::revert_interupt_state(status);
}
/*----------------------------------------------------------------------------*/
void Scheduler::dequeue(Thread* thread)
{
	ipl_t status = Processor::save_and_disable_interupts();
	ListItem<Thread*>* ptr = m_activeThreadList.removeFind(thread);
	if (thread->status() == Thread::KILLED || thread->status() == Thread::FINISHED) 
		--m_threadCount; // remove dead
	if (!ptr) {
		Processor::revert_interupt_state(status);
		return;  // not in the list
	}
	Kernel::instance().pool().put(ptr);
	//dprintf("Returning listitem %x.\n", ptr);
	dprintf("Thread %d dequeued.\n", thread->m_id);
	Processor::revert_interupt_state(status);
}
/*----------------------------------------------------------------------------*/
void * idleThread(void*) { asm volatile ( "wait" ); return NULL; }
