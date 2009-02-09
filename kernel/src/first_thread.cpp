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
#include "proc/UserThread.h"
#include "Kernel.h"
#include "tarfs/TarFS.h"
#include "proc/Process.h"

void* first_thread(void* data)
{
	#ifdef KERNEL_TEST
		thread_t thr;
		Thread* thread = KernelThread::create( &thr, (void*(*)(void*))run_test );
		Thread::getCurrent()->join( thread, NULL );
		KERNEL.halt();
	#endif
	
	#ifdef USER_TEST
		const char* file = "test.bin";
	#else
		const char* file = "test.bin";
	#endif

	puts( "Mounting root fs..." );
	TarFS* fs = new TarFS();


	if (fs && fs->mount( KERNEL.disk() )) {
		KERNEL.m_rootFS = fs;
		puts( "done\n" );
	} else {
		puts( "Disk mounting failed. Shutting down.\n" );
		KERNEL.halt();
	}

	file_t first_proc = fs->openFile( file, OPEN_R );
	
	if (first_proc < 0) {
		printf("Open file failed.\n");
		KERNEL.halt();
	}

	const size_t file_size = fs->sizeFile( first_proc );
	char* image = (char*)malloc(file_size);
	fs->readFile( first_proc, image, file_size );

	Process* main_proc = Process::create( image, file_size );
	free(image);

	if (main_proc) {
		printf( "Joining process: %s.\n", file );
		Thread::getCurrent()->join( main_proc->mainThread(), NULL );
	} else {
		printf( "Failed to launch process: %s.\n", file );
	}
	KERNEL.halt();	

	return NULL;
}
