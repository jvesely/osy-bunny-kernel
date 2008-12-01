/***
* Thread copy #1
*
* Change Log:
* 03/11/25 ghort	created
* 03/12/04 vilda	bug fixes, closer to ANSI C, polishing
* 04/11/29 pallas	updated to reflect the new assignment spec
*/

static char *	desc =
	"Thread copy #1\n\n"
	"Creates a new thread with clean virtual memory map. Waits until the\n"
	"spawned thread notifies the parent by writing a magic value to its\n"
	"mapped memory.\n\n";


#include <api.h>
#include "../include/defs.h"

#define MAGIC 		(void*) 0x863bc982
#define EMPTY 		0xdeadbeef
#define SIGNAL1 	0xfacefeed
#define SIGNAL2 	0xfacebead
#define DATA1 		0xcafebabe

#define MAP_ADDR	0x0


static thread_t		child;
static thread_t		parent;
static volatile int	error_flag;


static void *
thr_proc (void *data)
{
	int result;
	volatile unsigned int *mapped_area;

	if (data != MAGIC) {
		printk ("Error: Thread data do not match.\n");
		error_flag = 1;
		return NULL;
	}

	mapped_area = MAP_ADDR;
	result = vma_alloc ((void **) & mapped_area, PAGE_SIZE, VF_USER_ADDR);
	if (result != EOK) {
		printk ("Error: Failed to allocate memory to perform the "
			"test.\n");
		error_flag = 1;
		return NULL;
	}

	/*
	 * Make sure we have different memory than the parent...
	 */
	if (*mapped_area == EMPTY) {
		printk ("Error: Child thread has access to parent thread's "
			"private memory.\n");
		error_flag = 1;
		return NULL;
	}


	/*
	 * Send SIGNAL1 to parent thread.
	 */
	*mapped_area = SIGNAL1;
	printk( "<2> Sending signal 1\n");
	copy_to_thread (parent, MAP_ADDR, MAP_ADDR, sizeof(unsigned int));

	/*
	 * Receive SIGNAL2 from parent thread.
	 */
	while (*mapped_area != SIGNAL2) {
		thread_usleep (1000);
	}

	printk( "<2> Received signal 2\n");

	/*
	 * Read data from parent.
	 */
	copy_from_thread (parent, MAP_ADDR, MAP_ADDR, sizeof(unsigned int));
	if (*mapped_area != DATA1) {
		printk("Error: received data do not match.\n");
		error_flag = 1;
		return NULL;
	}

	/*
	 * Send SIGNAL2 to parent thread.
	 */
	*mapped_area = SIGNAL2;
	copy_to_thread(parent, MAP_ADDR, MAP_ADDR, sizeof(unsigned int));

	return NULL;
}


void
run_test (void)
{
	int result;
	unsigned int sig;
	volatile unsigned int *mapped_area;

	printk (desc);

	// init
	error_flag = 0;

	mapped_area = MAP_ADDR;
	result = vma_alloc ((void **) & mapped_area, PAGE_SIZE, VF_USER_ADDR);
	if (result != EOK) {
		printk ("Failed to allocate memory for the test.\n");
		return;
	}

	*mapped_area = EMPTY;

	/*
	 * Create a new thread with empty memory map.
	 */
	parent = thread_get_current();
	result = thread_create (& child, thr_proc, MAGIC, TF_NEW_VMM);
	if (result != EOK) {
		panic ("Test failed...\n"
		       "Unable to create thread (%d).\n", result);
		return ;
	}

	/*
	 * Wait for SIGNAL1 from child thread.
	 */
	while (*mapped_area != SIGNAL1 && error_flag == 0) {
		thread_usleep (1000);
	}

	if (error_flag)
		return;

	printk( "<1> Received signal 1\n");

	/*
	 * Send SIGNAL2 to child thread.
	 */
	*mapped_area = DATA1;
	sig = SIGNAL2;
	printk( "<1> Sending signal 2\n");
	copy_to_thread (child, MAP_ADDR, &sig, sizeof (unsigned int));


	/*
	 * Wait for SIGNAL2 from child thread.
	 */
	while (*mapped_area != SIGNAL2 && error_flag == 0) {
		thread_usleep (1000);
	}

	if (error_flag)
		return ;

	thread_join (child);
	printk("Test passed...\n");
}

