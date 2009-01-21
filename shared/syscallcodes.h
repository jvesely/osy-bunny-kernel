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
 * @brief Syscall codes.
 *
 * Contains translation macros from syscall names to codes.
 */

#pragma once

#define SYS_PUTS            1
#define SYS_GETS            2
#define SYS_EXIT            3

#define SYS_THREAD_CREATE   5
#define SYS_THREAD_SELF     6
#define SYS_THREAD_JOIN     7
#define SYS_THREAD_DETACH   8
#define SYS_THREAD_CANCEL   9
#define SYS_THREAD_SLEEP   10
#define SYS_THREAD_YIELD   11
#define SYS_THREAD_SUSPEND 12
#define SYS_THREAD_WAKEUP  13
#define SYS_THREAD_EXIT    14

#define SYS_LAST 15
