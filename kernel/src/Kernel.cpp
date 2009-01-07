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
#include "proc/UserThread.h"
#include "api.h"
#include "devices.h"
#include "tools.h"
#include "InterruptDisabler.h"
#include "timer/Timer.h"
#include "mem/FrameAllocator.h"
#include "SysCallHandler.h"
#include "mem/TLB.h"
#include "drivers/DiscDevice.h"

//#define KERNEL_DEBUG

#ifndef KERNEL_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...)\
	puts("[ KERNEL_DEBUG ]: ");\
	printf(ARGS);
#endif


/*! This is our great bunny :) */
static const char* BUNNY_STR[5] = {
"|\\   /| ",
" \\|_|/  ",
" /. .\\  ",
"=\\_T_/= ",
" (>o<)  "
};

static const uint BUNNIES_PER_LINE = 10;
static const uint BUNNY_LINES = 5;



Kernel::Kernel() :
	m_console(CHARACTER_OUTPUT_ADDRESS, CHARACTER_INPUT_ADDRESS), m_clock(CLOCK)
{
	Processor::reg_write_status(0);

	registerInterruptHandler( &m_console, CHARACTER_INPUT_INTERRUPT );
	registerInterruptHandler( &Timer::instance(), TIMER_INTERRUPT );

	registerExceptionHandler( this, Processor::CAUSE_EXCCODE_SYS );
	registerExceptionHandler( this, Processor::CAUSE_EXCCODE_INT );
	//registerExceptionHandler( this, Processor::CAUSE_EXCCODE_TLBL );
	//registerExceptionHandler( this, Processor::CAUSE_EXCCODE_TLBS );
	registerExceptionHandler( this, Processor::CAUSE_EXCCODE_ADEL );
	registerExceptionHandler( this, Processor::CAUSE_EXCCODE_ADES );
	registerExceptionHandler( this, Processor::CAUSE_EXCCODE_RI );
	registerExceptionHandler( this, Processor::CAUSE_EXCCODE_BP );
}
extern void* test(void*);

extern unative_t COUNT_CPU;
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

	TLB::instance().mapDevices( DEVICES_MAP_START, DEVICES_MAP_START, PAGE_4K);
	registerExceptionHandler( &TLB::instance(), Processor::CAUSE_EXCCODE_TLBL );
	registerExceptionHandler( &TLB::instance(), Processor::CAUSE_EXCCODE_TLBS );

	puts("HELLO WORLD!\n");

	ASSERT (COUNT_CPU);

	printBunnies( COUNT_CPU );
	printf("Running on %d processors\n", COUNT_CPU);

	if (COUNT_CPU > 1)
		puts("Warning: It's nice to have more processors, but we currently support only one.\n");

	const unative_t cpu_type = reg_read_prid();
	printf("Running on MIPS R%d000 revision %d.%d \n",
	        cpu_type >> CPU_IMPLEMENTATION_SHIFT,
	        (cpu_type >> CPU_REVISION_SHIFT) & CPU_REVISION_MASK,
				  cpu_type & CPU_REVISION_MASK );

	native_t to = 0;
	const unsigned int start = m_clock.time();
	printf("Detecting freq....");
	while (m_clock.time() == start) ;
	const native_t from = reg_read_count();
	while (m_clock.time() - (start + 1) < 1) { //1 s
		printf("\b.");
		to = reg_read_count();
	}
	m_timeToTicks = (to - from) / 1000000;

	/* I would use constants here but they would not be used
	 * in any other part of the program and still it's clear what this does.
	 * (counts Mhz :) )
	 */

	printf("%d.%d MHz\n", m_timeToTicks, (to - from) % 1000 );
	uintptr_t total_stacks = COUNT_CPU * KERNEL_STATIC_STACK_SIZE;

	// detect memory
	m_physicalMemorySize = getPhysicalMemorySize((uintptr_t)&_kernel_end + total_stacks);
	printf("Detected %d MB of accessible memory\n", m_physicalMemorySize / (1024 *1024) );


	// init frame allocator
		printf("Kernel ends at: %p.\n", &_kernel_end );
		printf("Stacks(%x) end at: %p.\n", total_stacks, (uintptr_t)&_kernel_end + total_stacks);

	MyFrameAllocator::instance().init( 
		m_physicalMemorySize, ((uintptr_t)&_kernel_end + total_stacks) );

//	printf("Frame allocator initialized: %s\n",
//		(MyFrameAllocator::instance().isInitialized()) ? "Yes" : "No" );
	//m_alloc.setup(ADDR_TO_KSEG0(m_physicalMemorySize - 0x100000), 0x100000);
	ASSERT(MyFrameAllocator::instance().isInitialized());

	//init and run the main thread
	thread_t mainThread;
	Thread* main = KernelThread::create(&mainThread, test, NULL, TF_NEW_VMM);
	ASSERT (main);
	main->switchTo();

	panic("Should never reach this.\n");
}
/*----------------------------------------------------------------------------*/
size_t Kernel::getPhysicalMemorySize(uintptr_t from){
	printf("Probing memory range...");
	TLB::instance().switchAsid( 0 );
	const uint32_t MAGIC = 0xDEADBEEF;

	size_t size = 0;
	const size_t range = 0x100000/sizeof(uint32_t); /* 1MB */
	volatile uint32_t* front = (uint32_t*)(ADDR_TO_USEG(from) );
	volatile uint32_t* back = (uint32_t *)range - 1;
	volatile uint32_t* point = front;

	while (true) {
		TLB::instance().setMapping(
			(uintptr_t)front, (uintptr_t)point, Processor::PAGE_1M, false );
		(*front) = MAGIC; //write
		(*back) = MAGIC; //write
		if ( (*front != MAGIC) || (*back != MAGIC) ) break; //check
		size += range * sizeof(uint32_t);
		point += range; //add
	}
	printk("OK\n");
	TLB::instance().clearAsid( 0 );
	return size;
}
/*----------------------------------------------------------------------------*/
void* Kernel::malloc(const size_t size) //const
{
	PRINT_DEBUG ("Malloc request for: %u.\n", size);
	void* ret =  m_alloc.getMemory(size);
	PRINT_DEBUG ("Malloc %u %p.\n", size, ret);
	return ret;
}
/*----------------------------------------------------------------------------*/
void Kernel::free(const void * address) //const
{
	PRINT_DEBUG ("Free %p.\n", address);
	if (address)
		m_alloc.freeMemory(address);
}
/*----------------------------------------------------------------------------*/
void Kernel::exception( Processor::Context* registers )
{
	const Processor::Exceptions reason = Processor::get_exccode(registers->cause);
	if (Processor::EXCEPTIONS[reason].handler) {
		if (!(*Processor::EXCEPTIONS[reason].handler)( registers )) {
			printf( "Exception handling for EXCEPTION: %s(%u) FAILED => TRHEAD KILLED.\n",
				Processor::EXCEPTIONS[reason].name, reason);
			Thread::getCurrent()->kill();
		}
	} else {
		panic("Unhandled exception(%u) %s.\n", 
			reason, Processor::EXCEPTIONS[reason].name );
	}
}
/*----------------------------------------------------------------------------*/
bool Kernel::handleException( Processor::Context* registers )
{
	using namespace Processor;
	const Exceptions reason = get_exccode(registers->cause);

	switch (reason){
		case CAUSE_EXCCODE_INT:
			handleInterrupts( registers );
			break;
		case CAUSE_EXCCODE_SYS:
			SysCallHandler( registers ).handle();
			break;
		case CAUSE_EXCCODE_TLBL:
		case CAUSE_EXCCODE_TLBS:
			refillTLB();
			break;
		case CAUSE_EXCCODE_ADEL:
		case CAUSE_EXCCODE_ADES:
			return false;
			printf("Exception: Address error exception. THREAD KILLED\n");
			Thread::getCurrent()->kill();
			panic("Exception: Address error exception.\n");
		case CAUSE_EXCCODE_BP:
			if (!(registers->cause & CAUSE_BD_MASK) ) {
				registers->epc += 4; // go to the next instruction
				return true;
			}
			panic("Exception: Break.\n");
		case CAUSE_EXCCODE_TR:
			panic("Exception: Conditional instruction.\n");
		case CAUSE_EXCCODE_OV:
			panic("Exception: Arithmetic overflow.\n");
		case CAUSE_EXCCODE_CPU:
			panic("Exception: Coprocessor unusable.\n");
		case CAUSE_EXCCODE_RI:
			return false;
			printf("Exception: Reserved Instruction exception. THREAD KILLED\n");
			Thread::getCurrent()->kill();
			panic("Exception: Reserved Instruction.\n");
		case CAUSE_EXCCODE_IBE:
		case CAUSE_EXCCODE_DBE:
			panic("Exception: Invalid address.\n");
		default:
			panic("Exception: Unknown.\n");
	}
	return true;
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
	
	if (Thread::shouldSwitch())
		Thread::getCurrent()->yield();

}
/*----------------------------------------------------------------------------*/
void Kernel::setTimeInterrupt(const Time& time)
{
	InterruptDisabler interrupts;

	const Time now = Time::getCurrent();
	const Time relative = ( time > now ) ? time - now : Time( 0, 0 );

	unative_t current = Processor::reg_read_count();
	const uint usec = relative.toUsecs();

 	if (time) {
		current = roundUp(current + (usec * m_timeToTicks), m_timeToTicks * 10 * RTC::MILLI_SECOND);
	}
	
	Processor::reg_write_compare( current );
	
	PRINT_DEBUG
		("[%u:%u] Set time interrupt in %u usecs current: %x, planned: %x.\n",
			now.secs(), now.usecs(), usec, current, Processor::reg_read_compare());
}
/*----------------------------------------------------------------------------*/
void Kernel::refillTLB()
{
  InterruptDisabler inter;

  bool success = TLB::instance().refill(
		IVirtualMemoryMap::getCurrent().data(), Processor::reg_read_badvaddr());
	
	PRINT_DEBUG ("TLB refill for address: %p was a %s.\n",
		Processor::reg_read_badvaddr(), success ? "SUCESS" : "FAILURE");

  if (!success) {
		printf( "Access to invalid address %p, KILLING offending thread.\n",
			Processor::reg_read_badvaddr() );
    if (Thread::getCurrent())
			Thread::getCurrent()->kill();
		else
			panic( "No thread and invalid tlb refill.\n" );
	}
}
/*----------------------------------------------------------------------------*/
void Kernel::attachDiscs()
{
	m_discs = new DiscDevice( 
		HDD0_DATA_ADDR, HDD0_SEC_NO_ADDR, HDD0_STATUS_ADDR, HDD0_BLOCK_SIZE);
}
