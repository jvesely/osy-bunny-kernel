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
 * @brief Kernel class implementation.
 *
 * File contains Kernel class implementation.
 */
#include "Kernel.h"
#include "proc/KernelThread.h"
#include "api.h"
#include "devices.h"
#include "tools.h"
#include "InterruptDisabler.h"
#include "timer/Timer.h"
#include "mem/FrameAllocator.h"
#include "mem/TLB.h"
#include "drivers/MsimDisk.h"

//#define KERNEL_DEBUG

#ifndef KERNEL_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...)\
	puts("[ KERNEL_DEBUG ]: ");\
	printf(ARGS);
#endif


static const uint BUNNIES_PER_LINE = 10;
static const uint BUNNY_LINES = 5;
/*! This is our great bunny :) */
static const char* BUNNY_STR[BUNNY_LINES] = {
"|\\   /| ",
" \\|_|/  ",
" /. .\\  ",
"=\\_T_/= ",
" (>o<)  "
};

extern unative_t COUNT_CPU;
extern native_t SIMPLE_LOCK;
extern void* volatile* other_stack_ptr;

Kernel::Kernel() :
	Thread( 0 ),   /* We need no stack. (We use static stack one :) ) */
	m_console( CHARACTER_OUTPUT_ADDRESS, CHARACTER_INPUT_ADDRESS ),
	m_clock( CLOCK )
{
	registerInterruptHandler( &m_console, CHARACTER_INPUT_INTERRUPT );
	registerInterruptHandler( &Timer::instance(), TIMER_INTERRUPT );

	registerExceptionHandler( &m_syscalls, Processor::CAUSE_EXCCODE_SYS );
	registerExceptionHandler( this, Processor::CAUSE_EXCCODE_INT  );
	registerExceptionHandler( this, Processor::CAUSE_EXCCODE_BP   );

	m_status = INITIALIZED;
}
/*----------------------------------------------------------------------------*/
void Kernel::printBunnies( uint count )
{
	for (uint printed = 0; count; count -= printed) {
		printed = min( count, BUNNIES_PER_LINE);
		for (uint j = 0; j < BUNNY_LINES; ++j) {
			for (uint k = 0; k < printed; ++k)
				puts(BUNNY_STR[j]);
			puts("\n");
		}
	}
}
/*----------------------------------------------------------------------------*/
void Kernel::run()
{
	using namespace Processor;
	
	TLB::instance().flush();
	TLB::instance().mapDevices( DEVICES_MAP_START, DEVICES_MAP_START, PAGE_MIN );
	reg_write_status( STATUS_CU0_MASK | STATUS_IM_MASK | STATUS_IE_MASK );
	other_stack_ptr = &m_otherStackTop;

	//printf( "HELLO WORLD! from processor: %d\n", *DORDER_ADDRESS );

	if (*(volatile unative_t*)DORDER_ADDRESS)
		goto sleep;
	
	{
		ASSERT (COUNT_CPU);     /* We need at least one processor to run. */
		
		SIMPLE_LOCK = 0;

		printBunnies( COUNT_CPU );
		printf( "Running on %d processors\n", COUNT_CPU );

		if (COUNT_CPU > 1)
			puts( "Warning: It's nice to have more processors, but we currently support only one.\n" );

		const unative_t cpu_type = reg_read_prid();
		printf( "Running on MIPS R%d000 revision %d.%d \n",
						cpu_type >> CPU_IMPLEMENTATION_SHIFT,
						(cpu_type >> CPU_REVISION_SHIFT) & CPU_REVISION_MASK,
						cpu_type & CPU_REVISION_MASK );

		native_t to = 0;
		const unsigned int start = m_clock.time();
		printf( "Detecting freq..." );
		while (m_clock.time() == start) ;  /* sync time for the current second */

		const native_t from = reg_read_count();
		while (m_clock.time() - (start + 1) < 1) { //1 s
			putc( '.' );
			to = reg_read_count();
			putc( '\b' );
		}
		/* I would use constants here but they would not be used
		 * in any other part of the program and still it's clear what this does.
		 * (counts Mhz :) )
		 */
		m_timeToTicks = (to - from) / 1000000;
		printf( "%d.%d MHz\n", m_timeToTicks, ((to - from) / 1000) % 1000 );
		
		const uintptr_t total_stacks = COUNT_CPU * KERNEL_STATIC_STACK_SIZE;

		printf( "Kernel ends at: %p.\n", &_kernel_end );
		printf( "Stacks(%x) end at: %p.\n",
			total_stacks, (uintptr_t)&_kernel_end + total_stacks );

		// detect memory
		m_physicalMemorySize = getPhysicalMemorySize( (uintptr_t)&_kernel_end + total_stacks );
		printf( "Detected %d MB of accessible memory\n",
			m_physicalMemorySize / (1024 * 1024) );


		// init frame allocator

		FrameAllocator::instance().init( 
			m_physicalMemorySize, ((uintptr_t)&_kernel_end + total_stacks) );

		ASSERT (FrameAllocator::instance().isInitialized());

		attachDisks();
	}
	{
		//init and run the main thread
		thread_t mainThread;
		Thread* main = KernelThread::create(&mainThread, first_thread, NULL, TF_NEW_VMM);
		ASSERT (main);
		main = 0;
		yield();
	}
sleep:
	SIMPLE_LOCK = 0;
	while (true) {
		asm volatile ("wait");
	}
	panic( "Should never reach this.\n" );
}
/*----------------------------------------------------------------------------*/
size_t Kernel::getPhysicalMemorySize(uintptr_t from)
{
	printf( "Probing memory range...(%x)", from );
	
	TLB::instance().switchAsid( 0 );
	const uint32_t MAGIC = 0xDEADBEEF;

	size_t size = 0;
	const size_t range = Processor::pages[Processor::PAGE_512K].size;
	volatile uint32_t* front = (uint32_t*)ADDR_TO_USEG(from);
	volatile uint32_t* back  = (uint32_t *)range - 1;
	volatile char* point = (char*)front;

	ASSERT(front < back);

	while (true) {
		TLB::instance().setMapping(
			(uintptr_t)front, (uintptr_t)point, Processor::PAGE_512K, 0 );
//		printf("Writing to %x-%x.\n", front, back);
		(*front) = MAGIC; //write
		(*back) = MAGIC; //write
		if ( (*front != MAGIC) || (*back != MAGIC) ) break; //check
		size  += range;
		point += range; //add
		TLB::instance().clearAsid( 0 );
	}
	TLB::instance().clearAsid( 0 );
	printk("OK\n");
	return size;
}
/*----------------------------------------------------------------------------*/
void Kernel::exception( Processor::Context* registers )
{
	InterruptDisabler inter;

	const Processor::Exceptions reason = Processor::get_exccode(registers->cause);
	
	if (!Processor::EXCEPTIONS[reason].handler ||
		  !(*Processor::EXCEPTIONS[reason].handler)( registers )
		) {
			printf( "Exception handling for: %s(%u) UNHANDLED or FAILED => KILLING offending thread (%u).\n",
				Processor::EXCEPTIONS[reason].name, reason, Thread::getCurrent()->id());
			Thread::getCurrent()->kill();
		}
	
	if (Thread::shouldSwitch())
		Thread::getCurrent()->yield();
}
/*----------------------------------------------------------------------------*/
bool Kernel::handleException( Processor::Context* registers )
{
	const Processor::Exceptions reason = Processor::get_exccode( registers->cause );

	if (reason == Processor::CAUSE_EXCCODE_INT)
	{
		handleInterrupts( registers );
		return true;
	}

	if (reason == Processor::CAUSE_EXCCODE_BP &&
		!(registers->cause & Processor::CAUSE_BD_MASK) )
	{
  	registers->epc += 4; // go to the next instruction
    return true;
  }
	ASSERT (!"Incorrect exception for this handler!!!");
	return false;
}
/*----------------------------------------------------------------------------*/
void Kernel::registerExceptionHandler( 
	ExceptionHandler* handler, Processor::Exceptions exception)
{
	using namespace Processor;
	const_cast<Exception*>(&EXCEPTIONS[exception])->handler = handler;
}
/*----------------------------------------------------------------------------*/
void Kernel::registerInterruptHandler( InterruptHandler* handler, uint inter)
{
	ASSERT (inter < Processor::INTERRUPT_COUNT);
	m_interruptHandlers[inter] = handler;
}
/*----------------------------------------------------------------------------*/
void Kernel::handleInterrupts( Processor::Context* registers )
{
	using namespace Processor;
	InterruptDisabler inter;

	for (uint i = 0; i < INTERRUPT_COUNT; ++i)
		if (registers->cause & INTERRUPT_MASKS[i]) {
			ASSERT (m_interruptHandlers[i]);
			m_interruptHandlers[i]->handleInterrupt();
		}
}
/*----------------------------------------------------------------------------*/
void Kernel::setTimeInterrupt(const Time& time)
{
	InterruptDisabler interrupts;

	const Time now      = Time::getCurrent();
	const Time relative = ( time > now ) ? time - now : Time( 0, 0 );

	unative_t current   = Processor::reg_read_count();
	const uint usec     = relative.toUsecs();

 	if (time) {
		current = roundUp(current + (usec * m_timeToTicks), m_timeToTicks * 10 * RTC::MILLI_SECOND); // 10 ms time slot
	}
	
	PRINT_DEBUG
		("[%u:%u] Set time interrupt in %u usecs current: %x, planned: %x.\n",
			now.secs(), now.usecs(), usec, current, Processor::reg_read_compare());
	
	Processor::reg_write_compare( current );
}
/*----------------------------------------------------------------------------*/
void Kernel::refillTLB()
{
  InterruptDisabler inter;

  bool success = TLB::instance().refill(
		IVirtualMemoryMap::getCurrent(), Processor::reg_read_badvaddr() );
	
	PRINT_DEBUG ("TLB refill for address: %p (%u) was a %s.\n",
		Processor::reg_read_badvaddr(), Thread::getCurrent()->id(), success ? "SUCESS" : "FAILURE");

  if (!success) {
		printf( "Access to invalid address %p, KILLING offending thread(%u).\n",
			Processor::reg_read_badvaddr(), Thread::getCurrent()->id() );
		ASSERT (Thread::getCurrent());
		Thread::getCurrent()->kill();
	}

	if (Thread::shouldSwitch())
		Thread::getCurrent()->yield();
}
/*----------------------------------------------------------------------------*/
void Kernel::attachDisks()
{
	DiskDevice* disk = new MsimDisk( HDD0_ADDRESS );
	registerInterruptHandler( disk, HDD0_INTERRUPT );
	ASSERT (disk);
	m_disks.pushBack( disk );
}
/*----------------------------------------------------------------------------*/
Time Time::getCurrentTime()
{
	return Time( KERNEL.clock().time(), KERNEL.clock().usec() );
}

