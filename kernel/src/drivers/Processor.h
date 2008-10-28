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
 * @brief Processor namespace.
 *
 * Constains registry reading/writing. Registry specific masks and shifts.
 */
#pragma once

#include "types.h"

/*! reads value from given register */
#define read_register(reg)\
({ \
	unative_t __result; \
	asm volatile ( \
		".set push\n" \
		".set noreorder\n" \
		"nop\n" \
		"mfc0 %0, $"#reg"\n" \
		".set pop\n" \
		: "=r" (__result) \
	); \
	__result; \
})

/*! writes value to given registry */
#define write_register(reg, value) \
	asm volatile( \
    ".set push\n" \
    ".set noreorder\n" \
		"nop\n" \
    "mtc0 %0, $"#reg"\n" \
    ".set pop\n" \
    : \
    : "r" (value) \
	);

/*! @namespace Processor
 * @brief Contains constants and routines associated with 
 * handling processor and control processor.
 */
namespace Processor
{

static const unative_t CPU_IMPLEMENTATION_SHIFT = 8; /*!< upper  byte */
static const unative_t CPU_REVISION_SHIFT = 4; /*!< half byte */
static const unative_t CPU_REVISION_MASK = 0xf; /*!< lower hlaf of byte */

static const unative_t MASK_SHIFT = 13; /*!< lower 13 bits should be zeros */
static const unative_t ASID_MASK = 0x00001fff; /*!< lower 13 bits */
static const unative_t VPN2_MASK = 0x01ffe000; /*!< upper 19 bits */
static const unative_t PFN_SHIFT = 6; /*!< lower 6 bits contains flags */
static const unative_t PFN_ADDR_MASK = (0xffffff << PFN_SHIFT); /*!< bits 6-30*/

static const unative_t PROBE_FAILURE = 0x80000000; /*!< highest bit */
static const unative_t ENTRY_LO_VALID_MASK = 0x00000002; /*!< second bit */
static const unative_t ENTRY_LO_DIRTY_MASK = 0x00000004; /*!< third bit */

static const unative_t STATUS_IE_MASK = 1; /*!< subject to change */

static const uint32_t ENTRY_COUNT = 48; /*!< number of avialable TLB entries */

/*! reverted bit usage mask in TLB according to page size */
enum PageSize{
	PAGE_4K   = 0x000, 	             /*!< all bits used */
	PAGE_16K  = 0x003 << MASK_SHIFT, /*!< ignore lower  2 bits (of 24) */
	PAGE_64K  = 0x00f << MASK_SHIFT, /*!< ignore lower  4 bits (of 24) */
	PAGE_256K = 0x03f << MASK_SHIFT, /*!< ignore lower  6 bits (of 24) */
	PAGE_1M   = 0x0ff << MASK_SHIFT, /*!< ignore lower  8 bits (of 24) */
	PAGE_4M   = 0x3ff << MASK_SHIFT, /*!< ignore lower 10 bits (of 24) */
	PAGE_16M  = 0xfff << MASK_SHIFT  /*!< ignore lower 12 bits (of 24) */

};

extern "C" void switch_cpu_context(void** old_top, void** new_top);

/*! "tlbwr" instruction wrapper */
inline void TLB_write_random() { asm volatile ("tlbwr\n"); }
/*! "tlbwi" instruction wrapper */
inline void TLB_write_index() { asm volatile ("tlbwi\n"); }
/*! "tlbr" instruction wrapper */
inline void TLB_read() { asm volatile ("tlbr\n"); }
/*! "tlbp" instruction wrapper */
inline void TLB_probe() { asm volatile ("tlbp\n"); }


/*! named register read wrapper */
inline unative_t reg_read_index()     { return read_register(0); }
/*! named register read wrapper */
inline unative_t reg_read_pagemask()	{ return read_register(5); }
/*! named register read wrapper */
inline unative_t reg_read_badvaddr()  { return read_register(8); }
/*! named register read wrapper */
inline unative_t reg_read_count()     { return read_register(9); }
/*! named register read wrapper */
inline unative_t reg_read_compare()   { return read_register(11); }
/*! named register read wrapper */
inline unative_t reg_read_status()    { return read_register(12); }
/*! named register read wrapper */
inline unative_t reg_read_cause()     { return read_register(13); }
/*! named register read wrapper */
inline unative_t reg_read_epc()       { return read_register(14); }
/*! named register read wrapper */
inline unative_t reg_read_prid()      { return read_register(15); }
/*! named register read wrapper */
inline unative_t reg_read_xcontext()  { return read_register(20); }
/*! named register read wrapper */
inline unative_t reg_read_eepc()      { return read_register(30); }

/*! named register write wrapper */
inline void reg_write_index(unative_t value)    { write_register(0, value); }
/*! named register write wrapper */
inline void reg_write_entrylo0(unative_t value) { write_register(2, value); }
/*! named register write wrapper */
inline void reg_write_entrylo1(unative_t value) { write_register(3, value); }
/*! named register write wrapper */
inline void reg_write_pagemask(unative_t value) { write_register(5, value); }
/*! named register write wrapper */
inline void reg_write_wired(unative_t value)    { write_register(6, value); }
/*! named register write wrapper */
inline void reg_write_count(unative_t value)    { write_register(9, value); }
/*! named register write wrapper */
inline void reg_write_entryhi(unative_t value)  { write_register(10, value); }
/*! named register write wrapper */
inline void reg_write_compare(unative_t value)  { write_register(11, value); }
/*! named register write wrapper */
inline void reg_write_status(unative_t value)   { write_register(12, value); }
/*! named register write wrapper */
inline void reg_write_cause(unative_t value)    { write_register(13, value); }
/*! named register write wrapper */
inline void reg_write_epc(unative_t value)      { write_register(14, value); }
/*! named register write wrapper */
inline void reg_write_eepc(unative_t value)     { write_register(30, value); }

/*! return current status of interupts and disable them, taken from Kalisto */
inline ipl_t save_and_disable_interupts()
{
	ipl_t status = reg_read_status();
	reg_write_status(status & ~STATUS_IE_MASK);
	return (status & STATUS_IE_MASK);
}

/*! revert to previous interupt status */
inline void revert_interupt_state(ipl_t state)
{
	if (state)
		reg_write_status(reg_read_status() & STATUS_IE_MASK);
}

/*! msim_special instruction: turn trace ON */
inline void msim_trace_on (void)
{
  asm volatile (
    ".insn\n"
    ".word 0x39\n"
  );
}

/*! msim_special instruction: turn trace OFF */
inline void msim_trace_off (void)
{
  asm volatile (
    ".insn\n"
    ".word 0x3d\n"
  );
}

/*! msim_special instruction: dumps registers */
inline void msim_reg_dump (void)
{
  asm volatile (
    ".insn\n"
    ".word 0x37\n"
  );
}

/*! msim_special instruction: halts machine */
inline void msim_halt (void)
{
  asm volatile (
    ".insn\n"
    ".word 0x28\n"
  );
}

/*! msim_special instruction: stops macine and returns to interactive cmdline */
inline void msim_stop (void)
{
  asm volatile (
    ".insn\n"
    ".word 0x29\n"
  );
}

}
