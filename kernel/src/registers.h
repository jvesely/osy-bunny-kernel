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
 * @brief Register stuff taken from Kalisto.
 *
 * Lots of defines some of them are redundant and are present in 
 * drivers/Processor.h too (needs to get sorted).
 */

#pragma once
/***************************************************************************\
| Processor Related Definitions                                             |
\***************************************************************************/


/*
 * Constants for the System Control Coprocessor registers.
 * All the constants have the following name structure:
 *
 * 	CP0_<register name>_<item name>_{MASK,SHIFT}
 *
 */


/*
 * Index Register (read-write).
 * Used as a pointer to TLB by the TLBWI instruction.
 */
#define CP0_INDEX_INDEX_MASK   0x0000003f
#define CP0_INDEX_RES_MASK     0x7fffffc0
#define CP0_INDEX_P_MASK       0x80000000

#define CP0_INDEX_INDEX_SHIFT  0
#define CP0_INDEX_RES_SHIFT    6
#define CP0_INDEX_P_SHIFT      31

#define CP0_INDEX_INDEX(R)     (((R) & CP0_INDEX_INDEX_MASK) >> CP0_INDEX_INDEX_SHIFT)
#define CP0_INDEX_RES(R)       (((R) & CP0_INDEX_RES_MASK) >> CP0_INDEX_RES_SHIFT)
#define CP0_INDEX_P(R)         (((R) & CP0_INDEX_P_MASK) >> CP0_INDEX_P_SHIFT)

/* Number of entries in TLB. */
#define CP0_INDEX_INDEX_COUNT  48

/*
 * Random Register (read-only)
 *
 * Used as a pseudo-random pointer to TLB by the TLBWR instruction.
 * This register decrements every cycle from the maximum value of
 * 47 to the minimum value set in the Wired register. On reaching
 * the minimum value, the register is set back to the maximum value.
 *
 * The Random register is also set to the maximum
 * value when the Wired register is written.
 */
#define CP0_RANDOM_RANDOM_MASK   0x0000003f
#define CP0_RANDOM_RES_MASK      0xffffffc0

#define CP0_RANDOM_RANDOM_SHIFT  0
#define CP0_RANDOM_RES_SHIFT     6

#define CP0_RANDOM_RANDOM(R)     (((R) & CP0_RANDOM_RANDOM_MASK) >> CP0_RANDOM_RANDOM_SHIFT)
#define CP0_RANDOM_RES(R)        (((R) & CP0_RANDOM_RES_MASK) >> CP0_RANDOM_RES_SHIFT)

/*
 * Status Register (read-write)
 * Essential processor status control. Fields include:
 *
 *  - Interrupt Enable (IE) masks all interrupts.
 *  - Interrupt Mask (IM) masks individual interrupts.
 *  - Coprocessor Usability (CU)
 *  - Kernel-Supervisor-User (KSU)
 *      00 for kernel mode
 *      01 for supervisor mode
 *      10 for user mode
 *  - Exception Level (EXL)
 *      Set whenever an exception occur. Forces interrupts
 *      disabled and kernel mode until status can be set
 *      normally.
 *  - Error Level (ERL)
 * 	Set whenever RESET, SOFT RESET or NMI occurs.
 */

#define CP0_STATUS_IE_MASK       0x00000001
#define CP0_STATUS_EXL_MASK      0x00000002
#define CP0_STATUS_ERL_MASK      0x00000004
#define CP0_STATUS_KSU_MASK      0x00000018
#define CP0_STATUS_UX_MASK       0x00000020
#define CP0_STATUS_SX_MASK       0x00000040
#define CP0_STATUS_KX_MASK       0x00000080
#define CP0_STATUS_IM_MASK       0x0000ff00
	#define CP0_STATUS_IM0_MASK  0x00000100
	#define CP0_STATUS_IM1_MASK  0x00000200
	#define CP0_STATUS_IM2_MASK  0x00000400
	#define CP0_STATUS_IM3_MASK  0x00000800
	#define CP0_STATUS_IM4_MASK  0x00001000
	#define CP0_STATUS_IM5_MASK  0x00002000
	#define CP0_STATUS_IM6_MASK  0x00004000
	#define CP0_STATUS_IM7_MASK  0x00008000
#define CP0_STATUS_DE_MASK       0x00010000
#define CP0_STATUS_CE_MASK       0x00020000
#define CP0_STATUS_CH_MASK       0x00040000
#define CP0_STATUS_RES1_MASK     0x00080000
#define CP0_STATUS_SR_MASK       0x00100000
#define CP0_STATUS_TS_MASK       0x00200000
#define CP0_STATUS_BEV_MASK      0x00400000
#define CP0_STATUS_RES2_MASK     0x01800000
#define CP0_STATUS_RE_MASK       0x02000000
#define CP0_STATUS_FR_MASK       0x04000000
#define CP0_STATUS_RP_MASK       0x08000000
#define CP0_STATUS_CU0_MASK      0x10000000
#define CP0_STATUS_CU1_MASK      0x20000000
#define CP0_STATUS_CU2_MASK      0x40000000
#define CP0_STATUS_CU3_MASK      0x80000000
#define CP0_STATUS_CU_MASK       0xf0000000

#define CP0_STATUS_IE_SHIFT      0
#define CP0_STATUS_EXL_SHIFT     1
#define CP0_STATUS_ERL_SHIFT     2
#define CP0_STATUS_KSU_SHIFT     3
#define CP0_STATUS_UX_SHIFT      5
#define CP0_STATUS_SX_SHIFT      6
#define CP0_STATUS_KX_SHIFT      7
#define CP0_STATUS_IM_SHIFT      8
#define CP0_STATUS_DE_SHIFT      16
#define CP0_STATUS_CE_SHIFT      17
#define CP0_STATUS_CH_SHIFT      18
#define CP0_STATUS_RES1_SHIFT    19
#define CP0_STATUS_SR_SHIFT      20
#define CP0_STATUS_TS_SHIFT      21
#define CP0_STATUS_BEV_SHIFT     22
#define CP0_STATUS_RES2_SHIFT    23
#define CP0_STATUS_RE_SHIFT      25
#define CP0_STATUS_FR_SHIFT      26
#define CP0_STATUS_RP_SHIFT      27
#define CP0_STATUS_CU0_SHIFT     28
#define CP0_STATUS_CU1_SHIFT     29
#define CP0_STATUS_CU2_SHIFT     30
#define CP0_STATUS_CU3_SHIFT     31
#define CP0_STATUS_CU_SHIFT      28

#define CP0_STATUS_IE(R)         (((R) & CP0_STATUS_IE_MASK) >> CP0_STATUS_IE_SHIFT)
#define CP0_STATUS_EXL(R)        (((R) & CP0_STATUS_EXL_MASK) >> CP0_STATUS_EXL_SHIFT)
#define CP0_STATUS_ERL(R)        (((R) & CP0_STATUS_ERL_MASK) >> CP0_STATUS_ERL_SHIFT)
#define CP0_STATUS_KSU(R)        (((R) & CP0_STATUS_KSU_MASK) >> CP0_STATUS_KSU_SHIFT)
#define CP0_STATUS_UX(R)         (((R) & CP0_STATUS_UX_MASK) >> CP0_STATUS_UX_SHIFT)
#define CP0_STATUS_SX(R)         (((R) & CP0_STATUS_SX_MASK) >> CP0_STATUS_SX_SHIFT)
#define CP0_STATUS_KX(R)         (((R) & CP0_STATUS_KX_MASK) >> CP0_STATUS_KX_SHIFT)
#define CP0_STATUS_IM(R)         (((R) & CP0_STATUS_IM_MASK) >> CP0_STATUS_IM_SHIFT)
#define CP0_STATUS_DE(R)         (((R) & CP0_STATUS_DE_MASK) >> CP0_STATUS_DE_SHIFT)
#define CP0_STATUS_CE(R)         (((R) & CP0_STATUS_CE_MASK) >> CP0_STATUS_CE_SHIFT)
#define CP0_STATUS_CH(R)         (((R) & CP0_STATUS_CH_MASK) >> CP0_STATUS_CH_SHIFT)
#define CP0_STATUS_RES1(R)       (((R) & CP0_STATUS_RES1_MASK) >> CP0_STATUS_RES1_SHIFT)
#define CP0_STATUS_SR(R)         (((R) & CP0_STATUS_SR_MASK) >> CP0_STATUS_SR_SHIFT)
#define CP0_STATUS_TS(R)         (((R) & CP0_STATUS_TS_MASK) >> CP0_STATUS_TS_SHIFT)
#define CP0_STATUS_BEV(R)        (((R) & CP0_STATUS_BEV_MASK) >> CP0_STATUS_BEV_SHIFT)
#define CP0_STATUS_RES2(R)       (((R) & CP0_STATUS_RES2_MASK) >> CP0_STATUS_RES2_SHIFT)
#define CP0_STATUS_RE(R)         (((R) & CP0_STATUS_RE_MASK) >> CP0_STATUS_RE_SHIFT)
#define CP0_STATUS_FR(R)         (((R) & CP0_STATUS_FR_MASK) >> CP0_STATUS_FR_SHIFT)
#define CP0_STATUS_RP(R)         (((R) & CP0_STATUS_RP_MASK) >> CP0_STATUS_RP_SHIFT)
#define CP0_STATUS_CU0(R)        (((R) & CP0_STATUS_CU0_MASK) >> CP0_STATUS_CU0_SHIFT)
#define CP0_STATUS_CU1(R)        (((R) & CP0_STATUS_CU1_MASK) >> CP0_STATUS_CU1_SHIFT)
#define CP0_STATUS_CU2(R)        (((R) & CP0_STATUS_CU2_MASK) >> CP0_STATUS_CU2_SHIFT)
#define CP0_STATUS_CU3(R)        (((R) & CP0_STATUS_CU3_MASK) >> CP0_STATUS_CU3_SHIFT)
#define CP0_STATUS_CU(R)         (((R) & CP0_STATUS_CU_MASK) >> CP0_STATUS_CU_SHIFT)

/** EntryHi Register (read-write)
 *
 * Used when setting a TLB entry. Contains the high bits
 * of the virtual page number and the address space identification.
 *
 */
#define CP0_ENTRYHI_ASID_MASK    0x000000ff
#define CP0_ENTRYHI_RES1_MASK    0x00001f00
#define CP0_ENTRYHI_VPN2_MASK    0xffffe000

#define CP0_ENTRYHI_ASID_SHIFT   0
#define CP0_ENTRYHI_RES1_SHIFT   8
#define CP0_ENTRYHI_VPN2_SHIFT   13

#define CP0_ENTRYHI_ASID(R)      (((R) & CP0_ENTRYHI_ASID_MASK) >> CP0_ENTRYHI_ASID_SHIFT)
#define CP0_ENTRYHI_RES1(R)      (((R) & CP0_ENTRYHI_RES1_MASK) >> CP0_ENTRYHI_RES1_SHIFT)
#define CP0_ENTRYHI_VPN2(R)      (((R) & CP0_ENTRYHI_VPN2_MASK) >> CP0_ENTRYHI_VPN2_SHIFT)

/** EntryLo(0/1) Registers (read-write)
 *
 * Used when setting a TLB entry. Contain the physical
 * page number and the dirty, valid, global bits.
 *
 */
#define CP0_ENTRYLO_G_MASK      0x00000001
#define CP0_ENTRYLO_V_MASK      0x00000002
#define CP0_ENTRYLO_D_MASK      0x00000004
#define CP0_ENTRYLO_C_MASK      0x00000038
#define CP0_ENTRYLO_PFN_MASK    0x3fffffc0
#define CP0_ENTRYLO_RES1_MASK   0xc0000000

#define CP0_ENTRYLO_G_SHIFT     0
#define CP0_ENTRYLO_V_SHIFT     1
#define CP0_ENTRYLO_D_SHIFT     2
#define CP0_ENTRYLO_C_SHIFT     3
#define CP0_ENTRYLO_PFN_SHIFT   6
#define CP0_ENTRYLO_RES1_SHIFT  30

#define CP0_ENTRYLO0_G(R)       (((R) & CP0_ENTRYLO_G_MASK) >> CP0_ENTRYLO_G_SHIFT)
#define CP0_ENTRYLO0_V(R)       (((R) & CP0_ENTRYLO_V_MASK) >> CP0_ENTRYLO_V_SHIFT)
#define CP0_ENTRYLO0_D(R)       (((R) & CP0_ENTRYLO_D_MASK) >> CP0_ENTRYLO_D_SHIFT)
#define CP0_ENTRYLO0_C(R)       (((R) & CP0_ENTRYLO_C_MASK) >> CP0_ENTRYLO_C_SHIFT)
#define CP0_ENTRYLO0_PFN(R)     (((R) & CP0_ENTRYLO_PFN_MASK) >> CP0_ENTRYLO_PFN_SHIFT)
#define CP0_ENTRYLO0_RES1(R)    (((R) & CP0_ENTRYLO_RES1_MASK) >> CP0_ENTRYLO_RES1_SHIFT)

#define CP0_ENTRYLO1_G(R)       (((R) & CP0_ENTRYLO_G_MASK) >> CP0_ENTRYLO_G_SHIFT)
#define CP0_ENTRYLO1_V(R)       (((R) & CP0_ENTRYLO_V_MASK) >> CP0_ENTRYLO_V_SHIFT)
#define CP0_ENTRYLO1_D(R)       (((R) & CP0_ENTRYLO_D_MASK) >> CP0_ENTRYLO_D_SHIFT)
#define CP0_ENTRYLO1_C(R)       (((R) & CP0_ENTRYLO_C_MASK) >> CP0_ENTRYLO_C_SHIFT)
#define CP0_ENTRYLO1_PFN(R)     (((R) & CP0_ENTRYLO_PFN_MASK) >> CP0_ENTRYLO_PFN_SHIFT)
#define CP0_ENTRYLO1_RES1(R)    (((R) & CP0_ENTRYLO_RES1_MASK) >> CP0_ENTRYLO_RES1_SHIFT)

/* Number of bits in EntryLo virtual address. */
#define CP0_ENTRYLO_PFN_BITS    24

/** Wired Register (read-write)
 *
 * Used as a pointer to TLB by the TLBWR instruction.
 * Entries below the value of this register are never
 * chosen as victims by the TLBWR instruction.
 *
 */
#define CP0_WIRED_W_MASK      0x0000001f
#define CP0_WIRED_RES1_MASK   0xffffffe0

#define CP0_WIRED_W_SHIFT     0
#define CP0_WIRED_RES1_SHIFT  6

#define CP0_WIRED_W(R) 		  (((R) & CP0_WIRED_W_MASK) >> CP0_WIRED_W_SHIFT)
#define CP0_WIRED_RES1(R) 	  (((R) & CP0_WIRED_RES1_MASK) >> CP0_WIRED_RES1_SHIFT)

/** Context Register (read-write)
 *
 * Information on TLB exception.
 *
 */
#define CP0_CONTEXT_RES1_MASK       0x0000000f
#define CP0_CONTEXT_BADVPN2_MASK    0x007ffff0
#define CP0_CONTEXT_PTEBASE_MASK    0xff800000

#define CP0_CONTEXT_RES1_SHIFT      0
#define CP0_CONTEXT_BADVPN2_SHIFT   4
#define CP0_CONTEXT_PTEBASE_SHIFT   23

#define CP0_CONTEXT_RES1(R)         (((R) & CP0_CONTEXT_RES1_MASK) >> CP0_CONTEXT_RES1_SHIFT)
#define CP0_CONTEXT_BADVPN2(R)      (((R) & CP0_CONTEXT_BADVPN2_MASK) >> CP0_CONTEXT_BADVPN2_SHIFT)
#define CP0_CONTEXT_PTEBASE(R)      (((R) & CP0_CONTEXT_PTEBASE_MASK) >> CP0_CONTEXT_PTEBASE_SHIFT)

/** Pagemask Register (read-write)
 *
 * Used when setting a TLB entry.
 *
 */
#define CP0_PAGEMASK_RES1_MASK      0x00001fff
#define CP0_PAGEMASK_MASK_MASK      0x01ffe000
#define CP0_PAGEMASK_RES2_MASK      0xfe000000

#define CP0_PAGEMASK_RES1_SHIFT     0
#define CP0_PAGEMASK_MASK_SHIFT     13
#define CP0_PAGEMASK_RES2_SHIFT     25

#define CP0_PAGEMASK_RES1(R)        (((R) & CP0_PAGEMASK_RE S1_MASK) >> CP0_PAGEMASK_RES1_SHIFT)
#define CP0_PAGEMASK_MASK(R)        (((R) & CP0_PAGEMASK_MA SK_MASK) >> CP0_PAGEMASK_MASK_SHIFT)
#define CP0_PAGEMASK_RES2(R)        (((R) & CP0_PAGEMASK_RE S2_MASK) >> CP0_PAGEMASK_RES2_SHIFT)

#define	CP0_PAGEMASK_4K             (0x000 << CP0_PAGEMASK_MASK_SHIFT)
#define	CP0_PAGEMASK_16K            (0x003 << CP0_PAGEMASK_MASK_SHIFT)
#define	CP0_PAGEMASK_64K            (0x00f << CP0_PAGEMASK_MASK_SHIFT)
#define	CP0_PAGEMASK_256K           (0x03f << CP0_PAGEMASK_MASK_SHIFT)
#define	CP0_PAGEMASK_1M             (0x0ff << CP0_PAGEMASK_MASK_SHIFT)
#define	CP0_PAGEMASK_4M             (0x3ff << CP0_PAGEMASK_MASK_SHIFT)
#define	CP0_PAGEMASK_16M            (0xfff << CP0_PAGEMASK_MASK_SHIFT)

/** Count Register (read-write)
 *
 * Counts system ticks, useful for timing.
 *
 */
#define CP0_COUNT_COUNT_MASK        0xffffffff
#define CP0_COUNT_COUNT_SHIFT       0
#define CP0_COUNT_COUNT(R)          (((R) & CP0_COUNT_COUNT_MASK) >> CP0_COUNT_COUNT_SHIFT)

/** Bad Virtual Address Register (read-only)
 *
 * Information on TLB exception.
 *
 */
#define CP0_BADVAADDR_BADVAADDR_MASK    0xffffffff
#define CP0_BADVAADDR_BADVAADDR_SHIFT   0
#define CP0_BADVAADDR_BADVAADDR(R)      (((R) & CP0_BADVAADDR_BADVAADDR_MASK) >> CP0_BADVAADDR_BADVAADDR_SHIFT)

/** Compare (read-write)
 *
 * An interrupt is generated when the value of Count reaches the value of Compare.
 * The interrupt request is dropped when a new value is written into Compare.
 *
 */
#define CP0_COMPARE_COMPARE_MASK    0xffffffff
#define CP0_COMPARE_COMPARE_SHIFT   0
#define CP0_COMPARE_COMPARE(R)      (((R) & CP0_COMPARE_COMPARE_MASK) >> CP0_COMPARE_COMPARE_SHIFT)

/** Exception Program Counter (read-write)
 *
 * Stores the address at which an exception occured. If the exception
 * occured in the branch delay slot, the register points to the branch
 * instruction and the BD field in the Cause register is set.
 *
 */
#define CP0_EPC_EPC_MASK    0xffffffff
#define CP0_EPC_EPC_SHIFT   0
#define CP0_EPC_EPC(R)      (((R) & CP0_EPC_EPC_MASK) >> CP0_EPC_EPC_SHIFT)

/** Cause Register (read-write)
 *
 * General information about an exception, in particular:
 *
 *  - Exception Code (EXCCODE) tells the reason for the exception.
 *  - Interrupt Pending (IP) tells which interrupt is pending.
 *  - Coprocessor Error (CE) tells which coprocessor is unusable.
 *  - Branch Delay (BD) tells that an exception occured in the branch delay slot.
 *
 */
#define CP0_CAUSE_RES1_MASK     0x00000003
#define CP0_CAUSE_EXCCODE_MASK  0x0000007c
#define CP0_CAUSE_RES2_MASK     0x00000080
#define CP0_CAUSE_IP_MASK       0x0000ff00
	#define CP0_CAUSE_IP0_MASK  0x00000100
	#define CP0_CAUSE_IP1_MASK  0x00000200
	#define CP0_CAUSE_IP2_MASK  0x00000400
	#define CP0_CAUSE_IP3_MASK  0x00000800
	#define CP0_CAUSE_IP4_MASK  0x00001000
	#define CP0_CAUSE_IP5_MASK  0x00002000
	#define CP0_CAUSE_IP6_MASK  0x00004000
	#define CP0_CAUSE_IP7_MASK  0x00008000
#define CP0_CAUSE_RES3_MASK     0x0fff0000
#define CP0_CAUSE_CE_MASK       0x30000000
#define CP0_CAUSE_BD_MASK       0x80000000
#define CP0_CAUSE_RES4_MASK     0x40000000

#define CP0_CAUSE_CE_CU1        0x10000000
#define CP0_CAUSE_CE_CU2        0x20000000
#define CP0_CAUSE_CE_CU3        0x30000000

#define CP0_CAUSE_RES1_SHIFT    0
#define CP0_CAUSE_EXCCODE_SHIFT 2
#define CP0_CAUSE_RES2_SHIFT    7
#define CP0_CAUSE_IP_SHIFT      8
	#define CP0_CAUSE_IP0_SHIFT	8
	#define CP0_CAUSE_IP1_SHIFT	9
	#define CP0_CAUSE_IP2_SHIFT	10
	#define CP0_CAUSE_IP3_SHIFT	11
	#define CP0_CAUSE_IP4_SHIFT	12
	#define CP0_CAUSE_IP5_SHIFT	13
	#define CP0_CAUSE_IP6_SHIFT 14
	#define CP0_CAUSE_IP7_SHIFT 15
#define CP0_CAUSE_RES3_SHIFT    16
#define CP0_CAUSE_CE_SHIFT      28
#define CP0_CAUSE_RES4_SHIFT    30
#define CP0_CAUSE_BD_SHIFT      31

#define CP0_CAUSE_RES1(R)       (((R) & CP0_CAUSE_RES1_MASK) >> CP0_CAUSE_RES1_SHIFT)
#define CP0_CAUSE_EXCCODE(R)    (((R) & CP0_CAUSE_EXCCODE_MASK) >> CP0_CAUSE_EXCCODE_SHIFT)
#define CP0_CAUSE_RES2(R)       (((R) & CP0_CAUSE_RES2_MASK) >> CP0_CAUSE_RES2_SHIFT)
#define CP0_CAUSE_IP(R)         (((R) & CP0_CAUSE_IP_MASK) >> CP0_CAUSE_IP_SHIFT)
#define CP0_CAUSE_RES3(R)       (((R) & CP0_CAUSE_RES3_MASK) >> CP0_CAUSE_RES3_SHIFT)
#define CP0_CAUSE_CE(R)         (((R) & CP0_CAUSE_CE_MASK) >> CP0_CAUSE_CE_SHIFT)
#define CP0_CAUSE_RES4(R)       (((R) & CP0_CAUSE_RES4_MASK) >> CP0_CAUSE_RES4_SHIFT)
#define CP0_CAUSE_BD(R)         (((R) & CP0_CAUSE_BD_MASK) >> CP0_CAUSE_BD_SHIFT)

/* Exception caused by an interrupt. */
#define CP0_CAUSE_EXCCODE_INT   0

/* TLB Modification Exception. */
#define CP0_CAUSE_EXCCODE_MOD   1

/* TLB Load Exception. */
#define CP0_CAUSE_EXCCODE_TLBL  2

/* TLB Store Exception. */
#define CP0_CAUSE_EXCCODE_TLBS  3

/* Address Error Load Exception. */
#define CP0_CAUSE_EXCCODE_ADEL  4

/* Address Error Store Exception. */
#define CP0_CAUSE_EXCCODE_ADES  5

/* Instruction Bus Error Exception. */
#define CP0_CAUSE_EXCCODE_IBE   6

/* Data Bus Error Exception. */
#define CP0_CAUSE_EXCCODE_DBE   7

/* Exception caused by a syscall. */
#define CP0_CAUSE_EXCCODE_SYS   8

/* Exception caused by a breakpoint. */
#define CP0_CAUSE_EXCCODE_BP    9

/* Reserved Instruction Exception. */
#define CP0_CAUSE_EXCCODE_RI    10

/* Coprocessor Unusable Exception. */
#define CP0_CAUSE_EXCCODE_CPU   11

/* Integer Overflow Exception. */
#define CP0_CAUSE_EXCCODE_OV    12

/* Trap Exception. */
#define CP0_CAUSE_EXCCODE_TR    13
