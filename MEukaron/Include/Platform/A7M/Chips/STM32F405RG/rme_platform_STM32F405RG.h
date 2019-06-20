/******************************************************************************
Filename   : rme_platform_STM32F405RG.h
Author     : pry
Date       : 24/06/2017
Licence    : LGPL v3+; see COPYING for details.
Description: The configuration file for STM32F405RG.
******************************************************************************/

/* Defines *******************************************************************/
/* The HAL library */
#include "stm32f4xx.h"
#include "core_cm4.h"
#include "stm32f4xx_hal.h"
/* The virtual memory start address for the kernel objects */
#define RME_KMEM_VA_START            0x10002000
/* The size of the kernel object virtual memory */
#define RME_KMEM_SIZE                0x6000
/* The virtual memory start address for the virtual machines - If no virtual machines is used, set to 0 */
#define RME_HYP_VA_START             0x20000000
/* The size of the hypervisor reserved virtual memory */
#define RME_HYP_SIZE                 0x20000
/* The granularity of kernel memory allocation, in bytes */
#define RME_KMEM_SLOT_ORDER          4
/* Kernel stack size and address */
#define RME_KMEM_STACK_ADDR          0x10000FF0
/* The maximum number of preemption priority levels in the system.
 * This parameter must be divisible by the word length - 32 is usually sufficient */
#define RME_MAX_PREEMPT_PRIO         32

/* Shared interrupt flag region address - always use 256*4 = 1kB memory */
#define RME_A7M_INT_FLAG_ADDR        0x10008000
/* Initial kenel object frontier limit */
#define RME_A7M_KMEM_BOOT_FRONTIER   0x10003000
/* Number of MPU regions available */
#define RME_A7M_MPU_REGIONS          8
/* Init process's first thread's entry point address */
#define RME_A7M_INIT_ENTRY           0x08004001
/* Init process's first thread's stack address */
#define RME_A7M_INIT_STACK           0x1000FFF0
/* What is the FPU type? */
#define RME_A7M_FPU_TYPE             RME_A7M_FPU_VFPV4
/* What is the NVIC priority grouping? */
#define RME_A7M_NVIC_GROUPING        RME_A7M_NVIC_GROUPING_P2S6
/* What is the Systick value? - 10ms per tick*/
#define RME_A7M_SYSTICK_VAL          1680000

/* Kernel functions standard to Cortex-M, interrupt management and power */
#define RME_A7M_KERN_INT(X)          (X)
#define RME_A7M_INT_OP               0
#define RME_A7M_INT_ENABLE           1
#define RME_A7M_INT_DISABLE          0
#define RME_A7M_INT_PRIO             1
#define RME_A7M_KERN_PWR             240

/* Other low-level initialization stuff - The serial port */
#define RME_A7M_LOW_LEVEL_INIT() \
do \
{ \
    RCC_ClkInitTypeDef RCC_ClkInitStruct; \
    RCC_OscInitTypeDef RCC_OscInitStruct; \
    UART_HandleTypeDef UART1_Handle; \
    GPIO_InitTypeDef GPIO_Init; \
    _RME_Clear(&RCC_ClkInitStruct, sizeof(RCC_ClkInitStruct)); \
    _RME_Clear(&RCC_OscInitStruct, sizeof(RCC_OscInitStruct)); \
    _RME_Clear(&UART1_Handle, sizeof(UART_HandleTypeDef)); \
    _RME_Clear(&GPIO_Init, sizeof(GPIO_InitTypeDef)); \
    \
    /* Enable Power Control clock */ \
    __HAL_RCC_PWR_CLK_ENABLE(); \
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1); \
    \
    /* Enable HSE Oscillator and activate PLL with HSE as source */ \
    RCC_OscInitStruct.OscillatorType=RCC_OSCILLATORTYPE_HSE; \
    RCC_OscInitStruct.HSEState=RCC_HSE_ON; \
    RCC_OscInitStruct.PLL.PLLState=RCC_PLL_ON; \
    RCC_OscInitStruct.PLL.PLLSource=RCC_PLLSOURCE_HSE; \
    RCC_OscInitStruct.PLL.PLLM=8; \
    RCC_OscInitStruct.PLL.PLLN=336; \
    RCC_OscInitStruct.PLL.PLLP=RCC_PLLP_DIV2; \
    RCC_OscInitStruct.PLL.PLLQ=7; \
    HAL_RCC_OscConfig(&RCC_OscInitStruct); \
    \
    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */ \
    RCC_ClkInitStruct.ClockType=(RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2); \
    RCC_ClkInitStruct.SYSCLKSource=RCC_SYSCLKSOURCE_PLLCLK; \
    RCC_ClkInitStruct.AHBCLKDivider=RCC_SYSCLK_DIV1; \
    RCC_ClkInitStruct.APB1CLKDivider=RCC_HCLK_DIV4; \
    RCC_ClkInitStruct.APB2CLKDivider=RCC_HCLK_DIV2; \
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5); \
    __HAL_FLASH_INSTRUCTION_CACHE_ENABLE(); \
    __HAL_FLASH_DATA_CACHE_ENABLE(); \
    /* STM32F405x/407x/415x/417x Revision Z/1/2 devices: prefetch is supported */ \
    if(HAL_GetREVID()!=0x1000) \
        __HAL_FLASH_PREFETCH_BUFFER_ENABLE();\
    \
    /* Enable USART 1 for user-level operations */ \
    /* Clock enabling */ \
    __HAL_RCC_GPIOB_CLK_ENABLE(); \
	__HAL_RCC_USART1_CLK_ENABLE(); \
    /* UART IO initialization */ \
	GPIO_Init.Pin=GPIO_PIN_6; \
	GPIO_Init.Mode=GPIO_MODE_AF_PP; \
	GPIO_Init.Pull=GPIO_PULLUP; \
	GPIO_Init.Speed=GPIO_SPEED_FREQ_HIGH; \
	GPIO_Init.Alternate=GPIO_AF7_USART1; \
	HAL_GPIO_Init(GPIOB,&GPIO_Init); \
    /* UART initialization */ \
	UART1_Handle.Instance=USART1; \
	UART1_Handle.Init.BaudRate=115200; \
	UART1_Handle.Init.WordLength=UART_WORDLENGTH_8B; \
	UART1_Handle.Init.StopBits=UART_STOPBITS_1; \
	UART1_Handle.Init.Parity=UART_PARITY_NONE; \
	UART1_Handle.Init.HwFlowCtl=UART_HWCONTROL_NONE; \
	UART1_Handle.Init.Mode=UART_MODE_TX; \
	HAL_UART_Init(&UART1_Handle); \
} \
while(0)

/* This is for debugging output */
#define RME_A7M_PUTCHAR(CHAR) \
do \
{ \
    ITM_SendChar((rme_s8_t)(CHAR)); \
} \
while(0)
/* End Defines ***************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
