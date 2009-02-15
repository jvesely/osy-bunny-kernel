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
 *   @version $Id: putc_a.cpp 733 2009-02-14 23:26:50Z slovak $
 *   @note
 *   Semestral work for Operating Systems course at MFF UK \n
 *   http://dsrg.mff.cuni.cz/~ceres/sch/osy/main.php
 *   
 *   @date 2008-2009
 */

/*!
 * @file 
 * @brief putc_a function using buffer to write to the console to save
 * time with syscalls.
 */

#include "types.h"

extern "C" size_t puts( const char * str );

size_t putc_a( const char c )
{
  const size_t BUFFER_SIZE = 80;
  
  static char buffer[BUFFER_SIZE];
  static off_t pos = 0;
  
  if (c == '\0') {
    buffer[pos] = c;
    puts(buffer);
    pos = 0;
    
    return 0;
  } else {
    buffer[pos++] = c;
    if (pos == BUFFER_SIZE || c == '\n') { 
		putc_a('\0'); 
	}
    
    return 1;
  }

}
