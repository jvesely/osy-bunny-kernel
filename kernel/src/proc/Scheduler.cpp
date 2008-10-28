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
	if (m_activeThreadList.size() == 0) {
		//TODO:switch to idle thread
		return;
	}

	void* DUMMYSTACK = (void*)0xF00;
	void** old_stack = (void**)(m_currentThread?m_currentThread->stackTop():&DUMMYSTACK);
	m_currentThread->setStatus(Thread::READY);
	m_currentThread = *m_activeThreadList.rotate();
	m_currentThread->setStatus(Thread::RUNNING);
	void** new_stack = m_currentThread->stackTop();
	//dprintf("Switching stacks %x,%x\n", old_stack, new_stack);
	Processor::switch_cpu_context(old_stack, new_stack);
	//enable interupts
}
/*----------------------------------------------------------------------------*/
void Scheduler::schedule(Thread * thread)
{
	assert( Kernel::instance().pool().reserved() );
	ListItem<Thread*>* item = Kernel::instance().pool().get();
	item->data() = thread;
	m_activeThreadList.pushBack(item);
	thread->setStatus(Thread::READY);
	dprintf("Scheduled thread %u to run.\n", thread->id());
}
/*----------------------------------------------------------------------------*/
void Scheduler::suspend()
{
	ListItem<Thread*>* ptr = m_activeThreadList.removeFind(m_currentThread);
	if (!ptr) return;
	//return to the pool
	Kernel::instance().pool().put(ptr);
	m_currentThread->setStatus(Thread::WAITING);
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
