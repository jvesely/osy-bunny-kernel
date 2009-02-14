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
 * @brief ExceptionHandler interface class declaration.
 *
 * Long description. I would paste some Loren Ipsum rubbish here, but I'm afraid
 * It would stay that way. Not that this comment is by any means ingenious but 
 * at least people can understand it. 
 */

#pragma once

#include "drivers/Processor.h"

/*!
 * @class ExceptionHandler ExceptionHandler.h "ExceptionHandler.h"
 * @brief Generic exception handling interface.
 *
 * Class ExceptionHandler provides interface that all eception handlers
 * should follow.
 */
class ExceptionHandler
{
public:
	/*! @brief Interface function, every exception handling class shall implement it.
	 * @param registers Pointer to the stored context.
	 * @return @a true if exception was handled successfully, @a false otherwise.
	 */
	virtual bool handleException( Processor::Context* registers ) = 0;

	/*! @brief Overloaded function provided for convenience. */
	inline bool operator()( Processor::Context* registers )
		{ return handleException( registers ); }
};
