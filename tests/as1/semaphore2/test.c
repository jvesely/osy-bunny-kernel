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
	"Semaphore test #2\n"
	"Spawns a number of threads each incrementing a counter.\n"
	"The atomicity of the increments is assured by a critical\n"
	"section guarded by a mutex.\n\n";


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
 * The global counter and associated mutex.
 */
static volatile int	counter;
static semaphore_t	counter_mtx;

/*
 * Number of running threads and associated mutex.
 */
static volatile int	threads_running;
static semaphore_t	threads_running_mtx;


static void *
thread_proc (void * data)
{
	int cnt;
	int local_counter;

	assert (data == THREAD_MAGIC);


	for (cnt = 0; cnt < CYCLE_COUNT; cnt++) {
		semaphore_down (& counter_mtx, 1);

		/*
		 * Get a copy of the counter, wait a while and store an
		 * increment of the local counter. If the mutexes don't work,
		 * the counter will be damaged.
		 */
		local_counter = counter;
		thread_usleep (SLEEP_TIME_MS * 1000);
		counter = local_counter + 1;

		semaphore_up (& counter_mtx, 1);
	}

	semaphore_down(&threads_running_mtx, 1);
	threads_running -= 1;
	semaphore_up(&threads_running_mtx, 1);
	
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

	semaphore_init (& counter_mtx, 1);
	semaphore_init (& threads_running_mtx, 1);


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
	cond_wait_semaphore_verbose (
		threads_running > 0, threads_running_mtx, threads_running);


	// clean up
	for (cnt = 0; cnt < THREAD_COUNT; cnt++) {
		robust_thread_join (threads [cnt]);
	}

	semaphore_destroy (& threads_running_mtx);
	semaphore_destroy (& counter_mtx);

	// print the result
	if (counter == THREAD_COUNT * CYCLE_COUNT) {
		printk ("Test passed...\n");
	} else {
		printk ("Counter value (%d) is wrong. Should be %d.\n"
			"Test failed...\n", counter,
			THREAD_COUNT * CYCLE_COUNT);
	}
}
