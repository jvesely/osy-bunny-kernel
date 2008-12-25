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
 * @brief Time representation
 *
 * Class Time storing time in both seconds and microseconds
 */
#pragma once

#include "api.h"

//------------------------------------------------------------------------------
/** @brief time respresentation
*	Stores both seconds and microseconds, count of microseconds is less than one
*	million.
*
*	Use of implicit operator = and copy constructor is safe and implicit
*	implementation is sufficient.
*
*/
class Time
{
public:
	/** @brief million*/
	static const uint MILLION = 1000000;

	/** @brief max seconds in unsigned int microseconds
	*
	*	maxint-1 / 1000000. Is not maxint, to ensure that
	*	MAX_USEC_SECS * MILLION + 999999 < maxint
	*/
	static const uint MAX_USEC_SECS = -2 / MILLION;

	/** @brief no brief comment
	*
	*	default time is 0 seconds, 0 microSecs
	*/
	inline Time( uint sec = 0, uint usec = 0 ) 
	{
		setTime( sec, usec );
	}

	/*  Time(const Time & time)
	*	implicit implementation is sufficient and safe
	*/

	/*	operator =
	*	implicit implementation is sufficient and safe
	*/

	/** @brief
	*
	*	calls setTime (normalizes time, so that usec part is less than million)
	*	@return *this const
	*/
	inline const Time& operator += ( const Time& other ) {
		ASSERT (m_usecs < MILLION);
		ASSERT (other.m_usecs < MILLION);

		setTime( other.m_secs + m_secs, other.m_usecs + m_usecs );
		return *this;
	}

	/** @brief
	*
	*	calls setTime (normalizes time, so that usec part is less than million)
	*	@note Due to usage of unsigned types may the result of this operation be
	*	greater than original *this time.
	*	@return *this const
	*/
	inline const Time& operator -= ( const Time& other )
	{
		ASSERT (m_usecs < MILLION);
		ASSERT (other.m_usecs < MILLION);

		setTime( (m_secs - other.m_secs) - 1, (MILLION + m_usecs) - other.m_usecs );
		return *this;
	}
	/** @brief
	*
	*	compares Time values
	*	Times are equal if both their timestamp (seconds) parts and their
	*	useconds parts are equal.
	*/
	inline bool operator == ( const Time& other ) const
	{
		ASSERT (m_usecs < MILLION);
		ASSERT (other.m_usecs < MILLION);

		return ( (m_secs == other.m_secs) && (m_usecs == other.m_usecs) );
	}

	/** @brief
	*
	*	Times are equal if both their timestamp (seconds) parts and their
	*	useconds parts are equal.
	*	function is wrapper for !(operator==)
	*/
	inline bool operator != ( const Time& other ) const {
		ASSERT (m_usecs < MILLION);
		ASSERT (other.m_usecs < MILLION);

		return !( operator == ( other ) );
	}

	/** @brief
	*
	*	Time is less than other if either timestamp (seconds) is smaller
	*	than other's, or timestamp parts are equal and useconds part is less
	*	than other's.
	*/
	inline bool operator < ( const Time& other ) const {
		ASSERT (m_usecs < MILLION);
		ASSERT (other.m_usecs < MILLION);

		return ( m_secs < other.m_secs )
	      || ( ( m_secs  == other.m_secs ) && ( m_usecs <  other.m_usecs ) );
	}

	/** @brief
	*
	*	Time is less than other if either timestamp (seconds) is smaller
	*	than other's, or timestamp parts are equal and useconds part is less
	*	than other's.
	*	@note Implementation uses bool operator < (const Time& other) const ;
	*/
	inline bool operator > ( const Time& other ) const {
		ASSERT (m_usecs < MILLION);
		ASSERT (other.m_usecs < MILLION);

		return other < *this;	
	}

	/** @brief
	*
	*	Time is greater than or equals other if either timestamp (seconds) is
	*	greater than or equals other's, or timestamp parts are equal and
	*	useconds part is greater than or equals other's.
	*	@note Implementation uses bool operator < (const Time& other) const;
	*/
	bool operator >= ( const Time& other ) const {
		ASSERT (m_usecs < MILLION);
		ASSERT (other.m_usecs < MILLION);

		return !( operator < ( other ) );
	}

	/** @brief
	*
	*	Time is less than or equals other if either timestamp (seconds) is
	*	less than or equals other's, or timestamp parts are equal and
	*	useconds part is greater than or equals other's.
	*	@note Implementation uses bool operator > (const Time& other) const;
	*/
	bool operator <= ( const Time& other ) const {
		ASSERT (m_usecs < MILLION);
		ASSERT (other.m_usecs < MILLION);

		return !(other < *this);
	}

	/** @brief set time to values
	*
	*	normalizes time, so that microsecs part is less than million.
	*	If usec parameter is greater than million, seconds part is increased.
	*/
	inline void setTime( uint sec, uint usec ){
		m_secs = sec;
		m_usecs = usec;
		correctTime();
	}

	/** @brief Returns new time that is sum of this Time and the other Time.
	*
	*	Each component is added separately and then they are normalized.
	*	@note Implementation uses Time& operator += (const Time& other);
	*/
	Time operator + ( const Time& other ) const
	{
		ASSERT (m_usecs < MILLION);
		ASSERT (other.m_usecs < MILLION);
		
		Time result( *this );
		result += other;
		return result;
	};

	/** @brief Returns new time that is equal to this Time - other Time.
	*
	*	Each component is substracted separately and then they are normalized.
	*	@note Implementation uses Time& operator -= (const Time& other);
	*/
	inline Time operator - ( const Time& other ) const
	{
		ASSERT (m_usecs < MILLION);
		ASSERT (other.m_usecs < MILLION);
	
		Time result( *this );
		result -= other;
		return result;
	};

	inline operator bool () const { return m_secs || m_usecs; }

	/** @brief Gets timestamp part. */
	inline uint secs() const { return m_secs; }

	/** @brief Gets useconds part. */
	inline uint usecs() const { return m_usecs; }

	/** @brief Gets timestamp part. wrapper for usecs*/
	inline uint getSecs() const { return secs(); }

	/** @brief Gets useconds part. wrapper for secs*/
	inline uint getUsecs() const { return usecs(); }

	/** @brief Gets current time.
	 *
	 * Given time should not differ from actual time by more than 1 second.
	 */
	static Time getCurrentTime();

	/** @brief Gets current time.
	*
	* Given time should not differ from actual time by more than 1 second.
	*	wrapper for getCurrentTime()
	*/
	inline static Time getCurrent() { return getCurrentTime(); }

	inline static Time fromSeconds( uint secs ) { return Time(secs, 0); }

	inline static Time fromMicroSeconds( uint usecs ) { return Time(0, usecs); }

protected:
	/** @brief corrects time
	*
	*	Ensures that microsecond part of time is less than million
	*/
	inline void correctTime()
	{
		m_secs += ( m_usecs / MILLION );
		m_usecs = m_usecs % MILLION;
	}

	/** @brief seconds time */
	uint m_secs;

	/** @brief microseconds time */
	uint m_usecs;
};

