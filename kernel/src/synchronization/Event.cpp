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
 * @brief Class Event declaration - the simplest synchronization primitive.
 */

#include "api.h"
#include "Event.h"
#include "InterruptDisabler.h"
#include "proc/Thread.h"

//#define EVENT_DEBUG

#ifndef EVENT_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...) \
	printf("[ EVENT_DEBUG ]: "); \
	printf(ARGS);
#endif

/*----------------------------------------------------------------------------*/

Event::Event()
{
	PRINT_DEBUG ("Event created at address %x.\n", this);
}

/*----------------------------------------------------------------------------*/

Event::~Event()
{
	ASSERT(m_list.empty());
}

/*----------------------------------------------------------------------------*/

void Event::wait()
{
	InterruptDisabler interrupts;

	Thread* thr = Thread::getCurrent();
	thr->block();
	thr->append(&m_list);

	thr->yield();
}

/*----------------------------------------------------------------------------*/

void Event::waitTimeout( const Time& timeout )
{
	InterruptDisabler interrupts;
	PRINT_DEBUG("Event::waitTimeout() started...Waiting for %u usecs\n", 
		timeout.toUsecs());

	Thread* thr = Thread::getCurrent();
	thr->alarm(timeout);
	thr->append(&m_list);

	thr->yield();
	PRINT_DEBUG("Event::waitTimeout() finished.\n");
}

/*----------------------------------------------------------------------------*/

void Event::fire()
{
	InterruptDisabler interrupts;
	PRINT_DEBUG("Event::fire() started, unblocking all threads...(%u)\n",
		m_list.size());

	while (!m_list.empty()) {
		Thread* thr = m_list.getFront();
		PRINT_DEBUG ("Resuming thread: %u.\n", thr->id());
		thr->resume();
	}
}
