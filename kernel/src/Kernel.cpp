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
#include "SysCall.h"

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
	m_console(CHARACTER_OUTPUT_ADDRESS, CHARACTER_INPUT_ADDRESS), m_clock(CLOCK) {
	Processor::reg_write_status(0);
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

	m_tlb.mapDevices( DEVICES_MAP_START, DEVICES_MAP_START, PAGE_4K);

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


	Timer::instance();

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
	m_tlb.switchAsid( 0 );
	const uint32_t MAGIC = 0xDEADBEEF;

	size_t size = 0;
	const size_t range = 0x100000/sizeof(uint32_t); /* 1MB */
	volatile uint32_t* front = (uint32_t*)(ADDR_TO_USEG(from) );
	volatile uint32_t* back = (volatile uint32_t *)( (range - 1) * sizeof(uint32_t) );
	volatile uint32_t* point = front;


	while (true) {
		m_tlb.setMapping((uintptr_t)front, (uintptr_t)point, Processor::PAGE_1M, 0);
	//	PRINT_DEBUG( "Mapped %x to %x range = %d kB.\n", front, point, (range * sizeof(uint32_t)/1024) );

		(*front) = MAGIC; //write
		(*back) = MAGIC; //write
	//	PRINT_DEBUG("Proof read %x:%x %x:%x\n", front, *front, back, *back);
		if ( (*front != MAGIC) || (*back != MAGIC) ) break; //check
		size += range * sizeof(uint32_t);
		point += range; //add
	}
	printk("OK\n");
	m_tlb.clearAsid( 0 );
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
void Kernel::handle(Processor::Context* registers)
{
	using namespace Processor;
	const Exceptions reason = get_exccode(registers->cause);

	switch (reason){
		case CAUSE_EXCCODE_INT:
			handleInterrupts(registers);
			break;
		case CAUSE_EXCCODE_SYS:
			SysCall( registers ).handle();
			registers->epc += 4;
//			panic("Syscall.\n");
			break;
		case CAUSE_EXCCODE_TLBL:
		case CAUSE_EXCCODE_TLBS:
			refillTLB();
			break;
		case CAUSE_EXCCODE_ADEL:
		case CAUSE_EXCCODE_ADES:
			printf("Exception: Address error exception. THREAD KILLED\n");
			Thread::getCurrent()->kill();
			panic("Exception: Address error exception.\n");
		case CAUSE_EXCCODE_BP:
			if (!(reason & CAUSE_BD_MASK) ) {
				registers->epc += 4; // go to the next instruction
				break;
			}
			panic("Exception: Break.\n");
		case CAUSE_EXCCODE_TR:
			panic("Exception: Conditional instruction.\n");
		case CAUSE_EXCCODE_OV:
			panic("Exception: Arithmetic overflow.\n");
		case CAUSE_EXCCODE_CPU:
			panic("Exception: Coprocessor unusable.\n");
		case CAUSE_EXCCODE_RI:
			printf("Exception: Reserved Instruction exception. THREAD KILLED\n");
			Thread::getCurrent()->kill();
			panic("Exception: Reserved Instruction.\n");
		case CAUSE_EXCCODE_IBE:
		case CAUSE_EXCCODE_DBE:
			panic("Exception: Invalid address.\n");
		default:
			panic("Exception: Unknown.\n");
	}
}
/*----------------------------------------------------------------------------*/
void Kernel::handleInterrupts(Processor::Context* registers)
{
	using namespace Processor;
	InterruptDisabler inter;

	if (registers->cause & CAUSE_IP1_MASK) { //keyboard
		m_console.interrupt();
	}

	if (registers->cause & CAUSE_IP7_MASK) { //timer interrupt
		reg_write_cause(0);
		Timer::instance().interupt();
	}

	if (Thread::shouldSwitch())
		Thread::getCurrent()->yield();

}
/*----------------------------------------------------------------------------*/
void Kernel::setTimeInterrupt(const Time& time)
{
	using namespace Processor;
	InterruptDisabler interrupts;

	Time now = Time::getCurrent();
	Time relative = ( time > now ) ? time - now : Time( 0,0 );


	const unative_t current = reg_read_count();
	const uint usec = (relative.secs() * Time::MILLION) + relative.usecs();

 	if (time.usecs() || time.secs()) {
		reg_write_compare( roundUp(current + (usec * m_timeToTicks), m_timeToTicks * 10 * RTC::MILLI_SECOND) );
	} else {
		reg_write_compare( current );
	}
		PRINT_DEBUG
			("[%u:%u] Set time interrupt in %u usecs current: %x, planned: %x.\n",
				now.secs(), now.usecs(), usec, current, reg_read_compare());
}
/*----------------------------------------------------------------------------*/
void Kernel::refillTLB()
{
  InterruptDisabler inter;

	using namespace Processor;

  bool success = m_tlb.refill(IVirtualMemoryMap::getCurrent().data(), reg_read_badvaddr());
	
	PRINT_DEBUG ("TLB refill for address: %p was a %s.\n",
		reg_read_badvaddr(), success ? "SUCESS" : "FAILURE");

  if (!success) {
		printf( "Access to invalid address %p, KILLING offending thread.\n",
			reg_read_badvaddr() );
    if (Thread::getCurrent())
			Thread::getCurrent()->kill();
		else
			panic( "No thread and invalid tlb refill.\n" );
	}

}
