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
/*
	switch ( m_call ) {
		case SYS_PUTS: //Syscalls::SC_PUTS:
			registers->v0 = handlePuts();
			break;
		case SYS_GETS: //Syscalls::SC_GETS:
			registers->v0 = handleGets();
			break;
		default:
			puts("Unknown SYSCALL.\n");
			return false;
	}
	registers->epc += 4;
	return true;
*/
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
		(thread_t*)m_params[0], (void*(*)(void*))m_params[1], (void*)m_params[2] );
	PRINT_DEBUG ("Thread create handled : %s.\n", success ? "OK" : "FAIL");
	return success ? EOK : ENOMEM;
}
/*----------------------------------------------------------------------------*/
unative_t SyscallHandler::handleThreadJoin()
{
	Thread* thr = PROCESS_THREAD();
	return Thread::getCurrent()->join( thr, m_params[2], *(Time*)m_params[2] );
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
	return 0;
}
/*----------------------------------------------------------------------------*/
unative_t SyscallHandler::handleThreadSleep()
{
	Time sleep_time(((Time*)m_params[0])->secs(), ((Time*)m_params[0])->usecs());
	PRINT_DEBUG ("Sleep: %u:%u.\n",
		((Time*)m_params[0])->secs(), ((Time*)m_params[0])->usecs());
	Thread::getCurrent()->sleep( sleep_time );
	return 0;
}
