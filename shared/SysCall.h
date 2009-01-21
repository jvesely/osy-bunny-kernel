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
#include "Time.h"
#include "syscallcodes.h"

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

inline void exit()
{
	SYSCALL( SYS_EXIT, 0, 0, 0, 0);
}

inline int thread_create( 
	thread_t *thread_ptr, void *(*thread_start)(void *), void *arg )
{
	return SYSCALL( SYS_THREAD_CREATE, thread_ptr, thread_start, arg, 0 );
}

inline thread_t thread_self()
{
	return SYSCALL( SYS_THREAD_SELF, 0, 0, 0, 0);
}

inline int thread_join(
	thread_t thr, void** retval, bool timed = false, Time time = Time(0, 0))
{
	return SYSCALL( SYS_THREAD_JOIN, thr, retval, timed, &time );
}

inline int thread_detach( thread_t thr )
{
	return SYSCALL( SYS_THREAD_DETACH, thr, 0, 0, 0 );
}

inline int thread_cancel( thread_t thr )
{
	return SYSCALL( SYS_THREAD_CANCEL, thr, 0, 0, 0 );
}

inline int thread_wakeup( thread_t thr )
{
	return SYSCALL( SYS_THREAD_WAKEUP, thr, 0, 0, 0 );
}

void thread_exit( void* retval )
{
	return SYSCALL( SYS_THREAD_EXIT, retval, 0, 0, 0 );
}
#undef QUOT
#undef SYSCALL
}
