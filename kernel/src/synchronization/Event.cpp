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
 *   @version $Id: Event.cpp 605 2009-01-22 23:58:59Z slovak $
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

Event::~Event()
{
	ASSERT(m_list.empty());
}

void Event::wait() 
{
	InterruptDisabler interrupts;

	Thread* thr = Thread::getCurrent();
	thr->block();
	thr->append(&m_list);

	thr->yield();
}

void Event::waitTimeout( const Time& timeout )
{
	InterruptDisabler interrupts;
	PRINT_DEBUG("Event::waitTimeout() started...Waiting for %u usecs\n", timeout.toUsecs());
/*	
	if (timeout == Time(0, 0))
		return;
*/

	Thread* thr = Thread::getCurrent();
	thr->alarm(timeout);
	thr->append(&m_list);

	thr->yield();
	PRINT_DEBUG("Event::waitTimeout() finished.\n");
}

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
