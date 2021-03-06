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
#include "ExceptionHandler.h"
#include "SyscallHandler.h"
#include "proc/Thread.h"
#include "Time.h"

#include "drivers/Console.h"
#include "drivers/RTC.h"
#include "drivers/Processor.h"

#include "structures/List.h"
#include "tarfs/TarFS.h"

extern void* first_thread(void*);

/*! symbol specified in linker script */
extern uint32_t _kernel_end;

class DiskDevice;

typedef List<DiskDevice*> DiskList;

/*!
 * @class Kernel Kernel.h "Kernel.h"
 * @brief Main class.
 *
 * Kernel will handle all stuff that kernel should :)
 */
class Kernel:
public Singleton<Kernel>, public ExceptionHandler, public Thread
{

public:
	/*!
	 * @brief Bootstrap code member function.
	 *
	 * This function should never return from it's call. I initializes all stuff
	 * that needs initializing, except that which is already initialized and
	 * schedules first thread.
	 */
	void run() __attribute__ ((noreturn));

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

	bool kill() { halt(); return true; };

	/*! @brief Stops execution, returns control to msim. */
	static inline void stop() { Processor::msim_stop(); };
	
	/*! @brief Stops execution, shuts down msim. */
	static inline void halt() { Processor::msim_halt(); };

	/*! @brief Dumps registers of the active processor. */
	static inline void regDump() { Processor::msim_reg_dump(); };

	/*! @brief Blocks processor by falling in infinite loop. */
	static inline void block() 
		{ Processor::save_and_disable_interrupts(); while(true) ;; };

	/*!
	 * @brief Exception handling member function.
	 * @param registers Pointer to the stored registers at the time
	 * when exception occured.
	 */
	void exception( Processor::Context* registers );

	/*! 
	 * @brief Handler for exceptions that are handled directly by the kernel.
	 */
	bool handleException( Processor::Context* registers );

	void registerExceptionHandler( 
		ExceptionHandler* handler, Processor::Exceptions exception );

	/*! @brief Sets interrupt on given time or sooner.
	 * @param time Desired time of interrupt
	 */
	void setTimeInterrupt( const Time& time );

	void refillTLB();
	
	/*! @brief Registers handler for the given interrupt.
	 *	@param handler Run this handler when interrupt occurs.
	 *	@param intn Associate with this interrupt.
	 */
	void registerInterruptHandler( InterruptHandler* handler, uint intn );

	/*! @brief Gets the first disk. */
	inline DiskDevice* disk() { return m_disks.getFront(); };

	inline TarFS* rootFS() { return m_rootFS; };

private:
	Console m_console;                 /*!< Console device.        */
	const RTC m_clock;                 /*!< Clock device.          */
	size_t m_physicalMemorySize;       /*!< Detected memory size.  */	
	uint m_timeToTicks;                /*!< Converting constant.   */
	DiskList m_disks;									 /*!< Disks.                 */
	TarFS* m_rootFS;                   /*!< /.                     */
	SyscallHandler m_syscalls;         /*!< Handles Syscalls.      */
	void printBunnies( uint count );   /*!< @brief Prints BUNNIES. */

	/*! Vector of the Interrupt handlers. */
	InterruptHandler* m_interruptHandlers[Processor::INTERRUPT_COUNT]; 

	/*! @brief Detects accessible memory.
	 *
	 * Detects accesible memory by moving mapping of the first MB.
	 * @return size of detected memory.
	 */
	size_t getPhysicalMemorySize( uintptr_t from );

	/*! @brief Interrupt hanling member function.
	 * @param registers pointer to the stored registers at the time
	 * when interrupt occured.
	 */
	void handleInterrupts( Processor::Context* registers );

	void attachDisks();
	
	/*! @brief Initializes structures.
	 *
	 * Resets status register to turn on useg mapping. Sets clock and console 
	 * addresses. Registers self as the interrupt handler for interrupts.
	 */
	Kernel();

	friend class Singleton<Kernel>;
	friend void* first_thread(void*);
};

#define KERNEL Kernel::instance()
