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

/*! basic platfom types, taken from Kalisto */
typedef signed char int8_t;
typedef unsigned char uint8_t;

typedef signed short int16_t;
typedef unsigned short uint16_t;

typedef signed long int32_t;
typedef unsigned long uint32_t;

typedef signed long long int64_t;
typedef unsigned long long uint64_t;

typedef int32_t native_t;
typedef uint32_t unative_t;
typedef uint32_t uintptr_t;
typedef uint32_t off_t;
typedef uint32_t size_t;
typedef int32_t ssize_t;

typedef uint32_t thread_t;
typedef uint32_t ipl_t;

typedef __builtin_va_list va_list;

#include "errors.h"

#ifndef NULL
#define NULL 0
#endif

#ifndef false
#define false 0
#endif

#ifndef true
#define true !(false)
#endif

