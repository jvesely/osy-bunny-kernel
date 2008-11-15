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


#include <timer/ClassTimer.h>
#include <timer/TimerManager.h>
#include <cpp.h>

//------------------------------------------------------------------------------
ClassTimer::ClassTimer()
{
	//m_eventStruct.setData( this );
	m_state = 0;
	m_delay.setTime( 0, 0 );
}

//------------------------------------------------------------------------------
int ClassTimer::init( timer * tmr, const unsigned int usec,
                      void ( *handler )( struct timer *, void * ), void *data )
{
	//printk("ClassTimer::init \n");
	(ClassTimer*) (new ((void*)this) ClassTimer());

	if ( handler == NULL ) return EINVAL;
	m_delay.setTime( 0, usec );
	m_handler = handler;
	m_data = data;
	m_tmrThis = tmr;
	//m_eventStruct.setData( this );
	//abs time not yet to be set
	//m_eventStruct.setNext(NULL);
	//m_eventStruct.setPrev(NULL);
	m_state = TIMER_INITIALISED;
	//if(m_tmrThis == NULL) printk("classtimer: tmrThis is null!!!\n");//debug
	return EOK;
}

//------------------------------------------------------------------------------
void ClassTimer::deinit()
{
	//printk("Calling ClassTimer::deinit()\n");
	m_delay.setTime( 0, 0 );
	m_handler = NULL;
	m_data = NULL;
	m_state = 0;
}
//------------------------------------------------------------------------------

bool ClassTimer::operator < (const HeapItem<ClassTimer*, 4>& other) const
{
	return TimerManager::instance().isLater(((ClassTimer*)(&other))->getAbsTime(), getAbsTime());
}















