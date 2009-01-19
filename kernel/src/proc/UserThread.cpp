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
 * @brief Short description.
 *
 * Long description. I would paste some Loren Ipsum rubbish here, but I'm afraid
 * It would stay that way. Not that this comment is by any means ingenious but 
 * at least people can understand it. 
 */

#include "address.h"
#include "UserThread.h"
#include "drivers/Processor.h"
#include "proc/Scheduler.h"

//#define USER_THREAD_DEBUG

#ifndef USER_THREAD_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...) \
  puts("[ USER THREAD ]: "); \
  printf(ARGS);
#endif



Thread* UserThread::create( thread_t* thread_ptr, void* (*thread_start)(void*),
    void* thread_data, const unsigned int thread_flags )
{
	PRINT_DEBUG ("Creating Thread with userland stack...\n");

	Thread* new_thread = new UserThread(thread_start, thread_data, thread_flags);

	PRINT_DEBUG ("Thread created at address: %p", new_thread);

  if ( (new_thread == NULL) || (new_thread->status() != INITIALIZED) ) {
    delete new_thread;
    return NULL;
  }

  *thread_ptr = Scheduler::instance().getId(new_thread);
  if (!(*thread_ptr)) { //id space allocation failed
    delete new_thread;
    return NULL;
  }

  new_thread->resume();
  return new_thread;
}
/*----------------------------------------------------------------------------*/
UserThread::UserThread( void* (*thread_start)(void*), void* data,
  native_t flags, uint stack_size ):
	KernelThread( thread_start, data, flags )
{
	m_status = UNINITIALIZED;

	PRINT_DEBUG ("Constructing Userland thread.\n");

	unative_t vm_flags = VF_VA_USER << VF_VA_SHIFT;
	vm_flags |= VF_AT_KUSEG << VF_AT_SHIFT;

	m_userstack = (void*)(ADDR_PREFIX_KSEG0 - stack_size);

	if (m_virtualMap->allocate( &m_userstack, stack_size, vm_flags ) != EOK)
		return;
	
	PRINT_DEBUG ("Stack at address: %p.\n", m_stack);

	m_status = INITIALIZED;
	return;
}
/*----------------------------------------------------------------------------*/
UserThread::~UserThread()
{
	if (m_userstack) {
		ASSERT(m_virtualMap);
		m_virtualMap->free( m_userstack );
	}
}
/*----------------------------------------------------------------------------*/
extern "C" void switch_to_usermode(void*(*exec)(void*), void* sp);

void UserThread::run()
{
	PRINT_DEBUG ("Started thread %u.\n", m_id);

	m_stackTop = (char*)m_userstack + m_stackSize - 12;
	*(uint*)m_stackTop = 0xdead;
	switch_to_usermode( m_runFunc, m_stackTop);

/*
	asm volatile(
		"jal %0\n"
		"nop"
		:
		:"r" (m_runFunc)
		: 
	);
*/
//	m_runFunc(NULL);

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
