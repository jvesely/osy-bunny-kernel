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
 * @brief Short description.
 *
 * Long description. I would paste some Loren Ipsum rubbish here, but I'm afraid
 * It would stay that way. Not that this comment is by any means ingenious but 
 * at least people can understand it. 
 */
#pragma once
#include "Thread.h"
#include "structures/List.h"
#include "structures/HashMap.h"

class Scheduler
{
public:
	Scheduler():m_currentThread(NULL){};
	Thread* thread(thread_t thread){ return NULL; };
	thread_t addThread(Thread* newThread);
	int killThread(thread_t thread){ return EOK; };
	int wakeup(thread_t thread);
	void suspend();
	inline Thread* activeThread()
		{ return m_currentThread; };
	void switchThread();

private:
	List<Thread*> m_activeThreadList;
	HashMap<thread_t, Thread*> m_threadMap;
	Thread* m_currentThread;
	thread_t m_nextThread;

	void schedule(Thread * thread);
};
