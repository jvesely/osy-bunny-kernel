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
#include "syscalls.h"
#include "proc/UserThread.h"

extern "C" void* test1(void*);
extern "C" void* test2(void*);

void* test(void* data)
{
	#ifdef KERNEL_TEST
		run_test();
	#else
		puts( "No test specified !!!\n" );
	#endif

	if (data) {
		char * text = (char*)"FOO";
		printf ("text at %p is %s.\n", text, text);

		SysCalls::puts( "FOO\n" );
	} else {
		thread_t user_t;
		Thread* user = UserThread::create(&user_t, test, (void*)0xff, TF_NEW_VMM);
		printf ("created thread: %p.\n", user);
	}
	return NULL;
}
