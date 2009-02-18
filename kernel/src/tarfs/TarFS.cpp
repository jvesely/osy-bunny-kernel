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

#include "api.h"
#include "TarFS.h"
#include "TarHeader.h"
#include "FileEntry.h"
#include "drivers/DiskDevice.h"
#include "address.h"
#include "tools.h"

//#define TARFS_DEBUG

#ifndef TARFS_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...)\
  puts("[ TARFS DEBUG ]: ");\
	  printf(ARGS);
#endif

#define TAR_BLOCK_SIZE (sizeof(TarHeader))
#define SIGNED_BITS_MASK (0x7fffffff)


TarFS::TarFS( DiskDevice* disk ): m_lastDescriptor( 0 )
{
	PRINT_DEBUG ("Creating tarfs on disk %p...\n", disk);
	mount( disk );
}
/*----------------------------------------------------------------------------*/
bool TarFS::mount( DiskDevice* disk )
{

	if (!disk || !disk->size()) return false;

	m_mountedDisk = disk;
	TarHeader header;
	uint block = 0;

	PRINT_DEBUG ("Mounting device %p of size %u B.\n", disk, disk->size());

	while (disk->read( &header, TAR_BLOCK_SIZE, block, 0 )) {
		/* if there is no name it might only be the end*/
		if (header.fileName()[0] == '\0') break;

		if (header.fileType() == TarHeader::File) {
				PRINT_DEBUG ("Found file: %s (%d).\n", header.fileName(), header.fileSize() );
				FileEntry* file = new FileEntry( header, block, disk );
				m_rootDir.addSubEntry( header.fileName(), file );
		}

		block += roundUp(header.fileSize(), TAR_BLOCK_SIZE) / TAR_BLOCK_SIZE + 1;

		PRINT_DEBUG ("Next Block: %d.\n", block);
	}
#ifdef TARFS_DEBUG
	PRINT_DEBUG ("Mount end.\n");
	PRINT_DEBUG ("Mounted:\n");
	String str = m_rootDir.firstEntry();
	while (!str.empty()) {
			PRINT_DEBUG ("%s\n", str.cstr());
			str =  m_rootDir.nextEntry( str );
	}
#endif
	return true;
}
/*----------------------------------------------------------------------------*/
file_t TarFS::openFile( const char file_name[], char mode )
{
	Entry* file = m_rootDir.subEntry( file_name );
	PRINT_DEBUG ("Opening file %s(%p) mode: %i.\n", file_name, file, mode);
	if (!file || !file->open( mode )) return EIO;

	file_t fd = ++m_lastDescriptor & SIGNED_BITS_MASK;
	while (m_entryMap.exists( fd )){
		fd = ++m_lastDescriptor & SIGNED_BITS_MASK;
	}

	m_entryMap.insert( fd, file );
	return fd;
}
/*----------------------------------------------------------------------------*/
void TarFS::closeFile( file_t file )
{
	Entry* entry = getFile( file );
	if (entry) entry->close();
}
/*----------------------------------------------------------------------------*/
ssize_t TarFS::readFile( file_t src, void* buffer, size_t size )
{
	Entry* entry = getFile( src );
	PRINT_DEBUG ("Reading file: %u.\n", src);
	if (entry)
		return entry->read( buffer, size );
	return EIO;
}
/*----------------------------------------------------------------------------*/
uint TarFS::seekFile( file_t file, FilePos pos, int offset )
{
	Entry* entry = getFile( file );
	if (entry)
		return entry->seek( pos, offset );
	return EIO;
}
/*----------------------------------------------------------------------------*/
bool TarFS::eof( file_t file )
{
	Entry* entry = m_entryMap.at( file );
	if (entry) return true;
		else     return false;
}
/*----------------------------------------------------------------------------*/
Entry* TarFS::getFile( file_t fd )
{
	return m_entryMap.at( fd );
}
