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
 * Contians Thread member functions' implementation.
 */

#include "api.h"
#include "Thread.h"
#include "Scheduler.h"
#include "InterruptDisabler.h"
#include "address.h"
#include "timer/Timer.h"
#include "proc/ThreadCollector.h"

//#define THREAD_DEBUG

#ifndef THREAD_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...) \
  printf("[ THREAD_DEBUG ]: "); \
  printf(ARGS)
#endif


extern void* volatile* other_stack_ptr;

Thread* Thread::getCurrent()
{
	return SCHEDULER.currentThread();
}
/*----------------------------------------------------------------------------*/
Thread* Thread::getNext()
{
	return SCHEDULER.nextThread();
}
/*----------------------------------------------------------------------------*/
Thread* Thread::fromId(thread_t id)
{
	return SCHEDULER.thread( id );
}
/*----------------------------------------------------------------------------*/
bool Thread::shouldSwitch()
{
	return SCHEDULER.m_shouldSwitch;
}
/*----------------------------------------------------------------------------*/
void Thread::requestSwitch()
{
	SCHEDULER.m_shouldSwitch = true;
}
/*----------------------------------------------------------------------------*/
Thread::Thread( uint stackSize ):
	ListInsertable<Thread>(),
	HeapInsertable<Thread, Time, THREAD_HEAP_CHILDREN>(), m_otherStackTop( NULL ),
	m_stackSize( stackSize ),	m_detached( false ), m_status( UNINITIALIZED ),
	m_id( 0 ), m_follower( NULL ), m_joinTarget( NULL ), m_virtualMap( NULL )
{
	if (!m_stackSize) return;
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

	context->ra = *(unative_t*)(&runPtr);  /* return address (run this)       */

	context->a0 = (unative_t)this;         /* the first and the only argument */
	context->gp = ADDR_TO_KSEG0(0);        /* global pointer                  */
	context->status = STATUS_IM_MASK | STATUS_IE_MASK;

	PRINT_DEBUG ("Successfully created thread.\n");

	m_status = INITIALIZED;
}
/*----------------------------------------------------------------------------*/
void Thread::switchTo()
{
	InterruptDisabler interrupts;

	ASSERT (this); 

	PRINT_DEBUG ("Switching to thread %u.\n", m_id);

	static const Time DEFAULT_QUANTUM(0, 20000);

	Thread* old_thread = getCurrent();

	ASSERT ( old_thread );
	
	void** old_stack = &(old_thread->m_stackTop);
	void** new_stack = &m_stackTop;

	if (old_thread->status() == RUNNING)
		old_thread->setStatus( READY );

	setStatus( RUNNING );

	SCHEDULER.m_currentThread = this;
	SCHEDULER.m_shouldSwitch  = false;
	other_stack_ptr = &m_otherStackTop;

	PRINT_DEBUG ("Switching VMM to: %p.\n", m_virtualMap.data());

	if (m_virtualMap) {
		m_virtualMap->switchTo();
	} else {
		IVirtualMemoryMap::switchOff();
	}

	/* plan my end if I'm not the idle thread */
	if (this != SCHEDULER.m_idle) {
		PRINT_DEBUG ("Planning preemptive strike for thread %u, quantum %u:%u.\n",
			id(), DEFAULT_QUANTUM.secs(), DEFAULT_QUANTUM.usecs());
		TIMER.plan( this, DEFAULT_QUANTUM );
	}

	PRINT_DEBUG ("Switching stacks: %p, %p.\n", old_stack, new_stack);

	Processor::switch_cpu_context( old_stack, new_stack );

	PRINT_DEBUG ("Thread %u, cleaning inactive.\n", id());
	THREAD_BIN.clean();
}
/*----------------------------------------------------------------------------*/
void Thread::yield()
{
	InterruptDisabler inter;
	PRINT_DEBUG ("Yielding thread %u.\n", id());

	/* voluntary yield should remove me from the Timer */
	if (status() == RUNNING) {
		PRINT_DEBUG ("Removed from heap during yield. (%u)\n", id());
		removeFromHeap();
	}

	/* switch to the next thread */
	Thread::getNext()->switchTo();
}
/*----------------------------------------------------------------------------*/
void Thread::alarm( const Time& alarm_time )
{
	InterruptDisabler interrupts;

	removeFromHeap();

	/* Plan self for enqueueing in proper time */
	Timer::instance().plan( this, alarm_time );

	/* remove from the sheduling queue */
	SCHEDULER.dequeue( this );

	m_status = BLOCKED;
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
	ASSERT (status() == RUNNING);
	PRINT_DEBUG ("Thread %u suspending.\n", m_id);

	block();
	/* WAITING indicates that I have to waken by another thread */
	m_status = WAITING;

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
int Thread::join( Thread* thread, void** retval, bool timed, const Time& wait_time )
{
	/* Need to disable interupts as the status of the thread
	 * may not survive during reschedule
	 */
	InterruptDisabler interrupts;

	/* Initial check */
	if (!thread                                          /* no such thread */
		|| thread == this                                  /* it's me */
		|| thread->detached()                              /* detached thread */
		|| thread->m_follower                              /* already waited for */
	) {
		return EINVAL;
	}

	PRINT_DEBUG ("Thread %u joining thread %u%s.\n",
		m_id, thread->m_id, timed ? " (TIMED)" : "");

	Status others_status = thread->status();

	/* Trying to join KILLED or FINISHED thread */
	if (others_status == KILLED || others_status == FINISHED) {
		PRINT_DEBUG ("Thread %u(%p) joined %s thread %u.\n",
			m_id, this, (others_status == KILLED)? "KILLED" : "FINISHED", thread->m_id);
		thread->deactivate();
		return (others_status == KILLED) ? EKILLED : EOK;
	}

	if ( timed ) {
		alarm( wait_time );
	} else {
		/* No more action for me until thread is dead. */
		block();
	}

	/* set the joining stuff */
	thread->m_follower = this;
	m_status           = JOINING;
	m_joinTarget       = thread;

	yield();

	ASSERT (thread == m_joinTarget);

	thread->m_follower = NULL;
	m_joinTarget = NULL;

	others_status = thread->status();

	/* Woken by the death of the thread (either timed or untimed) */
	if ( (others_status == FINISHED) || (others_status == KILLED))
	{
		PRINT_DEBUG ("Thread %u joined thread %u %s.\n", m_id, thread->m_id, timed ? "TIMED" : "");
		if (retval) *retval = thread->m_ret;
		thread->deactivate();
		return (others_status == KILLED) ? EKILLED : EOK;
	}

	/* I was obviously awaken by the timer */
	ASSERT (timed);
	PRINT_DEBUG ("Thread %u joining thread %u timedout.\n", m_id, thread->m_id);

	/* I'm no longer waiting for his death */
	return ETIMEDOUT;
}
/*----------------------------------------------------------------------------*/
void Thread::block()
{
	/* accessing both timer and scheduling queue */
	InterruptDisabler inter;

	PRINT_DEBUG ("Thread %u removing from both scheduler and timer.\n", m_id);
	removeFromHeap();
	SCHEDULER.dequeue( this );
}
/*----------------------------------------------------------------------------*/
void Thread::resume()
{
	/* accessing both timer and scheduling queue */
	InterruptDisabler inter;

	PRINT_DEBUG ("Thread %u resuming to scheduling queue.\n", m_id);
	removeFromHeap();
	SCHEDULER.enqueue( this );
}
/*----------------------------------------------------------------------------*/
bool Thread::kill()
{
	InterruptDisabler inter;

	/* only active threads can be killed */
	if ( status() != READY  && status() != BLOCKED && status() != RUNNING) {
		PRINT_DEBUG ("Thread %u cannot be killed its status is %u\n", id(), status());
		return false;
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

	if (Thread::getCurrent() == this) {
		SCHEDULER.m_shouldSwitch = true;
	} else {
		if (m_detached) deactivate();
	}
	return true;
}
/*----------------------------------------------------------------------------*/
void Thread::exit( void* return_value  )
{
	m_ret = return_value;
	PRINT_DEBUG ("Exiting thread %u with return value: %p.\n", id(), m_ret);
	bool is_detached = detached();
	kill();
	if (!is_detached)
		m_status = FINISHED;
}
/*----------------------------------------------------------------------------*/
bool Thread::deactivate()
{
	if (m_follower) return false; //cannot be deleted while i'm followed

	PRINT_DEBUG ("Deactivating thread %u(%p).\n", m_id, this);

	if (m_joinTarget)
	{
		PRINT_DEBUG ("FOUND Join target: %p.\n", m_joinTarget);
		m_joinTarget->m_follower = NULL;
		m_joinTarget = NULL;
	}
	
	m_status = UNINITIALIZED;
	THREAD_BIN.add( this );
	SCHEDULER.returnId( m_id );
	m_id = 0;
	PRINT_DEBUG("Deactivating done.\n");
	return true;
}
/*----------------------------------------------------------------------------*/
thread_t Thread::registerWithScheduler()
{
	return m_id = SCHEDULER.getFreeId( this );
}
/*----------------------------------------------------------------------------*/
Thread::~Thread()
{
	PRINT_DEBUG ("Thread %p erased from the world.===\n", this);
	if ( m_id ) {
		PRINT_DEBUG ("Returning id %d.\n", m_id);
		SCHEDULER.returnId( m_id );
	}
	PRINT_DEBUG ("Freeing stack.\n");
	free(m_stack);
}
