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

#include "librt.h"
#include "SysCall.h"

struct thread_startup {
	void *(*func)(void*);
	void* data;
};

extern "C" int main();
extern "C" void __start() __attribute__ ((section (".entry"), noreturn)) ;
void __start()
{
	main();
	exit();
}
/*----------------------------------------------------------------------------*/
void* thread_start( void* func, void* data) __attribute__(( noreturn ));
void* thread_start( void* func, void* data)
{
	printf("Starting Process thread %p(%p)\n", func, data);
	void* ret = ((void*(*)(void*))func)(data);
	thread_exit( ret );
}
/*----------------------------------------------------------------------------*/
/* Basic IO */
size_t putc( const char c )
{
	static char buffer[2] = { '\0', '\0' };
	buffer[0] = c;
	return SysCall::puts( buffer );
}
/*----------------------------------------------------------------------------*/
size_t puts( const char* str )
{
	return SysCall::puts( str );
}
/*----------------------------------------------------------------------------*/
char getc()
{
	char buffer;
	SysCall::gets( &buffer, 1);
	return buffer;
}
/*----------------------------------------------------------------------------*/
ssize_t gets( char* str, const size_t len )
{
	/* we don't need to call syscall just to report error */
	if (!len) return EINVAL;  
	return SysCall::gets( str, len );
}
/* -------------------------------------------------------------------------- */
/* ---------------------------   MEMORY   ----------------------------------- */
/* -------------------------------------------------------------------------- */
void* malloc( const size_t size )
{
	return NULL;
}
/*----------------------------------------------------------------------------*/
void free( const void *ptr )
{
}
/* -------------------------------------------------------------------------- */
/* ---------------------------   THREADS   ---------------------------------- */
/* -------------------------------------------------------------------------- */
int thread_create(
  thread_t *thread_ptr, void *(* func )(void*), void* arg )
{
	printf("Creating thread with func: %p(%p,%p).\n", thread_start, func, arg);
	return SysCall::thread_create( thread_ptr, thread_start, (void*)func, arg  );
}
/*----------------------------------------------------------------------------*/
thread_t thread_self( void )
{
	return SysCall::thread_self();
}
/*----------------------------------------------------------------------------*/
int thread_join( thread_t thr, void **thread_retval )
{
	return SysCall::thread_join( thr, thread_retval );
}
/*----------------------------------------------------------------------------*/
int thread_join_timeout(
  thread_t thr, void **thread_retval, const unsigned int usec )
{
	const Time time( 0, usec );
	return SysCall::thread_join( thr, thread_retval, true, &time );
}
/*----------------------------------------------------------------------------*/
int thread_detach( thread_t thr )
{
	return SysCall::thread_detach( thr );
}
/*----------------------------------------------------------------------------*/
int thread_cancel( thread_t thr )
{
	return SysCall::thread_cancel( thr );
}
/*----------------------------------------------------------------------------*/
void thread_sleep( const unsigned int sec )
{
	const Time time( sec, 0 );
	SysCall::thread_sleep( &time );
}
/*----------------------------------------------------------------------------*/
void thread_usleep( const unsigned int usec )
{
	const Time time( 0, usec );
	SysCall::thread_sleep( &time );
}
/*----------------------------------------------------------------------------*/
void thread_yield( void )
{
	SysCall::thread_yield();
}
/*----------------------------------------------------------------------------*/
void thread_suspend( void )
{
	SysCall::thread_suspend();
}
/*----------------------------------------------------------------------------*/
int thread_wakeup( thread_t thr )
{
	return SysCall::thread_wakeup( thr );
}
/*----------------------------------------------------------------------------*/
void thread_exit( void *thread_retval )
{
	SysCall::thread_exit( thread_retval );
}
/*----------------------------------------------------------------------------*/
void exit()
{
	SysCall::exit();
}
/* -------------------------------------------------------------------------- */
/* -----------------------------   MUTEX   ---------------------------------- */
/* -------------------------------------------------------------------------- */
int mtx_ok( struct mutex* mtx )
{
	if (!mtx)
		return EINVAL;
	
	return EOK;
}
/*----------------------------------------------------------------------------*/
int mutex_init( struct mutex* mtx )
{
	return 0;
}
/*----------------------------------------------------------------------------*/
int mutex_destroy( struct mutex* mtx )
{
	return 0;
}
/*----------------------------------------------------------------------------*/
int mutex_lock( struct mutex* mtx )
{
	return 0;
}
/*----------------------------------------------------------------------------*/
int mutex_lock_timeout ( struct mutex* mtx, const unsigned int usec )
{
	return 0;
}
/*----------------------------------------------------------------------------*/
int mutex_unlock( struct mutex* mtx )
{
	return 0;
}
/*----------------------------------------------------------------------------*/

