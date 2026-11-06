/******************************************************************************
Filename    : rme_platform_spacemit_k3.h
Author      : yhy
Date        : 20/10/2026
Licence     : The Unlicense; see LICENSE for details.
Description : The configuration file for the SpacemiT K3 platform
              (Banana Pi BPI-SM10 / SpacemiT K3 SoC).
******************************************************************************/

/* Define ********************************************************************/
/* Debugging *****************************************************************/
#define RME_ASSERT_ENABLE                               (1U)
#define RME_DBGLOG_ENABLE                               (1U)
/* Are we using raw memory mappings? */
#define RME_PGT_RAW_ENABLE                              (0U)

/* Kernel ********************************************************************/
/* Number of harts to bring up. This configuration is single-core: the boot
 * hart is the only CPU and no secondary hart is started. */
#define RME_RV64V_CPU_NUM                               (1U)
/* Kernel object virtual memory base - the kernel lives in the Sv39 high
 * half: VA = RME_RV64V_VA_BASE + physical. K3 DRAM starts at 0x102000000;
 * U-Boot enters the kernel in S-mode at 0x102200000; we keep everything
 * below one 1GB window so a single direct-map leaf suffices.
 *   - KOM      : PA 0x103000000 -> VA 0xFFFFFFC103000000
 *   - init image: PA 0x102600000 -> VA 0xFFFFFFC102600000 (embedded)
 * NOTE: the kernel .bss holds the KOM bitmap (RME_RV64V_Kot), the per-CPU
 * kernel stacks and the per-CPU user stacks (the latter are mapped into
 * the user window [0x20180000, 0x20200000) below), and ends around
 * PA 0x102302000, so the embedded init image must stay above that. */
#define RME_KOM_VA_BASE                                 (0xFFFFFFC103000000UL)
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
 * table (__RME_Pgt_Init). K3 DRAM is 0x102000000 and below 0x140000000, so
 * the window is [0x100000000, 0x140000000). */
#define RME_RV64V_KPGT_PA                               (0x100000000U)

/* Init process's first thread - user virtual addresses, decoupled from the
 * physical pages, matching X64 (first user program at 0x20000000). The code
 * is embedded in the kernel image at physical 0x102600000; each CPU's user
 * stack is a 64KB block in the kernel .bss mapped below 0x20200000 (see
 * RME_RV64V_INIT_STACK). */
#define RME_RV64V_INIT_ENTRY                            (0x20000000U)
/* Physical backing of the embedded init image. The image is placed 4MB above
 * the kernel load address so the kernel .bss (KOT bitmap + per-CPU
 * kernel/user stacks) has enough room. */
#define RME_RV64V_INIT_PA                               (0x102600000U)

/* What is the Systick value? The K3 time CSR ticks at 24MHz (DTB
 * timebase-frequency = 0x16e3600); 10ms per tick = 240000. */
#define RME_RV64V_OSTIM_VAL                             (240000U)

/* What is the FPU type? K3 is rv64imafdcvh - it has both F and D, so
 * enable the double-precision context. */
#define RME_COP_NUM                                     (1U)
#define RME_RV64V_COP_RVF                               (1U)
#define RME_RV64V_COP_RVD                               (1U)

/* Reboot system - SBI system reset (SRST extension), the K3 OpenSBI
 * implements it (Linux log: "SBI SRST extension detected"). */
#define RME_RV64V_REBOOT() \
do \
{ \
    (void)___RME_RV64V_Sbi_Reset(); \
} \
while(0)

/* Preinitialization of critical hardware - nothing needed on the K3, the
 * SBI services (console/timer/reset) are ready as soon as OpenSBI hands off. */
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
    (void)___RME_RV64V_Sbi_Timer_Set(___RME_RV64V_Time_Get()+RME_RV64V_OSTIM_VAL); \
} \
while(0)

#if(RME_DBGLOG_ENABLE!=0U)
/* Debugging output - SBI DBCN console write byte (the platform firmware is
 * required to provide the DBCN extension) */
#define RME_RV64V_PUTCHAR(CHAR) \
do \
{ \
    (void)___RME_RV64V_Sbi_Console_Putchar((rme_ptr_t)(CHAR)); \
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
