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
		thread_wakeup(m_follower->id());
	thread_suspend();
	while (true) 
		printf("Called dead Thread.\n");
}
/*----------------------------------------------------------------------------*/
int Thread::setup()
{
	m_stack = malloc(m_stackSize);
	if (!m_stack) return ENOMEM;
	using namespace Processor;

	m_stackTop = (void*)((uintptr_t)m_stack + m_stackSize - sizeof(Context));
	Context * context = (Context*)(m_stackTop);
	
	//would use run but virtual methods are tricky
	void (Thread::*runPtr)(void) = &Thread::start; 
	//http://www.goingware.com/tips/member-pointers.html
	//dereferencing address trick, advised by M. Burda
	context->ra = *(unative_t*)(&runPtr);
	//the first (and the only) param
	context->a0 = (unative_t)this;
	// global pointer
	context->gp = ADDR_TO_KSEG0(0);
	context->sp = (unative_t)m_stackTop;
	context->status = STATUS_IM_MASK | STATUS_IE_MASK | STATUS_CU0_MASK;
	return EOK;
}
/*----------------------------------------------------------------------------*/
void Thread::yield()
{
	Kernel::instance().yield();
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
