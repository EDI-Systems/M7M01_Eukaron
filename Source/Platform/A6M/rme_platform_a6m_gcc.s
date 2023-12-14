/******************************************************************************
Filename    : rme_platform_a6m_asm_gcc.S
Author      : pry
Date        : 19/01/2017
Description : The Cortex-M assembly support of the RME RTOS, for gcc.
******************************************************************************/

/* The ARM Cortex-M3/4/7 Architecture *****************************************
R0-R7:General purpose registers that are accessible. 
R8-R12:general purpose registers that can only be reached by 32-bit instructions.
R13:SP/SP_process/SP_main    Stack pointer
R14:LR                       Link Register(used for returning from a subfunction)
R15:PC                       Program counter.
IPSR                         Interrupt Program Status Register.
APSR                         Application Program Status Register.
EPSR                         Execute Program Status Register.
The above 3 registers are saved into the stack in combination(xPSR).
******************************************************************************/
    .syntax             unified
    .arch               armv6s-m
    .thumb
/* Import ********************************************************************/
    /* Locations provided by the linker */
    .extern             __RME_Stack
    .extern             __RME_Data_Load
    .extern             __RME_Data_Start
    .extern             __RME_Data_End
    .extern             __RME_Zero_Start
    .extern             __RME_Zero_End
    /* Kernel entry */
    .extern             main
    /* Preinitialization routine */
    .extern             __RME_A6M_Lowlvl_Preinit
    /* The system call handler */
    .extern             __RME_A6M_Svc_Handler
    /* The system tick handler */
    .extern             __RME_A6M_Tim_Handler
    /* The memory management fault handler */
    .extern             __RME_A6M_Exc_Handler
    /* The generic interrupt handler for all other vectors. */
    .extern             __RME_A6M_Vct_Handler
/* End Import ****************************************************************/

/* Export ********************************************************************/
    /* Entry point */
    .global             __RME_Entry
    /* Disable all interrupts */
    .global             __RME_Int_Disable
    /* Enable all interrupts */
    .global             __RME_Int_Enable
    /* A full barrier */
    .global             __RME_A6M_Barrier
    /* Full system reset */
    .global             __RME_A6M_Reset
    /* Wait until interrupts happen */
    .global             __RME_A6M_Wait_Int
    /* Entering of the user mode */
    .global             __RME_User_Enter
    /* The MPU setup routines */
    .global             ___RME_A6M_MPU_Set1
    .global             ___RME_A6M_MPU_Set2
    .global             ___RME_A6M_MPU_Set3
    .global             ___RME_A6M_MPU_Set4
    .global             ___RME_A6M_MPU_Set5
    .global             ___RME_A6M_MPU_Set6
    .global             ___RME_A6M_MPU_Set7
    .global             ___RME_A6M_MPU_Set8
    .global             ___RME_A6M_MPU_Set9
    .global             ___RME_A6M_MPU_Set10
    .global             ___RME_A6M_MPU_Set11
    .global             ___RME_A6M_MPU_Set12
    .global             ___RME_A6M_MPU_Set13
    .global             ___RME_A6M_MPU_Set14
    .global             ___RME_A6M_MPU_Set15
    .global             ___RME_A6M_MPU_Set16
/* End Export ****************************************************************/

/* Entry *********************************************************************/
    .section            .text.rme_entry
    .align              3

    .thumb_func
__RME_Entry:
    LDR                 R0, =__RME_A6M_Lowlvl_Preinit
    BLX                 R0
    /* Load data section from flash to RAM */
    LDR                 R0,=__RME_Data_Start
    LDR                 R1,=__RME_Data_End
    LDR                 R2,=__RME_Data_Load
__RME_Data_Load:
    CMP                 R0,R1
    BEQ                 __RME_Data_Done
    LDR                 R3,[R2]
    STR                 R3,[R0]
    ADDS                R0,#0x04
    ADDS                R2,#0x04
    B                   __RME_Data_Load
__RME_Data_Done:
    /* Clear bss zero section */
    LDR                 R0,=__RME_Zero_Start
    LDR                 R1,=__RME_Zero_End
    LDR                 R2,=0x00
__RME_Zero_Clear:
    CMP                 R0,R1
    BEQ                 __RME_Zero_Done
    STR                 R2,[R0]
    ADDS                R0,#0x04
    B                   __RME_Zero_Clear
__RME_Zero_Done:
    LDR                 R0, =main
    BX                  R0
/* End Entry *****************************************************************/

/* Vector ********************************************************************/
    .section            .text.rme_vector
    .align              3

__RME_Vector:
	.word 				__RME_Stack  		/* Top of Stack */
    .word               __RME_Entry         /* Reset Handler */
    .word               NMI_Handler         /* NMI Handler */
    .word               HardFault_Handler   /* Hard Fault Handler */
    .word               0                   /* Reserved */
    .word               0                   /* Reserved */
    .word               0                   /* Reserved */
    .word               0                   /* Reserved */
    .word               0                   /* Reserved */
    .word               0                   /* Reserved */
    .word               0                   /* Reserved */
    .word               SVC_Handler         /* SVCall Handler */
    .word               0                   /* Reserved */
    .word               0                   /* Reserved */
    .word               PendSV_Handler      /* PendSV Handler */
    .word               SysTick_Handler     /* SysTick Handler */

    .word               IRQ0_Handler        /* 32 External Interrupts */
    .word               IRQ1_Handler
    .word               IRQ2_Handler
    .word               IRQ3_Handler
    .word               IRQ4_Handler
    .word               IRQ5_Handler
    .word               IRQ6_Handler
    .word               IRQ7_Handler
    .word               IRQ8_Handler
    .word               IRQ9_Handler

    .word               IRQ10_Handler
    .word               IRQ11_Handler
    .word               IRQ12_Handler
    .word               IRQ13_Handler
    .word               IRQ14_Handler
    .word               IRQ15_Handler
    .word               IRQ16_Handler
    .word               IRQ17_Handler
    .word               IRQ18_Handler
    .word               IRQ19_Handler

    .word               IRQ20_Handler
    .word               IRQ21_Handler
    .word               IRQ22_Handler
    .word               IRQ23_Handler
    .word               IRQ24_Handler
    .word               IRQ25_Handler
    .word               IRQ26_Handler
    .word               IRQ27_Handler
    .word               IRQ28_Handler
    .word               IRQ29_Handler

    .word               IRQ30_Handler
    .word               IRQ31_Handler

    .weak               IRQ0_Handler        /* 32 External Interrupts */
    .weak               IRQ1_Handler
    .weak               IRQ2_Handler
    .weak               IRQ3_Handler
    .weak               IRQ4_Handler
    .weak               IRQ5_Handler
    .weak               IRQ6_Handler
    .weak               IRQ7_Handler
    .weak               IRQ8_Handler
    .weak               IRQ9_Handler

    .weak               IRQ10_Handler
    .weak               IRQ11_Handler
    .weak               IRQ12_Handler
    .weak               IRQ13_Handler
    .weak               IRQ14_Handler
    .weak               IRQ15_Handler
    .weak               IRQ16_Handler
    .weak               IRQ17_Handler
    .weak               IRQ18_Handler
    .weak               IRQ19_Handler

    .weak               IRQ20_Handler
    .weak               IRQ21_Handler
    .weak               IRQ22_Handler
    .weak               IRQ23_Handler
    .weak               IRQ24_Handler
    .weak               IRQ25_Handler
    .weak               IRQ26_Handler
    .weak               IRQ27_Handler
    .weak               IRQ28_Handler
    .weak               IRQ29_Handler

    .weak               IRQ30_Handler
    .weak               IRQ31_Handler

    .thumb_func
Default_Handler:
    .thumb_func
IRQ0_Handler:
    .thumb_func
IRQ1_Handler:
    .thumb_func
IRQ2_Handler:
    .thumb_func
IRQ3_Handler:
    .thumb_func
IRQ4_Handler:
    .thumb_func
IRQ5_Handler:
    .thumb_func
IRQ6_Handler:
    .thumb_func
IRQ7_Handler:
    .thumb_func
IRQ8_Handler:
    .thumb_func
IRQ9_Handler:

    .thumb_func
IRQ10_Handler:
    .thumb_func
IRQ11_Handler:
    .thumb_func
IRQ12_Handler:
    .thumb_func
IRQ13_Handler:
    .thumb_func
IRQ14_Handler:
    .thumb_func
IRQ15_Handler:
    .thumb_func
IRQ16_Handler:
    .thumb_func
IRQ17_Handler:
    .thumb_func
IRQ18_Handler:
    .thumb_func
IRQ19_Handler:

    .thumb_func
IRQ20_Handler:
    .thumb_func
IRQ21_Handler:
    .thumb_func
IRQ22_Handler:
    .thumb_func
IRQ23_Handler:
    .thumb_func
IRQ24_Handler:
    .thumb_func
IRQ25_Handler:
    .thumb_func
IRQ26_Handler:
    .thumb_func
IRQ27_Handler:
    .thumb_func
IRQ28_Handler:
    .thumb_func
IRQ29_Handler:

    .thumb_func
IRQ30_Handler:
    .thumb_func
IRQ31_Handler:
    PUSH                {R4-R7,LR}          /* Save registers */
    MOV                 R4,R8
    MOV                 R5,R9
    MOV                 R6,R10
    MOV                 R7,R11
    PUSH                {R4-R7}

    MRS                 R0,PSP
    PUSH                {R0}

    MRS                 R1,xPSR             /* Interrupt number */
    MOVS                R0,#0x3F
    ANDS                R1,R0              
    SUBS                R1,#16              /* The IRQ0's number is 16; subtract */

    MOV                 R0,SP               /* Pass in the regs parameter */
    BL                  __RME_A6M_Vct_Handler

    POP                 {R0}
    MSR                 PSP,R0

    POP                 {R4-R7}             /* Restore registers */
    MOV                 R8,R4
    MOV                 R9,R5
    MOV                 R10,R6
    MOV                 R11,R7
    POP                 {R4-R7,PC}
/* End Vector ****************************************************************/

/* Function:__RME_Int_Disable *************************************************
Description : The function for disabling all interrupts.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
    .section            .text.__rme_int_disable
    .align              3

    .thumb_func
__RME_Int_Disable:
    CPSID               I
    BX                  LR
/* End Function:__RME_Int_Disable ********************************************/

/* Function:__RME_Int_Enable **************************************************
Description : The function for enabling all interrupts.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
    .section            .text.__rme_int_enable
    .align              3

    .thumb_func
__RME_Int_Enable:
    CPSIE               I 
    BX                  LR
/* End Function:__RME_Int_Enable *********************************************/

/* Function:__RME_A6M_Barrier *************************************************
Description : A full data/instruction barrier.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
    .section            .text.__rme_a6m_barrier
    .align              3

    .thumb_func
__RME_A6M_Barrier:
    DSB                 SY
    ISB                 SY
    BX                  LR
/* End Function:__RME_A6M_Barrier ********************************************/

/* Function:__RME_A6M_Reset ***************************************************
Description : A full system reset.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
    .section            .text.__rme_a6m_reset
    .align              3

    .thumb_func
__RME_A6M_Reset:
    /* Disable all interrupts */
    CPSID               I
    /* ARMv6-M Standard system reset */
    LDR                 R0,=0xE000ED0C
    LDR                 R1,=0x05FA0004
    STR                 R1,[R0]
    ISB                 SY
    /* Deadloop */
    B                   .
/* End Function:__RME_A6M_Reset **********************************************/

/* Function:__RME_A6M_Wait_Int ************************************************
Description : Wait until a new interrupt comes, to save power.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
    .section            .text.__rme_a6m_wait_int
    .align              3

    .thumb_func
__RME_A6M_Wait_Int:
    WFE 
    BX                  LR
/* End Function:__RME_A6M_Wait_Int *******************************************/

/* Function:__RME_User_Enter *********************************************
Description : Entering of the user mode, after the system finish its preliminary
              booting. The function shall never return. This function should only
              be used to boot the first process in the system.
Input       : ptr_t Entry - The user execution startpoint.
              ptr_t Stack - The user stack.
              ptr_t CPUID - The CPUID.
Output      : None.
Return      : None.
******************************************************************************/
    .section            .text.__rme_user_enter
    .align              3

    .thumb
__RME_User_Enter:
    MSR                 PSP,R1              /* Set the stack pointer */
    MOVS                R4,#0x03            /* Unprevileged thread mode */
    MSR                 CONTROL,R4
    ISB
    MOV                 R1,R0               /* Save the entry to R1 */
    MOV                 R0,R2               /* Save CPUID(0) to R0 */
    BLX                 R1                  /* Branch to our target */
/* End Function:__RME_User_Enter ****************************************/

/* Function:SysTick_Handler ***************************************************
Description : The System Tick Timer handler routine. This will in fact call a
              C function to resolve the system service routines.             
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
    .section            .text.systick_handler
    .align              3

    .thumb_func
SysTick_Handler:
    PUSH                {R4-R7,LR}          /* Save registers */
    MOV                 R4,R8
    MOV                 R5,R9
    MOV                 R6,R10
    MOV                 R7,R11
    PUSH                {R4-R7}
    
    MRS                 R0,PSP
    PUSH                {R0}

    MOV                 R0,SP               /* Pass in the regs */
    BL                  __RME_A6M_Tim_Handler
    
    POP                 {R0}
    MSR                 PSP,R0

    POP                 {R4-R7}             /* Restore registers */
    MOV                 R8,R4
    MOV                 R9,R5
    MOV                 R10,R6
    MOV                 R11,R7
    POP                 {R4-R7,PC}
/* End Function:SysTick_Handler **********************************************/

/* Function:SVC_Handler *******************************************************
Description : The SVC handler routine. This will in fact call a C function to resolve
              the system service routines.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
    .section            .text.svc_handler
    .align              3

    .thumb_func
SVC_Handler:
    PUSH                {R4-R7,LR}          /* Save registers */
    MOV                 R4,R8
    MOV                 R5,R9
    MOV                 R6,R10
    MOV                 R7,R11
    PUSH                {R4-R7}

    MRS                 R0,PSP
    PUSH                {R0}

    MOV                 R0,SP               /* Pass in the regs */
    BL                  __RME_A6M_Svc_Handler

    POP                 {R0}
    MSR                 PSP,R0

    POP                 {R4-R7}             /* Restore registers */
    MOV                 R8,R4
    MOV                 R9,R5
    MOV                 R10,R6
    MOV                 R11,R7
    POP                 {R4-R7,PC}
/* End Function:SVC_Handler **************************************************/

/* Function:NMI/HardFault_Handler *********************************************
Description : The multi-purpose handler routine. This will in fact call
              a C function to resolve the system service routines.             
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
    .section            .text.system_handler
    .align              3

    .thumb_func
NMI_Handler:
    NOP
    .thumb_func
PendSV_Handler:
    NOP
    .thumb_func
HardFault_Handler:
    PUSH                {R4-R7,LR}          /* Save registers */
    MOV                 R4,R8
    MOV                 R5,R9
    MOV                 R6,R10
    MOV                 R7,R11
    PUSH                {R4-R7}

    MRS                 R0,PSP
    PUSH                {R0}

    MOV                 R0,SP               /* Pass in the regs */
    BL                  __RME_A6M_Exc_Handler

    POP                 {R0}
    MSR                 PSP,R0

    POP                 {R4-R7}             /* Restore registers */
    MOV                 R8,R4
    MOV                 R9,R5
    MOV                 R10,R6
    MOV                 R11,R7
    POP                 {R4-R7,PC}
/* End Function:NMI/HardFault_Handler ****************************************/

/* Function:___RME_A6M_MPU_Set ************************************************
Description : Set the MPU context. We write 8 registers at a time to increase efficiency.            
Input       : R0 - The pointer to the MPU content.
Output      : None.
Return      : None.
******************************************************************************/
    .macro              MPU_PRE
    PUSH                {R4-R7}             /* Save registers */
    LDR                 R2,=0xE000ED9C      /* RBAR */
    LDR                 R3,=0xE000EDA0      /* RASR */
    .endm
    
    .macro              MPU_SET
    LDMIA               R0!,{R4-R5}         /* Read settings */
    STR                 R4,[R2]             /* Program */
    STR                 R5,[R3]
    .endm
    
    .macro              MPU_SET2
    LDMIA               R0!,{R4-R7}
    STR                 R4,[R2]
    STR                 R5,[R3]
    STR                 R6,[R2]
    STR                 R7,[R3]
    .endm
    
    .macro              MPU_SET3
    MPU_SET2
    MPU_SET
    .endm
    
    .macro              MPU_SET4
    MPU_SET2
    MPU_SET2
    .endm
    
    .macro              MPU_POST
    POP                 {R4-R7}             /* Restore registers */
    ISB                                     /* Barrier */
    BX                  LR
    .endm
                        
/* 1-region version */
    .section            .text.___rme_a6m_mpu_set1
    .align              3

    .thumb_func
___RME_A6M_MPU_Set1:
    MPU_PRE
    MPU_SET
    MPU_POST

/* 2-region version */
    .section            .text.___rme_a6m_mpu_set2
    .align              3

    .thumb_func
___RME_A6M_MPU_Set2:
    MPU_PRE
    MPU_SET2
    MPU_POST

/* 3-region version */
    .section            .text.___rme_a6m_mpu_set3
    .align              3

    .thumb_func
___RME_A6M_MPU_Set3:
    MPU_PRE
    MPU_SET3
    MPU_POST

/* 4-region version */
    .section            .text.___rme_a6m_mpu_set4
    .align              3

    .thumb_func
___RME_A6M_MPU_Set4:
    MPU_PRE
    MPU_SET4
    MPU_POST

/* 5-region version */
    .section            .text.___rme_a6m_mpu_set5
    .align              3

    .thumb_func
___RME_A6M_MPU_Set5:
    MPU_PRE
    MPU_SET4
    MPU_SET
    MPU_POST

/* 6-region version */
    .section            .text.___rme_a6m_mpu_set6
    .align              3

    .thumb_func
___RME_A6M_MPU_Set6:
    MPU_PRE
    MPU_SET4
    MPU_SET2
    MPU_POST

/* 7-region version */
    .section            .text.___rme_a6m_mpu_set7
    .align              3

    .thumb_func
___RME_A6M_MPU_Set7:
    MPU_PRE
    MPU_SET4
    MPU_SET3
    MPU_POST

/* 8-region version */
    .section            .text.___rme_a6m_mpu_set8
    .align              3

    .thumb_func
___RME_A6M_MPU_Set8:
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_POST

/* 9-region version */
    .section            .text.___rme_a6m_mpu_set9
    .align              3

    .thumb_func
___RME_A6M_MPU_Set9:
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_SET
    MPU_POST

/* 10-region version */
    .section            .text.___rme_a6m_mpu_set10
    .align              3

    .thumb_func
___RME_A6M_MPU_Set10:
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_SET2
    MPU_POST

/* 11-region version */
    .section            .text.___rme_a6m_mpu_set11
    .align              3

    .thumb_func
___RME_A6M_MPU_Set11:
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_SET3
    MPU_POST

/* 12-region version */
    .section            .text.___rme_a6m_mpu_set12
    .align              3

    .thumb_func
___RME_A6M_MPU_Set12:
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_SET4
    MPU_POST

/* 13-region version */
    .section            .text.___rme_a6m_mpu_set13
    .align              3

    .thumb_func
___RME_A6M_MPU_Set13:
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_SET4
    MPU_SET
    MPU_POST

/* 14-region version */
    .section            .text.___rme_a6m_mpu_set14
    .align              3

    .thumb_func
___RME_A6M_MPU_Set14:
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_SET4
    MPU_SET2
    MPU_POST

/* 15-region version */
    .section            .text.___rme_a6m_mpu_set15
    .align              3

    .thumb_func
___RME_A6M_MPU_Set15:
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_SET4
    MPU_SET3
    MPU_POST

/* 16-region version */
    .section            .text.___rme_a6m_mpu_set16
    .align              3

    .thumb_func
___RME_A6M_MPU_Set16:
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_SET4
    MPU_SET4
    MPU_POST
/* End Function:___RME_A6M_MPU_Set *******************************************/

	.end
/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
