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
 *   @version $Id: ClassTimer.h 179 2008-11-09 17:44:12Z dekanek $
 *   @note
 *   Semestral work for Operating Systems course at MFF UK \n
 *   http://dsrg.mff.cuni.cz/~ceres/sch/osy/main.php
 *
 *   @date 2008-2009
 */

/*!
 * @file
 * @brief ClassTimer
 *
 *	class over struct timer. Stores most information about timed event settings.
 */

#pragma once
#include <structures/List.h>
#include <timer/Time.h>


//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
struct timer;

/** @brief class representing timer
*	wrapper over struct timer. Cannot be constructed, can be only initialised and
*	deinitialised.
*	constains ListItem<ClassTimer> with pointer to this(if initialised)\n
*	(should once be inherited from HeapInsertable class)
*/
class ClassTimer
{
public:

	/** @brief magic value indicating initialised timer
	*/
	static const int TIMER_INITIALISED = 0x600DF00D;

	/** @brief magic value indicating started timer
	*/
	static const int TIMER_STARTED = 0xFee1600D;


	/** @brief initialisator
	*	@param usec delay time from start
	*	@param handler event handler
	*	@param data pointer to data (parameter of handler)
	*	@param tmr timer structure - should be equal to this,
	*			or at least tmr2Tmr(tmr) should return this. This is not checked though.
				Such attempt might have unexpected result.
	*	@return error code; if handler == NULL return EINVAL ,returns EOK
	*	sets state to initialised. Can be initialised more times, if not active.
	*	trying to initialize active timer will liead to unspecified behaviour.\n
	*	initializes eventStruct
	*	implementation note: does not allocate anything.
	*	sets tmrThis to tmr
	*/
	int init(timer * tmr,const unsigned int usec,
				void (*handler)(struct timer *, void *), void *data);

	/** @brief deinitialisator
	*	sets values to NULL, state to not-initialised
	*	@note does not remove from list of active events, this is work of TimerManager
	*/
	void deinit();

	/** @brief sets state to Stopped (= initialised)
	*/
	inline void setToStopped(){
		m_state = TIMER_INITIALISED;
	}

	/** @brief sets state to active
	*	Use for two-step starting of an event. Call setAbsTime first.
	*/
	inline void setToStarted(){
		m_state = TIMER_STARTED;
	}

	/** @brief set abs event time
	*	Use for two-step starting of an event. Call this method first.
	*/
	inline void setAbsTime(const Time & time){
		m_absTime = time;
	}

	/** @brief sets state to active and sets event absolute time
	*	One step starting. Is not used, because before event really is started, it must have
	*	it`s absolute time set.
	*/
	inline void setToStarted(const Time & time){
		m_state = TIMER_STARTED;
		m_absTime = time;
	}

	/** @brief gets timer state
		@return TIMER_INITIALISED if initialised, TIMER_STARTED if is started(pending)
			unspecifiead value else
	*/
	inline int getState(){
		return m_state;
	}

	/** @brief get timer delay time
	*/
	inline const Time & getDelay() const {
		return m_delay;
	}

	/** @brief get abs timer event time
	*	If is not started(or even initialized),
		returns unspecified value, but should not have unspecified behaviour.
	*/
	inline const Time & getAbsTime() const{
		return m_absTime;
	}

	/** @brief is pending
	*	returns whether event has state TIMER_STARTED (this should mean, than event is
	*	planned to occur in future...)
	*/
	bool pending() const{
		return m_state == TIMER_STARTED;
	}

	/** @brief event structure item
	*	is ListItem<ClassTimer*> right now. Once there is heap in timer, HeapItem should be used instead.
	*	If timer is initialised, this item should not be connected to any other ListItem.
	*	If timer is started, item should be connected to planed events list.
	*	If timer is not initialised, value is not specified.
	*/
	ListItem<ClassTimer*> * getEventStruct(){
		return &m_eventStruct;
	}

	/** @brief calls event handler
	*	This function is called when this event occurs and calls handler set in init function.
	*	calling this function on uninitialised function vould have unspecified behaviour.
	*/
	void handler(){
		(*m_handler)(m_tmrThis,m_data);
	}

	/** @brief get timer structure of this object
	*	Actually has right now no use, should return this pointer, although might
	*	be set differently in init function.
	*
	*/
	inline timer * getTmrThis(){
		return m_tmrThis;
	}

protected:
	/** @brief forbidden ctor
	*/
	ClassTimer();

	/** @brief time of delay
	*	set in init method
	*/
	Time m_delay;

	/** @brief absolute time of event
	*	Is set in setAbsTime or startEvent.
	*/
	Time m_absTime;

	/** @brief event handler function
	*	this function is cassed when event occurs
	*/
	void (*m_handler)(struct timer *, void *);

	/** @brief data for event handler
	*/
	void * m_data;

	/** @brief state representation
	*/
	int m_state;

	/** @brief representation in internal structure of TimerManager
	*	contains pointer to this if initialised.
	*	If timer is not initialised, contains unspecified values and pointers.
	*/
	ListItem<ClassTimer*> m_eventStruct;

	/** @brief pointer to timer struct containing this
	*/
	timer * m_tmrThis;

};

//------------------------------------------------------------------------------
/** @brief convertor from struct timer to ClassTimer
*/
inline ClassTimer * tmr2Tmr(timer * tmr){
	return (ClassTimer*)tmr;
};

/** @brief dereference of converted pointer to struct timer
*/
inline ClassTimer & drftmr(timer * tmr){
	return *(tmr2Tmr(tmr));
};
//------------------------------------------------------------------------------

