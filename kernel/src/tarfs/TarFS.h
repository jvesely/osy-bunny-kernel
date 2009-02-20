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
#include "structures/HashMap.h"
#include "DirEntry.h"

/*!
 * @class TarFS TarFS.h "tarfs/TarFS.h"
 * @brief VFS implementation using TAR format.
 *
 * Class provides only RO interface. No write, no unmount, no create, ...
 */
class TarFS
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
	Entry* getFile( const char file_name[] );

	/*!
	 * @brief Gets Entry of the root directory
	 */
	DirEntry* rootDir()
		{ return &m_rootDir; };

private:
	DirEntry m_rootDir;        /*!< Top of the directory tree. */
	DiskDevice* m_mountedDisk; /*!< DiskDevice beeing used.    */
};
