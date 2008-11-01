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

#include "Singleton.h"
#include "Thread.h"
#include "structures/List.h"
#include "structures/HashMap.h"

/*! @class Scheduler Scheduler.h "proc/Scheduler.h"
 * @brief Thread handling class.
 *
 * Schedules threads and thread queue of active threads, can suspend, wakeup
 * switch kill and join
 */
class Scheduler: public Singleton<Scheduler>
{
public:
	/*! @brief Just sets current thread to NULL, creates Idle thread */
	Scheduler();

	/*! @brief Converts identifier to pointer 
	 * @param thread id to be converted
	 * @return pointer to Thread class, NULL on failure
	 */
	Thread* thread(thread_t thread)
		{ return m_threadMap.exists(thread)?m_threadMap.at(thread):NULL; };
	
	/*! @brief Adds new thread, generates id for it and makes it READY
	 * @param newThread thread to be added
	 * @result id of the new thread
	 */
	thread_t getId(Thread* newThread);

	inline void returnId(thread_t id)
		{ m_threadMap.erase(id); };

	/*! @brief Removes thread from scheduling queue (ONLY).
	 *
	 * To actually suspend you need to call yeild after this.
	 */
	void dequeue(Thread * thread);
	
	/*! Enqueue Thread* to the scheduling queue */
	void enqueue(Thread * thread);

	
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

	inline unsigned int addThread() { return ++m_threadCount; };

	inline unsigned int removeThread() { return --m_threadCount; }; 

private:
	/*! Planning queue */
	List<Thread*> m_activeThreadList;

	/*! Conversion table thread_t -> Thread* */
	HashMap<thread_t, Thread*> m_threadMap;

	/*! Currently running thread */
	Thread* m_currentThread;

	/*! Thread id generating helper. Increases avery time thread is added. */
	thread_t m_nextThread;

	unsigned int m_threadCount;

	Thread* m_idle;

	static const int DEFAULT_QUATNUM = 2000; /*!< 2 msec */
	
};

void* idleThread(void*);
