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
#include "Kernel.h"
#include "flags.h"
#include "proc/UserThread.h"
#include "tools.h"
#include "proc/Scheduler.h"
#include "InterruptDisabler.h"


//#define PROCESS_DEBUG

#ifndef PROCESS_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...) \
  puts("[ PROCESS THREAD ]: "); \
  printf(ARGS);
#endif


Process* Process::create( const char* filename )
{
	InterruptDisabler inter;

	PRINT_DEBUG ("Creating process.\n");
	VFS* fs = KERNEL.rootFS();
	file_t bin_file = fs->openFile( filename, OPEN_R );
	
	if (bin_file < 0) {
		PRINT_DEBUG ("Failed to open file %d.\n", bin_file);
		return 0;
	}

	const size_t file_size = fs->sizeFile( bin_file ); 

	if (file_size == 0) {
		PRINT_DEBUG ("File is empty.\n");
		return 0;
	}

	void * start = (void*)0x1000000;

	UserThread* main = new UserThread( (void* (*)(void*))start, NULL, TF_NEW_VMM );

	/* Thread creation might have failed. */
	if (main == NULL || (main->status() != Thread::INITIALIZED)) {
		PRINT_DEBUG ("Main thread creation failed.\n");
		delete main;
		return 0;
	}

	/* Getting id might fail */
	if (!Scheduler::instance().getId( main )) {
		PRINT_DEBUG ("Getting ID failed.\n");
		delete main;
		return 0;
	}

	Pointer<IVirtualMemoryMap> vmm = main->getVMM();

	PRINT_DEBUG ("Mapping main area.\n");

	const unative_t flags = (VF_VA_USER << VF_VA_SHIFT) | (VF_AT_KUSEG << VF_AT_SHIFT);


	/* Space allocation may fail. */
	if (vmm->allocate( &start, roundUp( file_size, 0x1000 ), flags ) != EOK) {
		PRINT_DEBUG ("Main area allocation failed.\n");
		delete main;
		return 0;
	}

	Pointer<IVirtualMemoryMap> old_vmm = IVirtualMemoryMap::getCurrent();
	ASSERT (old_vmm);
	ASSERT (old_vmm != vmm);

	char* place = (char*)malloc( file_size );
	fs->readFile( bin_file, place, file_size );
//	printf( "First int: %x.\n", *(uint*)place );
	old_vmm->copyTo( place, vmm, start, file_size );
	
	free(place);

	Process* me = new Process();
	me->m_mainThread = main;
	me->m_mainThread->resume();
	me->m_mainThread->m_process = me;
	return me;
}
/*----------------------------------------------------------------------------*/
void Process::exit()
{
	m_mainThread->kill();
}
/*----------------------------------------------------------------------------*/
Process* Process::getCurrent()
{
	return Thread::getCurrent()->process();
}
/*----------------------------------------------------------------------------*/
UserThread* Process::addThread( thread_t* thread_ptr,
	void* (*thread_start)(void*), void* thread_data, const unsigned int thread_flags )
{
  PRINT_DEBUG ("Creating Thread with userland stack...\n");

  UserThread* new_thread = new UserThread( thread_start, thread_data, thread_flags);

  PRINT_DEBUG ("Thread created at address: %p", new_thread);

  if ( (new_thread == NULL) || (new_thread->status() != Thread::INITIALIZED) ) {
    delete new_thread;
    return NULL;
  }

  *thread_ptr = Scheduler::instance().getId( new_thread );
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
	return new_thread;
}
