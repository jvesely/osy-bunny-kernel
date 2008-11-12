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

#include "api.h"
#include "structures/ListInsertable.h"
#include "structures/HeapInsertable.h"
#include "timer/Time.h"
/*!
 * @class Thread Thread.h "proc/Thread.h"
 * @brief Thread class.
 *
 * Abstract class that handles basic thread abilities. Member function to be
 * run in the separate thread is pure virtual, thus to use thread is is 
 * necessary to inherit this class and reimplment this member function.
 */
class Thread: public ListInsertable<Thread>,
              public HeapInsertable<Thread, Time, 4>
{

public:
	static const int DEFAULT_STACK_SIZE = 0x1000; /*!< 4KB */

	static Thread* getCurrent();
	/*! @brief Contructs Thread usinng the given parameters.
	 *
	 * Unless paramters are given contructs the thread using defaults.
	 * Construction involves allocating stack and preparing context.
	 * If successfull thread's status will be INITIALIZED, if someting went 
	 * wrong it will be UNITIALIZED.
	 * @param flags Thread flags -- ingored
	 * @param stackSize size of requested stack
	 */
	Thread(	unative_t flags = 0, uint stackSize = DEFAULT_STACK_SIZE);

	/*! @enum Status
	 * @brief Possible states of threads
	 */
	enum Status {
		UNINITIALIZED, INITIALIZED, READY, RUNNING, KILLED, WAITING, BLOCKED, FINISHED, JOINING
	};
	
	/*! @brief If stack was sucessfully allocated, it is freed here */
	virtual ~Thread();

	/*! @brief This will be run in the separate thread, includes some management */
	virtual void run() = 0;

	/*! Suspend thread for the given time, no status is set */
	void alarm( const Time& alarm_time );

	/*! @brief new thread entry point */
	void start() { run(); };

	/*! @brief Wrapper to Scheduler yield, surrenders processing time. */
	void yield();

	/*! @brief Puts Thread back into the running queue */
	void wakeup() const;

	/*! @brief Takes Thread of the running queue */
	void suspend();

	/*! @brief Takes the thread off the queue, and if it was detached 
	 * deletes it on the spot.
	 */
	void kill();

	/*! @brief Detached getter
	 * @return detached state
	 */
	inline bool detached() const { return m_detached; }

	/*! @brief Sets state to detached
	 * @return new detached state (true)
	 */
	inline bool detach() { return m_detached = true; }
	
	/*! @brief Surrenders processing time for given time
	 * @param sec number of seconds to sleep
	 */
	void sleep(const uint sec);

	/*! @brief Microsec brother of sleep()
	 * @param usec number of microseconds to sleep
	 */
	void usleep(const uint usec);

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
	int join(Thread* other, bool timed = false);

	/*! @brief Timed version of join.
	 *
	 * If the thread other is still running after timeout.
	 * @return is same as join adding ETIMEDOUT, if the time is up and other
	 * thread is still running.
	 */
	int joinTimeout(Thread* other, const uint usec);

	/*! @brief Removes thread from the scheduling queue */
	void block();
	
	/*! @brief Resumes thread to the scheduling queue */
	void resume();

	/*! @brief Sets my thread_t identifier. */
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
	
	/*! @brief prepare stack and set initialized */
	Thread();

protected:
	void* m_stack;	          /*!< that's my stack */
	void* m_stackTop;	        /*!< top of my stack */
	unsigned int m_stackSize; /*!< size of my stack */



	bool m_detached;	/*!< detached flag */
	Status m_status;	/*!< my status */
	thread_t m_id;	/*!< my id */
	Thread* m_follower;	/*!< someone waiting */

private:
	Thread(const Thread& other); /*!< no copying */
	const Thread& operator=(const Thread& other);	/*!< no assigning */

};

template class ListInsertable<Thread>; 
template class HeapInsertable<Thread, Time, 4>;
