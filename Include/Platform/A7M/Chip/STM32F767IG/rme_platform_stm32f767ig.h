/******************************************************************************
Filename   : rme_platform_stm32f767ig.h
Author     : pry
Date       : 24/06/2017
Licence    : The Unlicense; see LICENSE for details.
Description: The configuration file for STM32F767IG.
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
/* Modifiable ****************************************************************/
/* Kernel object virtual memory base */
#define RME_KOM_VA_BASE                                 (0x20003000U)
/* Kernel object virtual memory size */
#define RME_KOM_VA_SIZE                                 (0xD000U)
/* Hypervisor context virtual memory base - set to 0 if no VM */
#define RME_HYP_VA_BASE                                 (0x20020000U)
/* Hypervisor context virtual memory size - set to 0 if no VM */
#define RME_HYP_VA_SIZE                                 (0x60000U)
/* Kernel memory allocation granularity order */
#define RME_KOM_SLOT_ORDER                              (4U)
/* Kernel stack size and address */
#define RME_KSTK_VA_BASE                                (0x20000FF0U)
#define RME_KSTK_VA_SIZE                                (0x1000U)
/* The maximum number of preemption priorities */
#define RME_PREEMPT_PRIO_NUM                            (32U)

/* Physical vector number, flag area base and size */
#define RME_RVM_PHYS_VCT_NUM                            (110U)
#define RME_RVM_PHYS_VCTF_BASE                          (0x20005C00U)
#define RME_RVM_PHYS_VCTF_SIZE                          (0x400U)
/* Virtual event number, flag area base and size */
#define RME_RVM_VIRT_EVT_NUM                            (20U)
#define RME_RVM_VIRT_EVTF_BASE                          (0x20005E00U)
#define RME_RVM_VIRT_EVTF_SIZE                          (0x400U)
/* Size of initial capability table */
#define RME_RVM_INIT_CPT_SIZE                           (54U)
/* Initial kernel object frontier limit */
#define RME_RVM_CPT_BOOT_FRONT                          (9U)
#define RME_RVM_KOM_BOOT_FRONT                          (0x1000U)
/* Post-boot kernel object frontier limit */
#define RME_RVM_CPT_DONE_FRONT                          (18U)
#define RME_RVM_KOM_DONE_FRONT                          (0x1800U)

/* Init process's first thread's entry point address */
#define RME_A7M_INIT_ENTRY                              (0x08004001U)
/* Init process's first thread's stack address */
#define RME_A7M_INIT_STACK                              (0x2000FFF0U)
/* What is the NVIC priority grouping? */
#define RME_A7M_NVIC_GROUPING                           (RME_A7M_NVIC_GROUPING_P2S6)
/* What is the Systick value? - 10ms per tick*/
#define RME_A7M_SYSTICK_VAL                             (2160000U)
/* Number of MPU regions available */
#define RME_A7M_REGION_NUM                              (8U)
/* What is the FPU type? */
#define RME_COP_NUM                                     (3U)
#define RME_A7M_COP_FPV4_SP                             (1U)
#define RME_A7M_COP_FPV5_SP                             (1U)
#define RME_A7M_COP_FPV5_DP                             (1U)

/* Fixed *********************************************************************/
/* What is the external crystal frequency? */
#define RME_A7M_STM32F767IG_XTAL                        (25U)
/* What are the PLL values? */
#define RME_A7M_STM32F767IG_PLLM                        (25U)
#define RME_A7M_STM32F767IG_PLLN                        (432U)
#define RME_A7M_STM32F767IG_PLLP                        (2U)
#define RME_A7M_STM32F767IG_PLLQ                        (9U)
#define RME_A7M_STM32F767IG_PLLR                        (0U)

/* Initialization registers **************************************************/
#define RME_A7M_RCC_APB1ENR                             RME_A7M_REG(0x40023840U)
#define RME_A7M_RCC_APB1ENR_PWREN                       RME_POW2(28U)

#define RME_A7M_PWR_CR1                                 RME_A7M_REG(0x40007000U)
#define RME_A7M_PWR_CR1_VOS_SCALE1                      RME_FIELD(3U,14U)
#define RME_A7M_PWR_CR1_ODEN                            RME_POW2(16U)
#define RME_A7M_PWR_CR1_ODSWEN                          RME_POW2(17U)
#define RME_A7M_PWR_CSR1                                RME_A7M_REG(0x40007004U)
#define RME_A7M_PWR_CSR1_ODRDY                          RME_POW2(16U)
#define RME_A7M_PWR_CSR1_ODSWRDY                        RME_POW2(17U)

#define RME_A7M_RCC_CR                                  RME_A7M_REG(0x40023800U)
#define RME_A7M_RCC_CR_HSEON                            RME_POW2(16U)
#define RME_A7M_RCC_CR_HSERDY                           RME_POW2(17U)
#define RME_A7M_RCC_CR_PLLON                            RME_POW2(24U)
#define RME_A7M_RCC_CR_PLLRDY                           RME_POW2(25U)

#define RME_A7M_RCC_PLLCFGR                             RME_A7M_REG(0x40023804U)
#define RME_A7M_RCC_PLLCFGR_SOURCE_HSE                  RME_POW2(22U)
#define RME_A7M_RCC_PLLCFGR_PLLM(X)                     (X)
#define RME_A7M_RCC_PLLCFGR_PLLN(X)                     RME_FIELD(X,6U)
#define RME_A7M_RCC_PLLCFGR_PLLP(X)                     RME_FIELD(((X)>>1)-1U,16U)
#define RME_A7M_RCC_PLLCFGR_PLLQ(X)                     RME_FIELD(X,24U)
#define RME_A7M_RCC_PLLCFGR_PLLR(X)                     RME_FIELD(X,28U)

#define RME_A7M_RCC_CIR                                 RME_A7M_REG(0x4002380CU)

#define RME_A7M_FLASH_ACR                               RME_A7M_REG(0x40023C00U)
#define RME_A7M_FLASH_ACR_LATENCY(X)                    RME_A7M_FLASH_ACR=((RME_A7M_FLASH_ACR&~0x0FU)|(X))
#define RME_A7M_FLASH_ACR_ARTEN                         RME_POW2(9U)
#define RME_A7M_FLASH_ACR_PRFTEN                        RME_POW2(8U)

#define RME_A7M_RCC_CFGR                                RME_A7M_REG(0x40023808U)
#define RME_A7M_RCC_CFGR_PCLK1(X)                       RME_A7M_RCC_CFGR=(RME_A7M_RCC_CFGR&~0x1C00U)|((X)<<10)
#define RME_A7M_RCC_CFGR_PCLK2(X)                       RME_A7M_RCC_CFGR=(RME_A7M_RCC_CFGR&~0xE000U)|((X)<<13)
#define RME_A7M_RCC_CFGR_HCLK(X)                        RME_A7M_RCC_CFGR=(RME_A7M_RCC_CFGR&~0x00F0U)|((X)<<4)
#define RME_A7M_RCC_CFGR_SYSCLK(X)                      RME_A7M_RCC_CFGR=(RME_A7M_RCC_CFGR&~0x0003U)|((X))

#define RME_A7M_RCC_CFGR_SYSCLKSOURCE_PLLCLK            (0x02U)
#define RME_A7M_RCC_CFGR_SYSCLK_DIV1                    (0x00U)
#define RME_A7M_RCC_CFGR_HCLK_DIV1                      (0x00U)
#define RME_A7M_RCC_CFGR_HCLK_DIV2                      (0x04U)
#define RME_A7M_RCC_CFGR_HCLK_DIV4                      (0x05U)
#define RME_A7M_RCC_CFGR_HCLK_DIV8                      (0x06U)
#define RME_A7M_RCC_CFGR_HCLK_DIV16                     (0x07U)

#define RME_A7M_TIM4_SR                                 RME_A7M_REG(0x40000810U)
#define RME_A7M_TIM_FLAG_UPDATE                         RME_POW2(0U)

#define RME_A7M_RCC_AHB1ENR                             RME_A7M_REG(0x40023830U)
#define RME_A7M_RCC_AHB1ENR_GPIOAEN                     RME_POW2(0U)

#define RME_A7M_RCC_APB2ENR                             RME_A7M_REG(0x40023844U)
#define RME_A7M_RCC_APB2ENR_USART1EN                    RME_POW2(4U)

#define RME_A7M_GPIOA_MODER                             RME_A7M_REG(0x40020000U)
#define RME_A7M_GPIOA_OTYPER                            RME_A7M_REG(0x40020004U)
#define RME_A7M_GPIOA_OSPEEDR                           RME_A7M_REG(0x40020008U)
#define RME_A7M_GPIOA_PUPDR                             RME_A7M_REG(0x4002000CU)
#define RME_A7M_GPIOA_IDR                               RME_A7M_REG(0x40020010U)
#define RME_A7M_GPIOA_ODR                               RME_A7M_REG(0x40020014U)
#define RME_A7M_GPIOA_BSRR                              RME_A7M_REG(0x40020018U)
#define RME_A7M_GPIOA_LCKR                              RME_A7M_REG(0x4002001CU)
#define RME_A7M_GPIOA_AFR0                              RME_A7M_REG(0x40020020U)
#define RME_A7M_GPIOA_AFR1                              RME_A7M_REG(0x40020024U)

#define RME_A7M_GPIO_MODE_INPUT                         (0U)
#define RME_A7M_GPIO_MODE_OUTPUT                        (1U)
#define RME_A7M_GPIO_MODE_ALTERNATE                     (2U)
#define RME_A7M_GPIO_MODE_ANALOG                        (3U)

#define RME_A7M_GPIO_OTYPE_PUSHPULL                     (0U)
#define RME_A7M_GPIO_OTYPE_OPENDRAIN                    (1U)

#define RME_A7M_GPIO_OSPEED_LOW                         (0U)
#define RME_A7M_GPIO_OSPEED_MEDIUM                      (1U)
#define RME_A7M_GPIO_OSPEED_HIGH                        (2U)
#define RME_A7M_GPIO_OSPEED_VERYHIGH                    (3U)

#define RME_A7M_GPIO_PUPD_NONE                          (0U)
#define RME_A7M_GPIO_PUPD_PULLUP                        (1U)
#define RME_A7M_GPIO_PUPD_PULLDOWN                      (2U)

#define RME_A7M_GPIO_AF7_USART1                         (0x07U)

#define RME_A7M_GPIOA_MODE(MODE,PIN)                    RME_A7M_GPIOA_MODER=(RME_A7M_GPIOA_MODER&~(0x03U<<((PIN)*2)))|((MODE)<<((PIN)*2))
#define RME_A7M_GPIOA_OTYPE(OTYPE,PIN)                  RME_A7M_GPIOA_OTYPER=(RME_A7M_GPIOA_OTYPER&~(0x01U<<(PIN)))|((OTYPE)<<(PIN))
#define RME_A7M_GPIOA_OSPEED(OSPEED,PIN)                RME_A7M_GPIOA_OSPEEDR=(RME_A7M_GPIOA_OSPEEDR&~(0x03U<<((PIN)*2)))|((OSPEED)<<((PIN)*2))
#define RME_A7M_GPIOA_PUPD(PUPD,PIN)                    RME_A7M_GPIOA_PUPDR=(RME_A7M_GPIOA_PUPDR&~(0x03U<<((PIN)*2)))|((PUPD)<<((PIN)*2))
#define RME_A7M_GPIOA_AF0(AF,PIN)                       RME_A7M_GPIOA_AFR0=(RME_A7M_GPIOA_AFR0&~(0x0FU<<((PIN)*4)))|((AF)<<((PIN)*4))
#define RME_A7M_GPIOA_AF1(AF,PIN)                       RME_A7M_GPIOA_AFR1=(RME_A7M_GPIOA_AFR1&~(0x0FU<<((PIN-8)*4)))|((AF)<<((PIN-8)*4))

#define RME_A7M_USART1_CR1                              RME_A7M_REG(0x40011000U)
#define RME_A7M_USART1_CR2                              RME_A7M_REG(0x40011004U)
#define RME_A7M_USART1_CR3                              RME_A7M_REG(0x40011008U)
#define RME_A7M_USART1_BRR                              RME_A7M_REG(0x4001100CU)
#define RME_A7M_USART1_ISR                              RME_A7M_REG(0x4001101CU)
#define RME_A7M_USART1_TDR                              RME_A7M_REG(0x40011028U)

#define RME_A7M_USART1_CR1_UE                           RME_POW2(0U)

/* Preinitialization of critical hardware */
#define RME_A7M_LOWLVL_PREINIT() \
do \
{ \
    /* Set HSION bit */ \
    RME_A7M_RCC_CR|=0x00000001U; \
    /* Reset CFGR register */ \
    RME_A7M_RCC_CFGR=0x00000000U; \
    /* Reset HSEON, CSSON and PLLON bits */ \
    RME_A7M_RCC_CR&=0xFEF6FFFFU; \
    /* Reset PLLCFGR register */ \
    RME_A7M_RCC_PLLCFGR=0x24003010U; \
    /* Reset HSEBYP bit */ \
    RME_A7M_RCC_CR&=0xFFFBFFFFU; \
    /* Disable all interrupts */ \
    RME_A7M_RCC_CIR=0x00000000U; \
    /* Vector table address */ \
    RME_A7M_SCB_VTOR=0x08000000U; \
} \
while(0)

/* Other low-level initialization stuff - clock and serial
 * STM32F7xx APB1<45MHz, APB2<90MHz. When running at 216MHz,
 * actually we are overdriving the bus a little, which might
 * be fine. */
#define RME_A7M_LOWLVL_INIT_COMMON() \
do \
{ \
    /* Set the clock tree in the system */ \
    /* Enble power regulator clock, and configure voltage scaling function */ \
    RME_A7M_RCC_APB1ENR|=RME_A7M_RCC_APB1ENR_PWREN; \
    __RME_A7M_Barrier(); \
    RME_A7M_PWR_CR1|=RME_A7M_PWR_CR1_VOS_SCALE1; \
    __RME_A7M_Barrier(); \
    /* Initialize the oscillator */ \
    RME_A7M_RCC_CR|=RME_A7M_RCC_CR_HSEON; \
    __RME_A7M_Barrier(); \
    while((RME_A7M_RCC_CR&RME_A7M_RCC_CR_HSERDY)==0); \
    /* Fpll=Fin/PLLM*PLLN, Fsys=Fpll/PLLP, Fperiph=Fpll/PLLQ */ \
    RME_A7M_RCC_CR&=~RME_A7M_RCC_CR_PLLON; \
    __RME_A7M_Barrier(); \
    while((RME_A7M_RCC_CR&RME_A7M_RCC_CR_PLLRDY)!=0); \
    RME_A7M_RCC_PLLCFGR=RME_A7M_RCC_PLLCFGR_SOURCE_HSE| \
                        RME_A7M_RCC_PLLCFGR_PLLM(RME_A7M_STM32F767IG_PLLM)| \
                        RME_A7M_RCC_PLLCFGR_PLLN(RME_A7M_STM32F767IG_PLLN)| \
                        RME_A7M_RCC_PLLCFGR_PLLP(RME_A7M_STM32F767IG_PLLP)| \
                        RME_A7M_RCC_PLLCFGR_PLLQ(RME_A7M_STM32F767IG_PLLQ)| \
                        RME_A7M_RCC_PLLCFGR_PLLR(RME_A7M_STM32F767IG_PLLR); \
    __RME_A7M_Barrier(); \
    RME_A7M_RCC_CR|=RME_A7M_RCC_CR_PLLON; \
    __RME_A7M_Barrier(); \
    while((RME_A7M_RCC_CR&RME_A7M_RCC_CR_PLLRDY)==0U); \
    /* Overdrive to 216MHz */ \
    RME_A7M_PWR_CR1|=RME_A7M_PWR_CR1_ODEN; \
    __RME_A7M_Barrier(); \
    while((RME_A7M_PWR_CSR1&RME_A7M_PWR_CSR1_ODRDY)==0U); \
    RME_A7M_PWR_CR1|=RME_A7M_PWR_CR1_ODSWEN; \
    __RME_A7M_Barrier(); \
    while((RME_A7M_PWR_CSR1&RME_A7M_PWR_CSR1_ODSWRDY)==0U); \
    \
    /* HCLK,PCLK1 & PCLK2 configuration */ \
    RME_A7M_FLASH_ACR_LATENCY(7U); \
    RME_ASSERT((RME_A7M_FLASH_ACR&0x0FU)==7U); \
    RME_A7M_RCC_CFGR_PCLK1(RME_A7M_RCC_CFGR_HCLK_DIV4); \
    RME_A7M_RCC_CFGR_PCLK2(RME_A7M_RCC_CFGR_HCLK_DIV2); \
    RME_A7M_RCC_CFGR_HCLK(RME_A7M_RCC_CFGR_SYSCLK_DIV1); \
    RME_A7M_RCC_CFGR_SYSCLK(RME_A7M_RCC_CFGR_SYSCLKSOURCE_PLLCLK); \
    __RME_A7M_Barrier(); \
    \
    /* Cache/Flash ART enabling */ \
    __RME_A7M_Cache_Init(); \
    RME_A7M_FLASH_ACR|=RME_A7M_FLASH_ACR_ARTEN; \
    RME_A7M_FLASH_ACR|=RME_A7M_FLASH_ACR_PRFTEN; \
} \
while(0)

#if(RME_DBGLOG_ENABLE!=0U)
/* Other low-level initialization stuff - clock and serial */
#define RME_A7M_LOWLVL_INIT() \
do \
{ \
    RME_A7M_LOWLVL_INIT_COMMON(); \
    \
    /* Enable USART 1 for user-level operations */ \
    /* Clock enabling */ \
    RME_A7M_RCC_AHB1ENR|=RME_A7M_RCC_AHB1ENR_GPIOAEN; \
    RME_A7M_RCC_APB2ENR|=RME_A7M_RCC_APB2ENR_USART1EN; \
    /* UART IO initialization */ \
    RME_A7M_GPIOA_MODE(RME_A7M_GPIO_MODE_ALTERNATE,9U); \
    RME_A7M_GPIOA_OTYPE(RME_A7M_GPIO_OTYPE_PUSHPULL,9U); \
    RME_A7M_GPIOA_OSPEED(RME_A7M_GPIO_OSPEED_HIGH,9U); \
    RME_A7M_GPIOA_PUPD(RME_A7M_GPIO_PUPD_PULLUP,9U); \
    RME_A7M_GPIOA_AF1(RME_A7M_GPIO_AF7_USART1,9U); \
    /* UART initialization */ \
    RME_A7M_USART1_CR1=0x0000008U; \
    RME_A7M_USART1_CR2=0x00U; \
    RME_A7M_USART1_CR3=0x00U; \
    RME_A7M_USART1_BRR=0x3AAU; \
    RME_A7M_USART1_CR1|=RME_A7M_USART1_CR1_UE; \
} \
while(0)

/* This is for debugging output */
#define RME_A7M_PUTCHAR(CHAR) \
do \
{ \
    RME_A7M_USART1_TDR=(rme_ptr_t)(CHAR); \
    while((RME_A7M_USART1_ISR&0x40U)==0U); \
} \
while(0)

#else

/* Other low-level initialization stuff - clock and serial */
#define RME_A7M_LOWLVL_INIT() \
do \
{ \
    RME_A7M_LOWLVL_INIT_COMMON(); \
} \
while(0)
#endif
    
/* Prefetcher state set and get */
#define RME_A7M_PRFTH_STATE_SET(STATE) \
do \
{ \
    if((STATE)!=0) \
    { \
        RME_A7M_FLASH_ACR|=RME_A7M_FLASH_ACR_ARTEN; \
        RME_A7M_FLASH_ACR|=RME_A7M_FLASH_ACR_PRFTEN; \
    } \
    else \
    { \
        RME_A7M_FLASH_ACR&=~RME_A7M_FLASH_ACR_PRFTEN; \
        RME_A7M_FLASH_ACR&=~RME_A7M_FLASH_ACR_ARTEN; \
    } \
} \
while(0)
    
#define RME_A7M_PRFTH_STATE_GET() ((RME_A7M_FLASH_ACR&RME_A7M_FLASH_ACR_ARTEN)!=0U)
/* End Define ****************************************************************/

/* Struct ********************************************************************/

/* End Struct ****************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
