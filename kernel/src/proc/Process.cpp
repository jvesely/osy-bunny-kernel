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

#include "api.h"
#include "Process.h"
#include "flags.h"
#include "proc/UserThread.h"
#include "tools.h"
#include "InterruptDisabler.h"
#include "address.h"
#include "synchronization/Event.h"
#include "ProcessInfo.h"
#include "ProcessTable.h"

//#define PROCESS_DEBUG

#ifndef PROCESS_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...) \
  puts("[ PROCESS THREAD ]: "); \
  printf(ARGS);
#endif


Process* Process::create( const void* image, size_t size )
{
	InterruptDisabler inter;

	PRINT_DEBUG ("Creating process.\n");

	void * (*start)(void*) = (void*(*)(void*))0x1000000;
	const size_t request = roundUp( size + sizeof(ProcessInfo), Processor::pages[Processor::PAGE_MIN].size );

	ProcessInfo * info   = (ProcessInfo*)((char*)start + request - sizeof(ProcessInfo));
	
	PRINT_DEBUG ("Requested size: %x,%x(%p), info at %p.\n", size, request, start, info);


	UserThread* main = new UserThread(
		start, info, NULL, (char*)ADDR_PREFIX_KSEG0 - Thread::DEFAULT_STACK_SIZE, TF_NEW_VMM );

	/* Thread creation might have failed. */
	if (main == NULL || (main->status() != Thread::INITIALIZED)) {
		PRINT_DEBUG ("Main thread creation failed %p.\n", main);
		delete main;
		return NULL;
	}

	/* Getting id might fail */
	if (! main->registerWithScheduler()) {
		PRINT_DEBUG ("Getting ID failed.\n");
		delete main;
		return NULL;
	}

	Pointer<IVirtualMemoryMap> vmm = main->getVMM();

	PRINT_DEBUG ("Mapping main area.\n");

	const unative_t flags = (VF_VA_USER << VF_VA_SHIFT) | (VF_AT_KUSEG << VF_AT_SHIFT);


	/* Space allocation may fail. */
	if (vmm->allocate( (void**)&start, request, flags ) != EOK) {
		PRINT_DEBUG ("Main area allocation failed.\n");
		delete main;
		return NULL;
	}

	Pointer<IVirtualMemoryMap> old_vmm = IVirtualMemoryMap::getCurrent();
	ASSERT (old_vmm);
	ASSERT (old_vmm != vmm);


	old_vmm->copyTo( image, vmm, (void*)start, size );

	Process* me  = new Process();
	me->m_id     = PIDTable.getFreeId( me );

	if (!me->m_id){
		delete me;
		delete main;
		return NULL;
	}

	old_vmm->copyTo( &(me->m_id), vmm, (void*)info, sizeof( me->m_id ) );

	me->m_mainThread = main;
	me->m_mainThread->resume();
	me->m_mainThread->m_process = me;
	me->m_info = info;
//	me->m_info->PID = me->m_id;
	PRINT_DEBUG ("Created process, info at %p.\n", info);
	return me;
}
/*----------------------------------------------------------------------------*/
void Process::setActiveThread( thread_t thread )
{
	m_info->RunningThread = thread;
	PRINT_DEBUG ("Setting active thread for process %u: %u.(%u,%u)\n", m_id, thread, m_info->PID, m_info->RunningThread);
}
/*----------------------------------------------------------------------------*/
void Process::clearEvents()
{
	PRINT_DEBUG ("Clearing used events: %u.\n", eventTable.map().size());
	const uint list_count = eventTable.map().getArraySize();
	for (uint i = 0; i < list_count; ++i)
	{
		List< Pair<event_t, Event*> >* list =	eventTable.map().getList( i );
		List< Pair<event_t, Event*> >::Iterator it;
		ASSERT (list);
		for (it = list->begin(); it != list->end(); ++it) {
			if (it->second) {//don't fire dummy event
				PRINT_DEBUG ("Found event FIRING.\n");
				it->second->fire();
			}
			delete it->second;
		}
		list->clear();
	}
	PRINT_DEBUG ("Events cleared.\n");
}
/*----------------------------------------------------------------------------*/
void Process::clearThreads()
{
	PRINT_DEBUG ("Clearing process threads.\n");
	m_mainThread->deactivate() || m_mainThread->kill();
	
	UserThreadList::Iterator it;
	for ( it = m_list.begin(); it != m_list.end(); ++it )
	{
		(*it)->deactivate() || (*it)->kill();
	}
	m_list.clear();
	PRINT_DEBUG ("Threads cleared");
}
/*----------------------------------------------------------------------------*/
void Process::exit()
{
	clearEvents();
	clearThreads();
}
/*----------------------------------------------------------------------------*/
Process* Process::getCurrent()
{
	return Thread::getCurrent()->process();
}
/*----------------------------------------------------------------------------*/
int Process::join( const Time* time )
{
	ASSERT (Thread::getCurrent()->process() != this);
	return Thread::getCurrent()->join( m_mainThread, NULL, time, *time);
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
	PRINT_DEBUG ("New thread added to the process and scheduler %u.\n", new_thread->id());
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
