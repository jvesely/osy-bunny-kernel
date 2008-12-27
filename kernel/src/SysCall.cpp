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

#include "SysCall.h"
#include "proc/Thread.h"
#include "address.h"

SysCall::SysCall(Processor::Context* registers): m_registers( registers )
{
	call      = (SysCalls::SysCalls)((*(unative_t*)Processor::reg_read_epc())>>6);
	params[0] = registers->a0;
	params[1] = registers->a1;
	params[2] = registers->a2;
	params[3] = registers->a3;
}
/*----------------------------------------------------------------------------*/
void SysCall::handle()
{
	printf( "Handling syscall: %x, with params %x,%x,%x,%x.\n",
		call, params[0], params[1], params[2], params[3]);

	switch ( call ) {
		case SysCalls::SC_PUTS:
			handlePuts();
			break;
		case SysCalls::SC_GETS:
			handleGets();
			break;
		default:
			puts("Unknown SYSCALL killing thread.\n");
			Thread::getCurrent()->kill();
	}
}
/*----------------------------------------------------------------------------*/
void SysCall::handlePuts()
{
/*	printf ("Should output string at %p(%p):%s.\n", 
		params[0], ADDR_TO_USEG(params[0]) ,params[0]); */
//	Processor::msim_stop();
//	if (ADDR_TO_USEG(params[0]) == params[0]) {
		m_registers->a0 = puts( (const char*)params[0] );
//	} else {
//		Thread::getCurrent()->kill();
//	}

}
