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

#define SYS_THREAD_CREATE   4
#define SYS_THREAD_SELF     5
#define SYS_THREAD_JOIN     6
#define SYS_THREAD_DETACH   7
#define SYS_THREAD_CANCEL   8
#define SYS_THREAD_SLEEP    9
#define SYS_THREAD_YIELD   10
#define SYS_THREAD_SUSPEND 11
#define SYS_THREAD_WAKEUP  12
#define SYS_THREAD_EXIT    13

#define SYS_EVENT_INIT     14
#define SYS_EVENT_WAIT     15
#define SYS_EVENT_WAIT_TIMEOUT 16
#define SYS_EVENT_FIRE     17
#define SYS_EVENT_DESTROY  18

#define SYS_VMA_ALLOC		19
#define SYS_VMA_FREE		20
#define SYS_VMA_RESIZE		21

#define SYS_GET_TIME		22

#define SYS_COUNT			23

