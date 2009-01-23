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
#include "syscallcodes.h"

class Time;

/*!
 * @namespace SysCall
 * @brief SysCall invoking functions.
 *
 * See @a SysCall class for implementation of syscalls
 */
namespace SysCall
{

#define QUOT(expr) #expr
#define SYSCALL( call, p0, p1, p2, p3 ) \
({ \
	register unative_t __a0 asm("$4") = (unative_t) p0;\
	register unative_t __a1 asm("$5") = (unative_t) p1;\
	register unative_t __a2 asm("$6") = (unative_t) p2;\
	register unative_t __a3 asm("$7") = (unative_t) p3;\
	\
	register native_t __v0 asm("$2");\
	\
	asm volatile ( \
		"syscall "QUOT(call)" \n"\
		:"=r"(__v0)\
		:"r"(__a0), "r"(__a1), "r"(__a2), "r"(__a3)\
		:\
	);\
	__v0;\
})

inline size_t puts( const char* str )
{
	return SYSCALL( SYS_PUTS, str, 0, 0, 0 );
}

inline size_t gets( char* buffer, size_t size)
{
	return SYSCALL( SYS_GETS, buffer, size, 0, 0 );
}

inline int event_init( event_t* id )
{
	return (id) ? SYSCALL(SYS_EVENT_INIT, id, 0, 0, 0) : (int)(EINVAL);
}

inline void event_wait( event_t id, volatile native_t* locked )
{
	SYSCALL(SYS_EVENT_WAIT, id, locked, 0, 0);
}

inline void event_wait_timeout( 
	event_t id, const Time* time, volatile native_t* locked )
{
	SYSCALL(SYS_EVENT_WAIT_TIMEOUT, id, time, locked, 0);
}

inline void event_fire( event_t id )
{
	SYSCALL(SYS_EVENT_FIRE, id, 0, 0, 0);
}

inline int event_destroy( event_t id )
{
	return SYSCALL(SYS_EVENT_DESTROY, id, 0, 0, 0);
}

inline void thread_sleep( uint sec, uint usec )
{
	SYSCALL(SYS_THREAD_SLEEP, sec, usec, 0, 0);
}

inline void thread_yield()
{
	SYSCALL(SYS_THREAD_YIELD, 0, 0, 0, 0);
}

inline void thread_suspend()
{
	SYSCALL(SYS_THREAD_SUSPEND, 0, 0, 0, 0);
}

inline void exit() {
	SYSCALL( SYS_EXIT, 0, 0, 0, 0);
}



#undef QUOT
#undef SYSCALL
}
