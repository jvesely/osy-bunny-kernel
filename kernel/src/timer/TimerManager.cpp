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

#include <timer/TimerManager.h>

//------------------------------------------------------------------------------
TimerManager::TimerManager():
		Thread(),
		m_mySemaphore( 0 )
{
	//printk( "TimerManager ctor\n" );//debug
	//printk( "timer size: %x \n", sizeof( ClassTimer ) );
	m_lastEvent.setTime( 0, 0 );
	//Scheduler::instance().getId(this);
	//Scheduler::instance().enqueue( this );
	resume();

	//printk("TimerManager ctor OK\n");
}

//------------------------------------------------------------------------------
void TimerManager::deactivateEvent( ClassTimer * tmr )
{
	//printk( "TimerManager deactivating event\n" );
	assert( tmr != NULL );//this should not happen, because public methods would not allow it
	if ( !tmr->pending() ) return;//is not started

	//setting to stopped
	tmr->setToStopped();//is not started any more

	//removing from structure
	//ListItem<ClassTimer*> * tmrItem = tmr->getEventStruct();

	//assert( tmrItem != NULL );//this should not happen, because tmr is initialised(and started)
	//assert( m_activeEvents.size() > 0 );
	assert( m_hactiveEvents.size() > 0);//or equivalent - to be done

	//removing from event structure
	//m_activeEvents.remove( tmrItem );
	tmr->removeFromHeap();
}
//------------------------------------------------------------------------------
void TimerManager::makeReady(ClassTimer * tmr)
{
	//printk("TimerManager making ready event\n");
	assert( tmr != NULL );//this should not happen, because public methods would not allow it

	//checking state
	if ( !tmr->pending() ) return;//is not started

	//setting to ready
	tmr->setToReady();

	//removing from active
	//ListItem<ClassTimer*> * tmrItem = tmr->getEventStruct();
	//assert( tmrItem != NULL );//this should not happen, because tmr is initialised(and started)
	//assert( m_activeEvents.size() > 0 );
	assert( m_hactiveEvents.size() > 0);
	//m_activeEvents.remove( tmrItem );
	tmr->removeFromHeap();

	//inserting into ready
	//m_readyEvents.pushBack(tmrItem);
	tmr->insertIntoHeap(&m_hreadyEvents,Time(0,0));
}

//------------------------------------------------------------------------------
void TimerManager::executeEvent( ClassTimer * tmr )
{
	//printk("TimerManager executing event\n");
	assert( tmr != NULL );//this should not happen, because public methods would not allow it

	//checking state
	if ( tmr->getState() != ClassTimer::TIMER_READY_TO_BE_EXECUTED ) return;

	//setting state to initialised
	tmr->setToStopped();

	//removing from ready events
	//ListItem<ClassTimer*> * tmrItem = tmr->getEventStruct();
	//assert( tmrItem != NULL );//this should not happen, because tmr is initialised
	//assert( m_readyEvents.size() > 0 );
	assert( m_hreadyEvents.size() > 0);//or equivalent - to be done
	//m_readyEvents.remove( tmrItem );
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
	//ListItem<ClassTimer*> *tmrItem = tmr->getEventStruct();
	//ListItem<ClassTimer*> *afterItem = m_activeEvents.begin().getItem();

	//assert( tmrItem != NULL );
	tmr->setAbsTime( currentTime + tmr->getDelay() );
	//insert listItem: increasing order
	// this routine will not be needed once there is a heap structure
	/*while ( ( afterItem != NULL ) && ( isLater( tmr, afterItem->getData() ) ) )
	{
		afterItem = afterItem->next();
	}*/

	//now either afterItem==NULL or tmr is before afterItem
	//List<ClassTimer*>::Iterator it = m_activeEvents.insert( afterItem, tmrItem );//insertbefore

	tmr->insertIntoHeap(&m_hactiveEvents,tmr->getAbsTime());

	//now it is ready to be se
	tmr->setToStarted();


	if ( /*it == m_activeEvents.begin()*/ tmr == m_hactiveEvents.topItem() )
	{//is earliest - change nearest time interrupt
		//printk( "starting event: unlocking structure and waking up timer thread\n" );
		m_timerMutex.unlock();//this is only a workround.
		//we need to replace semaphore completely by locked variable
		m_mySemaphore++;
	}else{
		//printk( "starting event: unlocking structure\n" );
		m_timerMutex.unlock();
	}


}

//------------------------------------------------------------------------------
void TimerManager::run()
{
	Time timeout( 0, 0 );
	Time currentTime;
	ClassTimer * event;
	//List<ClassTimer*>::Iterator eventIt;


	while ( true ){
		//printk("timerManager Thread in loop\n");
		m_timerMutex.lock();

		//printk("timerManager Thread locked\n");
		if ( m_hactiveEvents.size() == 0 ){
			timeout.setTime( 0, 0 );
			//printk("timerManager no events now\n");
		}else{
			//handle events from lastEvent to now
			//printk("timerManager got events \n");
			currentTime = Time::getCurrentTime();

			//resolving what should be done
			while (
			    //( ( eventIt = m_activeEvents.begin() ) != m_activeEvents.end() )	//there are some events and eventIt = begin()
			    ( (event = (ClassTimer*) m_hactiveEvents.topItem()) != NULL )//get from structure and check whether something in structure
			    //&&	( isLater( ( *eventIt )->getAbsTime(), m_lastEvent ) ) 		//is after last timer event
			    &&	( isLater( event->getAbsTime(), m_lastEvent ) ) 		//is after last timer event
			    //&&	( !isLater( ( *eventIt )->getAbsTime(), currentTime ) ) )	//is before or equal to current time
			    &&	( !isLater( event->getAbsTime(), currentTime ) ) )	//is before or equal to current time
			{
				//printk("timerManager handling event \n");

				//event = ( *eventIt ); //list implementation

				//printk("m_last  = %d:%d \n", m_lastEvent.getSecs(), m_lastEvent.getUsecs());
				//printk("event   = %d:%d \n",event->getAbsTime().getSecs(),event->getAbsTime().getUsecs());
				//printk("current = %d:%d \n", currentTime.getSecs(),currentTime.getUsecs()) ;
				//if ( event->getTmrThis() == NULL ) printk( "event has null tmrThis!!!\n" );

				//ClassTimer * event2 = (ClassTimer*) m_hactiveEvents.topItem();//debug
				//if(event == NULL) printk("This is bad!!!\n");
				//printk("event2   = %d:%d \n",event2->getAbsTime().getSecs(),event2->getAbsTime().getUsecs());

				//change state of event to ready
				makeReady(event);

				//set last event (or at least 'current' time)
				currentTime = Time::getCurrentTime();
			}
		}
		m_lastEvent = currentTime;
		m_timerMutex.unlock();

		//executing ready to be done events
		//while (( eventIt = m_readyEvents.begin() ) != m_readyEvents.end() )
		while (m_hreadyEvents.size() > 0 )
		{
			//event = ( *eventIt );
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
			//printk("timerManager preparing new event\n");
			//m_nearest = m_activeEvents.getFront()->getAbsTime();
			m_nearest = ((ClassTimer*)m_hactiveEvents.topItem())->getAbsTime();
			timeout = m_nearest - currentTime;
		}else{//no events
			//printk("timerManager all events done\n");
			timeout.setTime( 0, 0 );
		};
		m_timerMutex.unlock();

		if ( timeout == Time( 0, 0 ) ){
			//printk( "timerManager no events, waiting\n" );
			//wait till someone adds an event
			m_mySemaphore.down(1);
		}else{
			//wait to nearest event or till somebody adds and event
			//printk("timerManager waiting with time\n");
			//actually still not working
			m_mySemaphore.downTimeout( 1, timeout );
		}
	}
}


//------------------------------------------------------------------------------
//api functions implementation
/*
** @brief initializes timer
*	@param usec delay time
*	@param handler event handler for this timer
*	@param data pointer to data sent to handler routine
*	@note almost wrapper for ClassTimer::init();
*
int timer_init( struct timer *tmr, const unsigned int usec,
				void (*handler)(struct timer *, void *), void *data)
{
	if(!tmr) return EINVAL;
	return drftmr(tmr).init(tmr,usec,handler,data);
}


** @brief start timer
*	inserts timer into kernel structure holding all timed events
*	@note wrapper for Timer->startEvent()
*
void timer_start(struct timer *tmr)
{
	//timermanager instance call startEvent(tmr2Tmr(tmr));
	TimerManager::instance().startEvent(tmr2Tmr(tmr));
}

** @brief safely destroys (deletes) timer
*	is NULL safe
*	@note uses delete operator
*
void timer_destroy(struct timer *tmr)
{
	//timermanager instance call destroyTimer(tmr2Tmr(tmr));
	TimerManager::instance().destroyTimer(tmr2Tmr(tmr));
}

** @brief returns whether tmr is pending*
int timer_pending(struct timer *tmr)
{
	if(!tmr) return (int) false;
	return (int) drftmr(tmr).pending();
}



*/











