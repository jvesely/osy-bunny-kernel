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
 * @brief Contains ASSERT and assert macros.
 *
 */

#pragma once
/*! assert and ASSERT are the same thing. */
#define assert ASSERT

/*! ASSERT macro.
 * Macro tests condition and calls panic, if condition is false.
 * This macro only works when compiled without NDEBUG macro defined.
 * @param test Condition to be tested.
 */
#ifndef NDEBUG
#define ASSERT(test) \
  if (! (test) ) { \
    panic("ASSERTION FAILED: \"%s\" in %s on line %d\n", (char*)#test, __FILE__, __LINE__); \
  } else ((void) 0)
#else
# define ASSERT(x) ((void) 0)
#endif

