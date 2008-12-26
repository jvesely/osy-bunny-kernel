/***
 * Thread test #2
 *
 * Change Log:
 * 2003/10/22 ghort	created
 * 2003/11/03 ghort	cleaned up and integrated
 * 2003/11/03 pallas	more cleanups
 * 2003/11/07 pallas	converted to robust_thread_create
 * 2004/10/25 pallas	updated for new assignment specification
 * 2005/01/13 pallas	updated for user space API
 * 2005/12/19 pallas	brought up-to-date with user space API
 */

static char * desc =
	"Basic test of thread facilities such as suspend, \n"
	"wakeup, join, detach, and kill.\n";


#include <librt.h>
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

	thread = thread_self ();
	printf ("Thread %x started...\n", thread);

	printf ("Thread %x suspending...\n", thread);
	thread_suspend ();

	printf ("Thread %x woke up...\n", thread);

	do {
		printf (".");
		thread_usleep (PRINT_DELAY_MS * 1000);
	} while (1);
	
	return NULL;
}

/*
 * Intermediate function to make robust_* definitions that return pointers work
 */
static char *
main_thread (void)
{
	int		result;
	thread_t	thread;


	/*
	 * Start the thread.
	 */
	printf ("Starting thread...\n");
	thread = robust_thread_create (thread_proc, NULL);

	thread_usleep (STEP_DELAY_MS * 1000);

	/*
	 * Wake up the thread.
	 */
	printf ("Waking up thread...\n");
	robust_thread_wakeup (thread);

	thread_usleep (STEP_DELAY_MS * 1000);

	/*
	 * Detach the thread.
	 */
	printf ("Detaching thread...\n");
	robust_thread_detach (thread);

	thread_usleep (STEP_DELAY_MS * 1000);

	/*
	 * Try to join the thread.
	 */
	printf ("Trying to join the thread...\n");
	result = thread_join (thread, NULL);
	if (result != EINVAL)
		return "Should not be able to join detached thread";

	thread_usleep (STEP_DELAY_MS * 1000);

	/*
	 * Try to detach the thread again.
	 */
	printf ("Trying to detach the thread...\n");
	result = thread_detach (thread);
	if (result != EINVAL)
		return "Should not be able to detach already detached thread";

	thread_usleep (STEP_DELAY_MS * 1000);

	/*
	 * Kill the thread.
	 */
	printf ("Cancelling thread...\n");
	robust_thread_cancel (thread);

	thread_usleep (STEP_DELAY_MS * 1000);
	
	return NULL;
}

int
main (void)
{
	printf (desc);

	char * ret = main_thread();

	// print the result
	if (ret == NULL) {
		printf("\nTest passed...\n\n");
		return 0;
	} else {
		printf("\n%s\n\n", ret);
		return 1;
	}
}
