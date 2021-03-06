/***
 * Exception handling test #1
 *
 * Change Log:
 * 2003/10/22 ghort	created
 * 2003/11/03 ghort	cleaned up and integrated
 * 2004/10/25 pallas	updated for new assignment specification
 */

static char *	desc =
	"Exception handling test #1\n"
	"Tests whether the kernel handles the AdEL (address error\n"
	"at load/fetch) exception.\n\n";


#include <api.h>
#include "../include/defs.h"


void
run_test (void)
{
	printk (desc);

	/*
	 * Generate AdEL.
	 */
	printk ("Generating address error at load/fetch, the kernel should\n"
		"either panic or kill the offending thread on the spot...\n");

	* ((volatile int *) 0xdeadbeef);


	/* This statement should not be reached */
	printk ("This statement should not have been reached.\n"
		"Test failed...\n");
}
