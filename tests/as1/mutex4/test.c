/***
 * Mutex test #4
 *
 * Change Log:
 * 2003/10/20 ghort	created
 * 2003/11/02 pallas	cleaned up and integrated
 * 2003/11/07 pallas	converted to robust_thread_create
 * 2004/10/25 pallas	updated for new assignment specification
 */

static char *	desc =
	"Mutex test #4\n"
	"Creates a mutex and locks it in one thread and unlocks it\n"
	"in another. Since DEBUG_MUTEX is not set, no errors should "
	"be reported.\n\n";


#include <api.h>
#include "../include/defs.h"


/*
 * The tested mutex.
 */
static struct mutex	mtx;

/*
 * Global termination flag.
 */
static volatile int	finish_flag;


static void *
thread_proc (void * data)
{
	assert (data == THREAD_MAGIC);

	mutex_unlock (& mtx);
	finish_flag = 1;
	
	return NULL;
}


void
run_test (void)
{
	thread_t thread;

	printk (desc);

	// init
	finish_flag = 0;

	mutex_init (& mtx);
	mutex_lock (& mtx);


	// start the thread and wait until the mutex is unlocked
	thread = robust_thread_create (thread_proc, THREAD_MAGIC);

	while (! finish_flag) {
		thread_sleep (1);
	}

	robust_thread_join (thread);

	// clean up
	mutex_destroy (& mtx);

	printk ("Test passed...\n");
}

