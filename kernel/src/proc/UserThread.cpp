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
#include "proc/Process.h"
#include "drivers/Processor.h"
#include "InterruptDisabler.h"

//#define USER_THREAD_DEBUG

#ifndef USER_THREAD_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...) \
  puts("[ USER THREAD ]: "); \
  printf(ARGS);
#endif

/*----------------------------------------------------------------------------*/
UserThread::UserThread( void* (*thread_start)(void*), void* data, void* data2,
	void* stack_pos, native_t flags, uint stack_size ):
	KernelThread( thread_start, data, flags ), m_runData2( data2 )
{
	m_status = UNINITIALIZED;

	PRINT_DEBUG ("Constructing Userland thread, suggested stack: %p.\n", stack_pos);

	unative_t vm_flags = VF_VA_USER << VF_VA_SHIFT;
	vm_flags |= VF_AT_KUSEG << VF_AT_SHIFT;

	m_userstack = stack_pos;

	if (m_virtualMap->allocate( &m_userstack, stack_size, vm_flags ) != EOK)
		return;
	
	PRINT_DEBUG ("Kernel stack at address: %p User stack: %p.\n", 
		m_stack, m_userstack);

	m_otherStackTop = (char*)m_userstack + stack_size;

	m_status = INITIALIZED;
	return;
}
/*----------------------------------------------------------------------------*/
UserThread::~UserThread()
{
	if (m_userstack) {
		ASSERT(m_virtualMap);
		m_virtualMap->free( m_userstack );
		PRINT_DEBUG ("Freed stack at address: %p.\n", m_userstack);
	}
	ASSERT (m_process);
	m_process->removeThread( this );
}
/*----------------------------------------------------------------------------*/
extern "C" void switch_to_usermode(void*, void*, void*(*)(void*), void*);
void UserThread::run()
{
	disable_interrupts();

	PRINT_DEBUG ("Started thread %u.\n", m_id);

	switch_to_usermode( m_runData, m_runData2, m_runFunc, m_stackTop );

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
