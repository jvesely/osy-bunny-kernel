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

#define SYSCALL_HANDLER_DEBUG

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
/*
	if (m_call == SYS_GETS)
		printf("Handled syscall: %u, params: %x %x, res: %x",
			m_call, m_params[0], m_params[1], registers->v0 );
	// */
	return true;
}
/*----------------------------------------------------------------------------*/
unative_t SyscallHandler::handlePuts()
{
/*	printf ("Should output string at %p(%p):%s.\n",
		params[0], ADDR_TO_USEG(params[0]) ,params[0]); */
//	Processor::msim_stop();
//	if (ADDR_TO_USEG(params[0]) == params[0]) {
		return puts( (const char*)m_params[0] );
//	} else {
//		Thread::getCurrent()->kill();
//	}

}
/*----------------------------------------------------------------------------*/
unative_t SyscallHandler::handleGets()
{
	if (m_params[1] == 1) {
		*(char*)m_params[0] = getc();
		return 1;
	} else {
		return gets((char*)m_params[0], m_params[1]);
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
#define PROCESS_THREAD() \
	({ \
	Process* current = Process::getCurrent(); \
	ASSERT (current); \
	Thread* thread = current->getThread( m_params[0] ); \
	if (!thread) return EINVAL; \
	thread;\
	})

unative_t SyscallHandler::handleThreadCreate()
{
	Process* current = Process::getCurrent();
	ASSERT (current);
	bool success = current->addThread( 
		(thread_t*)m_params[0], (void*(*)(void*))m_params[1], (void*)m_params[2], (void*)m_params[3] );
	PRINT_DEBUG ("Thread create handled : %s.\n", success ? "OK" : "FAIL");
	return success ? EOK : ENOMEM;
}
/*----------------------------------------------------------------------------*/
unative_t SyscallHandler::handleThreadJoin()
{
	Thread* thr = PROCESS_THREAD();
	bool timed = (bool)m_params[2];
	PRINT_DEBUG ("Handling thread join on %u, %s, %p.\n", thr->id(),
		timed?"TIMED":"DIRECT", m_params[3]);
	return Thread::getCurrent()->join( thr, (void**)m_params[1], timed, *(Time*)m_params[3] );
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
	Time sleep_time(((Time*)m_params[0])->secs(), ((Time*)m_params[0])->usecs());
	PRINT_DEBUG ("Sleep:  %p %u:%u.\n", m_params[0],
		((Time*)m_params[0])->secs(), ((Time*)m_params[0])->usecs());
	Thread::getCurrent()->sleep( sleep_time );
	return 0;
}
/*----------------------------------------------------------------------------*/
unative_t SyscallHandler::handleEventInit()
{
	// is it a pointer in USEG?
	if (!ADDR_IN_USEG((uintptr_t)(m_params[0]))) {
		Thread::getCurrent()->kill();
		return 0;
	}

	Event* evnt = new Event;
	event_t ev = EventMap::instance().map(evnt);
	
	if (ev == EventMap::INVALID_ID)
		return ENOMEM;

	*((event_t*)(m_params[0])) = ev;

	return EOK;
}
/*----------------------------------------------------------------------------*/
unative_t SyscallHandler::handleEventWait()
{
	PRINT_DEBUG("SyscallHandler::handleEventWait() started\n");
	event_t evnt = m_params[0];
	
	if (!ADDR_IN_USEG((uintptr_t)(m_params[1]))) {
		Thread::getCurrent()->kill();
		return 0;
	}
	
	native_t* locked = (native_t*)(m_params[1]);

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

	// is it a pointer in USEG?
	if (!ADDR_IN_USEG((uintptr_t)(m_params[1]))) {
		Thread::getCurrent()->kill();
		return 0;
	}

	Time* time = (Time*)(m_params[1]);
	PRINT_DEBUG("SyscallHandler::handleEventWaitTimeout() time pointer: %p\n", time);
	Time alarm_time = Time::getCurrent() + *time;

	if (!ADDR_IN_USEG((uintptr_t)(m_params[2]))) {
		Thread::getCurrent()->kill();
		return 0;
	}

	native_t* locked = (native_t*)(m_params[2]);
	PRINT_DEBUG("SyscallHandler::handleEventWaitTimeout() locked pointer: %p\n", locked);
	
	Event* ev = EventMap::instance().getEvent(evnt);
	if (!ev) {
		Thread::getCurrent()->kill();
		return 0;
	}
/*
	if (!(*time))
		return ETIMEDOUT;
*/
	if (*locked)
		ev->waitTimeout(*time);

	Time current = Time::getCurrent();
	
	if (current >= alarm_time) {
		*time = Time();
		return ETIMEDOUT;
	}
	
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
	//the same as vma_alloc in api.h
	Thread* thread = Thread::getCurrent();

	ASSERT (thread);
	ASSERT (thread->getVMM());

	//round the size according to HW specifications
	(*((size_t*)m_params[1])) = roundUp(*((size_t*)m_params[1]),Processor::pages[0].size);

	//unative_t res = thread->getVMM()->allocate((void **)m_params[0], *((size_t*)m_params[1]),(unsigned int) m_params[2]);
	//return res;
	return thread->getVMM()->allocate((void **)m_params[0], *((size_t*)m_params[1]),(unsigned int) m_params[2]);
}
/*----------------------------------------------------------------------------*/
unative_t SyscallHandler::handleVMAFree()
{
	//the same as vma_free in api.h
	Thread* thread = Thread::getCurrent();
	
	ASSERT (thread);
	ASSERT (thread->getVMM());
	
	//unative_t res = thread->getVMM()->free((void *)m_params[0]);
	//return res;
	return thread->getVMM()->free((void *)m_params[0]);
}
