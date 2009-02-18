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
 * @brief Class MsimDisk declaration.
 *
 * Long description. I would paste some Loren Ipsum rubbish here, but I'm afraid
 * It would stay that way. Not that this comment is by any means ingenious but 
 * at least people can understand it. 
 */

#pragma once

#include "devices.h"
#include "synchronization/Mutex.h"
#include "DiskDevice.h"

#define BLOCK_SIZE 512

class Thread;

/*!
 * @class MsimDisk MsimDisk.h "drivers/MsimDisk.h"
 * @brief Class provides hardware specific access to disk device used by MSIM.
 */
class MsimDisk: public DiskDevice
{
public:
	/*!
	 * @brief Constructs disk from pointer to the control registers.
	 * @param address Address of the first control register.
	 */
	MsimDisk( unative_t* address ):
		m_registers( address )
		{};

	/*!
	 * @brief Reads data from the disk into the provided buffer.
	 * @param buffer Place to store the data.
	 * @param count Number of bytes to read(should be <= size of the buffer).
	 * @param block Starting block.
	 * @param start_pos Offset of the first requested byte from 
	 * 	the start of the block.
	 * @return @a True on success, @a false otherwise.
	 *
	 * @note: if buffer is accessible to the disk device data are copied directly
	 * 	otherwise internall driver buffer is used.
	 */
	bool read( void* buffer, uint count, uint block, uint start_pos );


	/*!
	 * @brief Writes data to the disk from the provided buffer.
	 * @param buffer Data to write.
	 * @param count Number of bytes to write(should be <= size of the buffer).
	 * @param block Starting block.
	 * @param start_pos Offset of the first requested byte from 
	 * 	the start of the block.
	 * @return @a True on success, @a false otherwise.
	 *
	 * @note: NOT implemented.
	 */
	bool write( void* buffer, uint count, uint block, uint start_pos );

	/*!
	 * @brief Gets size of the disk.
	 * @return Size of fthe disk in bytes.
	 */
	size_t size() { return m_registers[SIZE]; };

	/*!
	 * @brief Handles data written, data ready, error...
	 */
	void handleInterrupt();

private:
	enum OperationMask {
		OP_READ  = 0x1,     /*!< First bit.  */
		OP_WRITE = 0x2      /*!< Second bit. */
	};
	static const unative_t DONE_MASK  = 0x4;     /*!< Third bit.  */
	static const unative_t ERROR_MASK = 0x8;     /*!< Fourth bit. */

	/*! @brief Known register posotions. */
	enum Offsets {
		DATA, SECTOR, STATUS, SIZE, LIMIT
	};

	/*!
	 * @brief directly accesses device to perform oeration.
	 * @param block_num NUmber of block to operate on.
	 * @param buffer Buffer to use for the operation.
	 * @param op Operation to perform (read/write).
	 */
	void diskOp( uint block_num, unative_t buffer, OperationMask op );

	/*! @brief Blocks current thread until the operation is finished.	 */
	void block();

	/*!
	 * @brief Gets operation status of the device.
	 * @return @a True if there is an unfinished operation, false otherwise.
	 */
	inline bool pending()
		{ return m_registers[STATUS] && !(m_registers[STATUS] & DONE_MASK); };

	/*! @brief Device registers. */
	volatile unative_t* m_registers;

	/*! @brief Thread that caused the current activity. */
	Thread* m_waitingThread;

	/*! Internal driver buffer. */
	char m_buffer[BLOCK_SIZE];
	
	/*! Only one Thread can access disk. */
	Mutex m_guard;
};
