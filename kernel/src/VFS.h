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
 * @brief VFS class declaration.
 *
 * Long description. I would paste some Loren Ipsum rubbish here, but I'm afraid
 * It would stay that way. Not that this comment is by any means ingenious but 
 * at least people can understand it. 
 */

#pragma once

#include "types.h"


class DiskDevice;
typedef int file_t;

/*!
 * @class VFS VFS.h "VFS.h"
 * @brief Filesystem interface class.
 *
 * VFS Provides ID based access to files and directories, as well as some
 * basic functions on these entities.
 */
class VFS
{
public:

	VFS(){};
	/*! @brief Use data on the device. */
	virtual bool mount( DiskDevice* disk ) = 0;

	/*!
	 * @brief Opens file using given parameters.
	 * @param file_name Name of the file.
	 * @param mode Mode to use.
	 * @return ID (>0) under whish the file is accessible,
	 * 	EIO if something went wrong.
	 */
	virtual file_t openFile( const char file_name[], const char mode ) = 0;

	/*!
	 * @brief Closes opened fiel and returns used id.
	 * @param file ID of the file to close.
	 */
	virtual void closeFile( file_t file ) = 0;

	/*!
	 * @brief Reads data from the file.
	 * @param src Source file.
	 * @param buffer Place to hold data.
	 * @param size Size of requested data.
	 * @return Number(>=0) of bytes read on success, <0 on failure.
	 */
	virtual ssize_t readFile( file_t src, void* buffer, size_t size ) = 0;

	/*!
	 * @brief Moves position in the file.
	 * @param file File to move position in.
	 * @param pos Reference point.
	 * @param offset Distance of the new position from the ref. point.
	 * @return  New postion.
	 */
	virtual uint seekFile( file_t file, FilePos pos, int offset ) = 0; 
	
	/*!
	 * @brief Checks for further data in the file.
	 * @param file File to check.
	 * @return @a True if there are NO more data, @a false otherwise.
	 */
	virtual bool eof( file_t file ) = 0;

	/*! @breif Disposing correctly. */
	virtual ~VFS(){};

private:
	
	/*! @brief No copies.    */
	VFS( const VFS& );
	/*! @brief No assigning. */
	VFS& operator = ( const VFS& );
};
