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
 * Some longer Thread methods like run(), setup(), yield(), sleep() ...
 */

#include "Thread.h"
#include "Kernel.h"
#include "drivers/Processor.h"
#include "address.h"
#include "api.h"
/*----------------------------------------------------------------------------*/
void Thread::run()
{
	m_runFunc(m_runData);
//	dprintf("Thread has ended\n");

	m_status = FINISHED;
	dprintf("Thread %d finished.\n", m_id);
	if (m_follower) {
		assert(m_follower->m_status == JOINING);
		m_follower->setStatus(READY);
		Scheduler::instance().enqueue(m_follower);
	}

	Scheduler::instance().dequeue(this);
	Scheduler::instance().removeThread();
	Scheduler::instance().switchThread();
	// do detached stuff
	while (true) 
		printf("Called dead Thread.\n");
}
/*----------------------------------------------------------------------------*/
Thread::Thread(void* (*thread_start)(void*), void* data, 
	unative_t flags = 0, unsigned int stackSize = DEFAULT_STACK_SIZE):
	m_stackSize(stackSize), m_runFunc(thread_start), m_runData(data), 
	m_detached(false), m_follower(NULL)
{
	if (Kernel::instance().pool().reserve() == 0) return; /* prepare listItem */

	m_stack = malloc(m_stackSize);
	if (m_stack == 0) return;  /* test stack */


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

	m_status = READY;
}
/*----------------------------------------------------------------------------*/
void Thread::yield()
{
	m_status = READY;
	Scheduler::instance().switchThread();
}
/*----------------------------------------------------------------------------*/
void Thread::sleep(const unsigned int sec)
{
	unsigned int end_time = Kernel::instance().clock().time() + sec;
	// vulnurable to Y2k38 bug :)
	// bad implementation, it will surely change when timer becomes avilable
	while(Kernel::instance().clock().time() < end_time)
		yield();
}
/*----------------------------------------------------------------------------*/
void Thread::usleep(const unsigned int usec)
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
	Scheduler::instance().dequeue(this);
	m_status = WAITING;
	Scheduler::instance().switchThread();
}
/*----------------------------------------------------------------------------*/
void Thread::wakeup()
{
//	dprintf("My(%x) status: %d\n", this, m_status);
	assert(m_status == WAITING);
	m_status = READY;
	Scheduler::instance().enqueue(this);
}
/*----------------------------------------------------------------------------*/
int Thread::join(Thread * thread)
{
	ipl_t status = Processor::save_and_disable_interupts();
	dprintf("Trying to join thread %d with thread %d\n",
		m_id, thread->id());
	if (!thread                                          /* no such thread */
		|| thread == Scheduler::instance().activeThread()  /* it's me */
		|| thread->detached()                              /* detached thread */
		|| thread->follower()                              /* already waited for */
	) {
		Processor::revert_interupt_state(status);
		return EINVAL;
	}
	if (thread->status() == KILLED) {
		delete thread;
		Processor::revert_interupt_state(status);
		return EKILLED;
	}
	if (thread->status() == FINISHED) {
		delete thread;
		Processor::revert_interupt_state(status);
		return EOK;
	}
	thread->setFollower(this);
	Scheduler::instance().dequeue(this);
	this->m_status = JOINING;
	Scheduler::instance().switchThread();
	delete thread;
	Processor::revert_interupt_state(status);
	return EOK;
}
/*----------------------------------------------------------------------------*/
void Thread::kill()
{
	m_status = KILLED;
	if (m_follower) {
		assert(m_follower->m_status == JOINING);
		m_follower->setStatus(READY);
		Scheduler::instance().enqueue(m_follower);
	}
	Scheduler::instance().dequeue(this);
	Scheduler::instance().removeThread();

	if (Scheduler::instance().activeThread() == this)
		Scheduler::instance().switchThread();
	
}
/*----------------------------------------------------------------------------*/
Thread::~Thread()
{
	Kernel::instance().free(m_stack);
	Kernel::instance().pool().free();
	Scheduler::instance().returnId(m_id);
}
/*----------------------------------------------------------------------------*/
int Thread::create(thread_t* thread_ptr, void* (*thread_start)(void*),
  void* thread_data, const unsigned int thread_flags)
{
//	if (!Kernel::instance().pool().reserve()) return ENOMEM;
	Thread* new_thread = new Thread(thread_start, thread_data);
	if (!new_thread || new_thread->m_stack == NULL ) {
		delete new_thread;
//		Kernel::instance().pool().free();
		return ENOMEM;
	}
	*thread_ptr = Scheduler::instance().getId(new_thread);
	Scheduler::instance().enqueue(new_thread);
	Scheduler::instance().addThread();
	return EOK;
}
