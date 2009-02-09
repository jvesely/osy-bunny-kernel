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
 * @brief SyscallHandler class declaration.
 *
 * SyscallHandler class is a class that stores handling routines and
 * calls them when necessary.
 */
#pragma once
#include "drivers/Processor.h"
#include "ExceptionHandler.h"
#include "syscallcodes.h"

/*!
 * @class SyscallHandler SyscallHandler.h "SyscallHandler.h"
 * @brief Syscall handling class.
 *
 * Stores handling routines in the vector, identifies Syscall and
 * calls appropriate routine. As syscall is one of the expcetions
 * SyscallHandler implements ExceptionHandler interface.
 */
class SyscallHandler: public ExceptionHandler
{
public:
	/*!
	 * @brief Constructs handling vector.
	 */
	SyscallHandler();

	/*!
	 * @brief Handles Exception.
	 * @param registers Stored context of the thread that caused the exception.
	 * @return @a true if handler to the identified syscall was found and called, 
	 * 	@a false otherwise.
	 */
	bool handleException( Processor::Context* registers );

private:
	/*! @brief Syscall handling vector. */
	unative_t (*m_handles[SYS_COUNT])(unative_t par[]);
};
