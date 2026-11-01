/******************************************************************************
Filename    : rme_platform_qemu_virt.h
Author      : yhy
Date        : 29/09/2026
Licence     : The Unlicense; see LICENSE for details.
Description : The configuration file for the QEMU virt platform.
******************************************************************************/

/* Define ********************************************************************/
/* Debugging *****************************************************************/
#define RME_ASSERT_ENABLE                               (1U)
#define RME_DBGLOG_ENABLE                               (1U)
/* Are we using raw memory mappings? */
#define RME_PGT_RAW_ENABLE                              (0U)
/* Kernel ********************************************************************/
/* Number of harts to bring up. The boot hart is always CPU0 and the
 * remaining harts are started through the SBI HSM extension. */
#define RME_RV64V_CPU_NUM                               (8U)
/* Kernel object virtual memory base - kernel lives in the Sv39 high half:
 * VA = 0xFFFFFFC000000000 + physical 0x81000000 (see RME_RV64V_VA_BASE) */
#define RME_KOM_VA_BASE                                 (0xFFFFFFC081000000UL)
/* Kernel object virtual memory size */
#define RME_KOM_VA_SIZE                                 (0x1F000000U)
/* Hypervisor context virtual memory base - set to 0 if no VM */
#define RME_HYP_VA_BASE                                 (0x0U)
/* Hypervisor context virtual memory size - set to 0 if no VM */
#define RME_HYP_VA_SIZE                                 (0xFFFFFFFFU)
/* Kernel memory allocation granularity order */
#define RME_KOM_SLOT_ORDER                              (6U)
/* The maximum number of preemption priorities */
#define RME_PREEMPT_PRIO_NUM                            (64U)

/* Bootstrap / direct-map window *********************************************/
/* Physical base of the single 1GB window that maps the whole kernel working
 * set (image, KOM, stacks, init program). This is also the identity/high-half
 * leaf in the _start trampoline and the kernel leaf in every top-level page
 * table (__RME_Pgt_Init). QEMU virt RAM is 0x80000000..0xC0000000. */
#define RME_RV64V_KPGT_PA                               (0x80000000U)

/* Init process's first thread - user virtual addresses, decoupled from the
 * physical pages, matching X64 (first user program at 0x20000000). The code
 * is loaded physically at 0x81030000; each CPU's user stack is a 64KB block
 * in the kernel .bss mapped below 0x20200000 (see RME_RV64V_INIT_STACK). */
#define RME_RV64V_INIT_ENTRY                            (0x20000000U)
/* Physical backing of the init image (QEMU loader) */
#define RME_RV64V_INIT_PA                               (0x81030000U)
/* What is the Systick value? - 10ms per tick*/
#define RME_RV64V_OSTIM_VAL                             (100000U)
/* What is the FPU type? QEMU virt is rv64imafdch, so enable F and D. */
#define RME_COP_NUM                                     (1U)
#define RME_RV64V_COP_RVF                               (1U)
#define RME_RV64V_COP_RVD                               (1U)

/* Reboot system - SBI system reset (SRST extension) */
#define RME_RV64V_REBOOT() \
do \
{ \
    ___RME_RV64V_Sbi_Reset(); \
} \
while(0)

/* Preinitialization of critical hardware */
#define RME_RV64V_LOWLVL_PREINIT() \
do \
{ \
} \
while(0)

/* Low-level initialization - arm the SBI timer */
#define RME_RV64V_LOWLVL_INIT() \
do \
{ \
    /* Enable the S-mode timer interrupt (sie.STIE) */ \
    ___RME_RV64V_Sie_Timer_Enable(); \
    /* Arm the first timer event via OpenSBI - no auto-reload, re-armed on \
     * every tick inside the timer handler */ \
    ___RME_RV64V_Sbi_Timer_Set(___RME_RV64V_Time_Get()+RME_RV64V_OSTIM_VAL); \
} \
while(0)

#if(RME_DBGLOG_ENABLE!=0U)
/* Debugging output - SBI console putchar */
#define RME_RV64V_PUTCHAR(CHAR) \
do \
{ \
    ___RME_RV64V_Sbi_Console_Putchar((rme_ptr_t)(CHAR)); \
} \
while(0)

#else
#define RME_RV64V_PUTCHAR(CHAR)
#endif

/* Action before placing processor in low-power mode */
#define RME_RV64V_WAIT_INT_PRE()
/* Action after placing processor in low-power mode */
#define RME_RV64V_WAIT_INT_POST()
/* End Define ****************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/

