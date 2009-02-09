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
#include "structures/List.h"
#include "structures/IdMap.h"

/*! @class Scheduler Scheduler.h "proc/Scheduler.h"
 * @brief Stores and handles active threads.
 *
 * Stores active thread and decides who is next to run.
 */

class Thread;

typedef IdMap<thread_t, Thread*> ThreadMap; 
typedef List<Thread*> ThreadList;


template class IdMap<thread_t, Thread*>;

class Scheduler: public Singleton<Scheduler>, private ThreadMap
{

	/*! @brief Translates identifier to pointer.
	 * @param thread identifier to translate.
	 * @retval Pointer to Thread class.
	 * @retval NULL if no thread with such an id exists.
	 */
	inline Thread* thread( thread_t thread )
		{ return translateId( thread ); }

	/*! @brief Removes thread from scheduling queue (ONLY). */
	void dequeue( Thread* thread );
	
	/*! @brief Enqueues Thread* to the scheduling queue.
	 * @note If IdleThread was running Thread switch is planned.
	 */
	void enqueue( Thread* thread );
	
	/*! @brief Gets pointer to current thread.
	 * @return Pointer to structure representing running thread.
	 */
	inline Thread* currentThread() const
		{ return m_currentThread; };

	/*! @brief Chooses the next thread to run.
	 * @return Pointer to the Thrad class holding the next thread.
	 */
	Thread* nextThread();

	/*! Planning queue */
	ThreadList m_activeThreadList;

	/*! Conversion table thread_t -> Thread* */
//	ThreadMap m_threadMap;

	/*! Currently running thread */
	Thread* m_currentThread;

	/*! Thread id generating helper. Increases avery time thread is added. */
	thread_t m_nextThreadId;

	/*! @brief Thread that runs when no one else will. */
	Thread* m_idle;
	
	/*! @brief Remember if switching is due. */
	bool m_shouldSwitch;

	/*! @brief Just sets current thread to &KERNEL, creates Idle thread */
	Scheduler();
	
	friend class Singleton<Scheduler>;
	friend class Thread;
};
#define SCHEDULER Scheduler::instance()
