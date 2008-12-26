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
 * @brief Short description.
 *
 * Long description. I would paste some Loren Ipsum rubbish here, but I'm afraid
 * It would stay that way. Not that this comment is by any means ingenious but 
 * at least people can understand it. 
 */
#pragma once
#include "types.h"

namespace SysCalls
{
enum SysCalls {
	SC_PUTS, SC_GETS
};

inline void syscall( SysCalls call, unative_t p0 = 0, unative_t p1 = 0, unative_t p2 = 0 ) {
	register unative_t __a0 asm("$4") = (unative_t) call;
	register unative_t __a1 asm("$5") = (unative_t) p0;
	register unative_t __a2 asm("$6") = (unative_t) p1;
	register unative_t __a3 asm("$7") = (unative_t) p2;

	asm volatile ("syscall");

	 __a0 = __a1 = __a2 = __a3;
}

inline size_t puts( const char* str ) {
	syscall( SC_PUTS, (unative_t)str );
	return 0;
}

inline size_t gets( char* str, size_t count ){
	syscall( SC_GETS, (unative_t)str, count );
	return 0;
}

}
