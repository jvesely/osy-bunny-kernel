/*
 *          _     _
 *          \`\ /`/
 *           \ V /
 *           /. .\            Bunny Kernel for MIPS
 *          =\ T /=
 *           / ^ \
 *        {}/\\ //\
 *        __\ " " /__
 *   jgs (____/^\____)
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
/*! 	 
 *   @author Matus Dekanek, Tomas Petrusek, Lubos Slovak, Jan Vesely
 *   @par "SVN Repository"
 *   svn://aiya.ms.mff.cuni.cz/osy0809-depeslve
 *   
 *   @version $Id$
 *   @note
 *   Semestral work for Operating Systems course at MFF UK \n
 *   http://dsrg.mff.cuni.cz/~ceres/sch/osy/main.php
 *   
 *   @date 2008-2009
 */

/*!
 * @file 
 * @brief Contains body of the first thread.
 *
 * First "program" run by the kernel.
 */

#include "api.h"
#include "drivers/Processor.h"

extern "C" void* test1(void*);
extern "C" void* test2(void*);

void* test(void*)
{
	#ifdef KERNEL_TEST
		run_test();
		return NULL;
	#endif
//	dprintf("Pausing execution");
//	Processor::msim_stop();
/*  
	thread_t thread1;
	thread_t thread2;
	thread_t thread3;
	thread_create(&thread1, test1, NULL, 0);
	thread_create(&thread2, test1, NULL, 0);
	thread_create(&thread3, test1, NULL, 0);
*/

	while (true) {
//		char c = getc();
//		printf("Hello: %c\n",c);
//		continue;
		thread_t thread;

//		printf("My thread num is %d\n", thread_get_current());
//		return NULL;
		if (thread_create(&thread, test1, NULL, 0) == EOK) {
//			thread_detach(thread);

//			printf("Before kill\n");
			thread_kill(thread);
//			printf("After kill.\n");
			int res = thread_join(thread);
			printf("Thread join: %d\n", res);
//			assert(res == EINVAL);
			printf("Thread again.\n");
//			thread_sleep(10);
//			return NULL;
		} else {
			printf("Thread creation failed.\n");
			assert(false);
			break;
		}// */
	}
	printf("Bailing out.\n");
	return NULL;
}

void* test1(void*)
{
	while (true){
		printf("Test...%d\n", thread_get_current());
		break;
		thread_sleep(1);
//		for (int i = 1; i < 4000000; ++i) ;
	}
	return NULL;
}

void* test2(void*)
{
	while (true)
		printf("Test...\n");
	return NULL;
}
