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

#include "EventMap.h"

/*----------------------------------------------------------------------------*/

EventMap::EventMap(): m_nextEventId(1), m_map(HASHMAP_SIZE) 
{
	ASSERT(m_map.getArraySize());
	int ok = m_map.insert(INVALID_ID, NULL);
	ASSERT(ok == EOK);
	ok = 0;
}

/*----------------------------------------------------------------------------*/

event_t EventMap::map( Event* evnt )
{
	int res;
	while ((res = m_map.insert(m_nextEventId, evnt)) == EINVAL)
		m_nextEventId++;
	
	if (res == ENOMEM)
		return INVALID_ID;
	
	return m_nextEventId++;
}
