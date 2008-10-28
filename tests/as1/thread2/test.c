/***
 * Thread test #2
 *
 * Change Log:
 * 2003/10/22 ghort	created
 * 2003/11/03 ghort	cleaned up and integrated
 * 2003/11/03 pallas	more cleanups
 * 2003/11/07 pallas	converted to robust_thread_create
 * 2004/10/25 pallas	updated for new assignment specification
 */

static char *	desc =
	"Thread test #2\n"
	"Basic test of thread facilities such as suspend, \n"
	"wakeup, join, detach, and kill.\n\n";


#include <api.h>
#include "../include/defs.h"


/*
 * The delay between printing characters in thread.
 */
#define PRINT_DELAY_MS	500

/*
 * The delay between individual steps.
 */
#define STEP_DELAY_MS	1500


static void *
thread_proc (void *data)
{
	thread_t thread;

	thread = thread_get_current ();
	printk ("Thread %p started...\n", thread);

	printk ("Thread %p suspending...\n", thread);
	thread_suspend ();

	printk ("Thread %p woke up...\n", thread);

	do {
		printk (".");
		thread_usleep (PRINT_DELAY_MS * 1000);
	} while (1);
	
	return NULL;
}


void
run_test (void)
{
	int		result;
	thread_t	thread;

	printk (desc);


	/*
	 * Start the thread.
	 */
	printk ("Starting thread...\n");
	thread = robust_thread_create (thread_proc, NULL);

	thread_usleep (STEP_DELAY_MS * 1000);

	/*
	 * Wake up the thread.
	 */
	printk ("Waking up thread...\n");
	robust_thread_wakeup (thread);

	thread_usleep (STEP_DELAY_MS * 1000);

	/*
	 * Detach the thread.
	 */
	printk ("Detaching thread...\n");
	robust_thread_detach (thread);

	thread_usleep (STEP_DELAY_MS * 1000);

	/*
	 * Try to join the thread.
	 */
	printk ("Trying to join the thread...\n");
	result = thread_join (thread);
	if (result != EINVAL) {
		printk ("Should not be able to join detached thread.\n"
			"Test failed\n");
		return;
	}

	thread_usleep (STEP_DELAY_MS * 1000);

	/*
	 * Try to detach the thread again.
	 */
	printk ("Trying to detach the thread...\n");
	result = thread_detach (thread);
	if (result != EINVAL) {
		printk ("Should not be able to detach already detached thread.\n"
			"Test failed\n");
		return;
	}

	thread_usleep (STEP_DELAY_MS * 1000);

	/*
	 * Kill the thread.
	 */
	printk ("Killing thread...\n");
	robust_thread_kill (thread);

	thread_usleep (STEP_DELAY_MS * 1000);


	// print the result
	printk ("Test passed...\n");
}
