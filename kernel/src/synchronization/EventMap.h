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
 *   @version $Id: EventMap.h 605 2009-01-22 23:58:59Z slovak $
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

#include "structures/HashMap.h"
#include "types.h"
#include "Singleton.h"

class Event;

typedef HashMap<event_t, Event*> EventHashMap;

class EventMap: public Singleton<EventMap>
{
public:
	static const uint HASHMAP_SIZE = 100;
	static const uint INVALID_ID   = 0;


	EventMap();

	inline Event* getEvent( const event_t evnt );

	event_t map( Event* evnt );

	inline void unmap( const event_t evnt );

private:
	/*! @brief Disable copy. */
	EventMap( const EventMap& other );

	/*! @brief Disable operator =. */
	EventMap& operator=( const EventMap& other );


	event_t m_nextEventId;
	EventHashMap m_map;


	friend class Singleton<EventMap>;
};

/*----------------------------------------------------------------------------*/

inline Event* EventMap::getEvent( const event_t evnt )
{
	ASSERT(m_map.exists(evnt));
	return m_map.at(evnt);
}

/*----------------------------------------------------------------------------*/

inline void EventMap::unmap( const event_t evnt )
{
	m_map.erase(evnt);
}
