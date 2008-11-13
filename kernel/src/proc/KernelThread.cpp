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
 * @brief Thread class implementation.
 *
 * Constians Thread member functions' implementations.
 */

#include "KernelThread.h"
#include "Kernel.h"
#include "InterruptDisabler.h"
#include "address.h"
#include "api.h"
#include "timer/Timer.h"
/*----------------------------------------------------------------------------*/
void KernelThread::run()
{
	m_runFunc(m_runData);

	m_status = FINISHED;
	
	if (m_follower) {
		assert(m_follower->status() == JOINING);
		m_follower->resume();
	}

	block();
	Scheduler::instance().switchThread();
	
	printf("[ THREAD %u ] Don't you wake me. I'm dead.\n", m_id);
	assert(false);
}
/*----------------------------------------------------------------------------*/
KernelThread::KernelThread(void* (*thread_start)(void*), void* data, 
	unative_t flags = 0, uint stackSize = DEFAULT_STACK_SIZE):
	Thread(flags, stackSize), m_runFunc(thread_start), m_runData(data)
{
//	dprintf("Creating Kernel Thread %p\n", this);
}
/*----------------------------------------------------------------------------*/
KernelThread::~KernelThread()
{
	//printf("[ THREAD %u ] R.I.P. (detached:%S)\n", m_id, m_detached ? "YES":"NO" );
	Scheduler::instance().returnId(m_id);
}
/*----------------------------------------------------------------------------*/
int KernelThread::create(thread_t* thread_ptr, void* (*thread_start)(void*),
  void* thread_data, const unsigned int thread_flags)
{
	Thread* new_thread = new KernelThread(thread_start, thread_data, thread_flags);

	if ( (new_thread == NULL) || (new_thread->status() != INITIALIZED) ) {
		delete new_thread;
		return ENOMEM;
	}
//	dprintf("Getting ID.\n");
	*thread_ptr = Scheduler::instance().getId(new_thread);
//	dprintf("Thread %d(%p) created, now enqueue.\n", new_thread->id(), new_thread);
	Scheduler::instance().enqueue(new_thread);
//	dprintf("Enqueued and leaving.\n");
	return EOK;
}
