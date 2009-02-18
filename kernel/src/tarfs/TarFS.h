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
 * @brief TarFS class declaration.
 *
 * Long description. I would paste some Loren Ipsum rubbish here, but I'm afraid
 * It would stay that way. Not that this comment is by any means ingenious but
 * at least people can understand it.
 */

#pragma once

#include "api.h"
#include "VFS.h"
#include "structures/HashMap.h"
#include "DirEntry.h"

typedef HashMap<file_t, Entry*> EntryMap;

/*!
 * @class TarFS TarFS.h "tarfs/TarFS.h"
 * @brief VFS implementation using TAR format.
 *
 * Class provides only RO interface. No write, no unmount, no create, ...
 */
class TarFS: public VFS
{
public:
	/*!
	 * @brief Uses disk to load and store data (if provided).
	 * @param disk DiskDevice to use.
	 */
	TarFS( DiskDevice* disk = NULL );

	/*!
	 * @brief Builds structures using data on @a disk.
	 * @param disk DiskDevice to use.
	 * @return @a True if structures were sucessfully built, @a false otherwise.
	 */
	bool mount( DiskDevice* disk );

	/*!
	 * @brief Opens file.
	 * @param file_name Name of the file (full path).
	 * @param mode Mode to use.
	 * @return ID (>=0) if the file was opened sucessfully, <0 otherwise.
	 */
	file_t openFile( const char file_name[], const char mode );

	/*!
	 * @brief Closes opened file.
	 * @param file File to close.
	 */
	void closeFile( file_t file );

	/*!
	 * @brief Reads data from the file.
	 * @param src Id of the file to read from.
	 * @param buffer Place to store the read data.
	 * @param size Number of bytes to read.
	 * @return Number of bytes read, <0 indicates error.
	 */
	ssize_t readFile( file_t src, void* buffer, size_t size );
	
	/*!
	 * @brief Change position in the file.
	 * @param file File to change position in.
	 * @param pos Reference point.
	 * @param offset Position as offset from the ref. point.
	 * @return New position in the file.
	 */
	uint seekFile( file_t file, FilePos pos, int offset);

	/*!
	 * @brief Checks for further data in the file.
	 * @param file File to check.
	 * @return @a True if there are NO MORE data, @ false otherwise.
	 */
	bool eof( file_t file );

private:

	/*!
	 * @brief Translation functions from ids to pointers.
	 * @param fd Id to translate.
	 */
	Entry* getFile( file_t fd );

	EntryMap m_entryMap;       /*!< Translation map.           */
	DirEntry m_rootDir;        /*!< Top of the directory tree. */
	DiskDevice* m_mountedDisk; /*!< DiskDevice beeing used.    */
	file_t m_lastDescriptor;   /*!< The last assigned id.      */
};
