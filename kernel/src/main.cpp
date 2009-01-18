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
 * @brief Kernel entry points.
 *
 * Entry point implementation, most of them just gets Kernel instance and calls
 * its member function.
 */

#include "main.h"
#include "Kernel.h"
#include "asm/atomic.h"

volatile unative_t COUNT_CPU = 0;
volatile native_t SIMPLE_LOCK = 0;

/*! bootstrap entry point */
void wrapped_start( void )
{
	while ( swap(SIMPLE_LOCK, 1) ) ;
	KERNEL.run();
	while (1) ;
}

/*! entry point for general_exceptions */
void wrapped_general( Processor::Context* registers )
{
	KERNEL.exception( registers );
}

/*! TLB miss handler */
void wrapped_tlbrefill( void )
{
	KERNEL.refillTLB();
}
