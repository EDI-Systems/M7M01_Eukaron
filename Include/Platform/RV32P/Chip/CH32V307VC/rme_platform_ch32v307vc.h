/******************************************************************************
Filename   : rme_platform_ch32v307vc.h
Author     : pry
Date       : 24/06/2017
Licence    : The Unlicense; see LICENSE for details.
Description: The configuration file for CH32V307VC.
******************************************************************************/

/* Define ********************************************************************/
/* Debugging *****************************************************************/
#define RME_ASSERT_CORRECT                              (0U)
#define RME_DEBUG_PRINT                                 (1U)
/* Generator *****************************************************************/
/* Are we using the generator? */
#define RME_RVM_GEN_ENABLE                              (0U)
/* Modifiable ****************************************************************/
/* The virtual memory start address for the kernel objects */
#define RME_KOM_VA_BASE                                 (0x20003000)
/* The size of the kernel object virtual memory */
#define RME_KOM_VA_SIZE                                 (0xD000)
/* The virtual memory start address for the virtual machines - If no VM is used, set to 0 */
#define RME_HYP_VA_BASE                                 (0x0U)
/* The size of the hypervisor reserved virtual memory */
#define RME_HYP_VA_SIZE                                 (0xFFFFFFFFU)
/* The granularity of kernel memory allocation, in bytes */
#define RME_KOM_SLOT_ORDER                              (4U)
/* Kernel stack size and address */
#define RME_KSTK_VA_BASE                                (0x20000FF0U)
#define RME_KSTK_VA_SIZE                                (0x800U)
/* The maximum number of preemption priority levels in the system.
 * This parameter must be divisible by the word length - 32 is usually sufficient */
#define RME_PREEMPT_PRIO_NUM                            (32U)

/* Physical vector number, flag area base and its size */
#define RME_RVM_PHYS_VCT_NUM                            (104U)
#define RME_RVM_PHYS_VCTF_BASE                          (0x20002FC0U)
#define RME_RVM_PHYS_VCTF_SIZE                          (0x40U)
/* Virtual event number, flag area base and its size */
#define RME_RVM_VIRT_EVT_NUM                            (32U)
#define RME_RVM_VIRT_EVTF_BASE                          (0x20002FA0U)
#define RME_RVM_VIRT_EVTF_SIZE                          (0x20U)
/* Size of initial capability table */
#define RME_RVM_INIT_CPT_SIZE                           (54U)
/* Initial kernel object frontier limit */
#define RME_RVM_CPT_BOOT_FRONT                          (9U)
#define RME_RVM_KOM_BOOT_FRONT                          (0x1000U)
/* Post-boot kernel object frontier limit */
#define RME_RVM_CPT_DONE_FRONT                          (22U)
#define RME_RVM_KOM_DONE_FRONT                          (0x1C10U)

/* Init process's first thread's entry point address */
#define RME_RV32P_INIT_ENTRY                            (0x00000000U)
/* Init process's first thread's stack address */
#define RME_RV32P_INIT_STACK                            (0x2001FFF0U)
/* What is the Systick value? - 10ms per tick*/
#define RME_RV32P_OSTIM_VAL                             (1440000U)
/* Number of MPU regions available */
#define RME_RV32P_REGION_NUM                            (4U)
/* What is the FPU type? */
#define RME_COP_NUM                                     (1U)
#define RME_RV32P_COP_RVF                               (1U)
#define RME_RV32P_COP_RVD                               (0U)

/* Timer interrupt MCAUSE value */
#define RME_RV32P_MCAUSE_TIM                            (0x8000000CU)

/* RCC CTLR */
#define RME_RV32P_RCC_CTLR                              RME_RV32P_REG(0x40021000U)
/* RCC control */
#define RME_RV32P_RCC_CTLR_HSION                        (0x00000001U)
#define RME_RV32P_RCC_CTLR_HSIRDY                       (0x00000002U)
#define RME_RV32P_RCC_CTLR_HSITRIM                      (0x000000F8U)
#define RME_RV32P_RCC_CTLR_HSICAL                       (0x0000FF00U)
#define RME_RV32P_RCC_CTLR_HSEON                        (0x00010000U)
#define RME_RV32P_RCC_CTLR_HSERDY                       (0x00020000U)
#define RME_RV32P_RCC_CTLR_HSEBYP                       (0x00040000U)
#define RME_RV32P_RCC_CTLR_CSSON                        (0x00080000U)
#define RME_RV32P_RCC_CTLR_PLLON                        (0x01000000U)
#define RME_RV32P_RCC_CTLR_PLLRDY                       (0x02000000U)

/* RCC CFGR */
#define RME_RV32P_RCC_CFGR0                             RME_RV32P_REG(0x40021004U)
#define RME_RV32P_RCC_CFGR2                             RME_RV32P_REG(0x4002102CU)
/* HCLK divisions */
#define RME_RV32P_RCC_CFGR0_HPRE_DIV1                   (0x00000000U)
#define RME_RV32P_RCC_CFGR0_HPRE_DIV2                   (0x00000080U)
#define RME_RV32P_RCC_CFGR0_HPRE_DIV4                   (0x00000090U)
#define RME_RV32P_RCC_CFGR0_HPRE_DIV8                   (0x000000A0U)
#define RME_RV32P_RCC_CFGR0_HPRE_DIV16                  (0x000000B0U)
#define RME_RV32P_RCC_CFGR0_HPRE_DIV64                  (0x000000C0U)
#define RME_RV32P_RCC_CFGR0_HPRE_DIV128                 (0x000000D0U)
#define RME_RV32P_RCC_CFGR0_HPRE_DIV256                 (0x000000E0U)
#define RME_RV32P_RCC_CFGR0_HPRE_DIV512                 (0x000000F0U)
/* PCLK1 divisions */
#define RME_RV32P_RCC_CFGR0_PPRE1_DIV1                  (0x00000000U)
#define RME_RV32P_RCC_CFGR0_PPRE1_DIV2                  (0x00000400U)
#define RME_RV32P_RCC_CFGR0_PPRE1_DIV4                  (0x00000500U)
#define RME_RV32P_RCC_CFGR0_PPRE1_DIV8                  (0x00000600U)
#define RME_RV32P_RCC_CFGR0_PPRE1_DIV16                 (0x00000700U)
/* PCLK2 divisions */
#define RME_RV32P_RCC_CFGR0_PPRE2_DIV1                  (0x00000000U)
#define RME_RV32P_RCC_CFGR0_PPRE2_DIV2                  (0x00002000U)
#define RME_RV32P_RCC_CFGR0_PPRE2_DIV4                  (0x00002800U)
#define RME_RV32P_RCC_CFGR0_PPRE2_DIV8                  (0x00003000U)
#define RME_RV32P_RCC_CFGR0_PPRE2_DIV16                 (0x00003800U)
/* PLL sources */
#define RME_RV32P_RCC_CFGR0_PLLSRC_HSE                  (0x00010000U)
#define RME_RV32P_RCC_CFGR0_PLLXTPRE_HSE                (0x00000000U)
/* PLL multiplier */
#define RME_RV32P_RCC_CFGR0_PLLMULL3_EXTEN              (0x00040000U)
#define RME_RV32P_RCC_CFGR0_PLLMULL4_EXTEN              (0x00080000U)
#define RME_RV32P_RCC_CFGR0_PLLMULL5_EXTEN              (0x000C0000U)
#define RME_RV32P_RCC_CFGR0_PLLMULL6_EXTEN              (0x00100000U)
#define RME_RV32P_RCC_CFGR0_PLLMULL6_5_EXTEN            (0x00340000U)
#define RME_RV32P_RCC_CFGR0_PLLMULL7_EXTEN              (0x00140000U)
#define RME_RV32P_RCC_CFGR0_PLLMULL8_EXTEN              (0x00180000U)
#define RME_RV32P_RCC_CFGR0_PLLMULL9_EXTEN              (0x001C0000U)
#define RME_RV32P_RCC_CFGR0_PLLMULL10_EXTEN             (0x00200000U)
#define RME_RV32P_RCC_CFGR0_PLLMULL11_EXTEN             (0x00240000U)
#define RME_RV32P_RCC_CFGR0_PLLMULL12_EXTEN             (0x00280000U)
#define RME_RV32P_RCC_CFGR0_PLLMULL13_EXTEN             (0x002C0000U)
#define RME_RV32P_RCC_CFGR0_PLLMULL14_EXTEN             (0x00300000U)
#define RME_RV32P_RCC_CFGR0_PLLMULL15_EXTEN             (0x00380000U)
#define RME_RV32P_RCC_CFGR0_PLLMULL16_EXTEN             (0x003C0000U)
#define RME_RV32P_RCC_CFGR0_PLLMULL18_EXTEN             (0x00000000U)
/* Clock source select */
#define RME_RV32P_RCC_CFGR0_SW                          (0x00000003U)
#define RME_RV32P_RCC_CFGR0_SW_CLR                      (0xFFFFFFFCU)
#define RME_RV32P_RCC_CFGR0_SW_HSI                      (0x00000000U)
#define RME_RV32P_RCC_CFGR0_SW_HSE                      (0x00000001U)
#define RME_RV32P_RCC_CFGR0_SW_PLL                      (0x00000002U)
/* Current clock source */
#define RME_RV32P_RCC_CFGR0_SWS                         (0x0000000CU)
#define RME_RV32P_RCC_CFGR0_SWS_HSI                     (0x00000000U)
#define RME_RV32P_RCC_CFGR0_SWS_HSE                     (0x00000004U)
#define RME_RV32P_RCC_CFGR0_SWS_PLL                     (0x00000008U)

/* RCC INTR */
#define RME_RV32P_RCC_INTR                              RME_RV32P_REG(0x40021008U)

/* RCC APB2PCENR */
#define RME_RV32P_RCC_APB2PCENR                         RME_RV32P_REG(0x40021018U)
#define RME_RV32P_RCC_APB2PCENR_GPIOA                   (0x00000004U)
#define RME_RV32P_RCC_APB2PCENR_USART1                  (0x00004000U)

/* PFIC */
#define RME_RV32P_PFIC_ISR(X)                           RME_RV32P_REG(0xE000E000U+(X))
#define RME_RV32P_PFIC_CFGR                             RME_RV32P_REG(0xE000E048U)
#define RME_RV32P_PFIC_IENR(X)                          RME_RV32P_REG(0xE000E100U+(X))
#define RME_RV32P_PFIC_IRER(X)                          RME_RV32P_REG(0xE000E180U+(X))
#define RME_RV32P_PFIC_IPSR(X)                          RME_RV32P_REG(0xE000E200U+(X))
#define RME_RV32P_PFIC_IPRIOR(X)                        RME_RV32P_REGB(0xE000E400U+(X))
#define RME_RV32P_PFIC_SCTLR                            RME_RV32P_REG(0xE000ED10U)
#define RME_RV32P_PFIC_SCTLR_SYSRST                     (0x80000000U)

/* SysTick */
#define RME_RV32P_SYSTICK_CTLR                          RME_RV32P_REG(0xE000F000U)
#define RME_RV32P_SYSTICK_CMPHR                         RME_RV32P_REG(0xE000F014U)
#define RME_RV32P_SYSTICK_CMPLR                         RME_RV32P_REG(0xE000F010U)

/* GPIO & USART */
#define RME_RV32P_GPIOA_CFGHR                           RME_RV32P_REG(0x40010804U)
#define RME_RV32P_USART1_BRR                            RME_RV32P_REG(0x40013808U)
#define RME_RV32P_USART1_CTLR1                          RME_RV32P_REG(0x4001380CU)
#define RME_RV32P_USART1_CTLR1_TE                       (0x00000008U)
#define RME_RV32P_USART1_CTLR1_UE                       (0x00002000U)
#define RME_RV32P_USART1_CTLR2                          RME_RV32P_REG(0x40013810U)
#define RME_RV32P_USART1_CTLR3                          RME_RV32P_REG(0x40013814U)
#define RME_RV32P_USART1_STATR                          RME_RV32P_REG(0x40013800U)
#define RME_RV32P_USART1_STATR_TC                       (0x00000040U)
#define RME_RV32P_USART1_DATAR                          RME_RV32P_REG(0x40013804U)

/* Flash */
#define RME_RV32P_FLASH_CTLR                            RME_RV32P_REG(0x40022010U)
#define RME_RV32P_FLASH_CTLR_RSENACT                    (0x00400000U)
#define RME_RV32P_FLASH_CTLR_EHMOD                      (0x01000000U)

/* Wait for interrupt and place processor in low-power mode */
#define RME_RV32P_WAIT_INT()

/* Get interrupt state */
#define RME_RV32P_INT_STATE_GET(INT_NUM)                ((RME_RV32P_PFIC_ISR((INT_NUM)>>5U)& \
                                                          RME_POW2((INT_NUM)&0x1FU))!=0U)

/* Enable interrupt */
#define RME_RV32P_INT_STATE_ENABLE(INT_NUM) \
do \
{ \
    RME_RV32P_PFIC_IENR((INT_NUM)>>5U)|=RME_POW2((INT_NUM)&0x1FU); \
} \
while(0)

/* Disable interrupt */
#define RME_RV32P_INT_STATE_DISABLE(INT_NUM) \
do \
{ \
    RME_RV32P_PFIC_IRER((INT_NUM)>>5U)|=RME_POW2((INT_NUM)&0x1FU); \
} \
while(0)

/* Get interrupt priority */
#define RME_RV32P_INT_PRIO_GET(INT_NUM) \
do \
{ \
    return RME_RV32P_PFIC_IPRIOR(INT_NUM); \
} \
while(0)

/* Set interrupt priority */
#define RME_RV32P_INT_PRIO_SET(INT_NUM, PRIO) \
do \
{ \
    RME_RV32P_PFIC_IPRIOR(INT_NUM)=(PRIO); \
} \
while(0)

/* Local interrupt triggering */
#define RME_RV32P_INT_LOCAL_TRIG(INT_NUM) \
do \
{ \
    RME_RV32P_PFIC_IPSR(INT_NUM)|=RME_POW2((INT_NUM)&0x1FU); \
} \
while(0)

/* Cache mode configuration */
#define RME_RV32P_CACHE_MOD(CACHE_ID, OPERATION, PARAM)
/* Cache maintenance */
#define RME_RV32P_CACHE_MAINT(CACHE_ID, OPERATION, PARAM)
/* Prefetcher mode ocnfiguration */
#define RME_RV32P_PRFTH_MOD(PRFTH_ID, OPERATION, PARAM)

/* Performance monitor configuration */
#define RME_RV32P_PERF_MON_MOD(PERF_ID, OPERATION, PARAM)
/* Performance monitor cycle counter read or write */
#define RME_RV32P_PERF_CYCLE_MOD(CYCLE_ID, OPERATION, VALUE)

/* Reboot system */
#define RME_RV32P_REBOOT() \
do \
{ \
    RME_RV32P_PFIC_SCTLR=RME_RV32P_PFIC_SCTLR_SYSRST; \
} \
while(0)

/* Preinitialization of critical hardware */
#define RME_RV32P_LOWLVL_PREINIT() \
do \
{ \
} \
while(0)

#define RME_RV32P_LOWLVL_INIT_COMMON() \
do \
{ \
    /* 8MHz internal OSC on, all other clock off */ \
    RME_RV32P_RCC_CTLR=RME_RV32P_RCC_CTLR_HSION; \
    /* Default values for all PLL */ \
    RME_RV32P_RCC_CFGR0=0U; \
    /* Clear all clock ready bits */ \
    RME_RV32P_RCC_INTR=0x00FF0000U; \
    /* Turn off all USB PLL */ \
    RME_RV32P_RCC_CFGR2=0x00000000U; \
    /* Turn on HSE and wait for it */ \
    RME_RV32P_RCC_CTLR|=RME_RV32P_RCC_CTLR_HSEON; \
    while((RME_RV32P_RCC_CTLR&RME_RV32P_RCC_CTLR_HSERDY)==0U); \
    /* HCLK = 2xPCLK1 = PCLK2 = SYSCLK */ \
    RME_RV32P_RCC_CFGR0|=RME_RV32P_RCC_CFGR0_HPRE_DIV1| \
                          RME_RV32P_RCC_CFGR0_PPRE1_DIV2| \
                          RME_RV32P_RCC_CFGR0_PPRE2_DIV1; \
    /* PLL clock source = external crystal, not divided */ \
    RME_RV32P_RCC_CFGR0|=RME_RV32P_RCC_CFGR0_PLLSRC_HSE| \
                          RME_RV32P_RCC_CFGR0_PLLXTPRE_HSE| \
                          RME_RV32P_RCC_CFGR0_PLLMULL18_EXTEN; \
    /* Enable PLL and wait for it */ \
    RME_RV32P_RCC_CTLR|=RME_RV32P_RCC_CTLR_PLLON; \
    while((RME_RV32P_RCC_CTLR&RME_RV32P_RCC_CTLR_PLLRDY)==0U); \
    /* Select PLL as system clock source (was HSI) */ \
    RME_RV32P_RCC_CFGR0&=RME_RV32P_RCC_CFGR0_SW_CLR; \
    RME_RV32P_RCC_CFGR0|=RME_RV32P_RCC_CFGR0_SW_PLL; \
    /* Wait till PLL is used as system clock source */ \
    while((RME_RV32P_RCC_CFGR0&RME_RV32P_RCC_CFGR0_SWS)!=RME_RV32P_RCC_CFGR0_SWS_PLL); \
    /* Set up and enable SysTick */ \
    __RME_Int_Disable(); \
    RME_RV32P_SYSTICK_CMPHR=0U; \
    RME_RV32P_SYSTICK_CMPLR=RME_RV32P_OSTIM_VAL; \
    RME_RV32P_SYSTICK_CTLR=0x3FU; \
} \
while(0)

#if(RME_DEBUG_PRINT==1U)
/* Other low-level initialization stuff - clock and serial */
#define RME_RV32P_LOWLVL_INIT() \
do \
{ \
    RME_RV32P_LOWLVL_INIT_COMMON(); \
    \
    /* Enable GPIOA */ \
    RME_RV32P_RCC_APB2PCENR|=RME_RV32P_RCC_APB2PCENR_GPIOA; \
    RME_RV32P_GPIOA_CFGHR=0x488444B4U; \
    /* Enable USART1 */ \
    RME_RV32P_RCC_APB2PCENR|=RME_RV32P_RCC_APB2PCENR_USART1; \
    RME_RV32P_USART1_BRR=0x04E2U; \
    RME_RV32P_USART1_CTLR1=RME_RV32P_USART1_CTLR1_TE; \
    RME_RV32P_USART1_CTLR2=0U; \
    RME_RV32P_USART1_CTLR3=0U; \
    RME_RV32P_USART1_CTLR1|=RME_RV32P_USART1_CTLR1_UE; \
} \
while(0)

/* This is for debugging output */
#define RME_RV32P_PUTCHAR(CHAR) \
do \
{ \
    while((RME_RV32P_USART1_STATR&RME_RV32P_USART1_STATR_TC)==0U); \
    RME_RV32P_USART1_DATAR=(rme_ptr_t)(CHAR); \
} \
while(0)

#else

/* Other low-level initialization stuff - clock and serial */
#define RME_RV32P_LOWLVL_INIT() \
do \
{ \
    RME_RV32P_LOWLVL_INIT_COMMON(); \
} \
while(0)
#endif

/* Prefetcher state set and get */
#define RME_RV32P_PRFTH_STATE_SET(STATE) \
do \
{ \
    if((STATE)!=0U) \
    { \
        RME_RV32P_FLASH_CTLR|=RME_RV32P_FLASH_CTLR_EHMOD; \
    } \
    else \
    { \
        RME_RV32P_FLASH_CTLR&=~RME_RV32P_FLASH_CTLR_EHMOD; \
        RME_RV32P_FLASH_CTLR|=RME_RV32P_FLASH_CTLR_RSENACT; \
    } \
} \
while(0)

#define RME_RV32P_PRFTH_STATE_GET()                    ((RME_RV32P_FLASH_CTLR& \
                                                         RME_RV32P_FLASH_CTLR_EHMOD)!=0U)
/* End Define ****************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/

