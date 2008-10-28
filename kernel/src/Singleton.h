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
 * @brief Singleton template class.
 *
 * Singleton template class impementation. Singleton tmeplate is parent to all future 
 * singletons.
 */

/*! @class Singleton Singleton.h "Singleton.h"
 * @brief parent to all singletons.
 *
 * Template class, uses type T. provides metod instance that returns reference 
 * to the static instance of type T.
 */
#pragma once

template<typename T>
class Singleton
{
public:

	/*! @return reference to static instance of the class */
	static T& instance()
	{
		static T it;
		return it;
	}

protected:
	/*! should not be constructed by onyone but children */
	inline Singleton(){};

	/*! no contruction no destructiono */
	virtual ~Singleton(){};
private:
	/*! no copies of Singleton */
	Singleton(const Singleton& other);

	/*! no assigning of Singleton */
	Singleton& operator=(const Singleton& other);

};
