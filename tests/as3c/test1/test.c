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
 * @brief This is a testfile to satisfy assignment 3
 *
 */

static char* desc =
	"\n---\n"
	"Test1 - for assignment 3.\n"
	"Vytvorte ukazkovy uzivatelsky proces, ktery bude ve vice uzivatelskych "
	"vlaknech demonstrovat vystup na konzoli a synchronizaci.\n\n";

#include <librt.h>
#include "../include/defs.h"

#define THREAD_COUNT 10

#define PRINT_COUNT 10
#define PRINT_SLEEP 10

#define PUTC_COUNT 100
#define PUTC_SLEEP 1

struct mutex mtx;

void* thread_proc(void* data)
{
	thread_t self = thread_self();
	printf("Thread %x started...\n", thread);

	thread_yield();

	// test printf
	for (int i = 1; i <= PRINT_COUNT; i++) {
		printf("Printf call (%d) from thread %x.\n", i, thread);
		thread_usleep(PRINT_SLEEP);
	}

	// test puts
	for (int i = 1; i <= PRINT_COUNT; i++) {
		puts("|>--------------------<|");
		thread_usleep(PRINT_SLEEP);
	}

	// test puts
	for (int i = 1; i <= PRINT_COUNT; i++) {
		puts((char *)data);
		thread_usleep(PRINT_SLEEP);
	}

	// test putc without lock
	for (int i = 1; i <= PUTC_COUNT; i++) {
		putc(((char *)data)[0]);
		thread_usleep(PUTC_SLEEP);
	}

	// test putc with lock
	mutex_lock(&mtx);
	printf("\nSynchronized output from thread %x: ", thread);

	for (int i = 1; i <= PUTC_COUNT; i++) {
		putc(((char *)data)[0]);
		thread_usleep(PUTC_SLEEP);
	}

	printf("\n");
	mutex_unlock(&mtx);

	return NULL;
}


int main(void)
{
	// print description
	printf(desc);

	// var definitions
	thread_t thread[THREAD_COUNT];


	// var initialization
	mutex_init(&mtx);

	// work to do
	//mutex_lock(&mtx);
	//mutex_unlock(&mtx);


	// clean up
	mutex_destroy(&mtx);

	// finish
	printf("\n\nTest passed...\n\n");
	return 0;
}

