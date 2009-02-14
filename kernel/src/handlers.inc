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
#include "proc/Process.h"
#include "proc/Thread.h"

#include "synchronization/Event.h"
#include "tools.h"

//#define SYSCALL_HANDLER_DEBUG

#ifndef SYSCALL_HANDLER_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...) \
  puts("[ DEBUG SYSCALL ]: "); \
  printf(ARGS);
#endif



/*----------------------------------------------------------------------------*/
#define PROCESS_THREAD( id ) \
	({ \
	Process* current = Process::getCurrent(); \
	ASSERT (current); \
	Thread* thread = current->getThread( id ); \
	if (!thread) return EINVAL; \
	thread;\
	})
/*----------------------------------------------------------------------------*/
#define CHECK_PTR_IN_USEG( ptr ) \
	({ \
	if (! (ADDR_IN_USEG( (uintptr_t)(ptr) ) )) \
	{ \
		Thread::getCurrent()->kill(); \
		return EKILLED; \
	}; \
	(void*)(ptr);\
	})
/*----------------------------------------------------------------------------*/
static unative_t handlePuts( unative_t params[] )
{
	const char * str = (const char*)CHECK_PTR_IN_USEG(params[0]);
	return puts( str );
}
/*----------------------------------------------------------------------------*/
static unative_t handleGets( unative_t params[] )
{
	char* place = (char*)CHECK_PTR_IN_USEG(params[0]);

	if (params[1] == 1) {
		return (*place = getc()), 1;
	} else {
		return gets(place, params[1]);
	}
}
/*----------------------------------------------------------------------------*/
static unative_t handleExit( unative_t params[] )
{
	Process* current = Process::getCurrent();
	ASSERT (current);
	current->exit();
	return 0;
}
/*----------------------------------------------------------------------------*/
static unative_t handleThreadCreate( unative_t params[] )
{
	Process* current = Process::getCurrent();
	ASSERT (current);

	thread_t* thread_ptr = (thread_t*)CHECK_PTR_IN_USEG(params[0]);

	bool success = current->addThread( 
		thread_ptr, (void*(*)(void*))params[1], (void*)params[2], (void*)params[3] );
	PRINT_DEBUG ("Thread create handled : %s.\n", success ? "OK" : "FAIL");
	return success ? EOK : ENOMEM;
}
/*----------------------------------------------------------------------------*/
static unative_t handleThreadJoin( unative_t params[] )
{
	Thread* thr = PROCESS_THREAD( params[0] );
	bool timed = (bool)params[2];

	const Time* time = (const Time*)CHECK_PTR_IN_USEG(params[2]);
	void** place     = (void**)CHECK_PTR_IN_USEG(params[1]);
	
	PRINT_DEBUG ("Handling thread join on %u, %s, %p.\n", thr->id(),
		timed?"TIMED":"DIRECT", params[3]);
	
	return Thread::getCurrent()->join( thr, place, timed, *time );
}
/*----------------------------------------------------------------------------*/
static unative_t handleThreadSelf( unative_t params[] )
{
	return Thread::getCurrent()->id();
}
/*----------------------------------------------------------------------------*/
static unative_t handleThreadDetach( unative_t params[] )
{
	Thread* thr = PROCESS_THREAD( params[0] );
	bool success = thr->detach();
	PRINT_DEBUG ("Handling detach %s.\n", success ? "OK" : "FAIL");
	return success ? EOK : EINVAL;
}
/*----------------------------------------------------------------------------*/
static unative_t handleThreadSuspend( unative_t params[] )
{
	Thread::getCurrent()->suspend();
	return EOK;
}
/*----------------------------------------------------------------------------*/
static unative_t handleThreadWakeup( unative_t params[] )
{
	Thread* thr = PROCESS_THREAD( params[0] );
	thr->wakeup();
	return EOK;
}
/*----------------------------------------------------------------------------*/
static unative_t handleThreadYield( unative_t params[] )
{
	Thread::getCurrent()->yield();
	return EOK;
}
/*----------------------------------------------------------------------------*/
static unative_t handleThreadCancel( unative_t params[] )
{
	Thread* thr = PROCESS_THREAD( params[0] );
	thr->kill();
	return EOK;
}
/*----------------------------------------------------------------------------*/
static unative_t handleThreadExit( unative_t params[] )
{
	Thread::getCurrent()->exit( (void*)params[0] );
	return EOK;
}
/*----------------------------------------------------------------------------*/
static unative_t handleThreadSleep( unative_t params[] )
{

	const Time* time = (const Time*)CHECK_PTR_IN_USEG(params[0]);

	PRINT_DEBUG ("Sleep:  %p %u:%u.\n", time, time->secs(), time->usecs());

	Thread::getCurrent()->sleep( *time );
	return 0;
}
/*----------------------------------------------------------------------------*/
static unative_t handleEventInit( unative_t params[] )
{
	// is it a pointer in USEG?
	event_t* event_ptr = (event_t*)CHECK_PTR_IN_USEG(params[0]);
	
	Event* event = new Event;
	if (!event) return ENOMEM;

	event_t ev = CURRENT_EVENT_TABLE.getFreeId( event );
	if (ev == CURRENT_EVENT_TABLE.BAD_ID) {
		delete event;
		return ENOMEM;
	}

	*event_ptr = ev;

	return EOK;
}
/*----------------------------------------------------------------------------*/
static unative_t handleEventWait( unative_t params[] )
{
	PRINT_DEBUG("handleEventWait() started\n");
	
	event_t   event   = params[0];
	native_t* locked = (native_t*)CHECK_PTR_IN_USEG(params[1]);

	Event* ev = CURRENT_EVENT_TABLE.translateId( event );

	if (!ev) {
		Thread::getCurrent()->kill();
		return 0;
	}

	if (*locked) {
		PRINT_DEBUG("handleEventWait(): mutex still locked, so waiting.\n");
		ev->wait();
	}

	return 0;
}
/*----------------------------------------------------------------------------*/
static unative_t handleEventWaitTimeout( unative_t params[] )
{
	PRINT_DEBUG("handleEventWaitTimeout() started\n");
	event_t event = params[0];

	// Are these pointers to USEG?
	Time* time                 = (Time*)CHECK_PTR_IN_USEG(params[1]);
	volatile unative_t* locked = (unative_t*)CHECK_PTR_IN_USEG(params[2]);
	
	PRINT_DEBUG("handleEventWaitTimeout() time pointer: %p, locked: %p\n", time, locked);
	
	const Time alartime = Time::getCurrent() + *time;
	
	Event* ev = CURRENT_EVENT_TABLE.translateId( event );
	if (!ev) {
		Thread::getCurrent()->kill();
		return 0;
	}
	
	if (*locked)
		ev->waitTimeout(*time);

	const Time current = Time::getCurrent();
	
	if (current >= alartime) {
		*time = Time();
		PRINT_DEBUG("handleEventWaitTimeout() timedout.\n");
		return ETIMEDOUT;
	}
	
	PRINT_DEBUG("handleEventWaitTimeout() woken.\n");
	*time = alartime - current;
	return EOK;
}
/*----------------------------------------------------------------------------*/
static unative_t handleEventFire( unative_t params[] )
{
	event_t event = params[0];

	Event* ev = CURRENT_EVENT_TABLE.translateId(event);
	if (!ev) {
		Thread::getCurrent()->kill();
		return 0;
	}

	ev->fire();

	return 0;
}
/*----------------------------------------------------------------------------*/
static unative_t handleEventDestroy( unative_t params[] )
{
	event_t event = params[0];

	Event* ev = CURRENT_EVENT_TABLE.translateId( event );

	PRINT_DEBUG ("Handling event destroy.\n");

	if (!ev || ev->waiting()) {
		Thread::getCurrent()->kill();
		return 0;
	}

	CURRENT_EVENT_TABLE.returnId( event );
	delete ev;

	PRINT_DEBUG ("Destroy handled.\n");

	return EOK;
}
/*----------------------------------------------------------------------------*/
static unative_t handleVMAAlloc( unative_t params[] )
{
	void**  area_start     = (void**) CHECK_PTR_IN_USEG(params[0]);
	size_t* size           = (size_t*)CHECK_PTR_IN_USEG(params[1]);
	IVirtualMemoryMap* vmm = IVirtualMemoryMap::getCurrent().data();	

	ASSERT (vmm);

	//round the size according to HW specifications
	*size = roundUp(*size, Processor::pages[Processor::PAGE_MIN].size);

	return vmm->allocate(area_start, *size, params[2]);
}
/*----------------------------------------------------------------------------*/
static unative_t handleVMAFree( unative_t params[] )
{
	//the same as vma_free in api.h
	IVirtualMemoryMap* vmm = IVirtualMemoryMap::getCurrent().data();
	ASSERT (vmm);

	return vmm->free((void *)params[0]);
}
/*----------------------------------------------------------------------------*/
unative_t handleVMAResize( unative_t params[] )
{
	void*  area_start = (void*)  CHECK_PTR_IN_USEG(params[0]);
	size_t* size      = (size_t*)CHECK_PTR_IN_USEG(params[1]);
	//the same as vma_resize in api.h
	IVirtualMemoryMap* vmm = IVirtualMemoryMap::getCurrent().data();
	ASSERT (vmm);

	//round the size according to HW specifications
	*size = roundUp(*size, Processor::pages[Processor::PAGE_MIN].size);

	return vmm->resize(area_start, *size);
}
//------------------------------------------------------------------------------
unative_t handleGetTime( unative_t params[])
{
	Time* time_place = (Time*) CHECK_PTR_IN_USEG(params[0]);
	*time_place = Time::getCurrentTime();

	return EOK;
}
