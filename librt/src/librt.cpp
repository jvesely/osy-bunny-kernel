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
#include "UserMemoryAllocator.h"

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
	return UserMemoryAllocator::instance().getMemory( size );
}
/* -------------------------------------------------------------------------- */
void free( const void *ptr )
{
	UserMemoryAllocator::instance().freeMemory( ptr );
}
//------------------------------------------------------------------------------
void mallocStrategyDefault()
{
	UserMemoryAllocator::instance().setStrategyDefault();
}

void mallocStrategyFirstFit()
{
	UserMemoryAllocator::instance().setStrategyFirstFit();
}

void mallocStrategyNextFit()
{
	UserMemoryAllocator::instance().setStrategyNextFit();
}

void mallocStrategyBestFit()
{
	UserMemoryAllocator::instance().setStrategyBestFit();
}

void mallocStrategyWorstFit()
{
	UserMemoryAllocator::instance().setStrategyWorstFit();
}

size_t mallocatorGetFreeSize()
{
	return UserMemoryAllocator::instance().getFreeSize();
}

size_t mallocatorGetTotalSize()
{
	return UserMemoryAllocator::instance().getTotalSize();
}

/* -------------------------------------------------------------------------- */
/* --------------------------   THREADS   ----------------------------------- */
/* -------------------------------------------------------------------------- */

int thread_create(
  thread_t *thread_ptr, void *(* func )(void*), void* arg )
{
	return SysCall::thread_create( thread_ptr, thread_start, (void*)func, arg  );
}
/* -------------------------------------------------------------------------- */
thread_t thread_self()
{
	return SysCall::thread_self();
}
/* -------------------------------------------------------------------------- */
int thread_join( thread_t thr, void **thread_retval )
{
	return SysCall::thread_join( thr, thread_retval );
}
/* -------------------------------------------------------------------------- */
int thread_join_timeout(
	thread_t thr, void **thread_retval, const unsigned int usec )
{
	const Time time( 0, usec );
	return SysCall::thread_join( thr, thread_retval, &time );
}
/* -------------------------------------------------------------------------- */
int thread_detach( thread_t thr )
{
	return SysCall::thread_detach( thr );
}
/* -------------------------------------------------------------------------- */
int thread_cancel( thread_t thr )
{
	return SysCall::thread_cancel( thr );
}
/* -------------------------------------------------------------------------- */
void thread_sleep( const unsigned int sec )
{
	const Time time( sec, 0 );
	SysCall::thread_sleep( &time );
}
/* -------------------------------------------------------------------------- */
void thread_usleep( const unsigned int usec )
{
	const Time time( 0, usec );
	SysCall::thread_sleep( &time );
}
/* -------------------------------------------------------------------------- */
void thread_yield()
{
	SysCall::thread_yield();
}
/* -------------------------------------------------------------------------- */
void thread_suspend()
{
	SysCall::thread_suspend();
}
/* -------------------------------------------------------------------------- */
int thread_wakeup( thread_t thr )
{
	return SysCall::thread_wakeup( thr );
}
/* -------------------------------------------------------------------------- */
void thread_exit( void* thread_retval )
{
	SysCall::thread_exit( thread_retval );
}
/* -------------------------------------------------------------------------- */
void exit()
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
/* -------------------------------------------------------------------------- */
int mutex_destroy( struct mutex* mtx )
{
	if (!mtx)
		return EINVAL;

	ASSERT(sizeof(*mtx) == sizeof(Mutex));
	((Mutex*)mtx)->destroy();

	return EOK;
}
/* -------------------------------------------------------------------------- */
int mutex_lock( struct mutex* mtx )
{
	if (!mtx)
		return EINVAL;

	ASSERT(sizeof(*mtx) == sizeof(Mutex));
	return ((Mutex*)mtx)->lock();
}
/* -------------------------------------------------------------------------- */
int mutex_lock_timeout ( struct mutex* mtx, const unsigned int usec )
{
	if (!mtx)
		return EINVAL;

	ASSERT(sizeof(*mtx) == sizeof(Mutex));
	return ((Mutex*)mtx)->lockTimeout(Time(0, usec));
}
/* -------------------------------------------------------------------------- */
int mutex_unlock_uncheck( struct mutex* mtx )
{
	if (!mtx)
		return EINVAL;

	ASSERT(sizeof(*mtx) == sizeof(Mutex));
	return ((Mutex*)mtx)->unlock();
}
/* -------------------------------------------------------------------------- */
int mutex_unlock_check( struct mutex* mtx )
{
	if (!mtx)
		return EINVAL;

	ASSERT(sizeof(*mtx) == sizeof(Mutex));
	return ((Mutex*)mtx)->unlockCheck();
}

/*----------------------------------------------------------------------------*/
void spinlock_init(spinlock_t* s) {
	ASSERT(sizeof(spinlock_t) >= sizeof(Spinlock));
	new (s) Spinlock();
}
/*----------------------------------------------------------------------------*/
void spinlock_destroy(spinlock_t* s) {
	((Spinlock *)s)->~Spinlock();
}
/*----------------------------------------------------------------------------*/
void spinlock_lock(spinlock_t* s) {
	((Spinlock *)s)->lock();
}
/*----------------------------------------------------------------------------*/
void spinlock_unlock(spinlock_t* s) {
	((Spinlock *)s)->unlock();
}

/* -------------------------------------------------------------------------- */
int vma_alloc(void **from, size_t * size)
{
	return SysCall::vma_alloc( from, size, ((VF_AT_KUSEG << VF_AT_SHIFT) | (VF_VA_AUTO << VF_VA_SHIFT)) );
}

/* -------------------------------------------------------------------------- */
int vma_free(void *from)
{
	return SysCall::vma_free(from);
}
/* -------------------------------------------------------------------------- */
int vma_resize(const void *from, size_t * size)
{
	return SysCall::vma_resize(from,size);
}



