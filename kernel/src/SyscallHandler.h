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
#include "drivers/Processor.h"
#include "ExceptionHandler.h"
#include "syscallcodes.h"

class SyscallHandler: public ExceptionHandler {
public:
	SyscallHandler();
	bool handleException( Processor::Context* registers );

private:
	
	unative_t m_call;
	unative_t m_params[4];
	unative_t (SyscallHandler::*m_handles[SYS_LAST])(void);

	unative_t handlePuts();
	unative_t handleGets();
	unative_t handleExit();
	unative_t handleThreadCreate();
	unative_t handleThreadJoin();
	unative_t handleThreadSleep();
	unative_t handleThreadYield();
	unative_t handleThreadSuspend();
	unative_t handleThreadWakeup();
	unative_t handleThreadSelf();
	unative_t handleThreadDetach();
	unative_t handleThreadCancel();
	unative_t handleThreadExit();
};
