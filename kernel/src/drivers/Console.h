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
	/*! @brief Initializes parent classes for input and output. */
	Console(char* outAddress = NULL, char* inAddress = NULL):
		OutputCharacterDevice(outAddress),
		InputCharacterDevice(inAddress) {};
	
	/*! @brief Prints string on associated device.
	 * @param str pointer to the first char
	 * @return number of printed chars
	 */
	size_t outputString(const char* str) const;

	/*! @brief Gets next character in the buffer. (BLOCKING)
	 * @return Read character
	 */
	char readChar();

	/*! @brief Reads and stores multiple chars from the input.
	 * @param str Place to store the characters.
	 * @param len Maximum number of characters to store, including ending \\0.
	 * @return Number of read characters
	 */
	ssize_t readString(char* str, const size_t len);

	/*! @brief Handles keyboard interrupts.
	 *
	 * Fills the buffer, wakes the threads.
	 */
	void interrupt();

private:
	/*! @brief List of threads wating for input */
	ThreadList m_waitList;
};
