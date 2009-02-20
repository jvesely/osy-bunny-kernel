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
#include "api.h"
#include "Time.h"
#include "direntry.h"

/*!
 * @namespace SysCall
 * @brief SysCall invoking functions.
 *
 * See @a SysCall class for implementation of syscalls
 */
namespace SysCall
{

size_t puts( const char* str );

size_t gets( char* buffer, size_t size);

/*----------------------------------------------------------------------------*/
int thread_create(
	thread_t *thread_ptr, void *(*thread_start)(void*, void*), void *arg, void* arg2 );

thread_t thread_self();

int thread_join(
	thread_t thr, void** retval, const Time* time = NULL);

int thread_detach( thread_t thr );

int thread_cancel( thread_t thr );

void thread_sleep( const Time* time );

void thread_yield();

void thread_suspend();

int thread_wakeup( thread_t thr );

void thread_exit( void* retval ) __attribute__ ((noreturn));
/*----------------------------------------------------------------------------*/
/** @brief allocate virtual memory area
*
*	Wrapper for kernel space vma_alloc, except that parameter size is pointer.
*	Because size might need to be alligned to some value (supported page size),
*	resultant size is returned via size pointer. This is main difference from
*	kernel space vma_alloc funcion.
*	@param from pointer to pointer where result should be stored
*		address of newly allocated area is stored in this pointer
*	@param size pointer to required size of new virtual memory area
*		Resultant size of new area is returned via this pointer.
*	@param flags flags for vma_allocator, see kernel space vma_alloc reference
*	@return
*/
int vma_alloc(void ** from, volatile size_t * size, const unsigned int flags);

/** @brief deallocate virtual memory area
*
*	Wrapper for kernel space vma_free.
*/
int vma_free(const void * from);

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
int vma_resize(const void *from, volatile size_t * size);
/*----------------------------------------------------------------------------*/
int event_init( event_t* id );

void event_wait( event_t id, volatile native_t* locked );

int event_wait_timeout(
	event_t id, const Time* time, volatile native_t* locked );

void event_fire( event_t id );

int event_destroy( event_t id );
/*----------------------------------------------------------------------------*/
void getCurrentTime( Time * time );
/*----------------------------------------------------------------------------*/
int process_create( process_t *process_ptr, const void *img, const size_t size );

int process_join( process_t proc, const Time * time );

int process_kill( process_t proc );


void exit() __attribute__ ((noreturn));
/*----------------------------------------------------------------------------*/
int open( file_t* fd, const char* file_name, const char mode = 0 );

int close( file_t fd );

int fread( file_t fd, void* buffer, size_t size );

int fseek( file_t fd, FilePos, int offset );

int direntry( file_t fd, DIR_ENTRY* entry );
}
