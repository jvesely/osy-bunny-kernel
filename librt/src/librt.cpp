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

#include "librt.h"
#include "syscalls.h"

/* Basic IO */
size_t putc( const char c )
{
	static char buffer[2];
	buffer[0] = c;
	return SysCalls::puts( buffer );
}
/*----------------------------------------------------------------------------*/
size_t puts( const char* str )
{
	return SysCalls::puts( str );
}
/*----------------------------------------------------------------------------*/
char getc()
{
	static char buffer;
	if (SysCalls::gets( &buffer, 1 ) == 1)
		return buffer;
	else
		return EOTHER;
}
/*----------------------------------------------------------------------------*/
ssize_t gets( char* str, const size_t len )
{
	/* we don't need to call syscall just to report error */
	if (!len) return EINVAL;  
	return SysCalls::gets( str, len );
}
