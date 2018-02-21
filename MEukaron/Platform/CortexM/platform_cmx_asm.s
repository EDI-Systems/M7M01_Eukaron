;/*****************************************************************************
;Filename    : platform_cmx.s
;Author      : pry
;Date        : 19/01/2017
;Description : The Cortex-M assembly support of the RME RTOS.
;*****************************************************************************/

;/*The ARM Cortex-M3/4/7 Structure*********************************************
;R0-R7:General purpose registers that are accessible. 
;R8-R12:general purpose registers that can only be reached by 32-bit instructions.
;R13:SP/SP_process/SP_main    Stack pointer
;R14:LR                       Link Register(used for returning from a subfunction)
;R15:PC                       Program counter.
;IPSR                         Interrupt Program Status Register.
;APSR                         Application Program Status Register.
;EPSR                         Execute Program Status Register.
;The above 3 registers are saved into the stack in combination(xPSR).
;
;The ARM Cortex-M4 include a single-precision FPU, and the Cortex-M7 will feature
;a double-precision FPU.
;*****************************************************************************/
            
;/* Begin Header *************************************************************/
                ;The align is "(2^3)/8=1(Byte)." In fact it does not take effect.            
                AREA            ARCH,CODE,READONLY,ALIGN=3                     
                
                THUMB
                REQUIRE8
                PRESERVE8
;/* End Header ***************************************************************/

;/* Begin Exports ************************************************************/
                ;Disable all interrupts
                EXPORT          __RME_Disable_Int
                ;Enable all interrupts            
                EXPORT          __RME_Enable_Int
                ;Get the MSB in a word
                EXPORT          __RME_MSB_Get
                ;Kernel main function wrapper
                EXPORT          _RME_Kmain 
                ;Entering of the user mode
                EXPORT          __RME_Enter_User_Mode   
                ;The system tick timer handler routine
                EXPORT          SysTick_Handler
                ;The system call handler routine              
                EXPORT          SVC_Handler
                ;The fault handler routines
                EXPORT          NMI_Handler
                EXPORT          HardFault_Handler
                EXPORT          MemManage_Handler
                EXPORT          BusFault_Handler
                EXPORT          UsageFault_Handler
                ;The FPU register save routine
                EXPORT          ___RME_CMX_Thd_Cop_Save
                ;The FPU register restore routine
                EXPORT          ___RME_CMX_Thd_Cop_Restore
                ;The MPU setup routine
                EXPORT          ___RME_CMX_MPU_Set                
;/* End Exports **************************************************************/

;/* Begin Imports ************************************************************/
                ;The kernel entry of RME. This will be defined in C language.
                IMPORT          RME_Kmain
                ;The system call handler of RME. This will be defined in C language.
                IMPORT          _RME_Svc_Handler
                ;The system tick handler of RME. This will be defined in C language.
                IMPORT          _RME_Tick_Handler
                ;The memory management fault handler of RME. This will be defined in C language.
                IMPORT          __RME_CMX_Fault_Handler
;/* End Imports **************************************************************/

;/* Begin Function:__RME_Disable_Int ******************************************
;Description    : The function for disabling all interrupts.
;Input          : None.
;Output         : None.    
;Register Usage : None.                                  
;*****************************************************************************/    
__RME_Disable_Int
                ;Disable all interrupts (I is primask, F is faultmask.)
                CPSID     I 
                BX        LR                                                 
;/* End Function:__RME_Disable_Int *******************************************/

;/* Begin Function:__RME_Enable_Int *******************************************
;Description    : The Function For Enabling all interrupts.
;Input          : None.
;Output         : None.    
;Register Usage : None.                                  
;*****************************************************************************/
__RME_Enable_Int
                ;Enable all interrupts.
                CPSIE    I 
                BX       LR
;/* End Function:__RME_Enable_Int ********************************************/

;/* Begin Function:_RME_Kmain *************************************************
;Description    : The entry address of the kernel. Never returns.
;Input          : ptr_t Stack - The stack address to set SP to.
;Output         : None.
;Return         : None.   
;Register Usage : None. 
;*****************************************************************************/
_RME_Kmain
                MOV      SP,R0
                B        RME_Kmain
                B        .
;/* End Function:_RME_Kmain **************************************************/

;/* Begin Function:__RME_MSB_Get **********************************************
;Description    : Get the MSB of the word.
;Input          : ptr_t Val - The value.
;Output         : None.
;Return         : ptr_t - The MSB position.   
;Register Usage : None. 
;*****************************************************************************/
__RME_MSB_Get
                CLZ      R1,R0
                MOV      R0,#31
                SUB      R0,R1
                BX       LR
;/* End Function:__RME_MSB_Get ***********************************************/

;/* Begin Function:__RME_Enter_User_Mode **************************************
;Description : Entering of the user mode, after the system finish its preliminary
;              booting. The function shall never return. This function should only
;              be used to boot the first process in the system.
;Input       : R0 - The user execution startpoint.
;              R1 - The user stack.
;Output      : None.                              
;*****************************************************************************/
__RME_Enter_User_Mode
                MSR       PSP,R1                       ; Set the stack pointer
                MOV       R4,#0x03                     ; Unprevileged thread mode
                MSR       CONTROL,R4
                BLX       R0                           ; Branch to our target
                B         .                            ; Capture faults
;/* End Function:__RME_Enter_User_Mode ***************************************/

;/* Begin Function:SysTick_Handler ********************************************
;Description : The System Tick Timer handler routine. This will in fact call a
;              C function to resolve the system service routines.             
;Input       : None.
;Output      : None.
;*****************************************************************************/
SysTick_Handler
                PUSH      {LR}
                PUSH      {R4-R11}         ; Spill all the general purpose registers; empty descending
                MRS       R0,PSP
                PUSH      {R0}
                
                MOV       R0,SP            ; Pass in the pt_regs parameter, and call the handler.
                BL        _RME_Tick_Handler
                
                POP       {R0}
                MSR       PSP,R0
                POP       {R4-R11}
                POP       {PC}             ; Now we reset the PC.
                B         .                ; Capture faults
;/* End Function:SysTick_Handler *********************************************/

;/* Begin Function:SVC_Handler ************************************************
;Description : The SVC handler routine. This will in fact call a C function to resolve
;              the system service routines.             
;Input       : None.
;Output      : None.
;*****************************************************************************/
SVC_Handler
                PUSH      {LR}
                PUSH      {R4-R11}         ; Spill all the general purpose registers; empty descending
                MRS       R0,PSP
                PUSH      {R0}
                
                MOV       R0,SP            ; Pass in the pt_regs parameter, and call the handler.
                BL        _RME_Svc_Handler
                
                POP       {R0}
                MSR       PSP,R0
                POP       {R4-R11}
                POP       {PC}             ; Now we reset the PC.
                B         .                ; Capture faults
;/* End Function:SVC_Handler *************************************************/

;/* Begin Function:NMI/HardFault/MemManage/BusFault/UsageFault_Handler ********
;Description : The multi-purpose handler routine. This will in fact call
;              a C function to resolve the system service routines.             
;Input       : None.
;Output      : None.
;*****************************************************************************/
NMI_Handler
                NOP
PendSV_Handler
                NOP
DebugMon_Handler
                NOP
HardFault_Handler
                NOP
                
MemManage_Handler
                NOP
BusFault_Handler
                NOP
UsageFault_Handler
                PUSH      {LR}
                PUSH      {R4-R11}         ; Spill all the general purpose registers; empty descending
                MRS       R0,PSP
                PUSH      {R0}
                
                MOV       R0,SP            ; Pass in the pt_regs parameter, and call the handler.
                BL        __RME_CMX_Fault_Handler
                
                POP       {R0}
                MSR       PSP,R0
                POP       {R4-R11}
                POP       {PC}             ; Now we reset the PC.
                B         .                ; Capture faults
;/* End Function:NMI/HardFault/MemManage/BusFault/UsageFault_Handler *********/

;/* Begin Function:___RME_CMX_Thd_Cop_Save ************************************
;Description : Save the coprocessor context on switch.         
;Input       : R0 - The pointer to the coprocessor struct.
;Output      : None.
;*****************************************************************************/
___RME_CMX_Thd_Cop_Save
                ;Use DCI to avoid compilation errors when FPU not enabled. Anyway,
                ;this will not be called when FPU not enabled.
                DCI       0xED20        ; VSTMDB    R0!,{S16-S31}
                DCI       0x8A10        ; Save all the FPU registers
                BX        LR
                B         .
;/* End Function:___RME_CMX_Thd_Cop_Save *************************************/

;/* Begin Function:___RME_CMX_Thd_Cop_Restore *********************************
;Description : Restore the coprocessor context on switch.             
;Input       : R0 - The pointer to the coprocessor struct.
;Output      : None.
;*****************************************************************************/
___RME_CMX_Thd_Cop_Restore                
                ;Use DCI to avoid compilation errors when FPU not enabled. Anyway,
                ;this will not be called when FPU not enabled.
                DCI       0xECB0        ; VLDMIA    R0!,{S16-S31}
                DCI       0x8A10        ; Restore all the FPU registers
                BX        LR
                B         .
;/* End Function:___RME_CMX_Thd_Cop_Restore **********************************/

;/* Begin Function:___RME_CMX_MPU_Set *****************************************
;Description : Set the MPU context. We write 8 registers at a time to increase efficiency.            
;Input       : R0 - The pointer to the MPU content.
;Output      : None.
;*****************************************************************************/
___RME_CMX_MPU_Set
                PUSH      {R4-R9}          ; Clobber registers manually
                LDR       R1,=0xE000ED9C   ; The base address of MPU RBAR and all 4 registers
                LDMIA     R0!,{R2-R9}      ; Read MPU settings from the array, and increase pointer
                STMIA     R1,{R2-R9}       ; Write the settings but do not increase pointer
                LDMIA     R0!,{R2-R9}
                STMIA     R1,{R2-R9}
                POP       {R4-R9}
                DSB                        ; Make sure that the MPU update completes.
                ISB                        ; Fetch new instructions
                BX        LR          
                B         .                ; Capture faults
;/* End Function:___RME_CMX_MPU_Set ******************************************/

                END
;/* End Of File **************************************************************/

;/* Copyright (C) 2011-2013 Evo-Devo Instrum. All rights reserved ************/
