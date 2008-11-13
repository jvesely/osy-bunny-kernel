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
 * @brief Timer Mananger
 *
 *	TimerManager stores information about time-planned events.
 */

#pragma once

#include <Kernel.h>
#include <proc/Scheduler.h>
#include <structures/List.h>
#include <synchronization/Mutex.h>
#include <synchronization/Semaphore.h>
#include <timer/Time.h>
#include <timer/ClassTimer.h>



/** @brief class handling timed events
*
*	This class is not handling thread dependent timed events, such as timed mutexes.\n
*	For planning events is used semaphore, one mutex is used for locking event structure.\n
*	Current implementation is using sorted linked list, once should be implemented with
*	heap.
*	TimerManager is thread, which waits for nearest event (timed semaphore) or until somebody
*	sets new timed event. When an event occurs, it is processed in this thread.
*	@note each member function, which modifies active events list locks and unlocks this list.
*/
class TimerManager: public Thread , public Singleton<TimerManager>
{
public:
	/** @brief
	*
	*	Sets only basic settings and schedules self. Presets semaphore to zero and
	*	m_lastEvent to Time(0,0).
	*/
	TimerManager();

	/** @brief plans event
	*	@param tmr structure
	*
	*	Inserts event to active events list.\n
	*	If tmr already is active, it is restarted = deactivated and activated again.
	*	Is null safe. If tmr is not initialised, nothing happens.
	*	@note wakes up this thread if needed and lets him to handle new timed event
	*/
	void startEvent(ClassTimer * tmr);

	/** @brief destroy timer
	*
	*	removes from active events and deinitializes it (see ClassTimer.deinit()).
	*	If tmr is not active, it is only deinitialized. If tmr is not even initialized,
	*	it is deinitialized as well (this operation is safe).\n
	*	Function is NULL safe.
	*	@note does not wake up this thread
	*/
	void destroyTimer(ClassTimer * tmr);

	/** @brief main thread method
	*
	*	Waits in cycles for timed semaphore (timeout is set to nearest timed event).
	*	If new event, set in other thread, should occur earlier than curren earliest event,
	*	thread is woken up, as well as in case timeout passes.\n
	*	When woken up, handles all events between last handled event and current time.
	*	Last event time is then set to current time and new timeout is set. Thread then
	*	waits again for new event or until timeout passes.\n
	*	If needed timeout is greater than maximum timeout for semaphore, maximum timeout
	*	for semaphore is used (Time::MAX_USECS_SECS). TimerManager then waits for next
	*	wakeup.
	*/
	void run();


protected:
	/** @brief deactivates event
	*
	*	Does not invalidate it by classing tmr.deinit().\n
	*	Does not lock or unlock anything
	*	(this should be done by member functions calling this one).
	*	If tmr is not in started, nothing happpens, even if it has state
	*	TIMER_READY_TO_BE_EXECUTED (once event occured, it`s handling is inevitable).
	*
	*	In event state semantics, correctly changes state TIMER_STARTED to
	*	TIMER_INITIALISED.
	*/
	void deactivateEvent(ClassTimer * tmr);

	/** @brief handles event and deactivates
	*
	*	If event has not state TIMER_READY_TO_BE_EXECUTED nothing happens.
	*	Should be called only from context of TimerManager thread.
	*	Does not lock anything, byt event handler can, must be called with open m_timerMutex lock.
	*	After execution of event handler is event deactivated (removed from ready events
	*	and set back to initialised state).\n
	*
	*	In event state semantics, correctly changes state TIMER_READY_TO_BE_EXECUTED to
	*	TIMER_INITIALISED.
	*/
	void executeEvent(ClassTimer * tmr);

	/** @brief makes an active event ready to be executed
	*
	*	Moves event from active events to ready events and sets it`s state
	*	to TIMER_READY_TO_BE_EXECUTED.
	*	If event is not active, nothing happens.
	*	Does not lock or unlock anything.
	*
	*	In event state semantics, correctly changes state TIMER_STARTED to
	*	TIMER_READY_TO_BE_EXECUTED.
	*/
	void makeReady(ClassTimer * tmr);

	/** @brief a is later than b
	*
	*	compares times accoding to m_lastEvent//current time.
	*	Time is later than another, if it should occur from current time later than other,
	*	including unsigned int owerflow. That means:\n
	*	if time is greater or equal(>=) current time and other time is less(<)
	*	if time is greater than other time and both are greater or less than current time
	*/
	inline bool isLater(const Time &a,const Time &b){
		Time currentTime = m_lastEvent;//Time::getCurrentTime();
		if((a>=currentTime)&&(b<currentTime)) return false;
		if((a<currentTime)&&(b>=currentTime)) return true;
		return a>b;
	}

	/** @brief tmr event a is later than tmr event b
	*
	*	calls isLater for absolut time of events
	*/
	inline bool isLater(ClassTimer * tmr1, ClassTimer * tmr2){
		return isLater(tmr1->getAbsTime(),tmr2->getAbsTime());
	}

	/** @brief event structure lock
	*
	*	Locks access to active events list and m_nearest.
	*	Should be locked before acess to m_activeEvents and m_nearest.
	*/
	Mutex m_timerMutex;

	/** @brief semaphore used for time planning
	*
	*	Is locked by this thread and unlocked by other threads
	*/
	Semaphore m_mySemaphore;

	/** @brief structure holding planned events
	*
	*	Events are inserted (and sometimes removed - see destroyTimer) by other threads,
	*	removed by this thread when they are executed.
	*/
	List<ClassTimer*> m_activeEvents;

	/** @brief structure holding ready-to-be-executed events
	*
	*	Events in this list have already occured, but have not been executed yet.
	*	They are ready to be executed.
	*/
	List<ClassTimer*> m_readyEvents;


	/** @brief time of last event
	*
	*	All events between m_lastEvent and current time are processed when this thread is woken up.\n
	*	Of course we use absolut time, not relative to current time.
	*/
	Time m_lastEvent;

	/** @brief time of nearest planed event
	*
	*	Of course we use absolut time, not relative to current time.
	*/
	Time m_nearest;

};





