;/*****************************************************************************
;Filename    : rme_platform_a6m_armcc.s
;Author      : pry
;Date        : 19/01/2017
;Description : The ARMv6-M assembly support of the RME RTOS.
;*****************************************************************************/

;/* The ARMv6-M Architecture **************************************************
;R0-R7:General purpose registers that are accessible. 
;R8-R12:general purpose registers that can only be reached by 32-bit instructions.
;R13:SP/SP_process/SP_main    Stack pointer
;R14:LR                       Link Register(used for returning from a subfunction)
;R15:PC                       Program counter.
;IPSR                         Interrupt Program Status Register.
;APSR                         Application Program Status Register.
;EPSR                         Execute Program Status Register.
;The above 3 registers are saved into the stack in combination(xPSR).
;*****************************************************************************/

;/* Import *******************************************************************/
    ;Preinitialization routine
    IMPORT              __RME_A6M_Lowlvl_Preinit
    ;The ARM C library entry
    IMPORT              __main
    ;The ARM linker stack segment - provided by linker
    IMPORT              |Image$$ARM_LIB_STACK$$ZI$$Limit|
    ;The system call handler
    IMPORT              __RME_A6M_Svc_Handler
    ;The system tick handler
    IMPORT              __RME_A6M_Tim_Handler
    ;The memory management fault handler
    IMPORT              __RME_A6M_Exc_Handler
    ;The generic interrupt handler for all other vectors
    IMPORT              __RME_A6M_Vct_Handler
;/* End Import ***************************************************************/

;/* Export *******************************************************************/
    ;Disable all interrupts
    EXPORT              __RME_Int_Disable
    ;Enable all interrupts
    EXPORT              __RME_Int_Enable
    ;A full barrier
    EXPORT              __RME_A6M_Barrier
    ;Full system reset
    EXPORT              __RME_A6M_Reset
    ;Wait until interrupts happen
    EXPORT              __RME_A6M_Wait_Int
    ;Entering of the user mode
    EXPORT              __RME_User_Enter
    ;The MPU setup routines
    EXPORT              ___RME_A6M_MPU_Set1
    EXPORT              ___RME_A6M_MPU_Set2
    EXPORT              ___RME_A6M_MPU_Set3
    EXPORT              ___RME_A6M_MPU_Set4
    EXPORT              ___RME_A6M_MPU_Set5
    EXPORT              ___RME_A6M_MPU_Set6
    EXPORT              ___RME_A6M_MPU_Set7
    EXPORT              ___RME_A6M_MPU_Set8
    EXPORT              ___RME_A6M_MPU_Set9
    EXPORT              ___RME_A6M_MPU_Set10
    EXPORT              ___RME_A6M_MPU_Set11
    EXPORT              ___RME_A6M_MPU_Set12
    EXPORT              ___RME_A6M_MPU_Set13
    EXPORT              ___RME_A6M_MPU_Set14
    EXPORT              ___RME_A6M_MPU_Set15
    EXPORT              ___RME_A6M_MPU_Set16
;/* End Export ***************************************************************/

;/* Entry ********************************************************************/
    AREA                RME_ENTRY,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

Reset_Handler           PROC
    LDR                 R0,=__RME_A6M_Lowlvl_Preinit
    BLX                 R0
    LDR                 R0,=__main
    BX                  R0
    ENDP
    ALIGN
    LTORG
    ALIGN
;/* End Entry ****************************************************************/

;/* Vector *******************************************************************/
    AREA                RME_VECTOR,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

    ;Vector table
    DCD                 |Image$$ARM_LIB_STACK$$ZI$$Limit|
    DCD                 Reset_Handler       ;Reset Handler
    DCD                 NMI_Handler         ;NMI Handler
    DCD                 HardFault_Handler   ;Hard Fault Handler
    DCD                 0                   ;Reserved
    DCD                 0                   ;Reserved
    DCD                 0                   ;Reserved
    DCD                 0                   ;Reserved
    DCD                 0                   ;Reserved
    DCD                 0                   ;Reserved
    DCD                 0                   ;Reserved
    DCD                 SVC_Handler         ;SVCall Handler
    DCD                 0                   ;Reserved
    DCD                 0                   ;Reserved
    DCD                 PendSV_Handler      ;PendSV Handler
    DCD                 SysTick_Handler     ;SysTick Handler

    DCD                 IRQ0_Handler        ;32 External Interrupts
    DCD                 IRQ1_Handler
    DCD                 IRQ2_Handler
    DCD                 IRQ3_Handler
    DCD                 IRQ4_Handler
    DCD                 IRQ5_Handler
    DCD                 IRQ6_Handler
    DCD                 IRQ7_Handler
    DCD                 IRQ8_Handler
    DCD                 IRQ9_Handler

    DCD                 IRQ10_Handler
    DCD                 IRQ11_Handler
    DCD                 IRQ12_Handler
    DCD                 IRQ13_Handler
    DCD                 IRQ14_Handler
    DCD                 IRQ15_Handler
    DCD                 IRQ16_Handler
    DCD                 IRQ17_Handler
    DCD                 IRQ18_Handler
    DCD                 IRQ19_Handler

    DCD                 IRQ20_Handler
    DCD                 IRQ21_Handler
    DCD                 IRQ22_Handler
    DCD                 IRQ23_Handler
    DCD                 IRQ24_Handler
    DCD                 IRQ25_Handler
    DCD                 IRQ26_Handler
    DCD                 IRQ27_Handler
    DCD                 IRQ28_Handler
    DCD                 IRQ29_Handler

    DCD                 IRQ30_Handler
    DCD                 IRQ31_Handler

Default_Handler         PROC                ;32 ExternalInterrupts
    EXPORT              IRQ0_Handler        [WEAK]  
    EXPORT              IRQ1_Handler        [WEAK]
    EXPORT              IRQ2_Handler        [WEAK]
    EXPORT              IRQ3_Handler        [WEAK]
    EXPORT              IRQ4_Handler        [WEAK]
    EXPORT              IRQ5_Handler        [WEAK]
    EXPORT              IRQ6_Handler        [WEAK]
    EXPORT              IRQ7_Handler        [WEAK]
    EXPORT              IRQ8_Handler        [WEAK]
    EXPORT              IRQ9_Handler        [WEAK]

    EXPORT              IRQ10_Handler       [WEAK]
    EXPORT              IRQ11_Handler       [WEAK]
    EXPORT              IRQ12_Handler       [WEAK]
    EXPORT              IRQ13_Handler       [WEAK]
    EXPORT              IRQ14_Handler       [WEAK]
    EXPORT              IRQ15_Handler       [WEAK]
    EXPORT              IRQ16_Handler       [WEAK]
    EXPORT              IRQ17_Handler       [WEAK]
    EXPORT              IRQ18_Handler       [WEAK]
    EXPORT              IRQ19_Handler       [WEAK]

    EXPORT              IRQ20_Handler       [WEAK]
    EXPORT              IRQ21_Handler       [WEAK]
    EXPORT              IRQ22_Handler       [WEAK]
    EXPORT              IRQ23_Handler       [WEAK]
    EXPORT              IRQ24_Handler       [WEAK]
    EXPORT              IRQ25_Handler       [WEAK]
    EXPORT              IRQ26_Handler       [WEAK]
    EXPORT              IRQ27_Handler       [WEAK]
    EXPORT              IRQ28_Handler       [WEAK]
    EXPORT              IRQ29_Handler       [WEAK]

    EXPORT              IRQ30_Handler       [WEAK]
    EXPORT              IRQ31_Handler       [WEAK]

IRQ0_Handler
IRQ1_Handler
IRQ2_Handler
IRQ3_Handler
IRQ4_Handler
IRQ5_Handler
IRQ6_Handler
IRQ7_Handler
IRQ8_Handler
IRQ9_Handler

IRQ10_Handler
IRQ11_Handler
IRQ12_Handler
IRQ13_Handler
IRQ14_Handler
IRQ15_Handler
IRQ16_Handler
IRQ17_Handler
IRQ18_Handler
IRQ19_Handler

IRQ20_Handler
IRQ21_Handler
IRQ22_Handler
IRQ23_Handler
IRQ24_Handler
IRQ25_Handler
IRQ26_Handler
IRQ27_Handler
IRQ28_Handler
IRQ29_Handler

IRQ30_Handler
IRQ31_Handler
    PUSH                {R4-R7,LR}          ;Save registers
    MOV                 R4,R8
    MOV                 R5,R9
    MOV                 R6,R10
    MOV                 R7,R11
    PUSH                {R4-R7}

    MRS                 R0,PSP
    PUSH                {R0}

    MRS                 R1,xPSR             ;Interrupt number
    MOVS                R0,#0x3F
    ANDS                R1,R0              
    SUBS                R1,#16              ;The IRQ0's number is 16; subtract

    MOV                 R0,SP               ;Pass in the regs parameter
    BL                  __RME_A6M_Vct_Handler

    POP                 {R0}
    MSR                 PSP,R0

    POP                 {R4-R7}             ;Restore registers
    MOV                 R8,R4
    MOV                 R9,R5
    MOV                 R10,R6
    MOV                 R11,R7
    POP                 {R4-R7,PC}
    ENDP
    ALIGN
    LTORG
    ALIGN
;/* End Vector ***************************************************************/

;/* Function:__RME_Int_Disable ************************************************
;Description : The function for disabling all interrupts.
;Input       : None.
;Output      : None.
;Return      : None.
;*****************************************************************************/
    AREA                __RME_INT_DISABLE,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

__RME_Int_Disable       PROC
    CPSID               I 
    BX                  LR
    ENDP
    ALIGN
    LTORG
    ALIGN
;/* End Function:__RME_Int_Disable *******************************************/

;/* Function:__RME_Int_Enable *************************************************
;Description : The function for enabling all interrupts.
;Input       : None.
;Output      : None.    
;Return      : None.
;*****************************************************************************/
    AREA                __RME_INT_ENABLE,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

__RME_Int_Enable        PROC
    CPSIE               I 
    BX                  LR
    ENDP
    ALIGN
    LTORG
    ALIGN
;/* End Function:__RME_Int_Enable ********************************************/

;/* Function:__RME_A6M_Barrier ************************************************
;Description : A full data/instruction barrier.
;Input       : None.
;Output      : None.    
;Return      : None.
;*****************************************************************************/
    AREA                __RME_A6M_BARRIER,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

__RME_A6M_Barrier       PROC
    DSB                 SY
    ISB                 SY
    BX                  LR
    ENDP
    ALIGN
    LTORG
    ALIGN
;/* End Function:__RME_A6M_Barrier *******************************************/

;/* Function:__RME_A6M_Reset **************************************************
;Description : A full system reset.
;Input       : None.
;Output      : None.    
;Return      : None.
;*****************************************************************************/
    AREA                __RME_A6M_RESET,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

__RME_A6M_Reset         PROC
    ;Disable all interrupts
    CPSID               I
    ;ARMv7-M Standard system reset
    LDR                 R0,=0xE000ED0C
    LDR                 R1,=0x05FA0004
    STR                 R1,[R0]
    ISB                 SY
    ;Deadloop
    B                   .
    ENDP
    ALIGN
    LTORG
    ALIGN
;/* End Function:__RME_A6M_Reset *********************************************/

;/* Function:__RME_A6M_Wait_Int ***********************************************
;Description : Wait until a new interrupt comes, to save power.
;Input       : None.
;Output      : None.
;Return      : None.
;*****************************************************************************/
    AREA                __RME_A6M_WAIT_INT,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

__RME_A6M_Wait_Int      PROC
    WFE
    BX                  LR
    ENDP
    ALIGN
    LTORG
    ALIGN
;/* End Function:__RME_A6M_Wait_Int ******************************************/

;/* Function:__RME_User_Enter *************************************************
;Description : Entering of the user mode, after the system finish its preliminary
;              booting. The function shall never return. This function should only
;              be used to boot the first process in the system.
;Input       : ptr_t Entry - The user execution startpoint.
;              ptr_t Stack - The user stack.
;              ptr_t CPUID - The CPUID.
;Output      : None.
;Return      : None.
;*****************************************************************************/
    AREA                __RME_USER_ENTER,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

__RME_User_Enter        PROC
    MSR                 PSP,R1              ;Set the stack pointer
    MOVS                R4,#0x03            ;Unprevileged thread mode
    MSR                 CONTROL,R4
    ISB
    MOV                 R1,R0               ;Save the entry to R1
    MOV                 R0,R2               ;Save CPUID(0) to R0
    BX                  R1                  ;Branch to target
    ENDP
    ALIGN
    LTORG
    ALIGN
;/* End Function:__RME_User_Enter ********************************************/

;/* Function:SysTick_Handler **************************************************
;Description : The System Tick Timer handler routine. This will in fact call a
;              C function to resolve the system service routines.             
;Input       : None.
;Output      : None.
;Return      : None.
;*****************************************************************************/
    AREA                SYSTICK_HANDLER,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

SysTick_Handler         PROC
    PUSH                {R4-R7,LR}          ;Save registers
    MOV                 R4,R8
    MOV                 R5,R9
    MOV                 R6,R10
    MOV                 R7,R11
    PUSH                {R4-R7}
    
    MRS                 R0,PSP
    PUSH                {R0}
    
    MOV                 R0,SP               ;Pass in the regs
    BL                  __RME_A6M_Tim_Handler
    
    POP                 {R0}
    MSR                 PSP,R0
    
    POP                 {R4-R7}             ;Restore registers
    MOV                 R8,R4
    MOV                 R9,R5
    MOV                 R10,R6
    MOV                 R11,R7
    POP                 {R4-R7,PC}
    ENDP
    ALIGN
    LTORG
    ALIGN
;/* End Function:SysTick_Handler *********************************************/

;/* Function:SVC_Handler ******************************************************
;Description : The SVC handler routine. This will in fact call a C function to resolve
;              the system service routines.             
;Input       : None.
;Output      : None.
;Return      : None.
;*****************************************************************************/
    AREA                SVC_HANDLER,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

SVC_Handler             PROC
    PUSH                {R4-R7,LR}          ;Save registers
    MOV                 R4,R8
    MOV                 R5,R9
    MOV                 R6,R10
    MOV                 R7,R11
    PUSH                {R4-R7}

    MRS                 R0,PSP
    PUSH                {R0}

    MOV                 R0,SP               ;Pass in the regs
    BL                  __RME_A6M_Svc_Handler

    POP                 {R0}
    MSR                 PSP,R0

    POP                 {R4-R7}             ;Restore registers
    MOV                 R8,R4
    MOV                 R9,R5
    MOV                 R10,R6
    MOV                 R11,R7
    POP                 {R4-R7,PC}
    ENDP
    ALIGN
    LTORG
    ALIGN
;/* End Function:SVC_Handler *************************************************/

;/* Function:NMI/HardFault_Handler ********************************************
;Description : The multi-purpose handler routine. This will in fact call
;              a C function to resolve the system service routines.             
;Input       : None.
;Output      : None.
;Return      : None.
;*****************************************************************************/
    AREA                SYSTEM_HANDLER,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

NMI_Handler             PROC
    NOP
PendSV_Handler
    NOP
HardFault_Handler
    PUSH                {R4-R7,LR}          ;Save registers
    MOV                 R4,R8
    MOV                 R5,R9
    MOV                 R6,R10
    MOV                 R7,R11
    PUSH                {R4-R7}

    MRS                 R0,PSP
    PUSH                {R0}

    MOV                 R0,SP               ;Pass in the regs
    BL                  __RME_A6M_Exc_Handler

    POP                 {R0}
    MSR                 PSP,R0

    POP                 {R4-R7}             ;Restore registers
    MOV                 R8,R4
    MOV                 R9,R5
    MOV                 R10,R6
    MOV                 R11,R7
    POP                 {R4-R7,PC}
    ENDP
    ALIGN
    LTORG
    ALIGN
;/* End Function:NMI/HardFault_Handler ***************************************/

;/* Function:___RME_A6M_MPU_Set ***********************************************
;Description : Set the MPU context. 1-to-8-region versions are all declared here.
;Input       : R0 - The pointer to the MPU content.
;Output      : None.
;Return      : None.
;*****************************************************************************/
    MACRO
    MPU_PRE
    PUSH                {R4-R7}             ;Save registers
    LDR                 R2,=0xE000ED9C      ;RBAR
    LDR                 R3,=0xE000EDA0      ;RASR
    MEND

    MACRO
    MPU_SET
    LDMIA               R0!,{R4-R5}         ;Read settings
    STR                 R4,[R2]             ;Program
    STR                 R5,[R3]
    MEND
    
    MACRO
    MPU_SET2
    LDMIA               R0!,{R4-R7}
    STR                 R4,[R2]
    STR                 R5,[R3]
    STR                 R6,[R2]
    STR                 R7,[R3]
    MEND
    
    MACRO
    MPU_SET3
    MPU_SET2
    MPU_SET
    MEND
    
    MACRO
    MPU_SET4
    MPU_SET2
    MPU_SET2
    MEND
    
    MACRO
    MPU_POST
    POP                 {R4-R7}             ;Restore registers
    ISB                                     ;Barrier
    BX                  LR
    MEND

;1-region version
    AREA                ___RME_A6M_MPU_SET1,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A6M_MPU_Set1     PROC
    MPU_PRE
    MPU_SET
    MPU_POST
    ENDP
    ALIGN
    LTORG
    ALIGN

;2-region version
    AREA                ___RME_A6M_MPU_SET2,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A6M_MPU_Set2     PROC
    MPU_PRE
    MPU_SET2
    MPU_POST
    ENDP
    ALIGN
    LTORG
    ALIGN

;3-region version
    AREA                ___RME_A6M_MPU_SET3,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A6M_MPU_Set3     PROC
    MPU_PRE
    MPU_SET3
    MPU_POST
    ENDP
    ALIGN
    LTORG
    ALIGN

;4-region version
    AREA                ___RME_A6M_MPU_SET4,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A6M_MPU_Set4     PROC
    MPU_PRE
    MPU_SET4
    MPU_POST
    ENDP
    ALIGN
    LTORG
    ALIGN

;5-region version
    AREA                ___RME_A6M_MPU_SET5,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A6M_MPU_Set5     PROC
    MPU_PRE
    MPU_SET4
    MPU_SET
    MPU_POST
    ENDP
    ALIGN
    LTORG
    ALIGN

;6-region version
    AREA                ___RME_A6M_MPU_SET6,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A6M_MPU_Set6     PROC
    MPU_PRE
    MPU_SET4
    MPU_SET2
    MPU_POST
    ENDP
    ALIGN
    LTORG
    ALIGN

;7-region version
    AREA                ___RME_A6M_MPU_SET7,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A6M_MPU_Set7     PROC
    MPU_PRE
    MPU_SET4
    MPU_SET3
    MPU_POST
    ENDP
    ALIGN
    LTORG
    ALIGN

;8-region version
    AREA                ___RME_A6M_MPU_SET8,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A6M_MPU_Set8     PROC
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_POST
    ENDP
    ALIGN
    LTORG
    ALIGN

;9-region version
    AREA                ___RME_A6M_MPU_SET9,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A6M_MPU_Set9     PROC
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_SET
    MPU_POST
    ENDP
    ALIGN
    LTORG
    ALIGN

;10-region version
    AREA                ___RME_A6M_MPU_SET10,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A6M_MPU_Set10    PROC
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_SET2
    MPU_POST
    ENDP
    ALIGN
    LTORG
    ALIGN

;11-region version
    AREA                ___RME_A6M_MPU_SET11,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A6M_MPU_Set11    PROC
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_SET3
    MPU_POST
    ENDP
    ALIGN
    LTORG
    ALIGN

;12-region version
    AREA                ___RME_A6M_MPU_SET12,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A6M_MPU_Set12    PROC
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_SET4
    MPU_POST
    ENDP
    ALIGN
    LTORG
    ALIGN

;13-region version
    AREA                ___RME_A6M_MPU_SET13,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A6M_MPU_Set13    PROC
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_SET4
    MPU_SET
    MPU_POST
    ENDP
    ALIGN
    LTORG
    ALIGN

;14-region version
    AREA                ___RME_A6M_MPU_SET14,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A6M_MPU_Set14    PROC
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_SET4
    MPU_SET2
    MPU_POST
    ENDP
    ALIGN
    LTORG
    ALIGN

;15-region version
    AREA                ___RME_A6M_MPU_SET15,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A6M_MPU_Set15    PROC
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_SET4
    MPU_SET3
    MPU_POST
    ENDP
    ALIGN
    LTORG
    ALIGN

;16-region version
    AREA                ___RME_A6M_MPU_SET16,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A6M_MPU_Set16    PROC
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_SET4
    MPU_SET4
    MPU_POST
    ENDP
    ALIGN
    LTORG
    ALIGN
;/* End Function:___RME_A6M_MPU_Set ******************************************/

    END
;/* End Of File **************************************************************/

;/* Copyright (C) Evo-Devo Instrum. All rights reserved **********************/
