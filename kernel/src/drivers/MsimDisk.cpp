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
#include "MsimDisk.h"
#include "api.h"
#include "tools.h"
#include "InterruptDisabler.h"
#include "proc/Thread.h"
#include "synchronization/MutexLocker.h"
#include "address.h"

//#define DISC_DEBUG

#ifndef DISC_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...)\
  puts("[ DISC DEBUG ]: ");\
  printf(ARGS);
#endif

bool MsimDisk::read( void* buffer, uint count, uint secno, uint start_pos )
{
	InterruptDisabler inter;
	MutexLocker guard(m_guard);

	PRINT_DEBUG ("Started read status: %x.\n", m_registers[STATUS]);
	
	ASSERT (!pending());
	//if (pending()) return false;

	uint copy_count = 0;
	char* target = (char*)buffer;
	
	while (count) {
		/* from the begining and large enough block read directly */
		if ( start_pos == 0 && count >= BLOCK_SIZE && ADDR_IN_KSEG0(target) ) {
			PRINT_DEBUG ("Reading whole block to buffer %p.\n", target);
			diskOp( secno, ADDR_TO_USEG((uintptr_t)target), OP_READ ); 
			copy_count = BLOCK_SIZE;
		} else {
			PRINT_DEBUG ("Partial read to internal buffer %p.\n", m_buffer);	
			diskOp( secno, ADDR_TO_USEG((uintptr_t)m_buffer), OP_READ );
			copy_count = min( count, BLOCK_SIZE - start_pos );
			memcpy( buffer, m_buffer + start_pos, copy_count );
			/* If it did not read til the end of the block it won't be used again. */
			start_pos = 0;
		}
		PRINT_DEBUG ("Read %d B of data to: %p.\n", copy_count, buffer);
/*	  for (uint i = 0; i < copy_count; ++i) {
	    printf("  %p:\"%c\"<%d>  ",
				buffer + i, ((char*)ADDR_TO_KSEG0((uint)buffer))[i], 
				((char*)ADDR_TO_KSEG0((uint)buffer))[i]);
	  };
	*/
		count  -= copy_count;
		target += copy_count;
		++secno; /* read next sector. */
	}
	PRINT_DEBUG ("Leaving read to buffer: %p.\n", buffer);
	return (! (m_registers[STATUS] & ERROR_MASK));
}
/*----------------------------------------------------------------------------*/
void MsimDisk::block()
{
	
	m_waitingThread = Thread::getCurrent();
	ASSERT (m_waitingThread);
	if (m_waitingThread) {
		m_waitingThread->block();
		m_waitingThread->yield();
	} else {
		while (pending()) {};
	}

	//while (pending()) {};
	//PRINT_DEBUG ("Operation ended: %x.\n", m_registers[STATUS]);
	ASSERT (! (m_registers[STATUS] & ERROR_MASK));
	m_registers[STATUS] = DONE_MASK;

}
/*----------------------------------------------------------------------------*/
bool MsimDisk::write( void* buffer, uint count, uint block, uint start_pos )
{
	InterruptDisabler inter;

	if (pending()) return false;

	/* not IMPLEMENTED */
  return false;
}
/*----------------------------------------------------------------------------*/
void MsimDisk::diskOp( uint block_num, unative_t buffer, OperationMask op)
{
	ASSERT (!pending());
	PRINT_DEBUG ("Started Disk op. buffer %p, sector: %u.\n", buffer, block_num);
	PRINT_DEBUG ("Locations: %p, %p and %p.\n", &m_registers[DATA], &m_registers[SECTOR], &m_registers[STATUS]);
	m_registers[DATA]   = buffer;
	m_registers[SECTOR] = block_num;
	m_registers[STATUS] = op;
	PRINT_DEBUG ("Command issued %p(%p) %u.\n", m_registers[DATA], buffer, m_registers[SECTOR]);
	block();
}
/*----------------------------------------------------------------------------*/
void MsimDisk::handleInterrupt()
{
	PRINT_DEBUG ("Handling disk interrupt.\n");
	ASSERT (!pending());
	ASSERT (m_waitingThread);

	m_waitingThread->resume();
	m_waitingThread = NULL;
	m_registers[STATUS] = DONE_MASK;
}
/*----------------------------------------------------------------------------*/
