/******************************************************************************
Filename   : rme_platform_ch32v307vc_gcc.s
Author     : pry
Date       : 24/06/2017
Licence    : The Unlicense; see LICENSE for details.
Description: The boot stub file for CH32V307VC.
             This processor have non-standard CSRs called GINTENR and INTSYSCR,
             and these are URW. This gives user-level denial-of-service attack
             capability. For serious applications, the binaries must be scanned
             to confirm that they do not contain any CSR writes.
******************************************************************************/

/* Begin Exports *************************************************************/
    .global         _start
    .global         __RME_Start
/* End Exports ***************************************************************/

/* Begin Imports *************************************************************/
    .extern         __RME_Stack
    .extern         __RME_Global
    .extern         __RME_DATA_Load
    .extern         __RME_DATA_Start
    .extern         __RME_DATA_End
    .extern         __RME_BSS_Start
    .extern         __RME_BSS_End
    .extern         __RME_RV32GP_Handler
    .extern         __RME_RV32GP_Lowlvl_Preinit
    .extern         __RME_Kmain
/* End Imports ***************************************************************/

/* Begin Startup *************************************************************/
    .section        .text.reset,"ax",@progbits
__RME_Start:
    J               __RME_Init
    /* Weird constants that the architecture would require */
    .word           0x00000013
    .word           0x00000013
    .word           0x00000013
    .word           0x00000013
    .word           0x00000013
    .word           0x00000013
    .word           0x00000013
    .word           0x00000013
    .word           0x00000013
    .word           0x00000013
    .word           0x00000013
    .word           0x00000013
    .word           0x00100073
    .align          1
    /* Initialization */
_start:
__RME_Init:
.option push
.option norelax
    LA              gp,__RME_Global
.option pop
    LA              sp,__RME_Stack
    /* Load data section from flash to RAM */
    LA              a0,__RME_DATA_Load
    LA              a1,__RME_DATA_Start
    LA              a2,__RME_DATA_End
__RME_DATA_Load:
    LW              t0,(a0)
    SW              t0,(a1)
    ADDI            a0,a0,4
    ADDI            a1,a1,4
    BLTU            a1,a2,__RME_DATA_Load
    /* Clear bss section */
    LA              a0,__RME_BSS_Start
    LA              a1,__RME_BSS_End
__RME_BSS_Clear:
    SW              zero,(a0)
    ADDI            a0,a0,4
    BLTU            a0,a1,__RME_BSS_Clear
    /* Chip-specific CORECFGR(0x0BC0): default value 0x1F */
    LI              t0,0x1F
    CSRW            0x0BC0,t0
    /* Chip-specific INTSYSCR(0x0804): no hardware stack */
    LI              t0,0x0000E00E
    CSRW            0x0804,t0
    /* Machine mode forever, FPU disabled for now */
    LI              t0,0x1888
    CSRS            mstatus,t0
    /* Set vector table address */
    la              t0,__RME_Vector
    ORI             t0,t0,3
    CSRW            mtvec,t0
    /* Jump to kernel entry */
    LA              t0,main
    LA              a0,__RME_Stack
    CSRW            mepc,t0
    MRET
/* End Startup ***************************************************************/

/* Begin Vector Table ********************************************************/
    .section        .text.vector,"ax",@progbits
    .align          1
__RME_Vector:
    /* System vectors */
    .option         norvc;
    .word           __RME_Start
    .word           0
    .word           NMI_Handler                 /* NMI */
    .word           HardFault_Handler           /* Hard Fault */
    .word           0
    .word           Ecall_M_Mode_Handler        /* Ecall M Mode */
    .word           0
    .word           0
    .word           Ecall_U_Mode_Handler        /* Ecall U Mode */
    .word           Break_Point_Handler         /* Break Point */
    .word           0
    .word           0
    .word           SysTick_Handler             /* SysTick */
    .word           0
    .word           SW_Handler                  /* SW */
    .word           0
    /* External Interrupts */
    .word           WWDG_IRQHandler             /* Window Watchdog */
    .word           PVD_IRQHandler              /* PVD through EXTI Line detect */
    .word           TAMPER_IRQHandler           /* TAMPER */
    .word           RTC_IRQHandler              /* RTC */
    .word           FLASH_IRQHandler            /* Flash */
    .word           RCC_IRQHandler              /* RCC */
    .word           EXTI0_IRQHandler            /* EXTI Line 0 */
    .word           EXTI1_IRQHandler            /* EXTI Line 1 */
    .word           EXTI2_IRQHandler            /* EXTI Line 2 */
    .word           EXTI3_IRQHandler            /* EXTI Line 3 */
    .word           EXTI4_IRQHandler            /* EXTI Line 4 */
    .word           DMA1_Channel1_IRQHandler    /* DMA1 Channel 1 */
    .word           DMA1_Channel2_IRQHandler    /* DMA1 Channel 2 */
    .word           DMA1_Channel3_IRQHandler    /* DMA1 Channel 3 */
    .word           DMA1_Channel4_IRQHandler    /* DMA1 Channel 4 */
    .word           DMA1_Channel5_IRQHandler    /* DMA1 Channel 5 */
    .word           DMA1_Channel6_IRQHandler    /* DMA1 Channel 6 */
    .word           DMA1_Channel7_IRQHandler    /* DMA1 Channel 7 */
    .word           ADC1_2_IRQHandler           /* ADC1_2 */
    .word           USB_HP_CAN1_TX_IRQHandler   /* USB HP and CAN1 TX */
    .word           USB_LP_CAN1_RX0_IRQHandler  /* USB LP and CAN1RX0 */
    .word           CAN1_RX1_IRQHandler         /* CAN1 RX1 */
    .word           CAN1_SCE_IRQHandler         /* CAN1 SCE */
    .word           EXTI9_5_IRQHandler          /* EXTI Line 9..5 */
    .word           TIM1_BRK_IRQHandler         /* TIM1 Break */
    .word           TIM1_UP_IRQHandler          /* TIM1 Update */
    .word           TIM1_TRG_COM_IRQHandler     /* TIM1 Trigger and Commutation */
    .word           TIM1_CC_IRQHandler          /* TIM1 Capture Compare */
    .word           TIM2_IRQHandler             /* TIM2 */
    .word           TIM3_IRQHandler             /* TIM3 */
    .word           TIM4_IRQHandler             /* TIM4 */
    .word           I2C1_EV_IRQHandler          /* I2C1 Event */
    .word           I2C1_ER_IRQHandler          /* I2C1 Error */
    .word           I2C2_EV_IRQHandler          /* I2C2 Event */
    .word           I2C2_ER_IRQHandler          /* I2C2 Error */
    .word           SPI1_IRQHandler             /* SPI1 */
    .word           SPI2_IRQHandler             /* SPI2 */
    .word           USART1_IRQHandler           /* USART1 */
    .word           USART2_IRQHandler           /* USART2 */
    .word           USART3_IRQHandler           /* USART3 */
    .word           EXTI15_10_IRQHandler        /* EXTI Line 15..10 */
    .word           RTCAlarm_IRQHandler         /* RTC Alarm through EXTI Line */
    .word           USBWakeUp_IRQHandler        /* USB Wakeup from suspend */
    .word           TIM8_BRK_IRQHandler         /* TIM8 Break */
    .word           TIM8_UP_IRQHandler          /* TIM8 Update */
    .word           TIM8_TRG_COM_IRQHandler     /* TIM8 Trigger and Commutation */
    .word           TIM8_CC_IRQHandler          /* TIM8 Capture Compare */
    .word           RNG_IRQHandler              /* RNG */
    .word           FSMC_IRQHandler             /* FSMC */
    .word           SDIO_IRQHandler             /* SDIO */
    .word           TIM5_IRQHandler             /* TIM5 */
    .word           SPI3_IRQHandler             /* SPI3 */
    .word           UART4_IRQHandler            /* UART4 */
    .word           UART5_IRQHandler            /* UART5 */
    .word           TIM6_IRQHandler             /* TIM6 */
    .word           TIM7_IRQHandler             /* TIM7 */
    .word           DMA2_Channel1_IRQHandler    /* DMA2 Channel 1 */
    .word           DMA2_Channel2_IRQHandler    /* DMA2 Channel 2 */
    .word           DMA2_Channel3_IRQHandler    /* DMA2 Channel 3 */
    .word           DMA2_Channel4_IRQHandler    /* DMA2 Channel 4 */
    .word           DMA2_Channel5_IRQHandler    /* DMA2 Channel 5 */
    .word           ETH_IRQHandler              /* ETH */
    .word           ETH_WKUP_IRQHandler         /* ETH WakeUp */
    .word           CAN2_TX_IRQHandler          /* CAN2 TX */
    .word           CAN2_RX0_IRQHandler         /* CAN2 RX0 */
    .word           CAN2_RX1_IRQHandler         /* CAN2 RX1 */
    .word           CAN2_SCE_IRQHandler         /* CAN2 SCE */
    .word           OTG_FS_IRQHandler           /* OTGFS */
    .word           USBHSWakeup_IRQHandler      /* USBHS Wakeup */
    .word           USBHS_IRQHandler            /* USBHS */
    .word           DVP_IRQHandler              /* DVP */
    .word           UART6_IRQHandler            /* UART6 */
    .word           UART7_IRQHandler            /* UART7 */
    .word           UART8_IRQHandler            /* UART8 */
    .word           TIM9_BRK_IRQHandler         /* TIM9 Break */
    .word           TIM9_UP_IRQHandler          /* TIM9 Update */
    .word           TIM9_TRG_COM_IRQHandler     /* TIM9 Trigger and Commutation */
    .word           TIM9_CC_IRQHandler          /* TIM9 Capture Compare */
    .word           TIM10_BRK_IRQHandler        /* TIM10 Break */
    .word           TIM10_UP_IRQHandler         /* TIM10 Update */
    .word           TIM10_TRG_COM_IRQHandler    /* TIM10 Trigger and Commutation */
    .word           TIM10_CC_IRQHandler         /* TIM10 Capture Compare */
    .word           DMA2_Channel6_IRQHandler    /* DMA2 Channel 6 */
    .word           DMA2_Channel7_IRQHandler    /* DMA2 Channel 7 */
    .word           DMA2_Channel8_IRQHandler    /* DMA2 Channel 8 */
    .word           DMA2_Channel9_IRQHandler    /* DMA2 Channel 9 */
    .word           DMA2_Channel10_IRQHandler   /* DMA2 Channel 10 */
    .word           DMA2_Channel11_IRQHandler   /* DMA2 Channel 11 */
/* End Vector Table **********************************************************/

/* Begin Handler *************************************************************/
    .section        .text.handler, "ax", @progbits
    .align          1
    .option         rvc;
    .weak           NMI_Handler
    .weak           HardFault_Handler
    .weak           Ecall_M_Mode_Handler
    .weak           Ecall_U_Mode_Handler
    .weak           Break_Point_Handler
    .weak           SysTick_Handler
    .weak           SW_Handler
    .weak           WWDG_IRQHandler
    .weak           PVD_IRQHandler
    .weak           TAMPER_IRQHandler
    .weak           RTC_IRQHandler
    .weak           FLASH_IRQHandler
    .weak           RCC_IRQHandler
    .weak           EXTI0_IRQHandler
    .weak           EXTI1_IRQHandler
    .weak           EXTI2_IRQHandler
    .weak           EXTI3_IRQHandler
    .weak           EXTI4_IRQHandler
    .weak           DMA1_Channel1_IRQHandler
    .weak           DMA1_Channel2_IRQHandler
    .weak           DMA1_Channel3_IRQHandler
    .weak           DMA1_Channel4_IRQHandler
    .weak           DMA1_Channel5_IRQHandler
    .weak           DMA1_Channel6_IRQHandler
    .weak           DMA1_Channel7_IRQHandler
    .weak           ADC1_2_IRQHandler
    .weak           USB_HP_CAN1_TX_IRQHandler
    .weak           USB_LP_CAN1_RX0_IRQHandler
    .weak           CAN1_RX1_IRQHandler
    .weak           CAN1_SCE_IRQHandler
    .weak           EXTI9_5_IRQHandler
    .weak           TIM1_BRK_IRQHandler
    .weak           TIM1_UP_IRQHandler
    .weak           TIM1_TRG_COM_IRQHandler
    .weak           TIM1_CC_IRQHandler
    .weak           TIM2_IRQHandler
    .weak           TIM3_IRQHandler
    .weak           TIM4_IRQHandler
    .weak           I2C1_EV_IRQHandler
    .weak           I2C1_ER_IRQHandler
    .weak           I2C2_EV_IRQHandler
    .weak           I2C2_ER_IRQHandler
    .weak           SPI1_IRQHandler
    .weak           SPI2_IRQHandler
    .weak           USART1_IRQHandler
    .weak           USART2_IRQHandler
    .weak           USART3_IRQHandler
    .weak           EXTI15_10_IRQHandler
    .weak           RTCAlarm_IRQHandler
    .weak           USBWakeUp_IRQHandler
    .weak           TIM8_BRK_IRQHandler
    .weak           TIM8_UP_IRQHandler
    .weak           TIM8_TRG_COM_IRQHandler
    .weak           TIM8_CC_IRQHandler
    .weak           RNG_IRQHandler
    .weak           FSMC_IRQHandler
    .weak           SDIO_IRQHandler            /* SDIO */
    .weak           TIM5_IRQHandler            /* TIM5 */
    .weak           SPI3_IRQHandler            /* SPI3 */
    .weak           UART4_IRQHandler           /* UART4 */
    .weak           UART5_IRQHandler           /* UART5 */
    .weak           TIM6_IRQHandler            /* TIM6 */
    .weak           TIM7_IRQHandler            /* TIM7 */
    .weak           DMA2_Channel1_IRQHandler   /* DMA2 Channel 1 */
    .weak           DMA2_Channel2_IRQHandler   /* DMA2 Channel 2 */
    .weak           DMA2_Channel3_IRQHandler   /* DMA2 Channel 3 */
    .weak           DMA2_Channel4_IRQHandler   /* DMA2 Channel 4 */
    .weak           DMA2_Channel5_IRQHandler   /* DMA2 Channel 5 */
    .weak           ETH_IRQHandler             /* ETH */
    .weak           ETH_WKUP_IRQHandler        /* ETH WakeUp */
    .weak           CAN2_TX_IRQHandler         /* CAN2 TX */
    .weak           CAN2_RX0_IRQHandler        /* CAN2 RX0 */
    .weak           CAN2_RX1_IRQHandler        /* CAN2 RX1 */
    .weak           CAN2_SCE_IRQHandler        /* CAN2 SCE */
    .weak           OTG_FS_IRQHandler          /* OTGFS */
    .weak           USBHSWakeup_IRQHandler     /* USBHS Wakeup */
    .weak           USBHS_IRQHandler           /* USBHS */
    .weak           DVP_IRQHandler             /* DVP */
    .weak           UART6_IRQHandler           /* UART6 */
    .weak           UART7_IRQHandler           /* UART7 */
    .weak           UART8_IRQHandler           /* UART8 */
    .weak           TIM9_BRK_IRQHandler        /* TIM9 Break */
    .weak           TIM9_UP_IRQHandler         /* TIM9 Update */
    .weak           TIM9_TRG_COM_IRQHandler    /* TIM9 Trigger and Commutation */
    .weak           TIM9_CC_IRQHandler         /* TIM9 Capture Compare */
    .weak           TIM10_BRK_IRQHandler       /* TIM10 Break */
    .weak           TIM10_UP_IRQHandler        /* TIM10 Update */
    .weak           TIM10_TRG_COM_IRQHandler   /* TIM10 Trigger and Commutation */
    .weak           TIM10_CC_IRQHandler        /* TIM10 Capture Compare */
    .weak           DMA2_Channel6_IRQHandler   /* DMA2 Channel 6 */
    .weak           DMA2_Channel7_IRQHandler   /* DMA2 Channel 7 */
    .weak           DMA2_Channel8_IRQHandler   /* DMA2 Channel 8 */
    .weak           DMA2_Channel9_IRQHandler   /* DMA2 Channel 9 */
    .weak           DMA2_Channel10_IRQHandler  /* DMA2 Channel 10 */
    .weak           DMA2_Channel11_IRQHandler  /* DMA2 Channel 11 */

NMI_Handler:
HardFault_Handler:
Ecall_M_Mode_Handler:
Ecall_U_Mode_Handler:
Break_Point_Handler:
SysTick_Handler:
SW_Handler:
WWDG_IRQHandler:
PVD_IRQHandler:
TAMPER_IRQHandler:
RTC_IRQHandler:
FLASH_IRQHandler:
RCC_IRQHandler:
EXTI0_IRQHandler:
EXTI1_IRQHandler:
EXTI2_IRQHandler:
EXTI3_IRQHandler:
EXTI4_IRQHandler:
DMA1_Channel1_IRQHandler:
DMA1_Channel2_IRQHandler:
DMA1_Channel3_IRQHandler:
DMA1_Channel4_IRQHandler:
DMA1_Channel5_IRQHandler:
DMA1_Channel6_IRQHandler:
DMA1_Channel7_IRQHandler:
ADC1_2_IRQHandler:
USB_HP_CAN1_TX_IRQHandler:
USB_LP_CAN1_RX0_IRQHandler:
CAN1_RX1_IRQHandler:
CAN1_SCE_IRQHandler:
EXTI9_5_IRQHandler:
TIM1_BRK_IRQHandler:
TIM1_UP_IRQHandler:
TIM1_TRG_COM_IRQHandler:
TIM1_CC_IRQHandler:
TIM2_IRQHandler:
TIM3_IRQHandler:
TIM4_IRQHandler:
I2C1_EV_IRQHandler:
I2C1_ER_IRQHandler:
I2C2_EV_IRQHandler:
I2C2_ER_IRQHandler:
SPI1_IRQHandler:
SPI2_IRQHandler:
USART1_IRQHandler:
USART2_IRQHandler:
USART3_IRQHandler:
EXTI15_10_IRQHandler:
RTCAlarm_IRQHandler:
USBWakeUp_IRQHandler:
TIM8_BRK_IRQHandler:
TIM8_UP_IRQHandler:
TIM8_TRG_COM_IRQHandler:
TIM8_CC_IRQHandler:
RNG_IRQHandler:
FSMC_IRQHandler:
SDIO_IRQHandler:
TIM5_IRQHandler:
SPI3_IRQHandler:
UART4_IRQHandler:
UART5_IRQHandler:
TIM6_IRQHandler:
TIM7_IRQHandler:
DMA2_Channel1_IRQHandler:
DMA2_Channel2_IRQHandler:
DMA2_Channel3_IRQHandler:
DMA2_Channel4_IRQHandler:
DMA2_Channel5_IRQHandler:
ETH_IRQHandler:
ETH_WKUP_IRQHandler:
CAN2_TX_IRQHandler:
CAN2_RX0_IRQHandler:
CAN2_RX1_IRQHandler:
CAN2_SCE_IRQHandler:
OTG_FS_IRQHandler:
USBHSWakeup_IRQHandler:
USBHS_IRQHandler:
DVP_IRQHandler:
UART6_IRQHandler:
UART7_IRQHandler:
UART8_IRQHandler:
TIM9_BRK_IRQHandler:
TIM9_UP_IRQHandler:
TIM9_TRG_COM_IRQHandler:
TIM9_CC_IRQHandler:
TIM10_BRK_IRQHandler:
TIM10_UP_IRQHandler:
TIM10_TRG_COM_IRQHandler:
TIM10_CC_IRQHandler:
DMA2_Channel6_IRQHandler:
DMA2_Channel7_IRQHandler:
DMA2_Channel8_IRQHandler:
DMA2_Channel9_IRQHandler:
DMA2_Channel10_IRQHandler:
DMA2_Channel11_IRQHandler:
    J               __RME_RV32P_Handler
/* End Handler ***************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
