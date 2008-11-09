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
/*----------------------------------------------------------------------------*/
Thread::Thread( 
	unative_t flags = 0, unsigned int stackSize = DEFAULT_STACK_SIZE):
	ListInsertable<Thread>(), HeapInsertable<Thread, Time, 4>(),
	m_stackSize(stackSize),	m_detached(false), m_status(UNITIALIZED), 
	m_id(0), m_follower(NULL)
{
	m_stack = malloc(m_stackSize);
	if (m_stack == NULL) return;  /* test stack */

	using namespace Processor;
	
	m_stackTop = (void*)((uintptr_t)m_stack + m_stackSize - sizeof(Context));
	Context * context = (Context*)(m_stackTop);
	
	//would use run but virtual methods are tricky
	void (Thread::*runPtr)(void) = &Thread::start; 
	//http://www.goingware.com/tips/member-pointers.html
	//dereferencing address trick, advised by M. Burda
	context->ra = *(unative_t*)(&runPtr);  /* return address (run this) */
	context->a0 = (unative_t)this;         /* the first and the only argument */
	context->gp = ADDR_TO_KSEG0(0);        /* global pointer */
	context->status = STATUS_IM_MASK | STATUS_IE_MASK | STATUS_CU0_MASK;
	
	dprintf("Created thread %p id: %d and stack: %p\n", this, m_id, m_stack);

	m_status = INITIALIZED;
}
/*----------------------------------------------------------------------------*/
void Thread::yield() const
{
	Scheduler::instance().switchThread();
}
/*----------------------------------------------------------------------------*/
void Thread::sleep(const uint sec)
{
	InterruptDisabler inter;
	//unsigned int end_time = Kernel::instance().clock().time() + sec;
	// vulnurable to Y2k38 bug :)
	// bad implementation, it will surely change when timer becomes avilable
	m_status = BLOCKED;
	Timer::instance().plan(this, sec);
	yield();
//	while(Kernel::instance().clock().time() < end_time)
//		yield();
}
/*----------------------------------------------------------------------------*/
void Thread::usleep(const uint usec)
{
	if (usec >= RTC::SECOND)
		sleep(usec / RTC::SECOND);
	unsigned int start_time = Kernel::instance().clock().usec();
	unsigned int end_time = (start_time + usec) % RTC::SECOND; 
	
	if (end_time < start_time)
		while (Kernel::instance().clock().usec() > start_time) {
			yield();
		}
	
	while (Kernel::instance().clock().usec() < end_time) {
		yield();
	}
}
/*----------------------------------------------------------------------------*/
void Thread::suspend()
{
	assert(m_status == RUNNING);
	m_status = WAITING;
	Scheduler::instance().dequeue(this);
	Scheduler::instance().switchThread();
}
/*----------------------------------------------------------------------------*/
void Thread::wakeup() const
{
//	dprintf("My(%x) status: %d\n", this, m_status);
	assert(m_status == WAITING);
	Scheduler::instance().enqueue(const_cast<Thread*>(this));
}
/*----------------------------------------------------------------------------*/
int Thread::join(Thread * thread)
{
	InterruptDisabler interrupts;
	dprintf("Trying to join thread %d with thread %d (status: %d)\n",
		m_id, thread->m_id, thread->m_status);
	if (!thread                                          /* no such thread */
		|| thread == Scheduler::instance().activeThread()  /* it's me */
		|| thread->detached()                              /* detached thread */
		|| thread->follower()                              /* already waited for */
	) {
		return EINVAL;
	}
	if (thread->status() == KILLED) {
		delete thread;
		return EKILLED;
	}
	if (thread->status() == FINISHED) {
		dprintf("Thread %d already finished(%d).\n", thread->m_id, thread->m_detached);
		delete thread;
		return EOK;
	}
	thread->setFollower(this);
	m_status = JOINING;
	Scheduler::instance().dequeue(this);
	Scheduler::instance().switchThread();
	assert( (thread->m_status == FINISHED) || (thread->m_status == KILLED) );
	delete thread;
	return EOK;
}
/*----------------------------------------------------------------------------*/
void Thread::kill()
{
	dprintf("Started kill %p\n", this);
	InterruptDisabler inter;
	dprintf("Killing thread %d ().\n", m_id);
	if ( (m_status == KILLED) || (m_status == FINISHED) )
		return;
	m_status = KILLED;

	if (m_follower) {
		dprintf("Somenone is expecting me to die!!\n");
		assert(m_follower->m_status == JOINING);
		Scheduler::instance().enqueue(m_follower);
	}
	
	
	dprintf("Oficially dead getting off the list.\n");
	Scheduler::instance().dequeue(this);

	if (Scheduler::instance().activeThread() == this)
		Scheduler::instance().switchThread();
	else if (m_detached) delete this;
}
/*----------------------------------------------------------------------------*/
Thread::~Thread()
{
	dprintf("Deleting thread %u (det:%d)\n", m_id, m_detached);
	Kernel::instance().free(m_stack);
}

