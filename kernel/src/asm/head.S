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
 * @brief Assembler routines.
 *
 * Contains assembler routines copied from Kalisto. Some might need to be 
 * changed later.
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
 * Essential assembly routines of the kernel.
 *
 * Contains the kernel entry point, the exception handlers,
 * the temporary stack and the temporary variables. This
 * code usually only needs to be touched if multiple
 * processor support is required.
 *
 */

/*
 * The bootstrap and exception handling code requires having an absolute
 * control over the processor, we therefore include some directives that
 * prevent the compiler from being too smart with the assembly instructions:
 *
 *  - noat prevents the compiler from using the $at register in macros
 *  - noreorder prevents the compiler from reordering instructions
 *
 */

.set noat
.set noreorder


#include "address.h"
#include "asm.h"
#include "registers.h"

.macro SWITCH_STACK
	lw  $k0, (other_stack_ptr)
	lw  $k1, ($k0)
	beqz $k1, 1f
	nop
	sw  $sp, ($k0)
	ori $sp, $k1, 0
1:
.endm SWITCH_STACK

/***************************************************************************\
| Code at hardwired addresses                                               |
\***************************************************************************/

/*
 * Put the hardwired code to a special input section ".excvec".
 * This section is then placed by the linker (instructed by the
 * kernel.lds linker script) at the very beginning of the kernel
 * image.
 *
 */

.section .excvec, "ax"


/*
 * The exception handlers are placed at the hardwired addresses.
 * Rather than trying to fit the code in between the hardwired
 * addresses, the handlers simply jump away to the real code.
 *
 */


/*
 * TLB Refill exception
 *
 */

.org   ADDR_OFFSET (HANDLER_TLB_REFILL)
.globl exception_tlbrefill
.ent   exception_tlbrefill

exception_tlbrefill:
	j handle_tlbrefill
	nop

.end   exception_tlbrefill


/*
 * Cache error exception
 *
 */

.org   ADDR_OFFSET (HANDLER_CACHE_ERROR)
.globl exception_cacheerror
.ent   exception_cacheerror

exception_cacheerror:
	j handle_cacheerror
	nop

.end   exception_cacheerror


/*
 * General exception
 *
 */

.org   ADDR_OFFSET (HANDLER_GENERAL_EXCEPTION)
.globl exception_general
.ent   exception_general		

exception_general:
	j handle_general
	nop

.end   exception_general


/*
 * Between the exception entry points and the kernel entry point,
 * a space for temporary variables and temporary stack is made.
 *
 * Having the variables and the stack allocated statically
 * makes the interrupt and exception handling simpler, but
 * remember that it does not work with multiple processors.
 *
 * No code needs to be written here, the variables and the
 * stack are simply a space between here and the kernel
 * entry point ...
 *
 */


/***************************************************************************\
| Kernel entry point                                                        |
\***************************************************************************/


/*
 * Kernel entry point
 *
 * The bootstrap loader jumps here. The kernel initialiazes
 * the stack pointer register and the global pointer
 * register and passes control to compiled C code.
 *
 */

.org   ADDR_OFFSET (KERNEL_ENTRY_POINT)
.globl start
.ent   start

start:

  la    $t0, COUNT_CPU
9:
  ll    $t1, ($t0)
  addi  $t2, $t1, 1
  sc    $t2, ($t0)
  beqz  $t2, 9b
  nop

	la    $t2, KERNEL_STATIC_STACK_SIZE
  multu $t1, $t2
  nop
  mflo  $t0
  addi  $t3, $t0, KERNEL_STATIC_STACK_SIZE
  la    $t4, _kernel_end
  add   $sp, $t4, $t3

/*	
  beqz  $t1, 8f
  nop
  wait
*/
8:
	
	/* Set the statically allocated stack. */
	/* la $sp, ADDR_TO_KSEG0 (KERNEL_STATIC_STACK_TOP) */

	/* Set the global pointer to the kernel segment. */
	la $gp, ADDR_TO_KSEG0 (0) 

	/* Continue in compiled C code. */
	jal wrapped_start
	nop
	
	/* Not reached. */
	halt

.end start


/***************************************************************************\
| Code at arbitrary addresses                                               |
\***************************************************************************/

.text


/***************************************************************************\
| Exception handlers                                                        |
\***************************************************************************/


/*
 * TLB Refill exception handler
 *
 * The handler code saves all registers and passes control to compiled C code.
 *
 */

.ent handle_tlbrefill

handle_tlbrefill:

	SWITCH_STACK

	addi $sp, $sp, -CONTEXT_SIZE

	/* Save the general registers and the $lo and $hi registers. */
	
	SAVE_REGISTERS $sp
	mflo $t0
	mfhi $t1
	sw $t0, REGS_OFFSET_LO($sp)
	sw $t1, REGS_OFFSET_HI($sp)
	
	/* Call the exception handler in compiled C code, passing
	   the pointer to the stack frame as an argument. */
	
	jal wrapped_tlbrefill
	nop
 	/* or  $a0, $0, $sp */
	
	/* Restore the $lo and $hi registers and the general registers. */
	
	lw $t0, REGS_OFFSET_LO($sp)
	lw $t1, REGS_OFFSET_HI($sp)
	mtlo $t0
	mthi $t1
	LOAD_REGISTERS $sp
	
	addi $sp, $sp, CONTEXT_SIZE
	
	/* Restore the stack pointer last and return from exception. */
	
	SWITCH_STACK

	eret

.end handle_tlbrefill


/*
 * Cache Error exception handler
 *
 * The handler halts since this exception should not occur.
 *
 */

.ent handle_cacheerror

handle_cacheerror:
	halt

.end handle_cacheerror


/*
 * General Exception handler
 *
 * The handler code saves all registers and passes control to compiled C code.
 *
 */

.ent handle_general

handle_general:
	
	/* Save the content of the EPC, Status, BadVAddr and Cause registers
	   of the System Control Coprocessor to statically allocated
	   variables. These registers contain information on the
	   exception that we are handling, and the information
	   must be saved in case another exception comes
	   while we are inside this handler.
	   
	   The code can trash the $k0 and $k1 registers, which are
	   reserved for kernel use. All other registers are preserved.
	   Note that using $k0 and $k1 here means $k0 and $k1 must not be used
	   anywhere else where there is a chance of the General Exception
	   occuring!
	   
	   This code must be changed in configurations with more than
	   one processor, otherwise the static variables will get
	   overwritten. */

	la $k1, ADDR_TO_KSEG0 (KERNEL_STATIC_VARS)
	
	mfc0 $k0, $epc
	sw $k0, STATIC_OFFSET_EPC($k1)
	mfc0 $k0, $cause
	sw $k0, STATIC_OFFSET_CAUSE($k1)
	mfc0 $k0, $badvaddr
	sw $k0, STATIC_OFFSET_BADVA($k1)
	mfc0 $k0, $status
	sw $k0, STATIC_OFFSET_STATUS($k1)
	
	/* On entering the handler, the EXL bit in the Status register of
	   the System Control Coprocessor is set. This bit indicates that
	   the processor is just beginning to handle an exception, and
	   that nested exceptions require special handling.
	   
	   Now that we have saved the registers of the System Control
	   Coprocessor, we can clear the EXL bit in the Status register,
	   but to avoid nested exceptions, we also disable interrupts by
	   masking the IE bit of the Status register, and we make sure we
	   stay in the kernel mode by masking the KSU bits of the Status
	   register. */
	
	 la $k1, ~(CP0_STATUS_KSU_MASK | CP0_STATUS_EXL_MASK | CP0_STATUS_IE_MASK)
	and $k0, $k1
	mtc0 $k0, $status
	
	/* Stack is switched to second stack located in KSEG0 if th thread was
		 using USEG stack, this is done to avoid TLB refill/load/save exceptions. 
		 Then the rest of the registers is saved on the stack. */
	
	SWITCH_STACK

	addiu $sp, -CONTEXT_SIZE
	
	SAVE_REGISTERS $sp
	mflo $t0
	mfhi $t1
	sw $t0, REGS_OFFSET_LO($sp)
	sw $t1, REGS_OFFSET_HI($sp)
	
	/* Also move the previously stored System Control Coprocessor
	   registers from the statically allocated variables to the
	   stack. Once this is done, the handler becomes reentrant. */
	
	la $t1, ADDR_TO_KSEG0 (KERNEL_STATIC_VARS)
	
	lw $t0, STATIC_OFFSET_EPC($t1)
	sw $t0, REGS_OFFSET_EPC($sp)
	lw $t0, STATIC_OFFSET_CAUSE($t1)
	sw $t0, REGS_OFFSET_CAUSE($sp)
	lw $t0, STATIC_OFFSET_BADVA($t1)
	sw $t0, REGS_OFFSET_BADVA($sp)
	lw $t0, STATIC_OFFSET_STATUS($t1)
	sw $t0, REGS_OFFSET_STATUS($sp)
	
	/* Call the exception handler in compiled C code, passing
	   the pointer to the stack frame as an argument. */
	
	jal wrapped_general
	or $a0, $0, $sp
	
	/* Now the exception has hopefully been handled and all
	   that remains is returning to the interrupted code.
	   For that, we load the saved registers back first.
	   
	   Note that we use the static variables again, something
	   that needs to be changed in configurations with more
	   than one processor. */
	
	la $t1, ADDR_TO_KSEG0 (KERNEL_STATIC_VARS)
	
	lw $t0, REGS_OFFSET_STATUS($sp)
	sw $t0, STATIC_OFFSET_STATUS($t1)
	lw $t0, REGS_OFFSET_EPC($sp)
	sw $t0, STATIC_OFFSET_EPC($t1)

	lw $t0, REGS_OFFSET_LO($sp)
	lw $t1, REGS_OFFSET_HI($sp)
	mtlo $t0
	mthi $t1
	LOAD_REGISTERS $sp
	
	/* After the saved registers have been loaded, release
	   the stack frame that has been used to preserve them. */
	
	addiu $sp, CONTEXT_SIZE
	
	SWITCH_STACK
	
	/* Finally, restore the System Control Coprocessor registers
	   and return from the exception. The ERET instruction also
	   restores the processor status.
	   
	   Note that an interrupt or an exception here would trash
	   the content of the $k0 and $k1 registers, which would be bad! */

	la $k1, ADDR_TO_KSEG0 (KERNEL_STATIC_VARS)
	
	lw $k0, STATIC_OFFSET_EPC($k1)
	mtc0 $k0, $epc
	lw $k0, STATIC_OFFSET_STATUS($k1)
	mtc0 $k0, $status

	eret

.end handle_general


/***************************************************************************\
| Context switching                                                         |
\***************************************************************************/


/*
 * switch_cpu_context
 *
 * Switches processor context from one thread to another. The first argument
 * points to a pointer to the top of the stack of the old thread, the
 * second argument points to a pointer to the top of the stack of
 * the new thread.
 *
 * The pointer to the old stack is written, the pointer to the new stack
 * is read. Both are passed as pointers to handle the weird case where
 * the old thread and the new thread are identical, because then,
 * we would not know the address of the new stack before it
 * is written as the address of the old stack.
 *
 * The processor context switch does not include memory management context.
 *
 */

.globl switch_cpu_context
.ent   switch_cpu_context

switch_cpu_context:

	beqz $a0, load
	nop
	/* Allocate a frame on the stack of the old thread and update
	   the address of the stack top of the old thread. */

	addiu $sp, -CONTEXT_SIZE
	sw $sp, ($a0)
	
	/* Store the general registers and the $hi, $lo and Status
	   registers on the stack. Disable interrupts after
	   storing the Status register. */

	SAVE_REGISTERS $sp
	mflo $t0
	mfhi $t1
	sw $t0, REGS_OFFSET_LO($sp)
	sw $t1, REGS_OFFSET_HI($sp)
	
	mfc0 $t0, $status
	sw $t0, REGS_OFFSET_STATUS($sp)
	la $t1, ~CP0_STATUS_IE_MASK 
	and $t0, $t1
	mtc0 $t0, $status
	
	/* Switch to the stack of the new thread. The stack of the
	   new thread should be in the same state as the stack of
	   the old thread. */
load:
	
	beqz $a1, out
	nop
	lw $sp, ($a1)
	
	/* Load the general registers and the $hi and $lo registers from the stack. */
	
	lw $t0, REGS_OFFSET_LO($sp)
	lw $t1, REGS_OFFSET_HI($sp)
	mtlo $t0
	mthi $t1
	LOAD_REGISTERS $sp

	/* Prepare to restore the Status register. The Status register is
	   restored in the branch delay slot of the jump that returns
	   control to the new thread.
	   
	   Setting the Status register in the branch delay slot makes it
	   possible to return from kernel mode to user mode. Setting the
	   register sooner would mean switching from kernel mode to user
	   mode while executing in KSEG0, which is not allowed.
	   
	   A somewhat cleaner alternative to this particular method of
	   returning from kernel mode to user mode is the ERET instruction. */

	lw $k0, REGS_OFFSET_STATUS($sp)
	addiu $sp, CONTEXT_SIZE
	
	/* Note that from now on, an interrupt or an exception here would
	   trash the content of the $k0 and $k1 registers. This is why we
	   have disabled interrupts earlier, and do not touch anything
	   in memory. */
out:

	j $ra
	mtc0 $k0, $status

.end switch_cpu_context

.globl switch_to_usermode
.ent   switch_to_usermode
switch_to_usermode:
	SWITCH_STACK
	mfc0 $k0, $status
	ori  $k0, $k0, 0x00000011
	j $a0
	mtc0 $k0, $status 
.end   switch_to_usermode
