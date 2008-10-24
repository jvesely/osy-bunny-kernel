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
 * @brief OutputCharacterDevice class declaration and implementation.
 *
 * Contains class OutputCharacterDevice, simple driver for controling
 * character output devices.
 */
#pragma once

#include "types.h"

/*! @class OutputCharacterDevice drivers/OutputCharacterDevice "drivers/OutputCharacterDevice"
 * @brief Unbuffered output character class.
 *
 * class writes given chars to specified adress belonging to the device
 */
class OutputCharacterDevice
{
public:
	/*! constructor sets writing address 
	 * @param add writing address
	 */
	OutputCharacterDevice(char * add):m_outputAddress(add){};

	/*! method prints given char 
	 * @param c character to print
	 * @return number of chars printed (0/1)
	 */
	inline size_t outputChar(char c) const { *m_outputAddress = c; return 1; };

private:
	/*! writing address */
	char * m_outputAddress;
};