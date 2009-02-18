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
 * @brief Class Process declaration.
 *
 * Long description. I would paste some Loren Ipsum rubbish here, but I'm afraid
 * It would stay that way. Not that this comment is by any means ingenious but 
 * at least people can understand it. 
 */

#pragma once

#include "types.h"
#include "structures/List.h"
#include "structures/IdMap.h"

class  Thread;
class  UserThread;
class  Event;
class  Process;
class  Time;
struct ProcessInfo;

template class List<UserThread*>;
template class IdMap<event_t, Event*>;
template class IdMap<process_t, Process*>;

typedef List<UserThread*>          UserThreadList;
typedef IdMap<event_t, Event*>     EventTable;

/*!
 * @class Process Process.h "proc/Process.h"
 * @brief Class represents kernel side of the running process.
 *
 * Class Process stores per process kernel resources like
 * running threads, used kernel side synchronization primitives...
 * On exit all these resources are correctly disposed of.
 * Process contains one main thread (thread that runs main()), if this
 * thread finishes or is killed the Process ends.
 */
class Process
{
public:

	/*!
	 * @brief Creates new process from the process image.
	 * @param image Program to run.
	 * @param size Size of the prorgam image.
	 * @return Ptr to the newly created process on success, NULL on failure.
	 */
	static Process* create( const void* image, size_t size );

	/*!
	 * @brief Gets pointer to the Process of the currrently running thread.
	 * @return Ptr to the Process, NULL if the current thread does not belogn to 
	 * 	any process.
	 */
	static Process* getCurrent();
	
	/*!
	 * @brief Gets pointer to the main Thread of the Process.
	 * @return Ptr to the main thread.
	 */
	inline UserThread* mainThread() { return m_mainThread; };

	/*!
	 * @brief Starts new thread within the process.
	 * @param thread_ptr Place to store the assigned thread ID.
	 * @param thread_start Function to run in the new thread.
	 * @param data The first argument of the function.
	 * @param arg The second argument of the function.
	 * @param flags Flags to pass to the thread construction.
	 * @return Ptr to the newly added Thread on success, NULL otherwise.
	 */
	UserThread* addThread( thread_t* thread_ptr, void* (*thread_start)(void*),
	    void* data = NULL, void* arg = NULL, const unsigned int flags = 0 );

	/*!
	 * @brief Translates thread id.
	 * @param thread ID of the thread.
	 * @return Ptr to the thread using id @a thread if this thread exists
	 * 	and belongs to this process, NULL otherwise.
	 */
	Thread* getThread( thread_t thread );

	/*!
	 * @brief Removes thread from the process.
	 * @return @a True if thread belonged to thsi process and was sucessfully
	 * 	removed, @a false otherwise.
	 */
	bool removeThread( UserThread* );

	/*!
	 * @brief Frees resources used by the process.
	 *
	 * Kills all still running threads, frees all used events.
	 * @note As there is no return value passed this function is used in 
	 * 	killing the process as well.
	 */
	void exit();

	/*!
	 * @brief Gets assigned ID.
	 * @return Assigned ID.
	 */
	inline process_t id() { return m_id; }

	/*!
	 * @brief Suspends calling thread until the process is finished or time limit
	 * 	ends.
	 * @param time Time limit to use(use NULL to get non-timed version).
	 * @retval ETIMEDOUT If the the process did not finish in the given time.
   * @retval EOK    If the thread was suspended and successfully awaken on 
   * 	exit
	 */
	int join( const Time * time );

	/*!
	 * @brief Actualizes exported information. (Thread ID of the current thread).
	 */
	void setActiveThread( thread_t );

	/*! @brief Events used by this process. */
	EventTable eventTable;

private:
	
	UserThread*    m_mainThread;   /*!< The main thread of the process.   */
	UserThreadList m_list;         /*!< List of all other threads.        */

	process_t     m_id;            /*!< Assigned id.                      */
	ProcessInfo * m_info;          /*!< Position of exported information. */

	/*! @brief Nothing here. */
	inline Process(){};

	/*! @brief Destroys all used Event instances. */
	void clearEvents();
	
	/*! @brief Ends threads by either killing or deactivating them. */
	void clearThreads();

	/*! No copies.    */
	Process( const Process& );
	/*! No assigning. */
	Process& operator = ( const Process& );
};

#define PROCESS Process::getCurrent()
#define CURRENT_EVENT_TABLE Process::getCurrent()->eventTable
