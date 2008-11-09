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
Thread(0,Thread::DEFAULT_STACK_SIZE),
mySemaphore(0)
{
	//printk("TimerManager ctor\n");
	m_lastEvent.setTime( 0, 0 );
	//Scheduler::instance().getId(this);
	Scheduler::instance().enqueue(this);

	//printk("TimerManager ctor OK\n");
}

//------------------------------------------------------------------------------
void TimerManager::deactivateEvent( ClassTimer * tmr ){
	//printk("TimerManager deactivating event");
	assert( tmr != NULL );//this should not happen, because public methods would not allow it
	if ( !tmr->pending() ) return;

	/// \todo heap once
	ListItem<ClassTimer*> * tmrItem = tmr->getEventStruct();

	assert( tmrItem != NULL );
	assert( m_activeEvents.size() > 0 );
	//printk(" Removing and deactivating event\n");

	//removing
	m_activeEvents.remove( tmrItem );

	tmr->setToStopped();
}

//------------------------------------------------------------------------------
void TimerManager::destroyTimer( ClassTimer * tmr ){

	if ( tmr == NULL ) return;
	if ( tmr->pending() ){
		timerMutex.lock();
		deactivateEvent( tmr );
		timerMutex.unlock();
		//nothing more:
		//even if tmr was first event to happen, manager will handle it.
	}
	tmr->deinit();
}

//------------------------------------------------------------------------------
void TimerManager::startEvent( ClassTimer * tmr ){

	if ( tmr == NULL ) return;

	timerMutex.lock();
	//if pending, then get it out of planned events
	if ( tmr->pending() ){
		deactivateEvent( tmr );
	}
	//now it can be only initialised or not initialised (cannot be active)
	if ( tmr->getState() != ClassTimer::TIMER_INITIALISED ) {
		timerMutex.unlock();
		return;
	}

	Time currentTime = Time::getCurrentTime();

	//init listItem
	ListItem<ClassTimer*> *tmrItem = tmr->getEventStruct();
	ListItem<ClassTimer*> *afterItem = m_activeEvents.begin().getItem();

	assert( tmrItem != NULL );

	tmr->setAbsTime( currentTime + tmr->getDelay() );
	tmr->setToStarted();
	//insert listItem: increasing order
	// this routine will not be needed once there is a heap structure
	///\todo heap
	while ( ( afterItem != NULL ) && ( isLater( tmr, afterItem->getData() ) ) )
	{
		afterItem = afterItem->next();
	}


	//now either afterItem==NULL or tmr is before afterItem
	List<ClassTimer*>::Iterator it = m_activeEvents.insert( afterItem, tmrItem );//insertbefore

	if ( it == m_activeEvents.begin() )
	{//is earliest - change nearest time interrupt
		//mySemaphore++;
		//thread_wakeup(id());
		Scheduler::instance().enqueue(this);
	}

	timerMutex.unlock();
}

//------------------------------------------------------------------------------
void TimerManager::run()
{
	Time timeout( 0, 0 );
	//printk("timerManager Thread running\n");
	while ( true ){
		//printk("timerManager Thread in loop\n");
		timerMutex.lock();
		//printk("timerManager Thread locked\n");
		if ( m_activeEvents.empty() ){
			timeout.setTime( 0, 0 );
			//printk("timerManager no events now\n");
		}else{
			//handle events from lastEvent to now
			//printk("timerManager got events \n");
			Time currentTime = Time::getCurrentTime();
			ClassTimer * event;
			List<ClassTimer*>::Iterator eventIt;

			while (
					( (eventIt = m_activeEvents.begin())!=m_activeEvents.end() )	//there are some events and eventIt = begin()
			        &&	( isLater( ( *eventIt )->getAbsTime(), m_lastEvent ) ) 		//is after last timer event
			        &&	( !isLater( ( *eventIt )->getAbsTime(), currentTime ) ))	//is before or equal to current time
			{
				//printk("timerManager handling event");
				//process event
				event = ( *eventIt );
				event->handler();
				//remove event from active list //and find next
				//++eventIt;
				deactivateEvent( event );
				currentTime = Time::getCurrentTime();
			}
			//set last event (or at least 'current' time)
			m_lastEvent = currentTime;
			//plan nearest event
			if ( ! m_activeEvents.empty() )//there are events
			{
				//printk("timerManager preparing new event\n");
				m_nearest = m_activeEvents.getFront()->getAbsTime();
				timeout = m_nearest - currentTime;
			}else{//no events
				//printk("timerManager all events done\n");
				timeout.setTime(0,0);
			};
		}
		//printk("timerManager Thread unlocking\n");
		timerMutex.unlock();
		//it really is not possible to unlock earlier:
		if(timeout==Time(0,0)){
			//thread_yield();
			//dequeue(this);

			thread_suspend();
			//printk("timerManager no events, waiting\n");
			//wait till someone adds an event
			//mySemaphore.down(1);
		}else{
			//wait to nearest event or till somebody adds and event
			//printk("timerManager waiting with time\n");
			if(timeout.getSecs()>Time::MAX_USEC_SECS){
				mySemaphore.downTimeout(1, Time::MAX_USEC_SECS*Time::MILLION);//will wait max allowed time
			}else{
				mySemaphore.downTimeout(1, timeout.getSecs()*Time::MILLION+timeout.getUsecs());
			}
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











