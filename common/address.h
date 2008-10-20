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
 * @brief Usefull addresses as defines.
 *
 * Defines segment offset mask, prefix mask, kseg0 and kseg1 prefix.
 * All taken from Kalisto file.
 */


#define ADDR_PREFIX_MASK 	0xe0000000	/*! upper 3 bits */
#define ADDR_OFFSET_MASK 	0x1fffffff	/*! lower 29 bits */

#define ADDR_PREFIX_KSEG0	0x80000000	/*! 2GB */
#define ADDR_PREFIX_KSEG1	0xA0000000	/*! 2.5GB */

#define ADDR_OFFSET(addr)	( (addr) & ADDR_OFFSET_MASK ) /*! take the offset */

/*! replace offset by kseg0 offset */
#define ADDR_TO_KSEG0(addr)	( ADDR_OFFSET(addr) | ADDR_PREFIX_KSEG0 )
/*! replace offset by kseg1 offset */
#define ADDR_TO_KSEG1(addr) ( ADDR_OFFSET(addr) | ADDR_PREFIX_KSEG1 )

/*! skip first 1 kB */ 
#define KERNEL_ENTRY_POINT		ADDR_TO_KSEG0 (0x400)

