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
//#include "mem/Allocator.h"
#include "mem/BasicMemoryAllocator.h"

/*! symbol specified in linker script */
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
	 * @brief Bootstrap code member function.
	 *
	 * This function should never return from it's call. I initializes all stuff
	 * that needs initializing, except that which is already initialized and
	 * schedules first thread.
	 */
	void run();

	/*!
	 * @brief Gets current I/O device.
	 * @return Console I/O device.
	 */
	inline Console& console() { return m_console; };

	/*!
	 * @brief Gets current clock device.
	 * @return RTC class device.
	 */
	inline const RTC& clock() { return m_clock; };

	/*! 
	 * @brief Gets detected physicalMemorySize.
	 * @return Size of availale RAM.
	 */
	inline size_t physicalMemorySize() const 
		{ return m_physicalMemorySize; };

	/*! @brief Stops execution, returns control to msim. */
	static inline void stop() { Processor::msim_stop(); };
	
	/*! @brief Stops execution, shuts down msim. */
	static inline void halt() { Processor::msim_stop(); };

	/*! @brief Dumps registers of the active processor. */
	static inline void regDump() { Processor::msim_reg_dump(); };

	/*! @brief Blocks processor by falling in infinite loop. */
	static inline void block() 
		{ Processor::save_and_disable_interrupts(); while(true) ;; };

	/*! @brief Kernel heap alloc.
	 * @param size requested size
	 * @return adress to the block of given size, NULL on failure
	 */
	void* malloc( size_t size );// const;

	/*! @brief Kernel heap free.
	 * @param address adress of the returned block
	 */
	void free( const void* address );// const;

	/*!
	 * @brief Exception handling member function.
	 * @param registers pointer to the stored registers at the time
	 * when exception occured.
	 */
	void handle(Processor::Context* registers);

	/*! @brief Sets interrupt on given time or sooner.
	 * @param time Desired time of interrupt
	 */
	void setTimeInterrupt( const Time& time );

	void refillTLB();

	inline TLB& tlb() { return m_tlb; }

private:
	/*! @brief Prints requested number of BUNNIES */
	void printBunnies( uint count );

	/*! kernel heap manager */
	BasicMemoryAllocator m_alloc;

	/*! console device */
	Console m_console;

	/*! clock device */
	const RTC m_clock;

	/*! store memory size so it does not have to be detected again */
	size_t m_physicalMemorySize;

	/*! TLB managing */
	TLB m_tlb;

	/*! converting constant */
	uint m_timeToTicks;

	/*! @brief Detects accessible memory.
	 *
	 * Detects accesible memory by moving mapping of the first MB.
	 * @return size of detected memory.
	 */
	size_t getPhysicalMemorySize(uintptr_t from);

	/*! @brief Interrupt hanling member function.
	 * @param registers pointer to the stored registers at the time
	 * when interrupt occured.
	 */
	void handleInterrupts(Processor::Context* registers);

	/*! @brief Initializes structures.
	 *
	 * Resets status register to turn on useg mapping.
	 * Sets clock and console addresses.
	 */
	Kernel();

friend class Singleton<Kernel>;
};
