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
 * @brief IdleThread class.
 *
 * Class is used as idle thread, inherits class Thread, that handles most 
 * of the thread stuff.
 */
#pragma once

#include "Thread.h"
/*!
 * @class IdleThread IdleThread.h "proc/IdleThread.h"
 * @brief IdleThread class.
 *
 * Special thread that is executed when no other thread wishes to run.
 */
class IdleThread: public Thread
{

public:

	/*! @brief This will be run in the separate thread, stops processor until next interrupt */
	void run() { asm volatile (" wait "); };

private:
	/*! @brief Nothing is done so smaller stack is enough. */
	static const uint IDLE_STACK_SIZE = 1024;

	/*! @brief Send just flags and changed stack size. */
	IdleThread():Thread(0, IDLE_STACK_SIZE) {}; 
	
	friend class Scheduler;
};
