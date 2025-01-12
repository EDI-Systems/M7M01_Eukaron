/******************************************************************************
Filename    : rme_platform_a7a.h
Author      : pry
Date        : 01/04/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The header of "rme_platform_a7a.c".
******************************************************************************/

/* Define ********************************************************************/
#ifdef __HDR_DEF__
#ifndef __RME_PLATFORM_A7A_DEF__
#define __RME_PLATFORM_A7A_DEF__
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
#define RME_EXTERN                          extern
/* Compiler "inline" keyword setting */
#define RME_INLINE                          inline
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
#define RME_CPU_LOCAL()                 (__RME_A7A_CPU_Local_Get())
/* The order of bits in one CPU machine word */
#define RME_WORD_ORDER                  (5U)
/* Forcing VA=PA in user memory segments */
#define RME_VA_EQU_PA                   (RME_FALSE)
/* Quiescence timeslice value */
#define RME_QUIE_TIME                   0
/* Cpt size limit - not restricted */
#define RME_CPT_LIMIT                   0
/* Cpt size limit - not restricted */
#define RME_CPT_ENTRY_MAX                       (0U)
/* Read timestamp counter */
#define RME_TIMESTAMP                           (RME_A7A_Timestamp)
/* Invocation stack maximum depth - not restricted */
#define RME_INV_DEPTH_MAX                       (0U)
/* Normal page directory size calculation macro */
#define RME_PGT_SIZE_NOM(NUM_ORDER)   (1<<(NUM_ORDER))
/* Top-level page directory size calculation macro */
#define RME_PGT_SIZE_TOP(NUM_ORDER)   RME_PGT_SIZE_NOM(NUM_ORDER)
/* The kernel object allocation table address - original */
#define RME_KOT_VA_BASE                       RME_A7A_Kot
/* Compare-and-Swap(CAS) */
//#define RME_COMP_SWAP(PTR,OLD,NEW)      __RME_A7A_Comp_Swap(PTR,OLD,NEW)
#define RME_COMP_SWAP(PTR,OLD,NEW)              _RME_Comp_Swap_Single(PTR,OLD,NEW)
/* Fetch-and-Add(FAA) */
#define RME_FETCH_ADD(PTR,ADDEND)       __RME_A7A_Fetch_Add(PTR,ADDEND)
/* Fetch-and-And(FAND) */
#define RME_FETCH_AND(PTR,OPERAND)      __RME_A7A_Fetch_And(PTR,OPERAND)
/* Get most significant bit */
#define RME_MSB_GET(VAL)                __RME_A7A_MSB_Get(VAL)
/* Read/write barrier both needed on MPCore, because ARM is weakly ordered */
#define RME_READ_ACQUIRE(X)             __RME_A7A_Read_Acquire(X)
#define RME_WRITE_RELEASE(X,V)          __RME_A7A_Write_Release(X,V)

/* The CPU and application specific macros are here */
#include "rme_platform_a7a_conf.h"
/* End System macros *********************************************************/

/* Cortex-A specific macros **************************************************/
/* Initial boot capabilities */
/* The capability table of the init process */
#define RME_BOOT_INIT_CPT                 0
/* The top-level page table of the init process - always 4GB full range split into 8 pages */
#define RME_BOOT_INIT_PGT                  1
/* The init process */
#define RME_BOOT_INIT_PRC              2
/* The init thread */
#define RME_BOOT_INIT_THD               3
/* The initial kernel function capability */
#define RME_BOOT_INIT_KERN              4
/* The initial kernel memory capability */
#define RME_BOOT_INIT_KOM              5
/* The initial timer endpoint */
#define RME_BOOT_INIT_TIMER             6
/* The initial default endpoint for all other interrupts */
#define RME_BOOT_INIT_INT               7

/* Booting capability layout */
#define RME_A7A_CPT                     ((struct RME_Cap_Cpt*)(RME_KOM_VA_START))
/* Kernel virtual address base - this is fixed */
#define RME_A7A_VA_BASE                 (0x80000000U)
/* For Cortex-A:
 * The layout of the page entry is complicated.
 * Refer to ARMv7-AR architecture reference manual (ARM) for details.
 * Physical address extension is NOT supported in RME */
/* Get the actual table positions */
#define RME_A7A_PGT_TBL_NOM(X)        (X)
#define RME_A7A_PGT_TBL_TOP(X)        (X)

/* Cortex-A (ARMv7) */
#define RME_A7A_PGREG_POS(TABLE)        (((union __RME_A7A_Pgreg*)RME_A7A_PGREG_START) \
		                                  [(((rme_ptr_t)(TABLE))-RME_A7A_VA_BASE)>>RME_PGT_SIZE_1K])

/* MMU definitions operation flags, assuming the following changes:
 * TTBCR=0 : TTBR1 not used,
 * SCTLR.AFE=1 : AP[2:1] is the permission flags, AP[0] is now the access flag.
 * DACR=0x55555555 : All pages/tables are client and access permissions always checked.
 * SCTLR.TRE=1 : TEX remap engaged, CACHEABLE and BUFFERABLE works as RME defined.
 *               {MSB TEX[0], C, B LSB} will index PRRR and NMRR.
 * PRRR=0b 00 00 00 00 00 00 10 10 00 00 00 00 10 10 01 00=0x000A00A4
 *         OUTER_SHARE PTBL_DECIDE xx xx xx xx CB C- -B --
 *      In PRRR configuration, shareability is decided by the page table. This
 *      facilitates cache-incoherent user-level systems. Also, if the memory
 *      is shareable, then it is both inner and outer shareable.
 * NMRR=0b 00 00 00 00 01 10 11 00 00 00 00 00  01 10 11 00=0x006C006C
 *         xx xx xx xx CB C- -B -- xx xx xx xx  CB C- -B --
 * -- : Non-cacheable non-bufferable - strongly ordered, caching not allowed at all
 * -B : Non-cacheable bufferable - device, write-back without write-allocate (since
 *      reads will still have to load words from memory)
 * C- : Cacheable non-bufferable - normal, write-through without write allocate
 * CB : Cacheable bufferable - normal memory, write-back, write-allocate */
#define RME_A7A_MMU_1M_PGDIR_PRESENT    (0x02U)
#define RME_A7A_MMU_1M_PGDIR_NOTSECURE  (1U<<3U)
#define RME_A7A_MMU_1M_PAGE_PRESENT     (0x01U)
#define RME_A7A_MMU_1M_BUFFERABLE       (1U<<2U)
#define RME_A7A_MMU_1M_CACHEABLE        (1U<<3U)
#define RME_A7A_MMU_1M_EXECUTENEVER     (1U<<4U)
#define RME_A7A_MMU_1M_USER             (1U<<11U)
#define RME_A7A_MMU_1M_READONLY         (1U<<15U)
#define RME_A7A_MMU_1M_SHAREABLE        (1U<<16U)
#define RME_A7A_MMU_1M_NOTGLOBAL        (1U<<17U)
#define RME_A7A_MMU_1M_NOTSECURE        (1U<<19U)

#define RME_A7A_MMU_1M_PAGE_USER_COMMON (RME_A7A_MMU_1M_PAGE_PRESENT|RME_A7A_MMU_1M_USER| \
		                                  RME_A7A_MMU_1M_SHAREABLE|RME_A7A_MMU_1M_NOTGLOBAL)
#define RME_A7A_MMU_1M_PAGE_KERN_COMMON (RME_A7A_MMU_1M_PAGE_PRESENT|RME_A7A_MMU_1M_SHAREABLE|RME_A7A_MMU_1M_NOTGLOBAL)

/* These definitions are only used by the initial page table */
#define RME_A7A_MMU_1M_PAGE_USER_DEF    (RME_A7A_MMU_1M_PAGE_USER_COMMON|RME_A7A_MMU_1M_BUFFERABLE|RME_A7A_MMU_1M_CACHEABLE)
#define RME_A7A_MMU_1M_PAGE_KERN_DEF    (RME_A7A_MMU_1M_PAGE_KERN_COMMON|RME_A7A_MMU_1M_BUFFERABLE|RME_A7A_MMU_1M_CACHEABLE)
#define RME_A7A_MMU_1M_PAGE_KERN_DEV    (RME_A7A_MMU_1M_PAGE_KERN_COMMON|RME_A7A_MMU_1M_BUFFERABLE)
#define RME_A7A_MMU_1M_PAGE_KERN_SEQ    (RME_A7A_MMU_1M_PAGE_KERN_COMMON)

#define RME_A7A_MMU_4K_EXECUTENEVER     (1U<<0U)
#define RME_A7A_MMU_4K_PAGE_PRESENT     (1U<<1U)
#define RME_A7A_MMU_4K_BUFFERABLE       (1U<<2U)
#define RME_A7A_MMU_4K_CACHEABLE        (1U<<3U)
#define RME_A7A_MMU_4K_USER             (1U<<5U)
#define RME_A7A_MMU_4K_READONLY         (1U<<9U)
#define RME_A7A_MMU_4K_SHAREABLE        (1U<<10U)
#define RME_A7A_MMU_4K_NOTGLOBAL        (1U<<11U)

#define RME_A7A_MMU_4K_PAGE_USER_COMMON (RME_A7A_MMU_4K_PAGE_PRESENT|RME_A7A_MMU_4K_USER| \
                                          RME_A7A_MMU_4K_SHAREABLE|RME_A7A_MMU_4K_NOTGLOBAL)

#define RME_A7A_MMU_4G_PGT_ADDR(X)    ((X)&0xFFFFC000U)
#define RME_A7A_MMU_1M_PGT_ADDR(X)    ((X)&0xFFFFFC00U)
#define RME_A7A_MMU_1M_PAGE_ADDR(X)     ((X)&0xFFF00000U)
#define RME_A7A_MMU_4K_PAGE_ADDR(X)     ((X)&0xFFFFF000U)

#define RME_A7A_PGFLG_1M_RME2NAT(X)     (RME_A7A_Pgflg_1M_RME2NAT[X])
#define RME_A7A_PGFLG_1M_PREPRC(X)     ((((X)&RME_A7A_MMU_1M_READONLY)>>12)| \
                                          (((X)&RME_A7A_MMU_1M_EXECUTENEVER)>>2)| \
		                                  (((X)&RME_A7A_MMU_1M_CACHEABLE)>>2)| \
										  (((X)&RME_A7A_MMU_1M_BUFFERABLE)>>2))
#define RME_A7A_PGFLG_1M_NAT2RME(X)     (RME_A7A_Pgflg_1M_NAT2RME[RME_A7A_PGFLG_1M_PREPRC(X)])

#define RME_A7A_PGFLG_4K_RME2NAT(X)     (RME_A7A_Pgflg_4K_RME2NAT[X])
#define RME_A7A_PGFLG_4K_PREPRC(X)     ((((X)&RME_A7A_MMU_4K_READONLY)>>6)| \
		                                  (((X)&RME_A7A_MMU_4K_CACHEABLE)>>1)| \
		                                  (((X)&RME_A7A_MMU_4K_BUFFERABLE)>>1)| \
										  (((X)&RME_A7A_MMU_4K_EXECUTENEVER)>>0))
#define RME_A7A_PGFLG_4K_NAT2RME(X)     (RME_A7A_Pgflg_4K_NAT2RME[RME_A7A_PGFLG_4K_PREPRC(X)])

/* Processor type definitions */
#define RME_A7A_CPU_CORTEX_A5           (0)
#define RME_A7A_CPU_CORTEX_A7           (1)
#define RME_A7A_CPU_CORTEX_A8           (2)
#define RME_A7A_CPU_CORTEX_A9           (3)
#define RME_A7A_CPU_CORTEX_A15          (4)
#define RME_A7A_CPU_CORTEX_A17          (5)

/* FPU type definitions */
#define RME_A7A_FPU_NONE                (0)
#define RME_A7A_FPU_VFPV3               (1)
#define RME_A7A_FPU_VFPV3U              (2)
#define RME_A7A_FPU_VFPV3_HPE           (3)
#define RME_A7A_FPU_VFPV4               (4)
#define RME_A7A_FPU_VFPV5               (5)

/* GIC type definitions */
#define RME_A7A_GIC_V1                  (0)
#define RME_A7A_GIC_V2                  (1)

/* MPU definitions */
#define RME_A7A_MPU_PRIVDEF             0x00000004

#define RME_A7A_SFR(BASE,OFFSET)        (*((volatile rme_ptr_t*)((rme_ptr_t)((BASE)+(OFFSET)))))
/* GIC definitions - This is only present in MPCore platform. Most Cortex-A9s,
 * all Cortex-A7s and Cortex-A5s are on the multicore platform */
/* Distributor control register */
#define RME_A7A_GICD_CTLR               RME_A7A_SFR(RME_A7A_GICD_BASE,0x0000)
#define RME_A7A_GICD_CTLR_GRP1EN        (1U<<1U)
#define RME_A7A_GICD_CTLR_GRP0EN        (1U<<0U)
/* Interrupt controller type register */
#define RME_A7A_GICD_TYPER              RME_A7A_SFR(RME_A7A_GICD_BASE,0x0004)
/* Distributor implementer identification register */
#define RME_A7A_GICD_IIDR               RME_A7A_SFR(RME_A7A_GICD_BASE,0x0008)
/* Interrupt group registers */
#define RME_A7A_GICD_IGROUPR(X)         RME_A7A_SFR(RME_A7A_GICD_BASE,0x0080+(X)*4) /* 0-31 */
/* Interrupt enabling registers */
#define RME_A7A_GICD_ISENABLER(X)       RME_A7A_SFR(RME_A7A_GICD_BASE,0x0100+(X)*4) /* 0-31 */
/* Interrupt disabling registers */
#define RME_A7A_GICD_ICENABLER(X)       RME_A7A_SFR(RME_A7A_GICD_BASE,0x0180+(X)*4) /* 0-31 */
/* Interrupt set-pending registers */
#define RME_A7A_GICD_ISPENDR(X)         RME_A7A_SFR(RME_A7A_GICD_BASE,0x0200+(X)*4) /* 0-31 */
/* Interrupt clear-pending registers */
#define RME_A7A_GICD_ICPENDR(X)         RME_A7A_SFR(RME_A7A_GICD_BASE,0x0280+(X)*4) /* 0-31 */
/* Interrupt set-active registers */
#define RME_A7A_GICD_ISACTIVER(X)       RME_A7A_SFR(RME_A7A_GICD_BASE,0x0300+(X)*4) /* 0-31 */
/* Interrupt clear-active registers */
#define RME_A7A_GICD_ICACTIVER(X)       RME_A7A_SFR(RME_A7A_GICD_BASE,0x0380+(X)*4) /* 0-31 */
/* Interrupt priority registers */
#define RME_A7A_GICD_IPRIORITYR(X)      RME_A7A_SFR(RME_A7A_GICD_BASE,0x0400+(X)*4) /* 0-254 */
/* Interrupt processor targets registers */
#define RME_A7A_GICD_ITARGETSR(X)       RME_A7A_SFR(RME_A7A_GICD_BASE,0x0800+(X)*4) /* 0-7 */
/* Interrupt configuration registers */
#define RME_A7A_GICD_ICFGR(X)           RME_A7A_SFR(RME_A7A_GICD_BASE,0x0C00+(X)*4) /* 0-63 */
/* Non-secure access control registers */
#define RME_A7A_GICD_NSACR(X)           RME_A7A_SFR(RME_A7A_GICD_BASE,0x0E00+(X)*4) /* 0-63 */
/* Software generated interrupt register */
#define RME_A7A_GICD_SGIR               RME_A7A_SFR(RME_A7A_GICD_BASE,0x0F00)
/* SGI clear-pending registers */
#define RME_A7A_GICD_CPENDSGIR(X)       RME_A7A_SFR(RME_A7A_GICD_BASE,0x0F10+(X)*4) /* 0-3 */
/* SGI set-pending registers */
#define RME_A7A_GICD_SPENDSGIR(X)       RME_A7A_SFR(RME_A7A_GICD_BASE,0x0F20+(X)*4) /* 0-3 */

/* CPU interface control register */
#define RME_A7A_GICC_CTLR               RME_A7A_SFR(RME_A7A_GICC_BASE,0x0000)
/* Interrupt priority mask register */
#define RME_A7A_GICC_PMR                RME_A7A_SFR(RME_A7A_GICC_BASE,0x0004)
/* Binary pointer register */
#define RME_A7A_GICC_BPR                RME_A7A_SFR(RME_A7A_GICC_BASE,0x0004)
/* Interrupt acknowledge register */
#define RME_A7A_GICC_IAR                RME_A7A_SFR(RME_A7A_GICC_BASE,0x000C)
/* End of interrupt register */
#define RME_A7A_GICC_EOIR               RME_A7A_SFR(RME_A7A_GICC_BASE,0x0010)
/* Running priority register */
#define RME_A7A_GICC_RPR                RME_A7A_SFR(RME_A7A_GICC_BASE,0x0014)
/* Highest priority pending interrupt register */
#define RME_A7A_GICC_HPPIR              RME_A7A_SFR(RME_A7A_GICC_BASE,0x0018)
/* Aliased binary point register */
#define RME_A7A_GICC_ABPR               RME_A7A_SFR(RME_A7A_GICC_BASE,0x001C)
/* Aliased interrupt acknowledge register */
#define RME_A7A_GICC_AIAR               RME_A7A_SFR(RME_A7A_GICC_BASE,0x0020)
/* Aliased end of interrupt register */
#define RME_A7A_GICC_AEOIR              RME_A7A_SFR(RME_A7A_GICC_BASE,0x0024)
/* Aliased highest priority pending interrupt register */
#define RME_A7A_GICC_AHPPIR             RME_A7A_SFR(RME_A7A_GICC_BASE,0x0028)
/* Active priorities registers */
#define RME_A7A_GICC_APR(X)             RME_A7A_SFR(RME_A7A_GICC_BASE,0x00D0+(X)*4) /* 0-3 */
/* Non-secure active priorities registers */
#define RME_A7A_GICC_NSAPR(X)           RME_A7A_SFR(RME_A7A_GICC_BASE,0x00E0+(X)*4) /* 0-3 */
/* CPU interface identification register */
#define RME_A7A_GICC_IIDR               RME_A7A_SFR(RME_A7A_GICC_BASE,0x00FC)
/* Deactivate interrupt register */
#define RME_A7A_GICC_DIR                RME_A7A_SFR(RME_A7A_GICC_BASE,0x1000)

/* Control register */
#define RME_A7A_GICC_CBPR               (1U<<4U)
#define RME_A7A_GICC_FIQEN              (1U<<3U)
#define RME_A7A_GICC_ACKCTL             (1U<<2U)
#define RME_A7A_GICC_ENABLEGRP1         (1U<<1U)
#define RME_A7A_GICC_ENABLEGRP0         (1U<<0U)

/* Priority grouping */
#define RME_A7A_GIC_GROUPING_P7S1       (0U)
#define RME_A7A_GIC_GROUPING_P6S2       (1U)
#define RME_A7A_GIC_GROUPING_P5S3       (2U)
#define RME_A7A_GIC_GROUPING_P4S4       (3U)
#define RME_A7A_GIC_GROUPING_P3S5       (4U)
#define RME_A7A_GIC_GROUPING_P2S6       (5U)
#define RME_A7A_GIC_GROUPING_P1S7       (6U)
#define RME_A7A_GIC_GROUPING_P0S8       (7U)

/* Timer definitions */
/* Private timer load register */
#define RME_A7A_PTWD_PTLR               RME_A7A_SFR(RME_A7A_PTWD_BASE,0x0000)
/* Private timer counter register */
#define RME_A7A_PTWD_PTCNTR             RME_A7A_SFR(RME_A7A_PTWD_BASE,0x0004)
/* Private timer control register */
#define RME_A7A_PTWD_PTCTLR             RME_A7A_SFR(RME_A7A_PTWD_BASE,0x0008)
#define RME_A7A_PTWD_PTCTLR_PRESC(X)    ((X)<<8U)
#define RME_A7A_PTWD_PTCTLR_IRQEN       (1U<<2U)
#define RME_A7A_PTWD_PTCTLR_AUTOREL     (1U<<1U)
#define RME_A7A_PTWD_PTCTLR_TIMEN       (1U<<0U)
/* Private timer interrupt status register */
#define RME_A7A_PTWD_PTISR              RME_A7A_SFR(RME_A7A_PTWD_BASE,0x000C)

/* Watchdog load register */
#define RME_A7A_PTWD_WDLR               RME_A7A_SFR(RME_A7A_PTWD_BASE,0x0020)
/* Watchdog counter register */
#define RME_A7A_PTWD_WDCNTR             RME_A7A_SFR(RME_A7A_PTWD_BASE,0x0024)
/* Watchdog control register */
#define RME_A7A_PTWD_WDCTLR             RME_A7A_SFR(RME_A7A_PTWD_BASE,0x0028)
/* Watchdog interrupt status register */
#define RME_A7A_PTWD_WDISR              RME_A7A_SFR(RME_A7A_PTWD_BASE,0x002C)
/* Watchdog reset status register */
#define RME_A7A_PTWD_WDRSR              RME_A7A_SFR(RME_A7A_PTWD_BASE,0x0030)
/* Watchdog disable register */
#define RME_A7A_PTWD_WDDR               RME_A7A_SFR(RME_A7A_PTWD_BASE,0x0034)

/* Fault definitions */
/* These faults cannot be recovered and will lead to termination immediately */
#define RME_A7A_FAULT_FATAL             (0)
/*****************************************************************************/
/* __RME_PLATFORM_A7A_DEF__ */
#endif
/* __HDR_DEF__ */
#endif
/* End Define ****************************************************************/

/* Struct ********************************************************************/
#ifdef __HDR_STRUCT__
#ifndef __RME_PLATFORM_A7A_STRUCT__
#define __RME_PLATFORM_A7A_STRUCT__
/* We used structs in the header */

/* Use defines in these headers */
#define __HDR_DEF__
#undef __HDR_DEF__
/*****************************************************************************/
/* The register set struct - R0-R3, R12, PC, LR, xPSR is automatically pushed.
 * Here we need LR to decide EXC_RETURN, that's why it is here */
struct RME_Reg_Struct
{
    rme_ptr_t CPSR;
    rme_ptr_t R0;
    rme_ptr_t R1;
    rme_ptr_t R2;
    rme_ptr_t R3;
    rme_ptr_t R4;
    rme_ptr_t R5;
    rme_ptr_t R6;
    rme_ptr_t R7;
    rme_ptr_t R8;
    rme_ptr_t R9;
    rme_ptr_t R10;
    rme_ptr_t R11;
    rme_ptr_t SP;
    rme_ptr_t LR;
    rme_ptr_t PC;
};

/* The coprocessor register set structure. In Cortex-M, if there is a 
 * single-precision FPU, then the FPU S0-S15 is automatically pushed */
struct RME_Cop_Struct
{
    rme_ptr_t F0;
    rme_ptr_t F1;
    rme_ptr_t F2;
    rme_ptr_t F3;
    rme_ptr_t F4;
    rme_ptr_t F5;
    rme_ptr_t F6;
    rme_ptr_t F7;
    rme_ptr_t FPS;
};

/* The registers to keep to remember where to return after an invocation */
struct RME_Iret_Struct
{
    rme_ptr_t PC;
    rme_ptr_t SP;
};
/* Exception register structure - always indicate hardfault */
struct RME_Exc_Struct
{
    rme_ptr_t Cause;
};

/* Memory information - the layout is (offset from VA base):
 * |----------16MB|-----|-----|-----|-----|
 * |Kernel&Globals|Kot|Kom1|Kom2|
 *  Kernel&Globals : Initial kernel text segment and all static variables.
 *                   Also includes per-CPU variables and all other stuff.
 *  Kot          : Kernel object registration table.
 *  Kom1          : Kernel memory 1, linear mapping, allow creation of page tables.
 *  Kom2          : Kernel memory 2, non-linear mapping, allow creation of all other stuff.
 *  All values are in bytes, and are virtual addresses. If running the kernel or creating
 *  page tables on on-chip SRAM is desired, then the VA2PA and PA2VA macros must consider
 *  that issue properly, and the Kom1 description also needs to take care of that. */
struct RME_A7A_Mem_Layout
{
	rme_ptr_t Kot_Start;
	rme_ptr_t Kot_Size;

	rme_ptr_t Pgreg_Start;
	rme_ptr_t Pgreg_Size;

	rme_ptr_t Kom1_Start;
	rme_ptr_t Kom1_Size;

	rme_ptr_t Kom2_Start;
	rme_ptr_t Kom2_Size;
};

/* Interrupt flags - this type of flags will only appear on MPU-based systems */
struct __RME_A7A_Flag_Set
{
    rme_ptr_t Lock;
    rme_ptr_t Group;
    rme_ptr_t Flags[32];
};

struct __RME_A7A_Flags
{
    struct __RME_A7A_Flag_Set Set0;
    struct __RME_A7A_Flag_Set Set1;
};

/* Page table registration table - actually we can take advantage of the fact that
 * ARM page tables never have more than 16384 childs, and if the page have a child,
 * then there must be no parent (because there are only 2 levels). Also, this can save
 * us some memory, and shred down the pgreg memory consumption to 1/256 of the total */
union __RME_A7A_Pgreg
{
	/* What is the ASID of this page table? - This can be set through kernel function caps */
	rme_ptr_t ASID_Child_Cnt;
    /* How many parent page tables does this page table have? */
	rme_ptr_t Parent_Cnt;
};
/*****************************************************************************/
/* __RME_PLATFORM_A7A_STRUCT__ */
#endif
/* __HDR_STRUCT__ */
#endif
/* End Struct ****************************************************************/

/* Private Variable **********************************************************/
#if(!(defined __HDR_DEF__||defined __HDR_STRUCT__))
#ifndef __RME_PLATFORM_A7A_MEMBER__
#define __RME_PLATFORM_A7A_MEMBER__

/* In this way we can use the data structures and definitions in the headers */
#define __HDR_DEF__

#undef __HDR_DEF__

#define __HDR_STRUCT__

#undef __HDR_STRUCT__

/* If the header is not used in the public mode */
#ifndef __HDR_PUBLIC__
/*****************************************************************************/
/* Translate the flags into Cortex-A specific ones - the STATIC bit will never be
 * set thus no need to consider about it here. The flag bits order is shown below:
 * [MSB                                                                                         LSB]
 * RME_PGT_BUFFERABLE | RME_PGT_CACHEABLE | RME_PGT_EXECUTE | RME_PGT_WRITE | RME_PGT_READ
 * The C snippet to generate this (gcc x64):

#include <stdio.h>

#define A7A_1M_PRESENT      (0x01U)
#define A7A_1M_USER         (1U<<11U)
#define A7A_1M_SHAREABLE    (1U<<16U)
#define A7A_1M_NOTGLOBAL    (1U<<17U)

#define A7A_1M_EXECUTENEVER (1U<<4U)
#define A7A_1M_READONLY     (1U<<15U)
#define A7A_1M_BUFFERABLE   (1U<<2U)
#define A7A_1M_CACHEABLE    (1U<<3U)

#define A7A_1M_COMMON       (A7A_1M_PRESENT|A7A_1M_USER|A7A_1M_SHAREABLE|A7A_1M_NOTGLOBAL)

#define RME_READ             (1<<0)
#define RME_WRITE            (1<<1)
#define RME_EXECUTE          (1<<2)
#define RME_CACHEABLE        (1<<3)
#define RME_BUFFERABLE       (1<<4)

int main(void)
{
	unsigned long result;
	int count;
	for(count=0;count<32;count++)
	{
	    result=A7A_1M_COMMON;
		if((count&RME_WRITE)==0)
			result|=A7A_1M_READONLY;
		if((count&RME_EXECUTE)==0)
			result|=A7A_1M_EXECUTENEVER;
		if((count&RME_CACHEABLE)!=0)
			result|=A7A_1M_CACHEABLE;
		if((count&RME_BUFFERABLE)!=0)
			result|=A7A_1M_BUFFERABLE;
	    printf("0x%08lX,",result);
	    if(count%4==3)
	    	printf("\n");
	}
	return 0;
} */
static const rme_ptr_t RME_A7A_Pgflg_1M_RME2NAT[32]=
{
	0x00038811,0x00038811,0x00030811,0x00030811,
	0x00038801,0x00038801,0x00030801,0x00030801,
	0x00038819,0x00038819,0x00030819,0x00030819,
	0x00038809,0x00038809,0x00030809,0x00030809,
	0x00038815,0x00038815,0x00030815,0x00030815,
	0x00038805,0x00038805,0x00030805,0x00030805,
	0x0003881D,0x0003881D,0x0003081D,0x0003081D,
	0x0003880D,0x0003880D,0x0003080D,0x0003080D
};

/* Translate the flags into Cortex-A specific ones - the STATIC bit will never be
 * set thus no need to consider about it here. The flag bits order is shown below:
 * [MSB                                                                                         LSB]
 * RME_PGT_BUFFERABLE | RME_PGT_CACHEABLE | RME_PGT_EXECUTE | RME_PGT_WRITE | RME_PGT_READ
 * The C snippet to generate this (gcc x64):

#include <stdio.h>

#define A7A_4K_PRESENT      (1U<<1U)
#define A7A_4K_USER         (1U<<5U)
#define A7A_4K_SHAREABLE    (1U<<10U)
#define A7A_4K_NOTGLOBAL    (1U<<11U)

#define A7A_4K_EXECUTENEVER (1U<<0U)
#define A7A_4K_READONLY     (1U<<9U)
#define A7A_4K_BUFFERABLE   (1U<<2U)
#define A7A_4K_CACHEABLE    (1U<<3U)

#define A7A_4K_COMMON       (A7A_4K_PRESENT|A7A_4K_USER|A7A_4K_SHAREABLE|A7A_4K_NOTGLOBAL)

#define RME_READ             (1<<0)
#define RME_WRITE            (1<<1)
#define RME_EXECUTE          (1<<2)
#define RME_CACHEABLE        (1<<3)
#define RME_BUFFERABLE       (1<<4)

int main(void)
{
	unsigned long result;
	int count;
	for(count=0;count<32;count++)
	{
	    result=A7A_4K_COMMON;
		if((count&RME_WRITE)==0)
			result|=A7A_4K_READONLY;
		if((count&RME_EXECUTE)==0)
			result|=A7A_4K_EXECUTENEVER;
		if((count&RME_CACHEABLE)!=0)
			result|=A7A_4K_CACHEABLE;
		if((count&RME_BUFFERABLE)!=0)
			result|=A7A_4K_BUFFERABLE;
	    printf("0x%08lX,",result);
	    if(count%4==3)
	    	printf("\n");
	}
	return 0;
} */
static const rme_ptr_t RME_A7A_Pgflg_4K_RME2NAT[32]=
{
	0x00000E23,0x00000E23,0x00000C23,0x00000C23,
	0x00000E22,0x00000E22,0x00000C22,0x00000C22,
	0x00000E2B,0x00000E2B,0x00000C2B,0x00000C2B,
	0x00000E2A,0x00000E2A,0x00000C2A,0x00000C2A,
	0x00000E27,0x00000E27,0x00000C27,0x00000C27,
	0x00000E26,0x00000E26,0x00000C26,0x00000C26,
	0x00000E2F,0x00000E2F,0x00000C2F,0x00000C2F,
	0x00000E2E,0x00000E2E,0x00000C2E,0x00000C2E
};

/* Translate the 1M flags back to RME format. In order to use this table, it is needed to extract the
 * Cortex-A bits: [15](READONLY) [4](EXECUTENEVER) [3](CACHEABLE) [2](BUFFERABLE). The C snippet to
 * generate this (gcc x64):

#include <stdio.h>
#define A7A_1M_READONLY     (1U<<3U)
#define A7A_1M_EXECUTENEVER (1U<<2U)
#define A7A_1M_CACHEABLE    (1U<<1U)
#define A7A_1M_BUFFERABLE   (1U<<0U)

#define RME_READ             (1U<<0U)
#define RME_WRITE            (1U<<1U)
#define RME_EXECUTE          (1U<<2U)
#define RME_CACHEABLE        (1U<<3U)
#define RME_BUFFERABLE       (1U<<4U)
#define RME_STATIC           (1U<<5U)

int main(void)
{
    unsigned long int flag;
    int count;
    for(count=0;count<16;count++)
    {
        flag=RME_READ;
        if((count&A7A_1M_EXECUTENEVER)==0)
            flag|=RME_EXECUTE;
        if((count&A7A_1M_CACHEABLE)!=0)
            flag|=RME_CACHEABLE;
        if((count&A7A_1M_BUFFERABLE)!=0)
            flag|=RME_BUFFERABLE;
        if((count&A7A_1M_READONLY)==0)
            flag|=RME_WRITE;
        printf("0x%08lX,",flag);
	    if(count%4==3)
	    	printf("\n");
    }
} */
static const rme_ptr_t RME_A7A_Pgflg_1M_NAT2RME[16]=
{
	0x00000007,0x00000017,0x0000000F,0x0000001F,
	0x00000003,0x00000013,0x0000000B,0x0000001B,
	0x00000005,0x00000015,0x0000000D,0x0000001D,
	0x00000001,0x00000011,0x00000009,0x00000019
};

/* Translate the 4K flags back to RME format. In order to use this table, it is needed to extract the
 * Cortex-A bits: [9](READONLY) [3](CACHEABLE) [2](BUFFERABLE) [0](EXECUTENEVER). The C snippet to
 * generate this (gcc x64):

#include <stdio.h>
#define A7A_4K_READONLY     (1U<<3U)
#define A7A_4K_CACHEABLE    (1U<<2U)
#define A7A_4K_BUFFERABLE   (1U<<1U)
#define A7A_4K_EXECUTENEVER (1U<<0U)

#define RME_READ             (1U<<0U)
#define RME_WRITE            (1U<<1U)
#define RME_EXECUTE          (1U<<2U)
#define RME_CACHEABLE        (1U<<3U)
#define RME_BUFFERABLE       (1U<<4U)
#define RME_STATIC           (1U<<5U)

int main(void)
{
    unsigned long int flag;
    int count;
    for(count=0;count<16;count++)
    {
        flag=RME_READ;
        if((count&A7A_4K_EXECUTENEVER)==0)
            flag|=RME_EXECUTE;
        if((count&A7A_4K_CACHEABLE)!=0)
            flag|=RME_CACHEABLE;
        if((count&A7A_4K_BUFFERABLE)!=0)
            flag|=RME_BUFFERABLE;
        if((count&A7A_4K_READONLY)==0)
            flag|=RME_WRITE;
        printf("0x%08lX,",flag);
	    if(count%4==3)
	    	printf("\n");
    }
} */
static const rme_ptr_t RME_A7A_Pgflg_4K_NAT2RME[16]=
{
	0x00000007,0x00000003,0x00000017,0x00000013,
	0x0000000F,0x0000000B,0x0000001F,0x0000001B,
	0x00000005,0x00000001,0x00000015,0x00000011,
	0x0000000D,0x00000009,0x0000001D,0x00000019
};
/*****************************************************************************/
/* End Private Variable ******************************************************/

/* Private Function **********************************************************/ 
/*****************************************************************************/
static void __RME_A7A_Int_Init(void);
static void __RME_A7A_Timer_Init(void);
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
__RME_EXTERN__ rme_ptr_t RME_A7A_Timestamp;
/* Cortex-A9 can have up to 4 cores. We hard-code it here */
__RME_EXTERN__ struct RME_CPU_Local RME_A7A_Local[4];
/* ARMv6-M use simple kernel object table */
__RME_EXTERN__ rme_ptr_t RME_A7A_Kot[RME_KOT_WORD_NUM];
/* The memory layout of this chip */
__RME_EXTERN__ const rme_ptr_t RME_A7A_Mem_Info[RME_A7A_MEM_ENTRIES];
/* The start of the contiguous stack area for all processors */
RME_EXTERN rme_ptr_t __RME_A7A_Stack_Start;
/* The CPU initialization counter */
__RME_EXTERN__ rme_ptr_t RME_A7A_CPU_Cnt;
/* The initial page table */
RME_EXTERN rme_ptr_t __RME_A7A_Kern_Pgt;
/* The interrupt vector */
RME_EXTERN rme_ptr_t __RME_A7A_Vector_Table;
/* The memory layout struct - currently not used */
/* __RME_EXTERN__ struct RME_A7A_Mem_Layout RME_A7A_Layout; */
/*****************************************************************************/

/* End Public Variable *******************************************************/

/* Public Function ***********************************************************/
/*****************************************************************************/
__RME_EXTERN__ void Test(void);
/* Cortex-A (ARMv7) register reads */
RME_EXTERN rme_ptr_t __RME_A7A_CPSR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_SPSR_Get(void);
/* C0 */
RME_EXTERN rme_ptr_t __RME_A7A_MIDR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_CTR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_TCMTR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_TLBTR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_MPIDR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_REVIDR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_ID_PFR0_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_ID_PFR1_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_ID_DFR0_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_ID_AFR0_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_ID_MMFR0_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_ID_MMFR1_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_ID_MMFR2_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_ID_MMFR3_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_ID_ISAR0_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_ID_ISAR1_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_ID_ISAR2_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_ID_ISAR3_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_ID_ISAR4_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_ID_ISAR5_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_ID_CCSIDR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_ID_CLIDR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_ID_AIDR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_ID_CSSELR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_ID_VPIDR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_ID_VMPIDR_Get(void);
/* C1 */
RME_EXTERN rme_ptr_t __RME_A7A_SCTLR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_ACTLR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_CPACR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_SCR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_SDER_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_NSACR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_HSCTLR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_HACTLR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_HCR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_HDCR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_HCPTR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_HSTR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_HACR_Get(void);
/* C2 */
RME_EXTERN rme_ptr_t __RME_A7A_TTBR0_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_TTBR1_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_TTBCR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_HTCR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_VTCR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_DACR_Get(void);
/* C5 */
RME_EXTERN rme_ptr_t __RME_A7A_DFSR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_IFSR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_ADFSR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_AIFSR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_HADFSR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_HAIFSR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_HSR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_DFAR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_IFAR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_HDFAR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_HIFAR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_HPFAR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_PAR_Get(void);
/* C10 */
RME_EXTERN rme_ptr_t __RME_A7A_TLBLR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_PRRR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_NMRR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_AMAIR0_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_AMAIR1_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_HMAIR0_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_HMAIR1_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_HAMAIR0_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_HAMAIR1_Get(void);
/* C12 */
RME_EXTERN rme_ptr_t __RME_A7A_VBAR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_MVBAR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_ISR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_HVBAR_Get(void);
/* C13 */
RME_EXTERN rme_ptr_t __RME_A7A_FCSEIDR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_CONTEXTIDR_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_TPIDRURW_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_TPIDRURO_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_TPIDRPRW_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_HTPIDR_Get(void);
/* C14 */
RME_EXTERN rme_ptr_t __RME_A7A_CNTFRQ_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_CNTKCTL_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_CNTP_TVAL_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_CNTP_CTL_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_CNTV_TVAL_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_CNTV_CTL_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_CNTHCTL_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_CNTHP_TVAL_Get(void);
RME_EXTERN rme_ptr_t __RME_A7A_CNTHP_CTL_Get(void);
/* Double words */
RME_EXTERN void __RME_A7A_CNTPCT_DW_Get(rme_ptr_t* Low, rme_ptr_t* High);
RME_EXTERN void __RME_A7A_CNTVCT_DW_Get(rme_ptr_t* Low, rme_ptr_t* High);
RME_EXTERN void __RME_A7A_CNTP_CVAL_DW_Get(rme_ptr_t* Low, rme_ptr_t* High);
RME_EXTERN void __RME_A7A_CNTV_CVAL_DW_Get(rme_ptr_t* Low, rme_ptr_t* High);
RME_EXTERN void __RME_A7A_CNTVOFF_DW_Get(rme_ptr_t* Low, rme_ptr_t* High);
RME_EXTERN void __RME_A7A_CNTHP_CVAL_DW_Get(rme_ptr_t* Low, rme_ptr_t* High);

/* Cortex-A (ARMv7) register writes */
RME_EXTERN void __RME_A7A_CPSR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_SPSR_Set(rme_ptr_t Val);
/* C0 */
RME_EXTERN void __RME_A7A_ID_CSSELR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_ID_VPIDR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_ID_VMPIDR_Set(rme_ptr_t Val);
/* C1 */
RME_EXTERN void __RME_A7A_SCTLR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_ACTLR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_CPACR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_SCR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_SDER_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_NSACR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_HSCTLR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_HACTLR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_HCR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_HDCR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_HCPTR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_HSTR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_HACR_Set(rme_ptr_t Val);
/* C2,C3 */
RME_EXTERN void __RME_A7A_TTBR0_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_TTBR1_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_TTBCR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_HTCR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_VTCR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_DACR_Set(rme_ptr_t Val);
/* C5 */
RME_EXTERN void __RME_A7A_DFSR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_IFSR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_ADFSR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_AIFSR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_HADFSR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_HAIFSR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_HSR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_DFAR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_IFAR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_HDFAR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_HIFAR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_HPFAR_Set(rme_ptr_t Val);
/* C7 */
RME_EXTERN void __RME_A7A_ICIALLUIS_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_BPIALLIS_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_PAR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_ICIALLU_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_ICIMVAU_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_CP15ISB_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_BPIALL_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_BPIMVA_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_DCIMVAC_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_DCISW_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_ATS1CPR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_ATS1CPW_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_ATS1CUR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_ATS1CUW_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_ATS12NSOPR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_ATS12NSOPW_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_ATS12NSOUR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_ATS12NSOUW_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_DCCMVAC_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_DCCSW_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_CP15DSB_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_CP15DMB_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_DCCMVAU_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_DCCIMVAC_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_DCCISW_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_ATS1HR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_ATS1HW_Set(rme_ptr_t Val);
/* C8 */
RME_EXTERN void __RME_A7A_TLBIALLIS_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_TLBIMVAIS_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_TLBIASIDIS_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_TLBIMVAAIS_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_ITLBIALL_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_ITLBIMVA_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_ITLBIASID_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_DTLBIALL_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_DTLBIMVA_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_DTLBIASID_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_TLBIALL_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_TLBIMVA_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_TLBIASID_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_TLBIMVAA_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_TLBIALLHIS_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_TLBIMVAHIS_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_TLBIALLNSNHIS_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_TLBIALLH_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_TLBIMVAH_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_TLBIALLNSNH_Set(rme_ptr_t Val);
/* C10 */
RME_EXTERN void __RME_A7A_TLBLR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_PRRR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_NMRR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_AMAIR0_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_AMAIR1_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_HMAIR0_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_HMAIR1_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_HAMAIR0_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_HAMAIR1_Set(rme_ptr_t Val);
/* C12 */
RME_EXTERN void __RME_A7A_VBAR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_MVBAR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_HVBAR_Set(rme_ptr_t Val);
/* C13 */
RME_EXTERN void __RME_A7A_CONTEXTIDR_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_TPIDRURW_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_TPIDRURO_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_TPIDRPRW_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_HTPIDR_Set(rme_ptr_t Val);
/* C14 */
RME_EXTERN void __RME_A7A_CNTFRQ_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_CNTKCTL_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_CNTP_TVAL_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_CNTP_CTL_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_CNTV_TVAL_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_CNTV_CTL_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_CNTHCTL_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_CNTHP_TVAL_Set(rme_ptr_t Val);
RME_EXTERN void __RME_A7A_CNTHP_CTL_Set(rme_ptr_t Val);
/* Double words */
RME_EXTERN void __RME_A7A_CNTP_CVAL_DW_Set(rme_ptr_t Low, rme_ptr_t High);
RME_EXTERN void __RME_A7A_CNTV_CVAL_DW_Set(rme_ptr_t Low, rme_ptr_t High);
RME_EXTERN void __RME_A7A_CNTVOFF_DW_Set(rme_ptr_t Low, rme_ptr_t High);
RME_EXTERN void __RME_A7A_CNTHP_CVAL_DW_Set(rme_ptr_t Low, rme_ptr_t High);

/* Handlers */
__RME_EXTERN__ void __RME_A7A_Undefined_Handler(struct RME_Reg_Struct* Reg);
__RME_EXTERN__ void __RME_A7A_Prefetch_Abort_Handler(struct RME_Reg_Struct* Reg);
__RME_EXTERN__ void __RME_A7A_Data_Abort_Handler(struct RME_Reg_Struct* Reg);
__RME_EXTERN__ void __RME_A7A_IRQ_Handler(struct RME_Reg_Struct* Reg);
__RME_EXTERN__ void _RME_A7A_SGI_Handler(struct RME_Reg_Struct* Reg, rme_ptr_t CPUID, rme_ptr_t Int_ID);
/* Interrupts */
RME_EXTERN void __RME_Disable_Int(void);
RME_EXTERN void __RME_Enable_Int(void);
RME_EXTERN void __RME_A7A_Wait_Int(void);
/* Atomics */
__RME_EXTERN__ rme_ptr_t __RME_A7A_Comp_Swap(rme_ptr_t* Ptr, rme_ptr_t Old, rme_ptr_t New);
__RME_EXTERN__ rme_ptr_t __RME_A7A_Fetch_Add(rme_ptr_t* Ptr, rme_cnt_t Addend);
__RME_EXTERN__ rme_ptr_t __RME_A7A_Fetch_And(rme_ptr_t* Ptr, rme_ptr_t Operand);
/* Memory barriers */
RME_EXTERN rme_ptr_t __RME_A7A_Read_Acquire(rme_ptr_t* Ptr);
RME_EXTERN void __RME_A7A_Write_Release(rme_ptr_t* Ptr, rme_ptr_t Val);
/* MSB counting */
RME_EXTERN rme_ptr_t __RME_A7A_MSB_Get(rme_ptr_t Val);
/* Debugging */
__RME_EXTERN__ rme_ptr_t __RME_Putchar(char Char);
/* Coprocessor */
RME_EXTERN void ___RME_A7A_Thd_Cop_Save(struct RME_Cop_Struct* Cop_Reg);
RME_EXTERN void ___RME_A7A_Thd_Cop_Restore(struct RME_Cop_Struct* Cop_Reg);
/* Booting */
RME_EXTERN void _RME_Kmain(rme_ptr_t Stack);
RME_EXTERN void __RME_Enter_User_Mode(rme_ptr_t Entry_Addr, rme_ptr_t Stack_Addr, rme_ptr_t CPUID);
__RME_EXTERN__ rme_ptr_t __RME_Low_Level_Init(void);
__RME_EXTERN__ rme_ptr_t __RME_Boot(void);
__RME_EXTERN__ void __RME_Reboot(void);
__RME_EXTERN__ void __RME_Shutdown(void);
/* Syscall & invocation */
__RME_EXTERN__ struct RME_CPU_Local* __RME_A7A_CPU_Local_Get(void);
__RME_EXTERN__ void __RME_Get_Syscall_Param(struct RME_Reg_Struct* Reg, rme_ptr_t* Svc,
                                         rme_ptr_t* Capid, rme_ptr_t* Param);
__RME_EXTERN__ void __RME_Set_Syscall_Retval(struct RME_Reg_Struct* Reg, rme_ret_t Retval);
/* Thread register sets */
//__RME_EXTERN__ void __RME_Thd_Reg_Init(rme_ptr_t Entry, rme_ptr_t Stack, rme_ptr_t Param, struct RME_Reg_Struct* Reg);
__RME_EXTERN__ void __RME_Thd_Reg_Init(rme_ptr_t Attr,
                                       rme_ptr_t Entry,
                                       rme_ptr_t Stack,
                                       rme_ptr_t Param,
                                       struct RME_Reg_Struct* Reg);
__RME_EXTERN__ rme_ret_t __RME_Pgt_Pgdir_Unmap(struct RME_Cap_Pgt* Pgt_Parent,
                                               rme_ptr_t Pos,
                                               struct RME_Cap_Pgt* Pgt_Child);
__RME_EXTERN__ void __RME_Thd_Reg_Copy(struct RME_Reg_Struct* Dst, struct RME_Reg_Struct* Src);
__RME_EXTERN__ void __RME_Thd_Cop_Init(struct RME_Reg_Struct* Reg, struct RME_Cop_Struct* Cop_Reg);
__RME_EXTERN__ void __RME_Thd_Cop_Save(struct RME_Reg_Struct* Reg, struct RME_Cop_Struct* Cop_Reg);
__RME_EXTERN__ void __RME_Thd_Cop_Restore(struct RME_Reg_Struct* Reg, struct RME_Cop_Struct* Cop_Reg);
/* Invocation register sets */
__RME_EXTERN__ void __RME_Inv_Reg_Save(struct RME_Iret_Struct* Ret, struct RME_Reg_Struct* Reg);
__RME_EXTERN__ void __RME_Inv_Reg_Restore(struct RME_Reg_Struct* Reg, struct RME_Iret_Struct* Ret);
__RME_EXTERN__ void __RME_Set_Inv_Retval(struct RME_Reg_Struct* Reg, rme_ret_t Retval);
/* Kernel function handler */
__RME_EXTERN__ rme_ptr_t __RME_Kern_Func_Handler(struct RME_Reg_Struct* Reg, rme_ptr_t Func_ID, 
                                             rme_ptr_t Sub_ID, rme_ptr_t Param1, rme_ptr_t Param2);
/* Fault handler */
__RME_EXTERN__ void __RME_A7A_Fault_Handler(struct RME_Reg_Struct* Reg);
/* Generic interrupt handler */
__RME_EXTERN__ void __RME_A7A_Generic_Handler(struct RME_Reg_Struct* Reg, rme_ptr_t Int_Num);
/* Page table operations */
__RME_EXTERN__ void __RME_Pgt_Set(rme_ptr_t Pgt);
__RME_EXTERN__ rme_ptr_t __RME_Pgt_Kom_Init(void);
__RME_EXTERN__ rme_ptr_t __RME_Pgt_Check(rme_ptr_t Start_Addr, rme_ptr_t Is_Top, rme_ptr_t Size_Order, rme_ptr_t Num_Order, rme_ptr_t Vaddr);
__RME_EXTERN__ rme_ptr_t __RME_Pgt_Init(struct RME_Cap_Pgt* Pgt_Op);
__RME_EXTERN__ rme_ptr_t __RME_Pgt_Del_Check(struct RME_Cap_Pgt* Pgt_Op);
__RME_EXTERN__ rme_ptr_t __RME_Pgt_Page_Map(struct RME_Cap_Pgt* Pgt_Op, rme_ptr_t Paddr, rme_ptr_t Pos, rme_ptr_t Flags);
__RME_EXTERN__ rme_ptr_t __RME_Pgt_Page_Unmap(struct RME_Cap_Pgt* Pgt_Op, rme_ptr_t Pos);
__RME_EXTERN__ rme_ptr_t __RME_Pgt_Pgdir_Map(struct RME_Cap_Pgt* Pgt_Parent, rme_ptr_t Pos, 
                                           struct RME_Cap_Pgt* Pgt_Child, rme_ptr_t Flags);
//__RME_EXTERN__ rme_ptr_t __RME_Pgt_Pgdir_Unmap(struct RME_Cap_Pgt* Pgt_Op, rme_ptr_t Pos);
__RME_EXTERN__ rme_ptr_t __RME_Pgt_Lookup(struct RME_Cap_Pgt* Pgt_Op, rme_ptr_t Pos, rme_ptr_t* Paddr, rme_ptr_t* Flags);
__RME_EXTERN__ rme_ptr_t __RME_Pgt_Walk(struct RME_Cap_Pgt* Pgt_Op, rme_ptr_t Vaddr, rme_ptr_t* Pgt,
                                      rme_ptr_t* Map_Vaddr, rme_ptr_t* Paddr, rme_ptr_t* Size_Order, rme_ptr_t* Num_Order, rme_ptr_t* Flags);
/*****************************************************************************/
/* Undefine "__RME_EXTERN__" to avoid redefinition */
#undef __RME_EXTERN__
/* __RME_PLATFORM_A7A_MEMBER__ */
#endif
/* !(defined __HDR_DEF__||defined __HDR_STRUCT__) */
#endif
/* End Public Function *******************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
