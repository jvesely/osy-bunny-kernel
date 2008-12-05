/***
 * Condition variable test #3
 *
 * Change Log:
 * 2003/10/22 ghort	created
 * 2003/11/03 ghort	cleaned up and integrated
 * 2004/10/25 pallas	updated for new assignment specification
 */

static char *	desc =
	"Condition variable test #3\n"
	"Tests whether the timeout version of condition variables\n"
	"really does not block.\n\n";


#include <api.h>
#include "../include/defs.h"


/*
 * Length of wait timeout on condition variable.
 */
#define CV_TIMEOUT_MS	2000


void
run_test (void)
{
	struct mutex	mtx;
	struct cond	cond;

	printk (desc);

	// init
	mutex_init (& mtx);
	cond_init (& cond);


	/*
	 * Test the timeouts.
	 */
	mutex_lock (& mtx);

	printk ("Wait on condition variable for (approx.) 2 seconds...\n");
	cond_wait_timeout (& cond, & mtx, CV_TIMEOUT_MS * 1000);

	printk ("Wait on condition variable for (approx.) 4 seconds...\n");
	cond_wait_timeout (& cond, & mtx, 2 * CV_TIMEOUT_MS * 1000);


	// clean up
	cond_destroy (& cond);
	mutex_destroy (& mtx);

	// print the result
	printk ("Test passed...\n");
}
