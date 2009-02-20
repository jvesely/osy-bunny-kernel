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

#define panic(ARGS...) printf(ARGS); thread_exit(NULL);

#if DEBUG_MUTEX >= 1
	#define mutex_unlock mutex_unlock_check
#else
	#define mutex_unlock mutex_unlock_uncheck
#endif

#include "types.h"
#include "flags.h"
#include "assert.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Basic IO */
size_t putc( const char c );

size_t puts( const char* str );

size_t printf( const char* format, ...  );

char getc();

ssize_t gets( char* str, const size_t len );

/* -------------------------------------------------------------------------- */
/* ---------------------------   MEMORY   ----------------------------------- */
/* -------------------------------------------------------------------------- */

/*!
 * @brief Allocates block of size size on the heap.
 *
 * @param size requested size of the block.
 * @retval pointer to the allocated block.
 * @retval NULL on failure.
 */
void* malloc( const size_t size );

/*!
 * @brief Returns no longer needed block back to the heap.
 *
 * @param ptr pointer to the block.
 */
void free( const void *ptr );

//------------------------------------------------------------------------------
//advanced mallocator features

/** @brief set strategy to default
*
*	Wrapper function
*/
void mallocStrategyDefault();

/** @brief set strategy to first fit
*
*	Wrapper function
*/
void mallocStrategyFirstFit();

/** @brief set strategy to next fit
*
*	Wrapper function
*/
void mallocStrategyNextFit();

/** @brief set strategy to best fit
*
*	Wrapper function
*/
void mallocStrategyBestFit();

/** @brief set strategy to worst fit
*
*	Wrapper function
*/
void mallocStrategyWorstFit();

/** @brief get size of free memory currently available to mallocator
*
*	This has nothing to do with total free physical memory.
*/
size_t mallocatorGetFreeSize();

/** @brief get size of total memory ever requested by mallocator
*
*	This has nothing to do with total physical memory used.
*/
size_t mallocatorGetTotalSize();

/* -------------------------------------------------------------------------- */
/* ---------------------------   THREADS   ---------------------------------- */
/* -------------------------------------------------------------------------- */

/*!
 * @brief Creates and runs a new thread. Stores its identifier.
 *
 * @param thread_ptr pointer to place where identifier would be stored.
 * @param thread_start function to be run in the new thread.
 * @param arg ???
 * @retval ENOMEM on lack of memory.
 * @retval EOK on success.
 */
int thread_create(
	thread_t *thread_ptr, void *(*thread_start)(void *), void *arg );

/*! @brief Gets identifier of the currently running thread. */
thread_t thread_self( void );

/*!
 * @brief Blocks calling thread until the given thread finishes execution.
 *
 * @param thr thread id of the thread to wait for.
 * @param thread_retval pointer to a pointer to the return value of the
 *	@a thr thread.
 * @retval EINVAL if attempting to wait for oneself, non-existing thread
 *  id or if the thread is already beeing waited for.
 * @retval EKILLED if the thread has already been killed.
 * @retval EOK on success. (wakes when the thread is dead).
 */
int thread_join( thread_t thr, void **thread_retval );

/*!
 * @brief Similiar to version without timer, except that it waits max usec
 *  microseconds.
 *
 * @param thr thread id of the thread to wait for.
 * @param thread_retval pointer to place where return value should be stored.
 * @param usec number of microsecs to wait before returning ETIMEDOUT.
 * @return See non-timed version, plus ETIMEOUT if thread is still running
 * after given time.
 */
int thread_join_timeout(
	thread_t thr, void **thread_retval, const unsigned int usec );

/*!
 * @brief Sets thread state to detached.
 *
 * Detached thread can not be waited for
 * and its structures are deleted immediately after it finishes execution.
 *
 * @param thr thread to be detached.
 * @retval EOK on success.
 * @retval EINVAL if @a thr is a non-existing thread id, the thread is already
 * detached, the thread was finished and is waiting for a join, or if
 * some other thread waits for @a thr in thread_join().
 */
int thread_detach( thread_t thr );

/*!
 * @brief Kills the given thread (if running).
 *
 * Thread waiting for this thread to finish will be unblocked.
 * If the thread is detached it will be destroyed. Attached threads will stop
 * executing and won't be scheduled, but will remain in the memory until joined.
 */
int thread_cancel( thread_t thr );

/*!
 * @brief Stops executing calling thread for @a sec seconds.
 *
 * @param sec number of seconds to refrain from execution.
 */
void thread_sleep( const unsigned int sec );

/*!
 * @brief Stops executing calling thread for @a usec microseconds.
 *
 * @param usec number of microseconds to refrain from execution.
 */
void thread_usleep( const unsigned int usec );

/*! Gives up execution of the calling thread in favour of any other. */
void thread_yield( void );

/*!
 * @brief Stops executing calling thread and don't schedule it until the thread
 * has been woken up.
 *
 * @sa thread_wakeup().
 */
void thread_suspend( void );

/*!
 * @brief Starts scheduling thread for execution again.
 *
 * @param thr Thread to be woken up.
 * @retval EINVAL if @a thr is not a valid id of a suspended thread.
 * @retval EOK otherwise.
 */
int thread_wakeup( thread_t thr );

/*!
 * @brief Stops executing calling thread and allow access to the pointer
 * @a thread_retval to the thread waiting for the calling thread in thread_join.
 */
void thread_exit( void *thread_retval ) __attribute__ ((noreturn)) ;

/* -------------------------------------------------------------------------- */
/* ---------------------------   PROCESS   ---------------------------------- */
/* -------------------------------------------------------------------------- */

/*!
 * @brief Creates new process from the given image.
 *
 * @param process_ptr Place where id of the new thread will be stored.
 * @param img Pointer to the start of the process image.
 * @param size Size of the process image.
 * @return EOK if process was created successfully, ENOMEM if there was not
 * 	eneough free memory to create the process.
 */
int process_create(
          process_t *process_ptr, const void *img, const size_t size);
/*!
 * @brief Gets identifier of the currently running process.
 * @return ID of the current process.
 */
process_t process_self();

/*!
 * @brief Suspends thread until the given process has existed.
 * @param proc ID of the process to wait for.
 * @return EOK if process was successfully joined, EINVAL if no process
 * 	with such an ID exists.
 */
int process_join( process_t proc );

/*!
 * @brief Suspends thread until either the given process has exited,
 * 	or the time has run out.
 * @param proc ID of the process to wait for.
 * @param usec Time in microseconds to wait until the thread is woken 
 * 	even if the process is still alive.
 * @retval EINVAL no process with @a proc id exists.
 * @retval ETIMEDOUT Time is out and process is still alive
 * @retval EOK Process was successfully joined
 */
int process_join_timeout( process_t proc, const unsigned int usec );

/*!
 * @brief Stops execution of the given process.
 * @param proc ID of the process to kill.
 * @return EINVAL if no thread with @a proc id exists. EOK if process 
 * 	was killed sucessfully.
 *
 * All resources of the killed process are freed. All threads killed.
 */
int process_kill( process_t proc );

/*!
 * @brief Stops executing all threads of the current process.
 *
 * All process resources are freed. If there is no other user processes, the
 * kernel will end.
 */
void exit( void ) __attribute__ ((noreturn)) ;

/* -------------------------------------------------------------------------- */
/* -----------------------------   MUTEX   ---------------------------------- */
/* -------------------------------------------------------------------------- */

/*!
 * @struct mutex librt.h "librt.h"
 * @brief Sructure for mutex (mutual exclusion).
 */
struct mutex {
	/*! @brief Mutex class placeholder */
	char mtx[16];
};

/*!
 * @brief Initialize the given mutex struct (to unlocked state).
 *
 * @param mtx Mutex struct to initialize.
 * @retval EOK if successful.
 * @retval ENOMEM if the initialization failed due to lack of memory.
 * @retval EINVAL if @a mtx is not a valid mutex identifier.
 *
 * @todo maybe change the return values
 */
int mutex_init( struct mutex* mtx );

/*!
 * @brief Destroy the given mutex struct.
 *
 * If there are any locked threads on this mutex, the currently running thread
 * is stopped.
 *
 * @param mtx Mutex struct to destroy.
 * @retval EOK if successful.
 * @retval EINVAL if @a mtx is not a valid mutex identifier.
 */
int mutex_destroy( struct mutex* mtx );

/*!
 * @brief Locks the given mutex. If the mutex is already locked, blocks the thread
 * until it is unlocked.
 *
 * @param mtx Mutex to lock.
 * @retval EOK if successful.
 * @retval EINVAL if @a mtx is not a valid mutex identifier.
 */
int mutex_lock( struct mutex* mtx );

/*!
 * @brief Lock the given mutex, but don't let it take more than the given time
 * limit.
 *
 * @param mtx Mutex to lock within time limit.
 * @param usec Time limit in microseconds for trying to lock the mutex.
 * @retval EOK if successful.
 * @retval EINVAL if @a mtx is not a valid mutex identifier.
 * @retval ETIMEDOUT if the mutex could not been locked in the given time.
 */
int mutex_lock_timeout ( struct mutex* mtx, const unsigned int usec );

/*!
 * @brief Unlocks the mutex. Unblocks the first thread waiting for this mutex.
 *
 * In case the DEBUG_MUTEX symbol is defined and >= 1 and this function is
 * called by other thread than the one that locked the mutex, the currently running
 * thread is killed.
 *
 * @param mtx Mutex struct to unlock.
 * @retval EOK if successful.
 * @retval EINVAL if @a mtx is not a valid mutex identifier.
 */
int mutex_unlock( struct mutex* mtx );

int mutex_unlock_check( struct mutex* mtx );

int mutex_unlock_uncheck( struct mutex* mtx );

/* --------------------------------------------------------------------- */
/* ----------------------    SPINLOCK   -------------------------------- */
/* --------------------------------------------------------------------- */

/**
 * @struct spinlock_t librt.h "librt.h"
 * @brief Spinlock class placeholder for C code.
 */
typedef struct spinlock {
	char payload[4];
} spinlock_t;

/** @brief initializes spinlock
*
*	Uninitializes spinlock should not be used.
*/
void spinlock_init(spinlock_t* s);
/** @brief uninitializes spinlock
*
*	Does not dealloc spinlock structure.
*/
void spinlock_destroy(spinlock_t* s);

/** @brief lock spinlock
*
*	Waits actively in loop until lock is acquired.
*/
void spinlock_lock(spinlock_t* s);

/** @brief unlock spinlock*/
void spinlock_unlock(spinlock_t* s);


/* --------------------------------------------------------------------- */
/* ---------------------   VIRTUAL MEMORY AREA  ------------------------ */
/* --------------------------------------------------------------------- */

/** @brief allocate virtual memory area
*
*	Wrapper for syscall vma_alloc. Because size might need to be alligned
*	to some value (supported page size), resultant size is returned via size
*	pointer. This is main difference from kernel space vma_alloc funcion.
*	Also there is no flags parameter, only allowed space for user vma_alloc
*	is user segment.
*	@param from pointer to pointer where result should be stored
*		address of newly allocated area is stored in this pointer
*	@param size pointer to required size of new virtual memory area
*		Resultant size of new area is returned via this pointer.
*	@return EOK if success, EINVAL if could not allocate on address *from
*		(if from was required- see flags parameter) or ENOMEM if there
*		was not enough memory
*/
int vma_alloc(void **from, size_t * size);

/** @brief deallocate virtual memory area
*
*	Wrapper for syscall vma_free.
*/
int vma_free(const void *from);

/** @brief resize virtual memory area
*
*	Wrapper for syscall vma_resize. Because size might need to be alligned
*	to some value (supported page size), resultant size is returned via size
*	pointer. This is main difference from kernel space vma_resize funcion.
*	@param from pointer to pointer where result should be stored
*		address of newly allocated area is stored in this pointer
*	@param size pointer to required new size of virtual memory area
*		Resultant size of area is returned via this pointer.
*	@return EOK on success, ENOMEM if it was not possible to
*		increase vma size
*/
int vma_resize(const void *from, size_t * size);

/* -------------------------------------------------------------------------- */
/* -----------------------------   FILES   ---------------------------------- */
/* -------------------------------------------------------------------------- */

#include "direntry.h"

int fopen( file_t* fd, const char* file_name, const char mode );

int fclose( file_t fd );

int fread( file_t fd, void* buffer, size_t size );

int fseek( file_t fd, int pos, int offset );

int opendir( file_t* fd, const char* path );

int closedir( file_t fd );

int direntry( file_t dir_fd, DIR_ENTRY* entry );

#ifdef __cplusplus
}
#endif
