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
 * All taken from the Kalisto file.
 */


#define ADDR_PREFIX_MASK 	0xe0000000	/*! upper 3 bits */
#define ADDR_OFFSET_MASK 	0x1fffffff	/*! lower 29 bits */

#define ADDR_PREFIX_KSEG0	(0x80000000)	/*! 2GB */
#define ADDR_PREFIX_KSEG1	(0xA0000000)	/*! 2.5GB */
#define ADDR_PREFIX_KSSEG (0xC0000000)  /*! 3GB */
#define ADDR_PREFIX_KSEG3 (0xE0000000)  /*! 3.5GB */

#define ADDR_OFFSET(addr)	( (addr) & ADDR_OFFSET_MASK ) /*! take the offset */

/*! replace offset by kseg0 offset */
#define ADDR_TO_KSEG0(addr)	( (addr & ADDR_OFFSET_MASK) | ADDR_PREFIX_KSEG0 )
/*! replace offset by kseg1 offset */
#define ADDR_TO_KSEG1(addr) ( (addr & ADDR_OFFSET_MASK) | ADDR_PREFIX_KSEG1 )


/* Entry points (jumps to C++ code) */

/*! skip first 1 kB */ 
#define KERNEL_ENTRY_POINT		      ADDR_TO_KSEG0 (0x400)

/*! TLB refill at the very beginning */
#define HANDLER_TLB_REFILL          ADDR_TO_KSEG0 (0x000)

/*! cache error */
#define HANDLER_CACHE_ERROR         ADDR_TO_KSEG0 (0x100)

/*! Exception handler */
#define HANDLER_GENERAL_EXCEPTION   ADDR_TO_KSEG0 (0x180)

/*
 * Static Kernel Variables
 * In the interrupt and exception handling code, static variables are
 * used for simplicity. Remember that this solution does not work in
 * configurations with more than one processor.
 *
 */

#define KERNEL_STATIC_VARS              ADDR_TO_KSEG0 (0x200)

#define STATIC_OFFSET_EPC               0
#define STATIC_OFFSET_CAUSE             4
#define STATIC_OFFSET_BADVA             8
#define STATIC_OFFSET_STATUS            12

/*! just under the kernel and writing down */
#define KERNEL_STATIC_STACK_TOP         ADDR_TO_KSEG0 (0x400)

