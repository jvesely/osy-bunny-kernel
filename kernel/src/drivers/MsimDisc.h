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

#include "InterruptHandler.h"
#include "devices.h"
#include "synchronization/Mutex.h"
#include "DiscDevice.h"

#define BLOCK_SIZE 512

class Thread;

class MsimDisc: public InterruptHandler, public DiscDevice
{
public:
	MsimDisc( char** data_addr, uint* secno_addr, unative_t* status_addr ):
		m_addr( data_addr ), m_secno_addr( secno_addr ), m_status( status_addr )
		{};
	bool read( char* buffer, uint count, uint block, uint start_pos );
	bool write( char* buffer, uint count, uint block, uint start_pos );
	void handleInterrupt();

private:
	enum OperationMask {
		OP_READ  = 0x1,     /*!< First bit.  */
		OP_WRITE = 0x2      /*!< Second bit. */
	};
	static const unative_t DONE_MASK  = 0x4;     /*!< Third bit.  */

	void diskOp( uint block_num, char buffer[BLOCK_SIZE], OperationMask op );
	void block();

	inline bool pending()
		{ return !(*m_status & DONE_MASK); };


	char** m_addr;
	uint* m_secno_addr;
	unative_t* m_status;
	Thread* m_waitingThread;

	char m_buffer[BLOCK_SIZE];
	Mutex m_guard;
};
