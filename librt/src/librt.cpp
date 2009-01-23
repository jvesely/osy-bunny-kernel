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
#include "Time.h"
#include "Mutex.h"
#include "cpp.h"
#include "assert.h"

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

void *malloc(const size_t size)
{
	return NULL;
}

void free(const void *ptr)
{
}

/* -------------------------------------------------------------------------- */
/* --------------------------   THREADS   ----------------------------------- */
/* -------------------------------------------------------------------------- */

int thread_create( 
	thread_t *thread_ptr, void *(*thread_start)(void *), void *arg)
{
	return 0;
}

thread_t thread_self(void)
{
	// get the thread id somehow
	return 0;
}

int thread_join(thread_t thr, void **thread_retval)
{
	// would need some synchronization, or call syscall directly
	return EOK;
}

int thread_join_timeout(
	thread_t thr, void **thread_retval, const unsigned int usec)
{
	// would need some synchronization, or call syscall directly
	return EOK;
}

int thread_detach(thread_t thr)
{
	// would need some synchronization, or call syscall directly
	return EOK;
}

int thread_cancel(thread_t thr)
{
	// would need some synchronization, or call syscall directly
	return EOK;
}

void thread_sleep(const unsigned int sec)
{
	SysCall::thread_sleep(sec, 0);
}

void thread_usleep(const unsigned int usec)
{
	SysCall::thread_sleep(0, usec);
}

void thread_yield(void)
{
	SysCall::thread_yield();
}

void thread_suspend(void)
{
	SysCall::thread_suspend();
}

int thread_wakeup(thread_t thr)
{
	// would need some synchronization, or call syscall directly
	return EOK;
}

void thread_exit(void* thread_retval)
{
	// hmm..don't know what to do besides to kill the thread
}

void exit(void)
{
	SysCall::exit();
}

/* -------------------------------------------------------------------------- */
/* ----------------------------   MUTEX   ----------------------------------- */
/* -------------------------------------------------------------------------- */

int mutex_init( struct mutex* mtx )
{
	if (!mtx)
		return EINVAL;
	
	ASSERT(sizeof(*mtx) == sizeof(Mutex));
	mtx = (mutex*)(new ((void*)mtx) Mutex());

	return ((Mutex*)mtx)->init();
}

int mutex_destroy( struct mutex* mtx )
{
	if (!mtx)
		return EINVAL;

	ASSERT(sizeof(*mtx) == sizeof(Mutex));
	((Mutex*)mtx)->destroy();
	
	delete mtx;

	return EOK;
}

int mutex_lock( struct mutex* mtx )
{
	if (!mtx)
		return EINVAL;

	ASSERT(sizeof(*mtx) == sizeof(Mutex));
	return ((Mutex*)mtx)->lock();
}

int mutex_lock_timeout ( struct mutex* mtx, const unsigned int usec )
{
	if (!mtx)
		return EINVAL;

	ASSERT(sizeof(*mtx) == sizeof(Mutex));
	return ((Mutex*)mtx)->lockTimeout(Time(0, usec));
}

int mutex_unlock_uncheck( struct mutex* mtx )
{
	if (!mtx)
		return EINVAL;

	ASSERT(sizeof(*mtx) == sizeof(Mutex));
	return ((Mutex*)mtx)->unlock();
}

int mutex_unlock_check( struct mutex* mtx )
{
	if (!mtx)
		return EINVAL;

	ASSERT(sizeof(*mtx) == sizeof(Mutex));
	return ((Mutex*)mtx)->unlockCheck();
}

/*----------------------------------------------------------------------------*/

extern "C" int main();
extern "C" void __start() __attribute__ ((section (".entry"), noreturn)) ;
void __start()
{
	main();
	exit();
	while (1) ;
}
