/******************************************************************************
Filename   : rme_platform_stm32f405rg.h
Author     : pry
Date       : 24/06/2017
Licence    : The Unlicense; see LICENSE for details.
Description: The configuration file for STM32F405RG.
******************************************************************************/

/* Defines *******************************************************************/
/* Debugging *****************************************************************/
#define RME_ASSERT_CORRECT                              (0U)
#define RME_DEBUG_PRINT                                 (1U)
/* Generator *****************************************************************/
/* Are we using the generator? */
#define RME_RVM_GEN_ENABLE                              (0U)
/* Modifiable ****************************************************************/
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
#define RME_RVM_PHYS_VCT_NUM                            (82U)
#define RME_RVM_PHYS_VCTF_BASE                          (0x10005C00U)
#define RME_RVM_PHYS_VCTF_SIZE                          (0x200U)
/* Virtual event number, flag area base and its size */
#define RME_RVM_VIRT_EVT_NUM                            (20U)
#define RME_RVM_VIRT_EVTF_BASE                          (0x10005E00U)
#define RME_RVM_VIRT_EVTF_SIZE                          (0x200U)
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
#define RME_A7M_INIT_STACK                              (0x1000FFF0U)
/* What is the NVIC priority grouping? */
#define RME_A7M_NVIC_GROUPING                           (RME_A7M_NVIC_GROUPING_P2S6)
/* What is the Systick value? - 10ms per tick*/
#define RME_A7M_SYSTICK_VAL                             (1680000U)
/* Number of MPU regions available */
#define RME_A7M_REGION_NUM                              (8U)
/* What is the FPU type? */
#define RME_COP_NUM                                     (1U)
#define RME_A7M_COP_FPV4_SP                             (1U)
#define RME_A7M_COP_FPV5_SP                             (0U)
#define RME_A7M_COP_FPV5_DP                             (0U)

/* Fixed *********************************************************************/
/* What is the external crystal frequency? */
#define RME_A7M_STM32F405RG_XTAL                        (8U)
/* What are the PLL values? */
#define RME_A7M_STM32F405RG_PLLM                        (8U)
#define RME_A7M_STM32F405RG_PLLN                        (336U)
#define RME_A7M_STM32F405RG_PLLP                        (2U)
#define RME_A7M_STM32F405RG_PLLQ                        (7U)

/* Interrupts ****************************************************************/
#define WWDG_IRQHandler                                 IRQ0_Handler        /* Window WatchDog */
#define PVD_IRQHandler                                  IRQ1_Handler        /* PVD through EXTI Line detection */
#define TAMP_STAMP_IRQHandler                           IRQ2_Handler        /* Tamper and TimeStamp interrupts through the EXTI line */
#define RTC_WKUP_IRQHandler                             IRQ3_Handler        /* RTC Wakeup interrupt through the EXTI line */
#define FLASH_IRQHandler                                IRQ4_Handler        /* FLASH global */
#define RCC_IRQHandler                                  IRQ5_Handler        /* RCC global */
#define EXTI0_IRQHandler                                IRQ6_Handler        /* EXTI Line0 */
#define EXTI1_IRQHandler                                IRQ7_Handler        /* EXTI Line1 */
#define EXTI2_IRQHandler                                IRQ8_Handler        /* EXTI Line2 */
#define EXTI3_IRQHandler                                IRQ9_Handler        /* EXTI Line3 */

#define EXTI4_IRQHandler                                IRQ10_Handler       /* EXTI Line4 */
#define DMA1_Stream0_IRQHandler                         IRQ11_Handler       /* DMA1 Stream 0 global */
#define DMA1_Stream1_IRQHandler                         IRQ12_Handler       /* DMA1 Stream 1 global */
#define DMA1_Stream2_IRQHandler                         IRQ13_Handler       /* DMA1 Stream 2 global */
#define DMA1_Stream3_IRQHandler                         IRQ14_Handler       /* DMA1 Stream 3 global */
#define DMA1_Stream4_IRQHandler                         IRQ15_Handler       /* DMA1 Stream 4 global */
#define DMA1_Stream5_IRQHandler                         IRQ16_Handler       /* DMA1 Stream 5 global */
#define DMA1_Stream6_IRQHandler                         IRQ17_Handler       /* DMA1 Stream 6 global */
#define ADC_IRQHandler                                  IRQ18_Handler       /* ADC1, ADC2 and ADC3 global */
#define CAN1_TX_IRQHandler                              IRQ19_Handler       /* CAN1 TX */

#define CAN1_RX0_IRQHandler                             IRQ20_Handler       /* CAN1 RX0 */
#define CAN1_RX1_IRQHandler                             IRQ21_Handler       /* CAN1 RX1 */
#define CAN1_SCE_IRQHandler                             IRQ22_Handler       /* CAN1 SCE */
#define EXTI9_5_IRQHandler                              IRQ23_Handler       /* External Line[9:5] */
#define TIM1_BRK_TIM9_IRQHandler                        IRQ24_Handler       /* TIM1 Break interrupt and TIM9 global */
#define TIM1_UP_TIM10_IRQHandler                        IRQ25_Handler       /* TIM1 Update Interrupt and TIM10 global */
#define TIM1_TRG_COM_TIM11_IRQHandler                   IRQ26_Handler       /* TIM1 Trigger and Commutation Interrupt and TIM11 global */
#define TIM1_CC_IRQHandler                              IRQ27_Handler       /* TIM1 Capture Compare */
#define TIM2_IRQHandler                                 IRQ28_Handler       /* TIM2 global */
#define TIM3_IRQHandler                                 IRQ29_Handler       /* TIM3 global */

#define TIM4_IRQHandler                                 IRQ30_Handler       /* TIM4 global */
#define I2C1_EV_IRQHandler                              IRQ31_Handler       /* I2C1 Event */
#define I2C1_ER_IRQHandler                              IRQ32_Handler       /* I2C1 Error */
#define I2C2_EV_IRQHandler                              IRQ33_Handler       /* I2C2 Event */
#define I2C2_ER_IRQHandler                              IRQ34_Handler       /* I2C2 Error */
#define SPI1_IRQHandler                                 IRQ35_Handler       /* SPI1 global */
#define SPI2_IRQHandler                                 IRQ36_Handler       /* SPI2 global */
#define USART1_IRQHandler                               IRQ37_Handler       /* USART1 global */
#define USART2_IRQHandler                               IRQ38_Handler       /* USART2 global */
#define USART3_IRQHandler                               IRQ39_Handler       /* USART3 global */

#define EXTI15_10_IRQHandler                            IRQ40_Handler       /* External Line[15:10] */
#define RTC_Alarm_IRQHandler                            IRQ41_Handler       /* RTC Alarm (A and B) through EXTI Line */
#define OTG_FS_WKUP_IRQHandler                          IRQ42_Handler       /* USB OTG FS Wakeup through EXTI line */
#define TIM8_BRK_TIM12_IRQHandler                       IRQ43_Handler       /* TIM8 Break Interrupt and TIM12 global */
#define TIM8_UP_TIM13_IRQHandler                        IRQ44_Handler       /* TIM8 Update Interrupt and TIM13 global */
#define TIM8_TRG_COM_TIM14_IRQHandler                   IRQ45_Handler       /* TIM8 Trigger and Commutation Interrupt and TIM14 global */
#define TIM8_CC_IRQHandler                              IRQ46_Handler       /* TIM8 Capture Compare global */
#define DMA1_Stream7_IRQHandler                         IRQ47_Handler       /* DMA1 Stream7 */
#define FSMC_IRQHandler                                 IRQ48_Handler       /* FSMC global */
#define SDIO_IRQHandler                                 IRQ49_Handler       /* SDIO global */

#define TIM5_IRQHandler                                 IRQ50_Handler       /* TIM5 global */
#define SPI3_IRQHandler                                 IRQ51_Handler       /* SPI3 global */
#define UART4_IRQHandler                                IRQ52_Handler       /* UART4 global */
#define UART5_IRQHandler                                IRQ53_Handler       /* UART5 global */
#define TIM6_DAC_IRQHandler                             IRQ54_Handler       /* TIM6 global and DAC1&2 underrun error */
#define TIM7_IRQHandler                                 IRQ55_Handler       /* TIM7 global */
#define DMA2_Stream0_IRQHandler                         IRQ56_Handler       /* DMA2 Stream 0 global */
#define DMA2_Stream1_IRQHandler                         IRQ57_Handler       /* DMA2 Stream 1 global */
#define DMA2_Stream2_IRQHandler                         IRQ58_Handler       /* DMA2 Stream 2 global */
#define DMA2_Stream3_IRQHandler                         IRQ59_Handler       /* DMA2 Stream 3 global */

#define DMA2_Stream4_IRQHandler                         IRQ60_Handler       /* DMA2 Stream 4 global */
#define Reserved0_IRQHandler                            IRQ61_Handler       /* Reserved */
#define Reserved1_IRQHandler                            IRQ62_Handler       /* Reserved */
#define CAN2_TX_IRQHandler                              IRQ63_Handler       /* CAN2 TX */
#define CAN2_RX0_IRQHandler                             IRQ64_Handler       /* CAN2 RX0 */
#define CAN2_RX1_IRQHandler                             IRQ65_Handler       /* CAN2 RX1 */
#define CAN2_SCE_IRQHandler                             IRQ66_Handler       /* CAN2 SCE */
#define OTG_FS_IRQHandler                               IRQ67_Handler       /* USB OTG FS global */
#define DMA2_Stream5_IRQHandler                         IRQ68_Handler       /* DMA2 Stream 5 global */
#define DMA2_Stream6_IRQHandler                         IRQ69_Handler       /* DMA2 Stream 6 global */

#define DMA2_Stream7_IRQHandler                         IRQ70_Handler       /* DMA2 Stream 7 global */
#define USART6_IRQHandler                               IRQ71_Handler       /* USART6 global */
#define I2C3_EV_IRQHandler                              IRQ72_Handler       /* I2C3 event */
#define I2C3_ER_IRQHandler                              IRQ73_Handler       /* I2C3 error */
#define OTG_HS_EP1_OUT_IRQHandler                       IRQ74_Handler       /* USB OTG HS End Point 1 Out global */
#define OTG_HS_EP1_IN_IRQHandler                        IRQ75_Handler       /* USB OTG HS End Point 1 In global */
#define OTG_HS_WKUP_IRQHandler                          IRQ76_Handler       /* USB OTG HS Wakeup through EXTI */
#define OTG_HS_IRQHandler                               IRQ77_Handler       /* USB OTG HS global */
#define Reserved2_IRQHandler                            IRQ78_Handler       /* Reserved */
#define Reserved3_IRQHandler                            IRQ79_Handler       /* Reserved */

#define RNG_IRQHandler                                  IRQ80_Handler       /* RNG global */
#define FPU_IRQHandler                                  IRQ81_Handler       /* FPU global */

/* Initialization registers **************************************************/
#define RME_A7M_RCC_APB1ENR                             RME_A7M_REG(0x40023840U)
#define RME_A7M_RCC_APB1ENR_PWREN                       (1U<<28)

#define RME_A7M_PWR_CR1                                 RME_A7M_REG(0x40007000U)
#define RME_A7M_PWR_CR1_VOS_SCALE1                      (1U<<14)

#define RME_A7M_RCC_CR                                  RME_A7M_REG(0x40023800U)
#define RME_A7M_RCC_CR_HSEON                            (1U<<16)
#define RME_A7M_RCC_CR_HSERDY                           (1U<<17)
#define RME_A7M_RCC_CR_PLLON                            (1U<<24)
#define RME_A7M_RCC_CR_PLLRDY                           (1U<<25)

#define RME_A7M_RCC_PLLCFGR                             RME_A7M_REG(0x40023804U)
#define RME_A7M_RCC_PLLCFGR_SOURCE_HSE                  (1U<<22)
#define RME_A7M_RCC_PLLCFGR_PLLM(X)                     (X)
#define RME_A7M_RCC_PLLCFGR_PLLN(X)                     ((X)<<6)
#define RME_A7M_RCC_PLLCFGR_PLLP(X)                     ((((X)>>1)-1U)<<16)
#define RME_A7M_RCC_PLLCFGR_PLLQ(X)                     ((X)<<24)
#define RME_A7M_RCC_PLLCFGR_PLLR(X)                     ((X)<<28)

#define RME_A7M_RCC_CIR                                 RME_A7M_REG(0x4002380CU)

#define RME_A7M_FLASH_ACR                               RME_A7M_REG(0x40023C00U)
#define RME_A7M_FLASH_ACR_LATENCY(X)                    RME_A7M_FLASH_ACR=((RME_A7M_FLASH_ACR&~0x0FU)|(X))
#define RME_A7M_FLASH_ACR_PRFTEN                        (1U<<8)
#define RME_A7M_FLASH_ACR_ICEN                          (1U<<9)
#define RME_A7M_FLASH_ACR_DCEN                          (1U<<10)

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
#define RME_A7M_TIM_FLAG_UPDATE                         (1U<<0)

#define RME_A7M_RCC_AHB1ENR                             RME_A7M_REG(0x40023830U)
#define RME_A7M_RCC_AHB1ENR_GPIOBEN                     (1U<<1)

#define RME_A7M_RCC_APB2ENR                             RME_A7M_REG(0x40023844U)
#define RME_A7M_RCC_APB2ENR_USART1EN                    (1U<<4)

#define RME_A7M_GPIOB_MODER                             RME_A7M_REG(0x40020400U)
#define RME_A7M_GPIOB_OTYPER                            RME_A7M_REG(0x40020404U)
#define RME_A7M_GPIOB_OSPEEDR                           RME_A7M_REG(0x40020408U)
#define RME_A7M_GPIOB_PUPDR                             RME_A7M_REG(0x4002040CU)
#define RME_A7M_GPIOB_IDR                               RME_A7M_REG(0x40020410U)
#define RME_A7M_GPIOB_ODR                               RME_A7M_REG(0x40020414U)
#define RME_A7M_GPIOB_BSRR                              RME_A7M_REG(0x40020418U)
#define RME_A7M_GPIOB_LCKR                              RME_A7M_REG(0x4002041CU)
#define RME_A7M_GPIOB_AFR0                              RME_A7M_REG(0x40020420U)
#define RME_A7M_GPIOB_AFR1                              RME_A7M_REG(0x40020424U)

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

#define RME_A7M_GPIOB_MODE(MODE,PIN)                    RME_A7M_GPIOB_MODER=(RME_A7M_GPIOB_MODER&~(0x03U<<((PIN)*2)))|((MODE)<<((PIN)*2))
#define RME_A7M_GPIOB_OTYPE(OTYPE,PIN)                  RME_A7M_GPIOB_OTYPER=(RME_A7M_GPIOB_OTYPER&~(0x01U<<(PIN)))|((OTYPE)<<(PIN))
#define RME_A7M_GPIOB_OSPEED(OSPEED,PIN)                RME_A7M_GPIOB_OSPEEDR=(RME_A7M_GPIOB_OSPEEDR&~(0x03U<<((PIN)*2)))|((OSPEED)<<((PIN)*2))
#define RME_A7M_GPIOB_PUPD(PUPD,PIN)                    RME_A7M_GPIOB_PUPDR=(RME_A7M_GPIOB_PUPDR&~(0x03U<<((PIN)*2)))|((PUPD)<<((PIN)*2))
#define RME_A7M_GPIOB_AF0(AF,PIN)                       RME_A7M_GPIOB_AFR0=(RME_A7M_GPIOB_AFR0&~(0x0FU<<((PIN)*4)))|((AF)<<((PIN)*4))
#define RME_A7M_GPIOB_AF1(AF,PIN)                       RME_A7M_GPIOB_AFR1=(RME_A7M_GPIOB_AFR1&~(0x0FU<<((PIN-8)*4)))|((AF)<<((PIN-8)*4))

#define RME_A7M_USART1_SR                               RME_A7M_REG(0x40011000U)
#define RME_A7M_USART1_DR                               RME_A7M_REG(0x40011004U)
#define RME_A7M_USART1_BRR                              RME_A7M_REG(0x40011008U)
#define RME_A7M_USART1_CR1                              RME_A7M_REG(0x4001100CU)
#define RME_A7M_USART1_CR2                              RME_A7M_REG(0x40011010U)
#define RME_A7M_USART1_CR3                              RME_A7M_REG(0x40011014U)

#define RME_A7M_USART1_CR1_UE                           (1U<<13)

/* Preinitialization of critical hardware */
#define RME_A7M_LOWLVL_PREINIT() \
do \
{ \
    /* Set CP10&11 full access */ \
    RME_A7M_SCB_CPACR|=((3U<<(10U*2U))|(3U<<(11U*2U))); \
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
    while((RME_A7M_RCC_CR&RME_A7M_RCC_CR_HSERDY)==0U); \
    /* Fpll=Fin/PLLM*PLLN, Fsys=Fpll/PLLP, Fperiph=Fpll/PLLQ */ \
    RME_A7M_RCC_CR&=~RME_A7M_RCC_CR_PLLON; \
    __RME_A7M_Barrier(); \
    while((RME_A7M_RCC_CR&RME_A7M_RCC_CR_PLLRDY)!=0U); \
    RME_A7M_RCC_PLLCFGR=RME_A7M_RCC_PLLCFGR_SOURCE_HSE| \
                        RME_A7M_RCC_PLLCFGR_PLLM(RME_A7M_STM32F405RG_PLLM)| \
                        RME_A7M_RCC_PLLCFGR_PLLN(RME_A7M_STM32F405RG_PLLN)| \
                        RME_A7M_RCC_PLLCFGR_PLLP(RME_A7M_STM32F405RG_PLLP)| \
                        RME_A7M_RCC_PLLCFGR_PLLQ(RME_A7M_STM32F405RG_PLLQ); \
    __RME_A7M_Barrier(); \
    RME_A7M_RCC_CR|=RME_A7M_RCC_CR_PLLON; \
    __RME_A7M_Barrier(); \
    while((RME_A7M_RCC_CR&RME_A7M_RCC_CR_PLLRDY)==0U); \
    \
    /* HCLK,PCLK1 & PCLK2 configuration */ \
    RME_A7M_FLASH_ACR_LATENCY(5U); \
    RME_ASSERT((RME_A7M_FLASH_ACR&0x0FU)==5U); \
    RME_A7M_RCC_CFGR_PCLK1(RME_A7M_RCC_CFGR_HCLK_DIV4); \
    RME_A7M_RCC_CFGR_PCLK2(RME_A7M_RCC_CFGR_HCLK_DIV2); \
    RME_A7M_RCC_CFGR_HCLK(RME_A7M_RCC_CFGR_SYSCLK_DIV1); \
    RME_A7M_RCC_CFGR_SYSCLK(RME_A7M_RCC_CFGR_SYSCLKSOURCE_PLLCLK); \
    __RME_A7M_Barrier(); \
    \
    /* Flash ART enabling */ \
    RME_A7M_FLASH_ACR|=RME_A7M_FLASH_ACR_ICEN; \
    RME_A7M_FLASH_ACR|=RME_A7M_FLASH_ACR_DCEN; \
    RME_A7M_FLASH_ACR|=RME_A7M_FLASH_ACR_PRFTEN; \
} \
while(0)

#if(RME_DEBUG_PRINT==1U)
/* Other low-level initialization stuff - clock and serial */
#define RME_A7M_LOWLVL_INIT() \
do \
{ \
    RME_A7M_LOWLVL_INIT_COMMON(); \
    \
    /* Enable USART 1 for user-level operations */ \
    /* UART IO initialization */ \
    RME_A7M_RCC_AHB1ENR|=RME_A7M_RCC_AHB1ENR_GPIOBEN; \
    RME_A7M_GPIOB_MODE(RME_A7M_GPIO_MODE_ALTERNATE, 6U); \
    RME_A7M_GPIOB_OTYPE(RME_A7M_GPIO_OTYPE_PUSHPULL, 6U); \
    RME_A7M_GPIOB_OSPEED(RME_A7M_GPIO_OSPEED_HIGH, 6U); \
    RME_A7M_GPIOB_PUPD(RME_A7M_GPIO_PUPD_PULLUP ,6U); \
    RME_A7M_GPIOB_AF0(RME_A7M_GPIO_AF7_USART1, 6U); \
    \
    /* UART initialization */ \
    RME_A7M_RCC_APB2ENR|=RME_A7M_RCC_APB2ENR_USART1EN; \
    RME_A7M_USART1_CR1=0x0000008U; \
    RME_A7M_USART1_CR2=0x00U; \
    RME_A7M_USART1_CR3=0x00U; \
    RME_A7M_USART1_BRR=0x2D9U; \
    RME_A7M_USART1_CR1|=RME_A7M_USART1_CR1_UE; \
} \
while(0)

/* This is for debugging output */
#define RME_A7M_PUTCHAR(CHAR) \
do \
{ \
    RME_A7M_USART1_DR=(rme_ptr_t)(CHAR); \
    while((RME_A7M_USART1_SR&0x80U)==0U); \
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
        RME_A7M_FLASH_ACR|=RME_A7M_FLASH_ACR_ICEN; \
        RME_A7M_FLASH_ACR|=RME_A7M_FLASH_ACR_DCEN; \
        RME_A7M_FLASH_ACR|=RME_A7M_FLASH_ACR_PRFTEN; \
    } \
    else \
    { \
        RME_A7M_FLASH_ACR&=~RME_A7M_FLASH_ACR_PRFTEN; \
        RME_A7M_FLASH_ACR&=~RME_A7M_FLASH_ACR_DCEN; \
        RME_A7M_FLASH_ACR&=~RME_A7M_FLASH_ACR_ICEN; \
    } \
} \
while(0)
    
#define RME_A7M_PRFTH_STATE_GET() ((RME_A7M_FLASH_ACR&RME_A7M_FLASH_ACR_ICEN)!=0U)
/* End Defines ***************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/

