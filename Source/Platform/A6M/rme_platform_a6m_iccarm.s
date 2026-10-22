;/*****************************************************************************
;Filename    : rme_platform_a6m_iccarm.s
;Author      : pry ; Cline (ported from the armcc variant)
;Date        : 21/10/2026
;Description : The ARMv6-M assembly support of the RME RTOS.
;              This is the IAR (iasmarm) port.
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
        EXTERN              __RME_A6M_Lowlvl_Preinit
        ;The IAR C library initialization and startup
        EXTERN              __iar_program_start
        ;The system call handler
        EXTERN              __RME_A6M_Svc_Handler
        ;The system tick handler
        EXTERN              __RME_A6M_Tim_Handler
        ;The memory management fault handler
        EXTERN              __RME_A6M_Exc_Handler
        ;The generic interrupt handler for all other vectors
        EXTERN              __RME_A6M_Vct_Handler
;/* End Import ***************************************************************/

;/* Export *******************************************************************/
        ;Disable all interrupts
        PUBLIC              __RME_Int_Disable
        ;Enable all interrupts
        PUBLIC              __RME_Int_Enable
        ;A full barrier
        PUBLIC              __RME_A6M_Barrier
        ;Full system reset
        PUBLIC              __RME_A6M_Reset
        ;Wait until interrupts happen
        PUBLIC              __RME_A6M_Wait_Int
        ;Entering of the user mode
        PUBLIC              __RME_User_Enter
        ;The reset handler
        PUBLIC              Reset_Handler
        ;The system handlers(the vector table references them)
        PUBLIC              NMI_Handler
        PUBLIC              HardFault_Handler
        PUBLIC              SVC_Handler
        PUBLIC              PendSV_Handler
        PUBLIC              SysTick_Handler
        ;The MPU setup routines
        PUBLIC              ___RME_A6M_MPU_Set1
        PUBLIC              ___RME_A6M_MPU_Set2
        PUBLIC              ___RME_A6M_MPU_Set3
        PUBLIC              ___RME_A6M_MPU_Set4
        PUBLIC              ___RME_A6M_MPU_Set5
        PUBLIC              ___RME_A6M_MPU_Set6
        PUBLIC              ___RME_A6M_MPU_Set7
        PUBLIC              ___RME_A6M_MPU_Set8
        PUBLIC              ___RME_A6M_MPU_Set9
        PUBLIC              ___RME_A6M_MPU_Set10
        PUBLIC              ___RME_A6M_MPU_Set11
        PUBLIC              ___RME_A6M_MPU_Set12
        PUBLIC              ___RME_A6M_MPU_Set13
        PUBLIC              ___RME_A6M_MPU_Set14
        PUBLIC              ___RME_A6M_MPU_Set15
        PUBLIC              ___RME_A6M_MPU_Set16
        ;The weak default external interrupt handlers
        PUBWEAK             IRQ0_Handler
        PUBWEAK             IRQ1_Handler
        PUBWEAK             IRQ2_Handler
        PUBWEAK             IRQ3_Handler
        PUBWEAK             IRQ4_Handler
        PUBWEAK             IRQ5_Handler
        PUBWEAK             IRQ6_Handler
        PUBWEAK             IRQ7_Handler
        PUBWEAK             IRQ8_Handler
        PUBWEAK             IRQ9_Handler
        PUBWEAK             IRQ10_Handler
        PUBWEAK             IRQ11_Handler
        PUBWEAK             IRQ12_Handler
        PUBWEAK             IRQ13_Handler
        PUBWEAK             IRQ14_Handler
        PUBWEAK             IRQ15_Handler
        PUBWEAK             IRQ16_Handler
        PUBWEAK             IRQ17_Handler
        PUBWEAK             IRQ18_Handler
        PUBWEAK             IRQ19_Handler
        PUBWEAK             IRQ20_Handler
        PUBWEAK             IRQ21_Handler
        PUBWEAK             IRQ22_Handler
        PUBWEAK             IRQ23_Handler
        PUBWEAK             IRQ24_Handler
        PUBWEAK             IRQ25_Handler
        PUBWEAK             IRQ26_Handler
        PUBWEAK             IRQ27_Handler
        PUBWEAK             IRQ28_Handler
        PUBWEAK             IRQ29_Handler
        PUBWEAK             IRQ30_Handler
        PUBWEAK             IRQ31_Handler
;/* End Export ***************************************************************/

;/* Entry ********************************************************************/
        MODULE              RME_A6M
        SECTION             .rme_entry:CODE:NOROOT(3)
        THUMB
        REQUIRE8
        PRESERVE8

Reset_Handler
        LDR                 R0,=__RME_A6M_Lowlvl_Preinit
        BLX                 R0
        LDR                 R0,=__iar_program_start
        BX                  R0
;/* End Entry ****************************************************************/

;/* Save/Restore macros ******************************************************/
        ;Save registers
RME_A6M_SAVE MACRO
        PUSH                {R4-R7,LR}          ;Save registers
        MOV                 R4,R8
        MOV                 R5,R9
        MOV                 R6,R10
        MOV                 R7,R11
        PUSH                {R4-R7}
        MRS                 R0,PSP
        PUSH                {R0}
        LDR                 R4,=0xE000ED94      ;Turn off MPU in case it sets wrong permission for kernel
        LDR                 R5,=0x00000000
        STR                 R5,[R4]
        ENDM

        ;Restore registers
RME_A6M_LOAD MACRO
        LDR                 R4,=0xE000ED94      ;Turn on MPU
        LDR                 R5,=0x00000005
        STR                 R5,[R4]
        POP                 {R0}
        MSR                 PSP,R0
        POP                 {R4-R7}             ;Restore registers
        MOV                 R8,R4
        MOV                 R9,R5
        MOV                 R10,R6
        MOV                 R11,R7
        POP                 {R4-R7,PC}
        ENDM
;/* End Save/Restore macros **************************************************/

;/* Vector *******************************************************************/
        ;Declare the CSTACK segment so that sfe(CSTACK) resolves - the block
        ;itself is defined by the .icf, this empty section gets discarded
        SECTION             CSTACK:DATA:NOROOT(3)
        SECTION             .rme_vector:CODE:NOROOT(3)
        THUMB
        REQUIRE8
        PRESERVE8

        ;Vector table - the .icf places this section at the ROM start
        DC32                sfe(CSTACK)
        DC32                Reset_Handler       ;Reset Handler
        DC32                NMI_Handler         ;NMI Handler
        DC32                HardFault_Handler   ;Hard Fault Handler
        DC32                0                   ;Reserved
        DC32                0                   ;Reserved
        DC32                0                   ;Reserved
        DC32                0                   ;Reserved
        DC32                0                   ;Reserved
        DC32                0                   ;Reserved
        DC32                0                   ;Reserved
        DC32                SVC_Handler         ;SVCall Handler
        DC32                0                   ;Reserved
        DC32                0                   ;Reserved
        DC32                PendSV_Handler      ;PendSV Handler
        DC32                SysTick_Handler     ;SysTick Handler

        DC32                IRQ0_Handler        ;32 External Interrupts
        DC32                IRQ1_Handler
        DC32                IRQ2_Handler
        DC32                IRQ3_Handler
        DC32                IRQ4_Handler
        DC32                IRQ5_Handler
        DC32                IRQ6_Handler
        DC32                IRQ7_Handler
        DC32                IRQ8_Handler
        DC32                IRQ9_Handler

        DC32                IRQ10_Handler
        DC32                IRQ11_Handler
        DC32                IRQ12_Handler
        DC32                IRQ13_Handler
        DC32                IRQ14_Handler
        DC32                IRQ15_Handler
        DC32                IRQ16_Handler
        DC32                IRQ17_Handler
        DC32                IRQ18_Handler
        DC32                IRQ19_Handler

        DC32                IRQ20_Handler
        DC32                IRQ21_Handler
        DC32                IRQ22_Handler
        DC32                IRQ23_Handler
        DC32                IRQ24_Handler
        DC32                IRQ25_Handler
        DC32                IRQ26_Handler
        DC32                IRQ27_Handler
        DC32                IRQ28_Handler
        DC32                IRQ29_Handler

        DC32                IRQ30_Handler
        DC32                IRQ31_Handler
;/* End Vector ***************************************************************/

;/* Weak default handlers ****************************************************/
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
        RME_A6M_SAVE

        MRS                 R1,IPSR             ;Interrupt number
        MOVS                R0,#0x3F
        ANDS                R1,R0
        SUBS                R1,#16              ;The IRQ0's number is 16; subtract

        MOV                 R0,SP               ;Pass in the regs parameter
        BL                  __RME_A6M_Vct_Handler

        RME_A6M_LOAD
        LTORG
;/* End Weak default handlers ************************************************/

;/* Function:__RME_Int_Disable ************************************************
;Description : The function for disabling all interrupts.
;Input       : None.
;Output      : None.
;Return      : None.
;*****************************************************************************/
        SECTION             .text:CODE:NOROOT(3)
        THUMB
        REQUIRE8
        PRESERVE8

__RME_Int_Disable
        CPSID               I 
        BX                  LR
;/* End Function:__RME_Int_Disable *******************************************/

;/* Function:__RME_Int_Enable *************************************************
;Description : The function for enabling all interrupts.
;Input       : None.
;Output      : None.    
;Return      : None.
;*****************************************************************************/
__RME_Int_Enable
        CPSIE               I 
        BX                  LR
;/* End Function:__RME_Int_Enable ********************************************/

;/* Function:__RME_A6M_Barrier ************************************************
;Description : A full data/instruction barrier.
;Input       : None.
;Output      : None.    
;Return      : None.
;*****************************************************************************/
__RME_A6M_Barrier
        DSB                 
        ISB                 
        BX                  LR
;/* End Function:__RME_A6M_Barrier *******************************************/

;/* Function:__RME_A6M_Reset **************************************************
;Description : A full system reset.
;Input       : None.
;Output      : None.    
;Return      : None.
;*****************************************************************************/
__RME_A6M_Reset
        ;Disable all interrupts
        CPSID               I
        ;ARMv7-M Standard system reset
        LDR                 R0,=0xE000ED0C
        LDR                 R1,=0x05FA0004
        STR                 R1,[R0]
        ISB                 
        ;Deadloop
        B                   .
        LTORG
;/* End Function:__RME_A6M_Reset *********************************************/

;/* Function:__RME_A6M_Wait_Int ***********************************************
;Description : Wait until a new interrupt comes, to save power.
;Input       : None.
;Output      : None.
;Return      : None.
;*****************************************************************************/
__RME_A6M_Wait_Int
        WFE
        BX                  LR
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
__RME_User_Enter
        MSR                 PSP,R1              ;Set the stack pointer
        MOVS                R4,#0x03            ;Unprevileged thread mode
        MSR                 CONTROL,R4
        ISB
        MOV                 R1,R0               ;Save the entry to R1
        MOV                 R0,R2               ;Save CPUID(0) to R0
        BX                  R1                  ;Branch to target
;/* End Function:__RME_User_Enter ********************************************/

;/* Function:SysTick_Handler **************************************************
;Description : The System Tick Timer handler routine. This will in fact call a
;              C function to resolve the system service routines.             
;Input       : None.
;Output      : None.
;Return      : None.
;*****************************************************************************/
SysTick_Handler
        RME_A6M_SAVE

        MOV                 R0,SP               ;Pass in the regs
        BL                  __RME_A6M_Tim_Handler

        RME_A6M_LOAD
;/* End Function:SysTick_Handler *********************************************/

;/* Function:SVC_Handler ******************************************************
;Description : The SVC handler routine. This will in fact call a C function to resolve
;              the system service routines.             
;Input       : None.
;Output      : None.
;Return      : None.
;*****************************************************************************/
SVC_Handler
        RME_A6M_SAVE

        MOV                 R0,SP               ;Pass in the regs
        BL                  __RME_A6M_Svc_Handler

        RME_A6M_LOAD
;/* End Function:SVC_Handler *************************************************/

;/* Function:NMI/HardFault_Handler ********************************************
;Description : The multi-purpose handler routine. This will in fact call
;              a C function to resolve the system service routines.             
;Input       : None.
;Output      : None.
;Return      : None.
;*****************************************************************************/
NMI_Handler
PendSV_Handler
HardFault_Handler
        RME_A6M_SAVE

        MOV                 R0,SP               ;Pass in the regs
        BL                  __RME_A6M_Exc_Handler

        RME_A6M_LOAD
        LTORG
;/* End Function:NMI/HardFault_Handler ***************************************/

;/* Function:___RME_A6M_MPU_Set ***********************************************
;Description : Set the MPU context. 1-to-16-region versions are all declared here.
;Input       : R0 - The pointer to the MPU content.
;Output      : None.
;Return      : None.
;*****************************************************************************/
        SECTION             .rme_mpu:CODE:NOROOT(3)
        THUMB
        REQUIRE8
        PRESERVE8

MPU_PRE MACRO
        PUSH                {R4-R7}             ;Save registers
        LDR                 R2,=0xE000ED9C      ;RBAR
        LDR                 R3,=0xE000EDA0      ;RASR
        ENDM

MPU_SET MACRO
        LDMIA               R0!,{R4-R5}         ;Read settings
        STR                 R4,[R2]             ;Program
        STR                 R5,[R3]
        ENDM

MPU_SET2 MACRO
        LDMIA               R0!,{R4-R7}
        STR                 R4,[R2]
        STR                 R5,[R3]
        STR                 R6,[R2]
        STR                 R7,[R3]
        ENDM

MPU_SET3 MACRO
        MPU_SET2
        MPU_SET
        ENDM

MPU_SET4 MACRO
        MPU_SET2
        MPU_SET2
        ENDM

MPU_POST MACRO
        POP                 {R4-R7}             ;Restore registers
        ISB                                     ;Barrier
        BX                  LR
        ENDM

;1-region version
___RME_A6M_MPU_Set1
        MPU_PRE
        MPU_SET
        MPU_POST

;2-region version
___RME_A6M_MPU_Set2
        MPU_PRE
        MPU_SET2
        MPU_POST

;3-region version
___RME_A6M_MPU_Set3
        MPU_PRE
        MPU_SET3
        MPU_POST

;4-region version
___RME_A6M_MPU_Set4
        MPU_PRE
        MPU_SET4
        MPU_POST

;5-region version
___RME_A6M_MPU_Set5
        MPU_PRE
        MPU_SET4
        MPU_SET
        MPU_POST

;6-region version
___RME_A6M_MPU_Set6
        MPU_PRE
        MPU_SET4
        MPU_SET2
        MPU_POST

;7-region version
___RME_A6M_MPU_Set7
        MPU_PRE
        MPU_SET4
        MPU_SET3
        MPU_POST

;8-region version
___RME_A6M_MPU_Set8
        MPU_PRE
        MPU_SET4
        MPU_SET4
        MPU_POST
        LTORG

;9-region version
___RME_A6M_MPU_Set9
        MPU_PRE
        MPU_SET4
        MPU_SET4
        MPU_SET
        MPU_POST

;10-region version
___RME_A6M_MPU_Set10
        MPU_PRE
        MPU_SET4
        MPU_SET4
        MPU_SET2
        MPU_POST

;11-region version
___RME_A6M_MPU_Set11
        MPU_PRE
        MPU_SET4
        MPU_SET4
        MPU_SET3
        MPU_POST

;12-region version
___RME_A6M_MPU_Set12
        MPU_PRE
        MPU_SET4
        MPU_SET4
        MPU_SET4
        MPU_POST

;13-region version
___RME_A6M_MPU_Set13
        MPU_PRE
        MPU_SET4
        MPU_SET4
        MPU_SET4
        MPU_SET
        MPU_POST

;14-region version
___RME_A6M_MPU_Set14
        MPU_PRE
        MPU_SET4
        MPU_SET4
        MPU_SET4
        MPU_SET2
        MPU_POST

;15-region version
___RME_A6M_MPU_Set15
        MPU_PRE
        MPU_SET4
        MPU_SET4
        MPU_SET4
        MPU_SET3
        MPU_POST

;16-region version
___RME_A6M_MPU_Set16
        MPU_PRE
        MPU_SET4
        MPU_SET4
        MPU_SET4
        MPU_SET4
        MPU_POST
        LTORG
;/* End Function:___RME_A6M_MPU_Set ******************************************/

        END
;/* End Of File **************************************************************/

;/* Copyright (C) Evo-Devo Instrum. All rights reserved **********************/
