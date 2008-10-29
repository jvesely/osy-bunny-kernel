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

Scheduler::Scheduler(): m_currentThread(NULL)
{
	m_idle = new Thread(idleThread, NULL, 128); // small stack should be enough
	unsigned int success = m_idle->setup();
	assert(success == EOK); // must have odle thread
	
}
/*----------------------------------------------------------------------------*/
thread_t Scheduler::addThread(Thread* newThread)
{
	//ostrich stuff, there shall always be free id
	thread_t id = m_nextThread++;
	while (m_threadMap.insert(id, newThread) == -1) {
		//find free id
		id = m_nextThread++;
	}
	newThread->setId(id);

	schedule(newThread);

	return id;
}
/*----------------------------------------------------------------------------*/
int Scheduler::wakeup(thread_t thread)
{
	if (!m_threadMap.exists(thread))
		return EINVAL;
	Thread * thr = m_threadMap.at(thread);
	if (m_activeThreadList.find(thr) != m_activeThreadList.end())	
		schedule(thr);
	return EOK;
}
/*----------------------------------------------------------------------------*/
void Scheduler::switchThread()
{
	//disable interupts
	const ipl_t state = Processor::save_and_disable_interupts();

	void* DUMMYSTACK = (void*)0xF00;
	void** old_stack = (m_currentThread?m_currentThread->stackTop():&DUMMYSTACK);
	m_currentThread->setStatus(Thread::READY);
	m_currentThread = m_activeThreadList.getFront();
	//dprintf("Next thread %x.\n", m_currentThread);
	if (!m_currentThread) {
		m_currentThread = m_idle;
		dprintf("Nothing to do switching to the idle thread.\n");
	} else {
		m_activeThreadList.rotate();
	}

	m_currentThread->setStatus(Thread::RUNNING);
	void** new_stack = m_currentThread->stackTop();
	dprintf("Switching stacks %x,%x\n", old_stack, new_stack);
	if (m_currentThread != m_idle)
		Kernel::instance().setTimeInterupt(DEFAULT_QUATNUM);	
	else
		Kernel::instance().setTimeInterupt(0);
	Processor::switch_cpu_context(old_stack, new_stack);
	//enable interupts
	Processor::revert_interupt_state(state);
}
/*----------------------------------------------------------------------------*/
void Scheduler::schedule(Thread * thread)
{
	ipl_t status = Processor::save_and_disable_interupts();
	assert( Kernel::instance().pool().reserved() );
	ListItem<Thread*>* item = Kernel::instance().pool().get();
	item->data() = thread;
	m_activeThreadList.pushBack(item);
	thread->setStatus(Thread::READY);
	dprintf("Scheduled thread %u to run.\n", thread->id());
	Processor::revert_interupt_state(status);
}
/*----------------------------------------------------------------------------*/
void Scheduler::suspend()
{
	ipl_t status = Processor::save_and_disable_interupts();
	dprintf("Suspending thread %d.\n", m_currentThread->id());
	ListItem<Thread*>* ptr = m_activeThreadList.removeFind(m_currentThread);
	dprintf("Remaining threads in queue %d.\n", m_activeThreadList.size());
	if (!ptr) return;
	//return to the pool
	Kernel::instance().pool().put(ptr);
	m_currentThread->setStatus(Thread::WAITING);
	dprintf("Returning listitem %x.\n", ptr);
	Processor::revert_interupt_state(status);
}
/*----------------------------------------------------------------------------*/
int Scheduler::joinThread(thread_t thread)
{
	if (!m_threadMap.exists(thread))
		return EINVAL;
	Thread* thr = m_threadMap.at(thread);
	if (thr->status() == Thread::KILLED)
		return EKILLED;
	if (thr == m_currentThread || thr->detached() || thr->follower() ) {
		return EINVAL;
	}
	thr->setFollower(m_currentThread);
	suspend();
	switchThread();
	return EOK;
	
}
/*----------------------------------------------------------------------------*/
void * idleThread(void*) { asm volatile ( "wait" ); return NULL; }
