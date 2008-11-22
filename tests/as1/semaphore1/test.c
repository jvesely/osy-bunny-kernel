/***
 * Mutex test #1
 *
 * Change Log:
 * 2003/10/22 ghort	created
 * 2003/11/02 pallas	cleaned up and integrated
 * 2004/10/25 pallas	updated for new assignment specification
 */

static char *	desc =
	"Semaphore test #1\n"
	"Creates a mutex, which is then repeatedly locked and\n"
	"unlocked by a single thread.\n\n";


#include <api.h>
#include "../include/defs.h"


/*
 * The number of cycles we do in this test.
 */
#define CYCLE_COUNT	(TASK_SIZE * 1000)


void
run_test (void)
{
	int		cnt;
	semaphore_t mtx;

	printk (desc);

	// initialize
	semaphore_init (& mtx, 1);


	/*
	 * Play with the mutex...
	 */
	for (cnt = 0; cnt < CYCLE_COUNT; cnt++) {
		semaphore_down (& mtx, 1);
		semaphore_up (& mtx, 1);
	}


	// clean up
	semaphore_destroy (& mtx);

	// print the result
	printk ("Test passed...\n");
}

