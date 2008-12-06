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
 * @brief Contains InterruptDisabler convenience class.
 *
 */

#pragma once
#include "drivers/Processor.h"
/*!
 * @class InterruptDisabler InterruptDisabler.h "InterruptDisabler.h"
 * @brief Convience class used for disabling and reenabling interrupts.
 *
 * Class has private operator new so it can be only allocated on stack.
 * Disabling interrupts is done in the constructor and reverting to the 
 * previous state in destructor. Thus this class keeps interrupts disabled 
 * from the point of its creation until it is destroyed 
 * (for example when leaving function it is declared locally in)
 */
class InterruptDisabler
{
public:
	/*! @brief Disables interrupts and stores previous state */
	inline InterruptDisabler()
		{ m_status = Processor::save_and_disable_interrupts(); }

	/*! @brief Revertes interrupt state to match the stored one */
	inline ~InterruptDisabler()
		{ Processor::revert_interrupt_state(m_status); }

private:
	/*! @brief previous interrupt status */
	ipl_t m_status;

	/*! no copying */
	InterruptDisabler(const InterruptDisabler& other);
	
	/*! no assigning */
	const InterruptDisabler& operator=(const InterruptDisabler& other);

	/*! no creation on heap*/
	void* operator new(uint size);

};
