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
#include "KernelThread.h"
#include "IdleThread.h"
#include "structures/List.h"
#include "structures/HashMap.h"

/*! @class Scheduler Scheduler.h "proc/Scheduler.h"
 * @brief Stores and handles active threads.
 *
 * Stores active thread and decides who is next to run.
 */

typedef HashMap<thread_t, Thread*> ThreadMap; 

class Scheduler: public Singleton<Scheduler>
{
//	static const Time DEFAULT_QUANTUM;

	/*! @brief Converts identifier to pointer 
	 * @param thread id to be converted
	 * @return pointer to Thread class, NULL on failure
	 */
	Thread* thread( thread_t thread )
		{ return m_threadMap.exists( thread ) ? m_threadMap.at( thread ) : NULL; };

	/*! @brief Adds new thread, generates id for it and makes it READY
	 * @param newThread thread to be added
	 * @result id of the new thread
	 */
	thread_t getId( Thread* newThread );

	/*! @brief Maked thread id available for use again.
	 * @param id thread_t id to free
	 */
	inline void returnId( thread_t id )
		{ m_threadMap.erase( id ); };

	/*! @brief Removes thread from scheduling queue (ONLY). */
	void dequeue( Thread* thread );
	
	/*! @brief Enqueues Thread* to the scheduling queue.
	 * @note If IdleThread was running context is switched to the enqueued thread.
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
	ThreadMap m_threadMap;

	/*! Currently running thread */
	Thread* m_currentThread;

	/*! Thread id generating helper. Increases avery time thread is added. */
	thread_t m_nextThread;

	/*! Number of active threads */
	uint m_threadCount;

	/*! @brief Thread that runs when no one else will. */
	IdleThread* m_idle;
	
	/*! @brief Just sets current thread to NULL, creates Idle thread */
	Scheduler();
	
	friend class Singleton<Scheduler>;
	friend class Thread;
	friend Thread* KernelThread::create( thread_t* thread_ptr, void* (*thread_start)(void*), void* thread_data, const unsigned int thread_flags );

};

