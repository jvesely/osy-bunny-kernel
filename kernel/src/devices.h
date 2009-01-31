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
 * Device addresses and interrupts are defined in this place.
 */

#pragma once
#include "types.h"

#define DEVICES_MAP_START 0xFFFFFF00

#define TIMER_INTERRUPT 7

/*! simple character output */
#define CHARACTER_OUTPUT_ADDRESS (char*)(0xFFFFFFF0)

/*! keyboard */
#define CHARACTER_INPUT_ADDRESS (char*)(0xFFFFFFE0)
#define CHARACTER_INPUT_INTERRUPT 1

/*! rtc */
#define CLOCK (unative_t*)(0xFFFFFFD0)

/*! hdd */
#define HDD0_ADDRESS    (unative_t*)(0xffffffc0)
#define HDD0_INTERRUPT    2

/*! dorder */
#define DORDER_ADDRESS (unative_t*)(0xFFFFFFB0)
#define DORDER_INTERRUPT 3

