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
 * @brief RTC driver.
 *
 * Realtime clock driver brings to time to machine.
 */
#pragma once
#include "types.h"
#include "api.h"

#define TIMER_USEC  1 /*!< next byte, taken from msim source */

/*! 
 * @class RTC RTC.h "drivers/RTC.h"
 * @brief clock
 *
 * Class provides acces to realtime clock device.
 */
class RTC {

public:
	static const unsigned int SECOND = 1000000;    /*!< usecs in 1 sec  */
	static const unsigned int MILLI_SECOND = 1000;  /*!< usecs in 1 msec */

	/*! @brief asumes rtc device on given address 
	 *
	 * @param address addres from which time could be read
	 */
	RTC(const uint32_t * address = NULL):m_timeAddress(address) {};

	/*! @brief  Unix timestamp, unsingned.
	 * @return const timestamp
	 */
	inline unsigned int time() const 
		{	return m_timeAddress ? (*m_timeAddress) : 0; };

	/*! @brief get usec part of the time 
	 * @return const usec part of the time
	 */
	inline unsigned int usec() const
		{ return (m_timeAddress)?( *(m_timeAddress + USEC_TIMER_LOC) ):0; };

private:
	/*! device address */
	volatile const uint32_t * m_timeAddress;
	static const uintptr_t USEC_TIMER_LOC = 1;
	
};
