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
 * @brief Short description.
 *
 * Long description. I would paste some Loren Ipsum rubbish here, but I'm afraid
 * It would stay that way. Not that this comment is by any means ingenious but 
 * at least people can understand it. 
 */

#include "TarFS.h"
#include "TarHeader.h"
#include "drivers/DiskDevice.h"

#ifndef TARFS_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...)\
  puts("[ TARFS DEBUG ]: ");\
	  printf(ARGS);
#endif


TarFS::TarFS( DiskDevice* disk )
{
	mount( disk );
}
/*----------------------------------------------------------------------------*/
bool TarFS::mount( DiskDevice<512>* disk )
{
	if (!disk) return false;

	m_mountedDisk = disk;
	TarHeader header;
	uint block = 0;

	PRINT_DEBUG ("Mounting device.\n");

	do {
		disk->read( (char*)&header, 512, block, 0 );
		PRINT_DEBUG ("Found file: %s.\n", header.fileName);
	} while (header.fileName[0] != '\0');
	
	return true;
}
/*----------------------------------------------------------------------------*/
file_t TarFS::openFile( char file_name[], char mode )
{
	return 0;
}
/*----------------------------------------------------------------------------*/
void TarFS::closeFile( file_t file )
{
}
/*----------------------------------------------------------------------------*/
ssize_t TarFS::readFile( file_t src, void* buffer, size_t size )
{
	return -1;
}
/*----------------------------------------------------------------------------*/
uint TarFS::seekFile( file_t file, FilePos pos, uint offset )
{
	return 0;
}
/*----------------------------------------------------------------------------*/
bool TarFS::existsFile( file_t file )
{
	return false;
}
/*----------------------------------------------------------------------------*/
size_t TarFS::sizeFile( file_t file )
{
	Entry* entry = m_entryMap.at( file );
	if (entry) return 5;   //size here
		else     return 0;
}
/*----------------------------------------------------------------------------*/
bool TarFS::eof( file_t file )
{
	Entry* entry = m_entryMap.at( file );
	if (entry) return true;
		else     return false;
}
