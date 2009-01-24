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
	if (!timeout)
		return;
	
	InterruptDisabler interrupts;

	Thread* thr = Thread::getCurrent();
	thr->alarm(timeout);
	thr->append(&m_list);

	thr->yield();
}

void Event::fire()
{
	InterruptDisabler interrupts;
	
	while (!m_list.empty())
		m_list.getFront()->resume();	

//	for (ThreadList::Iterator it = m_list.begin(); it != m_list.end(); ++it)
//		(*it)->resume();
}
