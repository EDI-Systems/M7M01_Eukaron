/******************************************************************************
Filename   : rme_platform_stm32g071rb.h
Author     : pry
Date       : 24/06/2017
Licence    : The Unlicense; see LICENSE for details.
Description: The configuration file for STM32G071RB.
******************************************************************************/

/* Define ********************************************************************/
/* Debugging *****************************************************************/
#define RME_ASSERT_ENABLE                               (1U)
#define RME_DBGLOG_ENABLE                               (1U)
/* Generator *****************************************************************/
/* Are we using the generator? */
#define RME_RVM_GEN_ENABLE                              (0U)
/* Are we using raw memory mappings? */
#define RME_PGT_RAW_ENABLE                              (0U)
/* Kernel ********************************************************************/
/* Kernel object virtual memory base */
#define RME_KOM_VA_BASE                                 (0x10002000U)
/* Kernel object virtual memory size */
#define RME_KOM_VA_SIZE                                 (0x6000U)
/* Hypervisor context virtual memory base - set to 0 if no VM */
#define RME_HYP_VA_BASE                                 (0x20000000U)
/* Hypervisor context virtual memory size - set to 0 if no VM */
#define RME_HYP_VA_SIZE                                 (0x20000U)
/* Kernel memory allocation granularity order */
#define RME_KOM_SLOT_ORDER                              (4U)
/* Kernel stack base and size */
#define RME_KSTK_VA_BASE                                (0x10000FF0U)
#define RME_KSTK_VA_SIZE                                (0x400U)
/* The maximum number of preemption priorities */
#define RME_PREEMPT_PRIO_NUM                            (32U)

/* Physical vector number, flag area base and size */
#define RME_RVM_PHYS_VCT_NUM                            (31U)
#define RME_RVM_PHYS_VCTF_BASE                          (0x20002C00U)
#define RME_RVM_PHYS_VCTF_SIZE                          (0x100U)
/* Virtual event number, flag area base and size */
#define RME_RVM_VIRT_EVT_NUM                            (10U)
#define RME_RVM_VIRT_EVTF_BASE                          (0x20002E00U)
#define RME_RVM_VIRT_EVTF_SIZE                          (0x100U)
/* Size of initial capability table */
#define RME_RVM_INIT_CPT_SIZE                           (54U)
/* Initial kernel object frontier limit */
#define RME_RVM_CPT_BOOT_FRONT                          (9U)
#define RME_RVM_KOM_BOOT_FRONT                          (0x400U)
/* Post-boot kernel object frontier limit */
#define RME_RVM_CPT_DONE_FRONT                          (18U)
#define RME_RVM_KOM_DONE_FRONT                          (0x1200U)

/* Init process's first thread's entry point address */
#define RME_A6M_INIT_ENTRY                              (0x08004001U)
/* Init process's first thread's stack address */
#define RME_A6M_INIT_STACK                              (0x1000FFF0U)
/* What is the Systick value? - 10ms per tick */
#define RME_A6M_SYSTICK_VAL                             (640000U)
/* Number of MPU regions available */
#define RME_A6M_REGION_NUM                              (8U)
/* What is the FPU type? - always a no */
#define RME_COP_NUM                                     (0U)

/* Chip specific *************************************************************/
/* Register address */
#define RME_A6M_RCC_APBENR1                             RME_A6M_REG(0x4002103CU)
#define RME_A6M_RCC_APBENR1_PWREN                       RME_POW2(28U)

#define RME_A6M_RCC_CR                                  RME_A6M_REG(0x40021000U)
#define RME_A6M_RCC_CR_HSION                            RME_POW2(8U)
#define RME_A6M_RCC_CR_HSIRDY                           RME_POW2(10U)
#define RME_A6M_RCC_CR_HSEON                            RME_POW2(16U)
#define RME_A6M_RCC_CR_HSERDY                           RME_POW2(17U)
#define RME_A6M_RCC_CR_PLLON                            RME_POW2(24U)
#define RME_A6M_RCC_CR_PLLRDY                           RME_POW2(25U)

#define RME_A6M_RCC_PLLCFGR                             RME_A6M_REG(0x4002100CU)
#define RME_A6M_RCC_PLLCFGR_PLLSRC_HSI                  (0x00000002U)
#define RME_A6M_RCC_PLLCFGR_PLLM(X)                     RME_FIELD(((X)-1U),4U)
#define RME_A6M_RCC_PLLCFGR_PLLN(X)                     RME_FIELD((X),8U)
#define RME_A6M_RCC_PLLCFGR_PLLR_DIV2                   (0x20000000U)
#define RME_A6M_RCC_PLLCFGR_PLLREN                      RME_POW2(28U)

#define RME_A6M_RCC_CFGR                                RME_A6M_REG(0x40021008U)
#define RME_A6M_RCC_CFGR_SW_PLL                         (0x00000002U)

#define RME_A6M_RCC_CIER                                RME_A6M_REG(0x40021018U)

#define RME_A6M_FLASH_ACR                               RME_A6M_REG(0x40022000U)
#define RME_A6M_FLASH_ACR_LATENCY(X)                    RME_A6M_FLASH_ACR=((RME_A6M_FLASH_ACR&~0x07U)|(X))
#define RME_A6M_FLASH_ACR_PRFTEN                        RME_POW2(8U)

#define RME_A6M_PWR_CR1                                 RME_A6M_REG(0x40007000U)
#define RME_A6M_PWR_CR1_VOS_RANGE1                      RME_POW2(9U)
#define RME_A6M_PWR_SR2                                 RME_A6M_REG(0x40007014U)
#define RME_A6M_PWR_SR2_VOSF                            RME_POW2(10U)

#define RME_A6M_RCC_IOPENR                              RME_A6M_REG(0x40021034U)
#define RME_A6M_RCC_IOPENR_IOPAEN                       RME_POW2(0U)

#define RME_A6M_RCC_APBENR1_USART2EN                    RME_POW2(17U)
#define RME_A6M_GPIOA_MODER                             RME_A6M_REG(0x50000000U)
#define RME_A6M_GPIOA_OTYPER                            RME_A6M_REG(0x50000004U)
#define RME_A6M_GPIOA_OSPEEDR                           RME_A6M_REG(0x50000008U)
#define RME_A6M_GPIOA_PUPDR                             RME_A6M_REG(0x5000000CU)
#define RME_A6M_GPIOA_IDR                               RME_A6M_REG(0x50000010U)
#define RME_A6M_GPIOA_ODR                               RME_A6M_REG(0x50000014U)
#define RME_A6M_GPIOA_BSRR                              RME_A6M_REG(0x50000018U)
#define RME_A6M_GPIOA_LCKR                              RME_A6M_REG(0x5000001CU)
#define RME_A6M_GPIOA_AFRL                              RME_A6M_REG(0x50000020U)
#define RME_A6M_GPIOA_AFRH                              RME_A6M_REG(0x50000024U)
#define RME_A6M_GPIOA_BRR                               RME_A6M_REG(0x50000028U)

#define RME_A6M_GPIO_MODE_INPUT                         (0U)
#define RME_A6M_GPIO_MODE_OUTPUT                        (1U)
#define RME_A6M_GPIO_MODE_ALTERNATE                     (2U)
#define RME_A6M_GPIO_MODE_ANALOG                        (3U)

#define RME_A6M_GPIO_OTYPE_PUSHPULL                     (0U)
#define RME_A6M_GPIO_OTYPE_OPENDRAIN                    (1U)

#define RME_A6M_GPIO_OSPEED_LOW                         (0U)
#define RME_A6M_GPIO_OSPEED_MEDIUM                      (1U)
#define RME_A6M_GPIO_OSPEED_HIGH                        (2U)
#define RME_A6M_GPIO_OSPEED_VERYHIGH                    (3U)

#define RME_A6M_GPIO_PUPD_NONE                          (0U)
#define RME_A6M_GPIO_PUPD_PULLUP                        (1U)
#define RME_A6M_GPIO_PUPD_PULLDOWN                      (2U)

#define RME_A6M_GPIO_AF1_USART2                         (0x01U)

#define RME_A6M_GPIOA_MODE(MODE, PIN)                   RME_A6M_GPIOA_MODER=(RME_A6M_GPIOA_MODER&~(0x03U<<((PIN)*2)))|((MODE)<<((PIN)*2))
#define RME_A6M_GPIOA_OTYPE(OTYPE, PIN)                 RME_A6M_GPIOA_OTYPER=(RME_A6M_GPIOA_OTYPER&~(0x01U<<(PIN)))|((OTYPE)<<(PIN))
#define RME_A6M_GPIOA_OSPEED(OSPEED, PIN)               RME_A6M_GPIOA_OSPEEDR=(RME_A6M_GPIOA_OSPEEDR&~(0x03U<<((PIN)*2)))|((OSPEED)<<((PIN)*2))
#define RME_A6M_GPIOA_PUPD(PUPD, PIN)                   RME_A6M_GPIOA_PUPDR=(RME_A6M_GPIOA_PUPDR&~(0x03U<<((PIN)*2)))|((PUPD)<<((PIN)*2))
#define RME_A6M_GPIOA_AFL(AF, PIN)                      RME_A6M_GPIOA_AFRL=(RME_A6M_GPIOA_AFRL&~(0x0FU<<((PIN)*4)))|((AF)<<((PIN)*4))
#define RME_A6M_GPIOA_AFH(AF, PIN)                      RME_A6M_GPIOA_AFRH=(RME_A6M_GPIOA_AFRH&~(0x0FU<<((PIN-8)*4)))|((AF)<<((PIN-8)*4))

#define RME_A6M_USART2_CR1                              RME_A6M_REG(0x40004400U)
#define RME_A6M_USART2_CR2                              RME_A6M_REG(0x40004404U)
#define RME_A6M_USART2_CR3                              RME_A6M_REG(0x40004408U)
#define RME_A6M_USART2_BRR                              RME_A6M_REG(0x4000440CU)
#define RME_A6M_USART2_ISR                              RME_A6M_REG(0x4000441CU)
#define RME_A6M_USART2_TDR                              RME_A6M_REG(0x40004428U)

#define RME_A6M_USART2_CR1_UE                           RME_POW2(0U)

/* Preinitialization of critical hardware */
#define RME_A6M_LOWLVL_PREINIT() \
do \
{ \
    /* Set HSION bit */ \
    RME_A6M_RCC_CR|=0x00000100U; \
    /* Reset CFGR register */ \
    RME_A6M_RCC_CFGR=0x00000000U; \
    /* Reset HSEON, CSSHSEON and PLLON bits */ \
    RME_A6M_RCC_CR&=0xFEF6FFFFU; \
    /* Reset HSEBYP bit */ \
    RME_A6M_RCC_CR&=0xFFFBFFFFU; \
    /* Disable all interrupts */ \
    RME_A6M_RCC_CIER=0x00000000U; \
    /* Vector table address */ \
    RME_A6M_SCB_VTOR=0x08000000U; \
} \
while(0)

#define RME_A6M_LOWLVL_INIT_COMMON() \
do \
{ \
    /* Set the clock tree in the system */ \
    /* Enable power regulator clock, and configure voltage scaling function */ \
    RME_A6M_RCC_APBENR1|=RME_A6M_RCC_APBENR1_PWREN; \
    __RME_A6M_Barrier(); \
    RME_A6M_PWR_CR1|=RME_A6M_PWR_CR1_VOS_RANGE1; \
    __RME_A6M_Barrier(); \
    /* Wait for HSI16 to stabilize - it is always on after reset */ \
    while((RME_A6M_RCC_CR&RME_A6M_RCC_CR_HSIRDY)==0U); \
    /* Fpll=Fin/PLLM*PLLN/PLLR; 16MHz/1*8/2 = 64MHz from HSI16 */ \
    RME_A6M_RCC_CR&=~RME_A6M_RCC_CR_PLLON; \
    __RME_A6M_Barrier(); \
    while((RME_A6M_RCC_CR&RME_A6M_RCC_CR_PLLRDY)!=0U); \
    RME_A6M_RCC_PLLCFGR=RME_A6M_RCC_PLLCFGR_PLLSRC_HSI| \
                        RME_A6M_RCC_PLLCFGR_PLLM(1U)| \
                        RME_A6M_RCC_PLLCFGR_PLLN(8U)| \
                        RME_A6M_RCC_PLLCFGR_PLLR_DIV2| \
                        RME_A6M_RCC_PLLCFGR_PLLREN; \
    __RME_A6M_Barrier(); \
    RME_A6M_RCC_CR|=RME_A6M_RCC_CR_PLLON; \
    __RME_A6M_Barrier(); \
    while((RME_A6M_RCC_CR&RME_A6M_RCC_CR_PLLRDY)==0U); \
    \
    /* Flash latency and prefetch enabling */ \
    RME_A6M_FLASH_ACR=RME_A6M_FLASH_ACR_LATENCY(2U)|RME_A6M_FLASH_ACR_PRFTEN; \
    __RME_A6M_Barrier(); \
    RME_A6M_RCC_CFGR|=RME_A6M_RCC_CFGR_SW_PLL; \
    __RME_A6M_Barrier(); \
} \
while(0)

#if(RME_DBGLOG_ENABLE!=0U)
/* Other low-level initialization stuff - clock and serial */
#define RME_A6M_LOWLVL_INIT() \
do \
{ \
    RME_A6M_LOWLVL_INIT_COMMON(); \
    \
    /* Enable USART 2 for user-level operations - PA2 is the VCP TX on NUCLEO-G071RB */ \
    /* UART IO initialization */ \
    RME_A6M_RCC_IOPENR|=RME_A6M_RCC_IOPENR_IOPAEN; \
    RME_A6M_GPIOA_MODE(RME_A6M_GPIO_MODE_ALTERNATE, 2U); \
    RME_A6M_GPIOA_OTYPE(RME_A6M_GPIO_OTYPE_PUSHPULL, 2U); \
    RME_A6M_GPIOA_OSPEED(RME_A6M_GPIO_OSPEED_HIGH, 2U); \
    RME_A6M_GPIOA_PUPD(RME_A6M_GPIO_PUPD_PULLUP, 2U); \
    RME_A6M_GPIOA_AFL(RME_A6M_GPIO_AF1_USART2, 2U); \
    \
    /* UART initialization - 115200 baud at PCLK1=64MHz */ \
    RME_A6M_RCC_APBENR1|=RME_A6M_RCC_APBENR1_USART2EN; \
    RME_A6M_USART2_CR1=0x08U; \
    RME_A6M_USART2_CR2=0x00U; \
    RME_A6M_USART2_CR3=0x00U; \
    RME_A6M_USART2_BRR=0x22CU; \
    RME_A6M_USART2_CR1|=RME_A6M_USART2_CR1_UE; \
} \
while(0)

/* This is for debugging output */
#define RME_A6M_PUTCHAR(CHAR) \
do \
{ \
    RME_A6M_USART2_TDR=(rme_ptr_t)(CHAR); \
    while((RME_A6M_USART2_ISR&0x80U)==0U); \
} \
while(0)

#else

/* Other low-level initialization stuff - clock and serial */
#define RME_A6M_LOWLVL_INIT() \
do \
{ \
    RME_A6M_LOWLVL_INIT_COMMON(); \
} \
while(0)
#endif

/* Prefetcher state set and get */
#define RME_A6M_PRFTH_STATE_SET(STATE) \
do \
{ \
    if((STATE)!=0U) \
        RME_A6M_FLASH_ACR|=RME_A6M_FLASH_ACR_PRFTEN; \
    else \
        RME_A6M_FLASH_ACR&=~RME_A6M_FLASH_ACR_PRFTEN; \
} \
while(0)
    
#define RME_A6M_PRFTH_STATE_GET() ((RME_A6M_FLASH_ACR&RME_A6M_FLASH_ACR_PRFTEN)!=0U)

/* Action before placing processor in low-power mode */
#define RME_A6M_WAIT_INT_PRE()
/* Action after placing processor in low-power mode */
#define RME_A6M_WAIT_INT_POST()
/* End Define ****************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/