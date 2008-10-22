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

#define TIMER_USEC  1 /*!< next byte, taken from msim source */

/*! 
 * @class RTC RTC.h "drivers/RTC.h"
 * @brief clock
 *
 * Class provides acces to realtime clock device.
 */
class RTC {

public:
	/*! @brief asumes rtc device on given address 
	 *
	 * @param address addres from which time could be read
	 */
	RTC(const uint32_t * address = NULL):m_timeAddress(address) {};

	/*! @brief  Unix timestamp, unsingned.
	 * @return const timestamp
	 */
	inline uint32_t time() const 
		{	return m_timeAddress?*m_timeAddress:(uint32_t)0; };

	/*! @brief get usec part of the time 
	 * @return const usec part of the time
	 */
	inline uint32_t usec() const
		{ return (m_timeAddress)?( * (m_timeAddress + TIMER_USEC  ) ):0; };

private:
	/*! device address */
	volatile const uint32_t * m_timeAddress;
	
};
