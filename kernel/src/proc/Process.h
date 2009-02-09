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

#include "types.h"
#include "structures/List.h"
#include "structures/IdMap.h"

class Thread;
class UserThread;
class Event;
class Process;

template class List<UserThread*>;
template class IdMap<event_t, Event*>;
template class IdMap<process_t, Process*>;

typedef List<UserThread*>          UserThreadList;
typedef IdMap<event_t, Event*>     EventTable;
typedef IdMap<process_t, Process*> ProcessTable;

class Process
{
public:

	static Process* create( const char* image, size_t size );
	inline UserThread* mainThread() { return m_mainThread; };
	UserThread* addThread( thread_t* thread_ptr, void* (*thread_start)(void*),
	    void* data = NULL, void* arg = NULL, const unsigned int flags = 0 );
	Thread* getThread( thread_t thread );
	bool removeThread( UserThread* );
	void exit();
	inline process_t id() { return m_id; }

	EventTable eventTable;
	

	static Process* getCurrent();

private:
	UserThread* m_mainThread;
	UserThreadList m_list;

	process_t m_id;

	Process(){};
	void clearEvents();
	void clearThreads();

	Process( const Process& );
	Process& operator = ( const Process& );
};

#define PROCESS Process::getCurrent()
#define CURRENT_EVENT_TABLE Process::getCurrent()->eventTable
