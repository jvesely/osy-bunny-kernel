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

size_t puts( const char* str ) __attribute__ ((noinline));

size_t gets( char* buffer, size_t size) __attribute__ ((noinline));

void exit() __attribute__ ((noinline));

int thread_create( 
	thread_t *thread_ptr, void *(*thread_start)(void *), void *arg ) __attribute__ ((noinline));

thread_t thread_self() __attribute__ ((noinline));

int thread_join(
	thread_t thr, void** retval, bool timed = false, Time time = Time(0, 0)) __attribute__ ((noinline));

int thread_detach( thread_t thr ) __attribute__ ((noinline));

int thread_cancel( thread_t thr ) __attribute__ ((noinline));

void thread_sleep( const Time* time ) __attribute__ ((noinline));

void thread_yield() __attribute__ ((noinline));

void thread_suspend() __attribute__ ((noinline));

int thread_wakeup( thread_t thr ) __attribute__ ((noinline));

void thread_exit( void* retval ) __attribute__ ((noinline));

}
