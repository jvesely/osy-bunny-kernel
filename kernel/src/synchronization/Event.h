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
 *   @version $Id: Event.h 605 2009-01-22 23:58:59Z slovak $
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

#include "structures/List.h"
#include "Time.h"

class Thread;
typedef List<Thread *> ThreadList;

class Event
{
public:
	inline Event() {};
	~Event();

	void wait();
	void waitTimeout( const Time& timeout );
	void fire();

	inline bool waiting();

private:
	/*! @brief Disable copy. */
	Event( const Event& other );

	/*! @brief Disable operator =. */
	Event& operator=( const Event& other );


	ThreadList m_list;
};


inline bool Event::waiting()
{
	return !m_list.empty();
}
