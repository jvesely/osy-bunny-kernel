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

#pragma once

#include "types.h"
class DiskDevice;
class DirEntry;
class FileEntry;

/*!
 * @class Entry Entry.h "tarfs/Entry.h"
 * @brief Abstract class representing generic Entry on the TarFS filesystem.
 *
 * Class provides interface for both file and directory handling, this
 * interface is implemented by DirEntry and FileEntry classes representing
 * corresponding TarFS objects.
 */
class Entry
{
public:
	/*!
	 * @brief Assigns the DiskDevice this Entry is stored on.
	 */
	Entry(DiskDevice* storage): m_storage( storage ) {};

	/*!
	 * @brief Read operation.
	 * @param buffer Place to store read data.
	 * @param size Number of bytes to read.
	 * @return Number of bytes read.
	 */
	virtual ssize_t read( void* buffer, int size ) = 0;

	/*!
	 * @brief Change position wihtin the object.
	 * @param pos Reference point (start, end, current).
	 * @param offset Nmber of bytes to move,
	 * 	positve means forward, negative backward.
	 * @return New position as offset from the start.
	 */
	virtual uint seek( FilePos pos, int offset ) = 0;

	/*!
	 * @brief Signals object for future operations that might only be made
	 * 	on opened objects(files).
	 * @param mode Open mode.
	 * @return @a True on success, @a false otherwise.
	 */
	virtual bool open( const char mode ) { return false; };
	/*!
	 * @brief Singals object no further operations
	 * 	that need opening will be requested.
	 */
	virtual void close() {};
	
	/*!
	 * @brief Converts Entry into one of its successors.
	 *
	 * @note dynamic_cast might do the same job.
	 */
	virtual DirEntry*  dirEntry()  { return NULL; }

	/*!
	 * @brief Converts Entry into one of its successors.
	 *
	 * @note dynamic_cast might do the same job.
	 */
	virtual FileEntry* fileEntry() { return NULL; }

	/*! @brief Does nothing. */
	virtual ~Entry() {};
protected:
	/*! No copying */
	Entry( const Entry& other );

	/*! No assigning */
	Entry& operator = (const Entry& other);

	/*!
	 * @brief Provides read interface for the devices this Entry is stored on.
	 */
	bool readFromDevice(void* buffer, size_t count, uint start_block, uint offset);
private:
	DiskDevice* m_storage; /*!< Storage where the data of this Entry are stored.*/
};
