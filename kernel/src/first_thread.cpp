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

	Process* main_proc = Process::create( file );
	if (main_proc) {
		printf( "Launching process: %s.\n", file );
		Thread::getCurrent()->join( main_proc->mainThread(), NULL );
	} else {
		printf( "Failed to launch process: %s.\n", file );
	}
	KERNEL.halt();	


	file_t bin_file = fs->openFile( file, OPEN_R );
	if (bin_file < 0) {
		printf("Failed to open file: %s.\n", file);
		KERNEL.halt();
	}
	size_t filesize = fs->sizeFile( bin_file );
	printf( "Reading from file %s of size %u.\n", file, filesize );
	char* content = (char*)malloc( filesize + 1 );
	content[filesize] = 0;
	fs->readFile( bin_file, content, filesize );
	printf( "Contents: %s.\n", content );

	KERNEL.halt();
	return 0;
}
