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
 * @brief Device addresses.
 *
 * Device addreses are defined here in one place.
 */

#pragma once
#include "types.h"

#define DEVICES_MAP_START 0xFFFFFFF0

/*! simple character output */
#define CHARACTER_OUTPUT_ADDRESS (char*)(0xFFFFFFF0)

/*! keyboard */
#define CHARACTER_INPUT_ADDRESS (char*)(0xFFFFFFF4)
#define CHARACTER_INPUT_INTERUPT 1

/*! rtc */
#define CLOCK (uint32_t*)(0xFFFFFFF8)
