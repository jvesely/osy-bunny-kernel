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
 * @brief InputCharacterDevice class declaration and implementation.
 *
 * Contains class InputCharacterDevice, simple driver for controling
 * character input devices.
 */
#pragma once

#define BUFF_SIZE 128
#include "structures/Buffer.h"

/*!
 * @class InputCharacterDevice InputCharacterDevice.h "drivers/InputCharacterDevice.h"
 * @brief Buffered character input class.
 *
 * Class reads character from given address and stores them in the buffer.
 * It needs to take kick in the form of insert function to do that.
 */
class InputCharacterDevice
{
public:
	/*! @brief Constructor sets the reading address */
	InputCharacterDevice(char * address):m_inputAddress(address){};

	/*! @brief Reads one stored char from the buffer */
	inline char getChar() 
		{ ASSERT (m_buffer.count()); return m_buffer.getFirst(); };
	
	/*! @brief Outputs buffer size */
	inline size_t size() const { return BUFF_SIZE; };

	/*! @brief Outputs current buffer usege */
	inline size_t count() const { return m_buffer.count(); };

	/*! @brief Kick to read value from input and put it to the buffer */
	inline size_t insert() {	return m_buffer.append( *m_inputAddress );	};

protected:
	/*! device address */
	char * m_inputAddress;

	/*! buffer of the read characters */
	Buffer<char, BUFF_SIZE> m_buffer;
};
