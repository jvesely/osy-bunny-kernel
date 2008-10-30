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
 * Class is used as kernel thread, other thread-like stuff will hopefully 
 * be able to inherit it.
 */
#pragma once

#include "api.h"
/*!
 * @class Thread Thread.h "proc/Thread.h"
 * @brief Thread class.
 *
 * Thread class handles stack and routine that is to be executed
 * in the separate threadd.
 */
class Thread
{

public:
	static const int DEFAULT_STACK_SIZE = 0x1000; /*!< 4KB */
	
	/*! @enum Status
	 * @brief Possible states of threads
	 */
	enum Status {
		READY, RUNNING, KILLED, WAITING, BLOCKED, FINISHED
	};
	
	static int create(thread_t* thread_ptr, void* (*thread_start)(void*),
	  void* data = NULL, const unsigned int flags = 0);
	/*! this method will be run in separate thread, includes some management */
	virtual void run();
	
	/*! @brief initial setup that could not be done in the constructor.
	 *
	 * Allocates stack, stores initial context and sets some reg values
	 * @return ENOMEM if stack allocation fails, otherwise EOK
	 */
	//inline bool isOK() { return m_stack != NULL; };
	
	/*! @brief new thread entry point */
	void start() { run(); };

	/*! @brief Wrapper to Scheduler yield, surrenders processing time. */
	void yield();

	void wakeup();

	void suspend();

	void kill();

	/*! @brief Detached getter
	 * @return detached state
	 */
	inline bool detached() const { return m_detached; }

	/*! @brief Sets state to detached
	 * @return new detached state (true)
	 */
	inline bool detach() { return m_detached = true; }
	
	/*! @brief Surrenderrs processing time for given time
	 * @param sec number of seconds to sleep
	 */
	void sleep(const unsigned int sec);

	/*! @brief Microsec brother of sleep()
	 * @param usec number of microseconds to sleep
	 */
	void usleep(const unsigned int usec);

	/*! @brief Conversion to thread_t type.
	 * @return thread_t identifier of this thread
	 */
	inline thread_t id() { return m_id; };

	int join(Thread* other);

	/*! @brief Sets my thread_t identifier */
	inline void setId(thread_t id) { m_id = id; };

	/*! @brief Pointer to the pointer to the stacktop
	 * @return pointer that store stacktop pointer address
	 */
	inline void** stackTop() { return &m_stackTop; };

	/*! @brief Follower is the Thread waiting for me.
	 * @return pointer to the thread waiting for my end
	 */
	inline Thread* follower() const { return m_follower; };

	/*! @brief Sets follower.
	 * @param follower new follower
	 */
	inline void setFollower(Thread* follower) { m_follower = follower; };

	/*! @brief Gets current thread status.
	 * @return current status
	 */
	inline Status status() const { return m_status; };

	/*! @brief Sets current thread status.
	 *
	 * Used during thread switching, mutex locking, waiting,....
	 * @param status new status
	 */
	inline void setStatus(Status status) { m_status = status; };

protected:
	void* m_stack;	          /*!< that's my stack */
	void* m_stackTop;	        /*!< top of my stack */
	unsigned int m_stackSize; /*!< size of my stack */

	void* (*m_runFunc)(void*); /*! I'm supposed to run this */
	void* m_runData; /*!< runFunc expects this */


	bool m_detached;	/*!< detached flag */
	Status m_status;	/*!< my status */
	thread_t m_id;	/*!< my id */
	Thread* m_follower;	/*!< someone waiting */

private:
	Thread(); /*!< no constructing without params */
	Thread(const Thread& other); /*!< no copying */
	const Thread& operator=(const Thread& other);	/*!< no assigning */

	/*! @brief Creates thread.
	 * @param func function to be executed in the separate thread
	 * @param data the only parameter to handled to the function "func"
	 * @param stackSize size of stack that will be available to this thread
	 * @param flags ignored param :)
	 */
	Thread(	void* (*func)(void*), void* data, unative_t flags, unsigned int stackSize);
	friend class Scheduler;
};
