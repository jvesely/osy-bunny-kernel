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
 * @brief Class Event definition - the simplest synchronization primitive.
 */

#include "structures/List.h"

class Time;
class Thread;

/*! @brief List of waiting threads. */
typedef List<Thread *> ThreadList;

/*!
 * @class Event Event.h "synchronization/Event.h"
 * 
 * @brief Class Event represents a simplest synchronization primitive managing
 * a queue of threads waiting for some event.
 */
class Event
{
public:
	/*! @brief Default constructor. Just prints some debug output.*/
	Event();

	/*! @brief Default destructor. Just checks if the queue is empty. */
	~Event();

	/*! @brief Adds the currently running thread to the queue and yields it. */
	void wait();

	/*! @brief Adds the currently running thread to the queue, but suspends
	 * it only for the given time.
	 */
	void waitTimeout( const Time& timeout );

	/*! @brief Fires the event, waking up all threads waiting for it. */
	void fire();

	/*! @brief Retrurns whether there are some threads waiting for the event. */
	inline bool waiting();

private:
	/*! @brief Disable copy. */
	Event( const Event& other );

	/*! @brief Disable operator =. */
	Event& operator=( const Event& other );

	/*! @brief Queue of threads waiting for the event. */
	ThreadList m_list;
};

/*----------------------------------------------------------------------------*/

inline bool Event::waiting()
{
	return !m_list.empty();
}
