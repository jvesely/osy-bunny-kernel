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
 *   @version $Id: header.tmpl 41 2008-10-26 18:00:14Z vesely $
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

#include "Timer.h"
#include "Kernel.h"
#include "InterruptDisabler.h"

//#define TIMER_DEBUG

#ifndef TIMER_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...) \
  printf("[ TIMER_DEBUG ]: "); \
  printf(ARGS);
#endif

/*----------------------------------------------------------------------------*/
void Timer::plan(Thread* thread, const Time& time)
{
	ASSERT (thread);
	
	/* Mangling with shared structure that can be used during interupts
	 * requires posptoning them
	 */
	InterruptDisabler inter;

	/* It is higly probable that this thread was already planned so 
	 * we'd better remove it first.
	 */
	//thread->removeFromHeap();

	/* Convert relative time to abslute and insert into time heap. */
	Time now = Time::getCurrent();
	Time planned = (now + time);
	
	PRINT_DEBUG ("----------PLANNING Time: %u:%u ----------------\n", now.secs(), now.usecs());
	PRINT_DEBUG ("Pending events: %u and planning thread %p for the time: %u,%u.\n",
		m_heap.size(), thread->id(), planned.secs(), planned.usecs());

	thread->insertIntoHeap(&m_heap, planned);

	/* if the newest event is sooner than the former it is needed to replan interupts */
	if ( thread == static_cast<Thread*>(m_heap.topItem()) ) {
		PRINT_DEBUG ("Replanning interupt to time: %u:%u.\n", thread->key().secs(), thread->key().usecs());
		Kernel::instance().setTimeInterrupt(thread->key());
	}

	PRINT_DEBUG ("Pending events: %u.\n", m_heap.size());
	PRINT_DEBUG ("------------------PLANNING END--------------\n");
}
/*----------------------------------------------------------------------------*/
void Timer::interupt()
{

	InterruptDisabler interrupts;

	/* We need to know about replanning as we need to do it as the last thing */
	bool nextThread = false;

	Time now = Time::getCurrent();

	PRINT_DEBUG ("===============INTERRUPT START==============\n");
	PRINT_DEBUG ("Handling interupt in time %u, %u, pending events: %u.\n", 
		now.secs(), now.usecs(), m_heap.size());
	
	Thread * thr = NULL;

	/* While there are events that are due execute them */
	while ( (thr = static_cast<Thread*>(m_heap.topItem())) && (thr->key() < now) )
	{
				
		PRINT_DEBUG ("Removing thread %u from the heap.\n", thr->id());

		if ( thr->status() == Thread::RUNNING ) {
			/* Current thread might have only requested recheduling */
			PRINT_DEBUG ("Timer to replan thread %u.\n", thr->id());
			nextThread = true;
			thr->removeFromHeap();
		} else {
			/* Other thread might have only requested waking up */
			ASSERT (thr->status() != Thread::READY);
			PRINT_DEBUG ("Waking thread %u.\n", thr->id());
			thr->resume();
		}
	}

	/* Get the next event */
	thr = static_cast<Thread*>(m_heap.topItem());

	Time nextEvent = thr ? thr->key() : Time();

	PRINT_DEBUG ("Next event in %u,%u.\n", nextEvent.secs(), nextEvent.usecs());
	Kernel::instance().setTimeInterrupt(nextEvent);

	/* Rescheduling was requested */
	if (nextThread) {
		PRINT_DEBUG ("Thread switching was due.\n");
		Thread::getCurrent()->yield();
	}
	PRINT_DEBUG ("=====================INTERRUPT END==================\n");
}
