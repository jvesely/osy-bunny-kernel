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
 * @brief Kernel class declaration.
 *
 * Central class of the entire project. Hope it works well.
 */
#pragma once

#include "Singleton.h"
#include "drivers/Console.h"
#include "drivers/RTC.h"
#include "mem/TLB.h"
#include "mem/Allocator.h"
#include "proc/Scheduler.h"

/*! symbol speciefied in linker script */
extern uint32_t _kernel_end;

/*!
 * @class Kernel Kernel.h "Kernel.h"
 * @brief Main class.
 *
 * Kernel will handle all stuff that kernel should :)
 */
class Kernel:public Singleton<Kernel>
{

public:
	/*! 
	 * @brief Main member function.
	 *
	 * This function should never return from it's call. I initializes all stuff
	 * that needs initializing, except that which is already initialized and
	 * schedules first thread. 
	 */
	void run();

	/*! @return Console IO device */
	inline Console& console() { return m_console; };

	/*! @return pool of Listitems used by threads in scheduler and mutex */
//	inline ItemPool& pool() { return m_pool; };

	inline Scheduler& scheduler() { return Scheduler::instance(); };

	inline const RTC& clock() { return m_clock; };

	/*! getter for physicalMemorySize */
	inline size_t physicalMemorySize() const 
		{ return m_physicalMemorySize; };

	/*! just a msim wrapper */
	static inline void stop() { Processor::msim_stop(); };

	static inline void halt() { Processor::msim_halt(); };

	/*! another msim wrapper */
	static inline void regDump() { Processor::msim_reg_dump(); };

	/*! block processor by falling in infinite loog */
	static inline void block() 
		{ Processor::save_and_disable_interrupts(); while(true) ;; };

	/*! @brief Kernel heap alloc.
	 * @param size requested size
	 * @return adress to the block of given size, NULL on failure
	 */
	void* malloc( size_t size ) const;

	/*! @brief Kernel heap free.
	 * @param address adress of the returned block
	 */
	void free( const void* address ) const;

	void handle(Processor::Context* registers);

	void setTimeInterrupt( const Time& time );
private:
	/*! kernel heap manager */	
	Allocator m_alloc;

	/*! console device */
	Console m_console;

	/*! clock device */
	const RTC m_clock;

	/*! store memory size so it does not have to be detected again */
	size_t m_physicalMemorySize;

	/*! TLB managing */
	TLB m_tlb;

	uint m_timeToTicks;

	/*! @brief Detects accessible memory.
	 *
	 * Detects accesible memory by moving mapping of the first MB.
	 * @return size of detected memory.
	 */
	size_t getPhysicalMemorySize();
	
	void handleInterrupts(Processor::Context* registers);

	/*! @brief initialize structures
	 *
	 * reset status register to turn on useg mapping
	 * sets clock and console
	 */
	Kernel();

friend class Singleton<Kernel>;
};
