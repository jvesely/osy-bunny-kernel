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

size_t gets_feedback(char* buffer, size_t buffer_size)
{
	uint ptr = 0;
	char c;
	while (( (c = getc()) != '\n') && ptr < (buffer_size) - 1 )
	{
		if (c == 127) {
			if (ptr > 0) {
				--ptr;
				puts( "\b \b" );
			}
		} else {
			buffer[ptr++] = c;
			putc( c );
		}
	}
	buffer[ptr] = '\0';
	return ptr;
}
Process* exec( const char* file, VFS* fs )
{
	file_t first_proc = fs->openFile( file, OPEN_R );
	
	if (first_proc < 0) {
		printf("Open file failed.\n");
		return NULL;
	}

	const size_t file_size = fs->seekFile( first_proc, POS_END, 0 );
	fs->seekFile( first_proc, POS_START, 0 );

	char* image = (char*)malloc(file_size);
	fs->readFile( first_proc, image, file_size );

	Process* main_proc = Process::create( image, file_size );
	free(image);

	if (!main_proc) {
		printf( "Failed to launch process: %s.\n", file );
		return NULL;
	}
	return main_proc;
}

void* first_thread(void* data)
{
	#ifdef KERNEL_TEST
		thread_t thr;
		Thread* thread = KernelThread::create( &thr, (void*(*)(void*))run_test );
		Thread::getCurrent()->join( thread, NULL );
		KERNEL.halt();
	#endif
/*	
	#ifdef USER_TEST
		const char* file = "test.bin";
	#else
		const char* file = "test.bin";
	#endif
*/
	puts( "Mounting root fs..." );
	TarFS* fs = new TarFS();


	if (fs && fs->mount( KERNEL.disk() )) {
		KERNEL.m_rootFS = fs;
		puts( "done\n" );
	} else {
		puts( "Disk mounting failed. Shutting down.\n" );
		KERNEL.halt();
	}
	
	bool exit = false;
	const int BUFFER_SIZE(50);
	char buffer[BUFFER_SIZE];
		
	while (!exit) {
	{
	printf ("Select program to run:\n");
	String name = fs->rootDir()->firstEntry();
	while (! name.empty()) {
		printf ("%s ", name.cstr());
		name = fs->rootDir()->nextEntry( name );
	}

	printf("\n or type halt to shut down.\n");
	}
		printf("# ");

		size_t read = gets_feedback(buffer, BUFFER_SIZE);
		putc('\n');
		if (read == 0){
			printf("Read error halting...\n");
			KERNEL.halt();
		}
		if (String(buffer) == String("halt")) {
			printf("Shutting down");
			exit = true; break;
		}
		printf("Running: %s.\n", buffer);
		Process* proc = exec( buffer, fs );
		if (proc) {
			Thread::getCurrent()->join( proc->mainThread(), NULL );
		}
	}

	KERNEL.halt();	

	return NULL;
}
