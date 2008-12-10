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
#include "mem/FrameAllocator.h"

#include "structures/Trees.h"

extern "C" void* test1(void*);
extern "C" void* test2(void*);

template class Tree<SplayBinaryNode<int> >;

void* test(void*)
{

	#ifdef KERNEL_TEST
		run_test();
		return NULL;
	#endif

//	dprintf("Pausing execution");
//	Processor::msim_stop();

	thread_t thread1;
//	thread_t thread2;
//	thread_t thread3;
	thread_create(&thread1, test1, NULL, 0);
//	thread_create(&thread2, test1, NULL, 0);
//	thread_create(&thread3, test1, NULL, 0);
	thread_detach(thread_get_current());
	thread_join_timeout( thread1, 1000000 );
	printf("After join timeout.\n");
	thread_kill( thread1 );
	printf("After kill.\n");
	thread_join( thread1 );

	// fralloc tests

	//printf("Frame allocator testing...\n");

	//MyFrameAllocator::instance().test();


	return NULL;
	thread_yield();
	while (true) {
		thread_wakeup( thread1 );
		thread_sleep(1);
	}

	return NULL;
// */
	while (true) {
/*		char c = getc();
		printf("Hello: %c\n",c);
		continue;
			printf("Sleeping. %x %d %u\n", 0x4916b35f, 0x4916b35f, 0x4916b35f);
			thread_sleep(8);
			printf("done: %x\n",Time::getCurrent().secs());
			continue; // */
/*
		thread_t thread;

//		printf("My thread num is %d\n", thread_get_current());
//		return NULL;
		if (thread_create(&thread, test1, NULL, 0) == EOK) {
			thread_detach(thread);

//			printf("Before kill\n");
//			thread_kill(thread);
//			printf("After kill.\n");
			int res = thread_join(thread);
			printf("Thread join: %d\n", res);
//			assert(res == EINVAL);
			thread_yield();
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
	thread_sleep(1);

	//while (true){
		printf("Test...%d\n", thread_get_current());
//		thread_sleep(1);
//		thread_usleep(1000000);
//		thread_suspend();
//	}
	return NULL;
}

