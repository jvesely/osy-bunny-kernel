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

#include "devices.h"
#include "synchronization/Mutex.h"
#include "DiskDevice.h"

#define BLOCK_SIZE 512

class Thread;

class MsimDisk: public DiskDevice
{
public:
	MsimDisk( unative_t* address ):
		m_registers( address )
		{};
	bool read( void* buffer, uint count, uint block, uint start_pos );
	bool write( void* buffer, uint count, uint block, uint start_pos );
	size_t size() { return m_registers[SIZE]; };
	void handleInterrupt();

private:
	enum OperationMask {
		OP_READ  = 0x1,     /*!< First bit.  */
		OP_WRITE = 0x2      /*!< Second bit. */
	};
	static const unative_t DONE_MASK  = 0x4;     /*!< Third bit.  */
	static const unative_t ERROR_MASK = 0x8;     /*!< Fourth bit. */

	enum Offsets {
		DATA, SECTOR, STATUS, SIZE, LIMIT
	};

	void diskOp( uint block_num, unative_t buffer, OperationMask op );
	void block();

	inline bool pending()
		{ return m_registers[STATUS] && !(m_registers[STATUS] & DONE_MASK); };

	volatile unative_t* m_registers;

	Thread* m_waitingThread;

	char m_buffer[BLOCK_SIZE];
	Mutex m_guard;
};
