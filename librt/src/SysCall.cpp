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

#include "SysCall.h"
using namespace SysCall;
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

size_t SysCall::puts( const char* str )
{
  return SYSCALL( SYS_PUTS, str, 0, 0, 0 );
}

size_t SysCall::gets( char* buffer, size_t size)
{
  return SYSCALL( SYS_GETS, buffer, size, 0, 0 );
}

void SysCall::exit()
{
  SYSCALL( SYS_EXIT, 0, 0, 0, 0);
}

int SysCall::thread_create( 
  thread_t *thread_ptr, void *( *thread_start)(void *), void *arg )
{
  return SYSCALL( SYS_THREAD_CREATE, thread_ptr, thread_start, arg, 0 );
}

thread_t SysCall::thread_self()
{
  return SYSCALL( SYS_THREAD_SELF, 0, 0, 0, 0);
}

int SysCall::thread_join(
  thread_t thr, void** retval, bool timed, Time time)
{
  return SYSCALL( SYS_THREAD_JOIN, thr, retval, timed, &time );
}

int SysCall::thread_detach( thread_t thr )
{
  return SYSCALL( SYS_THREAD_DETACH, thr, 0, 0, 0 );
}

int SysCall::thread_cancel( thread_t thr )
{
  return SYSCALL( SYS_THREAD_CANCEL, thr, 0, 0, 0 );
}

void SysCall::thread_sleep( const Time* time )
{
  SYSCALL( SYS_THREAD_SLEEP, time, 0, 0, 0 );
}

void SysCall::thread_yield()
{
  SYSCALL( SYS_THREAD_YIELD, 0, 0, 0, 0 );
}

void SysCall::thread_suspend()
{
  SYSCALL( SYS_THREAD_SUSPEND, 0, 0, 0, 0 );
}

int SysCall::thread_wakeup( thread_t thr )
{
  return SYSCALL( SYS_THREAD_WAKEUP, thr, 0, 0, 0 );
}

void SysCall::thread_exit( void* retval )
{
  SYSCALL( SYS_THREAD_EXIT, retval, 0, 0, 0 );
}

#undef QUOT
#undef SYSCALL


