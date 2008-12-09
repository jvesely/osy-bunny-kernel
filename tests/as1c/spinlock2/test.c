/***
 * Mutex test #2
 *
 * Change Log:
 * 2003/10/22 ghort	created
 * 2003/11/02 pallas	cleaned up and integrated
 * 2003/11/07 pallas	converted to robust_thread_create, cond_wait_mutex
 * 2004/10/25 pallas	updated for new assignment specification
 */

static char *	desc =
	"spinlock test #2\n"
	"Spawns a number of threads each incrementing a counter.\n"
	"The atomicity of the increments is assured by a critical\n"
	"section guarded by a spinlock.\n\n";


#include <api.h>
#include "../include/defs.h"


/*
 * The number of cycles we do in this test.
 */
#define CYCLE_COUNT	(TASK_SIZE)

/*
 * The number of threads to use concurrently.
 */
#define THREAD_COUNT	(TASK_SIZE * 10)

/*
 * The length of sleep in milliseconds.
 */
#define	SLEEP_TIME_MS	10


/*
 * The global counter and associated spinlock.
 */
static volatile int	counter;
static spinlock_t	counter_mtx;

/*
 * Number of running threads and associated spinlock.
 */
static volatile int	threads_running;
static spinlock_t	threads_running_mtx;


static void *
thread_proc (void * data)
{
	int cnt;
	int local_counter;

	assert (data == THREAD_MAGIC);


	for (cnt = 0; cnt < CYCLE_COUNT; cnt++) {
		spinlock_lock (& counter_mtx);

		/*
		 * Get a copy of the counter, wait a while and store an
		 * increment of the local counter. If the mutexes don't work,
		 * the counter will be damaged.
		 */
		local_counter = counter;
		thread_usleep (SLEEP_TIME_MS * 1000);
		counter = local_counter + 1;

		spinlock_unlock (& counter_mtx);
	}

	dec_var_spinlock (threads_running, threads_running_mtx);
	
	return NULL;
}


void
run_test (void)
{
	int		cnt;
	thread_t	threads [THREAD_COUNT];

	printk (desc);

	// init
	counter = 0;
	threads_running = THREAD_COUNT;

	spinlock_init (& counter_mtx);
	spinlock_init (& threads_running_mtx);


	/*
	 * Start the threads.
	 */
	for (cnt = 0; cnt < THREAD_COUNT; cnt++) {
		threads [cnt] = robust_thread_create (
			thread_proc, THREAD_MAGIC);
	}

	/*
	 * Wait for all the threads to finish.
	 */
	printk ("Waiting for all the threads to finish.\n");
	cond_wait_spinlock_verbose (
		threads_running > 0, threads_running_mtx, threads_running);


	// clean up
	for (cnt = 0; cnt < THREAD_COUNT; cnt++) {
		robust_thread_join (threads [cnt]);
	}

	spinlock_destroy (& threads_running_mtx);
	spinlock_destroy (& counter_mtx);

	// print the result
	if (counter == THREAD_COUNT * CYCLE_COUNT) {
		printk ("Test passed...\n");
	} else {
		printk ("Counter value (%d) is wrong. Should be %d.\n"
			"Test failed...\n", counter,
			THREAD_COUNT * CYCLE_COUNT);
	}
}
