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
#include "api.h"

//------------------------------------------------------------------------------
/** @brief time respresentation
*	both seconds and microseconds
*/
class Time
{
public:

	/** @brief million*/
	static const int MILLION = 1000000;

	/** @brief max seconds in unsigned int microseconds
	 *	maxint-1 / 1000000
	 */
	static const uint MAX_USEC_SECS = -1 / MILLION;

	/** @brief Constructs Time at the beginning of the Era.
	 */
	inline Time(uint sec = 0, uint usec = 0):
		m_secs(sec), m_usecs(usec){
		m_secs += (m_usecs / MILLION);
		m_usecs %= MILLION;
	};
/*
	inline Time& operator =(const Time& other) {
//		dprintf("Calling operator = \n");

		m_secs = other.m_secs;
		m_usecs = other.m_usecs;
		return *this;
	}
*/
	/** @brief Adds the given time to this time, and 
	 * returns a reference to this time.
	 *
	 * Time is normalized during the addition, thus usec part should not be 
	 * greater than or equal 1 000 000.
	 */
	inline Time& operator += (const Time& other) {
		m_usecs += other.m_usecs;
		m_secs  += other.m_secs + (m_usecs / MILLION);
		m_usecs %= MILLION;
		return *this;
	}

	/** @brief Substracts the given time to this time, and
	 * returns a reference to this time.
	 *
	 * Due to usage of unsigned types may the result of this operation be
	 * the future time. Time is normalized during substraction so both
	 * seconds and microseconds should be within their boundaries.
	 */
	inline Time& operator -= (const Time& other) {
		if (m_usecs < other.m_usecs) --m_secs;
		m_usecs = (MILLION + m_usecs - other.m_usecs) % MILLION;
		m_secs  -= other.m_secs;
		return *this;
	}

	/** @brief Returns true if this is equal to  other, otherwise returns false.
	 *
	 * Times are equal if both their timestamp (seconds) parts and their 
	 * useconds parts are equal.
	 */
	inline bool operator == (const Time& other) const {
		return (m_secs == other.m_secs) && (m_usecs == other.m_usecs) ;
	}

	/** @brief Returns true if this is not equal to  other, 
	 * otherwise returns false.
	 *	
	 *	Times are not equal if either their timestamp (seconds) parts OR 
	 *	their useconds parts differ.
	 *	@note Implementation uses operator == (const Time& other) const
	 */
	inline bool operator != (const Time& other) const
		{	return !((*this) == other);	}
  
	/** @brief Returns true if this is less than other, 
   * otherwise returns false.
   *  
   *  Time is less than other if either timestamp (seconds) is smaller
	 *  than other's, or timestamp parts are equal and useconds part is less
	 *  than other's.
   */
	inline bool operator < (const Time& other) const {
		return (m_secs < other.m_secs) 
				|| ( (m_secs  == other.m_secs) && (m_usecs <  other.m_usecs) );
	}

	
	/** @brief Returns true if this is greater than other 
	 *  otherwise returns false.
   *  
   *  Time is less than other if either timestamp (seconds) is smaller
   *  than other's, or timestamp parts are equal and useconds part is less
   *  than other's.	
	 *  @note Implementation uses bool operator < (const Time& other) const ;
   */
	inline bool operator > (const Time& other) const
		{	return other < *this;	}

 /** @brief Returns true if this is greater than or equals other 
   *  otherwise returns false.
   *  
   *  Time is greater than or equals other if either timestamp (seconds) is
	 *  greater than or equals other's, or timestamp parts are equal and 
	 *  useconds part is greater than or equals other's. 
   *  @note Implementation uses bool operator < (const Time& other) const;
   */
	bool operator >=(const Time & other) const
		{	return !(operator<(other));	}

	/** @brief Returns true if this is less than or equals other 
   *  otherwise returns false.
   *  
   *  Time is less than or equals  other if either timestamp (seconds) is
   *  less than or equals other's, or timestamp parts are equal and 
   *  useconds part is greater than or equals other's. 
   *  @note Implementation uses bool operator > (const Time& other) const;
   */
	bool operator <=(const Time & time) const 
		{	return !(operator>(time)); }
	
	/** @brief Returns new time that is sum of this Time and other Time.
	 *
	 * Each component is added separately and then they are normalized.
	 * @note Implementation uses Time& operator += (const Time& other);
	 */
	inline Time operator + (const Time& other) const
	{
		Time result(*this);
		result += other;
		return result;
	};

	/** @brief Returns new time that is equal to this Time - other Time.
   *
   * Each component is substracted separately and then they are normalized.
   * @note Implementation uses Time& operator -= (const Time& other);
   */
	inline Time operator - (const Time& time) const
	{
		Time result(*this);
		result -= time;
		return result;
	};

	/** @brief Gets timestamp part. */
	inline uint secs() const { return m_secs; }

	/** @brief Gets useconds part. */
	inline uint usecs() const { return m_usecs; }

	/** @brief Gets current time. 
	 *
	 * Given time should not differ from actual time by more than 1 second.
	 */
	static Time getCurrent();
/*	{
		const uint usecs = Kernel::instance().clock().usec();
		const uint secs  = Kernel::instance().clock().time();
		return Time(secs, usecs);
	}
*/
protected:
	/** @brief UNIX timestamp */
	uint m_secs;

	/** @brief microseconds */
	uint m_usecs;
};
