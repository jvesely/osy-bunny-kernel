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
#include "assert.h"
#include "api.h"

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

void exit() __attribute__ ((noreturn));

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

int vma_alloc(void ** from, size_t * size, const unsigned int flags);

int vma_free(const void * from);

int event_init( event_t* id );

void event_wait( event_t id, volatile native_t* locked );

int event_wait_timeout( 
	event_t id, const Time* time, volatile native_t* locked );

void event_fire( event_t id );

int event_destroy( event_t id );
}
