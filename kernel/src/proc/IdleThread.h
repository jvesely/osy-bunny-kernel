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
#include "Thread.h"
/*!
 * @class Thread Thread.h "proc/Thread.h"
 * @brief Thread class.
 *
 * Thread class handles stack and routine that is to be executed
 * in the separate threadd.
 */
class IdleThread: public Thread
{

public:

	/*! this will be run in the separate thread, includes some management */
	void run() { asm volatile (" wait "); };

private:
	static const uint IDLE_STACK_SIZE = 1024;
	IdleThread():Thread(0, IDLE_STACK_SIZE) {}; 
//	KernelThread(const KernelThread& other); /*!< no copying */
//	const Thread& operator=(const KerrnelThread& other);	/*!< no assigning */

	/*! @brief Creates thread.
	 * @param func function to be executed in the separate thread
	 * @param data the only parameter to handled to the function "func"
	 * @param stackSize size of stack that will be available to this thread
	 * @param flags ignored param :)
	 */
	friend class Scheduler;
};
