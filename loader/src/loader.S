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
 * @brief Bootstrap code
 *
 * Taken from Kalisto with only minor changes Ipsum rubbish here, but I'm afraid
 * It would stay that way. Not that this comment is by any means ingenious but 
 * at least people can understand it. 
 */
/*
 * Kalisto
 *
 * Copyright (c) 2001-2008
 * 	Distributed Systems Research Group
 * 	Department of Software Engineering
 * 	Faculty of Mathematics and Physics
 * 	Charles University, Czech Republic
 *
 * System bootstrap code
 *
 * This is the module that gets executed as the processor powers up. The
 * processor is hardwired to start executing at the virtual address
 * 0xBFC00000, which maps to the physical address 0x1FC00000.
 *
 * The bootstrap code resides in segment KSEG1, which has both address
 * translation and memory caching disabled. This makes it possible to
 * prepare the address translation structures before address
 * translation is enabled.
 *
 * The only thing that the module does is jumping to the real kernel code.
 * The real kernel occupies segment KSEG0 from address 0x8000000, where
 * the hardwired addresses of the exception handlers are. Theoretically,
 * the real kernel code could be here, but having everything in one
 * place makes things a tiny bit simpler.
 *
 * When the processor powers up, the content of all general registers is
 * undefined. The BEV and EXL bits in the Status Register are set to 1,
 * which means that the processor operates in kernel mode with
 * interrupts disabled.
 *
 */

.text

/*
 * The bootstrap code requires having an absolute control over the processor,
 * we therefore include some directives that prevent the compiler from being
 * too smart with the assembly instructions:
 *
 *  - noat prevents the compiler from using the $at register in macros
 *  - noreorder prevents the compiler from reordering instructions
 *
 */

.set noat
.set noreorder

/*
#include "shared.h"
#include "asm.h"
asm is not needed and shared is needed for ENTRY_POINT and KSEG0 shift macros
I may as well declare them here
*/

.ent __start

__start:
	
	/*
	 * Just jump to the real kernel code straight away.
	 *
	 * If you are tracing this code, you might be surprised to see
	 * that the LA instruction is not a machine code instruction.
	 * The processor does not implement instructions that load
	 * a 32 bit constant to a register, a pair of instructions
	 * that load the constant in two 16 bit parts is used
	 * instead.
	 *
	 */
#define KERNEL_ENTRY_POINT_IN_KSEG0 0x80000400

	la $ra, KERNEL_ENTRY_POINT_IN_KSEG0
	/* ADDR_IN_KSEG0 (KERNEL_ENTRY_POINT) Kalisto version 
	   requires macros defined in shared.h
	*/
	j $ra
	nop
	
	/*
	 * The NOP is attached to the code sequence to make sure that
	 * no other instruction is executed in the branch delay slot
	 * of the jump.
	 *
	 */
.end __start
