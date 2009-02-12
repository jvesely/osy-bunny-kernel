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

#include "SyscallHandler.h"
#include "address.h"
#include "syscallcodes.h"
#include "proc/Process.h"
#include "proc/Thread.h"

#include "synchronization/EventMap.h"
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


SyscallHandler::SyscallHandler()
{
	m_handles[SYS_PUTS] = (&SyscallHandler::handlePuts);
	m_handles[SYS_GETS] = (&SyscallHandler::handleGets);
	m_handles[SYS_EXIT] = (&SyscallHandler::handleExit);

	m_handles[SYS_THREAD_CREATE]  = (&SyscallHandler::handleThreadCreate);
	m_handles[SYS_THREAD_SELF]    = (&SyscallHandler::handleThreadSelf);
	m_handles[SYS_THREAD_JOIN]    = (&SyscallHandler::handleThreadJoin);
	m_handles[SYS_THREAD_DETACH]  = (&SyscallHandler::handleThreadDetach);
	m_handles[SYS_THREAD_CANCEL]  = (&SyscallHandler::handleThreadCancel);
	m_handles[SYS_THREAD_SLEEP]   = (&SyscallHandler::handleThreadSleep);
	m_handles[SYS_THREAD_YIELD]   = (&SyscallHandler::handleThreadYield);
	m_handles[SYS_THREAD_SUSPEND] = (&SyscallHandler::handleThreadSuspend);
	m_handles[SYS_THREAD_WAKEUP]  = (&SyscallHandler::handleThreadWakeup);
	m_handles[SYS_THREAD_EXIT]    = (&SyscallHandler::handleThreadExit);

	m_handles[SYS_EVENT_INIT] = (&SyscallHandler::handleEventInit);
	m_handles[SYS_EVENT_WAIT] = (&SyscallHandler::handleEventWait);
	m_handles[SYS_EVENT_WAIT_TIMEOUT] = (&SyscallHandler::handleEventWaitTimeout);
	m_handles[SYS_EVENT_FIRE] = (&SyscallHandler::handleEventFire);
	m_handles[SYS_EVENT_DESTROY] = (&SyscallHandler::handleEventDestroy);

	m_handles[SYS_VMA_ALLOC] = (&SyscallHandler::handleVMAAlloc);
	m_handles[SYS_VMA_FREE] = (&SyscallHandler::handleVMAFree);
	m_handles[SYS_GET_TIME] = (&SyscallHandler::handleGetTime);
}
/*----------------------------------------------------------------------------*/
bool SyscallHandler::handleException( Processor::Context* registers )
{
	m_call    = ((*(unative_t*)registers->epc)>>6);

	m_params[0] = registers->a0;
	m_params[1] = registers->a1;
	m_params[2] = registers->a2;
	m_params[3] = registers->a3;
/*
	printf( "Handling syscall: %x, with params %x,%x,%x,%x.\n",
		m_call, m_params[0], m_params[1], m_params[2], m_params[3]);
// */
	registers->epc += 4;

	if (!m_handles[m_call]) return false;

	registers->v0   = (this->*m_handles[m_call]) ();
	return true;
}
/*----------------------------------------------------------------------------*/
#define PROCESS_THREAD() \
	({ \
	Process* current = Process::getCurrent(); \
	ASSERT (current); \
	Thread* thread = current->getThread( m_params[0] ); \
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
unative_t SyscallHandler::handlePuts()
{
	const char * str = (const char*)CHECK_PTR_IN_USEG(m_params[0]);
	return puts( str );
}
/*----------------------------------------------------------------------------*/
unative_t SyscallHandler::handleGets()
{
	char* place = (char*)CHECK_PTR_IN_USEG(m_params[0]);

	if (m_params[1] == 1) {
		return (*place = getc()), 1;
	} else {
		return gets(place, m_params[1]);
	}
}
/*----------------------------------------------------------------------------*/
unative_t SyscallHandler::handleExit()
{
	Process* current = Process::getCurrent();
	ASSERT (current);
	current->exit();
	return 0;
}
/*----------------------------------------------------------------------------*/
unative_t SyscallHandler::handleThreadCreate()
{
	Process* current = Process::getCurrent();
	ASSERT (current);

	thread_t* thread_ptr = (thread_t*)CHECK_PTR_IN_USEG(m_params[0]);

	bool success = current->addThread(
		thread_ptr, (void*(*)(void*))m_params[1], (void*)m_params[2], (void*)m_params[3] );
	PRINT_DEBUG ("Thread create handled : %s.\n", success ? "OK" : "FAIL");
	return success ? EOK : ENOMEM;
}
/*----------------------------------------------------------------------------*/
unative_t SyscallHandler::handleThreadJoin()
{
	Thread* thr = PROCESS_THREAD();
	bool timed = (bool)m_params[2];

	const Time* time = (const Time*)CHECK_PTR_IN_USEG(m_params[2]);

	PRINT_DEBUG ("Handling thread join on %u, %s, %p.\n", thr->id(),
		timed?"TIMED":"DIRECT", m_params[3]);

	return Thread::getCurrent()->join( thr, (void**)m_params[1], timed, *time );
}
/*----------------------------------------------------------------------------*/
unative_t SyscallHandler::handleThreadSelf()
{
	return Thread::getCurrent()->id();
}
/*----------------------------------------------------------------------------*/
unative_t SyscallHandler::handleThreadDetach()
{
	Thread* thr = PROCESS_THREAD();
	bool success = thr->detach();
	PRINT_DEBUG ("Handling detach %s.\n", success ? "OK" : "FAIL");
	return success ? EOK : EINVAL;
}
/*----------------------------------------------------------------------------*/
unative_t SyscallHandler::handleThreadSuspend()
{
	Thread::getCurrent()->suspend();
	return EOK;
}
/*----------------------------------------------------------------------------*/
unative_t SyscallHandler::handleThreadWakeup()
{
	Thread* thr = PROCESS_THREAD();
	thr->wakeup();
	return EOK;
}
/*----------------------------------------------------------------------------*/
unative_t SyscallHandler::handleThreadYield()
{
	Thread::getCurrent()->yield();
	return EOK;
}
/*----------------------------------------------------------------------------*/
unative_t SyscallHandler::handleThreadCancel()
{
	Thread* thr = PROCESS_THREAD();
	thr->kill();
	return 0;
}
/*----------------------------------------------------------------------------*/
unative_t SyscallHandler::handleThreadExit()
{
	Thread::getCurrent()->exit( (void*)m_params[0] );
	return 0;
}
/*----------------------------------------------------------------------------*/
unative_t SyscallHandler::handleThreadSleep()
{

	const Time* time = (const Time*)CHECK_PTR_IN_USEG(m_params[0])

	PRINT_DEBUG ("Sleep:  %p %u:%u.\n", time, time->secs(), time->usecs());

	Thread::getCurrent()->sleep( *time );
	return 0;
}
/*----------------------------------------------------------------------------*/
unative_t SyscallHandler::handleEventInit()
{
	// is it a pointer in USEG?
	event_t* event_ptr = (event_t*)CHECK_PTR_IN_USEG(m_params[0]);

	Event* evnt = new Event;
	if (!evnt) return ENOMEM;

	event_t ev = EventMap::instance().map(evnt);
	if (ev == EventMap::INVALID_ID)
		return ENOMEM;

	*event_ptr = ev;

	return EOK;
}
/*----------------------------------------------------------------------------*/
unative_t SyscallHandler::handleEventWait()
{
	PRINT_DEBUG("SyscallHandler::handleEventWait() started\n");

	event_t   evnt   = m_params[0];
	native_t* locked = (native_t*)CHECK_PTR_IN_USEG(m_params[1]);

	Event* ev = EventMap::instance().getEvent(evnt);

	if (!ev) {
		Thread::getCurrent()->kill();
		return 0;
	}

	if (*locked) {
		PRINT_DEBUG("SyscallHandler::handleEventWait(): mutex still locked, so waiting.\n");
		ev->wait();
	}

	return 0;
}
/*----------------------------------------------------------------------------*/
unative_t SyscallHandler::handleEventWaitTimeout()
{
	PRINT_DEBUG("SyscallHandler::handleEventWaitTimeout() started\n");
	event_t evnt = m_params[0];

	// Are these pointers to USEG?
	Time* time                = (Time*)CHECK_PTR_IN_USEG(m_params[1]);
	volatile native_t* locked = (native_t*)CHECK_PTR_IN_USEG(m_params[2]);

	PRINT_DEBUG("SyscallHandler::handleEventWaitTimeout() time pointer: %p, locked: %p\n", time, locked);

	const Time alarm_time = Time::getCurrent() + *time;

	Event* ev = EventMap::instance().getEvent(evnt);
	if (!ev) {
		Thread::getCurrent()->kill();
		return 0;
	}

	if (*locked)
		ev->waitTimeout(*time);

	const Time current = Time::getCurrent();

	if (current >= alarm_time) {
		*time = Time();
		PRINT_DEBUG("SyscallHandler::handleEventWaitTimeout() timedout.\n");
		return ETIMEDOUT;
	}

	PRINT_DEBUG("SyscallHandler::handleEventWaitTimeout() woken.\n");
	*time = alarm_time - current;
	return EOK;
}
/*----------------------------------------------------------------------------*/
unative_t SyscallHandler::handleEventFire()
{
	event_t evnt = m_params[0];

	Event* ev = EventMap::instance().getEvent(evnt);
	if (!ev) {
		Thread::getCurrent()->kill();
		return 0;
	}

	ev->fire();

	return 0;
}
/*----------------------------------------------------------------------------*/
unative_t SyscallHandler::handleEventDestroy()
{
	event_t evnt = m_params[0];

	Event* ev = EventMap::instance().getEvent(evnt);

	if (!ev || ev->waiting()) {
		Thread::getCurrent()->kill();
		return 0;
	}

	EventMap::instance().unmap(evnt);
	delete(ev);

	return EOK;
}
/*----------------------------------------------------------------------------*/
unative_t SyscallHandler::handleVMAAlloc()
{
	void**  area_start     = (void**) CHECK_PTR_IN_USEG(m_params[0]);
	size_t* size           = (size_t*)CHECK_PTR_IN_USEG(m_params[1]);
	IVirtualMemoryMap* vmm = IVirtualMemoryMap::getCurrent().data();

	ASSERT (vmm);

	//round the size according to HW specifications
	*size = roundUp(*size, Processor::pages[Processor::PAGE_MIN].size);

	return vmm->allocate(area_start, *size, m_params[2]);
}
/*----------------------------------------------------------------------------*/
unative_t SyscallHandler::handleVMAFree()
{
	//the same as vma_free in api.h
	IVirtualMemoryMap* vmm = IVirtualMemoryMap::getCurrent().data();
	ASSERT (vmm);

	return vmm->free((void *)m_params[0]);
}

//------------------------------------------------------------------------------
unative_t SyscallHandler::handleGetTime()
{
	*((Time*)(m_params[0])) = Time::getCurrentTime();

	return EOK;
}

