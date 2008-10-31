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

//	thread_t thread1;
	//thread_t thread2;
//	thread_create(&thread1, test1, NULL, 0);
	//thread_create(&thread2, test2, NULL, 0);

//	panic ("foo\n");
	while (true) {
		printf("\nWrite something: ");
		thread_usleep(1000000);
		//char c[150];
		//int ret = gets(c, 150);
		//if (ret >= 0 )
		//	printf("Your string(%d) \"%s\".\n", ret, c);
		//else
		//	printf("Error reading string: %d\n", ret);

//		char c = getc();
//		printf("Hi %c\n", c);
//		thread_sleep(1);
//		panic("foo");
/*		thread_t thread;
		if (thread_create(&thread, test1, NULL, 0) == EOK)
			printf("Thread join: %d\n", thread_join(thread));
		else {
			printf("Thread creation failed.\n");
			break;
		}*/
	}
	return NULL;
}

void* test1(void*)
{
//	while (true)
		printf("Test...%d\n", thread_get_current());
		return NULL;
}

void* test2(void*)
{
	while (true)
		printf("Test2...\n");
}
