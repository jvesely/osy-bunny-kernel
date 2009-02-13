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
 * @brief Yielding spinlock
 *
 */

#pragma once

#include "synchronization/Spinlock.h"

/** @brief Yielding spinlock
*

*	In contrast to normal spinlock this spinlock "does not" actively wait
*	and	yields it`s quantum until lock is unlocked.
*/
class YieldingSpinLock : public Spinlock
{
private:
	/** @brief wait function
	*
	*	Instead of active waiting, yiedlds.
	*/
	void wait()
	{
		thread_yield();
	}
};
