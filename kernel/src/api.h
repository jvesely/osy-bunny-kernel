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
native_t getc_try();

/*! gets tries to read multiple chars from the device buffer.
 * If len is 0 returns EINVAL. Reads from the buffer until '\n'
 * is read or len characters were read. \0 is always put at the end.
 * @param str pointer to the buffer to be filled.
 * @param len number of chars to be read if no \\n is encountered.
 * @return number of chars filled into buffer str
 */
ssize_t gets(char * str, const size_t len);


void * malloc(const size_t size);

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


int thread_create( thread_t* thread_ptr, void (*thread_start)(void*),
	void* data, const unsigned int flags);

thread_t thread_get_current();

int thread_join(thread_t thr);

int thread_join_timeout(thread_t thr, const unsigned int usec);

int thread_detach(thread_t thr);

void thread_sleep(const unsigned int sec);

void thread_usleep(const unsigned int usec);

void thread_yield();

void thread_suspend();

int thread_wakeup(thread_t thr);

int thread_kill(thread_t thr);

#ifdef __cplusplus
}
#endif
