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
#include "FileEntry.h"
#include "drivers/DiscDevice.h"
#include "address.h"
#include "tools.h"

#define TARFS_DEBUG

#ifndef TARFS_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...)\
  puts("[ TARFS DEBUG ]: ");\
	  printf(ARGS);
#endif

#define TAR_BLOCK_SIZE (sizeof(TarHeader))


TarFS::TarFS( DiscDevice* disk = NULL )
{
	PRINT_DEBUG ("Creating tarfs on disk %p...\n", disk);
	mount( disk );
}
/*----------------------------------------------------------------------------*/
bool TarFS::mount( DiscDevice* disk )
{

	if (!disk) return false;

	m_mountedDisk = disk;
	TarHeader header;
	uint block = 0;

	PRINT_DEBUG ("Mounting device %p of size %u B.\n", disk, disk->size());
	
	while (disk->read( (char*)ADDR_TO_USEG((uint)&header), TAR_BLOCK_SIZE, block, 0 )) {
		/* if there is no name it might only be the end*/
		if (header.fileName()[0] == '\0') break;
		
		if (header.fileType() == TarHeader::File) {
				PRINT_DEBUG ("Found file: %s (%d).\n", header.fileName(), header.fileSize() );
				FileEntry* file = new FileEntry( header, block );
				m_rootDir.addSubEntry( header.fileName(), file );
		}

		block += roundUp(header.fileSize(), TAR_BLOCK_SIZE) / TAR_BLOCK_SIZE + 1;

		PRINT_DEBUG ("Next Block: %d.\n", block);
	}
	PRINT_DEBUG ("Mount end.\n");
	PRINT_DEBUG ("Mounted:\n");
#ifdef TARFS_DEBUG
	String str = m_rootDir.firstEntry();
	while (!str.empty()) {
			PRINT_DEBUG ("%s\n", str.cstr());
			str =  m_rootDir.nextEntry( str );
	}
#endif
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
