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
 * @brief Console class declaration
 *
 * IO class Console handles text (character) input/output.
 */
#pragma once

#include "OutputCharacterDevice.h"
#include "InputCharacterDevice.h"
#include "structures/List.h"
#include "proc/Thread.h"

/*!
 * @class Console Console.h "drivers/Console.h"
 * @brief Character IO class.
 *
 * Console handles character output and keyboard input
 * inherits classes OutputCharacterDevice and InputCharacterDevice
 */
class Console:public OutputCharacterDevice, public InputCharacterDevice
{
public:
	/*! Initializes parent classes for input and output */
	Console(char* outAddress = NULL, char* inAddress = NULL):
		OutputCharacterDevice(outAddress),
		InputCharacterDevice(inAddress) {};
	
	/*! Prints string on associated device
	 * @param str pointer to the first char
	 * @return number of printed chars
	 */
	size_t outputString(const char* str) const;

	char readChar();
	
	ssize_t readString(char* str, const size_t len);

private:
	List<Thread*> m_waitList;
};
