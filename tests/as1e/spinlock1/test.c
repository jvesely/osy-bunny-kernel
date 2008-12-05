/***
 * Spinlock test #1
 *
 * Change Log:
 * 2004/10/25 pallas	updated for new assignment specification
 */

static char *	desc =
	"Spinlock test #1\n"
	"Creates a spinlock, which is then repeatedly locked and\n"
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
	struct spinlock	lock;

	printk (desc);

	// initialize
	spinlock_init (& lock);


	/*
	 * Play with the spinlock...
	 */
	for (cnt = 0; cnt < CYCLE_COUNT; cnt++) {
		spinlock_lock (& lock);
		spinlock_unlock (& lock);
	}


	// clean up
	spinlock_destroy (& lock);

	// print the result
	printk ("Test passed...\n");
}

