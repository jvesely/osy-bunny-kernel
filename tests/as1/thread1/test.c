/***
 * Thread test #1
 *
 * Change Log:
 * 2003/10/22 ghort	created
 * 2003/11/03 pallas	cleaned up and integrated
 * 2003/11/07 pallas	converted to robust_thread_create
 * 2004/10/25 pallas	updated for new assignment specification
 */

static char *	desc =
	"Thread test #1\n"
	"Basic test of thread facilities such as create,"
	"sleep and join.\n\n";


#include <api.h>
#include "../include/defs.h"


/*
 * Data for the threads.
 */
#define THREAD_A_DATA	((void*) 0xcafebabe)
#define THREAD_B_DATA	((void*) 0xfacebead)
#define THREAD_C_DATA	((void*) 0xdeadbeef)

/*
 * Join timeout in milliseconds.
 */
#define JOIN_TIMEOUT_MS	1000


static void *
thread_proc_a (void * data)
{
	printk ("Thread A: thread_t=%p data=%p\n",
		thread_get_current (), data);

	printk ("Thread A: sleeping for 1 second\n");
	thread_sleep (1);

	printk ("Thread A: sleeping for 2 seconds\n");
	thread_sleep (2);

	printk ("Thread A: sleeping for 3 seconds\n");
	thread_sleep (3);

	printk ("Thread A: sleeping for 1 microsecond\n");
	thread_usleep (1);

	printk ("Thread A: sleeping for 1000 microseconds\n");
	thread_usleep (1000);

	printk ("Thread A: sleeping for 1000000 microseconds\n");
	thread_usleep (1000000);

	printk ("Thread A: sleeping again for 1 second\n");
	thread_sleep (1);

	printk ("Thread A: exitting\n");
	
	return NULL;
} /* thread_proc_a */

static void *
thread_proc_c (void * data)
{
	int cnt;

	printk ("Thread C: thread_t=%p data=%p\n",
		thread_get_current (), data);

	for (cnt = 0; cnt < 5; cnt++) {
		printk ("Thread C: 1 second tick\n");
		thread_sleep (1);
	}

	printk ("Thread C: exitting\n");
	
	return NULL;
} /* thread_proc_c */

static void *
thread_proc_b (void * data)
{
	thread_t thread_c;

	printk ("Thread B: thread_t=%p data=%p\n",
		thread_get_current (), data);

	printk ("Thread B: waiting 2 seconds before starting thread C\n");
	thread_sleep (2);

	thread_c = robust_thread_create (thread_proc_c, THREAD_C_DATA);
	printk ("Thread B: started thread C, thread_t=%p data=%p\n",
		thread_c, THREAD_C_DATA);

	printk ("Thread B: joining thread C\n");
	robust_thread_join (thread_c);

	printk ("Thread B: exitting\n");
	
	return NULL;
} /* thread_proc_b */


void
run_test (void)
{
	thread_t	thread_a;
	thread_t	thread_b;
	int		thread_a_active;
	int		thread_b_active;

	printk (desc);


	/*
	 * Create the threads.
	 */
	thread_a = robust_thread_create (thread_proc_a, THREAD_A_DATA);
	printk ("Started thread A: thread_t=%p data=%p\n",
		thread_a, THREAD_A_DATA);
	thread_a_active = 1;

	thread_b = robust_thread_create (thread_proc_b, THREAD_B_DATA);
	printk ("Started thread B: thread_t=%p data=%p\n",
		thread_b, THREAD_B_DATA);
	thread_b_active = 1;

	/*
	 * Try joining the threads.
	 */
	while (thread_a_active || thread_b_active) {
		int result;

		if (thread_a_active) {
			result = thread_join_timeout (thread_a,
				JOIN_TIMEOUT_MS * 1000);
			if (result == EOK) {
				// we joined the thread
				thread_a_active = 0;
			}
		}

		if (thread_b_active) {
			result = thread_join_timeout (thread_b, 0);
			if (result == EOK) {
				// we joined the thread
				thread_b_active = 0;
			}
		}

		printk ("*");
	}

	printk ("\n");


	// print the result
	printk ("Test passed...\n");
}
