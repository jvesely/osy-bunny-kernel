/***
 * Mutex test #4
 *
 * Change Log:
 * 2003/10/22 ghort	created
 * 2003/11/02 pallas	cleaned up and integrated
 * 2004/10/25 pallas	updated for new assignment specification
 * 2005/01/13 pallas	updated for user space API
 */

static char * desc =
	"Tests whether the timeout version of mutexes really "
	"does not block.\n";


#include <librt.h>
#include "../include/defs.h"


/*
 * Length of lock timeout on mutex.
 */
#define MTX_TIMEOUT_MS	2000


int
main (void)
{
	int 		success;
	int		result;
	struct mutex	mtx;

	printf (desc);

	// init
	success = 1;
	mutex_init (& mtx);
	mutex_lock (& mtx);


	/*
	 * Test the timeouts.
	 */
	printf ("Try to lock mutex for (approx.) 2 seconds...\n");
	result = mutex_lock_timeout (& mtx, MTX_TIMEOUT_MS * 1000);
	success = (result == ETIMEDOUT) ? success : 0;

	printf ("Try to lock mutex for (approx.) 4 seconds...\n");
	result = mutex_lock_timeout (& mtx, 2 * MTX_TIMEOUT_MS * 1000);
	success = (result == ETIMEDOUT) ? success : 0;


	// clean up
	mutex_unlock (& mtx);
	mutex_destroy (& mtx);

	if (!success) {
		printf("Some of the operations did not return ETIMEDOUT\n");
		return 1;
	}
	
	printf("\nTest passed...\n\n");
	return 0;
}
