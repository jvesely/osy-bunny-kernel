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

#include "Process.h"
#include "flags.h"
#include "proc/UserThread.h"
#include "tools.h"
#include "InterruptDisabler.h"
#include "address.h"
#include "synchronization/Event.h"

//#define PROCESS_DEBUG

#ifndef PROCESS_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...) \
  puts("[ PROCESS THREAD ]: "); \
  printf(ARGS);
#endif


Process* Process::create( const char* image, size_t size )
{
	InterruptDisabler inter;

	PRINT_DEBUG ("Creating process.\n");
	

	void * (*start)(void*) = (void*(*)(void*))0x1000000;

	UserThread* main = new UserThread( 
		start, NULL, NULL, (char*)ADDR_PREFIX_KSEG0 - Thread::DEFAULT_STACK_SIZE, TF_NEW_VMM );

	/* Thread creation might have failed. */
	if (main == NULL || (main->status() != Thread::INITIALIZED)) {
		PRINT_DEBUG ("Main thread creation failed %p.\n", main);
		delete main;
		return 0;
	}

	/* Getting id might fail */
	if (! main->registerWithScheduler()) {
		PRINT_DEBUG ("Getting ID failed.\n");
		delete main;
		return 0;
	}

	Pointer<IVirtualMemoryMap> vmm = main->getVMM();

	PRINT_DEBUG ("Mapping main area.\n");

	const unative_t flags = (VF_VA_USER << VF_VA_SHIFT) | (VF_AT_KUSEG << VF_AT_SHIFT);


	/* Space allocation may fail. */
	if (vmm->allocate( (void**)&start, roundUp( size, Processor::pages[Processor::PAGE_MIN].size ), flags ) != EOK) {
		PRINT_DEBUG ("Main area allocation failed.\n");
		delete main;
		return 0;
	}

	Pointer<IVirtualMemoryMap> old_vmm = IVirtualMemoryMap::getCurrent();
	ASSERT (old_vmm);
	ASSERT (old_vmm != vmm);


	old_vmm->copyTo( image, vmm, (void*)start, size );
	
	Process* me = new Process();
	me->m_mainThread = main;
	me->m_mainThread->resume();
	me->m_mainThread->m_process = me;
	return me;
}
/*----------------------------------------------------------------------------*/
void Process::clearEvents()
{
	const uint list_count = eventTable.map().getArraySize();
	for (uint i = 0; i < list_count; ++i)
	{
		List< Pair<event_t, Event*> >* list =	eventTable.map().getList( i );
		List< Pair<event_t, Event*> >::Iterator it;
		ASSERT (list);
		for (it = list->begin(); it != list->end(); ++it){
			it->second->fire();
			delete it->second;
		}
		list->clear();
	}
}
/*----------------------------------------------------------------------------*/
void Process::clearThreads()
{
	m_mainThread->deactivate();
	for ( UserThreadList::Iterator it = m_list.begin(); it != m_list.end(); ++it)
	{
		Thread::Status st = (*it)->status();
		if (st != Thread::KILLED && st != Thread::FINISHED) {
			(*it)->deactivate();
		}
	}

}
/*----------------------------------------------------------------------------*/
void Process::exit()
{
	clearEvents();
//	clearThreads();
}
/*----------------------------------------------------------------------------*/
Process* Process::getCurrent()
{
	return Thread::getCurrent()->process();
}
/*----------------------------------------------------------------------------*/
UserThread* Process::addThread( thread_t* thread_ptr,
	void* (*thread_start)(void*), void* arg1, void* arg2, const uint thread_flags )
{
	InterruptDisabler inter;

  PRINT_DEBUG ("Creating Thread with userland stack...%p(%p,%p)\n",
		thread_start, arg1, arg2);

	void* stack_pos =
		(char*)ADDR_PREFIX_KSEG0 - (m_list.size() + 2) * Thread::DEFAULT_STACK_SIZE;

  UserThread* new_thread = new UserThread( 
		thread_start, arg1, arg2, stack_pos, thread_flags);

  PRINT_DEBUG ("Thread created at address: %p.\n", new_thread);

  if ( (new_thread == NULL) || (new_thread->status() != Thread::INITIALIZED) ) {
    delete new_thread;
    return NULL;
  }

  *thread_ptr =  new_thread->registerWithScheduler();
  if (!(*thread_ptr)) { //id space allocation failed
    delete new_thread;
    return NULL;
  }
	ListItem<UserThread*>* storage = new ListItem<UserThread*>( new_thread );
	if (!storage) {
		delete new_thread;
		return NULL;
	}
	m_list.pushBack( storage );
	new_thread->m_process = this;
  new_thread->resume();
	PRINT_DEBUG ("New thread added to the process and scheduler.\n");
	return new_thread;
}
/*----------------------------------------------------------------------------*/
Thread* Process::getThread( thread_t thread )
{
	Thread* ptr = Thread::fromId( thread );
	if (ptr && ptr->process() == this)
		return ptr;
	return NULL;
}
/*----------------------------------------------------------------------------*/
bool Process::removeThread( UserThread* thread )
{
	ASSERT (thread);
	ASSERT (thread->process() == this);
	ListItem<UserThread*>* item =  m_list.removeFind(thread);
	delete item;
	return item;
}
