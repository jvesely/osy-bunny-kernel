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
enum errors {
	EOK         =  0, /*!< everything OK */
	ENOMEM      = -1, /*!< no more memory */
	EINVAL      = -2, /*!< Invalid argument */
	EKILLED     = -3, /*!< Requested thread heas been killed */
	ETIMEDOUT   = -4, /*!< Time limit reached */
	EWOULDBLOCK = -5  /*!< Operation would block */
};
