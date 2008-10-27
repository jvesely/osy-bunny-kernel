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
#include "devices.h"
#include "drivers/Console.h"
#include "drivers/RTC.h"
#include "mem/TLB.h"
#include "mem/Allocator.h"
#include "proc/ItemPool.h"
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
	 * @brief Main method.
	 *
	 * This method should never return from it's call. I initializes all stuff
	 * that needs initializing, except that which is already initialized and
	 * schedules first thread. (not yet)
	 */
	void run();

	/*! returns currentConsole */
	inline const Console& console() const { return m_console; };

	inline ItemPool& pool() { return m_pool; };

	inline thread_t addThread(Thread* thread) 
		{ return m_scheduler->schedule(thread); };

	/*! getter for physicalMemorySize */
	inline size_t physicalMemorySize() const { return m_physicalMemorySize; };

	/*! just a msim wrapper */
	static inline void stop() { Processor::msim_stop(); };

	/*! another msim wrapper */
	static inline void regDump() { Processor::msim_reg_dump(); };

	/*! block processor by falling in infinite loog */
	static inline void block() { while(true) ;; };

	/*! kernel heap alloc */
	void* malloc(size_t size) const;

	/*! kernel heap free */
	void free(void* address) const;

	void inline yield() const { m_scheduler->switchThread(); };

private:
	/*! kernel heap manager */	
	Allocator m_alloc;

	/*! console device */
	Console m_console;

	/*! clock device */
	RTC m_clock;

	/*! store memory size so it does not have to be detected again */
	size_t m_physicalMemorySize;

	/*! TLB managing */
	TLB m_tlb;

	/*! reserve space needed by threads */
	ItemPool m_pool;

	/*! simple scheduler */
	Scheduler* m_scheduler;

	/*! counts accessible memory */
	size_t getPhysicalMemorySize();

	/*! @brief initialize structures
	 *
	 * reset status register to turn on useg mapping
	 * set clock address
	 */
	Kernel();

friend class Singleton<Kernel>;
};

