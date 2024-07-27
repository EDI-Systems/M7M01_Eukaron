/******************************************************************************
Filename    : rme_platform_a7m.h
Author      : pry
Date        : 01/04/2017
Licence     : The Unlicense; see LICENSE for details.
Description : The header of "rme_platform_a7m.c".
******************************************************************************/

/* Define ********************************************************************/
#ifdef __HDR_DEF__
#ifndef __RME_PLATFORM_A7M_DEF__
#define __RME_PLATFORM_A7M_DEF__
/*****************************************************************************/
/* Basic Type ****************************************************************/
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
/* End Basic Type ************************************************************/

/* Extended Type *************************************************************/
#ifndef __RME_CID_T__
#define __RME_CID_T__
/* Capability ID */
typedef rme_s32_t rme_cid_t;
#endif

#ifndef __RME_TID_T__
#define __RME_TID_T__
/* Thread ID */
typedef rme_s32_t rme_tid_t;
#endif

#ifndef __RME_PTR_T__
#define __RME_PTR_T__
/* Pointer */
typedef rme_u32_t rme_ptr_t;
#endif

#ifndef __RME_CNT_T__
#define __RME_CNT_T__
/* Counter */
typedef rme_s32_t rme_cnt_t;
#endif

#ifndef __RME_RET_T__
#define __RME_RET_T__
/* Return value */
typedef rme_s32_t rme_ret_t;
#endif
/* End Extended Type *********************************************************/

/* System Macro **************************************************************/
/* Compiler "extern" keyword setting */
#define RME_EXTERN                              extern
/* Compiler "inline" keyword setting */
#define RME_INLINE                              inline
/* Compiler "likely" & "unlikely" keyword setting */
#ifdef likely
#define RME_LIKELY(X)                           (likely(X))
#else
#define RME_LIKELY(X)                           (X)
#endif
#ifdef unlikely
#define RME_UNLIKELY(X)                         (unlikely(X))
#else
#define RME_UNLIKELY(X)                         (X)
#endif
/* CPU-local data structure location macro */
#define RME_CPU_LOCAL()                         (&RME_A7M_Local)
/* The order of bits in one CPU machine word */
#define RME_WORD_ORDER                          (5U)
/* Quiescence timeslice value */
#define RME_QUIE_TIME                           (0U)
/* Read timestamp counter */
#define RME_TIMESTAMP                           (RME_A7M_Timestamp)
/* Cpt size limit - not restricted */
#define RME_CPT_ENTRY_MAX                       (0U)
/* Forcing VA=PA in user memory segments */
#define RME_PGT_PHYS_ENABLE                     (1U)
/* Normal page directory size calculation macro */
#define RME_PGT_SIZE_NOM(NMORD)                 (sizeof(struct __RME_A7M_Pgt_Meta)+RME_POW2(NMORD)*RME_WORD_BYTE)
/* Top-level page directory size calculation macro */
#define RME_PGT_SIZE_TOP(NMORD)                 (sizeof(struct __RME_A7M_MPU_Data)+RME_PGT_SIZE_NOM(NMORD))
/* The kernel object allocation table address - original */
#define RME_KOT_VA_BASE                         RME_A7M_Kot
/* Compare-and-Swap(CAS) */
#define RME_COMP_SWAP(PTR,OLD,NEW)              _RME_Comp_Swap_Single(PTR,OLD,NEW)
/* Fetch-and-Add(FAA) */
#define RME_FETCH_ADD(PTR,ADDEND)               _RME_Fetch_Add_Single(PTR,ADDEND)
/* Fetch-and-And(FAND) */
#define RME_FETCH_AND(PTR,OPERAND)              _RME_Fetch_And_Single(PTR,OPERAND)
/* Get most significant bit */
#define RME_MSB_GET(VAL)                        __RME_A7M_MSB_Get(VAL)
/* Single-core processor */
#define RME_READ_ACQUIRE(X)                     (*(X))
#define RME_WRITE_RELEASE(X,V)                  ((*(X))=(V))
/* Reboot the processor if the assert fails in this port */
#define RME_ASSERT_FAIL(F,L,D,T)                __RME_A7M_Reboot()

/* The CPU and application specific macros are here */
#include "rme_platform_a7m_conf.h"

/* Vector/signal flag */
#define RME_RVM_VCT_SIG_ALL                     (0U)
#define RME_RVM_VCT_SIG_SELF                    (1U)
#define RME_RVM_VCT_SIG_INIT                    (2U)
#define RME_RVM_VCT_SIG_NONE                    (3U)
#define RME_RVM_FLAG_SET(B,S,N)                 ((volatile struct __RME_RVM_Flag*)((B)+((S)>>1)*(N)))
/* End System Macro **********************************************************/

/* ARMv7-M Macro *************************************************************/
/* Generic *******************************************************************/
/* Register access */
#define RME_A7M_REG(X)                          (*((volatile rme_ptr_t*)(X)))
#define RME_A7M_REGB(X)                         (*((volatile rme_u8_t*)(X)))

/* ARMv7-M EXC_RETURN bits */
#define RME_A7M_EXC_RET_INIT                    (0xFFFFFFFDU)
#define RME_A7M_EXC_RET_MASK                    (0x00000010U)
#define RME_A7M_EXC_RET_KEEP                    (0xFFFFFFEDU)
#define RME_A7M_EXC_RET_FIX(X)                  (((X)->LR)=((X)->LR)&RME_A7M_EXC_RET_MASK|RME_A7M_EXC_RET_KEEP)
/* Whether the stack frame is standard(contains no FPU data). 1 means yes, 0 means no */
#define RME_A7M_EXC_RET_STD_FRAME               RME_POW2(4U)
/* Are we returning to user mode? 1 means yes, 0 means no */
#define RME_A7M_EXC_RET_RET_USER                RME_POW2(3U)

/* FPU control settings */
#define RME_A7M_CONTROL_FPCA                    RME_POW2(2U)
/* FPU CPACR settings */
#define RME_A7M_SCB_CPACR_FPU_MASK              (RME_FIELD(3U,10U*2U)|RME_FIELD(3U,11U*2U))

/* Thread context attribute definitions */
#define RME_A7M_ATTR_NONE                       (0U)
#define RME_A7M_ATTR_FPV4_SP                    RME_POW2(0U)
#define RME_A7M_ATTR_FPV5_SP                    RME_POW2(1U)
#define RME_A7M_ATTR_FPV5_DP                    RME_POW2(2U)

/* Register ******************************************************************/
#define RME_A7M_ITM_TER                         RME_A7M_REG(0xE0000E00U)
#define RME_A7M_ITM_PORT(X)                     RME_A7M_REG(0xE0000000+((X)<<2))

#define RME_A7M_ITM_TCR                         RME_A7M_REG(0xE0000E80U)
#define RME_A7M_ITM_TCR_ITMENA                  RME_POW2(0U)

#define RME_A7M_DWT_CTRL                        RME_A7M_REG(0xE0001000U)
#define RME_A7M_DWT_CTRL_NOCYCCNT               RME_POW2(25U)
#define RME_A7M_DWT_CTRL_CYCCNTENA              RME_POW2(0U)

#define RME_A7M_DWT_CYCCNT                      RME_A7M_REG(0xE0001004U)

#define RME_A7M_SCNSCB_ICTR                     RME_A7M_REG(0xE000E004U)

#define RME_A7M_SCNSCB_ACTLR                    RME_A7M_REG(0xE000E008U)
#define RME_A7M_SCNSCB_ACTLR_DISBTAC            RME_POW2(13U)

#define RME_A7M_SYSTICK_CTRL                    RME_A7M_REG(0xE000E010U)
#define RME_A7M_SYSTICK_CTRL_CLKSOURCE          RME_POW2(2U)
#define RME_A7M_SYSTICK_CTRL_TICKINT            RME_POW2(1U)
#define RME_A7M_SYSTICK_CTRL_ENABLE             RME_POW2(0U)

#define RME_A7M_SYSTICK_LOAD                    RME_A7M_REG(0xE000E014U)
#define RME_A7M_SYSTICK_VALREG                  RME_A7M_REG(0xE000E018U)
#define RME_A7M_SYSTICK_CALIB                   RME_A7M_REG(0xE000E01CU)

#define RME_A7M_NVIC_ISER(X)                    RME_A7M_REG(0xE000E100U+(((X)>>5)<<2))
#define RME_A7M_NVIC_ICER(X)                    RME_A7M_REG(0xE000E180U+(((X)>>5)<<2))

#define RME_A7M_NVIC_IPR(X)                     RME_A7M_REGB(0xE000E400U+(X))
#define RME_A7M_NVIC_GROUPING_P7S1              (0U)
#define RME_A7M_NVIC_GROUPING_P6S2              (1U)
#define RME_A7M_NVIC_GROUPING_P5S3              (2U)
#define RME_A7M_NVIC_GROUPING_P4S4              (3U)
#define RME_A7M_NVIC_GROUPING_P3S5              (4U)
#define RME_A7M_NVIC_GROUPING_P2S6              (5U)
#define RME_A7M_NVIC_GROUPING_P1S7              (6U)
#define RME_A7M_NVIC_GROUPING_P0S8              (7U)

#define RME_A7M_IRQN_NONMASKABLEINT             (-14)
#define RME_A7M_IRQN_MEMORYMANAGEMENT           (-12)
#define RME_A7M_IRQN_BUSFAULT                   (-11)
#define RME_A7M_IRQN_USAGEFAULT                 (-10)
#define RME_A7M_IRQN_SVCALL                     (-5)
#define RME_A7M_IRQN_DEBUGMONITOR               (-4)
#define RME_A7M_IRQN_PENDSV                     (-2)
#define RME_A7M_IRQN_SYSTICK                    (-1)

#define RME_A7M_SCB_CPUID                       RME_A7M_REG(0xE000ED00U)
#define RME_A7M_SCB_ICSR                        RME_A7M_REG(0xE000ED04U)
#define RME_A7M_SCB_VTOR                        RME_A7M_REG(0xE000ED08U)
#define RME_A7M_SCB_AIRCR                       RME_A7M_REG(0xE000ED0CU)

#define RME_A7M_SCB_SCR                         RME_A7M_REG(0xE000ED10U)
#define RME_A7M_SCB_SCR_SEVONPEND               RME_POW2(4U)
#define RME_A7M_SCB_SCR_SLEEPDEEP               RME_POW2(2U)
#define RME_A7M_SCB_SCR_SLEEPONEXIT             RME_POW2(1U)

#define RME_A7M_SCB_CCR                         RME_A7M_REG(0xE000ED14U)
#define RME_A7M_SCB_CCR_IC                      RME_POW2(17U)
#define RME_A7M_SCB_CCR_DC                      RME_POW2(16U)

#define RME_A7M_SCB_SHPR(X)                     RME_A7M_REGB(0xE000ED18U+(X))

#define RME_A7M_SCB_SHCSR                       RME_A7M_REG(0xE000ED24U)
#define RME_A7M_SCB_SHCSR_MEMFAULTENA           RME_POW2(16U)
#define RME_A7M_SCB_SHCSR_BUSFAULTENA           RME_POW2(17U)
#define RME_A7M_SCB_SHCSR_USGFAULTENA           RME_POW2(18U)

#define RME_A7M_SCB_CFSR                        RME_A7M_REG(0xE000ED28U)
#define RME_A7M_SCB_HFSR                        RME_A7M_REG(0xE000ED2CU)
#define RME_A7M_SCB_MMFAR                       RME_A7M_REG(0xE000ED34U)
#define RME_A7M_SCB_ID_PFR0                     RME_A7M_REG(0xE000ED40U)
#define RME_A7M_SCB_ID_PFR1                     RME_A7M_REG(0xE000ED44U)
#define RME_A7M_SCB_ID_DFR0                     RME_A7M_REG(0xE000ED48U)
#define RME_A7M_SCB_ID_AFR0                     RME_A7M_REG(0xE000ED4CU)
#define RME_A7M_SCB_ID_MMFR0                    RME_A7M_REG(0xE000ED50U)
#define RME_A7M_SCB_ID_MMFR1                    RME_A7M_REG(0xE000ED54U)
#define RME_A7M_SCB_ID_MMFR2                    RME_A7M_REG(0xE000ED58U)
#define RME_A7M_SCB_ID_MMFR3                    RME_A7M_REG(0xE000ED5CU)
#define RME_A7M_SCB_ID_ISAR0                    RME_A7M_REG(0xE000ED60U)
#define RME_A7M_SCB_ID_ISAR1                    RME_A7M_REG(0xE000ED64U)
#define RME_A7M_SCB_ID_ISAR2                    RME_A7M_REG(0xE000ED68U)
#define RME_A7M_SCB_ID_ISAR3                    RME_A7M_REG(0xE000ED6CU)
#define RME_A7M_SCB_ID_ISAR4                    RME_A7M_REG(0xE000ED70U)
#define RME_A7M_SCB_ID_ISAR5                    RME_A7M_REG(0xE000ED74U)
#define RME_A7M_SCB_CLIDR                       RME_A7M_REG(0xE000ED78U)
#define RME_A7M_SCB_CTR                         RME_A7M_REG(0xE000ED7CU)

#define RME_A7M_SCB_CCSIDR                      RME_A7M_REG(0xE000ED80U)
#define RME_A7M_SCB_CCSIDR_WAYS(X)              (((X)&0x1FF8U)>>3)
#define RME_A7M_SCB_CCSIDR_SETS(X)              (((X)&0x0FFFE000U)>>13)

#define RME_A7M_SCB_CSSELR                      RME_A7M_REG(0xE000ED84U)
#define RME_A7M_SCB_CPACR                       RME_A7M_REG(0xE000ED88U)
#define RME_A7M_MPU_TYPE                        RME_A7M_REG(0xE000ED90U)

#define RME_A7M_MPU_CTRL                        RME_A7M_REG(0xE000ED94U)
#define RME_A7M_MPU_CTRL_PRIVDEF                RME_POW2(2U)
#define RME_A7M_MPU_CTRL_ENABLE                 RME_POW2(0U)

#define RME_A7M_SCNSCB_STIR                     RME_A7M_REG(0xE000EF00U)

#define RME_A7M_SCNSCB_MVFR0                    RME_A7M_REG(0xE000EF40U)
#define RME_A7M_SCNSCB_MVFR1                    RME_A7M_REG(0xE000EF44U)
#define RME_A7M_SCNSCB_MVFR2                    RME_A7M_REG(0xE000EF48U)

#define RME_A7M_SCNSCB_ICALLU                   RME_A7M_REG(0xE000EF50U)
#define RME_A7M_SCNSCB_ICIMVAU                  RME_A7M_REG(0xE000EF58U)
#define RME_A7M_SCNSCB_DCIMVAC                  RME_A7M_REG(0xE000EF5CU)
#define RME_A7M_SCNSCB_DCISW                    RME_A7M_REG(0xE000EF60U)
#define RME_A7M_SCNSCB_DCCMVAC                  RME_A7M_REG(0xE000EF68U)
#define RME_A7M_SCNSCB_DCCSW                    RME_A7M_REG(0xE000EF6CU)
#define RME_A7M_SCNSCB_DCCIMVAC                 RME_A7M_REG(0xE000EF70U)
#define RME_A7M_SCNSCB_DCCISW                   RME_A7M_REG(0xE000EF74U)
#define RME_A7M_SCNSCB_BPIALL                   RME_A7M_REG(0xE000EF78U)

#define RME_A7M_SCNSCB_DC(SET,WAY)              (RME_FIELD(WAY,30U)|RME_FIELD(SET,5U))

#define RME_A7M_SCNSCB_PID4                     RME_A7M_REG(0xE000EFD0U)
#define RME_A7M_SCNSCB_PID5                     RME_A7M_REG(0xE000EFD4U)
#define RME_A7M_SCNSCB_PID6                     RME_A7M_REG(0xE000EFD8U)
#define RME_A7M_SCNSCB_PID7                     RME_A7M_REG(0xE000EFDCU)
#define RME_A7M_SCNSCB_PID0                     RME_A7M_REG(0xE000EFE0U)
#define RME_A7M_SCNSCB_PID1                     RME_A7M_REG(0xE000EFE4U)
#define RME_A7M_SCNSCB_PID2                     RME_A7M_REG(0xE000EFE8U)
#define RME_A7M_SCNSCB_PID3                     RME_A7M_REG(0xE000EFECU)
#define RME_A7M_SCNSCB_CID0                     RME_A7M_REG(0xE000EFF0U)
#define RME_A7M_SCNSCB_CID1                     RME_A7M_REG(0xE000EFF4U)
#define RME_A7M_SCNSCB_CID2                     RME_A7M_REG(0xE000EFF8U)
#define RME_A7M_SCNSCB_CID3                     RME_A7M_REG(0xE000EFFCU)

/* Handler *******************************************************************/
/* Fault definitions */
/* The NMI is active */
#define RME_A7M_ICSR_NMIPENDSET                 RME_POW2(31U)
/* Debug event has occurred. The Debug Fault Status Register has been updated */
#define RME_A7M_HFSR_DEBUGEVT                   RME_POW2(31U)
/* Processor has escalated a configurable-priority exception to HardFault */
#define RME_A7M_HFSR_FORCED                     RME_POW2(30U)
/* Vector table read fault has occurred */
#define RME_A7M_HFSR_VECTTBL                    RME_POW2(1U)
/* Divide by zero */
#define RME_A7M_UFSR_DIVBYZERO                  RME_POW2(25U)
/* Unaligned load/store access */
#define RME_A7M_UFSR_UNALIGNED                  RME_POW2(24U)
/* No such coprocessor */
#define RME_A7M_UFSR_NOCP                       RME_POW2(19U)
/* Invalid vector return LR or PC value */
#define RME_A7M_UFSR_INVPC                      RME_POW2(18U)
/* Attempt to enter an invalid instruction set (ARM) state */
#define RME_A7M_UFSR_INVSTATE                   RME_POW2(17U)
/* Invalid IT instruction or related instructions */
#define RME_A7M_UFSR_UNDEFINSTR                 RME_POW2(16U)
/* The Bus Fault Address Register is valid */
#define RME_A7M_BFSR_BFARVALID                  RME_POW2(15U)
/* The bus fault happened during FP lazy stacking */
#define RME_A7M_BFSR_LSPERR                     RME_POW2(13U)
/* A derived bus fault has occurred on exception entry */
#define RME_A7M_BFSR_STKERR                     RME_POW2(12U)
/* A derived bus fault has occurred on exception return */
#define RME_A7M_BFSR_UNSTKERR                   RME_POW2(11U)
/* Imprecise data access error has occurred */
#define RME_A7M_BFSR_IMPRECISERR                RME_POW2(10U)
/* Precise data access error has occurred, BFAR updated */
#define RME_A7M_BFSR_PRECISERR                  RME_POW2(9U)
/* A bus fault on an instruction prefetch has occurred. The 
 * fault is signaled only if the instruction is issued */
#define RME_A7M_BFSR_IBUSERR                    RME_POW2(8U)
/* The Memory Management Fault Address Register have valid contents */
#define RME_A7M_MFSR_MMARVALID                  RME_POW2(7U)
/* A MemManage fault occurred during FP lazy state preservation */
#define RME_A7M_MFSR_MLSPERR                    RME_POW2(5U)
/* A derived MemManage fault occurred on exception entry */
#define RME_A7M_MFSR_MSTKERR                    RME_POW2(4U)
/* A derived MemManage fault occurred on exception return */
#define RME_A7M_MFSR_MUNSTKERR                  RME_POW2(3U)
/* Data access violation. The MMFAR shows the data address that
 * the load or store tried to access */
#define RME_A7M_MFSR_DACCVIOL                   RME_POW2(1U)
/* MPU or Execute Never (XN) default memory map access violation on an
 * instruction fetch has occurred. The fault is signalled only if the
 * instruction is issued */
#define RME_A7M_MFSR_IACCVIOL                   RME_POW2(0U)

/* Initialization ************************************************************/
/* The capability table of the init process */
#define RME_BOOT_INIT_CPT                       (0U)
/* The top-level page table of the init process - always 4GB full range split into 8 pages */
#define RME_BOOT_INIT_PGT                       (1U)
/* The init process */
#define RME_BOOT_INIT_PRC                       (2U)
/* The init thread */
#define RME_BOOT_INIT_THD                       (3U)
/* The initial kernel function capability */
#define RME_BOOT_INIT_KFN                       (4U)
/* The initial kernel memory capability */
#define RME_BOOT_INIT_KOM                       (5U)
/* The initial timer/interrupt endpoint */
#define RME_BOOT_INIT_VCT                       (6U)

/* Booting capability layout */
#define RME_A7M_CPT                             ((struct RME_Cap_Cpt*)(RME_KOM_VA_BASE))

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
#define RME_A7M_PGT_META                        (sizeof(struct __RME_A7M_Pgt_Meta)/RME_WORD_BYTE)
#define RME_A7M_MPU_DATA                        (sizeof(struct __RME_A7M_MPU_Data)/RME_WORD_BYTE)
#define RME_A7M_PGT_TBL_NOM(X)                  (((rme_ptr_t*)(X))+RME_A7M_PGT_META)
#define RME_A7M_PGT_TBL_TOP(X)                  (((rme_ptr_t*)(X))+RME_A7M_PGT_META+RME_A7M_MPU_DATA)
/* Page entry bit definitions */
#define RME_A7M_PGT_PRESENT                     RME_POW2(0U)
#define RME_A7M_PGT_TERMINAL                    RME_POW2(1U)
/* The address mask for the actual page address */
#define RME_A7M_PGT_PTE_ADDR(X)                 ((X)&0xFFFFFFFCU)
/* The address mask for the next level page table address */
#define RME_A7M_PGT_PGD_ADDR(X)                 ((X)&0xFFFFFFFCU)
/* MPU operation flag */
#define RME_A7M_MPU_CLR                         (0U)
#define RME_A7M_MPU_UPD                         (1U)
/* MPU definitions */
/* Extract address for/from MPU */
#define RME_A7M_MPU_ADDR(X)                     ((X)&0xFFFFFFE0U)
/* Get info from MPU - X is the region's size order, not a subregion */
#define RME_A7M_MPU_SZORD(X)                    ((((X)&0x3FU)>>1)+1U)
/* Write info to MPU - X is the region's size order, not a subregion */
#define RME_A7M_MPU_VALID                       RME_POW2(4U)
#define RME_A7M_MPU_SRDCLR                      (0x0000FF00U)
#define RME_A7M_MPU_XN                          RME_POW2(28U)
#define RME_A7M_MPU_RO                          RME_FIELD(2U,24U)
#define RME_A7M_MPU_RW                          RME_FIELD(3U,24U)
#define RME_A7M_MPU_CACHE                       RME_POW2(17U)
#define RME_A7M_MPU_BUFFER                      RME_POW2(16U)
#define RME_A7M_MPU_SIZE(X)                     RME_FIELD((X)-1U,1U)
#define RME_A7M_MPU_ENABLE                      (1U)

/* Kernel Function ***********************************************************/
/* Page table entry mode which property to get */
#define RME_A7M_KFN_PGT_ENTRY_MOD_FLAG_GET      (0U)
#define RME_A7M_KFN_PGT_ENTRY_MOD_SZORD_GET     (1U)
#define RME_A7M_KFN_PGT_ENTRY_MOD_NMORD_GET     (2U)
/* Interrupt source configuration */
#define RME_A7M_KFN_INT_LOCAL_MOD_STATE_GET     (0U)
#define RME_A7M_KFN_INT_LOCAL_MOD_STATE_SET     (1U)
#define RME_A7M_KFN_INT_LOCAL_MOD_PRIO_GET      (2U)
#define RME_A7M_KFN_INT_LOCAL_MOD_PRIO_SET      (3U)
/* Cache identifier */
#define RME_A7M_KFN_CACHE_ICACHE                (0U)
#define RME_A7M_KFN_CACHE_DCACHE                (1U)
#define RME_A7M_KFN_CACHE_BTAC                  (2U)
/* Cache modification */
#define RME_A7M_KFN_CACHE_MOD_STATE_GET         (0U)
#define RME_A7M_KFN_CACHE_MOD_STATE_SET         (1U)
/* Cache state */
#define RME_A7M_KFN_CACHE_STATE_DISABLE         (0U)
#define RME_A7M_KFN_CACHE_STATE_ENABLE          (1U)
/* Cache maintenance */
#define RME_A7M_KFN_CACHE_CLEAN_ALL             (0U)
#define RME_A7M_KFN_CACHE_CLEAN_ADDR            (1U)
#define RME_A7M_KFN_CACHE_CLEAN_SET             (2U)
#define RME_A7M_KFN_CACHE_CLEAN_WAY             (3U)
#define RME_A7M_KFN_CACHE_CLEAN_SETWAY          (4U)
#define RME_A7M_KFN_CACHE_INV_ALL               (5U)
#define RME_A7M_KFN_CACHE_INV_ADDR              (6U)
#define RME_A7M_KFN_CACHE_INV_SET               (7U)
#define RME_A7M_KFN_CACHE_INV_WAY               (8U)
#define RME_A7M_KFN_CACHE_INV_SETWAY            (9U)
#define RME_A7M_KFN_CACHE_CLEAN_INV_ALL         (10U)
#define RME_A7M_KFN_CACHE_CLEAN_INV_ADDR        (11U)
#define RME_A7M_KFN_CACHE_CLEAN_INV_SET         (12U)
#define RME_A7M_KFN_CACHE_CLEAN_INV_WAY         (13U)
#define RME_A7M_KFN_CACHE_CLEAN_INV_SETWAY      (14U)
/* Prefetcher modification */
#define RME_A7M_KFN_PRFTH_MOD_STATE_GET         (0U)
#define RME_A7M_KFN_PRFTH_MOD_STATE_SET         (1U)
/* Prefetcher state */
#define RME_A7M_KFN_PRFTH_STATE_DISABLE         (0U)
#define RME_A7M_KFN_PRFTH_STATE_ENABLE          (1U)
/* CPU feature support */
#define RME_A7M_KFN_CPU_FUNC_CPUID              (0U)
#define RME_A7M_KFN_CPU_FUNC_ID_PFR0            (1U)
#define RME_A7M_KFN_CPU_FUNC_ID_PFR1            (2U)
#define RME_A7M_KFN_CPU_FUNC_ID_DFR0            (3U)
#define RME_A7M_KFN_CPU_FUNC_ID_AFR0            (4U)
#define RME_A7M_KFN_CPU_FUNC_ID_MMFR0           (5U)
#define RME_A7M_KFN_CPU_FUNC_ID_MMFR1           (6U)
#define RME_A7M_KFN_CPU_FUNC_ID_MMFR2           (7U)
#define RME_A7M_KFN_CPU_FUNC_ID_MMFR3           (8U)
#define RME_A7M_KFN_CPU_FUNC_ID_ISAR0           (9U)
#define RME_A7M_KFN_CPU_FUNC_ID_ISAR1           (10U)
#define RME_A7M_KFN_CPU_FUNC_ID_ISAR2           (11U)
#define RME_A7M_KFN_CPU_FUNC_ID_ISAR3           (12U)
#define RME_A7M_KFN_CPU_FUNC_ID_ISAR4           (13U)
#define RME_A7M_KFN_CPU_FUNC_ID_ISAR5           (14U)
#define RME_A7M_KFN_CPU_FUNC_CLIDR              (15U)
#define RME_A7M_KFN_CPU_FUNC_CTR                (16U)
#define RME_A7M_KFN_CPU_FUNC_ICACHE_CCSIDR      (17U)
#define RME_A7M_KFN_CPU_FUNC_DCACHE_CCSIDR      (18U)
#define RME_A7M_KFN_CPU_FUNC_MPU_TYPE           (19U)
#define RME_A7M_KFN_CPU_FUNC_MVFR0              (20U)
#define RME_A7M_KFN_CPU_FUNC_MVFR1              (21U)
#define RME_A7M_KFN_CPU_FUNC_MVFR2              (22U)
#define RME_A7M_KFN_CPU_FUNC_PID0               (23U)
#define RME_A7M_KFN_CPU_FUNC_PID1               (24U)
#define RME_A7M_KFN_CPU_FUNC_PID2               (25U)
#define RME_A7M_KFN_CPU_FUNC_PID3               (26U)
#define RME_A7M_KFN_CPU_FUNC_PID4               (27U)
#define RME_A7M_KFN_CPU_FUNC_PID5               (28U)
#define RME_A7M_KFN_CPU_FUNC_PID6               (29U)
#define RME_A7M_KFN_CPU_FUNC_PID7               (30U)
#define RME_A7M_KFN_CPU_FUNC_CID0               (31U)
#define RME_A7M_KFN_CPU_FUNC_CID1               (32U)
#define RME_A7M_KFN_CPU_FUNC_CID2               (33U)
#define RME_A7M_KFN_CPU_FUNC_CID3               (34U)
/* Perfomance counters */
#define RME_A7M_KFN_PERF_CYCLE_CYCCNT           (0U)
/* Performance counter state operations */
#define RME_A7M_KFN_PERF_STATE_GET              (0U)
#define RME_A7M_KFN_PERF_STATE_SET              (1U)
/* Performance counter states */
#define RME_A7M_KFN_PERF_STATE_DISABLE          (0U)
#define RME_A7M_KFN_PERF_STATE_ENABLE           (1U)
/* Performance counter value operations */
#define RME_A7M_KFN_PERF_VAL_GET                (0U)
#define RME_A7M_KFN_PERF_VAL_SET                (1U)
/* Register read/write */
#define RME_A7M_KFN_DEBUG_REG_MOD_GET           (0U)
#define RME_A7M_KFN_DEBUG_REG_MOD_SET           RME_POW2(16U)
/* General-purpose registers */
#define RME_A7M_KFN_DEBUG_REG_MOD_SP            (0U)
#define RME_A7M_KFN_DEBUG_REG_MOD_R4            (1U)
#define RME_A7M_KFN_DEBUG_REG_MOD_R5            (2U)
#define RME_A7M_KFN_DEBUG_REG_MOD_R6            (3U)
#define RME_A7M_KFN_DEBUG_REG_MOD_R7            (4U)
#define RME_A7M_KFN_DEBUG_REG_MOD_R8            (5U)
#define RME_A7M_KFN_DEBUG_REG_MOD_R9            (6U)
#define RME_A7M_KFN_DEBUG_REG_MOD_R10           (7U)
#define RME_A7M_KFN_DEBUG_REG_MOD_R11           (8U)
#define RME_A7M_KFN_DEBUG_REG_MOD_LR            (9U)
/* FPU registers */
#define RME_A7M_KFN_DEBUG_REG_MOD_S16           (10U)
#define RME_A7M_KFN_DEBUG_REG_MOD_S17           (11U)
#define RME_A7M_KFN_DEBUG_REG_MOD_S18           (12U)
#define RME_A7M_KFN_DEBUG_REG_MOD_S19           (13U)
#define RME_A7M_KFN_DEBUG_REG_MOD_S20           (14U)
#define RME_A7M_KFN_DEBUG_REG_MOD_S21           (15U)
#define RME_A7M_KFN_DEBUG_REG_MOD_S22           (16U)
#define RME_A7M_KFN_DEBUG_REG_MOD_S23           (17U)
#define RME_A7M_KFN_DEBUG_REG_MOD_S24           (18U)
#define RME_A7M_KFN_DEBUG_REG_MOD_S25           (19U)
#define RME_A7M_KFN_DEBUG_REG_MOD_S26           (20U)
#define RME_A7M_KFN_DEBUG_REG_MOD_S27           (21U)
#define RME_A7M_KFN_DEBUG_REG_MOD_S28           (22U)
#define RME_A7M_KFN_DEBUG_REG_MOD_S29           (23U)
#define RME_A7M_KFN_DEBUG_REG_MOD_S30           (24U)
#define RME_A7M_KFN_DEBUG_REG_MOD_S31           (25U)
/* Invocation register read/write */
#define RME_A7M_KFN_DEBUG_INV_MOD_SP_GET        (0U)
#define RME_A7M_KFN_DEBUG_INV_MOD_SP_SET        (1U)
#define RME_A7M_KFN_DEBUG_INV_MOD_LR_GET        (2U)
#define RME_A7M_KFN_DEBUG_INV_MOD_LR_SET        (3U)
/* Exception register read */
#define RME_A7M_KFN_DEBUG_EXC_CAUSE_GET         (0U)
#define RME_A7M_KFN_DEBUG_EXC_ADDR_GET          (1U)
/* End ARMv7-M Macro *********************************************************/
/*****************************************************************************/
/* __RME_PLATFORM_A7M_DEF__ */
#endif
/* __HDR_DEF__ */
#endif
/* End Define ****************************************************************/

/* Struct ********************************************************************/
#ifdef __HDR_STRUCT__
#ifndef __RME_PLATFORM_A7M_STRUCT__
#define __RME_PLATFORM_A7M_STRUCT__
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
 * Here we need LR to decide EXC_RETURN, that's why it is here */
struct RME_Reg_Struct
{
    rme_ptr_t SP;
    rme_ptr_t R4;
    rme_ptr_t R5;
    rme_ptr_t R6;
    rme_ptr_t R7;
    rme_ptr_t R8;
    rme_ptr_t R9;
    rme_ptr_t R10;
    rme_ptr_t R11;
    rme_ptr_t LR;
};

/* The coprocessor register set structure. In ARMv7-M, if there is a 
 * single-precision FPU, then the FPU S0-S15 is automatically pushed */
struct RME_A7M_Cop_Struct
{
    rme_ptr_t S16;
    rme_ptr_t S17;
    rme_ptr_t S18;
    rme_ptr_t S19;
    rme_ptr_t S20;
    rme_ptr_t S21;
    rme_ptr_t S22;
    rme_ptr_t S23;
    rme_ptr_t S24;
    rme_ptr_t S25;
    rme_ptr_t S26;
    rme_ptr_t S27;
    rme_ptr_t S28;
    rme_ptr_t S29;
    rme_ptr_t S30;
    rme_ptr_t S31;
};

struct RME_Exc_Struct
{
    rme_ptr_t Cause;
    rme_ptr_t Addr;
};

/* Invocation register set structure */
struct RME_Iret_Struct
{
    rme_ptr_t LR;
    rme_ptr_t SP;
};

/* Page Table ****************************************************************/
/* MPU entry structure */
struct __RME_A7M_MPU_Entry
{
    rme_ptr_t RBAR;
    rme_ptr_t RASR;
};

/* Page table metadata structure */
#if(RME_PGT_RAW_ENABLE==0U)
struct __RME_A7M_Pgt_Meta
{
    /* The MPU setting is always in the top level. This is a pointer to the top level */
    rme_ptr_t Toplevel;
    /* The start mapping address of this page table, also as a top-level indicator */
    rme_ptr_t Base;
    /* The size/num order of this level (half word each) */
    rme_ptr_t Order;
    /* The page flags at this level. If any pages are mapped in, it must conform
     * to the same attributes as the older pages */
    rme_ptr_t Page_Flag;
};
#endif

/* Raw MPU cache - naked for user-level configurations only */
struct __RME_A7M_Raw_Pgt
{
    struct __RME_A7M_MPU_Entry Data[RME_A7M_REGION_NUM];
};

/* MPU metadata structure */
#if(RME_PGT_RAW_ENABLE==0U)
struct __RME_A7M_MPU_Data
{
    /* Bitmap showing whether these are static or not */
    rme_ptr_t Static;
    /* The MPU data itself */
    struct __RME_A7M_Raw_Pgt Raw;
};
#endif
/*****************************************************************************/
/* __RME_PLATFORM_A7M_STRUCT__ */
#endif
/* __HDR_STRUCT__ */
#endif
/* End Struct ****************************************************************/

/* Private Variable **********************************************************/
#if(!(defined __HDR_DEF__||defined __HDR_STRUCT__))
#ifndef __RME_PLATFORM_A7M_MEMBER__
#define __RME_PLATFORM_A7M_MEMBER__

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
#if(RME_RVM_GEN_ENABLE!=0U)
RME_EXTERN rme_ptr_t RME_Boot_Vct_Handler(rme_ptr_t Vct_Num);
RME_EXTERN rme_ptr_t RME_Boot_Vct_Init(struct RME_Cap_Cpt* Cpt,
                                       rme_ptr_t Cap_Front,
                                       rme_ptr_t Kom_Front);
RME_EXTERN void RME_Boot_Pre_Init(void);
RME_EXTERN void RME_Boot_Post_Init(void);
RME_EXTERN void RME_Reboot_Failsafe(void);
RME_EXTERN rme_ret_t RME_Hook_Kfn_Handler(rme_ptr_t Func_ID,
                                          rme_ptr_t Sub_ID,
                                          rme_ptr_t Param1,
                                          rme_ptr_t Param2);
#endif
/* MPU operations ************************************************************/
RME_EXTERN void ___RME_A7M_MPU_Set1(struct __RME_A7M_Raw_Pgt* Raw);
RME_EXTERN void ___RME_A7M_MPU_Set2(struct __RME_A7M_Raw_Pgt* Raw);
RME_EXTERN void ___RME_A7M_MPU_Set3(struct __RME_A7M_Raw_Pgt* Raw);
RME_EXTERN void ___RME_A7M_MPU_Set4(struct __RME_A7M_Raw_Pgt* Raw);
RME_EXTERN void ___RME_A7M_MPU_Set5(struct __RME_A7M_Raw_Pgt* Raw);
RME_EXTERN void ___RME_A7M_MPU_Set6(struct __RME_A7M_Raw_Pgt* Raw);
RME_EXTERN void ___RME_A7M_MPU_Set7(struct __RME_A7M_Raw_Pgt* Raw);
RME_EXTERN void ___RME_A7M_MPU_Set8(struct __RME_A7M_Raw_Pgt* Raw);
RME_EXTERN void ___RME_A7M_MPU_Set9(struct __RME_A7M_Raw_Pgt* Raw);
RME_EXTERN void ___RME_A7M_MPU_Set10(struct __RME_A7M_Raw_Pgt* Raw);
RME_EXTERN void ___RME_A7M_MPU_Set11(struct __RME_A7M_Raw_Pgt* Raw);
RME_EXTERN void ___RME_A7M_MPU_Set12(struct __RME_A7M_Raw_Pgt* Raw);
RME_EXTERN void ___RME_A7M_MPU_Set13(struct __RME_A7M_Raw_Pgt* Raw);
RME_EXTERN void ___RME_A7M_MPU_Set14(struct __RME_A7M_Raw_Pgt* Raw);
RME_EXTERN void ___RME_A7M_MPU_Set15(struct __RME_A7M_Raw_Pgt* Raw);
RME_EXTERN void ___RME_A7M_MPU_Set16(struct __RME_A7M_Raw_Pgt* Raw);
/* Vector flags **************************************************************/
static void __RME_A7M_Flag_Fast(rme_ptr_t Base,
                                rme_ptr_t Size,
                                rme_ptr_t Flag);
static void __RME_A7M_Flag_Slow(rme_ptr_t Base,
                                rme_ptr_t Size,
                                rme_ptr_t Pos);
/* Page table ****************************************************************/
#if(RME_PGT_RAW_ENABLE==0U)
static rme_ptr_t __RME_A7M_Rand(void);
static rme_ptr_t ___RME_A7M_MPU_RASR_Gen(rme_ptr_t* Table,
                                         rme_ptr_t Flag, 
                                         rme_ptr_t Size_Order,
                                         rme_ptr_t Num_Order);
static rme_ret_t ___RME_A7M_MPU_Clear(struct __RME_A7M_MPU_Data* Top_MPU, 
                                      rme_ptr_t Base_Addr,
                                      rme_ptr_t Size_Order,
                                      rme_ptr_t Num_Order);
static rme_ret_t ___RME_A7M_MPU_Add(struct __RME_A7M_MPU_Data* Top_MPU, 
                                    rme_ptr_t Base_Addr,
                                    rme_ptr_t Size_Order,
                                    rme_ptr_t Num_Order,
                                    rme_ptr_t RASR,
                                    rme_ptr_t Static);
static rme_ret_t ___RME_A7M_MPU_Update(struct __RME_A7M_Pgt_Meta* Meta,
                                       rme_ptr_t Op_Flag);
static rme_ptr_t ___RME_A7M_Pgt_Have_Page(rme_ptr_t* Table,
                                          rme_ptr_t Num_Order);
static rme_ptr_t ___RME_A7M_Pgt_Have_Pgdir(rme_ptr_t* Table,
                                           rme_ptr_t Num_Order);
static void ___RME_A7M_Pgt_Refresh(void);
#endif
/* Kernel function ***********************************************************/
#if(RME_PGT_RAW_ENABLE==0U)
static rme_ret_t __RME_A7M_Pgt_Entry_Mod(struct RME_Cap_Cpt* Cpt, 
                                         rme_cid_t Cap_Pgt,
                                         rme_ptr_t Vaddr,
                                         rme_ptr_t Type);
#endif
static rme_ret_t __RME_A7M_Int_Local_Mod(rme_ptr_t Int_Num,
                                         rme_ptr_t Operation,
                                         rme_ptr_t Param);
static rme_ret_t __RME_A7M_Int_Local_Trig(rme_ptr_t CPUID,
                                          rme_ptr_t Int_Num);
static rme_ret_t __RME_A7M_Evt_Local_Trig(struct RME_Reg_Struct* Reg,
                                          rme_ptr_t CPUID,
                                          rme_ptr_t Evt_Num);
static rme_ret_t __RME_A7M_Cache_Mod(rme_ptr_t Cache_ID,
                                     rme_ptr_t Operation,
                                     rme_ptr_t Param);
static rme_ret_t __RME_A7M_Cache_Maint(rme_ptr_t Cache_ID,
                                       rme_ptr_t Operation,
                                       rme_ptr_t Param);
static rme_ret_t __RME_A7M_Prfth_Mod(rme_ptr_t Prfth_ID,
                                     rme_ptr_t Operation,
                                     rme_ptr_t Param);
static rme_ret_t __RME_A7M_Perf_CPU_Func(struct RME_Reg_Struct* Reg,
                                         rme_ptr_t Freg_ID);
static rme_ret_t __RME_A7M_Perf_Mon_Mod(rme_ptr_t Perf_ID,
                                        rme_ptr_t Operation,
                                        rme_ptr_t Param);
static rme_ret_t __RME_A7M_Perf_Cycle_Mod(struct RME_Reg_Struct* Reg,
                                          rme_ptr_t Cycle_ID, 
                                          rme_ptr_t Operation,
                                          rme_ptr_t Value);
static rme_ret_t __RME_A7M_Debug_Reg_Mod(struct RME_Cap_Cpt* Cpt,
                                         struct RME_Reg_Struct* Reg, 
                                         rme_cid_t Cap_Thd,
                                         rme_ptr_t Operation,
                                         rme_ptr_t Value);
static rme_ret_t __RME_A7M_Debug_Inv_Mod(struct RME_Cap_Cpt* Cpt,
                                         struct RME_Reg_Struct* Reg, 
                                         rme_cid_t Cap_Thd,
                                         rme_ptr_t Operation,
                                         rme_ptr_t Value);
static rme_ret_t __RME_A7M_Debug_Exc_Get(struct RME_Cap_Cpt* Cpt,
                                         struct RME_Reg_Struct* Reg, 
                                         rme_cid_t Cap_Thd,
                                         rme_ptr_t Operation);
/*****************************************************************************/
#define __RME_EXTERN__
/* End Private Function ******************************************************/

/* Public Variable ***********************************************************/
/* __HDR_PUBLIC__ */
#else
#define __RME_EXTERN__ RME_EXTERN 
/* __HDR_PUBLIC__ */
#endif

/*****************************************************************************/
/* Timestamp counter */
__RME_EXTERN__ rme_ptr_t RME_A7M_Timestamp;
/* ARMv7-M only have one core, thus this is its CPU-local data structure */
__RME_EXTERN__ struct RME_CPU_Local RME_A7M_Local;
/* ARMv7-M use simple kernel object table */
__RME_EXTERN__ rme_ptr_t RME_A7M_Kot[RME_KOT_WORD_NUM];
/*****************************************************************************/

/* End Public Variable *******************************************************/

/* Public Function ***********************************************************/
/* Generic *******************************************************************/
/* Interrupts */
RME_EXTERN void __RME_Int_Disable(void);
RME_EXTERN void __RME_Int_Enable(void);
RME_EXTERN void __RME_A7M_Barrier(void);
RME_EXTERN void __RME_A7M_Wait_Int(void);
/* MSB counting */
RME_EXTERN rme_ptr_t __RME_A7M_MSB_Get(rme_ptr_t Val);
/* Getting CPUID */
__RME_EXTERN__ rme_ptr_t __RME_CPUID_Get(void);
/* Printing */
__RME_EXTERN__ void __RME_Putchar(char Char);
/* Handler *******************************************************************/
/* Fault handler */
__RME_EXTERN__ void __RME_A7M_Exc_Handler(struct RME_Reg_Struct* Reg);
/* Generic interrupt handler */
__RME_EXTERN__ void __RME_A7M_Vct_Handler(struct RME_Reg_Struct* Reg,
                                          rme_ptr_t Vct_Num);
/* Timer handler */
__RME_EXTERN__ void __RME_A7M_Tim_Handler(struct RME_Reg_Struct* Reg);
/* Syscall handler */
__RME_EXTERN__ void __RME_A7M_Svc_Handler(struct RME_Reg_Struct* Reg);

/* Kernel function handler */
__RME_EXTERN__ rme_ret_t __RME_Kfn_Handler(struct RME_Cap_Cpt* Cpt,
                                           struct RME_Reg_Struct* Reg,
                                           rme_ptr_t Func_ID,
                                           rme_ptr_t Sub_ID,
                                           rme_ptr_t Param1,
                                           rme_ptr_t Param2);

/* Initialization ************************************************************/
__RME_EXTERN__ void __RME_A7M_Lowlvl_Preinit(void);
__RME_EXTERN__ void __RME_Lowlvl_Init(void);
__RME_EXTERN__ void __RME_Boot(void);
RME_EXTERN void __RME_A7M_Reset(void);
__RME_EXTERN__ void __RME_A7M_Reboot(void);
__RME_EXTERN__ void __RME_A7M_NVIC_Set_Exc_Prio(rme_cnt_t Exc,
                                                rme_ptr_t Prio);
__RME_EXTERN__ void __RME_A7M_Cache_Init(void);
RME_EXTERN void __RME_User_Enter(rme_ptr_t Entry,
                                 rme_ptr_t Stack,
                                 rme_ptr_t CPUID);

/* Register manipulation *****************************************************/
/* Coprocessor */
RME_EXTERN void ___RME_A7M_Thd_Cop_Clear(void);
RME_EXTERN void ___RME_A7M_Thd_Cop_Save(struct RME_A7M_Cop_Struct* Cop);
RME_EXTERN void ___RME_A7M_Thd_Cop_Load(struct RME_A7M_Cop_Struct* Cop);
/* Syscall parameter */
__RME_EXTERN__ void __RME_Svc_Param_Get(struct RME_Reg_Struct* Reg,
                                        rme_ptr_t* Svc,
                                        rme_ptr_t* Capid,
                                        rme_ptr_t* Param);
__RME_EXTERN__ void __RME_Svc_Retval_Set(struct RME_Reg_Struct* Reg,
                                         rme_ret_t Retval);
/* Thread register sets */
__RME_EXTERN__ void __RME_Thd_Reg_Init(rme_ptr_t Attr,
                                       rme_ptr_t Entry,
                                       rme_ptr_t Stack,
                                       rme_ptr_t Param,
                                       struct RME_Reg_Struct* Reg);
__RME_EXTERN__ void __RME_Thd_Reg_Copy(struct RME_Reg_Struct* Dst,
                                       struct RME_Reg_Struct* Src);
/* Invocation register sets */
__RME_EXTERN__ void __RME_Inv_Reg_Save(struct RME_Iret_Struct* Ret,
                                       struct RME_Reg_Struct* Reg);
__RME_EXTERN__ void __RME_Inv_Reg_Restore(struct RME_Reg_Struct* Reg,
                                          struct RME_Iret_Struct* Ret);
__RME_EXTERN__ void __RME_Inv_Retval_Set(struct RME_Reg_Struct* Reg,
                                         rme_ret_t Retval);
/* Coprocessor register sets */
__RME_EXTERN__ rme_ret_t __RME_Thd_Cop_Check(rme_ptr_t Attr);
__RME_EXTERN__ rme_ptr_t __RME_Thd_Cop_Size(rme_ptr_t Attr);
__RME_EXTERN__ void __RME_Thd_Cop_Init(rme_ptr_t Attr,
                                       struct RME_Reg_Struct* Reg,
                                       void* Cop);
__RME_EXTERN__ void __RME_Thd_Cop_Swap(rme_ptr_t Attr_New,
                                       struct RME_Reg_Struct* Reg_New,
                                       void* Cop_New,
                                       rme_ptr_t Attr_Cur,
                                       struct RME_Reg_Struct* Reg_Cur,
                                       void* Cop_Cur);

/* Page table ****************************************************************/
/* Kernel portion initialization */
__RME_EXTERN__ rme_ret_t __RME_Pgt_Kom_Init(void);
#if(RME_PGT_RAW_ENABLE!=0U)
/* Setting the page table */
__RME_EXTERN__ void __RME_Pgt_Set(rme_ptr_t Pgt);
#else
/* Setting the page table */
__RME_EXTERN__ void __RME_Pgt_Set(struct RME_Cap_Pgt* Pgt);
/* Initialization */
__RME_EXTERN__ rme_ret_t __RME_Pgt_Init(struct RME_Cap_Pgt* Pgt_Op);
/* Checking */
__RME_EXTERN__ rme_ret_t __RME_Pgt_Check(rme_ptr_t Base_Addr,
                                         rme_ptr_t Is_Top, 
                                         rme_ptr_t Size_Order,
                                         rme_ptr_t Num_Order,
                                         rme_ptr_t Vaddr);
__RME_EXTERN__ rme_ret_t __RME_Pgt_Del_Check(struct RME_Cap_Pgt* Pgt_Op);
/* Table operations */
__RME_EXTERN__ rme_ret_t __RME_Pgt_Page_Map(struct RME_Cap_Pgt* Pgt_Op,
                                            rme_ptr_t Paddr,
                                            rme_ptr_t Pos,
                                            rme_ptr_t Flag);
__RME_EXTERN__ rme_ret_t __RME_Pgt_Page_Unmap(struct RME_Cap_Pgt* Pgt_Op,
                                              rme_ptr_t Pos);
__RME_EXTERN__ rme_ret_t __RME_Pgt_Pgdir_Map(struct RME_Cap_Pgt* Pgt_Parent,
                                             rme_ptr_t Pos, 
                                             struct RME_Cap_Pgt* Pgt_Child,
                                             rme_ptr_t Flag);
__RME_EXTERN__ rme_ret_t __RME_Pgt_Pgdir_Unmap(struct RME_Cap_Pgt* Pgt_Parent,
                                               rme_ptr_t Pos,
                                               struct RME_Cap_Pgt* Pgt_Child);
/* Lookup and walking */
__RME_EXTERN__ rme_ret_t __RME_Pgt_Lookup(struct RME_Cap_Pgt* Pgt_Op,
                                          rme_ptr_t Pos,
                                          rme_ptr_t* Paddr,
                                          rme_ptr_t* Flag);
__RME_EXTERN__ rme_ret_t __RME_Pgt_Walk(struct RME_Cap_Pgt* Pgt_Op,
                                        rme_ptr_t Vaddr,
                                        rme_ptr_t* Pgt,
                                        rme_ptr_t* Map_Vaddr,
                                        rme_ptr_t* Paddr,
                                        rme_ptr_t* Size_Order,
                                        rme_ptr_t* Num_Order,
                                        rme_ptr_t* Flag);
#endif
/*****************************************************************************/
/* Undefine "__RME_EXTERN__" to avoid redefinition */
#undef __RME_EXTERN__
/* __RME_PLATFORM_A7M_MEMBER__ */
#endif
/* !(defined __HDR_DEF__||defined __HDR_STRUCT__) */
#endif
/* End Public Function *******************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
