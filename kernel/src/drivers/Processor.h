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

static const unative_t PAGE_MASK_SHIFT = 13; /*!< lower 13 bits should be zeros */
static const unative_t ASID_MASK = 0x000000ff; /*!< lower 8 bits */
static const unative_t VPN2_MASK = 0xffffe000; /*!< upper 19 bits */
static const unative_t PFN_SHIFT = 6; /*!< lower 6 bits contains flags */
static const unative_t PFN_ADDR_MASK = (0xffffff << PFN_SHIFT); /*!< bits 6-30*/

static const unative_t PROBE_FAILURE        = 0x80000000; /*!< highest bit */
static const unative_t ENTRY_LO_VALID_MASK  = 0x00000002; /*!< second bit */
static const unative_t ENTRY_LO_DIRTY_MASK  = 0x00000004; /*!< third bit */
static const unative_t ENTRY_LO_GLOBAL_MASK = 0x00000001;

static const unative_t ENTRY_HI_EVEN_4K   = 1 << 12;
static const unative_t ENTRY_HI_EVEN_16K  = 1 << 14;
static const unative_t ENTRY_HI_EVEN_64K  = 1 << 16;
static const unative_t ENTRY_HI_EVEN_256K = 1 << 18;
static const unative_t ENTRY_HI_EVEN_1M   = 1 << 20;
static const unative_t ENTRY_HI_EVEN_4M   = 1 << 22;
static const unative_t ENTRY_HI_EVEN_16M  = 1 << 24;

static const unsigned int ENTRY_COUNT = 48; /*!< number of TLB entries */

struct Page {
	uint size;
	uint shift;
	unative_t mask;
};

static const Page pages[8] = {
	{ 0x0001000, 12, 0x000 },        /*!<   4KB */
	{ 0x0004000, 14, 0x003 },        /*!<  16KB */
	{ 0x0010000, 16, 0x00f },        /*!<  64KB */
	{ 0x0040000, 18, 0x03f },        /*!< 256KB */
	{ 0x0100000, 20, 0x0ff },        /*!<   1MB */
	{ 0x0400000, 22, 0x3ff },        /*!<   4MB */
	{ 0x1000000, 24, 0xfff }
};

/*! reverted bit usage mask in TLB according to page size */
enum PageSize {
	PAGE_4K,//   = 0x000, 	                  /*!< all bits used */
	PAGE_16K,//  = 0x003 << PAGE_MASK_SHIFT, /*!< bits 14,13 */
	PAGE_64K,//  = 0x00f << PAGE_MASK_SHIFT, /*!< bits 16-13 */
	PAGE_256K,// = 0x03f << PAGE_MASK_SHIFT, /*!< bits 18-13 */
	PAGE_1M,//   = 0x0ff << PAGE_MASK_SHIFT, /*!< bits 20-13 */
	PAGE_4M,//   = 0x3ff << PAGE_MASK_SHIFT, /*!< bits 22-13 */
	PAGE_16M//  = 0xfff << PAGE_MASK_SHIFT  /*!< bits 24-13 */

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
inline unative_t reg_read_random()    { return read_register(1); }
/*! named register read wrapper */
inline unative_t reg_read_pagemask()	{ return read_register(5); }
/*! named register read wrapper */
inline unative_t reg_read_badvaddr()  { return read_register(8); }
/*! named register read wrapper */
inline unative_t reg_read_count()     { return read_register(9); }
/*! named register read wrapper */
inline unative_t reg_read_entryhi()   { return read_register(10); }
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


/*! @struct Context Processor.h "drivers/Processor.h"
 * @brief Helper structure that helps address separate registers.
 */
struct Context
{
	unative_t zero;

	unative_t at;
	unative_t v0;
	unative_t v1;

	unative_t a0;
	unative_t a1;
	unative_t a2;
	unative_t a3;

	unative_t t0;
	unative_t t1;
	unative_t t2;
	unative_t t3;
	unative_t t4;
	unative_t t5;
	unative_t t6;
	unative_t t7;
	unative_t t8;
	unative_t t9;

	unative_t s0;
	unative_t s1;
	unative_t s2;
	unative_t s3;
	unative_t s4;
	unative_t s5;
	unative_t s6;
	unative_t s7;

	unative_t k0;
	unative_t k1;

	unative_t gp;
	unative_t fp;

	unative_t sp;
	unative_t ra;

	unative_t lo;
	unative_t hi;

	unative_t epc;
	unative_t cause;
	unative_t badva;
	unative_t status;
};

enum Cause {
	CAUSE_IP_MASK  = 0x0000ff00,
  CAUSE_IP0_MASK = 0x00000100,
  CAUSE_IP1_MASK = 0x00000200,
  CAUSE_IP2_MASK = 0x00000400,
  CAUSE_IP3_MASK = 0x00000800,
  CAUSE_IP4_MASK = 0x00001000,
  CAUSE_IP5_MASK = 0x00002000,
  CAUSE_IP6_MASK = 0x00004000,
  CAUSE_IP7_MASK = 0x00008000,

	CAUSE_BD_MASK  = 0x80000000
};

enum Status {
	STATUS_IM_MASK  = 0x0000ff00,
 	STATUS_IM0_MASK = 0x00000100,
 	STATUS_IM1_MASK = 0x00000200,
 	STATUS_IM2_MASK = 0x00000400,
 	STATUS_IM3_MASK = 0x00000800,
 	STATUS_IM4_MASK = 0x00001000,
 	STATUS_IM5_MASK = 0x00002000,
 	STATUS_IM6_MASK = 0x00004000,
 	STATUS_IM7_MASK = 0x00008000,

	STATUS_IE_MASK  = 0x00000001,

	STATUS_CU0_MASK = 0x10000000,
	STATUS_CU1_MASK = 0x20000000,
	STATUS_CU2_MASK = 0x40000000,
	STATUS_CU3_MASK = 0x80000000,
	STATUS_CU_MASK  = 0xf0000000

};
/*! return current status of interrupts and disable them, taken from Kalisto */
inline ipl_t save_and_disable_interrupts()
{
	ipl_t status = reg_read_status();
	reg_write_status(status & ~STATUS_IE_MASK);
	return (status & STATUS_IE_MASK);
}

/*! revert to previous interrupt status */
inline void revert_interrupt_state(ipl_t state)
{
	if (state)
		reg_write_status(reg_read_status() | STATUS_IE_MASK);
}
/*----------------------------------------------------------------------------*/
/*! @brief Exception codes */
enum Exceptions {
	CAUSE_EXCCODE_INT,  /*!< Interrupt */
	CAUSE_EXCCODE_MOD,  /*!< TLB Modification */
	CAUSE_EXCCODE_TLBL, /*!< TLB Load */
	CAUSE_EXCCODE_TLBS, /*!< TLB Store */
	CAUSE_EXCCODE_ADEL, /*!< Address Error Load */
	CAUSE_EXCCODE_ADES, /*!< Address Error Store */
	CAUSE_EXCCODE_IBE,  /*!< Instruction Bus Error */
	CAUSE_EXCCODE_DBE,  /*!< Data Bus Error */
	CAUSE_EXCCODE_SYS,  /*!< Syscall */
	CAUSE_EXCCODE_BP,   /*!< Breakpoint */
	CAUSE_EXCCODE_RI,		/*!< Resrved Instruction */
	CAUSE_EXCCODE_CPU,  /*!< Coprocessor Unusable */
	CAUSE_EXCCODE_OV,   /*!< Integer Overflow */
	CAUSE_EXCCODE_TR,   /*!< Trap */
	CAUSE_EXCCODE_MASK  = 0x0000007c,
	CAUSE_EXCCODE_SHIFT = 2
};

inline unative_t get_exccode(unative_t cause_reg)
{
	return (cause_reg & CAUSE_EXCCODE_MASK) >> CAUSE_EXCCODE_SHIFT;
}
/*----------------------------------------------------------------------------*/
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
