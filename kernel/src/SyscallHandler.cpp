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
#include "syscallcodes.h"

#include "handlers.inc"


SyscallHandler::SyscallHandler()
{
	m_handles[SYS_PUTS] = handlePuts;
	m_handles[SYS_GETS] = handleGets;
	m_handles[SYS_EXIT] = handleExit;

	m_handles[SYS_THREAD_CREATE]  = handleThreadCreate;
	m_handles[SYS_THREAD_SELF]    = handleThreadSelf;
	m_handles[SYS_THREAD_JOIN]    = handleThreadJoin;
	m_handles[SYS_THREAD_DETACH]  = handleThreadDetach;
	m_handles[SYS_THREAD_CANCEL]  = handleThreadCancel;
	m_handles[SYS_THREAD_SLEEP]   = handleThreadSleep;
	m_handles[SYS_THREAD_YIELD]   = handleThreadYield;
	m_handles[SYS_THREAD_SUSPEND] = handleThreadSuspend;
	m_handles[SYS_THREAD_WAKEUP]  = handleThreadWakeup;
	m_handles[SYS_THREAD_EXIT]    = handleThreadExit;

	m_handles[SYS_EVENT_INIT] = handleEventInit;
	m_handles[SYS_EVENT_WAIT] = handleEventWait;
	m_handles[SYS_EVENT_WAIT_TIMEOUT] = handleEventWaitTimeout;
	m_handles[SYS_EVENT_FIRE]    = handleEventFire;
	m_handles[SYS_EVENT_DESTROY] = handleEventDestroy;
	
	m_handles[SYS_VMA_ALLOC] = handleVMAAlloc;
	m_handles[SYS_VMA_FREE]  = handleVMAFree;
}
/*----------------------------------------------------------------------------*/
bool SyscallHandler::handleException( Processor::Context* registers )
{
	uint syscall = ((*(unative_t*)registers->epc)>>6);

	unative_t params[4];

	params[0] = registers->a0;
	params[1] = registers->a1;
	params[2] = registers->a2;
	params[3] = registers->a3;
/*
	printf( "Handling syscall: %x, with params %x,%x,%x,%x.\n",
		m_call, m_params[0], m_params[1], m_params[2], m_params[3]);
// */

	if (!m_handles[syscall]) return false;

	registers->v0   = this->m_handles[syscall]( params );
	registers->epc += 4;
	
	return true;
}
