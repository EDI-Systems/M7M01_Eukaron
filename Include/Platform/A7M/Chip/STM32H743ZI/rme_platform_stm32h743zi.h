/******************************************************************************
Filename   : rme_platform_stm32h743zi.h
Author     : pry
Date       : 24/06/2017
Licence    : The Unlicense; see LICENSE for details.
Description: The configuration file for STM32H743ZI.
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
#define RME_KOM_VA_BASE                                 (0x20003000U)
/* Kernel object virtual memory size */
#define RME_KOM_VA_SIZE                                 (0xD000U)
/* Hypervisor context virtual memory base - set to 0 if no VM */
#define RME_HYP_VA_BASE                                 (0x24000000U)
/* Hypervisor context virtual memory size - set to 0 if no VM */
#define RME_HYP_VA_SIZE                                 (0x80000U)
/* Kernel memory allocation granularity order */
#define RME_KOM_SLOT_ORDER                              (4U)
/* Kernel stack size and address */
#define RME_KSTK_VA_BASE                                (0x20000FF0U)
#define RME_KSTK_VA_SIZE                                (0x1000U)
/* The maximum number of preemption priorities */
#define RME_PREEMPT_PRIO_NUM                            (32U)

/* Physical vector number, flag area base and size */
#define RME_RVM_PHYS_VCT_NUM                            (150U)
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
#define RME_A7M_SYSTICK_VAL                             (4000000U)
/* Number of MPU regions available */
#define RME_A7M_REGION_NUM                              (16U)
/* What is the FPU type? */
#define RME_COP_NUM                                     (3U)
#define RME_A7M_COP_FPV4_SP                             (1U)
#define RME_A7M_COP_FPV5_SP                             (1U)
#define RME_A7M_COP_FPV5_DP                             (1U)

/* Chip specific *************************************************************/
/* Crystal frequency - 8MHz MCO from ST-LINK, bypass mode */
#define RME_A7M_STM32H743ZI_XTAL                        (8U)

/* PLL1 parameter */
#define RME_A7M_STM32H743ZI_PLLM                        (4U)
#define RME_A7M_STM32H743ZI_PLLN                        (400U)
#define RME_A7M_STM32H743ZI_PLLP                        (2U)
#define RME_A7M_STM32H743ZI_PLLQ                        (4U)
#define RME_A7M_STM32H743ZI_PLLR                        (2U)

/* Register address - power */
#define RME_A7M_PWR_CSR1                               RME_A7M_REG(0x58024804U)
#define RME_A7M_PWR_CSR1_ACTVOSRDY                     RME_POW2(13U)
#define RME_A7M_PWR_CR3                                RME_A7M_REG(0x5802480CU)
#define RME_A7M_PWR_CR3_BYPASS                         RME_POW2(0U)
#define RME_A7M_PWR_CR3_LDOEN                          RME_POW2(1U)
#define RME_A7M_PWR_CR3_SCUEN                          RME_POW2(2U)
#define RME_A7M_PWR_D3CR                                RME_A7M_REG(0x58024818U)
#define RME_A7M_PWR_D3CR_VOS_SCALE2                     (0x8000U)               /* VOS[15:14]=10b:Scale2 */
#define RME_A7M_PWR_D3CR_VOS_SCALE1                     (0xC000U)               /* VOS[15:14]=11b:Scale1(H743 encoding 00=S0,01=S3,10=S2,11=S1) */
#define RME_A7M_PWR_D3CR_VOSRDY                         RME_POW2(13U)

/* Register address - clock control */
#define RME_A7M_RCC_CR                                  RME_A7M_REG(0x58024400U)
#define RME_A7M_RCC_CR_HSEBYP                           RME_POW2(18U)
#define RME_A7M_RCC_CR_HSEON                            RME_POW2(16U)
#define RME_A7M_RCC_CR_HSERDY                           RME_POW2(17U)
#define RME_A7M_RCC_CR_PLL1ON                           RME_POW2(24U)
#define RME_A7M_RCC_CR_PLL1RDY                          RME_POW2(25U)

#define RME_A7M_RCC_PLLCKSELR                           RME_A7M_REG(0x58024428U)
#define RME_A7M_RCC_PLLCKSELR_PLL1SRC_HSE               RME_POW2(1U)
#define RME_A7M_RCC_PLLCKSELR_DIVM1(X)                  RME_FIELD(X,4U)

#define RME_A7M_RCC_PLLCFGR                             RME_A7M_REG(0x5802442CU)
#define RME_A7M_RCC_PLLCFGR_VCO_WIDE                    RME_POW2(0U)
#define RME_A7M_RCC_PLLCFGR_PLL1RGE_1                   RME_POW2(2U)
#define RME_A7M_RCC_PLLCFGR_DIVP1EN                     RME_POW2(16U)

#define RME_A7M_RCC_PLL1DIVR                            RME_A7M_REG(0x58024430U)
#define RME_A7M_RCC_PLL1FRACR                           RME_A7M_REG(0x58024434U)
#define RME_A7M_RCC_PLL1DIVR_N1(X)                      RME_FIELD(((X)-1U),0U)
#define RME_A7M_RCC_PLL1DIVR_P1(X)                      RME_FIELD(((X)-1U),9U)
#define RME_A7M_RCC_PLL1DIVR_Q1(X)                      RME_FIELD(((X)-1U),16U)
#define RME_A7M_RCC_PLL1DIVR_R1(X)                      RME_FIELD(((X)-1U),24U)
#define RME_A7M_RCC_D1CFGR                              RME_A7M_REG(0x58024418U)
#define RME_A7M_RCC_D2CFGR                              RME_A7M_REG(0x5802441CU)
#define RME_A7M_RCC_D3CFGR                              RME_A7M_REG(0x58024420U)
#define RME_A7M_RCC_D1CFGR_D1CPRE(X)                    RME_A7M_RCC_D1CFGR=(RME_A7M_RCC_D1CFGR&~0x0700U)|((X)<<8)
#define RME_A7M_RCC_D1CFGR_HPRE(X)                      RME_A7M_RCC_D1CFGR=(RME_A7M_RCC_D1CFGR&~0x000FU)|((X))
#define RME_A7M_RCC_D1CFGR_D1PPRE(X)                    RME_A7M_RCC_D1CFGR=(RME_A7M_RCC_D1CFGR&~0x0070U)|((X)<<4)
#define RME_A7M_RCC_D2CFGR_D2PPRE1(X)                   RME_A7M_RCC_D2CFGR=(RME_A7M_RCC_D2CFGR&~0x0070U)|((X)<<4)
#define RME_A7M_RCC_D2CFGR_D2PPRE2(X)                   RME_A7M_RCC_D2CFGR=(RME_A7M_RCC_D2CFGR&~0x0700U)|((X)<<8)
#define RME_A7M_RCC_D3CFGR_D3PPRE(X)                    RME_A7M_RCC_D3CFGR=(RME_A7M_RCC_D3CFGR&~0x0070U)|((X)<<4)

#define RME_A7M_RCC_CFGR                                RME_A7M_REG(0x58024410U)
#define RME_A7M_RCC_CFGR_SYSCLK(X)                      RME_A7M_RCC_CFGR=(RME_A7M_RCC_CFGR&~0x0007U)|((X))
#define RME_A7M_RCC_CFGR_SWS(X)                         (((X)>>3)&0x0007U)

#define RME_A7M_RCC_CFGR_SYSCLKSOURCE_PLL1              (0x03U)
/* The prescaler encodings: HPRE 0xxx=/1,1000=/2; DnPPRE 0xx=/1,100x=/2 */
#define RME_A7M_RCC_PRESC_DIV1                          (0x00U)
#define RME_A7M_RCC_HPRESC_DIV2                         (0x08U)
#define RME_A7M_RCC_PPRESC_DIV2                         (0x04U)

/* Register address - flash */
#define RME_A7M_FLASH_ACR                               RME_A7M_REG(0x52002000U)
#define RME_A7M_FLASH_ACR_LATENCY(X)                    RME_A7M_FLASH_ACR=((RME_A7M_FLASH_ACR&~0xFFU)|(X))

/* Register address - bus clocks */
#define RME_A7M_RCC_AHB4ENR                             RME_A7M_REG(0x580244E0U)
#define RME_A7M_RCC_AHB4ENR_GPIODEN                     RME_POW2(3U)
#define RME_A7M_RCC_APB1LENR                            RME_A7M_REG(0x580244E8U)
#define RME_A7M_RCC_APB1LENR_USART3EN                   RME_POW2(18U)
#define RME_A7M_RCC_APB1LENR_PWREN                      RME_POW2(28U)

#define RME_A7M_GPIOD_MODER                             RME_A7M_REG(0x58020C00U)
#define RME_A7M_GPIOD_OTYPER                            RME_A7M_REG(0x58020C04U)
#define RME_A7M_GPIOD_OSPEEDR                           RME_A7M_REG(0x58020C08U)
#define RME_A7M_GPIOD_PUPDR                             RME_A7M_REG(0x58020C0CU)
#define RME_A7M_GPIOD_IDR                               RME_A7M_REG(0x58020C10U)
#define RME_A7M_GPIOD_ODR                               RME_A7M_REG(0x58020C14U)
#define RME_A7M_GPIOD_BSRR                              RME_A7M_REG(0x58020C18U)
#define RME_A7M_GPIOD_LCKR                              RME_A7M_REG(0x58020C1CU)
#define RME_A7M_GPIOD_AFR0                              RME_A7M_REG(0x58020C20U)
#define RME_A7M_GPIOD_AFR1                              RME_A7M_REG(0x58020C24U)

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

#define RME_A7M_GPIO_AF7_USART3                         (0x07U)
#define RME_A7M_GPIOD_MODE(MODE,PIN)                    RME_A7M_GPIOD_MODER=(RME_A7M_GPIOD_MODER&~(0x03U<<((PIN)*2)))|((MODE)<<((PIN)*2))
#define RME_A7M_GPIOD_OTYPE(OTYPE,PIN)                  RME_A7M_GPIOD_OTYPER=(RME_A7M_GPIOD_OTYPER&~(0x01U<<(PIN)))|((OTYPE)<<(PIN))
#define RME_A7M_GPIOD_OSPEED(OSPEED,PIN)                RME_A7M_GPIOD_OSPEEDR=(RME_A7M_GPIOD_OSPEEDR&~(0x03U<<((PIN)*2)))|((OSPEED)<<((PIN)*2))
#define RME_A7M_GPIOD_PUPD(PUPD,PIN)                    RME_A7M_GPIOD_PUPDR=(RME_A7M_GPIOD_PUPDR&~(0x03U<<((PIN)*2)))|((PUPD)<<((PIN)*2))
#define RME_A7M_GPIOD_AF0(AF,PIN)                       RME_A7M_GPIOD_AFR0=(RME_A7M_GPIOD_AFR0&~(0x0FU<<((PIN)*4)))|((AF)<<((PIN)*4))
#define RME_A7M_GPIOD_AF1(AF,PIN)                       RME_A7M_GPIOD_AFR1=(RME_A7M_GPIOD_AFR1&~(0x0FU<<((PIN-8)*4)))|((AF)<<((PIN-8)*4))

#define RME_A7M_USART3_CR1                              RME_A7M_REG(0x40004800U)
#define RME_A7M_USART3_CR2                              RME_A7M_REG(0x40004804U)
#define RME_A7M_USART3_CR3                              RME_A7M_REG(0x40004808U)
#define RME_A7M_USART3_BRR                              RME_A7M_REG(0x4000480CU)
#define RME_A7M_USART3_ISR                              RME_A7M_REG(0x4000481CU)
#define RME_A7M_USART3_TDR                              RME_A7M_REG(0x40004828U)

#define RME_A7M_USART3_CR1_UE                           RME_POW2(0U)

/* Preinitialization of critical hardware */
#define RME_A7M_LOWLVL_PREINIT() \
do \
{ \
    /* Reset CFGR register */ \
    RME_A7M_RCC_CFGR=0x00000000U; \
    /* Reset HSEON, CSSON and PLL1ON bits */ \
    RME_A7M_RCC_CR&=0xFEF6FFFFU; \
    /* Reset PLL configuration registers */ \
    RME_A7M_RCC_PLLCKSELR=0x00000000U; \
    RME_A7M_RCC_PLLCFGR=0x01FF0000U; \
    RME_A7M_RCC_PLL1DIVR=0x01010280U; \
    RME_A7M_RCC_PLL1FRACR=0x00000000U; \
    /* Vector table address */ \
    RME_A7M_SCB_VTOR=0x08000000U; \
} \
while(0)

/* Other low-level initialization stuff - clock and serial
 * STM32H743 D1 core @400MHz(VOS1), HCLK=200MHz, APB1/APB2=100MHz,
 * APB1 timer clock=200MHz(cpu/2), from HSE(8MHz MCO from ST-LINK). */
#define RME_A7M_LOWLVL_INIT_COMMON() \
do \
{ \
    /* Set the clock tree in the system */ \
    /* Power init, register-exact port of the proven RMP/HAL sequence */ \
    /* (M5P01 rmp_platform_stm32h743zi.h + STM32CubeH7 V1.9.1): first lock */ \
    /* the supply configuration to the internal LDO, then request VOS1. */ \
    /* The PWR clock needs no explicit enable (RCC_APB4ENR.PWREN reset=1). */ \
    RME_A7M_PWR_CR3=(RME_A7M_PWR_CR3&~(RME_A7M_PWR_CR3_SCUEN| \
                                        RME_A7M_PWR_CR3_LDOEN| \
                                        RME_A7M_PWR_CR3_BYPASS))| \
                     RME_A7M_PWR_CR3_LDOEN; \
    __RME_A7M_Barrier(); \
    while((RME_A7M_PWR_CSR1&RME_A7M_PWR_CSR1_ACTVOSRDY)==0U); \
    __RME_A7M_Barrier(); \
    /* VOS1 for 400MHz. VOS[15:14]: 00=S0,01=S3(reset),10=S2,11=S1. */ \
    RME_A7M_PWR_D3CR=(RME_A7M_PWR_D3CR&0xFFFF3FFFU)|RME_A7M_PWR_D3CR_VOS_SCALE1; \
    __RME_A7M_Barrier(); \
    while((RME_A7M_PWR_D3CR&RME_A7M_PWR_D3CR_VOSRDY)==0U); \
    /* Initialize the oscillator - HSE bypass 8MHz from ST-LINK MCO */ \
    RME_A7M_RCC_CR|=RME_A7M_RCC_CR_HSEBYP; \
    __RME_A7M_Barrier(); \
    RME_A7M_RCC_CR|=RME_A7M_RCC_CR_HSEON; \
    __RME_A7M_Barrier(); \
    while((RME_A7M_RCC_CR&RME_A7M_RCC_CR_HSERDY)==0U); \
    /* Fpll=Fin/PLLM*PLLN, Fsys=Fpll/PLLP; 8MHz/4*400/2 = 400MHz */ \
    RME_A7M_RCC_CR&=~RME_A7M_RCC_CR_PLL1ON; \
    __RME_A7M_Barrier(); \
    while((RME_A7M_RCC_CR&RME_A7M_RCC_CR_PLL1RDY)!=0U); \
    RME_A7M_RCC_PLLCKSELR=RME_A7M_RCC_PLLCKSELR_PLL1SRC_HSE| \
                          RME_A7M_RCC_PLLCKSELR_DIVM1(RME_A7M_STM32H743ZI_PLLM); \
    RME_A7M_RCC_PLLCFGR=RME_A7M_RCC_PLLCFGR_VCO_WIDE| \
                        RME_A7M_RCC_PLLCFGR_PLL1RGE_1| \
                        RME_A7M_RCC_PLLCFGR_DIVP1EN; \
    RME_A7M_RCC_PLL1DIVR=RME_A7M_RCC_PLL1DIVR_N1(RME_A7M_STM32H743ZI_PLLN)| \
                         RME_A7M_RCC_PLL1DIVR_P1(RME_A7M_STM32H743ZI_PLLP)| \
                         RME_A7M_RCC_PLL1DIVR_Q1(RME_A7M_STM32H743ZI_PLLQ)| \
                         RME_A7M_RCC_PLL1DIVR_R1(RME_A7M_STM32H743ZI_PLLR); \
    __RME_A7M_Barrier(); \
    RME_A7M_RCC_CR|=RME_A7M_RCC_CR_PLL1ON; \
    __RME_A7M_Barrier(); \
    while((RME_A7M_RCC_CR&RME_A7M_RCC_CR_PLL1RDY)==0U); \
    /* HCLK=200MHz(HPRE/2), APB1/APB2/APB3/APB4=100MHz(PPRE/2) */ \
    RME_A7M_FLASH_ACR_LATENCY(4U); \
    RME_ASSERT((RME_A7M_FLASH_ACR&0xFFU)==4U); \
    RME_A7M_RCC_D1CFGR_D1CPRE(RME_A7M_RCC_PRESC_DIV1); \
    RME_A7M_RCC_D2CFGR_D2PPRE1(RME_A7M_RCC_PPRESC_DIV2); \
    RME_A7M_RCC_D2CFGR_D2PPRE2(RME_A7M_RCC_PPRESC_DIV2); \
    RME_A7M_RCC_D3CFGR_D3PPRE(RME_A7M_RCC_PPRESC_DIV2); \
    RME_A7M_RCC_D1CFGR_D1PPRE(RME_A7M_RCC_PPRESC_DIV2); \
    RME_A7M_RCC_D1CFGR_HPRE(RME_A7M_RCC_HPRESC_DIV2); \
    RME_A7M_RCC_CFGR_SYSCLK(RME_A7M_RCC_CFGR_SYSCLKSOURCE_PLL1); \
    __RME_A7M_Barrier(); \
    while(RME_A7M_RCC_CFGR_SWS(RME_A7M_RCC_CFGR)!=RME_A7M_RCC_CFGR_SYSCLKSOURCE_PLL1); \
    /* Cache enabling - the L1 caches of the Cortex-M7 core */ \
    __RME_A7M_Cache_Init(); \
} \
while(0)
#if(RME_DBGLOG_ENABLE!=0U)
/* Other low-level initialization stuff - clock and serial */
#define RME_A7M_LOWLVL_INIT() \
do \
{ \
    RME_A7M_LOWLVL_INIT_COMMON(); \
     \
    /* Enable USART 3 for user-level operations - PD8 is the VCP TX on NUCLEO-H743ZI */ \
    /* Clock enabling */ \
    RME_A7M_RCC_AHB4ENR|=RME_A7M_RCC_AHB4ENR_GPIODEN; \
    RME_A7M_RCC_APB1LENR|=RME_A7M_RCC_APB1LENR_USART3EN; \
    /* UART IO initialization */ \
    RME_A7M_GPIOD_MODE(RME_A7M_GPIO_MODE_ALTERNATE, 8U); \
    RME_A7M_GPIOD_OTYPE(RME_A7M_GPIO_OTYPE_PUSHPULL, 8U); \
    RME_A7M_GPIOD_OSPEED(RME_A7M_GPIO_OSPEED_HIGH, 8U); \
    RME_A7M_GPIOD_PUPD(RME_A7M_GPIO_PUPD_PULLUP ,8U); \
    RME_A7M_GPIOD_AF1(RME_A7M_GPIO_AF7_USART3, 8U); \
    /* UART initialization - 115200 baud at PCLK1=100MHz */ \
    RME_A7M_USART3_CR1=0x0000008U; \
    RME_A7M_USART3_CR2=0x00U; \
    RME_A7M_USART3_CR3=0x00U; \
    RME_A7M_USART3_BRR=0x364U; \
    RME_A7M_USART3_CR1|=RME_A7M_USART3_CR1_UE; \
} \
while(0)

/* This is for debugging output */
#define RME_A7M_PUTCHAR(CHAR) \
do \
{ \
    RME_A7M_USART3_TDR=(rme_ptr_t)(CHAR); \
    while((RME_A7M_USART3_ISR&0x40U)==0U); \
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
    
/* Prefetcher state set and get - no flash prefetch control on STM32H7 */
#define RME_A7M_PRFTH_STATE_SET(STATE) \
do \
{ \
} \
while(0)
    
#define RME_A7M_PRFTH_STATE_GET() (1U)

/* Action before placing processor in low-power mode */
#define RME_A7M_WAIT_INT_PRE()
/* Action after placing processor in low-power mode */
#define RME_A7M_WAIT_INT_POST()
/* End Define ****************************************************************/

/* Struct ********************************************************************/

/* End Struct ****************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/