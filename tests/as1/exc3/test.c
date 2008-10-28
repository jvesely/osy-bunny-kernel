/***
 * Exception handling test #3
 *
 * Change Log:
 * 2003/10/22 ghort	created
 * 2003/11/03 ghort	cleaned up and integrated
 * 2004/10/25 pallas	updated for new assignment specification
 */

static char *	desc =
	"Exception handling test #3\n"
	"Tests whether the kernel handles the RI (reserved "
	"instruction) exception.\n\n";


#include <api.h>
#include "../include/defs.h"


void
run_test (void)
{
	printk (desc);

	/*
	 * Issue reserved instruction.
	 */
	printk ("Issuing reserved instruction, the kernel should either\n"
		"panic or kill the offending thread on the spot...\n");
	asm (
		".insn\n"
		".word	0x15\n"
	);


	/* This statement should not be reached */
	printk ("This statement should not have been reached.\n"
		"Test failed...\n");
}
