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
 * @brief Some error constants.
 *
 * All error constants are <0, except for EOK, but EOK is no error. ;)
 */
#pragma once

static const uint32_t EOK = 0; /*!< everything OK */
static const uint32_t ENOMEM = -1; /*!< no more memory */
static const uint32_t EINVAL = -2; /*!< Invalid argument */
static const uint32_t EKILLED = -3; /*!< Requested thread heas been killed */
static const uint32_t ETIMEDOUT = -4; /*!< Time limit reached */
static const uint32_t EWOULDBLOCK = -5; /*!< Operation would block */
