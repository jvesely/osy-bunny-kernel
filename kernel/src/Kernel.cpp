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
#include "drivers/MsimDisc.h"

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

extern unative_t COUNT_CPU;

Kernel::Kernel() :
	Thread( 0 ),   /* We don't need no stack. */
	m_console( CHARACTER_OUTPUT_ADDRESS, CHARACTER_INPUT_ADDRESS ),
	m_clock( CLOCK )
{
	registerInterruptHandler( &m_console, CHARACTER_INPUT_INTERRUPT );
	registerInterruptHandler( &Timer::instance(), TIMER_INTERRUPT );

//	registerExceptionHandler( this, Processor::CAUSE_EXCCODE_SYS  );
	registerExceptionHandler( &m_syscalls, Processor::CAUSE_EXCCODE_SYS );
	registerExceptionHandler( this, Processor::CAUSE_EXCCODE_INT  );
	registerExceptionHandler( this, Processor::CAUSE_EXCCODE_ADEL );
	registerExceptionHandler( this, Processor::CAUSE_EXCCODE_ADES );
	registerExceptionHandler( this, Processor::CAUSE_EXCCODE_RI   );
	registerExceptionHandler( this, Processor::CAUSE_EXCCODE_BP   );

	Processor::reg_write_status( 0 );
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

	if (*DORDER_ADDRESS)
		goto sleep;
	{
		printf( "HELLO WORLD! from processor: %d\n", *DORDER_ADDRESS );
		ASSERT (COUNT_CPU);

		registerExceptionHandler( &TLB::instance(), Processor::CAUSE_EXCCODE_TLBL );
		registerExceptionHandler( &TLB::instance(), Processor::CAUSE_EXCCODE_TLBS );
		
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
		while (m_clock.time() == start) ;
		const native_t from = reg_read_count();
		while (m_clock.time() - (start + 1) < 1) { //1 s
			putc( '.' );
			to = reg_read_count();
			putc( '\b' );
		}
		puts( "done" );
		m_timeToTicks = (to - from) / 1000000;

		/* I would use constants here but they would not be used
		 * in any other part of the program and still it's clear what this does.
		 * (counts Mhz :) )
		 */

		printf( "%d.%d MHz\n", m_timeToTicks, (to - from) % 1000 );
		const uintptr_t total_stacks = COUNT_CPU * KERNEL_STATIC_STACK_SIZE;

		// detect memory
		m_physicalMemorySize = getPhysicalMemorySize( (uintptr_t)&_kernel_end + total_stacks );
		printf( "Detected %d MB of accessible memory\n",
			m_physicalMemorySize / (1024 * 1024) );


		// init frame allocator
		printf( "Kernel ends at: %p.\n", &_kernel_end );
		printf( "Stacks(%x) end at: %p.\n",
			total_stacks, (uintptr_t)&_kernel_end + total_stacks );


		MyFrameAllocator::instance().init( 
			m_physicalMemorySize, ((uintptr_t)&_kernel_end + total_stacks) );

		ASSERT (MyFrameAllocator::instance().isInitialized());

		attachDiscs();
		//init and run the main thread
	}
	{
		thread_t mainThread;
		Thread* main = KernelThread::create(&mainThread, first_thread, NULL, TF_NEW_VMM);
		ASSERT (main);
		yield();
	}
sleep:
	while (true) {
		asm volatile ("wait");
	}
	panic( "Should never reach this.\n" );
}
/*----------------------------------------------------------------------------*/
size_t Kernel::getPhysicalMemorySize(uintptr_t from){
	printf( "Probing memory range..." );
	
	TLB::instance().switchAsid( 0 );
	const uint32_t MAGIC = 0xDEADBEEF;

	size_t size = 0;
	const size_t range = 0x100000/sizeof(uint32_t); /* 1MB */
	volatile uint32_t* front = (uint32_t*)ADDR_TO_USEG(from);
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
void Kernel::exception( Processor::Context* registers )
{
	const Processor::Exceptions reason = Processor::get_exccode(registers->cause);
	
	if (Processor::EXCEPTIONS[reason].handler) {
		if (!(*Processor::EXCEPTIONS[reason].handler)( registers )) {
			printf( "Exception handling for: %s(%u) FAILED => TRHEAD KILLED.\n",
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
	const Exceptions reason = get_exccode( registers->cause );

	switch (reason){
		case CAUSE_EXCCODE_INT:
			handleInterrupts( registers );
			break;
		case CAUSE_EXCCODE_SYS:
			return false;
		case CAUSE_EXCCODE_RI:
			panic("exc %x", registers->epc);
		case CAUSE_EXCCODE_ADEL:
		case CAUSE_EXCCODE_ADES:
			return false;
		case CAUSE_EXCCODE_BP:
			if (!(registers->cause & CAUSE_BD_MASK) ) {
				registers->epc += 4; // go to the next instruction
				return true;
			}
			panic("Exception: Break.\n");
		case CAUSE_EXCCODE_TLBL:
		case CAUSE_EXCCODE_TLBS:
		case CAUSE_EXCCODE_TR:
		case CAUSE_EXCCODE_OV:
		case CAUSE_EXCCODE_CPU:
		case CAUSE_EXCCODE_IBE:
		case CAUSE_EXCCODE_DBE:
		default:
			panic( "Called for incorrect exception: %d.\n", reason );
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
	
	PRINT_DEBUG ("TLB refill for address: %p (%u) was a %s.\n",
		Processor::reg_read_badvaddr(), Thread::getCurrent()->id(), success ? "SUCESS" : "FAILURE");

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
	DiscDevice* disc = new MsimDisc( HDD0_ADDRESS );
	registerInterruptHandler( disc, HDD0_INTERRUPT );
	ASSERT (disc);
	m_discs.pushBack( disc );
}
/*----------------------------------------------------------------------------*/
void Kernel::switchTo()
{
	using namespace Processor;
	
	TLB::instance().mapDevices( DEVICES_MAP_START, DEVICES_MAP_START, PAGE_4K);
	
	m_stack = &_kernel_end + (KERNEL_STATIC_STACK_SIZE * (*DORDER_ADDRESS));
	m_stackSize = KERNEL_STATIC_STACK_SIZE;
	m_stackTop = (void*)((uintptr_t)m_stack + m_stackSize - sizeof(Context));
	Context * context = (Context*)(m_stackTop);
	
	/* Pointer to my member function that just calls virtual run()
	 * http://www.goingware.com/tips/member-pointers.html
	 * taking adress converting pointer and dereferencing trick 
	 * was advised by M. Burda
	 */
	void (Thread::*runPtr)(void) = &Thread::start; 
	
	context->ra = *(unative_t*)(&runPtr);  /* return address (run this)       */

	context->a0 = (unative_t)this;         /* the first and the only argument */
	context->gp = ADDR_TO_KSEG0(0);        /* global pointer                  */
	context->status = STATUS_IM_MASK | STATUS_IE_MASK | STATUS_CU0_MASK;
	m_status = INITIALIZED;
	Processor::switch_cpu_context( NULL, &m_stackTop);
}
