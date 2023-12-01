/******************************************************************************
Filename    : rme_platform_a6m.h
Author      : pry
Date        : 01/04/2017
Licence     : The Unlicense; see LICENSE for details.
Description : The header of "rme_platform_a6m.c".
******************************************************************************/

/* Define ********************************************************************/
#ifdef __HDR_DEF__
#ifndef __RME_PLATFORM_A6M_DEF__
#define __RME_PLATFORM_A6M_DEF__
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

/* Extended Types ************************************************************/
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

/* System macros *************************************************************/
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
#define RME_CPU_LOCAL()                 (&RME_A6M_Local)
/* The order of bits in one CPU machine word */
#define RME_WORD_ORDER                  (5U)
/* Forcing VA=PA in user memory segments */
#define RME_VA_EQU_PA                   (1U)
/* Quiescence timeslice value */
#define RME_QUIE_TIME                   (0U)
/* Read timestamp counter */
#define RME_TIMESTAMP                   (RME_A6M_Timestamp)
/* Cpt size limit - not restricted */
#define RME_CPT_ENTRY_MAX               (0U)
/* Normal page directory size calculation macro */
#define RME_PGT_SIZE_NOM(NUM_ORDER)     (RME_POW2(NUM_ORDER)*sizeof(rme_ptr_t)+sizeof(struct __RME_A6M_Pgt_Meta))
/* Top-level page directory size calculation macro */
#define RME_PGT_SIZE_TOP(NUM_ORDER)     (RME_PGT_SIZE_NOM(NUM_ORDER)+sizeof(struct __RME_A6M_MPU_Data))
/* The kernel object allocation table address - original */
#define RME_KOT_VA_BASE                 RME_A6M_Kot
/* Compare-and-Swap(CAS) */
#define RME_COMP_SWAP(PTR,OLD,NEW)      _RME_Comp_Swap_Single(PTR,OLD,NEW)
/* Fetch-and-Add(FAA) */
#define RME_FETCH_ADD(PTR,ADDEND)       _RME_Fetch_Add_Single(PTR,ADDEND)
/* Fetch-and-And(FAND) */
#define RME_FETCH_AND(PTR,OPERAND)      _RME_Fetch_And_Single(PTR,OPERAND)
/* Get most significant bit */
#define RME_MSB_GET(VAL)                _RME_MSB_Generic(VAL)
/* No read/write barriers needed on Cortex-M, because they are currently all
 * single core. If this changes in the future, we may need DMB barriers. */
#define RME_READ_ACQUIRE(X)             (*(X))
#define RME_WRITE_RELEASE(X, V)         ((*(X))=(V))
/* Reboot the processor if the assert fails in this port */
#define RME_ASSERT_FAILED(F, L, D, T)   __RME_A6M_Reboot()

/* The CPU and application specific macros are here */
#include "rme_platform_a6m_conf.h"

/* Vector/signal flag */
#define RME_RVM_VCT_SIG_ALL             (0U)
#define RME_RVM_VCT_SIG_SELF            (1U)
#define RME_RVM_VCT_SIG_INIT            (2U)
#define RME_RVM_VCT_SIG_NONE            (3U)
#define RME_RVM_FLAG_SET(B, S, N)       ((volatile struct __RME_RVM_Flag*)((B)+((S)>>1)*(N)))
/* End System macros *********************************************************/

/* ARMv6-M specific macros ***************************************************/
/* Registers *****************************************************************/
#define RME_A6M_REG(X)                  (*((volatile rme_ptr_t*)(X)))
#define RME_A6M_REGB(X)                 (*((volatile rme_u8_t*)(X)))

#define RME_A6M_ITM_TER                 RME_A6M_REG(0xE0000E00U)
#define RME_A6M_ITM_PORT(X)             RME_A6M_REG(0xE0000000+((X)<<2))

#define RME_A6M_ITM_TCR                 RME_A6M_REG(0xE0000E80U)
#define RME_A6M_ITM_TCR_ITMENA          (1U<<0)

#define RME_A6M_DWT_CTRL                RME_A6M_REG(0xE0001000U)
#define RME_A6M_DWT_CTRL_NOCYCCNT       (1U<<25)
#define RME_A6M_DWT_CTRL_CYCCNTENA      (1U<<0)

#define RME_A6M_DWT_CYCCNT              RME_A6M_REG(0xE0001004U)

#define RME_A6M_SCNSCB_ICTR             RME_A6M_REG(0xE000E004U)

#define RME_A6M_SCNSCB_ACTLR            RME_A6M_REG(0xE000E008U)
#define RME_A6M_SCNSCB_ACTLR_DISBTAC    (1U<<13)

#define RME_A6M_SYSTICK_CTRL            RME_A6M_REG(0xE000E010U)
#define RME_A6M_SYSTICK_CTRL_CLKSOURCE  (1U<<2)
#define RME_A6M_SYSTICK_CTRL_TICKINT    (1U<<1)
#define RME_A6M_SYSTICK_CTRL_ENABLE     (1U<<0)

#define RME_A6M_SYSTICK_LOAD            RME_A6M_REG(0xE000E014U)
#define RME_A6M_SYSTICK_VALREG          RME_A6M_REG(0xE000E018U)
#define RME_A6M_SYSTICK_CALIB           RME_A6M_REG(0xE000E01CU)

#define RME_A6M_NVIC_ISER(X)            RME_A6M_REG(0xE000E100U+(((X)>>5)<<2))
#define RME_A6M_NVIC_ICER(X)            RME_A6M_REG(0xE000E180U+(((X)>>5)<<2))
#define RME_A6M_NVIC_ISPR               RME_A6M_REG(0xE000E200U)

/* ARMv6-M does not allow IPR byte access, this is different from ARMv7-M */
#define RME_A6M_NVIC_IPR(X)             RME_A6M_REG(0xE000E400U+(((X)>>2)<<2))
#define RME_A6M_NVIC_IPR_BIT(X)         (((X)&0x3U)<<3)
#define RME_A6M_NVIC_IPR_GET(X)         ((RME_A6M_NVIC_IPR(X)>>RME_A6M_NVIC_IPR_BIT(X))&0xFFU)
#define RME_A6M_NVIC_IPR_SET(X, V)      RME_A6M_NVIC_IPR(X)=RME_A6M_NVIC_IPR(X)& \
                                                            ~(0xFFU<<RME_A6M_NVIC_IPR_BIT(X))| \
                                                            ((V)<<RME_A6M_NVIC_IPR_BIT(X))

#define RME_A6M_IRQN_NONMASKABLEINT     (-14)
#define RME_A6M_IRQN_MEMORYMANAGEMENT   (-12)
#define RME_A6M_IRQN_BUSFAULT           (-11)
#define RME_A6M_IRQN_USAGEFAULT         (-10)
#define RME_A6M_IRQN_SVCALL             (-5)
#define RME_A6M_IRQN_DEBUGMONITOR       (-4)
#define RME_A6M_IRQN_PENDSV             (-2)
#define RME_A6M_IRQN_SYSTICK            (-1)

#define RME_A6M_SCB_CPUID               RME_A6M_REG(0xE000ED00U)
#define RME_A6M_SCB_ICSR                RME_A6M_REG(0xE000ED04U)
#define RME_A6M_SCB_VTOR                RME_A6M_REG(0xE000ED08U)
#define RME_A6M_SCB_AIRCR               RME_A6M_REG(0xE000ED0CU)

#define RME_A6M_SCB_SCR                 RME_A6M_REG(0xE000ED10U)
#define RME_A6M_SCB_SCR_SEVONPEND       (1U<<4)
#define RME_A6M_SCB_SCR_SLEEPDEEP       (1U<<2)
#define RME_A6M_SCB_SCR_SLEEPONEXIT     (1U<<1)

#define RME_A6M_SCB_CCR                 RME_A6M_REG(0xE000ED14U)

/* ARMv6-M does not allow IPR byte access, this is different from ARMv7-M,
 * and is ERRORNOUSLY documented in the ARMv6-M reference manual (manual 
 * says that it have no usage restrictions, which is absurd)! */
#define RME_A6M_SCB_SHPR(X)             RME_A6M_REG(0xE000ED18U+(((X)>>2)<<2))
#define RME_A6M_SCB_SHPR_BIT(X)         (((X)&0x3U)<<3)
#define RME_A6M_SCB_SHPR_GET(X)         ((RME_A6M_SCB_SHPR(X)>>RME_A6M_SCB_SHPR_BIT(X))&0xFFU)
#define RME_A6M_SCB_SHPR_SET(X, V)      RME_A6M_SCB_SHPR(X)=RME_A6M_SCB_SHPR(X)& \
                                                            ~(0xFFU<<RME_A6M_SCB_SHPR_BIT(X))| \
                                                            ((V)<<RME_A6M_SCB_SHPR_BIT(X))

#define RME_A6M_SCB_SHCSR               RME_A6M_REG(0xE000ED24U)
#define RME_A6M_SCB_SHCSR_SVCALLPENDED  (1U<<15)

#define RME_A6M_MPU_TYPE                RME_A6M_REG(0xE000ED90U)

#define RME_A6M_MPU_CTRL                RME_A6M_REG(0xE000ED94U)
#define RME_A6M_MPU_CTRL_PRIVDEF        (1U<<2)
#define RME_A6M_MPU_CTRL_ENABLE         (1U<<0)

#define RME_A6M_SCNSCB_PID4             RME_A6M_REG(0xE000EFD0U)
#define RME_A6M_SCNSCB_PID5             RME_A6M_REG(0xE000EFD4U)
#define RME_A6M_SCNSCB_PID6             RME_A6M_REG(0xE000EFD8U)
#define RME_A6M_SCNSCB_PID7             RME_A6M_REG(0xE000EFDCU)
#define RME_A6M_SCNSCB_PID0             RME_A6M_REG(0xE000EFE0U)
#define RME_A6M_SCNSCB_PID1             RME_A6M_REG(0xE000EFE4U)
#define RME_A6M_SCNSCB_PID2             RME_A6M_REG(0xE000EFE8U)
#define RME_A6M_SCNSCB_PID3             RME_A6M_REG(0xE000EFECU)
#define RME_A6M_SCNSCB_CID0             RME_A6M_REG(0xE000EFF0U)
#define RME_A6M_SCNSCB_CID1             RME_A6M_REG(0xE000EFF4U)
#define RME_A6M_SCNSCB_CID2             RME_A6M_REG(0xE000EFF8U)
#define RME_A6M_SCNSCB_CID3             RME_A6M_REG(0xE000EFFCU)

/* Generic *******************************************************************/
/* ARMv6-M EXC_RETURN bits */
#define RME_A6M_EXC_RET_INIT            (0xFFFFFFFDU)
#define RME_A6M_EXC_RET_FIX(X)          ((X)->LR=RME_A6M_EXC_RET_INIT)
/* Are we returning to user mode? 1 means yes, 0 means no */
#define RME_A6M_EXC_RET_RET_USER        (1U<<3)
/* Coprocessor type definitions */
#define RME_A6M_ATTR_NONE               (0U)
/* Handler *******************************************************************/
/* Fault definitions */
/* The NMI is active */
#define RME_A6M_ICSR_NMIPENDSET         RME_POW2(31U)

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
/* The initial default endpoint for all other vectors */
#define RME_BOOT_INIT_VCT               (6U)

/* Booting capability layout */
#define RME_A6M_CPT                     ((struct RME_Cap_Cpt*)(RME_KOM_VA_BASE))

/* Page Table ****************************************************************/
/* For ARMv6-M:
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
#define RME_A6M_PGT_META                (sizeof(struct __RME_A6M_Pgt_Meta)/sizeof(rme_ptr_t))
#define RME_A6M_MPU_DATA                (sizeof(struct __RME_A6M_MPU_Data)/sizeof(rme_ptr_t))
#define RME_A6M_PGT_TBL_NOM(X)          (((rme_ptr_t*)(X))+RME_A6M_PGT_META)
#define RME_A6M_PGT_TBL_TOP(X)          (((rme_ptr_t*)(X))+RME_A6M_PGT_META+RME_A6M_MPU_DATA)
/* Page entry bit definitions */
#define RME_A6M_PGT_PRESENT             (1U<<0)
#define RME_A6M_PGT_TERMINAL            (1U<<1)
/* The address mask for the actual page address */
#define RME_A6M_PGT_PTE_ADDR(X)         ((X)&0xFFFFFFFCU)
/* The address mask for the next level page table address */
#define RME_A6M_PGT_PGD_ADDR(X)         ((X)&0xFFFFFFFCU)
/* Page table metadata definitions */
#define RME_A6M_PGT_START(X)            ((X)&0xFFFFFFFEU)
#define RME_A6M_PGT_SIZEORD(X)          ((X)>>16)
#define RME_A6M_PGT_NUMORD(X)           ((X)&0x0000FFFFU)
/* MPU operation flag */
#define RME_A6M_MPU_CLR                 (0U)
#define RME_A6M_MPU_UPD                 (1U)
/* MPU definitions */
/* Extract address for/from MPU */
#define RME_A6M_MPU_ADDR(X)             ((X)&0xFFFFFFE0U)
/* Get info from MPU - X is the region's size order, not a subregion */
#define RME_A6M_MPU_SZORD(X)            ((((X)&0x3FU)>>1)+1U)
/* Write info to MPU - X is the region's size order, not a subregion */
#define RME_A6M_MPU_VALID               (1U<<4)
#define RME_A6M_MPU_SRDCLR              (0x0000FF00U)
#define RME_A6M_MPU_XN                  (1U<<28)
#define RME_A6M_MPU_RO                  (2U<<24)
#define RME_A6M_MPU_RW                  (3U<<24)
#define RME_A6M_MPU_CACHE               (1U<<17)
#define RME_A6M_MPU_BUFFER              (1U<<16)
#define RME_A6M_MPU_REGIONSIZE(X)       ((X-1U)<<1)
#define RME_A6M_MPU_SZENABLE            (1U)

/* Platform-specific kernel function macros **********************************/
/* Page table entry mode which property to get */
#define RME_A6M_KFN_PGT_ENTRY_MOD_FLAG_GET          (0U)
#define RME_A6M_KFN_PGT_ENTRY_MOD_SZORD_GET         (1U)
#define RME_A6M_KFN_PGT_ENTRY_MOD_NUMORD_GET        (2U)
/* Interrupt source configuration */
#define RME_A6M_KFN_INT_LOCAL_MOD_STATE_GET         (0U)
#define RME_A6M_KFN_INT_LOCAL_MOD_STATE_SET         (1U)
#define RME_A6M_KFN_INT_LOCAL_MOD_PRIO_GET          (2U)
#define RME_A6M_KFN_INT_LOCAL_MOD_PRIO_SET          (3U)
/* Prefetcher modification */
#define RME_A6M_KFN_PRFTH_MOD_STATE_GET             (0U)
#define RME_A6M_KFN_PRFTH_MOD_STATE_SET             (1U)
/* Prefetcher state */
#define RME_A6M_KFN_PRFTH_STATE_DISABLE             (0U)
#define RME_A6M_KFN_PRFTH_STATE_ENABLE              (1U)
/* CPU feature support */
#define RME_A6M_KFN_CPU_FUNC_CPUID                  (0U)
#define RME_A6M_KFN_CPU_FUNC_MPU_TYPE               (19U)
#define RME_A6M_KFN_CPU_FUNC_PID0                   (23U)
#define RME_A6M_KFN_CPU_FUNC_PID1                   (24U)
#define RME_A6M_KFN_CPU_FUNC_PID2                   (25U)
#define RME_A6M_KFN_CPU_FUNC_PID3                   (26U)
#define RME_A6M_KFN_CPU_FUNC_PID4                   (27U)
#define RME_A6M_KFN_CPU_FUNC_PID5                   (28U)
#define RME_A6M_KFN_CPU_FUNC_PID6                   (29U)
#define RME_A6M_KFN_CPU_FUNC_PID7                   (30U)
#define RME_A6M_KFN_CPU_FUNC_CID0                   (31U)
#define RME_A6M_KFN_CPU_FUNC_CID1                   (32U)
#define RME_A6M_KFN_CPU_FUNC_CID2                   (33U)
#define RME_A6M_KFN_CPU_FUNC_CID3                   (34U)
/* Perfomance counters */
#define RME_A6M_KFN_PERF_CYCLE_CYCCNT               (0U)
/* Performance counter state operations */
#define RME_A6M_KFN_PERF_STATE_GET                  (0U)
#define RME_A6M_KFN_PERF_STATE_SET                  (1U)
/* Performance counter states */
#define RME_A6M_KFN_PERF_STATE_DISABLE              (0U)
#define RME_A6M_KFN_PERF_STATE_ENABLE               (1U)
/* Performance counter value operations */
#define RME_A6M_KFN_PERF_VAL_GET                    (0U)
#define RME_A6M_KFN_PERF_VAL_SET                    (1U)
/* Register read/write */
#define RME_A6M_KFN_DEBUG_REG_MOD_GET               (0U)
#define RME_A6M_KFN_DEBUG_REG_MOD_SET               (1U<<16)
/* General-purpose registers */
#define RME_A6M_KFN_DEBUG_REG_MOD_SP                (0U)
#define RME_A6M_KFN_DEBUG_REG_MOD_R8                (1U)
#define RME_A6M_KFN_DEBUG_REG_MOD_R9                (2U)
#define RME_A6M_KFN_DEBUG_REG_MOD_R10               (3U)
#define RME_A6M_KFN_DEBUG_REG_MOD_R11               (4U)
#define RME_A6M_KFN_DEBUG_REG_MOD_R4                (5U)
#define RME_A6M_KFN_DEBUG_REG_MOD_R5                (6U)
#define RME_A6M_KFN_DEBUG_REG_MOD_R6                (7U)
#define RME_A6M_KFN_DEBUG_REG_MOD_R7                (8U)
#define RME_A6M_KFN_DEBUG_REG_MOD_LR                (9U)
/* Invocation register read/write */
#define RME_A6M_KFN_DEBUG_INV_MOD_SP_GET            (0U)
#define RME_A6M_KFN_DEBUG_INV_MOD_SP_SET            (1U)
/* Exception register read */
#define RME_A6M_KFN_DEBUG_EXC_CAUSE_GET             (0U)
/*****************************************************************************/
/* __RME_PLATFORM_A6M_DEF__ */
#endif
/* __HDR_DEF__ */
#endif
/* End Define ****************************************************************/

/* Struct ********************************************************************/
#ifdef __HDR_STRUCT__
#ifndef __RME_PLATFORM_A6M_STRUCT__
#define __RME_PLATFORM_A6M_STRUCT__
/* We used structs in the header */

/* Use defines in these headers */
#define __HDR_DEF__
#undef __HDR_DEF__
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
/* The register set struct - R0-R3, R12, PC, LR, xPSR is automatically pushed.
 * Here we need LR to decide EXC_RETURN, that's why it is here. ARMv6-M must
 * separate pushes to two efforts, that's why its layout is a bit different from
 * ARMv7-M's. */
struct RME_Reg_Struct
{
    rme_ptr_t SP;
    /* Second group pushed to stack */
    rme_ptr_t R8;
    rme_ptr_t R9;
    rme_ptr_t R10;
    rme_ptr_t R11;
    /* First group pushed to stack */
    rme_ptr_t R4;
    rme_ptr_t R5;
    rme_ptr_t R6;
    rme_ptr_t R7;
    rme_ptr_t LR;
};

/* Exception register structure - always indicate hardfault */
struct RME_Exc_Struct
{
    rme_ptr_t Cause;
};

/* Invocation register set structure */
struct RME_Iret_Struct
{
    rme_ptr_t SP;
};

/* Page Table ****************************************************************/
/* MPU entry structure */
struct __RME_A6M_MPU_Entry
{
    rme_ptr_t MPU_RBAR;
    rme_ptr_t MPU_RASR;
};
/* Page table metadata structure */
struct __RME_A6M_Pgt_Meta
{
    /* The MPU setting is always in the top level. This is a pointer to the top level */
    rme_ptr_t Toplevel;
    /* The start mapping address of this page table */
    rme_ptr_t Base;
    /* The size/num order of this level */
    rme_ptr_t Size_Num_Order;
    /* The page flags at this level. If any pages are mapped in, it must conform
     * to the same attributes as the older pages */
    rme_ptr_t Page_Flag;
};

/* MPU metadata structure */
struct __RME_A6M_MPU_Data
{
    /* There are no static flags for ARMv6-M */
    
    /* The MPU data itself. For ARMv6-M, the number of regions shall not exceed 16 */
    struct __RME_A6M_MPU_Entry Data[RME_A6M_REGION_NUM];
};
/*****************************************************************************/
/* __RME_PLATFORM_A6M_STRUCT__ */
#endif
/* __HDR_STRUCT__ */
#endif
/* End Struct ****************************************************************/

/* Private Variable **********************************************************/
#if(!(defined __HDR_DEF__||defined __HDR_STRUCT__))
#ifndef __RME_PLATFORM_A6M_MEMBER__
#define __RME_PLATFORM_A6M_MEMBER__

/* In this way we can use the data structures and definitions in the headers */
#define __HDR_DEF__

#undef __HDR_DEF__

#define __HDR_STRUCT__

#undef __HDR_STRUCT__

/* If the header is not used in the public mode */
#ifndef __HDR_PUBLIC__
/*****************************************************************************/

/*****************************************************************************/
/* End Private Variable ******************************************************/

/* Private Function **********************************************************/
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
/* MPU operations ***********************************************************/
EXTERN void ___RME_A6M_MPU_Set1(rme_ptr_t* MPU_Meta);
EXTERN void ___RME_A6M_MPU_Set2(rme_ptr_t* MPU_Meta);
EXTERN void ___RME_A6M_MPU_Set3(rme_ptr_t* MPU_Meta);
EXTERN void ___RME_A6M_MPU_Set4(rme_ptr_t* MPU_Meta);
EXTERN void ___RME_A6M_MPU_Set5(rme_ptr_t* MPU_Meta);
EXTERN void ___RME_A6M_MPU_Set6(rme_ptr_t* MPU_Meta);
EXTERN void ___RME_A6M_MPU_Set7(rme_ptr_t* MPU_Meta);
EXTERN void ___RME_A6M_MPU_Set8(rme_ptr_t* MPU_Meta);
EXTERN void ___RME_A6M_MPU_Set9(rme_ptr_t* MPU_Meta);
EXTERN void ___RME_A6M_MPU_Set10(rme_ptr_t* MPU_Meta);
EXTERN void ___RME_A6M_MPU_Set11(rme_ptr_t* MPU_Meta);
EXTERN void ___RME_A6M_MPU_Set12(rme_ptr_t* MPU_Meta);
EXTERN void ___RME_A6M_MPU_Set13(rme_ptr_t* MPU_Meta);
EXTERN void ___RME_A6M_MPU_Set14(rme_ptr_t* MPU_Meta);
EXTERN void ___RME_A6M_MPU_Set15(rme_ptr_t* MPU_Meta);
EXTERN void ___RME_A6M_MPU_Set16(rme_ptr_t* MPU_Meta);
/* Vector Flags **************************************************************/
static void __RME_A6M_Flag_Fast(rme_ptr_t Base,
                                rme_ptr_t Size,
                                rme_ptr_t Flag);
static void __RME_A6M_Flag_Slow(rme_ptr_t Base,
                                rme_ptr_t Size,
                                rme_ptr_t Pos);
/* Page Table ****************************************************************/
static rme_ptr_t ___RME_A6M_MPU_RASR_Gen(rme_ptr_t* Table,
                                         rme_ptr_t Flag, 
                                         rme_ptr_t Size_Order,
                                         rme_ptr_t Num_Order);
static rme_ret_t ___RME_A6M_MPU_Clear(struct __RME_A6M_MPU_Data* Top_MPU, 
                                      rme_ptr_t Base_Addr,
                                      rme_ptr_t Size_Order,
                                      rme_ptr_t Num_Order);
static rme_ret_t ___RME_A6M_MPU_Add(struct __RME_A6M_MPU_Data* Top_MPU, 
                                    rme_ptr_t Base_Addr,
                                    rme_ptr_t Size_Order,
                                    rme_ptr_t Num_Order,
                                    rme_ptr_t MPU_RASR);
static rme_ret_t ___RME_A6M_MPU_Update(struct __RME_A6M_Pgt_Meta* Meta,
                                       rme_ptr_t Op_Flag);
static rme_ptr_t ___RME_A6M_Pgt_Have_Page(rme_ptr_t* Table,
                                          rme_ptr_t Num_Order);
static rme_ptr_t ___RME_A6M_Pgt_Have_Pgdir(rme_ptr_t* Table,
                                           rme_ptr_t Num_Order);
static void ___RME_A6M_Pgt_Refresh(void);
/* Kernel function ***********************************************************/
static rme_ret_t __RME_A6M_Pgt_Entry_Mod(struct RME_Cap_Cpt* Cpt, 
                                         rme_cid_t Cap_Pgt,
                                         rme_ptr_t Vaddr,
                                         rme_ptr_t Type);
static rme_ret_t __RME_A6M_Int_Local_Mod(rme_ptr_t Int_Num,
                                         rme_ptr_t Operation,
                                         rme_ptr_t Param);
static rme_ret_t __RME_A6M_Int_Local_Trig(rme_ptr_t CPUID,
                                          rme_ptr_t Int_Num);
static rme_ret_t __RME_A6M_Evt_Local_Trig(struct RME_Reg_Struct* Reg,
                                          rme_ptr_t CPUID,
                                          rme_ptr_t Evt_Num);
static rme_ret_t __RME_A6M_Prfth_Mod(rme_ptr_t Prfth_ID,
                                     rme_ptr_t Operation,
                                     rme_ptr_t Param);
static rme_ret_t __RME_A6M_Perf_CPU_Func(struct RME_Reg_Struct* Reg,
                                         rme_ptr_t Freg_ID);
static rme_ret_t __RME_A6M_Perf_Mon_Mod(rme_ptr_t Perf_ID,
                                        rme_ptr_t Operation,
                                        rme_ptr_t Param);
static rme_ret_t __RME_A6M_Perf_Cycle_Mod(struct RME_Reg_Struct* Reg,
                                          rme_ptr_t Cycle_ID, 
                                          rme_ptr_t Operation,
                                          rme_ptr_t Value);
static rme_ret_t __RME_A6M_Debug_Reg_Mod(struct RME_Cap_Cpt* Cpt,
                                         struct RME_Reg_Struct* Reg, 
                                         rme_cid_t Cap_Thd,
                                         rme_ptr_t Operation,
                                         rme_ptr_t Value);
static rme_ret_t __RME_A6M_Debug_Inv_Mod(struct RME_Cap_Cpt* Cpt,
                                         struct RME_Reg_Struct* Reg, 
                                         rme_cid_t Cap_Thd,
                                         rme_ptr_t Operation,
                                         rme_ptr_t Value);
static rme_ret_t __RME_A6M_Debug_Exc_Get(struct RME_Cap_Cpt* Cpt,
                                         struct RME_Reg_Struct* Reg, 
                                         rme_cid_t Cap_Thd,
                                         rme_ptr_t Operation);
/*****************************************************************************/
#define __EXTERN__
/* End Private Function ******************************************************/

/* Public Variable ***********************************************************/
/* __HDR_PUBLIC__ */
#else
#define __EXTERN__ EXTERN 
/* __HDR_PUBLIC__ */
#endif

/*****************************************************************************/
/* Timestamp counter */
__EXTERN__ rme_ptr_t RME_A6M_Timestamp;
/* ARMv6-M only have one core, thus this is its CPU-local data structure */
__EXTERN__ struct RME_CPU_Local RME_A6M_Local;
/* ARMv6-M use simple kernel object table */
__EXTERN__ rme_ptr_t RME_A6M_Kot[RME_KOT_WORD_NUM];
/*****************************************************************************/

/* End Public Variable *******************************************************/

/* Public Function ***********************************************************/
/* Generic *******************************************************************/
/* Interrupts */
EXTERN void __RME_Int_Disable(void);
EXTERN void __RME_Int_Enable(void);
EXTERN void __RME_A6M_Barrier(void);
EXTERN void __RME_A6M_Wait_Int(void);
#if(RME_DEBUG_PRINT==1U)
/* Debugging */
__EXTERN__ rme_ptr_t __RME_Putchar(char Char);
#endif
/* Getting CPUID */
__EXTERN__ rme_ptr_t __RME_CPUID_Get(void);

/* Handler *******************************************************************/
/* Exception handler */
__EXTERN__ void __RME_A6M_Exc_Handler(struct RME_Reg_Struct* Reg);
/* Generic interrupt handler */
__EXTERN__ void __RME_A6M_Vct_Handler(struct RME_Reg_Struct* Reg,
                                      rme_ptr_t Vct_Num);
/* Timer handler */
__EXTERN__ void __RME_A6M_Tim_Handler(struct RME_Reg_Struct* Reg);
/* System call handler */
__EXTERN__ void __RME_A6M_Svc_Handler(struct RME_Reg_Struct* Reg);

/* Kernel function handler */
__EXTERN__ rme_ret_t __RME_Kfn_Handler(struct RME_Cap_Cpt* Cpt,
                                       struct RME_Reg_Struct* Reg,
                                       rme_ptr_t Func_ID,
                                       rme_ptr_t Sub_ID,
                                       rme_ptr_t Param1,
                                       rme_ptr_t Param2);

/* Initialization ************************************************************/
__EXTERN__ void __RME_A6M_Lowlvl_Preinit(void);
__EXTERN__ void __RME_Lowlvl_Init(void);
__EXTERN__ void __RME_Boot(void);
EXTERN void __RME_A6M_Reset(void);
__EXTERN__ void __RME_A6M_Reboot(void);
__EXTERN__ void __RME_A6M_NVIC_Set_Exc_Prio(rme_cnt_t Exc,
                                            rme_ptr_t Prio);
EXTERN void __RME_User_Enter(rme_ptr_t Entry,
                             rme_ptr_t Stack,
                             rme_ptr_t CPUID);

/* Register Manipulation *****************************************************/
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
/* __RME_PLATFORM_A6M_MEMBER__ */
#endif
/* !(defined __HDR_DEF__||defined __HDR_STRUCT__) */
#endif
/* End Public Function *******************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
