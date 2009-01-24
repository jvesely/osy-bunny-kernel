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

#include "proc/KernelThread.h"

/*!
 * @class UserThread UserThread.h "proc/UserThread.h"
 * @brief Thread class with stack in USEG segment.
 *
 * Runs given function in separate thread, stack is mapped through 
 * virtual memory.
 * TODO: switch to userspace before executing function
 */
class UserThread: public KernelThread
{
public:
	/*!
	 * @brief Ensures correct UserThread creation.
	 */
//	static Thread* create( thread_t* thread_ptr, void* (*thread_start)(void*),
//		void* data = NULL,const unsigned int flags = 0 );


private:
	void* m_userstack;
	void* m_runData2;

	UserThread();
	UserThread( const UserThread& other );
	const UserThread& operator = ( const UserThread& other );

	void run();
	~UserThread();

	/*!
	 * @brief Prepares stack and initial context.
	 */
	UserThread( void* (*func)(void*), void* data, void* data2, void* stack_pos,
		native_t flags = 0, uint stackSize = DEFAULT_STACK_SIZE );

	friend class Process;
};
