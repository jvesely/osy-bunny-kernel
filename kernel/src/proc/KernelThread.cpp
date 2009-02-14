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

#include "api.h"
#include "KernelThread.h"
#include "InterruptDisabler.h"
#include "address.h"
#include "timer/Timer.h"
#include "mem/VirtualMemory.h"

//#define KERNEL_THREAD_DEBUG

#ifndef KERNEL_THREAD_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...) \
	puts("[ KERNEL THREAD ]: "); \
	printf(ARGS);
#endif

/*----------------------------------------------------------------------------*/
void KernelThread::run()
{
	PRINT_DEBUG ("Started thread %u.\n", m_id);

	m_runFunc( m_runData );

	m_status = FINISHED;
	PRINT_DEBUG ("Finished thread %u.\n", m_id);

	if (m_follower) {
		PRINT_DEBUG ("Waking up JOINING thread(%u) by thread %u.\n",
			m_id, m_follower->id());
		ASSERT (m_follower->status() == JOINING);
		m_follower->resume();
	}

	block();
	yield();

	panic("[ THREAD %u ] Don't you wake me. I'm dead.\n", m_id);
}
/*----------------------------------------------------------------------------*/
KernelThread::KernelThread( void* (*thread_start)(void*), void* data,
	unative_t flags, uint stackSize ):
	Thread( stackSize ), m_runFunc( thread_start ), m_runData( data )
{
	if (flags & TF_NEW_VMM) {
		m_virtualMap = new VirtualMemory;
		if (!m_virtualMap) this->m_status = Thread::UNINITIALIZED;
		PRINT_DEBUG ("Creating new Virtual Memory Map %d.\n", (bool)m_virtualMap);
	} else {
		Thread* creator = Thread::getCurrent();
		if (creator) {
			m_virtualMap = creator->getVMM();
			PRINT_DEBUG ("Using creators(%u) Virtual Memory Map.\n", creator->id());
		}
		m_status = INITIALIZED;
	}
}
/*----------------------------------------------------------------------------*/
Thread* KernelThread::create( thread_t* thread_ptr, void* (*thread_start)(void*), void* thread_data, const unsigned int thread_flags )
{
	Thread* new_thread = new KernelThread( thread_start, thread_data, thread_flags );

	if ( (new_thread == NULL) || (new_thread->status() != INITIALIZED) ) {
		delete new_thread;
		return NULL;
	}

	*thread_ptr = new_thread->registerWithScheduler();
	if (!(*thread_ptr)) { //id space allocation failed
		delete new_thread;
		return NULL;
	}

	new_thread->resume();
	return new_thread;
}

KernelThread::~KernelThread()
{
	PRINT_DEBUG ("Thread %u dying.\n", m_id);
}
