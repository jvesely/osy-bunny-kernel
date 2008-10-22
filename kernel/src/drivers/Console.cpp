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
 * @brief Console class implementation.
 *
 * Long description. I would paste some Loren Ipsum rubbish here, but I'm afraid
 * It would stay that way. Not that this comment is by any means ingenious but 
 * at least people can understand it. 
 */

#include "Console.h"

/*! 
 * @brief method output string on associated output device.
 *
 * This method uses inherited method outputChar to output multiple chars.
 * Method does not need to change any members so its constant
 * @param str pointer to the first char
 * @return return the number of printed chars
 */
size_t Console::outputString(const char* str) const
{
	const char *  it = str; // fly through the string
	
	for (;*it;++it)
	{
		outputChar(*it);
	}

	return it - str; /* finish - start should give the number of chars */
}
