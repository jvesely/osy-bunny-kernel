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
 * @brief API declarations
 *
 * Requiered functions that should be provided by kernel.
 */
#pragma once

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif
void run_test(void);

/*! putc outputs one character.
 * @param c character to be printed
 * @return number of printed chars (0 or 1)
 */
size_t putc(const char c);

/*! puts outputs strings.
 * checks whether param is not null, prints chars until "\0" is found
 * @param str pointer to the first char of the string
 * @return number of printed chars
 */
size_t puts(const char * str);

/*! printf and printk are the same thing */
#define printf printk

/*! printk prints formated string on the console.
 * formating string may include:
 * %c: corresponding input variable is treated as char
 * %s: corresponding input variale is treated as char *
 * %d: corresponding input variale is treated as signed decimal number
 * %i: same as %d
 * %u: same as %d but the number is treated as unsigned
 * %x: corresponding input variale is treated as unsinged hexadecimal number
 * %p: corresponding input variale is treated as void *output is the same as %x
 * @param format format string
 * @... variable number of paramters used in the format string
 * @return number of printed chars
 */
size_t printk(const char * format, ...);

/*! getc reads one char fromthe device buffer.
 * If there is no char in the input buffer requesting thread is blocked.
 * @return read char
 */
char getc();

/*! getc_try tries to read one char from the device buffer.
 * If there is no char in the input buffer returns EWOULDBLOCK instead
 * of blocking.
 * @return read char or EWOULDBLOCK on empty buffer
 */
int getc_try();

/*! gets tries to read multiple chars from the device buffer.
 * If len is 0 returns EINVAL. Reads from the buffer until '\n'
 * is read or len characters were read. \0 is always put at the end.
 * @param str pointer to the buffer to be filled.
 * @param len number of chars to be read if no \\n is encountered.
 * @return number of chars filled into buffer str
 */
ssize_t gets(char * str, const size_t len);

/*! @brief Allocates block of size size on the heap.
 *	@param size requested size of the block.
 *	@return pointer to the allocated block, NULL on failure
 */
void * malloc(const size_t size);

/*! @brief Returns no longer needed block back to the heap.
 * @param ptr pointer to the block
 */
void free (const void * ptr);

/*! assert and ASSERT are the same thing. */
#define assert ASSERT

/*! ASSERT macro.
 * Macro tests condition and calls panic, if condition is false.
 * This macro only works when compiled without NDEBUG macro defined.
 * @param test Condition to be tested.
 */
#ifndef NDEBUG
#	define ASSERT(test) \
	if (! (test) ) { \
		panic("ASSERTION FAILED: \"%s\" in %s on line %d\n", (char*)#test, __FILE__, __LINE__); \
	}
#else
#	define ASSERT(x)
#endif

/*! dprintf and dprintk are the same thing. */
#define dprintf dprintk

/*! dprintk macro.
 * Macro prints some debuging info before output.
 * This macro only works without NDEBUG macro defined
 * @param ARGS multiple vars beginning with format string, see printk
 */
#ifndef NDEBUG
#	define dprintk(ARGS...) \
	printf("Function %s on line %d: \n\t", __PRETTY_FUNCTION__, __LINE__);\
	printf(ARGS);
#else
#	define dprintk(ARGS...)
#endif

/*! panic macro.
 * Macro dumps registers, prints out message that beggins with "Kernel panic: "
 * and stops simulation, after that it block the kernel.
 */
#define panic(ARGS...) \
	reg_dump() \
	kpanic(ARGS); // still needs tu dump those registers

void kpanic(const char* format, ... );

/*! reg_dump macro.
 * Dumps processor registers, now uses msim special instruction,
 * should be changed later
 */
#define reg_dump()

/*! Creates and runs a new thread. Stores its identifier.
 * @param thread_ptr pointer to place where identifier would be stored.
 * @param thread_start routine to run in the new thread
 * @param data data that will be sent to the routine
 * @param flags ignored param
 * @return ENOMEM on lack of memory, EOK on sucess
 */
int thread_create( thread_t* thread_ptr, void* (*thread_start)(void*),
	void* data, const unsigned int flags);

/*! Gets identifier of the currently running thread.
 * @return thread identifier
 */
thread_t thread_get_current();

/*! Blocks calling thread until given thread finishes execution.
 * @param thr id of the thread to wait for
 * @return EINVAL if attempting to wait for oneself, non-existing thread
 * id or if the thread is already beeing waited for, EKILLED if the thread
 * has already been killed EOK on success. (wakes when the thread is dead).
 */
int thread_join(thread_t thr);

/*! Similiar to version without timer, except that it waits max usec
 * microsecs.
 * @param thr thread id of the threadd one wants to wait for
 * @param usec number of microsecs to wait before returning ETIMEDOUT.
 * @return See non-timed version, plus ETIMEOUT if thread is still running 
 * after given time.
 */
int thread_join_timeout(thread_t thr, const unsigned int usec);

/*! Sets thread state to detached. Detached thread can not be waited for
 * and its structures are deleted emediately after it finishes execution.
 * @param thr thread to detach
 */
int thread_detach(thread_t thr);

/*! Stops executing calling thread for sec seconds.
 * @param sec number of seconds to refrain from execution.
 */
void thread_sleep(const unsigned int sec);

/*! Similiar to sleep, this time in microseconds.
 * @param usec umber of microseconds to wait.
 */
void thread_usleep(const unsigned int usec);

/*! Surrender execution of the calling thread in favour of any other */
void thread_yield();

/*! Stop executing calling thread and don't schedule it until the thread has
 * been woken up.
 */
void thread_suspend();

/*! Start scheduling thread for execution again.
 * @param thr thread to wake up.
 */
int thread_wakeup(thread_t thr);

/*! Kills thread. If the thread is detached it will be destroyed. Attached
 * threads will stop executing and won't be scheduled, but will remain in 
 * the memory until ... .
 */
int thread_kill(thread_t thr);

#ifdef __cplusplus
}
#endif
