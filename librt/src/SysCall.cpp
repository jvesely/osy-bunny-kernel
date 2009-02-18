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
#define SYSCALL( call ) \
({ \
  register native_t __v0 asm("$2");\
  \
  asm volatile ( \
    "syscall "QUOT(call)" \n"\
    :"=r"(__v0)\
    ::\
  );\
  __v0;\
})

size_t SysCall::puts( const char* str )
{
	return SYSCALL( SYS_PUTS );
}

size_t SysCall::gets( char* buffer, size_t size)
{
	return SYSCALL( SYS_GETS );
}

void SysCall::exit()
{
	SYSCALL( SYS_EXIT );
	while (1) ;
}

int SysCall::thread_create(
  thread_t *thread_ptr, void *( *thread_start)(void*, void*), void *arg, void* arg2 )
{
  return SYSCALL( SYS_THREAD_CREATE );
}

thread_t SysCall::thread_self()
{
	return SYSCALL( SYS_THREAD_SELF );
}

int SysCall::thread_join(
  thread_t thr, void** retval, const Time* time)
{
	return SYSCALL( SYS_THREAD_JOIN );
}

int SysCall::thread_detach( thread_t thr )
{
	return SYSCALL( SYS_THREAD_DETACH );
}

int SysCall::thread_cancel( thread_t thr )
{
	return SYSCALL( SYS_THREAD_CANCEL );
}

void SysCall::thread_sleep( const Time* time )
{
	SYSCALL( SYS_THREAD_SLEEP );
}

void SysCall::thread_yield()
{
	SYSCALL( SYS_THREAD_YIELD );
}

void SysCall::thread_suspend()
{
	SYSCALL( SYS_THREAD_SUSPEND );
}

int SysCall::thread_wakeup( thread_t thr )
{
	return SYSCALL( SYS_THREAD_WAKEUP );
}

void SysCall::thread_exit( void* retval )
{
	SYSCALL( SYS_THREAD_EXIT );
	while (1) ;
}

int SysCall::vma_alloc(void ** from, volatile size_t * size, const unsigned int flags)
{
	return SYSCALL( SYS_VMA_ALLOC );
}

int SysCall::vma_free(const void * from)
{
	return SYSCALL( SYS_VMA_FREE );
}

int SysCall::vma_resize(const void *from, volatile size_t * size)
{
	return SYSCALL( SYS_VMA_RESIZE );
}


int SysCall::event_init( event_t* id )
{
	return (id) ? SYSCALL( SYS_EVENT_INIT ) : (int)(EINVAL);
}

void SysCall::event_wait( event_t id, volatile native_t* locked )
{
	SYSCALL( SYS_EVENT_WAIT );
}

int SysCall::event_wait_timeout(
	event_t id, const Time* time, volatile native_t* locked )
{
	return SYSCALL( SYS_EVENT_WAIT_TIMEOUT );
}

void SysCall::event_fire( event_t id )
{
	SYSCALL( SYS_EVENT_FIRE );
}

int SysCall::event_destroy( event_t id )
{
	return SYSCALL( SYS_EVENT_DESTROY );
}

void SysCall::getCurrentTime( Time * time )
{
	SYSCALL( SYS_GET_TIME );
}

int SysCall::process_create( process_t *process_ptr, const void *img, const size_t size )
{
	return SYSCALL( SYS_PROC_CREATE );
}

int SysCall::process_join( process_t proc, const Time * time )
{
	return SYSCALL( SYS_PROC_JOIN );
}

int SysCall::process_kill( process_t proc )
{
	return SYSCALL( SYS_PROC_KILL );
}


#undef QUOT
#undef SYSCALL

