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
	 * requires posponing them
	 */
	InterruptDisabler inter;

	/* It is higly probable that this thread was already planned so 
	 * we'd better remove it first.
	 */
	thread->removeFromHeap();

	if (thread->status() != Thread::RUNNING) {
		PRINT_DEBUG ("Planning thread wakeup for thread %u.\n", thread->id());
		Scheduler::instance().dequeue(thread);
	}

	/* Convert relative time to abslute and insert into time heap. */
	Time planned = (Time::getCurrent() + time);
	PRINT_DEBUG ("Planning thread %u for the time: %u,%u.\n",
		thread->id(), planned.secs(), planned.usecs());
	thread->insertIntoHeap(&m_heap, planned);

	/* if the newest event is sooner than the former it is needed to replan interupts */
	if ( thread == static_cast<Thread*>(m_heap.topItem()) ) {
		PRINT_DEBUG ("Replanning interupt.\n");
		Kernel::instance().setTimeInterrupt(thread->key());
	}
}
/*----------------------------------------------------------------------------*/
void Timer::interupt()
{
	

	/* We need to know about replanning as we need to do it as the last thing */
	bool nextThread = false;

	Time now = Time::getCurrent();
	PRINT_DEBUG ("Handling interupt in time %u, %u.\n", now.secs(), now.usecs());
	
	Thread * thr = NULL;

	/* While there are events that are due execute them */
	while ( (thr = static_cast<Thread*>(m_heap.topItem())) && (thr->key() < now) )
	{
		/* removing the top */
		thr->removeFromHeap();

		if ( thr->status() == Thread::RUNNING ) {
			/* Current thread might have only requested recheduling */
			PRINT_DEBUG ("Timer to replan.\n");
			nextThread = true;
		} else {
			/* Other threadd might have only requested waking up */
			ASSERT (thr->status() != Thread::READY);
			PRINT_DEBUG ("Waking thread %u.\n");
			Scheduler::instance().enqueue(thr);
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
		Scheduler::instance().switchThread();
	}
}
