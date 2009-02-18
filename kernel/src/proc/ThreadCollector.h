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
 * @brief ThreadCollector class.
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
typedef List<Thread*> ThreadList;

/*!
 * @class ThreadCollector ThreadCollector.h "proc/ThreadCollector.h"
 * @brief ThreadCollector collect threads that should be delted but for some reason need to postopne this deletion.
 */
class ThreadCollector: public Singleton<ThreadCollector>
{
public:
	/*!
	 * @brief Adds thread to the bin.
	 * @param thread Thread to add.
	 */
	inline void add( Thread* thread)
		{ thread->append(&m_list); }

	/*!
	 * @brief Deletes all the stored threads.
	 */
	inline void clean();

private:
	ThreadList m_list;   /*!< List of inactive Threads. */

	/*! @brief Nothing here. */
	ThreadCollector() {};
	/*! @brief No copying.   */
	ThreadCollector( const ThreadCollector& );
	/*! @brief No assigning. */
	ThreadCollector& operator = ( const ThreadCollector& );

	friend class Singleton<ThreadCollector>;
};

/*----------------------------------------------------------------------------*/
/* DEFINITIONS ---------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

inline void ThreadCollector::clean()
{
//	printf("Clean: Collector count: %u.\n", m_list.size());
	while (!m_list.empty()) { 
		ListItem<Thread*>* item = m_list.getFront();
		ASSERT (item && (item == item->data()));
		delete item;
	}
//	printf("Clean: Collector count: %u.\n", m_list.size());
}
/*----------------------------------------------------------------------------*/

#define THREAD_BIN ThreadCollector::instance()
