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
 * @brief DiskDevice class declaration.
 *
 * DiskDevice class only provides interface memebr functions.
 */

#pragma once
#include "InterruptHandler.h"

/*!
 * @class DiskDevice DiskDevice.h "drivers/DiskDevice.h"
 * @brief Abstract class that provides handling of the disk devices.
 */
class DiskDevice: public InterruptHandler
{
public:
	/*!
	 * @brief Reads requested data into the designated location.
	 * @param buffer Buffer to store data in.
	 * @param count Number of bytes to read (size of the buffer).
	 * @param block Starting block number (block containing the first byte).
	 * @param start_pos Offset of the first byte from the start of the block.
	 * @return @a true if data were read successfully. @a false otherwise.
	 */
	virtual bool read( void* buffer, uint count, uint block, uint start_pos ) = 0;

	/*!
	 * @brief Writes data from the designated location to the disk.
	 * @param buffer Buffer to read data from.
	 * @param count Number of bytes to write (size of the buffer).
	 * @param block Starting block number (block containing the first byte).
	 * @param start_pos Offset of the first byte from the start of the block.
	 * @return @a true if data were written successfully. @a false otherwise.
	 */
	virtual bool write( void* buffer, uint count, uint block, uint start_pos );

	/*!
	 * @brief Gets number of bytes the device can store.
	 * @return Number of bytes the device can store.
	 */
	virtual size_t size();
};
