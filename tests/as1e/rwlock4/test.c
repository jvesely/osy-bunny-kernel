/***
 * Read/Write lock test #4
 *
 * Change Log:
 * 2003/10/22 ghort	created
 * 2003/11/03 ghort	cleaned up and integrated
 * 2003/11/07 pallas	more cleanups, conversion to robust_thread_create
 * 2004/10/25 pallas	updated for new assignment specification
 */

static char *	desc =
	"Read/Write lock test #4\n"
	"Tests whether the timeout version of read/write locks "
	"really does not block.\n\n";


#include <api.h>
#include "../include/defs.h"


/*
 * The length of lock timeout on rwlock.
 */
#define RWL_TIMEOUT_MS	2000


/*
 * The rwlock to test.
 */
static struct rwlock	rwlock;

/*
 * Global success flag.
 */
static volatile int		success;


static void *
thread_proc (void * data)
{
	int	result;
	assert (data == THREAD_MAGIC);

	printk ("Try to read lock rwlock for (approx.) 2 seconds...\n");
	result = rwlock_read_lock_timeout (& rwlock, RWL_TIMEOUT_MS * 1000);
	success = (result == ETIMEDOUT) ? success : 0;

	printk ("Try to read lock rwlock for (approx.) 4 seconds...\n");
	result = rwlock_read_lock_timeout (& rwlock, 2 * RWL_TIMEOUT_MS * 1000);
	success = (result == ETIMEDOUT) ? success : 0;

	printk ("Try to write lock rwlock for (approx.) 2 seconds...\n");
	result = rwlock_write_lock_timeout (& rwlock, RWL_TIMEOUT_MS * 1000);
	success = (result == ETIMEDOUT) ? success : 0;

	printk ("Try to write lock rwlock for (approx.) 4 seconds...\n");
	result = rwlock_write_lock_timeout (& rwlock, 2 * RWL_TIMEOUT_MS * 1000);
	success = (result == ETIMEDOUT) ? success : 0;

	return NULL;
}


void
run_test (void)
{
	thread_t thread;

	printk (desc);

	// init
	success = 1;
	rwlock_init (& rwlock);


	/*
	 * Test the timeouts.
	 */
	rwlock_write_lock (& rwlock);
	thread = robust_thread_create (thread_proc, THREAD_MAGIC);
	robust_thread_join (thread);
	rwlock_write_unlock (& rwlock);


	// clean up
	rwlock_destroy (& rwlock);

	// print the result
	if (success) {
		printk ("Test passed...\n");
	} else {
		printk ("Some of the operations did not return ETIMEDOUT.\n"
			"Test failed.\n");
	}
}
