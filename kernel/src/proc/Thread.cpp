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

#include "Thread.h"
#include "Kernel.h"
#include "Scheduler.h"
#include "InterruptDisabler.h"
#include "address.h"
#include "timer/Timer.h"
#include "api.h"

//#define THREAD_DEBUG

#ifndef THREAD_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...) \
  printf("[ THREAD_DEBUG ]: "); \
  printf(ARGS);
#endif

Thread* Thread::getCurrent()
{
	return Scheduler::instance().currentThread();
}
/*----------------------------------------------------------------------------*/
Thread* Thread::getNext()
{
	return Scheduler::instance().nextThread();
}
/*----------------------------------------------------------------------------*/
Thread* Thread::fromId(thread_t id)
{
	return Scheduler::instance().thread( id );
}
/*----------------------------------------------------------------------------*/
Thread::Thread( unative_t flags, uint stackSize):
	ListInsertable<Thread>(), HeapInsertable<Thread, Time, 4>(),
	m_stackSize(stackSize),	m_detached(false), m_status(UNINITIALIZED), 
	m_id(0), m_follower(NULL), m_virtualMap( NULL )
{
	/* Alloc stack */	
	m_stack = malloc( m_stackSize );
	if (m_stack == NULL) return;  /* test stack */

	/* stack is created OK */
	using namespace Processor;

	/* Stack is moving down so we need to set it sizeof(Context), below 
	 * its end.
	 */
	m_stackTop = (void*)((uintptr_t)m_stack + m_stackSize - sizeof(Context));
	Context * context = (Context*)(m_stackTop);
	
	/* Pointer to my member function that just calls virtual run()
	 * http://www.goingware.com/tips/member-pointers.html
	 * taking adress converting pointer and dereferencing trick 
	 * was advised by M. Burda
	 */
	void (Thread::*runPtr)(void) = &Thread::start; 
	
	context->ra = *(unative_t*)(&runPtr);  /* return address (run this) */
	context->a0 = (unative_t)this;         /* the first and the only argument */
	context->gp = ADDR_TO_KSEG0(0);        /* global pointer */
	context->status = STATUS_IM_MASK | STATUS_IE_MASK | STATUS_CU0_MASK;
	
	PRINT_DEBUG ("Successfully created thread.\n");

	m_status = INITIALIZED;
}
/*----------------------------------------------------------------------------*/
void Thread::switchTo()
{
	InterruptDisabler interrupts;
		
	static const Time DEFAULT_QUANTUM(0, 20000);

	Thread* old_thread = getCurrent();
	void** old_stack = NULL;
	void** new_stack = &m_stackTop;

	if ( old_thread ) {
		if ((old_thread->status() == KILLED || old_thread->status() == FINISHED)
			&& old_thread->m_detached)
		{
			delete old_thread;
			old_thread = NULL;
		}

		if (old_thread->status() == RUNNING)
			old_thread->setStatus( READY );

		old_stack = &old_thread->m_stackTop;
	}
	
	setStatus( RUNNING );

	Scheduler::instance().m_currentThread = this;

	PRINT_DEBUG ("Switching VMM to: %p.\n", m_virtualMap.data());
	if (m_virtualMap)
		m_virtualMap->switchTo();

	if (this != Scheduler::instance().m_idle) {
		PRINT_DEBUG ("Planning preemptive strike for thread %u, quantum %u:%u.\n",
			getCurrent()->id(), DEFAULT_QUANTUM.secs(), DEFAULT_QUANTUM.usecs());
		Timer::instance().plan( this, DEFAULT_QUANTUM );
	}

	Processor::switch_cpu_context(old_stack, new_stack);
}
/*----------------------------------------------------------------------------*/
void Thread::yield()
{
	InterruptDisabler inter;
	PRINT_DEBUG ("Yielding thread %u.\n", m_id);

	/* voluntary yield should remove me from the Timer */
	if (m_status == RUNNING) {
		PRINT_DEBUG ("Removed from heap during yield. (%u)\n", m_id);
		removeFromHeap();
	}

	/* switch to the nextx thread */
	Thread* next = getNext();
	if (!next) {
		PRINT_DEBUG ("Last thread (%u) finished, shutting down.\n", m_id);
		Kernel::halt();
	} else {
		next->switchTo();
	}
}
/*----------------------------------------------------------------------------*/
void Thread::alarm(const Time& alarm_time)
{
	InterruptDisabler interrupts;
	
	removeFromHeap();

	/* Plan self for enqueueing in proper time */
	Timer::instance().plan( this, alarm_time );
	
	/* remove from the sheduling queue */
	Scheduler::instance().dequeue( this );
}
/*----------------------------------------------------------------------------*/
void Thread::sleep( const Time& interval)
{
	/* make me blocked. Difference from waiting is that blocked threads cannot be
	 * waken by thread_wakeup
	 */
	m_status = BLOCKED;

	PRINT_DEBUG ("Thread %u went sleeping for %u seconds %u useconds.\n", m_id, interval.secs(), interval.usecs());

	alarm( interval );
	yield();
}
/*----------------------------------------------------------------------------*/
void Thread::suspend()
{
	/* may only suspend self */
	ASSERT (m_status == RUNNING);

	/* WAITING indicates that I have to waken by another thread */
	m_status = WAITING;

	PRINT_DEBUG ("Thread %u suspended.\n", m_id);

	block();

	/* surrender processor */
	yield();
}
/*----------------------------------------------------------------------------*/
void Thread::wakeup()
{
	/* only WAITING threads can be artifiacially woken */
	ASSERT (m_status == WAITING);

	PRINT_DEBUG ("Thread %u awaken.\n", m_id);
	resume();
}
/*----------------------------------------------------------------------------*/
bool Thread::detach()
{
	Status my_status = status();
	if ( m_detached || (my_status == FINISHED) || (my_status == KILLED) 
		|| m_follower ) return false; 
	PRINT_DEBUG ("Thread %u detached.\n", m_id);
	return m_detached = true;
}
/*----------------------------------------------------------------------------*/
int Thread::joinTimeout( Thread* thread, const uint usec )
{
	InterruptDisabler interrupts;

	PRINT_DEBUG ("Thread %u trying to join thread %u in %u microsecs.\n", m_id, thread->m_id, usec);
	alarm( Time(0, usec) );
	return join( thread, true );
}
/*----------------------------------------------------------------------------*/
int Thread::join( Thread * thread, bool timed )
{
	/* Need to disable interupts as the status of the thread
	 * may not survive during reeschedule
	 */
	InterruptDisabler interrupts;
	
	/* Initial check */
	if (!thread                                          /* no such thread */
		|| thread == this                                  /* it's me */
		|| thread->detached()                              /* detached thread */
		|| thread->m_follower                              /* already waited for */
	) {
		if (timed) resume();
		return EINVAL;
	}
	
	PRINT_DEBUG ("Thread %u joining thread %u.\n", m_id, thread->m_id);

	Status others_status = thread->status();

	/* Trying to join KILLED or FINISHED thread */
	if (others_status == KILLED || others_status == FINISHED) {
		PRINT_DEBUG ("Thread %u joined KILLED thread %u.\n", m_id, thread->m_id);
		delete thread;
		if (timed) resume();
		return (others_status == KILLED) ? EKILLED : EOK;
	}

	/* set the joining stuff */
	thread->m_follower = this;
	m_status = JOINING;
	
	if (!timed) {
		/* No more action for me until thread is dead. */
		block();
	}

  yield();

	others_status = thread->status();
	/* Woken by the death of the thread (either timed or untimed) */
	if ( (others_status == FINISHED) || (others_status == KILLED))	
	{
		PRINT_DEBUG ("Thread %u joined thread %u %s.\n", m_id, thread->m_id, timed ? "TIMED" : "");
		delete thread;
		resume();
		return (others_status == KILLED) ? EKILLED : EOK;
	}

	/* I was obviously awaken by the timer */
	ASSERT (timed);
	PRINT_DEBUG ("Thread %u joining thread %u timedout.\n", m_id, thread->m_id);
	
	/* I'm no longer waiting for his death */
	thread->m_follower = NULL;
	return ETIMEDOUT;
}
/*----------------------------------------------------------------------------*/
void Thread::block()
{
	/* accessing both timer and scheduling queue */
	InterruptDisabler inter;

	PRINT_DEBUG ("Thread %u removing from both scheduler and timer.\n", m_id);
	removeFromHeap();
	Scheduler::instance().dequeue( this );
}
/*----------------------------------------------------------------------------*/
void Thread::resume()
{
	/* accessing both timer and scheduling queue */
	InterruptDisabler inter;

	PRINT_DEBUG ("Thread %u resuming to scheduling queue.\n", m_id);
	removeFromHeap();
	Scheduler::instance().enqueue( this );
}
/*----------------------------------------------------------------------------*/
void Thread::kill()
{
	InterruptDisabler inter;
	
	/* only active threads can be killed */
	if ( status() != READY  && status() != BLOCKED && status() != RUNNING) {
		PRINT_DEBUG ("Thread %u cannot be killed its status is %u\n", id(), status());
		return;
	}

	m_status = KILLED;

	if (m_follower) {
		/* if i had a follower than wake him up */	
		ASSERT (m_follower->status() == JOINING);
		ASSERT (!m_detached);
		m_follower->resume();
	}

	PRINT_DEBUG ("Thread %u killed (detached: %s).\n", m_id, m_detached ? "YES" : "NO");

	/* remove from both timer and scheduler */
	block();

	
	if (Thread::getCurrent() == this)
		yield();

/* detached threads should be removed immediately after they finish execution*/
	if (m_detached) delete this;

}
/*----------------------------------------------------------------------------*/
Thread::~Thread()
{
	PRINT_DEBUG ("Thread %u erased from the world.\n", m_id);
	free(m_stack);
	if ( m_id ) {
		Scheduler::instance().returnId( m_id );
	}
}

