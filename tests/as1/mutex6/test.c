/***
 * Mutex test #6
 *
 * Change Log:
 * 2003/10/22 ghort	created
 * 2003/11/02 pallas	cleaned up and integrated
 * 2004/10/25 pallas	updated for new assignment specification
 */

static char *	desc =
	"Mutex test #6\n"
	"Tests whether the timeout version of mutexes really "
	"does not block.\n\n";


#include <api.h>
#include "../include/defs.h"


/*
 * Length of lock timeout on mutex.
 */
#define MTX_TIMEOUT_MS	2000


void
run_test (void)
{
	int 		success;
	int		result;
	struct mutex	mtx;

	printk (desc);

	// init
	success = 1;
	mutex_init (& mtx);
	mutex_lock (& mtx);


	/*
	 * Test the timeouts.
	 */
	printk ("Try to lock mutex for (approx.) 2 seconds...\n");
	result = mutex_lock_timeout (& mtx, MTX_TIMEOUT_MS * 1000);
	success = (result == ETIMEDOUT) ? success : 0;

	printk ("Try to lock mutex for (approx.) 4 seconds...\n");
	result = mutex_lock_timeout (& mtx, 2 * MTX_TIMEOUT_MS * 1000);
	success = (result == ETIMEDOUT) ? success : 0;


	// clean up
	mutex_unlock (& mtx);
	mutex_destroy (& mtx);

	// print the result
	if (success)
		printk ("Test passed...\n");
	else {
		printk ("Some of the operations did not return ETIMEDOUT.\n"
			"Test failed.\n");
	}
}
