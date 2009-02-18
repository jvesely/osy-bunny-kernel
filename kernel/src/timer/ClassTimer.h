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
 * @brief ClassTimer
 *
 *	class over struct timer. Stores most information about timed event settings.
 */

#pragma once
#include "api.h"
//#include "structures/List.h"
#include "structures/HeapInsertable.h"
#include "Time.h"


struct timer;

/** @brief class representing timer
*
*	wrapper over struct timer. Cannot be constructed, can be only initialised and
*	deinitialised.
*	constains ListItem<ClassTimer> with pointer to this(if initialised)\n
*	(should once be inherited from HeapInsertable class)
*/
class ClassTimer : public HeapInsertable<ClassTimer,Time,4>
{
public:

	/** @brief initializes values
	*
	*	State will be zero, as well as delay.
	*	Does not initialize timer!
	*/
	ClassTimer();


	/** @brief magic value indicating initialised timer
	*
	*	From this state can timer be set only to uninitialised or TIMER_STARTED.
	*/
	static const int TIMER_INITIALISED = 0x600DF00D;

	/** @brief magic value indicating started timer
	*
	*	From this state can timer be set only to TIMER_INITIALISED or
	*	TIMER_READY_TO_BE_EXECUTED.
	*/
	static const int TIMER_STARTED = 0xFee1600D;

	/** @brief magic value indicating ready-to-be-executed state
	*	Flag ready-to-be-executed means, than event has already occured and
	*	should be handled as soon as possible.
	*
	*	From this state can timer be set only to TIMER_INITIALISED.
	*/
	static const int TIMER_READY_TO_BE_EXECUTED = 0xB001F001;


	/** @brief timer initialisator
	*	@param usec delay time from start
	*	@param handler event handler
	*	@param data pointer to data (parameter of handler)
	*	@param tmr timer structure - should be equal to this,
	*			or at least tmr2Tmr(tmr) should return this. This is not checked though.
				Such attempt might have unexpected result.
	*	@return error code; if handler == NULL return EINVAL ,returns EOK
	*
	*	Sets state to initialised. Can be initialised more times, if not active.
	*	Trying to initialize active timer will lead to undefined behaviour.
	*	Initializes eventStruct and sets tmrThis to tmr.\n
	*	implementation note: does not allocate anything
	*/
	int init(timer * tmr,const unsigned int usec,
				void (*handler)(struct timer *, void *), void *data);

	/** @brief gets timer state
		@return TIMER_INITIALISED if initialised, TIMER_STARTED if is started(pending)
			undefined value else
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
	*
	*	If is not started(or even initialized),
	*	returns unspecified value, but should not have unspecified behaviour.
	*/
	inline const Time & getAbsTime() const{
		return m_absTime;
	}

	/** @brief
	*
	*	returns whether event has state TIMER_STARTED (this should mean, than event is
	*	planned to occur in future)
	*/
	bool pending() const{
		return m_state == TIMER_STARTED;
	}

	/** @brief event structure item (list or heap item)
	*
	*	Is ListItem<ClassTimer*>* right now. When there is heap in timer,
	*	HeapItem should be used instead.
	*	If timer is initialised, this item should not be connected to any other ListItem.\n
	*	If timer is started, item should be connected to planed events list.\n
	*	If timer is not initialised, value is not specified.\n
	*	This class is to be child of class List/HeapInsertable, then this function will
	*	have no sense.
	*/
	/*ListItem<ClassTimer*> * getEventStruct(){
		return &m_eventStruct;
	}*/

	/** @brief calls event handler
	*
	*	This function is called when event occurs and calls handler set in init function.
	*	Calling this function on an uninitialised timer has undefined behaviour and
	*	probably would cause an exception.
	*/
	void handler(){
		(*m_handler)(m_tmrThis,m_data);
	}

	/** @brief get timer structure of this object
	*
	*	this should be equal to return value of this function.
	*
	*/
	inline timer * getTmrThis(){
		return m_tmrThis;
	}

	/** @brief event time comparator
	*
	*	Calls TimerManager::isLater function to compare this and other time.
	*/
	virtual bool operator < (const HeapItem<ClassTimer*, 4>& other) const;


protected:
	/** @brief deinitialisator, invalidates object
	*
	*	Sets values to NULL, state to not-initialised. This function should be called
	*	from TimerManager only.
	*	@note does not remove from list of active events, this is work of TimerManager
	*/
	void deinit();

	/** @brief sets state to Stopped (= initialised)
	*
	*	Should be called from TimerManager only.
	*/
	inline void setToStopped(){
		m_state = TIMER_INITIALISED;
	}

	/** @brief sets state to active
	*
	*	Should be called from TimerManager only.
	*	Use for two-step starting of an event. Call setAbsTime before setToStarted.
	*/
	inline void setToStarted(){
		m_state = TIMER_STARTED;
	}

	/** @brief sets state to ready-to-be-executed
	*
	*	Flag ready-to-be-executed means, than event has already occured, is in structure
	*	of ready-to-be-executed events, and should be handled as soon as possible.
	*	Should be called from TimerManager only.
	*/
	inline void setToReady(){
		m_state = TIMER_READY_TO_BE_EXECUTED;
	}

	/** @brief set abs event time
	*
	*	Should be called from TimerManager only.
	*	Use for two-step starting of an event. Call this method first.
	*/
	inline void setAbsTime(const Time & time){
		m_absTime = time;
	}

	/** @brief sets state to active and sets event absolute time
	*
	*	Should be called from TimerManager only.
	*	One step starting.
	*/
	inline void setToStarted(const Time & time){
		m_state = TIMER_STARTED;
		m_absTime = time;
	}

	/** @brief forbidden operator =	*/
	void operator = (const ClassTimer & other){}

	/** @brief time of delay
	*
	*	set in init method
	*/
	Time m_delay;

	/** @brief absolute time of event
	*
	*	Is set in setAbsTime or startEvent.
	*/
	Time m_absTime;

	/** @brief event handler function
	*
	*	This function is called when event occurs. Is set in init member function.
	*/
	void (*m_handler)(struct timer *, void *);

	/** @brief data for event handler
	*
	*	data parameter for event handler function.
	*/
	void * m_data;

	/** @brief state representation	*/
	int m_state;

	/** @brief representation of ths object in internal structure of TimerManager
	*
	*	Contains pointer to this if initialised.
	*	If timer is not initialised, contains undefined values and pointers.\n
	*	If timer is initialised, contains pointer to this as data and is disconnected from
	*		any data structure.\n
	*	If timer is started, should be connected to structure of planned timer events.
	*/
	//ListItem<ClassTimer*> m_eventStruct;

	/** @brief pointer to timer struct containing this*/
	timer * m_tmrThis;

	friend class TimerManager;

};

