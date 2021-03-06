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

#include "api.h"

class ExceptionHandler;

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

static const unative_t CPU_IMPLEMENTATION_SHIFT = 8; /*!< upper  byte        */
static const unative_t CPU_REVISION_SHIFT = 4;       /*!< half byte          */
static const unative_t CPU_REVISION_MASK = 0xf;      /*!< lower half of byte */

static const unative_t PAGE_MASK_SHIFT = 13; /*!< lower 13 bits should be zeros */
static const unative_t ASID_MASK  = 0x000000ff;      /*!< lower 8 bits       */
static const unative_t VPN2_SHIFT = 12;
static const unative_t VPN2_MASK  = 0xffffe000;      /*!< upper 19 bits      */
static const unative_t PFN_SHIFT  = 6; /*!< lower 6 bits contains flags */
static const unative_t PFN_ADDR_MASK = (0xffffff << PFN_SHIFT); /*!< bits 6-30*/

static const unative_t PROBE_FAILURE        = 0x80000000; /*!< highest bit */
static const unative_t ENTRY_LO_VALID_MASK  = 0x00000002; /*!< second bit  */
static const unative_t ENTRY_LO_DIRTY_MASK  = 0x00000004; /*!< third bit   */
static const unative_t ENTRY_LO_GLOBAL_MASK = 0x00000001;

static const unsigned int ENTRY_COUNT = 48; /*!< number of TLB entries */

/*! @struct Page Processor.h "drivers/Processor.h"
 * @brief Stores information about page size.
 */
struct Page {
	uint size;      /*!< @brief Size of the page                                */
	uint shift;     /*!< @brief Position of the lowest valid bit in an address. */
	unative_t mask; /*!< @brief Inverse mask of the used bit for upper 20 bits  */
};

static const Page pages[7] = {
	{ 0x0002000, 13, 0x000 },        /*!<   8KB */
	{ 0x0008000, 15, 0x003 },        /*!<  32KB */
	{ 0x0020000, 17, 0x00f },        /*!< 128KB */
	{ 0x0080000, 19, 0x03f },        /*!< 512KB */
	{ 0x0200000, 21, 0x0ff },        /*!<   2MB */
	{ 0x0800000, 23, 0x3ff },        /*!<   8MB */
	{ 0x2000000, 25, 0xfff }         /*!<  32MB */
};

/*! reverted bit usage mask in TLB according to page size */
enum PageSize {
	PAGE_8K, PAGE_32K, PAGE_128K, PAGE_512K, PAGE_2M, PAGE_8M, PAGE_32M,
	PAGE_MIN = PAGE_8K, PAGE_MAX = PAGE_32M
};

extern "C" void switch_cpu_context(void** old_top, void** new_top);

/*! "tlbwr" instruction wrapper */
inline void TLB_write_random()        { asm volatile ("tlbwr\n"); }
/*! "tlbwi" instruction wrapper */
inline void TLB_write_index()         { asm volatile ("tlbwi\n"); }
/*! "tlbr" instruction wrapper */
inline void TLB_read()                { asm volatile ("tlbr\n");  }
/*! "tlbp" instruction wrapper */
inline void TLB_probe()               { asm volatile ("tlbp\n");  }


/*! named register read wrapper */
inline unative_t reg_read_index()     { return read_register(0);  }
/*! named register read wrapper */
inline unative_t reg_read_random()    { return read_register(1);  }
/*! named register read wrapper */
inline unative_t reg_read_entrylo0()  { return read_register(2); }
/*! named register read wrapper */
inline unative_t reg_read_entrylo1()  { return read_register(3); }
/*! named register read wrapper */
inline unative_t reg_read_pagemask()  { return read_register(5);  }
/*! named register read wrapper */
inline unative_t reg_read_badvaddr()  { return read_register(8);  }
/*! named register read wrapper */
inline unative_t reg_read_wired()     { return read_register(6);  }
/*! named register read wrapper */
inline unative_t reg_read_count()     { return read_register(9);  }
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
inline void reg_write_index( unative_t value )    { write_register(0, value);  }
/*! named register write wrapper */
inline void reg_write_entrylo0( unative_t value ) { write_register(2, value);  }
/*! named register write wrapper */
inline void reg_write_entrylo1( unative_t value ) { write_register(3, value);  }
/*! named register write wrapper */
inline void reg_write_pagemask( unative_t value ) { write_register(5, value);  }
/*! named register write wrapper */
inline void reg_write_wired( unative_t value )    { write_register(6, value);  }
/*! named register write wrapper */
inline void reg_write_count( unative_t value )    { write_register(9, value);  }
/*! named register write wrapper */
inline void reg_write_entryhi( unative_t value )  { write_register(10, value); }
/*! named register write wrapper */
inline void reg_write_compare( unative_t value )  { write_register(11, value); }
/*! named register write wrapper */
inline void reg_write_status( unative_t value )   { write_register(12, value); }
/*! named register write wrapper */
inline void reg_write_cause( unative_t value )    { write_register(13, value); }
/*! named register write wrapper */
inline void reg_write_epc( unative_t value )      { write_register(14, value); }
/*! named register write wrapper */
inline void reg_write_eepc( unative_t value )     { write_register(30, value); }

inline uint random()
{
	return Processor::reg_read_count() * Processor::reg_read_random();
}

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

enum CauseMasks {
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


static const uint INTERRUPT_COUNT = 8;
static const uint IP_SHIFT = 8;         /*!< The lowest byte is skipped. */
static const unative_t INTERRUPT_MASKS[INTERRUPT_COUNT] =
{
	0x01 <<	IP_SHIFT,
	0x02 << IP_SHIFT,
	0x04 << IP_SHIFT,
	0x08 << IP_SHIFT,
	0x10 << IP_SHIFT,
	0x20 << IP_SHIFT,
	0x40 << IP_SHIFT,
	0x80 << IP_SHIFT
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
/*----------------------------------------------------------------------------*/
/*! return current status of interrupts and disable them, taken from Kalisto */
inline ipl_t save_and_disable_interrupts()
{
	ipl_t status = reg_read_status();
	reg_write_status(status & ~STATUS_IE_MASK);
	return (status & STATUS_IE_MASK);
}
/*----------------------------------------------------------------------------*/
/*! revert to previous interrupt status */
inline void revert_interrupt_state(ipl_t state)
{
	if (state)
		reg_write_status(reg_read_status() | STATUS_IE_MASK);
}
/*----------------------------------------------------------------------------*/
/*!
 * @struct Exception Processor.h "drivers/Processor.h"
 * @brief Structure holds some baic info about each exception.
 */
struct Exception
{
	uint code;                 /*!< Code of the exception. */
	const char* name;          /*!< Name of the exception. */
	ExceptionHandler* handler; /*!< Designated handler.    */
};

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
	CAUSE_EXCCODE_TR    /*!< Trap */
};

const uint EXCEPTION_COUNT = 14;

/*! @brief Exception handling vector. */
static const Exception EXCEPTIONS[EXCEPTION_COUNT] =
{
	{ CAUSE_EXCCODE_INT,  (char*)"Interrupt exception"            , NULL },
	{ CAUSE_EXCCODE_MOD,  (char*)"TLB modification exception"     , NULL },
	{ CAUSE_EXCCODE_TLBL, (char*)"TLB load exception"             , NULL },
	{ CAUSE_EXCCODE_TLBS, (char*)"TLB save exception"             , NULL },
	{ CAUSE_EXCCODE_ADEL, (char*)"Address error load exception"   , NULL },
	{ CAUSE_EXCCODE_ADES, (char*)"Address error save exception"   , NULL },
	{ CAUSE_EXCCODE_IBE,  (char*)"Instruction bus error exception", NULL },
	{ CAUSE_EXCCODE_DBE,  (char*)"Data bus error exception"       , NULL },
	{ CAUSE_EXCCODE_SYS,  (char*)"Syscall exception"              , NULL },
	{ CAUSE_EXCCODE_BP,   (char*)"Breakpoint exception"           , NULL },
	{ CAUSE_EXCCODE_RI,   (char*)"Reserved instruction exception" , NULL },
	{ CAUSE_EXCCODE_CPU,  (char*)"Coprocessor unusable exception" , NULL },
	{ CAUSE_EXCCODE_OV,   (char*)"Integer overflow exception"     , NULL },
	{ CAUSE_EXCCODE_TR,   (char*)"Trap instruction exception"     , NULL }
};

const unative_t	CAUSE_EXCCODE_MASK  = 0x0000007c;
const unative_t	CAUSE_EXCCODE_SHIFT = 2;

/*----------------------------------------------------------------------------*/
inline Exceptions get_exccode(unative_t cause_reg)
{
	unative_t exc = (cause_reg & CAUSE_EXCCODE_MASK) >> CAUSE_EXCCODE_SHIFT;
	ASSERT ( exc <= (uint) CAUSE_EXCCODE_TR);
	return (Exceptions) exc;
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

} // namespace Processor

inline Processor::PageSize& operator++(Processor::PageSize& variable)
{
	if (variable != Processor::PAGE_MAX) {
		variable = (Processor::PageSize)(variable + 1);
	}
	return variable;
}

inline Processor::PageSize operator++(Processor::PageSize& variable, int)
{
	Processor::PageSize old = variable;
	++variable;
	return old;
}

inline Processor::PageSize& operator--(Processor::PageSize& variable)
{
	if (variable != Processor::PAGE_MIN) {
		variable = (Processor::PageSize)(variable - 1);
	}
	return variable;
}

inline Processor::PageSize operator--(Processor::PageSize& variable, int)
{
	Processor::PageSize old = variable;
	--variable;
	return old;
}

inline bool operator< (const Processor::PageSize left, const Processor::PageSize right)
{
	return (int)left < (int)right;
}

