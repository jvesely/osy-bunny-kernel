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
 * @brief Kernel Thread class.
 *
 * Class is used as kernel thread, inherits general thread handling 
 * abilities from class Thread.
 */
#pragma once

#include "api.h"
#include "Thread.h"
/*!
 * @class KernelThread KernelThread.h "proc/KernelThread.h"
 * @brief Thread class.
 *
 * Thread class handles stack and routine that is to be executed
 * in the separate threadd.
 */
class KernelThread: public Thread
{

public:
	/*! At the end it returns used id to the Scheduler */
	~KernelThread();

	static int create(thread_t* thread_ptr, void* (*thread_start)(void*),
	  void* data = NULL, const unsigned int flags = 0);
	
	/*! this will be run in the separate thread, includes some management */
	void run();

protected:
	void* (*m_runFunc)(void*); /*!< I'm supposed to run this */
	void* m_runData; /*!< runFunc expects this */



private:
	KernelThread(); /*!< no constructing without params */
	KernelThread(const KernelThread& other); /*!< no copying */
	const Thread& operator=(const KerrnelThread& other);	/*!< no assigning */

	/*! @brief Creates thread.
	 * @param func function to be executed in the separate thread
	 * @param data the only parameter to handled to the function "func"
	 * @param stackSize size of stack that will be available to this thread
	 * @param flags ignored param :)
	 */
	KernelThread(	void* (*func)(void*), void* data, unative_t flags, unsigned int stackSize);
	friend class Scheduler;
};
