/***
 * Semaphore test #3
 *
 * Change Log:
 * 2003/10/22 ghort	created
 * 2004/10/25 pallas	updated for new assignment specification
 */

static char *	desc =
	"Semaphore test #3\n"
	"Tests whether the timeout version of semaphores really\n"
	"does not block.\n\n";


#include <api.h>
#include "../include/defs.h"


/*
 * Length of down timeout on semaphore.
 */
#define SEM_TIMEOUT_MS	2000


void
run_test (void)
{
	int 			success;
	int			result;
	struct semaphore	sem;

	printk (desc);

	// init
	success = 1;
	sem_init (& sem, 0);


	/*
	 * Test the timeouts.
	 */
	printk ("Try down on semaphore for (approx.) 2 seconds...\n");
	result = sem_down_timeout (& sem, SEM_TIMEOUT_MS * 1000);
	success = (result == ETIMEDOUT) ? success : 0;

	printk ("Try down on semaphore for (approx.) 4 seconds...\n");
	result = sem_down_timeout (& sem, 2 * SEM_TIMEOUT_MS * 1000);
	success = (result == ETIMEDOUT) ? success : 0;


	// clean up
	sem_destroy (& sem);

	// print the result
	if (success) {
		printk ("Test passed...\n");
	} else {
		printk ("Some of the operations did not return ETIMEDOUT.\n"
			"Test failed.\n");
	}
}
