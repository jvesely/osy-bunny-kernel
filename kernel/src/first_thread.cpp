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
#include "SysCall.h"
#include "proc/UserThread.h"
#include "Kernel.h"
#include "tarfs/TarFS.h"

void* first_thread(void* data)
{
	#ifdef KERNEL_TEST
		//KERNEL.stop();
		run_test();
		KERNEL.halt();
	#endif
	
	#ifdef USER_TEST
		const char* file = "test.bin";
	#else
		const char* file = "init.bin";
	#endif

	printf( "Mounting root fs..." );
	TarFS* fs = new TarFS();

	if (fs && fs->mount( KERNEL.disk() )) {
		KERNEL.m_rootFS = fs;
		printf( "done\n" );
	} else {
		printf( "Disk mounting failed. Shutting down.\n" );
		KERNEL.halt();
	}
	
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

	const char* foo = "FOO";
	SysCall::puts( foo );
	KERNEL.stop();
	return NULL;
}
