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
	dprintf("Thread has ended\n");

	m_status = FINISHED;
	if (m_follower)
		m_follower->wakeup();
	Scheduler::instance().dequeue(this);
	Scheduler::instance().switchThread();
	// do detached stuff
	while (true) 
		printf("Called dead Thread.\n");
}
/*----------------------------------------------------------------------------*/
Thread::Thread(void* (*thread_start)(void*), void* data, 
	unative_t flags = 0, unsigned int stackSize = DEFAULT_STACK_SIZE):
	m_stackSize(stackSize), m_runFunc(thread_start), m_runData(data), m_follower(NULL)
{
	m_stack = malloc(m_stackSize);
	if (!m_stack) return;
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
	context->sp = (unative_t)m_stackTop;   /* stack top */
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
	//another stupid implementation
	unsigned int start_time = Kernel::instance().clock().usec();
	unsigned int end_time = start_time + usec; // may overflow
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
	assert(m_status == WAITING);
	m_status = READY;
	Scheduler::instance().enqueue(this);
}
/*----------------------------------------------------------------------------*/
void Thread::kill()
{

}
/*----------------------------------------------------------------------------*/
int Thread::create(thread_t* thread_ptr, void* (*thread_start)(void*),
  void* thread_data, const unsigned int thread_flags)
{
	dprintf("Creating thread...\n");
	if (!Kernel::instance().pool().reserve()) return ENOMEM;
	dprintf("Reserved ListItem..\n");
	Thread* new_thread = new Thread(thread_start, thread_data);
	if (!new_thread || !new_thread->isOK() ) {
		delete new_thread;
		return ENOMEM;
	}
	dprintf("Thread created...\n");
	
	Scheduler::instance().getId(new_thread);
	Scheduler::instance().enqueue(new_thread);
	return new_thread->id();
}
