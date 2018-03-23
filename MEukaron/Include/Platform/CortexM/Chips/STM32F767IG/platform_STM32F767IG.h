/******************************************************************************
Filename   : platform_STM32F767IG.h
Author     : pry
Date       : 24/06/2017
Licence    : LGPL v3+; see COPYING for details.
Description: The configuration file for STM32F767IG.
******************************************************************************/

/* Defines *******************************************************************/
/* The HAL library */
#include "stm32f7xx.h"
#include "core_cm7.h"
#include "stm32f7xx_hal.h"
/* The virtual memory start address for the kernel objects */
#define RME_KMEM_VA_START            0x20004000
/* The size of the kernel object virtual memory */
#define RME_KMEM_SIZE                0xC000
/* The virtual memory start address for the virtual machines - If no virtual machines is used, set to 0 */
#define RME_HYP_VA_START             0x20020000
/* The size of the hypervisor reserved virtual memory */
#define RME_HYP_SIZE                 0x60000
/* The granularity of kernel memory allocation, in bytes */
#define RME_KMEM_SLOT_ORDER          4
/* Kernel stack size and address */
#define RME_KMEM_STACK_ADDR          0x20001FF0
/* The maximum number of preemption priority levels in the system.
 * This parameter must be divisible by the word length - 32 is usually sufficient */
#define RME_MAX_PREEMPT_PRIO         32

/* Initial kenel object frontier limit */
#define RME_CMX_KMEM_BOOT_FRONTIER   0x20005000
/* Number of MPU regions available */
#define RME_CMX_MPU_REGIONS          8
/* Init process's first thread's entry point address */
#define RME_CMX_INIT_ENTRY           0x08010001
/* Init process's first thread's stack address */
#define RME_CMX_INIT_STACK           0x2001FFF0
/* What is the FPU type? */
#define RME_CMX_FPU_TYPE             RME_CMX_FPV5_DP
/* What is the NVIC priority grouping? */
#define RME_CMX_NVIC_GROUPING        RME_CMX_NVIC_GROUPING_P2S6
/* What is the Systick value? - 10ms per tick*/
#define RME_CMX_SYSTICK_VAL          2160000

/* Other low-level initialization stuff - clock and serial
 * STM32F7xx APB1<45MHz, APB2<90MHz. When running at 216MHz,
 * actually we are overdriving the bus a little, which might
 * be fine. */
#define RME_CMX_LOW_LEVEL_INIT() \
do \
{ \
    RCC_OscInitTypeDef RCC_OscInitStructure; \
    RCC_ClkInitTypeDef RCC_ClkInitStructure; \
	GPIO_InitTypeDef GPIO_Init; \
    UART_HandleTypeDef UART1_Handle; \
    _RME_Clear(&RCC_OscInitStructure, sizeof(RCC_OscInitTypeDef)); \
    _RME_Clear(&RCC_ClkInitStructure, sizeof(RCC_ClkInitTypeDef)); \
    _RME_Clear(&GPIO_Init, sizeof(GPIO_InitTypeDef)); \
    _RME_Clear(&UART1_Handle, sizeof(UART_HandleTypeDef)); \
    \
    /* Set the clock tree in the system */ \
    /* Enble power regulator clock, and configure voltage scaling function */ \
    __HAL_RCC_PWR_CLK_ENABLE(); \
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1); \
    /* Initialize the oscillator */ \
    RCC_OscInitStructure.OscillatorType=RCC_OSCILLATORTYPE_HSE; \
    RCC_OscInitStructure.HSEState=RCC_HSE_ON; \
    RCC_OscInitStructure.PLL.PLLState=RCC_PLL_ON; \
    RCC_OscInitStructure.PLL.PLLSource=RCC_PLLSOURCE_HSE; \
    /* Fpll=Fin/PLLM*PLLN, Fsys=Fpll/PLLP, Fperiph=Fpll/PLLQ */ \
    RCC_OscInitStructure.PLL.PLLM=25; \
    RCC_OscInitStructure.PLL.PLLN=432; \
    RCC_OscInitStructure.PLL.PLLP=2; \
    RCC_OscInitStructure.PLL.PLLQ=9; \
    RME_ASSERT(HAL_RCC_OscConfig(&RCC_OscInitStructure)==HAL_OK); \
    /* Overdrive to 216MHz */ \
    RME_ASSERT(HAL_PWREx_EnableOverDrive()==HAL_OK); \
    \
    /* HCLK,PCLK1 & PCLK2 configuration */ \
    RCC_ClkInitStructure.ClockType=(RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2); \
    RCC_ClkInitStructure.SYSCLKSource=RCC_SYSCLKSOURCE_PLLCLK; \
    RCC_ClkInitStructure.AHBCLKDivider=RCC_SYSCLK_DIV1; \
    RCC_ClkInitStructure.APB1CLKDivider=RCC_HCLK_DIV4; \
    RCC_ClkInitStructure.APB2CLKDivider=RCC_HCLK_DIV2; \
    /* Flash latency = 7us, 8 CPU cycles */ \
    RME_ASSERT(HAL_RCC_ClockConfig(&RCC_ClkInitStructure,FLASH_LATENCY_7)==HAL_OK); \
    \
    /* Cache/Flash ART enabling */ \
    SCB_EnableICache(); \
    SCB_EnableDCache(); \
    __HAL_FLASH_ART_ENABLE(); \
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE(); \
    \
    /* Enable USART 1 for user-level operations */ \
    /* Clock enabling */ \
    __HAL_RCC_GPIOA_CLK_ENABLE(); \
	__HAL_RCC_USART1_CLK_ENABLE(); \
    /* UART IO initialization */ \
	GPIO_Init.Pin=GPIO_PIN_9; \
	GPIO_Init.Mode=GPIO_MODE_AF_PP; \
	GPIO_Init.Pull=GPIO_PULLUP; \
	GPIO_Init.Speed=GPIO_SPEED_HIGH; \
	GPIO_Init.Alternate=GPIO_AF7_USART1; \
	HAL_GPIO_Init(GPIOA,&GPIO_Init); \
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
    

//	GPIO_Init.Pin=GPIO_PIN_10; \
//	HAL_GPIO_Init(GPIOA,&GPIO_Init); \

/* This is for debugging output */
#define RME_CMX_PUTCHAR(CHAR) \
do \
{ \
    ITM_SendChar((s8)(CHAR)); \
} \
while(0)
/* End Defines ***************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
