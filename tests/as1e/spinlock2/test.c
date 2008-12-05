/***
 * Spinlock test #2
 *
 * Change Log:
 * 2004/10/25 pallas	updated for new assignment specification
 */

static char *	desc =
	"Spinlock test #2\n"
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
 * The global counter and associated spinlock.
 */
static volatile int	counter;
static struct spinlock	counter_lock;

/*
 * Number of running threads and associated spinlock.
 */
static volatile int	threads_running;
static struct spinlock	threads_running_lock;


/*
 * Simple active sleep.
 */
static void
busy_wait (void)
{
	unsigned int i;
	
	i = 0;
	while ((i >> 12) == 0) {
		i++;
	}
}


static void *
thread_proc (void * data)
{
	int cnt;
	int local_counter;

	assert (data == THREAD_MAGIC);


	for (cnt = 0; cnt < CYCLE_COUNT; cnt++) {
		spinlock_lock (& counter_lock);

		/*
		 * Get a copy of the counter, wait a while and store an
		 * increment of the local counter. If the spinlocks don't work,
		 * the counter will be damaged.
		 */
		local_counter = counter;
		busy_wait ();
		counter = local_counter + 1;

		spinlock_unlock (& counter_lock);
	}


	dec_var_spinlock (threads_running, threads_running_lock);

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

	spinlock_init (& counter_lock);
	spinlock_init (& threads_running_lock);


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
		threads_running > 0, threads_running_lock, threads_running);


	// clean up
	for (cnt = 0; cnt < THREAD_COUNT; cnt++) {
		robust_thread_join (threads [cnt]);
	}

	spinlock_destroy (& threads_running_lock);
	spinlock_destroy (& counter_lock);

	// print the result
	if (counter == THREAD_COUNT * CYCLE_COUNT) {
		printk ("Test passed...\n");
	} else {
		printk ("Counter value (%d) is wrong. Should be %d.\n"
			"Test failed...\n", counter,
			THREAD_COUNT * CYCLE_COUNT);
	}
}
