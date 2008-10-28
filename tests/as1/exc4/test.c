/***
 * Exception handling test #4
 *
 * Change Log:
 * 2003/10/22 ghort	created
 * 2003/11/03 ghort	cleaned up and integrated
 * 2004/10/25 pallas	updated for new assignment specification
 */

static char *	desc =
	"Exception handling test #4\n"
	"Tests whether the kernel ignores BP (breakpoint) exception.\n\n";


#include <api.h>
#include "../include/defs.h"


void
run_test (void)
{
	printk (desc);

	/*
	 * Issue BP instruction.
	 */
	printk ("Issuing break instruction, should be ignored...\n");
	asm (
		"break	10\n"
	);


	/* This statement must be reached */
	printk ("Test passed...\n");
}
