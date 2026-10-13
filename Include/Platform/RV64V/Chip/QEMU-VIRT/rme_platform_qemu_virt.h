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
/* Kernel object virtual memory base - kernel lives in the Sv39 high half:
 * VA = 0xFFFFFFC000000000 + physical 0x81000000 (see RME_RV64V_VA_BASE) */
#define RME_KOM_VA_BASE                                 (0xFFFFFFC081000000ULL)
/* Kernel object virtual memory size */
#define RME_KOM_VA_SIZE                                 (0x1F000000U)
/* Hypervisor context virtual memory base - set to 0 if no VM */
#define RME_HYP_VA_BASE                                 (0x0U)
/* Hypervisor context virtual memory size - set to 0 if no VM */
#define RME_HYP_VA_SIZE                                 (0xFFFFFFFFU)
/* Kernel memory allocation granularity order */
#define RME_KOM_SLOT_ORDER                              (6U)
/* Kernel stack size and address - high half as well */
#define RME_KSTK_VA_BASE                                (0xFFFFFFC08100E000ULL)
#define RME_KSTK_VA_SIZE                                (0x800U)
/* The maximum number of preemption priorities */
#define RME_PREEMPT_PRIO_NUM                            (64U)

/* Physical vector number */
#define RME_RVM_PHYS_VCT_NUM                            (104U)
/* Size of initial capability table */
#define RME_RVM_INIT_CPT_SIZE                           (54U)
/* Initial kernel object frontier limit. Must hold the capability table, the
 * three Sv39 page tables (12KB) and the first thread. */
#define RME_RVM_KOM_BOOT_FRONT                          (0x8000U)

/* Init process's first thread - user virtual addresses, decoupled from the
 * physical pages, matching X64 (first user program at 0x20000000 with its
 * stack about 2MB above it). The code is loaded physically at 0x81030000 and
 * the stack physically occupies [0x81010000,0x81020000). */
#define RME_RV64V_INIT_ENTRY                            (0x20000000U)
#define RME_RV64V_INIT_STACK                            (0x20200000U)
/* What is the Systick value? - 10ms per tick*/
#define RME_RV64V_OSTIM_VAL                             (100000U)
/* What is the FPU type? */
#define RME_COP_NUM                                     (1U)
#define RME_RV64V_COP_RVF                               (1U)
#define RME_RV64V_COP_RVD                               (0U)

/* Chip specific *************************************************************/
/* Timer interrupt SCAUSE value */
#define RME_RV64V_SCAUSE_TIM                            (0x8000000000000005U)

/* Register address */
/* RCC CTLR */
#define RME_RV64V_RCC_CTLR                              RME_RV64V_REG(0x40021000U)
/* RCC control */
#define RME_RV64V_RCC_CTLR_HSION                        (0x00000001U)
#define RME_RV64V_RCC_CTLR_HSIRDY                       (0x00000002U)
#define RME_RV64V_RCC_CTLR_HSITRIM                      (0x000000F8U)
#define RME_RV64V_RCC_CTLR_HSICAL                       (0x0000FF00U)
#define RME_RV64V_RCC_CTLR_HSEON                        (0x00010000U)
#define RME_RV64V_RCC_CTLR_HSERDY                       (0x00020000U)
#define RME_RV64V_RCC_CTLR_HSEBYP                       (0x00040000U)
#define RME_RV64V_RCC_CTLR_CSSON                        (0x00080000U)
#define RME_RV64V_RCC_CTLR_PLLON                        (0x01000000U)
#define RME_RV64V_RCC_CTLR_PLLRDY                       (0x02000000U)

/* RCC CFGR */
#define RME_RV64V_RCC_CFGR0                             RME_RV64V_REG(0x40021004U)
#define RME_RV64V_RCC_CFGR2                             RME_RV64V_REG(0x4002102CU)
/* HCLK divisions */
#define RME_RV64V_RCC_CFGR0_HPRE_DIV1                   (0x00000000U)
#define RME_RV64V_RCC_CFGR0_HPRE_DIV2                   (0x00000080U)
#define RME_RV64V_RCC_CFGR0_HPRE_DIV4                   (0x00000090U)
#define RME_RV64V_RCC_CFGR0_HPRE_DIV8                   (0x000000A0U)
#define RME_RV64V_RCC_CFGR0_HPRE_DIV16                  (0x000000B0U)
#define RME_RV64V_RCC_CFGR0_HPRE_DIV64                  (0x000000C0U)
#define RME_RV64V_RCC_CFGR0_HPRE_DIV128                 (0x000000D0U)
#define RME_RV64V_RCC_CFGR0_HPRE_DIV256                 (0x000000E0U)
#define RME_RV64V_RCC_CFGR0_HPRE_DIV512                 (0x000000F0U)
/* PCLK1 divisions */
#define RME_RV64V_RCC_CFGR0_PPRE1_DIV1                  (0x00000000U)
#define RME_RV64V_RCC_CFGR0_PPRE1_DIV2                  (0x00000400U)
#define RME_RV64V_RCC_CFGR0_PPRE1_DIV4                  (0x00000500U)
#define RME_RV64V_RCC_CFGR0_PPRE1_DIV8                  (0x00000600U)
#define RME_RV64V_RCC_CFGR0_PPRE1_DIV16                 (0x00000700U)
/* PCLK2 divisions */
#define RME_RV64V_RCC_CFGR0_PPRE2_DIV1                  (0x00000000U)
#define RME_RV64V_RCC_CFGR0_PPRE2_DIV2                  (0x00002000U)
#define RME_RV64V_RCC_CFGR0_PPRE2_DIV4                  (0x00002800U)
#define RME_RV64V_RCC_CFGR0_PPRE2_DIV8                  (0x00003000U)
#define RME_RV64V_RCC_CFGR0_PPRE2_DIV16                 (0x00003800U)
/* PLL sources */
#define RME_RV64V_RCC_CFGR0_PLLSRC_HSE                  (0x00010000U)
#define RME_RV64V_RCC_CFGR0_PLLXTPRE_HSE                (0x00000000U)
/* PLL multiplier */
#define RME_RV64V_RCC_CFGR0_PLLMULL3_EXTEN              (0x00040000U)
#define RME_RV64V_RCC_CFGR0_PLLMULL4_EXTEN              (0x00080000U)
#define RME_RV64V_RCC_CFGR0_PLLMULL5_EXTEN              (0x000C0000U)
#define RME_RV64V_RCC_CFGR0_PLLMULL6_EXTEN              (0x00100000U)
#define RME_RV64V_RCC_CFGR0_PLLMULL6_5_EXTEN            (0x00340000U)
#define RME_RV64V_RCC_CFGR0_PLLMULL7_EXTEN              (0x00140000U)
#define RME_RV64V_RCC_CFGR0_PLLMULL8_EXTEN              (0x00180000U)
#define RME_RV64V_RCC_CFGR0_PLLMULL9_EXTEN              (0x001C0000U)
#define RME_RV64V_RCC_CFGR0_PLLMULL10_EXTEN             (0x00200000U)
#define RME_RV64V_RCC_CFGR0_PLLMULL11_EXTEN             (0x00240000U)
#define RME_RV64V_RCC_CFGR0_PLLMULL12_EXTEN             (0x00280000U)
#define RME_RV64V_RCC_CFGR0_PLLMULL13_EXTEN             (0x002C0000U)
#define RME_RV64V_RCC_CFGR0_PLLMULL14_EXTEN             (0x00300000U)
#define RME_RV64V_RCC_CFGR0_PLLMULL15_EXTEN             (0x00380000U)
#define RME_RV64V_RCC_CFGR0_PLLMULL16_EXTEN             (0x003C0000U)
#define RME_RV64V_RCC_CFGR0_PLLMULL18_EXTEN             (0x00000000U)
/* Clock source select */
#define RME_RV64V_RCC_CFGR0_SW                          (0x00000003U)
#define RME_RV64V_RCC_CFGR0_SW_CLR                      (0xFFFFFFFCU)
#define RME_RV64V_RCC_CFGR0_SW_HSI                      (0x00000000U)
#define RME_RV64V_RCC_CFGR0_SW_HSE                      (0x00000001U)
#define RME_RV64V_RCC_CFGR0_SW_PLL                      (0x00000002U)
/* Current clock source */
#define RME_RV64V_RCC_CFGR0_SWS                         (0x0000000CU)
#define RME_RV64V_RCC_CFGR0_SWS_HSI                     (0x00000000U)
#define RME_RV64V_RCC_CFGR0_SWS_HSE                     (0x00000004U)
#define RME_RV64V_RCC_CFGR0_SWS_PLL                     (0x00000008U)

/* RCC INTR */
#define RME_RV64V_RCC_INTR                              RME_RV64V_REG(0x40021008U)

/* RCC APB2PCENR */
#define RME_RV64V_RCC_APB2PCENR                         RME_RV64V_REG(0x40021018U)
#define RME_RV64V_RCC_APB2PCENR_GPIOA                   (0x00000004U)
#define RME_RV64V_RCC_APB2PCENR_USART1                  (0x00004000U)

/* PFIC - This is different from ARMv7-M in the sense that the exceptions
 * are also configured by these registers, thus all interrupt numbers are
 * shifted by 16 when compared with ARMv7-M. WCH wrote the manual that
 * way, and to avoid confusions, we'll align with what the manual says. */
#define RME_RV64V_PFIC_ISR(X)                           RME_RV64V_REG(0xE000E000U+(((X)>>5)<<2))
#define RME_RV64V_PFIC_CFGR                             RME_RV64V_REG(0xE000E048U)
#define RME_RV64V_PFIC_IENR(X)                          RME_RV64V_REG(0xE000E100U+(((X)>>5)<<2))
#define RME_RV64V_PFIC_IRER(X)                          RME_RV64V_REG(0xE000E180U+(((X)>>5)<<2))
#define RME_RV64V_PFIC_IPSR(X)                          RME_RV64V_REG(0xE000E200U+(((X)>>5)<<2))
#define RME_RV64V_PFIC_IPRIOR(X)                        RME_RV64V_REGB(0xE000E400U+(X))
#define RME_RV64V_PFIC_SCTLR                            RME_RV64V_REG(0xE000ED10U)
#define RME_RV64V_PFIC_SCTLR_SYSRST                     RME_POW2(31U)
#define RME_RV64V_PFIC_SCTLR_SEVONPEND                  RME_POW2(4U)
#define RME_RV64V_PFIC_SCTLR_WFITOWFE                   RME_POW2(3U)
#define RME_RV64V_EXC_IRQN                              (3U)
#define RME_RV64V_ECALL_M_MODE_IRQN                     (5U)
#define RME_RV64V_ECALL_U_MODE_IRQN                     (8U)
#define RME_RV64V_BREAK_POINT_IRQN                      (9U)
#define RME_RV64V_SYSTICK_IRQN                          (12)
#define RME_RV64V_SOFTWARE_IRQN                         (14)

/* SysTick */
#define RME_RV64V_SYSTICK_CTLR                          RME_RV64V_REG(0xE000F000U)
#define RME_RV64V_SYSTICK_CMPHR                         RME_RV64V_REG(0xE000F014U)
#define RME_RV64V_SYSTICK_CMPLR                         RME_RV64V_REG(0xE000F010U)

/* GPIO & USART */
#define RME_RV64V_GPIOA_CFGHR                           RME_RV64V_REG(0x40010804U)
#define RME_RV64V_USART1_BRR                            RME_RV64V_REG(0x40013808U)
#define RME_RV64V_USART1_CTLR1                          RME_RV64V_REG(0x4001380CU)
#define RME_RV64V_USART1_CTLR1_TE                       (0x00000008U)
#define RME_RV64V_USART1_CTLR1_UE                       (0x00002000U)
#define RME_RV64V_USART1_CTLR2                          RME_RV64V_REG(0x40013810U)
#define RME_RV64V_USART1_CTLR3                          RME_RV64V_REG(0x40013814U)
#define RME_RV64V_USART1_STATR                          RME_RV64V_REG(0x40013800U)
#define RME_RV64V_USART1_STATR_TC                       (0x00000040U)
#define RME_RV64V_USART1_DATAR                          RME_RV64V_REG(0x40013804U)

/* Flash */
#define RME_RV64V_FLASH_CTLR                            RME_RV64V_REG(0x40022010U)
#define RME_RV64V_FLASH_CTLR_RSENACT                    (0x00400000U)
#define RME_RV64V_FLASH_CTLR_EHMOD                      (0x01000000U)

/* Get interrupt state */
#define RME_RV64V_INT_STATE_GET(INT_NUM)                ((RME_RV64V_PFIC_ISR(INT_NUM)& \
                                                          RME_POW2((INT_NUM)&0x1FU))!=0U)

/* Enable interrupt */
#define RME_RV64V_INT_STATE_ENABLE(INT_NUM) \
do \
{ \
    RME_RV64V_PFIC_IENR(INT_NUM)=RME_POW2((INT_NUM)&0x1FU); \
} \
while(0)

/* Disable interrupt */
#define RME_RV64V_INT_STATE_DISABLE(INT_NUM) \
do \
{ \
    RME_RV64V_PFIC_IRER(INT_NUM)=RME_POW2((INT_NUM)&0x1FU); \
} \
while(0)

/* Get interrupt priority */
#define RME_RV64V_INT_PRIO_GET(INT_NUM) \
do \
{ \
    return RME_RV64V_PFIC_IPRIOR(INT_NUM); \
} \
while(0)

/* Set interrupt priority */
#define RME_RV64V_INT_PRIO_SET(INT_NUM, PRIO) \
do \
{ \
    RME_RV64V_PFIC_IPRIOR(INT_NUM)=(PRIO); \
} \
while(0)

/* Local interrupt triggering */
#define RME_RV64V_INT_LOCAL_TRIG(INT_NUM) \
do \
{ \
    RME_RV64V_PFIC_IPSR(INT_NUM)=RME_POW2((INT_NUM)&0x1FU); \
} \
while(0)

/* Cache mode configuration */
#define RME_RV64V_CACHE_MOD(CACHE_ID,OPERATION,PARAM)
/* Cache maintenance */
#define RME_RV64V_CACHE_MAINT(CACHE_ID,OPERATION,PARAM)
/* Prefetcher mode ocnfiguration */
#define RME_RV64V_PRFTH_MOD(PRFTH_ID,OPERATION,PARAM)

/* Performance monitor configuration */
#define RME_RV64V_PERF_MON_MOD(PERF_ID,OPERATION,PARAM)
/* Performance monitor cycle counter read or write */
#define RME_RV64V_PERF_CYCLE_MOD(CYCLE_ID,OPERATION,VALUE)

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

/* Prefetcher state set and get */
#define RME_RV64V_PRFTH_STATE_SET(STATE) \
do \
{ \
    if((STATE)!=0U) \
    { \
        RME_RV64V_FLASH_CTLR|=RME_RV64V_FLASH_CTLR_EHMOD; \
    } \
    else \
    { \
        RME_RV64V_FLASH_CTLR&=~RME_RV64V_FLASH_CTLR_EHMOD; \
        RME_RV64V_FLASH_CTLR|=RME_RV64V_FLASH_CTLR_RSENACT; \
    } \
} \
while(0)

#define RME_RV64V_PRFTH_STATE_GET()                    ((RME_RV64V_FLASH_CTLR& \
                                                         RME_RV64V_FLASH_CTLR_EHMOD)!=0U)

/* Action before placing processor in low-power mode */
#define RME_RV64V_WAIT_INT_PRE()
/* Action after placing processor in low-power mode */
#define RME_RV64V_WAIT_INT_POST()
/* End Define ****************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/

