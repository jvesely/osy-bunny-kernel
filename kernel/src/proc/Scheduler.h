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
 * @brief Scheduler header.
 *
 * Class Scheduler declaration. 
 */
#pragma once

#include "Thread.h"
#include "structures/List.h"
#include "structures/HashMap.h"

/*! @class Scheduler Scheduler.h "proc/Scheduler.h"
 * @brief Thread handling class.
 *
 * Schedules threads and thread queue of active threads, can suspend, wakeup
 * switch kill and join
 */
class Scheduler
{
public:
	/*! @brief Just sets current thread to NULL, later may create Idle thread */
	Scheduler():m_currentThread(NULL){};

	/*! @brief Converts identifier to pointer 
	 * @param thread id to be converted
	 * @return pointer to Thread class, NULL on failure
	 */
	Thread* thread(thread_t thread)
		{ return m_threadMap.exists(thread)?m_threadMap.at(threadd):NULL; };
	
	/*! @brief Adds new thread, generates id for it and makes it READY
	 * @param newThread thread to be added
	 * @result id of the new thread
	 */
	thread_t addThread(Thread* newThread);

	/*! @brief Kills thread.
	 * @param thread thread to kill
	 * @return EINVAL on invalid identifier, EOK on sucess
	 * @note not yet implemented :)
	 */
	int killThread(thread_t thread){ return EOK; };

	/*! @brief Waits fot thread to finish.
	 * @param thread thread id of the thread to wait for
	 * @return EINVAL on invalid thread, EKILLED on killed thread, EOK on sucess
	 */
	int joinThread(thread_t thread);

	/*! @brief Timed version of waiting, waits only specified amount of time.
	 * @param thread thread id of the thread to wait for
	 * @return same as not timed version plus ETIMEDOUT on timeout
	 */
	int joinThread(thread_t thread, unsigned int usec);

	/*! @brief Queues thread in scheduling queue
	 * @param thread thread to woken
	 * @return EINVAL on non-existing thread, EOK on success
	 */
	int wakeup(thread_t thread);
	
	/*! @brief Removes thread from scheduling queue (ONLY).
	 *
	 * To actually suspend you need to call yeild after this.
	 */
	void suspend();
	
	/*! @brief Gets pointer to current thread.
	 * @return Pointer to structure representing running thread.
	 */
	inline Thread* activeThread()
		{ return m_currentThread; };

	/*! @brief Rescheduling method.
	 *
	 * Saves context of the running thread on its stack and loads next thread
	 * in queue.
	 */
	void switchThread();

private:
	/*! Planning queue */
	List<Thread*> m_activeThreadList;

	/*! Conversion table thread_t -> Thread* */
	HashMap<thread_t, Thread*> m_threadMap;

	/*! Currently running thread */
	Thread* m_currentThread;

	/*! Thread id generating helper. Increases avery time thread is added. */
	thread_t m_nextThread;
	
	/*! Enqueue Thread* to the scheduling queue */
	void schedule(Thread * thread);
};
