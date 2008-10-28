/***
 * Mutex test #3
 *
 * Change Log:
 * 2003/10/20 ghort	created
 * 2003/11/02 pallas	cleaned up and integrated
 * 2003/11/07 pallas	converted to robust_thread_create, dec_var_mutex, etc.
 * 2004/10/25 pallas	updated for new assignment specification
 */

static char *	desc =
	"Mutex test #3\n"
	"Spawns a number of threads and makes them all sleep on one\n"
	"mutex. Tests both blocking lock and lock with timeout.\n\n";


#include <api.h>
#include "../include/defs.h"


/*
 * The number of seconds to let the threads play with the mutex.
 */
#define TEST_DURATION 	3

/*
 * The number of milliseconds to wait when acquiring the mutex.
 */
#define LOCK_TIMEOUT_MS	100

/*
 * The number of threads of each kind to run concurrently.
 */
#define THREAD_COUNT	(TASK_SIZE * 5)


/*
 * The tested mutex.
 */
static struct mutex	mtx;

/*
 * Global failure indicator.
 */
static volatile int	test_failed;

/*
 * Global termination indicator.
 */
static volatile int	finish_flag;

/*
 * Number of running threads and associated mutex.
 */
static volatile int	threads_running;
static struct mutex	threads_running_mtx;


static void *
thread_proc_blocking (void * data)
{
	assert (data == THREAD_MAGIC);

	mutex_lock (& mtx);

	if (! finish_flag) {
		/*
		 * The mutex released us before it was allowed to.
		 */
		printk ("%s: mutex acquired before it was possible\n",
			__FUNCTION__);
		test_failed = 1;
	}

	mutex_unlock (& mtx);

	dec_var_mutex (threads_running, threads_running_mtx);
	
	return NULL;
}


static void *
thread_proc_timeout (void * data)
{
	assert (data == THREAD_MAGIC);

	while (mutex_lock_timeout (& mtx, LOCK_TIMEOUT_MS * 1000) != EOK);

	if (! finish_flag) {
		/*
		 * We locked the mutex before it was possible.
		 */
		printk ("%s: mutex acquired before it was possible\n",
			__FUNCTION__);
		test_failed = 1;
	}

	mutex_unlock (& mtx);

	dec_var_mutex (threads_running, threads_running_mtx);
	
	return NULL;
}


static void *
thread_proc_nonblocking (void * data)
{
	assert (data == THREAD_MAGIC);

	while (mutex_lock_timeout (& mtx, 0) != EOK) {
		thread_usleep (LOCK_TIMEOUT_MS * 1000);
	}

	if (! finish_flag) {
		/*
		 * We locked the mutex before it was possible.
		 */
		printk ("%s: mutex acquired before it was possible\n",
			__FUNCTION__);
		test_failed = 1;
	}

	mutex_unlock (& mtx);

	dec_var_mutex (threads_running, threads_running_mtx);
	
	return NULL;
}


void
run_test (void)
{
	int 		cnt;
	thread_t	threads [THREAD_COUNT] [3];

	printk (desc);

	// init
	test_failed = 0;
	finish_flag = 0;
	threads_running = THREAD_COUNT * 3;

	mutex_init (& mtx);
	mutex_init (& threads_running_mtx);


	/* 
	 * Lock the mutex and start the threads so they can sleep on it.
	 */
	mutex_lock (& mtx);

	for (cnt = 0; cnt < THREAD_COUNT; cnt++) {
		threads [cnt] [0] = robust_thread_create (
			thread_proc_blocking, THREAD_MAGIC);
		threads [cnt] [1] = robust_thread_create (
			thread_proc_timeout, THREAD_MAGIC);
		threads [cnt] [2] = robust_thread_create (
			thread_proc_nonblocking, THREAD_MAGIC);
	}

	/*
	 * Wait a while so that all the threads can play with the mutex.
	 */
	printk ("Giving the threads %d seconds to play...\n", TEST_DURATION);
	thread_sleep (TEST_DURATION);

	/*
	 * Signal the end of the test and unlock the mutex so the
	 * threads can wake up.
	 */
	finish_flag = 1;
	mutex_unlock (& mtx);

	/*
	 * Wait for all the threads to finish.
	 */
	printk ("Waiting for all the threads to finish...\n");
	cond_wait_mutex_verbose (threads_running > 0, threads_running_mtx,
		threads_running);

	/*
	 * Reap the threads.
	 */
	for (cnt = 0; cnt < THREAD_COUNT; cnt++) {
		robust_thread_join (threads [cnt] [0]);
		robust_thread_join (threads [cnt] [1]);
		robust_thread_join (threads [cnt] [2]);
	}
	
	/* 
	 * Lock/unlock the mutex to verify that it is in correct state.
 	 */
	mutex_lock (& mtx);
	mutex_unlock (& mtx);


	// clean up
	mutex_destroy (& threads_running_mtx);
	mutex_destroy (& mtx);

	// print the result
	if (! test_failed) {
		printk ("Test passed...\n");
	} else {
		printk ("Test failed...\n");
	}
}
