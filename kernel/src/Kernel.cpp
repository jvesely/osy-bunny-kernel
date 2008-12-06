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

/*! This is our great bunny :) */
const char * BUNNY_STR =
"       _     _\n\
       \\`\\ /`/\n\
        \\ V /\n\
        /. .\\\n\
       =\\ T /=\n\
        / ^ \\\n\
     {}/\\\\ //\\\n\
     __\\ \" \" /__\n\
jgs (____/^\\____)\n";

Kernel::Kernel() :
	m_console(CHARACTER_OUTPUT_ADDRESS, CHARACTER_INPUT_ADDRESS), m_clock(CLOCK) {
	Processor::reg_write_status(0);
}
extern void* test(void*);
/*----------------------------------------------------------------------------*/
void Kernel::run()
{
	using namespace Processor;

//	m_tlb.setMapping( DEVICES_MAP_START, DEVICES_MAP_START, PAGE_4K, 0xff, 0,true);
	m_tlb.mapDevices( DEVICES_MAP_START, DEVICES_MAP_START, 1);
	stop();

	printf("HELLO WORLD!\n%s\n", BUNNY_STR );
	const unative_t cpu_type = reg_read_prid();
	printf("Running on MIPS R%d revision %d.%d \n", 
	        cpu_type >> CPU_IMPLEMENTATION_SHIFT, 
	        (cpu_type >> CPU_REVISION_SHIFT) & CPU_REVISION_MASK,
				  cpu_type & CPU_REVISION_MASK );

	native_t to = 0;
	const unsigned int start = m_clock.time();
	printf("Detecting freq....");
	while (m_clock.time() == start) ;
	const native_t from = reg_read_count();
	while (m_clock.time() - (start + 1) < 1) { //1 ms
		printf("\b.");
		to = reg_read_count();
	}
	m_timeToTicks = (to - from) / 1000000;

	/* I would use constants here but they would not be used
	 * in any other part of the program and still it's clear what this does.
	 * (counts Mhz :) )
	 */

	printf("%d.%d MHz\n", m_timeToTicks, (to - from) % 1000 );

	// detect memory
	m_physicalMemorySize = getPhysicalMemorySize();
	printf("Detected %d B of accessible memory\n", m_physicalMemorySize);
	
	// setup allocator
	m_alloc.setup((uintptr_t)&_kernel_end, 0x100000); /* 1 MB */
	
	Timer::instance();

	//init and run the main thread
	thread_t mainThread;
	Thread* main = KernelThread::create(&mainThread, test, NULL, 0);
	ASSERT (main);
	main->switchTo();
	
	panic("Should never reach this.\n");
}
/*----------------------------------------------------------------------------*/
size_t Kernel::getPhysicalMemorySize(){
	printf("Probing memory range...");
	const uint32_t MAGIC = 0xDEADBEEF;
	
	size_t size = 0;
	const size_t range = 0x100000/sizeof(uint32_t); /* 1MB */
	volatile uint32_t * front = (&_kernel_end - 0x80000000/sizeof(uint32_t));
	volatile uint32_t * back = (volatile uint32_t *)( (range - 1) * sizeof(uint32_t) );
	volatile uint32_t * point = front;


	while (true) {
		m_tlb.setMapping((uintptr_t)front, (uintptr_t)point, Processor::PAGE_1M, 0);
	//	dprintf( "Mapped %x to %x range = %d kB.\n", front, point, (range * sizeof(uint32_t)/1024) );
		
		(*front) = MAGIC; //write
		(*back) = MAGIC; //write
	//	dprintf("Proof read %x:%x %x:%x\n", front, *front, back, *back);
		if ( (*front != MAGIC) || (*back != MAGIC) ) break; //check
		size += range * sizeof(uint32_t);
		point += range; //add
	}
	printk("OK\n");
	stop();
	m_tlb.clearAsid( 0 );
	return size;
}
/*----------------------------------------------------------------------------*/
void* Kernel::malloc(const size_t size) const
{
	return m_alloc.getMemory(size);
}
/*----------------------------------------------------------------------------*/
void Kernel::free(const void * address) const
{
	m_alloc.freeMemory(address);
}
/*----------------------------------------------------------------------------*/
void Kernel::handle(Processor::Context* registers)
{
	using namespace Processor;
	const unative_t reason = get_exccode(registers->cause);

	switch (reason){
		case CAUSE_EXCCODE_INT:
			handleInterrupts(registers);
			break;
		case CAUSE_EXCCODE_SYS:
			panic("Syscall.\n");
			break;
		case CAUSE_EXCCODE_TLBL:
		case CAUSE_EXCCODE_TLBS:
			panic("TLB Exception.\n");
		case CAUSE_EXCCODE_ADEL:
		case CAUSE_EXCCODE_ADES:
			printf("Exception: Address error exception. THREAD KILLED\n");
			Thread::getCurrent()->kill();
			panic("Exception: Address error exception.\n");
		case CAUSE_EXCCODE_BP:
			if (!(reason & CAUSE_BD_MASK) ) {
				registers->epc +=4; // go to the next instruction
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
			printf("Exception: Address error exception. THREAD KILLED\n");
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

}
/*----------------------------------------------------------------------------*/
void Kernel::setTimeInterrupt(const Time& time)
{
	using namespace Processor;
	InterruptDisabler interrupts;

	Time now = Time::getCurrent();
	if ( time < now )
		now = time;
	Time relative = time - now;

	const unative_t current = reg_read_count();
	const uint usec = (relative.secs() * Time::MILLION) + relative.usecs();

	const unative_t planned = (time.usecs() || time.secs())
		?	
	roundUp(current + (usec * m_timeToTicks), m_timeToTicks * 10 * RTC::MILLI_SECOND)
		: current;

		
	reg_write_compare( planned );
	//dprintf("Set time interrupt in %u usecs current: %x, planned: %x.\n", usec, current, planned);
}
/*----------------------------------------------------------------------------*/
