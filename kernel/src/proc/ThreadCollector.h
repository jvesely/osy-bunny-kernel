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

#pragma once
#include "Singleton.h"
#include "structures/List.h"
#include "api.h"

class Thread;
//class ListInsertable;
typedef List<Thread*> ThreadList;


class ThreadCollector: public Singleton<ThreadCollector>
{
public:
	inline void add( Thread* thread)
		{ thread->append(&m_list); }

	inline void clean();

private:
	ThreadList m_list;

	ThreadCollector() {};
	ThreadCollector( const ThreadCollector& );
	ThreadCollector& operator = ( const ThreadCollector& );

	friend class Singleton<ThreadCollector>;
};

#define THREAD_BIN ThreadCollector::instance()

inline void ThreadCollector::clean()
{
	while (!m_list.empty()) { 
		ListItem<Thread*>* item = m_list.removeFront();
		ASSERT (item && (item == item->data()));
		delete item;
	}
}
/*----------------------------------------------------------------------------*/

