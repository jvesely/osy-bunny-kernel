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
 * @brief Class FileEntry declaration.
 *
 * Long description. I would paste some Loren Ipsum rubbish here, but I'm afraid
 * It would stay that way. Not that this comment is by any means ingenious but 
 * at least people can understand it. 
 */

#pragma once
#include "TarHeader.h"
#include "Entry.h"

class DiskDevice;

/*!
 * @class FileEntry FileEntry.h "tarfs/FileEntry.h"
 * @brief Class represents files stored on the TarFS.
 *
 * Class provides basic RO interface for accessing files.
 */

class FileEntry: public Entry
{
public:
	/*!
	 * @brief Creates FileEntry using data from TarHeader, stored on the disk.
	 * @param tarHeader Header of the file as it is stored on the disk.
	 * @param block Starting block of the file (including header).
	 * @param disk DiskDevice the file is stored on.
	 */
	FileEntry( TarHeader& tarHeader, uint block, DiskDevice* disk );

	/*!
	 * @brief Converts to FileEntry.
	 */
	FileEntry* fileEntry() { return this; };

	/*!
	 * @brief Reads data from the file.
	 * @param buffer Place to store the read data.
	 * @param size Number of bytes to read.
	 * @return Number of bytes read, negative number indicates error.
	 *
	 * Data are read from the current posiont in the file
	 */
	ssize_t read( void* buffer, int size );

	/*!
	 * @brief Changes current posiont in the file.
	 * @param pos Point of reference (start, current, end).
	 * @param offset New position as offset from the reference.
	 * @return New position in the file.
	 */
	uint seek( FilePos pos, int offset );

	/*!
	 * @brief Opens file using @a mode.
	 * @param mode Mode (RW, RO, A,...) to use when opening.
	 * @return @a True on sucess, @a false otherwise.
	 */
	bool open( const char mode );

	/*! @brief Closes file. */
	void close();

	/*! @brief Correctly destructs FileEntry. */
	~FileEntry();
		
private:
	uint m_uid;        /*!< Owner of the file.              UNUSED */
	uint m_gid;        /*!< Owning group.                   UNUSED */
	uint m_size;       /*!< Size of the data.                      */
	uint m_startPos;   /*!< First data block.                      */
	uint m_modTime;    /*!< Time of the last modifications. UNUSED */
	uint m_readCount;  /*!< Number of openings or reading.         */
	uint m_pos;        /*!< Current position in the file.          */
		
	FileEntry( const FileEntry& );              /*!< No copies.      */
	FileEntry& operator = ( const FileEntry& ); /*!< No assignments. */
};
