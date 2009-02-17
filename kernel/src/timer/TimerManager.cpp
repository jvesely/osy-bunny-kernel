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
 * @brief TimerManager implementation
 *
 *
 */

#include "api.h"
#include "timer/TimerManager.h"

//------------------------------------------------------------------------------
TimerManager::TimerManager():
		Thread(),
		m_mySemaphore( 0 )
{
	m_lastEvent.setTime( 0, 0 );
	resume();
}

//------------------------------------------------------------------------------
void TimerManager::deactivateEvent( ClassTimer * tmr )
{

	assert( tmr != NULL );//this should not happen, because public methods would not allow it
	if ( !tmr->pending() ) return;//is not started

	//setting to stopped
	tmr->setToStopped();//is not started any more

	assert( m_hactiveEvents.size() > 0);//or equivalent - to be done

	tmr->removeFromHeap();
}
//------------------------------------------------------------------------------
void TimerManager::makeReady(ClassTimer * tmr)
{
	assert( tmr != NULL );//this should not happen, because public methods would not allow it

	//checking state
	if ( !tmr->pending() ) return;//is not started

	//setting to ready
	tmr->setToReady();

	//removing from active
	assert( m_hactiveEvents.size() > 0);
	tmr->removeFromHeap();

	//inserting into ready
	tmr->insertIntoHeap(&m_hreadyEvents,Time(0,0));
}

//------------------------------------------------------------------------------
void TimerManager::executeEvent( ClassTimer * tmr )
{
	assert( tmr != NULL );//this should not happen, because public methods would not allow it

	//checking state
	if ( tmr->getState() != ClassTimer::TIMER_READY_TO_BE_EXECUTED ) return;

	//setting state to initialised
	tmr->setToStopped();

	//removing from ready events
	assert( m_hreadyEvents.size() > 0);//or equivalent - to be done
	tmr->removeFromHeap();

	//executing
	tmr->handler();
}

//------------------------------------------------------------------------------
void TimerManager::destroyTimer( ClassTimer * tmr )
{

	if ( tmr == NULL ) return;
	if ( tmr->pending() ){
		m_timerMutex.lock();
		deactivateEvent( tmr );
		m_timerMutex.unlock();
		//nothing more:
		//even if tmr was first event to happen, manager will handle it.
	}
	tmr->deinit();
}

//------------------------------------------------------------------------------
void TimerManager::startEvent( ClassTimer * tmr )
{
	if ( tmr == NULL ) {
		//printk( "timer is null!!!\n" );
		return;
	}
	//printk( "starting event: locking structure\n" );
	m_timerMutex.lock();
	//if pending, then get it out of planned events
	if ( tmr->pending() ){
		deactivateEvent( tmr );
	}
	//now it can be only initialised or not initialised (cannot be active)
	if ( tmr->getState() != ClassTimer::TIMER_INITIALISED ) {
		//printk( "starting event: unlocking structure - bad event\n" );
		m_timerMutex.unlock();
		return;
	}

	Time currentTime = Time::getCurrentTime();

	//init listItem

	//assert( tmrItem != NULL );
	tmr->setAbsTime( currentTime + tmr->getDelay() );
	//insert listItem: increasing order
	tmr->insertIntoHeap(&m_hactiveEvents,tmr->getAbsTime());

	//now it is ready to be se
	tmr->setToStarted();


	if ( /*it == m_activeEvents.begin()*/ tmr == m_hactiveEvents.topItem() )
	{//is earliest - change nearest time interrupt
		m_timerMutex.unlock();//this is only a workround.
		m_mySemaphore++;
	}else{
		m_timerMutex.unlock();
	}


}

//------------------------------------------------------------------------------
void TimerManager::run()
{
	Time timeout( 0, 0 );
	Time currentTime;
	ClassTimer * event;

	while ( true ){
		m_timerMutex.lock();

		if ( m_hactiveEvents.size() == 0 ){
			timeout.setTime( 0, 0 );
		}else{
			//handle events from lastEvent to now
			currentTime = Time::getCurrentTime();

			//resolving what should be done
			while (
			    ( (event = (ClassTimer*) m_hactiveEvents.topItem()) != NULL )//get from structure and check whether something in structure
			    &&	( isLater( event->getAbsTime(), m_lastEvent ) ) 		//is after last timer event
			    &&	( !isLater( event->getAbsTime(), currentTime ) ) )	//is before or equal to current time
			{
				makeReady(event);

				//set last event (or at least 'current' time)
				currentTime = Time::getCurrentTime();
			}
		}
		m_lastEvent = currentTime;
		m_timerMutex.unlock();

		//executing ready to be done events
		while (m_hreadyEvents.size() > 0 )
		{
			event = (ClassTimer*)m_hreadyEvents.topItem();
			//set state from ready to initialised
			executeEvent(event);
		}

		//plan nearest event
		currentTime = Time::getCurrentTime();
		m_timerMutex.lock();
		//if ( ! m_activeEvents.empty() )//there are events
		if ( m_hactiveEvents.size() > 0 )//there are events
		{
			m_nearest = ((ClassTimer*)m_hactiveEvents.topItem())->getAbsTime();
			timeout = m_nearest - currentTime;
		}else{//no events
			timeout.setTime( 0, 0 );
		};
		m_timerMutex.unlock();

		if ( timeout == Time( 0, 0 ) ){
			//wait till someone adds an event
			m_mySemaphore.down(1);
		}else{
			//wait to nearest event or till somebody adds and event
			m_mySemaphore.downTimeout( 1, timeout );
		}
	}
}


//------------------------------------------------------------------------------












