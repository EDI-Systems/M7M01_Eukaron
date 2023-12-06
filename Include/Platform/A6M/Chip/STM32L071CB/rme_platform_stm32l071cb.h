/******************************************************************************
Filename   : rme_platform_stm32l071cb.h
Author     : pry
Date       : 24/06/2017
Licence    : The Unlicense; see LICENSE for details.
Description: The configuration file for STM32L071CB.
******************************************************************************/

/* Define ********************************************************************/
/* Debugging *****************************************************************/
#define RME_ASSERT_CORRECT                              (0U)
#define RME_DEBUG_PRINT                                 (1U)
/* Generator *****************************************************************/
/* Are we using the generator? */
#define RME_RVM_GEN_ENABLE                              (0U)
/* Modifiable ****************************************************************/
/* Are we assuming user-managed raw memory access control? */
#define RME_PGT_RAW_USER                                (0U)
/* The virtual memory start address for the kernel objects */
#define RME_KOM_VA_BASE                                 (0x10002000U)
/* The size of the kernel object virtual memory */
#define RME_KOM_VA_SIZE                                 (0x6000U)
/* The virtual memory start address for the virtual machines - If no VM is used, set to 0 */
#define RME_HYP_VA_BASE                                 (0x20000000U)
/* The size of the hypervisor reserved virtual memory */
#define RME_HYP_VA_SIZE                                 (0x20000U)
/* The granularity of kernel memory allocation, in bytes */
#define RME_KOM_SLOT_ORDER                              (4U)
/* Kernel stack size and address */
#define RME_KSTK_VA_BASE                                (0x10000FF0U)
#define RME_KSTK_VA_SIZE                                (0x400U)
/* The maximum number of preemption priority levels in the system.
 * This parameter must be divisible by the word length - 32 is usually sufficient */
#define RME_PREEMPT_PRIO_NUM                            (32U)

/* Physical vector number, flag area base and its size */
#define RME_RVM_PHYS_VCT_NUM                            (32U)
#define RME_RVM_PHYS_VCTF_BASE                          (0x20002C00U)
#define RME_RVM_PHYS_VCTF_SIZE                          (0x100U)
/* Virtual event number, flag area base and its size */
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
#define RME_A6M_SYSTICK_VAL                             (320000U)
/* Number of MPU regions available */
#define RME_A6M_REGION_NUM                              (8U)
/* What is the FPU type? - always a no */
#define RME_COP_NUM                                     (0U)

/* Initialization registers **************************************************/
#define RME_A6M_RCC_APB1ENR                             RME_A6M_REG(0x40021038U)
#define RME_A6M_RCC_APB1ENR_PWREN                       (1U<<28)

#define RME_A6M_RCC_CR                                  RME_A6M_REG(0x40021000U)
#define RME_A6M_RCC_CR_HSEON                            (1U<<16)
#define RME_A6M_RCC_CR_HSERDY                           (1U<<17)
#define RME_A6M_RCC_CR_PLLON                            (1U<<24)
#define RME_A6M_RCC_CR_PLLRDY                           (1U<<25)

#define RME_A6M_RCC_CFGR                                RME_A6M_REG(0x4002100CU)
#define RME_A6M_RCC_CFGR_SOURCE_HSE                     (1U<<16)
#define RME_A6M_RCC_CFGR_SW_PLL                         (3U)

#define RME_A6M_RCC_CFGR_PLLMUL3                        (0x00000000U)
#define RME_A6M_RCC_CFGR_PLLMUL4                        (0x00040000U)
#define RME_A6M_RCC_CFGR_PLLMUL6                        (0x00080000U)
#define RME_A6M_RCC_CFGR_PLLMUL8                        (0x000C0000U)
#define RME_A6M_RCC_CFGR_PLLMUL12                       (0x00100000U)
#define RME_A6M_RCC_CFGR_PLLMUL16                       (0x00140000U)
#define RME_A6M_RCC_CFGR_PLLMUL24                       (0x00180000U)
#define RME_A6M_RCC_CFGR_PLLMUL32                       (0x001C0000U)
#define RME_A6M_RCC_CFGR_PLLMUL48                       (0x00200000U)

#define RME_A6M_RCC_CFGR_PLLDIV2                        (0x00400000U)
#define RME_A6M_RCC_CFGR_PLLDIV3                        (0x00800000U)
#define RME_A6M_RCC_CFGR_PLLDIV4                        (0x00C00000U)

#define RME_A6M_RCC_CFGR_HPRE_DIV1                      (0x00000000U)
#define RME_A6M_RCC_CFGR_HPRE_DIV2                      (0x00000080U)
#define RME_A6M_RCC_CFGR_HPRE_DIV4                      (0x00000090U)
#define RME_A6M_RCC_CFGR_HPRE_DIV8                      (0x000000A0U)
#define RME_A6M_RCC_CFGR_HPRE_DIV16                     (0x000000B0U)
#define RME_A6M_RCC_CFGR_HPRE_DIV64                     (0x000000C0U)
#define RME_A6M_RCC_CFGR_HPRE_DIV128                    (0x000000D0U)
#define RME_A6M_RCC_CFGR_HPRE_DIV256                    (0x000000E0U)
#define RME_A6M_RCC_CFGR_HPRE_DIV512                    (0x000000F0U)

#define RME_A6M_RCC_CFGR_PPRE1_DIV1                     (0x00000000U)
#define RME_A6M_RCC_CFGR_PPRE1_DIV2                     (0x00000400U)
#define RME_A6M_RCC_CFGR_PPRE1_DIV4                     (0x00000500U)
#define RME_A6M_RCC_CFGR_PPRE1_DIV8                     (0x00000600U)
#define RME_A6M_RCC_CFGR_PPRE1_DIV16                    (0x00000700U)

#define RME_A6M_RCC_CFGR_PPRE2_DIV1                     (0x00000000U)
#define RME_A6M_RCC_CFGR_PPRE2_DIV2                     (0x00002000U)
#define RME_A6M_RCC_CFGR_PPRE2_DIV4                     (0x00002800U)
#define RME_A6M_RCC_CFGR_PPRE2_DIV8                     (0x00003000U)
#define RME_A6M_RCC_CFGR_PPRE2_DIV16                    (0x00003800U)    

#define RME_A6M_RCC_CIER                                RME_A6M_REG(0x40021010U)

#define RME_A6M_FLASH_ACR                               RME_A6M_REG(0x40022000U)
#define RME_A6M_FLASH_ACR_LATENCY                       (1U)
#define RME_A6M_FLASH_ACR_PRFTEN                        (1U<<1)

#define RME_A6M_PWR_CR                                  RME_A6M_REG(0x40007000U)
#define RME_A6M_PWR_CR_VOS0                             (1U<<11)

#define RME_A6M_RCC_IOPENR                              RME_A6M_REG(0x4002102CU)
#define RME_A6M_RCC_IOPENR_IOPAEN                       (1U<<0)

#define RME_A6M_RCC_APB2ENR                             RME_A6M_REG(0x40021034U)
#define RME_A6M_RCC_APB2ENR_USART1EN                    (1U<<14)

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

#define RME_A6M_GPIO_AF4_USART1                         (0x04U)

#define RME_A6M_GPIOA_MODE(MODE, PIN)                   RME_A6M_GPIOA_MODER=(RME_A6M_GPIOA_MODER&~(0x03U<<((PIN)*2)))|((MODE)<<((PIN)*2))
#define RME_A6M_GPIOA_OTYPE(OTYPE, PIN)                 RME_A6M_GPIOA_OTYPER=(RME_A6M_GPIOA_OTYPER&~(0x01U<<(PIN)))|((OTYPE)<<(PIN))
#define RME_A6M_GPIOA_OSPEED(OSPEED, PIN)               RME_A6M_GPIOA_OSPEEDR=(RME_A6M_GPIOA_OSPEEDR&~(0x03U<<((PIN)*2)))|((OSPEED)<<((PIN)*2))
#define RME_A6M_GPIOA_PUPD(PUPD, PIN)                   RME_A6M_GPIOA_PUPDR=(RME_A6M_GPIOA_PUPDR&~(0x03U<<((PIN)*2)))|((PUPD)<<((PIN)*2))
#define RME_A6M_GPIOA_AFL(AF, PIN)                      RME_A6M_GPIOA_AFRL=(RME_A6M_GPIOA_AFRL&~(0x0FU<<((PIN)*4)))|((AF)<<((PIN)*4))
#define RME_A6M_GPIOA_AFH(AF, PIN)                      RME_A6M_GPIOA_AFRH=(RME_A6M_GPIOA_AFRH&~(0x0FU<<((PIN-8)*4)))|((AF)<<((PIN-8)*4))

#define RME_A6M_USART1_ISR                              RME_A6M_REG(0x4001381CU)
#define RME_A6M_USART1_TDR                              RME_A6M_REG(0x40013828U)
#define RME_A6M_USART1_BRR                              RME_A6M_REG(0x4001380CU)
#define RME_A6M_USART1_CR1                              RME_A6M_REG(0x40013800U)
#define RME_A6M_USART1_CR2                              RME_A6M_REG(0x40013804U)
#define RME_A6M_USART1_CR3                              RME_A6M_REG(0x40013808U)

#define RME_A6M_USART1_CR1_UE                           (1U<<0)

/* Preinitialization of critical hardware */
#define RME_A6M_LOWLVL_PREINIT() \
do \
{ \
    /* Set HSION bit */ \
    RME_A6M_RCC_CR|=0x00000001U; \
    /* Reset CFGR register */ \
    RME_A6M_RCC_CFGR=0x00000000U; \
    /* Reset HSEON, CSSON and PLLON bits */ \
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
    /* Enble power regulator clock, and configure voltage scaling function */ \
    RME_A6M_RCC_APB1ENR|=RME_A6M_RCC_APB1ENR_PWREN; \
    __RME_A6M_Barrier(); \
    RME_A6M_PWR_CR=RME_A6M_PWR_CR_VOS0; \
    __RME_A6M_Barrier(); \
    /* Initialize the oscillator */ \
    RME_A6M_RCC_CR|=RME_A6M_RCC_CR_HSEON; \
    __RME_A6M_Barrier(); \
    while((RME_A6M_RCC_CR&RME_A6M_RCC_CR_HSERDY)==0U); \
    /* Fpll=Fin*PLLMUL/PLLDIV */ \
    RME_A6M_RCC_CR&=~RME_A6M_RCC_CR_PLLON; \
    __RME_A6M_Barrier(); \
    while((RME_A6M_RCC_CR&RME_A6M_RCC_CR_PLLRDY)!=0U); \
    RME_A6M_RCC_CFGR=RME_A6M_RCC_CFGR_SOURCE_HSE| \
                     RME_A6M_RCC_CFGR_PLLMUL8| \
                     RME_A6M_RCC_CFGR_PLLDIV2| \
                     RME_A6M_RCC_CFGR_HPRE_DIV1| \
                     RME_A6M_RCC_CFGR_PPRE1_DIV1| \
                     RME_A6M_RCC_CFGR_PPRE2_DIV1; \
    __RME_A6M_Barrier(); \
    RME_A6M_RCC_CR|=RME_A6M_RCC_CR_PLLON; \
    __RME_A6M_Barrier(); \
    while((RME_A6M_RCC_CR&RME_A6M_RCC_CR_PLLRDY)==0U); \
    \
    /* Flash ART enabling */ \
    RME_A6M_FLASH_ACR=RME_A6M_FLASH_ACR_LATENCY|RME_A6M_FLASH_ACR_PRFTEN; \
    __RME_A6M_Barrier(); \
    RME_A6M_RCC_CFGR|=RME_A6M_RCC_CFGR_SW_PLL; \
    __RME_A6M_Barrier(); \
} \
while(0)

#if(RME_DEBUG_PRINT==1U)
/* Other low-level initialization stuff - clock and serial */
#define RME_A6M_LOWLVL_INIT() \
do \
{ \
    RME_A6M_LOWLVL_INIT_COMMON(); \
    \
    /* Enable USART 1 for user-level operations */ \
    /* UART IO initialization */ \
    RME_A6M_RCC_IOPENR|=RME_A6M_RCC_IOPENR_IOPAEN; \
    RME_A6M_GPIOA_MODE(RME_A6M_GPIO_MODE_ALTERNATE, 9U); \
    RME_A6M_GPIOA_OTYPE(RME_A6M_GPIO_OTYPE_PUSHPULL, 9U); \
    RME_A6M_GPIOA_OSPEED(RME_A6M_GPIO_OSPEED_HIGH, 9U); \
    RME_A6M_GPIOA_PUPD(RME_A6M_GPIO_PUPD_PULLUP, 9U); \
    RME_A6M_GPIOA_AFH(RME_A6M_GPIO_AF4_USART1, 9U); \
    \
    /* UART initialization */ \
    RME_A6M_RCC_APB2ENR|=RME_A6M_RCC_APB2ENR_USART1EN; \
    RME_A6M_USART1_CR1=0x08U; \
    RME_A6M_USART1_CR2=0x00U; \
    RME_A6M_USART1_CR3=0x00U; \
    RME_A6M_USART1_BRR=0x116U; \
    RME_A6M_USART1_CR1|=RME_A6M_USART1_CR1_UE; \
} \
while(0)

/* This is for debugging output */
#define RME_A6M_PUTCHAR(CHAR) \
do \
{ \
    RME_A6M_USART1_TDR=(rme_ptr_t)(CHAR); \
    while((RME_A6M_USART1_ISR&0x80U)==0U); \
} \
while(0)

#else

/* Other low-level initialization stuff - clock and serial */
#define RME_A6M_LOWLVL_INIT() \
do \
{ \
    RME_A6M_LOW_LEVEL_INIT_COMMON(); \
} \
while(0)
#endif

/* Prefetcher state set and get */
#define RME_A6M_PRFTH_STATE_SET(STATE) \
do \
{ \
    if((STATE)!=0) \
        RME_A6M_FLASH_ACR|=RME_A6M_FLASH_ACR_PRFTEN; \
    else \
        RME_A6M_FLASH_ACR&=~RME_A6M_FLASH_ACR_PRFTEN; \
} \
while(0)
    
#define RME_A6M_PRFTH_STATE_GET() ((RME_A6M_FLASH_ACR&RME_A6M_FLASH_ACR_PRFTEN)!=0U)
/* End Define ****************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/

