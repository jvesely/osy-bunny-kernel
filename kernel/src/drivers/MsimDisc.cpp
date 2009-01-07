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
#include "MsimDisc.h"
#include "api.h"
#include "tools.h"
#include "InterruptDisabler.h"
#include "proc/Thread.h"
#include "synchronization/MutexLocker.h"

bool MsimDisc::read( char* buffer, uint count, uint secno, uint start_pos )
{
	InterruptDisabler inter;
	MutexLocker guard(m_guard);

	ASSERT (!pending());
	//if (pending()) return false;

	uint copy_count = 0;

	while (count) {
		/* from the begining and large enough block read directly */
		if ( start_pos == 0 && count >= BLOCK_SIZE ) {
			diskOp( secno, buffer, OP_READ ); 
			copy_count = BLOCK_SIZE;
		} else {
			diskOp( secno, m_buffer, OP_READ );
			copy_count = min( count, BLOCK_SIZE - start_pos );
			memcpy( buffer, m_buffer + start_pos, copy_count );
			
			/* If it did not read til the end of the block it won't be used again. */
			start_pos = 0;
		}
		count  -= copy_count;
		buffer += copy_count;
		++secno; /* read next sector. */
		block();
	}	
	return true;
}
/*----------------------------------------------------------------------------*/
void MsimDisc::block()
{
	m_waitingThread = Thread::getCurrent();
	if (m_waitingThread) {
		m_waitingThread->block();
		m_waitingThread->yield();
	} else {
		while (pending()) {};
	}

}
/*----------------------------------------------------------------------------*/
bool MsimDisc::write( char* buffer, uint count, uint block, uint start_pos )
{
	InterruptDisabler inter;

	if (pending()) return false;

	/* not IMPLEMENTED */
  return false;
}
/*----------------------------------------------------------------------------*/
void MsimDisc::diskOp( uint block_num, char buffer[], OperationMask op)
{
	ASSERT (!pending());
	*m_addr       = buffer;
	*m_secno_addr = block_num;
	*m_status     = op;
}
/*----------------------------------------------------------------------------*/
void MsimDisc::handleInterrupt()
{
}
/*----------------------------------------------------------------------------*/
