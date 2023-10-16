/******************************************************************************
Filename    : rme_platform_rv32g.h
Author      : pry
Date        : 01/04/2017
Licence     : The Unlicense; see LICENSE for details.
Description : The header of "rme_platform_rv32gp.c".
******************************************************************************/

/* Defines *******************************************************************/
#ifdef __HDR_DEFS__
#ifndef __RME_PLATFORM_RV32GP_H_DEFS__
#define __RME_PLATFORM_RV32GP_H_DEFS__
/*****************************************************************************/
/* Basic Types ***************************************************************/
#ifndef __RME_S32_T__
#define __RME_S32_T__
typedef signed int rme_s32_t;
#endif

#ifndef __RME_S16_T__
#define __RME_S16_T__
typedef signed short rme_s16_t;
#endif

#ifndef __RME_S8_T__
#define __RME_S8_T__
typedef signed char rme_s8_t;
#endif

#ifndef __RME_U32_T__
#define __RME_U32_T__
typedef unsigned int rme_u32_t;
#endif

#ifndef __RME_U16_T__
#define __RME_U16_T__
typedef unsigned short rme_u16_t;
#endif

#ifndef __RME_U8_T__
#define __RME_U8_T__
typedef unsigned char rme_u8_t;
#endif
/* End Basic Types ***********************************************************/

/* Begin Extended Types ******************************************************/
#ifndef __RME_TID_T__
#define __RME_TID_T__
/* The typedef for the Thread ID */
typedef rme_s32_t rme_tid_t;
#endif

#ifndef __RME_PTR_T__
#define __RME_PTR_T__
/* The typedef for the pointers - This is the raw style. Pointers must be unsigned */
typedef rme_u32_t rme_ptr_t;
#endif

#ifndef __RME_CNT_T__
#define __RME_CNT_T__
/* The typedef for the count variables */
typedef rme_s32_t rme_cnt_t;
#endif

#ifndef __RME_CID_T__
#define __RME_CID_T__
/* The typedef for capability ID */
typedef rme_s32_t rme_cid_t;
#endif

#ifndef __RME_RET_T__
#define __RME_RET_T__
/* The type for process return value */
typedef rme_s32_t rme_ret_t;
#endif
/* End Extended Types ********************************************************/

/* System Macros *************************************************************/
/* Compiler "extern" keyword setting */
#define EXTERN                          extern
/* Compiler "inline" keyword setting */
#define INLINE                          inline
/* Compiler likely & unlikely setting */
#ifdef likely
#define RME_LIKELY(X)                   (likely(X))
#else
#define RME_LIKELY(X)                   (X)
#endif
#ifdef unlikely
#define RME_UNLIKELY(X)                 (unlikely(X))
#else
#define RME_UNLIKELY(X)                 (X)
#endif
/* CPU-local data structure location macro */
#define RME_CPU_LOCAL()                 (&RME_RV32GP_Local)
/* The order of bits in one CPU machine word */
#define RME_WORD_ORDER                  (5U)
/* Forcing VA=PA in user memory segments */
#define RME_VA_EQU_PA                   (1U)
/* Quiescence timeslice value */
#define RME_QUIE_TIME                   (0U)
/* Read timestamp counter */
#define RME_TIMESTAMP                   (RME_RV32GP_Timestamp)
/* Cpt size limit - not restricted */
#define RME_CPT_ENTRY_MAX               (0U)
/* Normal page directory size calculation macro */
#define RME_PGT_SIZE_NOM(NUM_ORDER)     (RME_POW2(NUM_ORDER)*sizeof(rme_ptr_t)+sizeof(struct __RME_RV32GP_Pgt_Meta))
/* Top-level page directory size calculation macro */
#define RME_PGT_SIZE_TOP(NUM_ORDER)     (RME_PGT_SIZE_NOM(NUM_ORDER)+sizeof(struct __RME_RV32GP_PMP_Data))
/* The kernel object allocation table address - original */
#define RME_KOT_VA_BASE                 RME_RV32GP_Kot
/* Compare-and-Swap(CAS) */
#define RME_COMP_SWAP(PTR,OLD,NEW)      __RME_RV32GP_Comp_Swap(PTR,OLD,NEW)
/* Fetch-and-Add(FAA) */
#define RME_FETCH_ADD(PTR,ADDEND)       __RME_RV32GP_Fetch_Add(PTR,ADDEND)
/* Fetch-and-And(FAND) */
#define RME_FETCH_AND(PTR,OPERAND)      __RME_RV32GP_Fetch_And(PTR,OPERAND)
/* Get most significant bit */
#define RME_MSB_GET(VAL)                __RME_RV32GP_MSB_Get(VAL)
/* No read/write barriers needed on , because they are currently all
 * single core. If this changes in the future, we may need DMB barriers. */
#define RME_READ_ACQUIRE(X)             (*(X))
#define RME_WRITE_RELEASE(X, V)         ((*(X))=(V))

#define __RME_RV32GP_Reset()          NVIC_SystemReset()
/* Reboot the processor if the assert fails in this port */
#define RME_ASSERT_FAILED(F, L, D, T)   __RME_RV32GP_Reboot()

/* The CPU and application specific macros are here */
#include "rme_platform_rv32gp_conf.h"

/* Vector/signal flag */
#define RME_RVM_VCT_SIG_ALL             (0U)
#define RME_RVM_VCT_SIG_SELF            (1U)
#define RME_RVM_VCT_SIG_INIT            (2U)
#define RME_RVM_VCT_SIG_NONE            (3U)
#define RME_RVM_FLAG_SET(B, S, N)       ((volatile struct __RME_RVM_Flag*)((B)+((S)>>1)*(N)))
/* End System Macros *********************************************************/

/* RV32GP Specific Macros ****************************************************/
/* PMP Metadata Size *********************************************************/
#define RME_RV32GP_PMPCFG_NUM           ((RME_RV32GP_REGION_NUM+3U)>>2)
/* Registers *****************************************************************/
#define RME_RV32GP_REG(X)               (*((volatile rme_ptr_t*)(X)))
#define RME_RV32GP_REGB(X)              (*((volatile rme_u8_t*)(X)))

#define RME_RV32GP_STK_CTLR             RME_RV32GP_REG(0xE000F000U)
#define RME_RV32GP_STK_CTLR_STCLK       (1U<<2)
#define RME_RV32GP_STK_CTLR_STIE        (1U<<1)
#define RME_RV32GP_STK_CTLR_STE         (1U<<0)

#define RME_RV32GP_STK_CMPLR            RME_RV32GP_REG(0xE000F010U)
#define RME_RV32GP_STK_CNTL             RME_RV32GP_REG(0xE000F008U)
#define RME_RV32GP_STK_CNTH             RME_RV32GP_REG(0xE000F00CU)

#define RME_RV32GP_NVIC_IPR(X)          NVIC->IPRIOR[(uint32_t)(X)];

#define RME_RV32GP_NVIC_GROUPING_0      (0U)
#define RME_RV32GP_NVIC_GROUPING_1      (1U)
#define RME_RV32GP_NVIC_GROUPING_2      (2U)
#define RME_RV32GP_NVIC_GROUPING_3      (3U)
#define RME_RV32GP_NVIC_GROUPING_4      (4U)

/* Generic *******************************************************************/
/* MSTATUS default initialization */
#define RME_RV32GP_MSTATUS_INIT         (0x00000800U)
#define RME_RV32GP_MSTATUS_MASK         (0x00003000U)
#define RME_RV32GP_MSTATUS_FIX(X) \
do \
{ \
    if(RME_THD_ATTR(RME_CPU_LOCAL()->Thd_Cur->Ctx.Hyp_Attr)==RME_RV32GP_ATTR_NONE) \
        (X)->MSTATUS=RME_RV32GP_MSTATUS_INIT; \
    else \
        (X)->MSTATUS=(X)->MSTATUS&RME_RV32GP_MSTATUS_MASK|RME_RV32GP_MSTATUS_INIT; \
} \
while(0)
/* MSTATUS FPU states */
#define RME_RV32GP_MSTATUS_FPU_MASK     (0x00003000U)
#define RME_RV32GP_MSTATUS_FPU_OFF      (0x00001000U)
#define RME_RV32GP_MSTATUS_FPU_INIT     (0x00001000U)
#define RME_RV32GP_MSTATUS_FPU_CLEAN    (0x00002000U)
#define RME_RV32GP_MSTATUS_FPU_DIRTY    (0x00003000U)
/* FPU save/restore veneer choice */
#if(RME_COP_NUM!=0U)
#if(RME_RV32GP_COP_RVD==0U)
#define ___RME_RV32GP_Thd_Cop_Clear     ___RME_RV32GP_Thd_Cop_Clear_RVD
#define ___RME_RV32GP_Thd_Cop_Save      ___RME_RV32GP_Thd_Cop_Save_RVD
#define ___RME_RV32GP_Thd_Cop_Load      ___RME_RV32GP_Thd_Cop_Load_RVD
#else
#define ___RME_RV32GP_Thd_Cop_Clear     ___RME_RV32GP_Thd_Cop_Clear_RVF
#define ___RME_RV32GP_Thd_Cop_Save      ___RME_RV32GP_Thd_Cop_Save_RVF
#define ___RME_RV32GP_Thd_Cop_Load      ___RME_RV32GP_Thd_Cop_Load_RVF
#endif
#endif

/* FPU type definitions */
#define RME_RV32GP_ATTR_NONE            (0U)
#define RME_RV32GP_ATTR_RVF             (1U<<0)
#define RME_RV32GP_ATTR_RVFD            (1U<<1)
/* Other packed and vectored insns are not supported at thme moment */

/* Handler *******************************************************************/
/* Generic interrupt source definitions */
/* Software interrupts from user/supervisor/hypervisor/machine mode */
#define RME_RV32GP_MCAUSE_U_SWI         (0x80000000U)
#define RME_RV32GP_MCAUSE_S_SWI         (0x80000001U)
#define RME_RV32GP_MCAUSE_H_SWI         (0x80000002U)
#define RME_RV32GP_MCAUSE_M_SWI         (0x80000003U)
/* Timer interrupts from user/supervisor/hypervisor/machine mode */
#define RME_RV32GP_MCAUSE_U_TIM         (0x80000004U)
#define RME_RV32GP_MCAUSE_S_TIM         (0x80000005U)
#define RME_RV32GP_MCAUSE_H_TIM         (0x80000006U)
#define RME_RV32GP_MCAUSE_M_TIM         (0x80000007U)
/* Timer interrupts from user/supervisor/hypervisor/machine mode */
#define RME_RV32GP_MCAUSE_U_EXT         (0x80000008U)
#define RME_RV32GP_MCAUSE_S_EXT         (0x80000009U)
#define RME_RV32GP_MCAUSE_H_EXT         (0x8000000AU)
#define RME_RV32GP_MCAUSE_M_EXT         (0x8000000BU)
/* Fault definitions */
/* Instruction misaligned */
#define RME_RV32GP_MCAUSE_IMALIGN       (0x00000000U)
/* Instruction access fault */
#define RME_RV32GP_MCAUSE_IACCFLT       (0x00000001U)
/* Undefined instruction */
#define RME_RV32GP_MCAUSE_IUNDEF        (0x00000002U)
/* Breakpoint */
#define RME_RV32GP_MCAUSE_IBRKPT        (0x00000003U)
/* Data load misaligned */
#define RME_RV32GP_MCAUSE_LALIGN        (0x00000004U)
/* Data load access fault */
#define RME_RV32GP_MCAUSE_LACCFLT       (0x00000005U)
/* Data store misaligned */
#define RME_RV32GP_MCAUSE_SALIGN        (0x00000006U)
/* Data store access fault */
#define RME_RV32GP_MCAUSE_SACCFLT       (0x00000007U)
/* System calls from user/supervisor/hypervisor/machine mode */
#define RME_RV32GP_MCAUSE_U_ECALL       (0x00000008U)
#define RME_RV32GP_MCAUSE_S_ECALL       (0x00000009U)
#define RME_RV32GP_MCAUSE_H_ECALL       (0x0000000AU)
#define RME_RV32GP_MCAUSE_M_ECALL       (0x0000000BU)
/* Instruction page fault */
#define RME_RV32GP_MCAUSE_IPGFLT        (0x0000000CU)
/* Load page fault */
#define RME_RV32GP_MCAUSE_LPGFLT        (0x0000000DU)
/* Store page fault */
#define RME_RV32GP_MCAUSE_SPGFLT        (0x0000000FU)

/* Initialization ************************************************************/
/* The capability table of the init process */
#define RME_BOOT_INIT_CPT               (0U)
/* The top-level page table of the init process - always 4GB full range split into 8 pages */
#define RME_BOOT_INIT_PGT               (1U)
/* The init process */
#define RME_BOOT_INIT_PRC               (2U)
/* The init thread */
#define RME_BOOT_INIT_THD               (3U)
/* The initial kernel function capability */
#define RME_BOOT_INIT_KFN               (4U)
/* The initial kernel memory capability */
#define RME_BOOT_INIT_KOM               (5U)
/* The initial timer endpoint */
#define RME_BOOT_INIT_TIM               (6U)
/* The initial default endpoint for all other vectors */
#define RME_BOOT_INIT_VCT               (7U)

/* Booting capability layout */
#define RME_RV32GP_CPT                  ((struct RME_Cap_Cpt*)(RME_KOM_VA_BASE))

/* Page Table ****************************************************************/
/* For ARMv7-M:
 * The layout of the page entry is:
 * [31:5] Paddr - The physical address to map this page to, or the physical
 *                address of the next layer of page table. This address is
 *                always aligned to 32 bytes.
 * [4:2] Reserved - Because subregions must share attributes, we have the permission
 *                  flags in the page table headers ("Page_Flags" field). These flags
 *                  are completely identical to RME standard page flags.
 * [1] Terminal - Is this page a terminal page, or points to another page table?
 * [0] Present - Is this entry present?
 *
 * The layout of a directory entry is:
 * [31:2] Paddr - The in-kernel physical address of the lower page directory.
 * [1] Terminal - Is this page a terminal page, or points to another page table?
 * [0] Present - Is this entry present?
 */

/* Get the actual table positions */
#define RME_RV32GP_PGT_META                (sizeof(struct __RME_RV32GP_Pgt_Meta)/sizeof(rme_ptr_t))
#define RME_RV32GP_PMP_DATA                (sizeof(struct __RME_RV32GP_PMP_Data)/sizeof(rme_ptr_t))
#define RME_RV32GP_PGT_TBL_NOM(X)          (((rme_ptr_t*)(X))+RME_RV32GP_PGT_META)
#define RME_RV32GP_PGT_TBL_TOP(X)          (((rme_ptr_t*)(X))+RME_RV32GP_PGT_META+RME_RV32GP_PMP_DATA)
/* Get the PMP region size and base address */
#define RME_RV32GP_PMP_SIZE_ADDR(size,addr) \
int order = 0; \
while(1) \
{ \
    if((addr) & 0x1 == 1) \
    { \
        (addr) = (addr) >> 1; \
        order++; \
    } \
    else \
    { \
        addr = addr << (order+2); \
        order += 3; \
        size = 0x1<< (order); \
        break; \
    } \
}
/* Get the value of pmpaddr */
#define RME_RV32GP_PMP_ADDR(Base_Addr,order) (((Base_Addr) >> 2) | ((1<<((order)-3))-1))
/* Page entry bit definitions */
#define RME_RV32GP_PGT_PRESENT          (1U<<0)
#define RME_RV32GP_PGT_TERMINAL         (1U<<1)
/* The address mask for the actual page address */
#define RME_RV32GP_PGT_PTE_ADDR(X)      ((X)&0xFFFFFFFCU)
/* The address mask for the next level page table address */
#define RME_RV32GP_PGT_PGD_ADDR(X)      ((X)&0xFFFFFFFCU)
/* Page table metadata definitions */
#define RME_RV32GP_PGT_START(X)         ((X)&0xFFFFFFFEU)
#define RME_RV32GP_PGT_SIZEORD(X)       ((X)>>16)
#define RME_RV32GP_PGT_NUMORD(X)        ((X)&0x0000FFFFU)
#define RME_RV32GP_PGT_DIRNUM(X)        ((X)>>16)
#define RME_RV32GP_PGT_PAGENUM(X)       ((X)&0x0000FFFFU)
#define RME_RV32GP_PGT_INC_PAGENUM(X)   ((X)+=0x00000001U)
#define RME_A7M_PGT_DEC_PAGENUM(X)      ((X)-=0x00000001U)
#define RME_RV32GP_PGT_INC_DIRNUM(X)    ((X)+=0x00010000U)
#define RME_RV32GP_PGT_DEC_DIRNUM(X)    ((X)-=0x00010000U)
/* MPU operation flag */
#define RME_RV32GP_PMP_CLR              (0U)
#define RME_RV32GP_PMP_UPD              (1U)
/* MPU definitions */
#define RME_RV32GP_PMP_ENABLE(x)        (0x00011000 << (x*8))
/* Write info to MPU - X is the region's size order, not a subregion */
#define RME_RV32GP_PMP_R                (1U<<0)  //Readable attributes
#define RME_RV32GP_PMP_W                (1U<<1)  //Writeable attributes
#define RME_RV32GP_PMP_X                (1U<<2)  //Executable attributes
#define RME_RV32GP_PMP_A                (3U<<3)  //Address alignment and selection of protected area range,11 is NAPOT
#define RME_RV32GP_PMP_L(X)             (X & 0x80)  //Lock Enable

/* Platform-specific kernel function macros **********************************/
/* Page table entry mode which property to get */
#define RME_RV32GP_KFN_PGT_ENTRY_MOD_GET_FLAGS          (0U)
#define RME_RV32GP_KFN_PGT_ENTRY_MOD_GET_SIZEORDER      (1U)
#define RME_RV32GP_KFN_PGT_ENTRY_MOD_GET_NUMORDER       (2U)
/* Interrupt source configuration */
#define RME_RV32GP_KFN_INT_LOCAL_MOD_GET_STATE          (0U)
#define RME_RV32GP_KFN_INT_LOCAL_MOD_SET_STATE          (1U)
#define RME_RV32GP_KFN_INT_LOCAL_MOD_GET_PRIO           (2U)
#define RME_RV32GP_KFN_INT_LOCAL_MOD_SET_PRIO           (3U)
/* Prefetcher modification */
#define RME_RV32GP_KFN_PRFTH_MOD_GET_STATE              (0U)
#define RME_RV32GP_KFN_PRFTH_MOD_SET_STATE              (1U)
/* Prefetcher state */
#define RME_RV32GP_KFN_PRFTH_STATE_DISABLE              (0U)
#define RME_RV32GP_KFN_PRFTH_STATE_ENABLE               (1U)

/* Register read/write */
#define RME_RV32GP_KFN_DEBUG_REG_MOD_GET                (0U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_SET                (1U<<16U)
/* General-purpose registers */
#define RME_RV32GP_KFN_DEBUG_REG_MOD_MSTATUS            (0U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_PC                 (1U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_X1_RA              (2U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_X2_SP              (3U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_X3_GP              (4U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_X4_TP              (5U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_X5_T0              (6U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_X6_T1              (7U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_X7_T2              (8U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_X8_S0_FP           (9U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_X9_S1              (10U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_X10_A0             (11U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_X11_A1             (12U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_X12_A2             (13U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_X13_A3             (14U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_X14_A4             (15U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_X15_A5             (16U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_X16_A6             (17U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_X17_A7             (18U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_X18_S2             (19U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_X19_S3             (20U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_X20_S4             (21U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_X21_S5             (22U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_X22_S6             (23U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_X23_S7             (24U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_X24_S8             (25U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_X25_S9             (26U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_X26_S10            (27U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_X27_S11            (28U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_X28_T3             (29U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_X29_T4             (30U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_X30_T5             (31U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_X31_T6             (32U)
/* FCSR & FPU registers */
#define RME_RV32GP_KFN_DEBUG_REG_MOD_FCSR               (33U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_F0                 (34U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_F1                 (35U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_F2                 (36U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_F3                 (37U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_F4                 (38U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_F5                 (39U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_F6                 (40U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_F7                 (41U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_F8                 (42U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_F9                 (43U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_F10                (44U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_F11                (45U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_F12                (46U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_F13                (47U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_F14                (48U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_F15                (49U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_F16                (50U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_F17                (51U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_F18                (52U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_F19                (53U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_F20                (54U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_F21                (55U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_F22                (56U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_F23                (57U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_F24                (58U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_F25                (59U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_F26                (60U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_F27                (61U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_F28                (62U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_F29                (63U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_F30                (64U)
#define RME_RV32GP_KFN_DEBUG_REG_MOD_F31                (65U)

/* Invocation register read/write */
#define RME_RV32GP_KFN_DEBUG_INV_MOD_SP_GET             (0U)
#define RME_RV32GP_KFN_DEBUG_INV_MOD_SP_SET             (1U)
#define RME_RV32GP_KFN_DEBUG_INV_MOD_PC_GET             (2U)
#define RME_RV32GP_KFN_DEBUG_INV_MOD_PC_SET             (3U)

/* Exception register read */
#define RME_RV32GP_KFN_DEBUG_EXC_CAUSE_GET              (0U)
#define RME_RV32GP_KFN_DEBUG_EXC_ADDR_GET               (1U)
/*****************************************************************************/
/* __RME_PLATFORM_A7M_H_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Structs *******************************************************************/
#ifdef __HDR_STRUCTS__
#ifndef __RME_PLATFORM_RV32GP_H_STRUCTS__
#define __RME_PLATFORM_RV32GP_H_STRUCTS__
/* We used structs in the header */

/* Use defines in these headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__
/*****************************************************************************/
/* Handler *******************************************************************/
/* Interrupt flag structure */
struct __RME_RVM_Flag
{
    rme_ptr_t Lock;
    rme_ptr_t Fast;
    rme_ptr_t Group;
    rme_ptr_t Flag[1024];
};

/* Register Manipulation *****************************************************/
/* The register set struct */
struct RME_Reg_Struct
{
    rme_ptr_t MSTATUS;
    rme_ptr_t PC;
    rme_ptr_t X1_RA;
    rme_ptr_t X2_SP;
    rme_ptr_t X3_GP;
    rme_ptr_t X4_TP;
    rme_ptr_t X5_T0;
    rme_ptr_t X6_T1;
    rme_ptr_t X7_T2;
    rme_ptr_t X8_S0_FP;
    rme_ptr_t X9_S1;
    rme_ptr_t X10_A0;
    rme_ptr_t X11_A1;
    rme_ptr_t X12_A2;
    rme_ptr_t X13_A3;
    rme_ptr_t X14_A4;
    rme_ptr_t X15_A5;
    rme_ptr_t X16_A6;
    rme_ptr_t X17_A7;
    rme_ptr_t X18_S2;
    rme_ptr_t X19_S3;
    rme_ptr_t X20_S4;
    rme_ptr_t X21_S5;
    rme_ptr_t X22_S6;
    rme_ptr_t X23_S7;
    rme_ptr_t X24_S8;
    rme_ptr_t X25_S9;
    rme_ptr_t X26_S10;
    rme_ptr_t X27_S11;
    rme_ptr_t X28_T3;
    rme_ptr_t X29_T4;
    rme_ptr_t X30_T5;
    rme_ptr_t X31_T6;
};

#if(RME_COP_NUM!=0U)
#if(RME_RV32GP_COP_RVD==0U)
/* Single-precision register set */
struct RME_RV32GP_Cop_Struct
{
    rme_ptr_t FCSR;
    rme_ptr_t F0;
    rme_ptr_t F1;
    rme_ptr_t F2;
    rme_ptr_t F3;
    rme_ptr_t F4;
    rme_ptr_t F5;
    rme_ptr_t F6;
    rme_ptr_t F7;
    rme_ptr_t F8;
    rme_ptr_t F9;
    rme_ptr_t F10;
    rme_ptr_t F11;
    rme_ptr_t F12;
    rme_ptr_t F13;
    rme_ptr_t F14;
    rme_ptr_t F15;
    rme_ptr_t F16;
    rme_ptr_t F17;
    rme_ptr_t F18;
    rme_ptr_t F19;
    rme_ptr_t F20;
    rme_ptr_t F21;
    rme_ptr_t F22;
    rme_ptr_t F23;
    rme_ptr_t F24;
    rme_ptr_t F25;
    rme_ptr_t F26;
    rme_ptr_t F27;
    rme_ptr_t F28;
    rme_ptr_t F29;
    rme_ptr_t F30;
    rme_ptr_t F31;
};
#else
/* Double-precision register set */
struct RME_RV32GP_Cop_Struct
{
    rme_ptr_t FCSR;
    rme_ptr_t F0[2];
    rme_ptr_t F1[2];
    rme_ptr_t F2[2];
    rme_ptr_t F3[2];
    rme_ptr_t F4[2];
    rme_ptr_t F5[2];
    rme_ptr_t F6[2];
    rme_ptr_t F7[2];
    rme_ptr_t F8[2];
    rme_ptr_t F9[2];
    rme_ptr_t F10[2];
    rme_ptr_t F11[2];
    rme_ptr_t F12[2];
    rme_ptr_t F13[2];
    rme_ptr_t F14[2];
    rme_ptr_t F15[2];
    rme_ptr_t F16[2];
    rme_ptr_t F17[2];
    rme_ptr_t F18[2];
    rme_ptr_t F19[2];
    rme_ptr_t F20[2];
    rme_ptr_t F21[2];
    rme_ptr_t F22[2];
    rme_ptr_t F23[2];
    rme_ptr_t F24[2];
    rme_ptr_t F25[2];
    rme_ptr_t F26[2];
    rme_ptr_t F27[2];
    rme_ptr_t F28[2];
    rme_ptr_t F29[2];
    rme_ptr_t F30[2];
    rme_ptr_t F31[2];
};
#endif
#endif

/* Exception registere set */
struct RME_Exc_Struct
{
    rme_ptr_t Cause;
    rme_ptr_t Addr;
};

/* Invocation register set structure */
struct RME_Iret_Struct
{
    rme_ptr_t PC;
    rme_ptr_t X2_SP;
};

/* Page Table ****************************************************************/
/* For RV32GP:
 * The layout of the page entry is:
 * [31:2] Paddr - The physical address to map this page to, or the physical
 *                address of the next layer of page table. This address is
 *                always aligned to 32 bytes.
 * [1] Terminal - Is this page a terminal page, or points to another page table?
 * [0] Present - Is this entry present?
 *
 * The layout of a directory entry is:
 * [31:2] Paddr - The in-kernel physical address of the lower page directory.
 * [1] Terminal - Is this page a terminal page, or points to another page table?
 * [0] Present - Is this entry present?
 *
 * The Flag[] field decides the actual mapping of the pages and page directories.
 */
struct __RME_RV32GP_PMP_Data
{
    rme_ptr_t Static;
    rme_ptr_t Cfg[RME_RV32GP_PMPCFG_NUM];
    rme_ptr_t Addr[RME_RV32GP_REGION_NUM];
};

/* Page table metadata structure */
struct __RME_RV32GP_Pgt_Meta
{
    /* The start mapping address of this page table */
    rme_ptr_t Base;
    /* The size/num order of this level */
    rme_ptr_t Size_Num_Order;
};
/*****************************************************************************/
/* __RME_PLATFORM_A7M_H_STRUCTS__ */
#endif
/* __HDR_STRUCTS__ */
#endif
/* End Structs ***************************************************************/

/* Private Global Variables **************************************************/
#if(!(defined __HDR_DEFS__||defined __HDR_STRUCTS__))
#ifndef __RME_PLATFORM_A7M_MEMBERS__
#define __RME_PLATFORM_A7M_MEMBERS__

/* In this way we can use the data structures and definitions in the headers */
#define __HDR_DEFS__

#undef __HDR_DEFS__

#define __HDR_STRUCTS__

#undef __HDR_STRUCTS__

/* If the header is not used in the public mode */
#ifndef __HDR_PUBLIC_MEMBERS__
/*****************************************************************************/

/*****************************************************************************/
/* End Private Global Variables **********************************************/

/* Private C Function Prototypes *********************************************/
/* Generator *****************************************************************/
#if(RME_RVM_GEN_ENABLE==1U)
EXTERN rme_ptr_t RME_Boot_Vct_Handler(rme_ptr_t Vct_Num);
EXTERN rme_ptr_t RME_Boot_Vct_Init(struct RME_Cap_Cpt* Cpt,
                                   rme_ptr_t Cap_Front,
                                   rme_ptr_t Kom_Front);
EXTERN void RME_Boot_Pre_Init(void);
EXTERN void RME_Boot_Post_Init(void);
EXTERN void RME_Reboot_Failsafe(void);
#endif
/* PMP operations */
EXTERN void ___RME_RV32GP_PMP_Set1(rme_ptr_t* CFG_Meta, rme_ptr_t* ADDR_Meta);
EXTERN void ___RME_RV32GP_PMP_Set2(rme_ptr_t* CFG_Meta, rme_ptr_t* ADDR_Meta);
EXTERN void ___RME_RV32GP_PMP_Set3(rme_ptr_t* CFG_Meta, rme_ptr_t* ADDR_Meta);
EXTERN void ___RME_RV32GP_PMP_Set4(rme_ptr_t* CFG_Meta, rme_ptr_t* ADDR_Meta);
EXTERN void ___RME_RV32GP_PMP_Set5(rme_ptr_t* CFG_Meta, rme_ptr_t* ADDR_Meta);
EXTERN void ___RME_RV32GP_PMP_Set6(rme_ptr_t* CFG_Meta, rme_ptr_t* ADDR_Meta);
EXTERN void ___RME_RV32GP_PMP_Set7(rme_ptr_t* CFG_Meta, rme_ptr_t* ADDR_Meta);
EXTERN void ___RME_RV32GP_PMP_Set8(rme_ptr_t* CFG_Meta, rme_ptr_t* ADDR_Meta);
EXTERN void ___RME_RV32GP_PMP_Set9(rme_ptr_t* CFG_Meta, rme_ptr_t* ADDR_Meta);
EXTERN void ___RME_RV32GP_PMP_Set10(rme_ptr_t* CFG_Meta, rme_ptr_t* ADDR_Meta);
EXTERN void ___RME_RV32GP_PMP_Set11(rme_ptr_t* CFG_Meta, rme_ptr_t* ADDR_Meta);
EXTERN void ___RME_RV32GP_PMP_Set12(rme_ptr_t* CFG_Meta, rme_ptr_t* ADDR_Meta);
EXTERN void ___RME_RV32GP_PMP_Set13(rme_ptr_t* CFG_Meta, rme_ptr_t* ADDR_Meta);
EXTERN void ___RME_RV32GP_PMP_Set14(rme_ptr_t* CFG_Meta, rme_ptr_t* ADDR_Meta);
EXTERN void ___RME_RV32GP_PMP_Set15(rme_ptr_t* CFG_Meta, rme_ptr_t* ADDR_Meta);
EXTERN void ___RME_RV32GP_PMP_Set16(rme_ptr_t* CFG_Meta, rme_ptr_t* ADDR_Meta);
/* Handler *******************************************************************/
/* Fault handler */
static void __RME_RV32GP_Exc_Handler(struct RME_Reg_Struct* Reg,
                                     rme_ptr_t Mcause);
/* Vector Flags **************************************************************/
static void __RME_RV32GP_Flag_Fast(rme_ptr_t Base,
                                   rme_ptr_t Size,
                                   rme_ptr_t Flag);
static void __RME_RV32GP_Flag_Slow(rme_ptr_t Base,
                                   rme_ptr_t Size,
                                   rme_ptr_t Pos);
/* Page Table ****************************************************************/
static rme_ptr_t __RME_RV32GP_Rand(void);
/* Kernel function ***********************************************************/
static rme_ret_t __RME_RV32GP_Pgt_Entry_Mod(struct RME_Cap_Cpt* Cpt,
                                         rme_cid_t Cap_Pgt,
                                         rme_ptr_t Vaddr,
                                         rme_ptr_t Type);
static rme_ret_t __RME_RV32GP_Int_Local_Mod(rme_ptr_t Int_Num,
                                         rme_ptr_t Operation,
                                         rme_ptr_t Param);
static rme_ret_t __RME_RV32GP_Int_Local_Trig(rme_ptr_t CPUID,
                                          rme_ptr_t Int_Num);
static rme_ret_t __RME_RV32GP_Evt_Local_Trig(struct RME_Reg_Struct* Reg,
                                             rme_ptr_t CPUID,
                                             rme_ptr_t Evt_Num);

static rme_ret_t __RME_RV32GP_Cache_Maint(rme_ptr_t Cache_ID,
                                          rme_ptr_t Operation,
                                          rme_ptr_t Param);
static rme_ret_t __RME_RV32GP_Prfth_Mod(rme_ptr_t Prfth_ID,
                                        rme_ptr_t Operation,
                                        rme_ptr_t Param);
static rme_ret_t __RME_RV32GP_Perf_CPU_Func(struct RME_Reg_Struct* Reg,
                                            rme_ptr_t Freg_ID);
static rme_ret_t __RME_RV32GP_Perf_Mon_Mod(rme_ptr_t Perf_ID,
                                           rme_ptr_t Operation,
                                           rme_ptr_t Param);
static rme_ret_t __RME_RV32GP_Perf_Cycle_Mod(struct RME_Reg_Struct* Reg,
                                             rme_ptr_t Cycle_ID,
                                             rme_ptr_t Operation,
                                             rme_ptr_t Value);
static rme_ret_t __RME_RV32GP_Debug_Reg_Mod(struct RME_Cap_Cpt* Cpt,
                                            struct RME_Reg_Struct* Reg,
                                            rme_cid_t Cap_Thd,
                                            rme_ptr_t Operation,
                                            rme_ptr_t Value);
static rme_ret_t __RME_RV32GP_Debug_Inv_Mod(struct RME_Cap_Cpt* Cpt,
                                            struct RME_Reg_Struct* Reg,
                                            rme_cid_t Cap_Thd,
                                            rme_ptr_t Operation,
                                            rme_ptr_t Value);
static rme_ret_t __RME_RV32GP_Debug_Exc_Get(struct RME_Cap_Cpt* Cpt,
                                            struct RME_Reg_Struct* Reg,
                                            rme_cid_t Cap_Thd,
                                            rme_ptr_t Operation);
/*****************************************************************************/
#define __EXTERN__
/* End Private C Function Prototypes *****************************************/

/* Public Global Variables ***************************************************/
/* __HDR_PUBLIC_MEMBERS__ */
#else
#define __EXTERN__ EXTERN 
/* __HDR_PUBLIC_MEMBERS__ */
#endif

/*****************************************************************************/
/* Timestamp counter */
__EXTERN__ rme_ptr_t RME_RV32GP_Timestamp;
/* RV32GP only supports one core, thus this is its CPU-local data structure */
__EXTERN__ struct RME_CPU_Local RME_RV32GP_Local;
/* RV32GP use simple kernel object table */
__EXTERN__ rme_ptr_t RME_RV32GP_Kot[RME_KOT_WORD_NUM];
/*****************************************************************************/

/* End Public Global Variables ***********************************************/

/* Public C Function Prototypes **********************************************/
/* Generic *******************************************************************/
/* Interrupts */
EXTERN void __RME_Int_Disable(void);
EXTERN void __RME_Int_Enable(void);
EXTERN void __RME_RV32GP_Barrier(void);
EXTERN void __RME_RV32GP_Wait_Int(void);
/* CSR manipulation */
EXTERN rme_ptr_t __RME_RV32GP_MCAUSE_Get(void);
EXTERN void __RME_RV32GP_MTVEC_Set(rme_ptr_t Value);
EXTERN rme_ptr_t __RME_RV32GP_MCYCLE_Get(void);
EXTERN rme_ptr_t __RME_RV32GP_MISA_Get(void);
EXTERN rme_ptr_t __RME_RV32GP_MSTATUS_Get(void);
EXTERN void __RME_RV32GP_MSTATUS_Set(rme_ptr_t Value);
/* MSB counting */
EXTERN rme_ptr_t __RME_RV32GP_MSB_Get(rme_ptr_t Val);
/* Atomics */
__EXTERN__ rme_ptr_t __RME_RV32GP_Comp_Swap(volatile rme_ptr_t* Ptr,
                                            rme_ptr_t Old,
                                            rme_ptr_t New);
__EXTERN__ rme_ptr_t __RME_RV32GP_Fetch_Add(volatile rme_ptr_t* Ptr,
                                            rme_cnt_t Addend);
__EXTERN__ rme_ptr_t __RME_RV32GP_Fetch_And(volatile rme_ptr_t* Ptr,
                                            rme_ptr_t Operand);
#if(RME_DEBUG_PRINT==1U)
/* Debugging */
__EXTERN__ rme_ptr_t __RME_Putchar(char Char);
#endif
/* Getting CPUID */
__EXTERN__ rme_ptr_t __RME_CPUID_Get(void);

/* Kernel function handler */
__EXTERN__ rme_ret_t __RME_Kfn_Handler(struct RME_Cap_Cpt* Cpt,
                                       struct RME_Reg_Struct* Reg,
                                       rme_ptr_t Func_ID,
                                       rme_ptr_t Sub_ID,
                                       rme_ptr_t Param1,
                                       rme_ptr_t Param2);

/* Initialization ************************************************************/
__EXTERN__ void __RME_RV32GP_Lowlvl_Preinit(void);
__EXTERN__ void __RME_Lowlvl_Init(void);
__EXTERN__ void __RME_Boot(void);

__EXTERN__ void __RME_RV32GP_Reboot(void);
__EXTERN__ void __RME_RV32GP_NVIC_Set_Exc_Prio(rme_cnt_t Exc,
                                            rme_ptr_t Prio);
__EXTERN__ void __RME_RV32GP_Cache_Init(void);
EXTERN void __RME_User_Enter(rme_ptr_t Entry,
                             rme_ptr_t Stack,
                             rme_ptr_t CPUID);

/* Register Manipulation *****************************************************/
/* Coprocessor */
EXTERN void ___RME_RV32GP_Thd_Cop_Clear_RVF(void);
EXTERN void ___RME_RV32GP_Thd_Cop_Clear_RVD(void);
EXTERN void ___RME_RV32GP_Thd_Cop_Save_RVF(struct RME_RV32GP_Cop_Struct* Cop);
EXTERN void ___RME_RV32GP_Thd_Cop_Save_RVD(struct RME_RV32GP_Cop_Struct* Cop);
EXTERN void ___RME_RV32GP_Thd_Cop_Load_RVF(struct RME_RV32GP_Cop_Struct* Cop);
EXTERN void ___RME_RV32GP_Thd_Cop_Load_RVD(struct RME_RV32GP_Cop_Struct* Cop);
/* Syscall parameter */
__EXTERN__ void __RME_Svc_Param_Get(struct RME_Reg_Struct* Reg,
                                    rme_ptr_t* Svc,
                                    rme_ptr_t* Capid,
                                    rme_ptr_t* Param);
__EXTERN__ void __RME_Svc_Retval_Set(struct RME_Reg_Struct* Reg,
                                     rme_ret_t Retval);
/* Thread register sets */
__EXTERN__ void __RME_Thd_Reg_Init(rme_ptr_t Attr,
                                   rme_ptr_t Entry,
                                   rme_ptr_t Stack,
                                   rme_ptr_t Param,
                                   struct RME_Reg_Struct* Reg);
__EXTERN__ void __RME_Thd_Reg_Copy(struct RME_Reg_Struct* Dst,
                                   struct RME_Reg_Struct* Src);
/* Invocation register sets */
__EXTERN__ void __RME_Inv_Reg_Save(struct RME_Iret_Struct* Ret,
                                   struct RME_Reg_Struct* Reg);
__EXTERN__ void __RME_Inv_Reg_Restore(struct RME_Reg_Struct* Reg,
                                      struct RME_Iret_Struct* Ret);
__EXTERN__ void __RME_Inv_Retval_Set(struct RME_Reg_Struct* Reg,
                                     rme_ret_t Retval);
/* Coprocessor register sets */
__EXTERN__ rme_ret_t __RME_Thd_Cop_Check(rme_ptr_t Attr);
__EXTERN__ rme_ptr_t __RME_Thd_Cop_Size(rme_ptr_t Attr);
__EXTERN__ void __RME_Thd_Cop_Init(rme_ptr_t Attr,
                                   struct RME_Reg_Struct* Reg,
                                   void* Cop);
__EXTERN__ void __RME_Thd_Cop_Swap(rme_ptr_t Attr_New,
                                   struct RME_Reg_Struct* Reg_New,
                                   void* Cop_New,
                                   rme_ptr_t Attr_Cur,
                                   struct RME_Reg_Struct* Reg_Cur,
                                   void* Cop_Cur);

/* Page Table ****************************************************************/
/* Initialization */
__EXTERN__ rme_ret_t __RME_Pgt_Kom_Init(void);
__EXTERN__ rme_ret_t __RME_Pgt_Init(struct RME_Cap_Pgt* Pgt_Op);
/* Checking */
__EXTERN__ rme_ret_t __RME_Pgt_Check(rme_ptr_t Base_Addr,
                                     rme_ptr_t Is_Top,
                                     rme_ptr_t Size_Order,
                                     rme_ptr_t Num_Order,
                                     rme_ptr_t Vaddr);
__EXTERN__ rme_ret_t __RME_Pgt_Del_Check(struct RME_Cap_Pgt* Pgt_Op);
/* Setting the page table */
__EXTERN__ void __RME_Pgt_Set(struct RME_Cap_Pgt* Pgt);
/* Table operations */
__EXTERN__ rme_ret_t __RME_Pgt_Page_Map(struct RME_Cap_Pgt* Pgt_Op,
                                        rme_ptr_t Paddr,
                                        rme_ptr_t Pos,
                                        rme_ptr_t Flag);
__EXTERN__ rme_ret_t __RME_Pgt_Page_Unmap(struct RME_Cap_Pgt* Pgt_Op,
                                          rme_ptr_t Pos);
__EXTERN__ rme_ret_t __RME_Pgt_Pgdir_Map(struct RME_Cap_Pgt* Pgt_Parent,
                                         rme_ptr_t Pos,
                                         struct RME_Cap_Pgt* Pgt_Child,
                                         rme_ptr_t Flag);
__EXTERN__ rme_ret_t __RME_Pgt_Pgdir_Unmap(struct RME_Cap_Pgt* Pgt_Parent,
                                           rme_ptr_t Pos,
                                           struct RME_Cap_Pgt* Pgt_Child);
/* Lookup and walking */
__EXTERN__ rme_ret_t __RME_Pgt_Lookup(struct RME_Cap_Pgt* Pgt_Op,
                                      rme_ptr_t Pos,
                                      rme_ptr_t* Paddr,
                                      rme_ptr_t* Flag);
__EXTERN__ rme_ret_t __RME_Pgt_Walk(struct RME_Cap_Pgt* Pgt_Op,
                                    rme_ptr_t Vaddr,
                                    rme_ptr_t* Pgt,
                                    rme_ptr_t* Map_Vaddr,
                                    rme_ptr_t* Paddr,
                                    rme_ptr_t* Size_Order,
                                    rme_ptr_t* Num_Order,
                                    rme_ptr_t* Flag);
/*****************************************************************************/
/* Undefine "__EXTERN__" to avoid redefinition */
#undef __EXTERN__
/* __RME_PLATFORM_A7M_MEMBERS__ */
#endif
/* !(defined __HDR_DEFS__||defined __HDR_STRUCTS__) */
#endif
/* End Public C Function Prototypes ******************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
