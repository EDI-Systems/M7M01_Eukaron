/******************************************************************************
Filename    : rme_platform_spacemit_k3_rt24.h
Author      : yhy
Date        : 12/11/2026
Licence     : The Unlicense; see LICENSE for details.
Description : The configuration file for the SpacemiT K3 RT24 platform
              (Banana Pi BPI-SM10 / SpacemiT K3 SoC, RT24 core0).
******************************************************************************/

/* Define ********************************************************************/
/* Debugging *****************************************************************/
#define RME_ASSERT_ENABLE                               (1U)
#define RME_DBGLOG_ENABLE                               (1U)
/* Are we using raw memory mappings? */
#define RME_PGT_RAW_ENABLE                              (0U)

/* Kernel ********************************************************************/
/* Number of harts to bring up. This configuration is single-core: RT24 core0
 * is the only CPU and no secondary hart is started. */
#define RME_RV64V_CPU_NUM                               (1U)
/* Kernel object virtual memory base - the kernel lives in the Sv39 high
 * half: VA = RME_RV64V_VA_BASE + physical. The AP loader places the kernel
 * at PA 0x110000000; the M-mode firmware enters it in S-mode with satp=0.
 *   - KOM       : PA 0x111000000 -> VA 0xFFFFFFC111000000
 *   - init image: PA 0x110400000 -> VA 0xFFFFFFC110400000 (embedded)
 * The linker keeps the kernel .bss (KOT bitmap and per-CPU stacks) below
 * the embedded init image. All backing memory fits in one 1GB window. */
#define RME_KOM_VA_BASE                                 (0xFFFFFFC111000000UL)
/* Kernel object virtual memory size */
#define RME_KOM_VA_SIZE                                 (0x01000000U)
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
 * table (__RME_Pgt_Init). The window is [0x100000000, 0x140000000); it is a
 * mapping window, not a declaration that all of its memory is allocatable. */
#define RME_RV64V_KPGT_PA                               (0x100000000UL)
/* Bootstrap root storage in shared SRAM, outside the log ring. RME builds
 * the root and enables Sv39 in S-mode; neither firmware supplies a table. */
#define RME_RV64V_BOOT_PGT_PA                           (0xC0802000UL)

/* Init process's first thread - user virtual addresses, decoupled from the
 * physical pages, matching X64 (first user program at 0x20000000). The code
 * is embedded in the kernel image at physical 0x110400000; each CPU's user
 * stack is a 64KB block in the kernel .bss mapped below 0x20200000 (see
 * RME_RV64V_INIT_STACK). */
#define RME_RV64V_INIT_ENTRY                            (0x20000000UL)
/* Physical backing of the embedded init image. The image is placed 4MB above
 * the kernel load address so the kernel .bss (KOT bitmap + per-CPU
 * kernel/user stacks) has enough room. */
#define RME_RV64V_INIT_PA                               (0x110400000UL)

/* What is the Systick value? RT24 SysTimer ticks at 24MHz; the M-mode
 * firmware emulates the time CSR from mtime. 10ms per tick = 240000. */
#define RME_RV64V_OSTIM_VAL                             (240000U)

/* Keep the common cache-block alignment; this does not enable Zicbom. */
#define RME_RV64V_CACHE_BLOCK_SIZE                      (64U)
/* RT24 core0 traps on Zicbom clean/inval/flush in both M-mode and S-mode.
 * These D-cache KFNs fail; other vendor maintenance mechanisms are unverified. */
#define RME_RV64V_ZICBOM_ENABLE                         (0U)

/* What is the FPU type? RT24 is RV64GC - it has both F and D, so
 * enable the double-precision context. */
#define RME_COP_NUM                                     (1U)
#define RME_RV64V_COP_RVF                               (1U)
#define RME_RV64V_COP_RVD                               (1U)

/* Reboot system - issue SBI system reset. The current RT24 M-mode firmware
 * does not implement SRST and returns NOT_SUPPORTED; no reset is performed. */
#define RME_RV64V_REBOOT() \
do \
{ \
    (void)___RME_RV64V_Sbi_Reset(); \
} \
while(0)

/* Preinitialization of critical hardware - nothing needed here: the AP
 * loader and M-mode firmware prepare the hardware before entering RME. */
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
    /* Arm the first timer event via the M-mode firmware - no auto-reload, \
     * re-armed on every tick inside the timer handler */ \
    (void)___RME_RV64V_Sbi_Timer_Set(___RME_RV64V_Time_Get()+RME_RV64V_OSTIM_VAL); \
} \
while(0)

#if(RME_DBGLOG_ENABLE!=0U)
/* Debugging output - SBI DBCN console write byte. The M-mode firmware
 * publishes characters to shared SRAM and the AP relays them to UART0. */
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
