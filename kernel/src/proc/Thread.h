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
 * @brief Shared Thread class.
 *
 * Class is used as a generic thread, other thread-like class will hopefuly 
 * be able to inherit it.
 */
#pragma once

#include "structures/ListInsertable.h"
#include "structures/HeapInsertable.h"
#include "Time.h"
#include "mem/IVirtualMemoryMap.h"
#include "Pointer.h"

template class Pointer<IVirtualMemoryMap>;
class Process;

#define THREAD_HEAP_CHILDREN 4

/*!
 * @class Thread Thread.h "proc/Thread.h"
 * @brief Thread class.
 *
 * Abstract class that handles basic thread abilities. Member function to be
 * run in the separate thread is pure virtual, thus to use thread is is 
 * necessary to inherit this class and reimplment this member function.
 */
class Thread: public ListInsertable<Thread>,
              public HeapInsertable<Thread, Time, THREAD_HEAP_CHILDREN>
{

public:
	static const int DEFAULT_STACK_SIZE = 0x2000; /*!< 8KB */

	/*!
	 * @brief Gets currently running thread.
	 *
	 * @return Pointer to the Thread that si currently beeing executed,
	 * 	@a NULL if no such Threadd exists.
	 * Wrapper for Scheduler member function.
	 */
	static Thread* getCurrent();

	/*!
	 * @brief Selects the thread that is next to run.
	 *
	 * @return Pointer to the Thread that is next to be run.
	 * Wrapper for Scheduler member function. Repeated calls of this function
	 * will give different results even wihtout rescheduling.
	 */
	static Thread* getNext();

	/*! 
	 * @brief Converts thread_t to Thread*.
	 *
	 * @param id to convert.
	 * @return Pointer to the thread associated with given id, if such exists
	 * 	NULL if no such Thread exists.
	 * Wrapper for Scheduler memeber function.
	 */
	static Thread* fromId( thread_t );

	static bool shouldSwitch();

	static void requestSwitch();

	/*! @brief Contructs Thread usinng the given parameters.
	 *
	 * Unless paramters are given contructs the thread using defaults.
	 * Construction involves allocating stack and preparing context.
	 * If successfull thread's status will be INITIALIZED, if something went 
	 * wrong it will be UNINITIALIZED.
	 * @param stackSize size of requested stack
	 */
	Thread(	uint stackSize = DEFAULT_STACK_SIZE );

	/*! @enum Status
	 * @brief Possible states of threads
	 */
	enum Status {
		UNINITIALIZED, INITIALIZED, READY, RUNNING, KILLED, WAITING, BLOCKED, FINISHED, JOINING
	};
	
	/*! @brief If stack was sucessfully allocated, it is freed here */
	virtual ~Thread();

	/*! @brief This will be run in the separate thread, includes some management */
	virtual void run() __attribute__ ((noreturn)) = 0;

	/*! Suspend thread for the given time, no status is set */
	void alarm( const Time& alarm_time );

	/*! @brief New thread entry point. */
	void start() { run(); };

	/*! @brief Process I belong to. */
	Process* process() { return m_process; };

	/*! @brief Rescheduling member function.
	 *
	 * Saves context of the currentThread() on its stack 
	 * and loads new context from the Threads stack.
	 */
	void switchTo();

	/*! @brief Stops execution of the current thread and switches to the next. */
	void yield();

	/*! @brief Puts Thread back into the running queue */
	void wakeup();

	/*! @brief Takes Thread of the running queue */
	void suspend();

	/*! @brief Takes the thread off the queue, and if it was detached 
	 * deletes it on the spot.
	 */
	virtual bool kill();

	void exit( void* retval );

	/*! @brief Detached getter
	 * @return detached state
	 */
	inline bool detached() const { return m_detached; }

	/*! @brief Sets state to detached
	 * @return new detached state (true)
	 */
	bool detach();
	
	/*! @brief Surrenders processing time for given time.
	 * @param interval Time interval to sleep
	 */
	void sleep(const Time& interval);

	/*! @brief Conversion to thread_t type.
	 * @return thread_t identifier of this thread
	 */
	inline thread_t id() { return m_id; };

	/*! @brief Attempts to wait until the given thread ends.
	 *
	 * Unless the given thread is non existent detached or already beeing 
	 * joined by another thread
	 * @retval EINVAL if the thread non-existent, detached or already being joined
	 * @retval EKILLED if the thread was killed
	 * @retval FINISHED if the thread has already finished it's execution
	 * @retval EOK this thread was suspended and successfully awoken on 
	 * others ending
	 */
	int join( Thread* other, void ** retval, bool timed = false, const Time& wait_time = Time() );

	/*! @brief Removes thread from the scheduling queue */
	void block();
	
	/*! @brief Resumes thread to the scheduling queue */
	void resume();

	/*! @brief Gets current thread status.
	 * @return current status
	 */
	inline Status status() const { return m_status; };

	/*! @brief Sets current thread status.
	 *
	 * Used during thread switching, mutex locking, waiting,....
	 * @param status new status
	 */
	inline void setStatus( Status status ) { m_status = status; };
	
	inline Pointer<IVirtualMemoryMap> getVMM() { return m_virtualMap; }
	
	/*! @brief Sets my thread_t identifier. */
	thread_t registerWithScheduler();

protected:
	void*  m_stack;                            /*!< that's my stack            */
	void*  m_stackTop;                         /*!< top of my stack            */
	void*  m_otherStackTop;                    /*!< may switch to this stack   */
	void*  m_ret;                              /*!< return value               */
	size_t m_stackSize;                        /*!< size of my stack           */
	Process*  m_process;                       /*!< my process                 */

	bool m_detached;                           /*!< detached flag              */
	Status m_status;                           /*!< my status                  */
	thread_t m_id;	                           /*!< my id                      */
	Thread* m_follower;                        /*!< someone waiting for me     */
	Thread* m_joinTarget;                      /*!< I'm waiting for this       */
	Pointer<IVirtualMemoryMap> m_virtualMap;   /*!< @brief Virtual Memory Map. */

private:
	Thread( const Thread& other );              /*!< no copying   */
	Thread& operator = ( const Thread& other ); /*!< no assigning */

	bool deactivate();

	friend class Process;
};

template class ListInsertable<Thread>; 
template class HeapInsertable<Thread, Time, THREAD_HEAP_CHILDREN>;
