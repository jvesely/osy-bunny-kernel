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

static void* thread_proc(void* data)
{
	thread_t thread = thread_self();
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

	char* string = "A thread.";

	int res;
	char* str;
	// do the test
	for (int i = 0; i < THREAD_COUNT; i++) {
		str = (char *)malloc(10 * sizeof(char));
		// copy string
		for (int j = 0; j < 10; j++) str[j] = string[j];
		// set label
		str[0] += i;
		// create threads
		res = thread_create(thread + i, thread_proc, str);
		if (res != EOK) {
			printf ("Unable to create thread (%d)\n", res);
		}
	}

	// join threads
	void* x;
	for (int i = 0; i < THREAD_COUNT; i++) {
		thread_join(thread[i], &x);
	}

	// clean up
	mutex_destroy(&mtx);

	// finish
	printf("\n\nTest passed...\n\n");
	return 0;
}

