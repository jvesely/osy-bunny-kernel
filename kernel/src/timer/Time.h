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
#include <api.h>

/** @brief million*/
//const int MILLION = 1000000;

/** @brief max seconds in unsigned int microseconds
*	maxint-1 / 1000000
*/
//const unsigned int MAX_USEC_SECS = 4294;

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
	static const uint MAX_USEC_SECS = -2 / MILLION;

	/** @brief parametrised ctor
	*	default time is 0seconds, 0 microSecs
	*/
	inline Time(unsigned int sec = 0, unsigned int usec = 0){
		setTime(sec,usec);
	}

	/** @brief copy ctor
	*	though this is not needed (because implicit copy ctor would now do the same),
		it is here (to make clear that it is correctly implemented)
	*	is safe against incorrect input time
	*	calls setTime (normalizes time, so that usec part is less than million)
	*/
	inline Time(const Time & time){
		setTime(time.getSecs(),time.getUsecs());
	}

	/** @brief operator =
	*	copies seconds and microsecond value of time
	*	calls setTime (normalizes time, so that usec part is less than million)
	*	@return *this const
	*/
	inline const Time & operator = (const Time & time){
		setTime(time.getSecs(),time.getUsecs());
		return *this;
	}

	/** @brief operator +=
	*	calls setTime (normalizes time, so that usec part is less than million)
	*	@return *this const
	*/
	inline const Time & operator += (const Time & time){
		setTime(time.getSecs()+m_secs,time.getUsecs()+m_usecs);
		return *this;
	}

	/** @brief operator -=
	*	calls setTime (normalizes time, so that usec part is less than million)
	*	@note Due to usage of unsigned types may the result of this operation be
	*	greater than original *this time.
	*	@return *this const
	*/
	inline const Time & operator -= (const Time & time){
		if(m_usecs < time.getUsecs()){
			setTime(m_secs - time.getSecs() - 1,(MILLION + m_usecs) - time.getUsecs());
		}else{
			setTime(m_secs - time.getSecs() ,m_usecs - time.getUsecs());
		}
		return *this;
	}

	/** @brief operator ==
	*	compares Time values
	*	Times are equal if both their timestamp (seconds) parts and their
	*	useconds parts are equal.
	*/
	inline bool operator == (const Time & time) const{
		return ((time.getSecs()==m_secs)&&(time.getUsecs() == m_usecs));
	}

	/** @brief operator !=
	*	Times are equal if both their timestamp (seconds) parts and their
	*	useconds parts are equal.
	*	function is wrapper for !(operator==)
	*/
	inline bool operator != (const Time & time) const{
		return !(operator == (time));
	}

	/** @brief Returns true if this is less than other,
	*	otherwise returns false.
	*
	*	Time is less than other if either timestamp (seconds) is smaller
	*	than other's, or timestamp parts are equal and useconds part is less
	*	than other's.
	*/
	inline bool operator < (const Time& other) const {
		return (m_secs < other.m_secs)
				|| ( (m_secs  == other.m_secs) && (m_usecs <  other.m_usecs) );
	}


	/** @brief Returns true if this is greater than other
	*  otherwise returns false.
	*
	*	Time is less than other if either timestamp (seconds) is smaller
	*	than other's, or timestamp parts are equal and useconds part is less
	*	than other's.
	*	@note Implementation uses bool operator < (const Time& other) const ;
	*/
	inline bool operator > (const Time& other) const
		{	return other < *this;	}

	/** @brief Returns true if this is greater than or equals other
	*	otherwise returns false.
	*
	*	Time is greater than or equals other if either timestamp (seconds) is
	*	greater than or equals other's, or timestamp parts are equal and
	*	useconds part is greater than or equals other's.
	*	@note Implementation uses bool operator < (const Time& other) const;
	*/
	bool operator >=(const Time & other) const
		{	return !(operator<(other));	}

	/** @brief Returns true if this is less than or equals other
	*	otherwise returns false.
	*
	*	Time is less than or equals  other if either timestamp (seconds) is
	*	less than or equals other's, or timestamp parts are equal and
	*	useconds part is greater than or equals other's.
	*	@note Implementation uses bool operator > (const Time& other) const;
	*/
	bool operator <=(const Time & time) const
		{	return !(operator>(time)); }

	/** @brief set time to values
	*
	*	normalizes time, so that microsecs part is less than million. I usec parameter is greater
	*	than million, seconds part is increased
	*/
	inline void setTime(unsigned int sec, unsigned int usec){
		m_secs = sec;
		m_usecs = usec;
		correctTime();
	}

	/** @brief Returns new time that is sum of this Time and other Time.
	*
	*	Each component is added separately and then they are normalized.
	*	@note Implementation uses Time& operator += (const Time& other);
	*/

	Time operator + (const Time & time) const
	{
		Time result(m_secs,m_usecs);
		result += time;
		return result;
	};

	/** @brief Returns new time that is equal to this Time - other Time.
	*
	*	Each component is substracted separately and then they are normalized.
	*	@note Implementation uses Time& operator -= (const Time& other);
	*/
	Time operator - (const Time & time) const
	{
		Time result(m_secs,m_usecs);
		result -= time;
		return result;
	};

	/** @brief Gets timestamp part. */
	inline uint secs() const { return m_secs; }

	/** @brief Gets useconds part. */
	inline uint usecs() const { return m_usecs; }

	/** @brief Gets timestamp part. wrapper for usecs*/
	inline unsigned int getSecs() const{return secs();}

	/** @brief Gets useconds part. wrapper for secs*/
	inline unsigned int getUsecs() const{return usecs();}

	/** @brief Gets current time.
	 *
	 * Given time should not differ from actual time by more than 1 second.
	 */
	static Time getCurrentTime();
	/*{
		return Time(Kernel::instance().clock().time(), Kernel::instance().clock().usec());
	}*/

	/** @brief Gets current time.
	*
	* Given time should not differ from actual time by more than 1 second.
	*	wrapper for getCurrentTime()
	*/
	inline static Time getCurrent(){
		return getCurrentTime();
	}

protected:
	/** @brief corrects time
	*
	*	Ensures that microsecond part of time is less than million
	*/
	inline void correctTime(){
		m_secs = m_secs + (m_usecs / MILLION);
		m_usecs = m_usecs%MILLION;
	}

	/** @brief seconds time
	*/
	unsigned int m_secs;
	/** @brief microseconds time
	*/
	unsigned int m_usecs;
};



//------------------------------------------------------------------------------







