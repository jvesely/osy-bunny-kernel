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

/*----------------------------------------------------------------------------*/
/*uint Timer::add(const Event& event)
{
	assert(m_heap.isAvailable());
	return 0;
}*/
/*----------------------------------------------------------------------------*/
/*uint Timer::remove(Thread* thread)
{
	return 0;
}*/
/*----------------------------------------------------------------------------*/
void Timer::plan(Thread* thread, const Time& time)
{
	ASSERT(thread);
	Time planned = Time::getCurrent() + time;
//	dprintf("Inserting into heap %d(%p).\n", thread->id(), thread);
	thread->insertIntoHeap(&m_heap, planned);
//	dprintf("Inserted into heap %d(%d) to run on %u and now is %u.\n",
	//	thread->id(), m_heap.size(), planned.secs(), Time::getCurrent().secs());
	Scheduler::instance().dequeue(thread);
	Kernel::instance().setTimeInterrupt(planned);
}
/*----------------------------------------------------------------------------*/
void Timer::interupt()
{
//	dprintf("Handling interrupt....%d\n", m_heap.size());
	bool nextThread = false;
	Time now = Time::getCurrent();
	Thread * thr = NULL;
	while ( (thr = static_cast<Thread*>(m_heap.topItem())) && (thr->key() < now) )
	{
		thr->removeFromHeap();
//		dprintf("On the TOP of heap was: %p %u\n", thr, thr->id());
//		dprintf("Planned event was %u,%u and now is %u,%u\n",
			//thr->key().secs(), thr->key().usecs(), now.secs(), now.usecs());
		if ( thr->status() == Thread::RUNNING ) {
			dprintf("Thread was RUNNING.\n");
			nextThread = true;
		} else {
//			dprintf("Thread status was: %d\n", thr->status());
			ASSERT(thr->status() != Thread::READY);
			Scheduler::instance().enqueue(thr);
		}
	}
	thr = static_cast<Thread*>(m_heap.topItem());
	if (thr)
		now = thr->key();
	else
		now = Time(0,0);
//	dprintf("Nearest event: %p, time: %x\n", m_heap.topItem(), static_cast<Thread*>(m_heap.topItem())->key().secs() );

	Kernel::instance().setTimeInterrupt(now);
	if (nextThread)
		Scheduler::instance().switchThread();
}
