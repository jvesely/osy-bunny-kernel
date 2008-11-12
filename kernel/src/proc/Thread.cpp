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
	return Scheduler::instance().activeThread();
}
/*----------------------------------------------------------------------------*/
Thread::Thread( unative_t flags, uint stackSize):
	ListInsertable<Thread>(), HeapInsertable<Thread, Time, 4>(),
	m_stackSize(stackSize),	m_detached(false), m_status(UNINITIALIZED), 
	m_id(0), m_follower(NULL)
{
	/* Alloc stack */	
	m_stack = malloc(m_stackSize);
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
void Thread::yield()
{
	InterruptDisabler inter;
//	PRINT_DEBUG ("Yielding thread %u.\n", m_id);
	removeFromHeap();
	Scheduler::instance().switchThread();
}
/*----------------------------------------------------------------------------*/
void Thread::alarm(const Time& alarm_time)
{
	InterruptDisabler interrupts;
	
	/* Plan self for enqueueing in proper time */
	Timer::instance().plan( this, alarm_time );
	
	/* remove from the sheduling queue */
	Scheduler::instance().dequeue( this );
}
/*----------------------------------------------------------------------------*/
void Thread::sleep(const uint sec)
{
	/* make me blocked. Difference from waiting is that blocked threads cannot be
	 * waken by thread_wakeup
	 */
	m_status = BLOCKED;

	PRINT_DEBUG ("Thread %u went sleeping for %u seconds.\n", m_id, sec);

	alarm( Time(sec, 0) );
	Scheduler::instance().switchThread();
}
/*----------------------------------------------------------------------------*/
void Thread::usleep( const uint usec )
{
	PRINT_DEBUG ("Thread %u went sleeping for %u microseconds.\n", m_id, usec);

	/* If it's to long time to sleep then block */
	if (usec >= RTC::SECOND)
		sleep(usec / RTC::SECOND);

	Time end_time = Time::getCurrent() + Time(0, usec % RTC::SECOND);	

	while (Time::getCurrent() < end_time) {
		yield();
	}
}
/*----------------------------------------------------------------------------*/
void Thread::suspend()
{
	/* my only suspend self */
	ASSERT (m_status == RUNNING);

	/* WAITING indicates that I have to waken by another thread */
	m_status = WAITING;

	PRINT_DEBUG ("Thread %u suspended.\n", m_id);

	/* remove my rescheduling timer */
	removeFromHeap();

	/* take me from the Scheduler queue */
	Scheduler::instance().dequeue(this);

	/* surrender processor */
	Scheduler::instance().switchThread();
}
/*----------------------------------------------------------------------------*/
void Thread::wakeup() const
{
	/* only WAITING threads can be artifiacially woken */
	assert(m_status == WAITING);

	PRINT_DEBUG ("Thread %u awaken.\n", m_id);
	Scheduler::instance().enqueue(const_cast<Thread*>(this));
}
/*----------------------------------------------------------------------------*/
int Thread::joinTimeout( Thread* thread, const uint usec )
{
	InterruptDisabler interrupts;

	PRINT_DEBUG ("Thread %u trying to join thread %u in %u microsecs.\n", m_id, thread->m_id, usec);
	alarm( Time(0,usec) );
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
		|| thread == Scheduler::instance().activeThread()  /* it's me */
		|| thread->detached()                              /* detached thread */
		|| thread->follower()                              /* already waited for */
	) {
		return EINVAL;
	}
	
	PRINT_DEBUG ("Thread %u joining thread %u.\n", m_id, thread->m_id);

	/* Trying to join KILLED thread */
	if (thread->status() == KILLED) {
		PRINT_DEBUG ("Thread %u joined KILLED thread %u.\n", m_id, thread->m_id);
		delete thread;
		return EKILLED;
	}

	/* Joining finished thread */
	if (thread->status() == FINISHED) {
		PRINT_DEBUG ("Thread %u joined FINISHED thread %u.\n", m_id, thread->m_id);
		delete thread;
		return EOK;
	}

	/* set the joining stuff */
	thread->setFollower(this);
	m_status = JOINING;
	
	if (!timed) {
		/* No more action for me until thread is dead. */
		Scheduler::instance().dequeue(this);
		removeFromHeap();
	}

	Scheduler::instance().switchThread();

	/* Woken byb the death of the thread (either timed or untimed) */
	if (!timed || (thread->m_status == FINISHED) || (thread->m_status == KILLED))	
	{
		ASSERT ( (thread->m_status == FINISHED) || (thread->m_status == KILLED) );
		PRINT_DEBUG ("Thread %u joined thread %u.\n", m_id, thread->m_id);
		delete thread;
		Scheduler::instance().enqueue(this);
		if (timed)
			removeFromHeap();
		return EOK;
	}
	
	/* I was obviously awaken by the timer */
	ASSERT (timed);
	PRINT_DEBUG ("Thread %u joining thread %u timedout.\n", m_id, thread->m_id);
	
	/* I'm no longer waiting for his death */
	thread->m_follower = NULL;
	return ETIMEDOUT;
}
/*----------------------------------------------------------------------------*/
void Thread::kill()
{
	InterruptDisabler inter;
	
	/* only running threads other than  myself can be killed */
	if ( m_status != READY  && m_status != BLOCKED ) {
		PRINT_DEBUG ("Thread %u cannot be killed its status is %u\n", m_id, m_status);
		return;
	}


	m_status = KILLED;

	if (m_follower) {
		/* if i had a follower than wake him up */	
		ASSERT (m_follower->m_status == JOINING);
		ASSERT (!m_detached);
		Scheduler::instance().enqueue(m_follower);
	}

	PRINT_DEBUG ("Thread %u killed (detached: %s).\n", m_id, m_detached ? "YES" : "NO");
	
	removeFromHeap();
	Scheduler::instance().dequeue(this);

	/* detached threadds should be removed immediately after they finish execution*/
	if (m_detached) delete this;
}
/*----------------------------------------------------------------------------*/
Thread::~Thread()
{
	//dprintf("Deleting thread %u (det:%d)\n", m_id, m_detached);
	PRINT_DEBUG ("Thread %u erased from the world.\n", m_id);
	Kernel::instance().free(m_stack);
}

