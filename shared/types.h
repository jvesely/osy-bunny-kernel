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
 *   svn://---svn repository here---
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
 * @brief Basic types.
 *
 * Most oof the types are just copied from Kalisto.
 */

#pragma once

#include "errors.h"


/*! basic platfom types, taken from Kalisto */

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef unsigned char byte;

typedef signed short int16_t;
typedef unsigned short uint16_t;

typedef signed long int32_t;
typedef unsigned long uint32_t;

typedef signed long long int64_t;
typedef unsigned long long uint64_t;

typedef int32_t  native_t;
typedef uint32_t unative_t;
typedef uint32_t uintptr_t;
typedef uint32_t off_t;
typedef uint32_t size_t;
typedef int32_t  ssize_t;

typedef uint32_t thread_t;
typedef uint32_t ipl_t;
typedef uint32_t mtx_id_t;
typedef uint32_t event_t;
typedef uint32_t process_t;
typedef uint32_t file_t;

typedef unsigned int uint;

typedef __builtin_va_list va_list;
#define va_start __builtin_va_start
#define va_end __builtin_va_end
#define va_arg __builtin_va_arg


#ifndef __cplusplus
typedef uint8_t bool;
#endif

enum FileMode {
	OPEN_R  = 1,
	OPEN_W  = 2,
	OPEN_RW = 3,
	OPEN_A  = 4
	};
enum FilePos {
	POS_START, POS_CURRENT, POS_END
};


/*----------------------------------------------------------------------------*/
#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void*)0)
#endif
#endif

#ifndef false
#define false 0
#endif

#ifndef true
#define true !(false)
#endif
