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
#include "types.h"
#include "api.h"

/*! @class Object Object.h "Object.h"
 * @brief Parent class adding reference counting functionality.
 */
class Object
{
public:
	/*! @brief Creates class without any reference. */
	Object():m_refCount( 0 ){};

	/*! @brief just checks the count. */
	virtual ~Object() { ASSERT (m_refCount == 0); }

	/*! @brief Increases reference count. */
	inline uint incCount() { return ++m_refCount; };
	/*! @brief Decreases reference count, deletes object if it reaches 0. */
	inline uint decCount() { return (--m_refCount) ? m_refCount : (delete this, 0); };

protected:
	/*! @brief New instance new count. */
	Object( const Object& other) { m_refCount = 0; }

private:
	uint m_refCount;   /*!< @brief Reference count */
};
