/******************************************************************************
Filename    : rme_platform_rv32p_gcc.s
Author      : pry
Date        : 19/01/2017
Description : The GCC assembly stubs for RV32G PMP-based microcontrollers.
              This only uses two modes: The U mode, and the M mode.
              Due to the fact that RISC-V have many incompatible implementations,
              we don't make any assumptions about its startup and interrupt
              vectoring. Such details are handled by chip-specific assembly.
******************************************************************************/

/* The RISC-V RV32P Architecture **********************************************
R0:Hardwired register containing "0".
R1-R31:General purpose registers that can only be reached by 32-bit instructions.
PC:Program counter.
Detailed usage convention for X0-X31:
No.    Name       Explanation
X0     $zero      hard-wired zero
X1     $ra        return address (caller-save)
X2     $sp        stack pointer (callee-save)
X3     $gp        global pointer
X4     $tp        thread pointer
X5     $t0        temporary (caller-save)
X6     $t1        temporary (caller-save)
X7     $t2        temporary (caller-save)
X8     $s0/fp     saved register/frame pointer (callee-save)
X9     $s1        saved register (callee-save)
X10    $a0        argument/return value (caller-save)
X11    $a1        argument/return value (caller-save)
X12    $a2        argument (caller-save)
X13    $a3        argument (caller-save)
X14    $a4        argument (caller-save)
X15    $a5        argument (caller-save)
X16    $a6        argument (caller-save)
X17    $a7        argument (caller-save)
X18    $s2        saved register (callee-save)
X19    $s3        saved register (callee-save)
X20    $s4        saved register (callee-save)
X21    $s5        saved register (callee-save)
X22    $s6        saved register (callee-save)
X23    $s7        saved register (callee-save)
X24    $s8        saved register (callee-save)
X25    $s9        saved register (callee-save)
X26    $s10       saved register (callee-save)
X27    $s11       saved register (callee-save)
X28    $t3        temporary (caller-save)
X29    $t4        temporary (caller-save)
X30    $t5        temporary (caller-save)
X31    $t6        temporary (caller-save)
PC     $pc        program counter

On chips that have a FPU, the layout of the FPU registers are:
F0     $ft0       temporary (caller-save)
F1     $ft1       temporary (caller-save)
F2     $ft2       temporary (caller-save)
F3     $ft3       temporary (caller-save)
F4     $ft4       temporary (caller-save)
F5     $ft5       temporary (caller-save)
F6     $ft6       temporary (caller-save)
F7     $ft7       temporary (caller-save)
F8     $fs0       saved register (callee-save)
F9     $fs1       saved register (callee-save)
F10    $fa0       argument/return value (caller-save)
F11    $fa1       argument/return value (caller-save)
F12    $fa2       argument (caller-save)
F13    $fa3       argument (caller-save)
F14    $fa4       argument (caller-save)
F15    $fa5       argument (caller-save)
F16    $fa6       argument (caller-save)
F17    $fa7       argument (caller-save)
F18    $fs2       saved register (callee-save)
F19    $fs3       saved register (callee-save)
F20    $fs4       saved register (callee-save)
F21    $fs5       saved register (callee-save)
F22    $fs6       saved register (callee-save)
F23    $fs7       saved register (callee-save)
F24    $fs8       saved register (callee-save)
F25    $fs9       saved register (callee-save)
F26    $fs10      saved register (callee-save)
F27    $fs11      saved register (callee-save)
F28    $ft8       temporary (caller-save)
F29    $ft9       temporary (caller-save)
F30    $ft10      temporary (caller-save)
F31    $ft11      temporary (caller-save)
******************************************************************************/

/* Import ********************************************************************/
    /* Locations provided by the linker */
    .extern             __RME_Stack
    .extern             __RME_Global
    .extern             __RME_Data_Load
    .extern             __RME_Data_Start
    .extern             __RME_Data_End
    .extern             __RME_Zero_Start
    .extern             __RME_Zero_End
    /* Handlers */
    .extern             __RME_RV32GP_Handler
    .extern             __RME_RV32GP_Lowlvl_Preinit
    /* Kernel entry */
    .extern             main
/* End Import ****************************************************************/

/* Export ********************************************************************/
    /* Real entry after initial startup */
    .global             __RME_Entry_After
    /* Disable all interrupts */
    .global             __RME_Int_Disable
    /* Enable all interrupts */
    .global             __RME_Int_Enable
    /* Entering of the user mode */
    .global             __RME_User_Enter
    /* A full barrier */
    .global             __RME_RV32P_Barrier
    /* CSR manipulations */
    .global             ___RME_RV32P_MCAUSE_Get
    .global             ___RME_RV32P_MTVAL_Set
    .global             ___RME_RV32P_MTVAL_Get
    .global             ___RME_RV32P_MCYCLE_Get
    .global             ___RME_RV32P_MISA_Get
    .global             ___RME_RV32P_MSTATUS_Get
    .global             ___RME_RV32P_MSTATUS_Set
    /* Handler for everything */
    .global             __RME_RV32P_Handler
    /* Coprocessor save/load */
    .global             ___RME_RV32P_Thd_Cop_Clear_RVF
    .global             ___RME_RV32P_Thd_Cop_Clear_RVFD
    .global             ___RME_RV32P_Thd_Cop_Save_RVF
    .global             ___RME_RV32P_Thd_Cop_Save_RVFD
    .global             ___RME_RV32P_Thd_Cop_Load_RVF
    .global             ___RME_RV32P_Thd_Cop_Load_RVFD
    /* The MPU setup routine */
    .global             ___RME_RV32P_PMP_Set1
    .global             ___RME_RV32P_PMP_Set2
    .global             ___RME_RV32P_PMP_Set3
    .global             ___RME_RV32P_PMP_Set4
    .global             ___RME_RV32P_PMP_Set5
    .global             ___RME_RV32P_PMP_Set6
    .global             ___RME_RV32P_PMP_Set7
    .global             ___RME_RV32P_PMP_Set8
    .global             ___RME_RV32P_PMP_Set9
    .global             ___RME_RV32P_PMP_Set10
    .global             ___RME_RV32P_PMP_Set11
    .global             ___RME_RV32P_PMP_Set12
    .global             ___RME_RV32P_PMP_Set13
    .global             ___RME_RV32P_PMP_Set14
    .global             ___RME_RV32P_PMP_Set15
    .global             ___RME_RV32P_PMP_Set16
/* End Export ****************************************************************/

/* Entry *********************************************************************/
    .section            .text.__rme_entry_after
    .align              3

__RME_Entry_After:
    /* Set GP and SP */
    .option             push
    .option             norelax
    LA                  gp,__RME_Global
    .option             pop
    LA                  sp,__RME_Stack
    /* Preinitialize hardware before any initialization */
    CALL                __RME_RV32P_Lowlvl_Preinit
    /* Load data section from flash to RAM */
    LA                  a0,__RME_Data_Load
    LA                  a1,__RME_Data_Start
    LA                  a2,__RME_Data_End
__RME_Data_Load:
    LW                  t0,(a0)
    SW                  t0,(a1)
    ADDI                a0,a0,4
    ADDI                a1,a1,4
    BLTU                a1,a2,__RME_Data_Load
    /* Clear bss zero section */
    LA                  a0,__RME_Zero_Start
    LA                  a1,__RME_Zero_End
__RME_Zero_Clear:
    SW                  zero,(a0)
    ADDI                a0,a0,4
    BLTU                a0,a1,__RME_Zero_Clear
    /* Branch to main function */
    J                   main
/* End Entry *****************************************************************/

/* Function:__RME_Int_Disable *************************************************
Description : The function for disabling all interrupts.
Input       : None.
Output      : None. 
Return      : None.                                
******************************************************************************/
    .section            .text.__rme_int_disable
    .align              3

__RME_Int_Disable:
    /* Disable all interrupts */
    CSRCI               mstatus, 8
    RET
/* End Function:__RME_Int_Disable ********************************************/

/* Function:__RME_Int_Enable **************************************************
Description : The function for enabling all interrupts.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
    .section            .text.__rme_int_enable
    .align              3

__RME_Int_Enable:
    /* Enable all interrupts. */
    CSRSI               mstatus, 8
    RET
/* End Function:__RME_Int_Enable *********************************************/

/* Function:__RME_RV32P_Barrier ***********************************************
Description : A full data/instruction barrier.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
    .section            .text.__rme_rv32p_barrier
    .align              3

___RME_RV32P_Barrier:
    FENCE
    FENCE.I
    RET
/* End Function:__RME_RV32P_Barrier ******************************************/

/* Function:___RME_RV32P_MCAUSE_Get *******************************************
Description : Get the MCAUSE register content.
Input       : None.
Output      : None.
Return      : $a0 - MCAUSE value.
******************************************************************************/
    .section            .text.___rme_rv32p_mcause_get
    .align              3

___RME_RV32P_MCAUSE_Get:
    CSRR                a0, mcause
    RET
/* End Function:___RME_RV32P_MCAUSE_Get **************************************/

/* Function:___RME_RV32P_MTVAL_Get ********************************************
Description : Get the MTVAL register content.
Input       : None.
Output      : None.
Return      : $a0 - MCAUSE value.
******************************************************************************/
    .section            .text.___rme_rv32p_mtval_get
    .align              3

___RME_RV32P_MTVAL_Get:
    CSRR                a0, mtval
    RET
/* End Function:___RME_RV32P_MTVAL_Get ***************************************/

/* Function:___RME_RV32P_MCYCLE_Get *******************************************
Description : Get the MCYCLE register content.
Input       : None.
Output      : None.
Return      : $a0 - MCYCLE value.
******************************************************************************/
    .section            .text.___rme_rv32p_mcycle_get
    .align              3

___RME_RV32P_MCYCLE_Get:
    CSRR                a0, mcycle
    RET
/* End Function:___RME_RV32P_MCYCLE_Get **************************************/

/* Function:___RME_RV32P_MISA_Get *********************************************
Description : Get the MISA register content.
Input       : None.
Output      : None.
Return      : $a0 - MISA value.
******************************************************************************/
    .section            .text.___rme_rv32p_misa_get
    .align              3

___RME_RV32P_MISA_Get:
    CSRR                a0, misa
    RET
/* End Function:___RME_RV32P_MISA_Get ****************************************/

/* Function:___RME_RV32P_MSTATUS_Get ******************************************
Description : Get the MSTATUS register content.
Input       : None.
Output      : None.
Return      : $a0 - MSTATUS value.
******************************************************************************/
    .section            .text.___rme_rv32p_mstatus_get
    .align              3

___RME_RV32P_MSTATUS_Get:
    CSRR                a0, mstatus
    RET
/* End Function:___RME_RV32P_MSTATUS_Get *************************************/

/* Function:___RME_RV32P_MSTATUS_Set ******************************************
Description : Set the MSTATUS register content.
Input       : $a0 - MSTATUS value.
Output      : None.
Return      : None.
******************************************************************************/
    .section            .text.___rme_rv32p_mstatus_set
    .align              3

___RME_RV32P_MSTATUS_Set:
    CSRW                mstatus, a0
    RET
/* End Function:___RME_RV32P_MSTATUS_Set *************************************/

/* Function:__RME_User_Enter **************************************************
Description : Entering of the user mode, after the system finish its preliminary
              booting. The function shall never return. This function should only
              be used to boot the first process in the system.
Input       : rme_ptr_t Entry - The user execution startpoint.
              rme_ptr_t Stack - The user stack.
              rme_ptr_t CPUID - The CPUID.
Output      : None.
Return      : None.
******************************************************************************/
    .section            .text.__rme_user_enter
    .align              3

__RME_User_Enter:
    /* FS clear, previous mode = U, previous interrupt enabled */
    LI                  t0, 0x00001800
    CSRW                mstatus, t0
    CSRW                mepc, a0
    MV                  sp, a1
    MV                  a0, a2
    MRET
/* End Function:__RME_User_Enter *********************************************/

/* Function:__RME_RV32P_Handler ***********************************************
Description : The generic handler routine. Assuming no vector mode is available
              regardless. The C program instead of this is responsible for
              software PC adjustments before and after an exception.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
    .section            .text.__rme_rv32p_handler
    .align              3

__RME_RV32P_Handler:
    CSRRW               sp,mscratch,sp
    /* Allocate stack space for context saving */
    ADDI                sp,sp,-33*4
    /* Save all general-purpose registers - RV32G does not have any flag registers */
    SW                  x31,32*4(sp)
    SW                  x30,31*4(sp)
    SW                  x29,30*4(sp)
    SW                  x28,29*4(sp)
    SW                  x27,28*4(sp)
    SW                  x26,27*4(sp)
    SW                  x25,26*4(sp)
    SW                  x24,25*4(sp)
    SW                  x23,24*4(sp)
    SW                  x22,23*4(sp)
    SW                  x21,22*4(sp)
    SW                  x20,21*4(sp)
    SW                  x19,20*4(sp)
    SW                  x18,19*4(sp)
    SW                  x17,18*4(sp)
    SW                  x16,17*4(sp)
    SW                  x15,16*4(sp)
    SW                  x14,15*4(sp)
    SW                  x13,14*4(sp)
    SW                  x12,13*4(sp)
    SW                  x11,12*4(sp)
    SW                  x10,11*4(sp)
    SW                  x9,10*4(sp)
    SW                  x8,9*4(sp)
    SW                  x7,8*4(sp)
    SW                  x6,7*4(sp)
    SW                  x5,6*4(sp)
    SW                  x4,5*4(sp)
    SW                  x3,4*4(sp)
    /* Save sp (x2) */
    CSRR                x3,mscratch
    SW                  x3,3*4(sp)
    /* Save x1 */
    SW                  x1,2*4(sp)
    /* Save pc (mepc) */
    CSRR                x3,mepc
    SW                  x3,1*4(sp)
    /* Save mstatus */
    CSRR                x3,mstatus
    SW                  x3,0*4(sp)

    /* Load gp for kernel - defined by linker script */
    .option push
    .option norelax
    LA                  gp,__RME_Global
    .option pop
    /* Call system main interrupt handler */
    CALL                _RME_RV32P_Handler

    /* Load mstatus */
    LW                  x3,0*4(sp)
    CSRW                mstatus, x3
    /* Load pc (into mepc) */
    LW                  x3,1*4(sp)
    CSRW                mepc, x3
    /* Load x1 */
    LW                  x1,2*4(sp)
    /* Load sp (x2, into mscratch) */
    LW                  x3,3*4(sp)
    CSRW                mscratch,x3
    /* Load all general-purpose registers - RV32G does not have any flag registers */
    LW                  x3,4*4(sp)
    LW                  x4,5*4(sp)
    LW                  x5,6*4(sp)
    LW                  x6,7*4(sp)
    LW                  x7,8*4(sp)
    LW                  x8,9*4(sp)
    LW                  x9,10*4(sp)
    LW                  x10,11*4(sp)
    LW                  x11,12*4(sp)
    LW                  x12,13*4(sp)
    LW                  x13,14*4(sp)
    LW                  x14,15*4(sp)
    LW                  x15,16*4(sp)
    LW                  x16,17*4(sp)
    LW                  x17,18*4(sp)
    LW                  x18,19*4(sp)
    LW                  x19,20*4(sp)
    LW                  x20,21*4(sp)
    LW                  x21,22*4(sp)
    LW                  x22,23*4(sp)
    LW                  x23,24*4(sp)
    LW                  x24,25*4(sp)
    LW                  x25,26*4(sp)
    LW                  x26,27*4(sp)
    LW                  x27,28*4(sp)
    LW                  x28,29*4(sp)
    LW                  x29,30*4(sp)
    LW                  x30,31*4(sp)
    LW                  x31,32*4(sp)
    ADDI                sp,sp,33*4
    /* Return to user mode */
    CSRRW               sp,mscratch,sp
    MRET
/* End Function:_RME_RV32P_Handler *******************************************/

/* Function:___RME_RV32P_Thd_Cop_Clear ****************************************
Description : Clean up the coprocessor state so that the FP information is not
              leaked when switching from a FPU-enabled thread to a FPU-disabled
              thread. Contains two versions for RVF and RVD.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
/* RVF ***********************************************************************/
    .section            .text.___rme_rv32p_thd_cop_clear_rvf
    .align              3

___RME_RV32P_Thd_Cop_Clear_RVF:
    .hword              0x1073              /* FSCSR       zero */
    .hword              0x0030
    .hword              0x7053              /* FCVT.S.W    f0,zero */
    .hword              0xD000
    .hword              0x70D3              /* FCVT.S.W    f1,zero */
    .hword              0xD000
    .hword              0x7153              /* FCVT.S.W    f2,zero */
    .hword              0xD000
    .hword              0x71D3              /* FCVT.S.W    f3,zero */
    .hword              0xD000
    .hword              0x7253              /* FCVT.S.W    f4,zero */
    .hword              0xD000
    .hword              0x72D3              /* FCVT.S.W    f5,zero */
    .hword              0xD000
    .hword              0x7353              /* FCVT.S.W    f6,zero */
    .hword              0xD000
    .hword              0x73D3              /* FCVT.S.W    f7,zero */
    .hword              0xD000
    .hword              0x7453              /* FCVT.S.W    f8,zero */
    .hword              0xD000
    .hword              0x74D3              /* FCVT.S.W    f9,zero */
    .hword              0xD000
    .hword              0x7553              /* FCVT.S.W    f10,zero */
    .hword              0xD000
    .hword              0x75D3              /* FCVT.S.W    f11,zero */
    .hword              0xD000
    .hword              0x7653              /* FCVT.S.W    f12,zero */
    .hword              0xD000
    .hword              0x76D3              /* FCVT.S.W    f13,zero */
    .hword              0xD000
    .hword              0x7753              /* FCVT.S.W    f14,zero */
    .hword              0xD000
    .hword              0x77D3              /* FCVT.S.W    f15,zero */
    .hword              0xD000
    .hword              0x7853              /* FCVT.S.W    f16,zero */
    .hword              0xD000
    .hword              0x78D3              /* FCVT.S.W    f17,zero */
    .hword              0xD000
    .hword              0x7953              /* FCVT.S.W    f18,zero */
    .hword              0xD000
    .hword              0x79D3              /* FCVT.S.W    f19,zero */
    .hword              0xD000
    .hword              0x7A53              /* FCVT.S.W    f20,zero */
    .hword              0xD000
    .hword              0x7AD3              /* FCVT.S.W    f21,zero */
    .hword              0xD000
    .hword              0x7B53              /* FCVT.S.W    f22,zero */
    .hword              0xD000
    .hword              0x7BD3              /* FCVT.S.W    f23,zero */
    .hword              0xD000
    .hword              0x7C53              /* FCVT.S.W    f24,zero */
    .hword              0xD000
    .hword              0x7CD3              /* FCVT.S.W    f25,zero */
    .hword              0xD000
    .hword              0x7D53              /* FCVT.S.W    f26,zero */
    .hword              0xD000
    .hword              0x7DD3              /* FCVT.S.W    f27,zero */
    .hword              0xD000
    .hword              0x7E53              /* FCVT.S.W    f28,zero */
    .hword              0xD000
    .hword              0x7ED3              /* FCVT.S.W    f29,zero */
    .hword              0xD000
    .hword              0x7F53              /* FCVT.S.W    f30,zero */
    .hword              0xD000
    .hword              0x7FD3              /* FCVT.S.W    f31,zero */
    .hword              0xD000
    RET

/* RVD ***********************************************************************/
    .section            .text.___rme_rv32p_thd_cop_clear_rvd
    .align              3

___RME_RV32P_Thd_Cop_Clear_RVD:
    .hword              0x1073              /* FSCSR       zero */
    .hword              0x0030
    .hword              0x0053              /* FCVT.D.W    f0,zero */
    .hword              0xD200
    .hword              0x00D3              /* FCVT.D.W    f1,zero */
    .hword              0xD200
    .hword              0x0153              /* FCVT.D.W    f2,zero */
    .hword              0xD200
    .hword              0x01D3              /* FCVT.D.W    f3,zero */
    .hword              0xD200
    .hword              0x0253              /* FCVT.D.W    f4,zero */
    .hword              0xD200
    .hword              0x02D3              /* FCVT.D.W    f5,zero */
    .hword              0xD200
    .hword              0x0353              /* FCVT.D.W    f6,zero */
    .hword              0xD200
    .hword              0x03D3              /* FCVT.D.W    f7,zero */
    .hword              0xD200
    .hword              0x0453              /* FCVT.D.W    f8,zero */
    .hword              0xD200
    .hword              0x04D3              /* FCVT.D.W    f9,zero */
    .hword              0xD200
    .hword              0x0553              /* FCVT.D.W    f10,zero */
    .hword              0xD200
    .hword              0x05D3              /* FCVT.D.W    f11,zero */
    .hword              0xD200
    .hword              0x0653              /* FCVT.D.W    f12,zero */
    .hword              0xD200
    .hword              0x06D3              /* FCVT.D.W    f13,zero */
    .hword              0xD200
    .hword              0x0753              /* FCVT.D.W    f14,zero */
    .hword              0xD200
    .hword              0x07D3              /* FCVT.D.W    f15,zero */
    .hword              0xD200
    .hword              0x0853              /* FCVT.D.W    f16,zero */
    .hword              0xD200
    .hword              0x08D3              /* FCVT.D.W    f17,zero */
    .hword              0xD200
    .hword              0x0953              /* FCVT.D.W    f18,zero */
    .hword              0xD200
    .hword              0x09D3              /* FCVT.D.W    f19,zero */
    .hword              0xD200
    .hword              0x0A53              /* FCVT.D.W    f20,zero */
    .hword              0xD200
    .hword              0x0AD3              /* FCVT.D.W    f21,zero */
    .hword              0xD200
    .hword              0x0B53              /* FCVT.D.W    f22,zero */
    .hword              0xD200
    .hword              0x0BD3              /* FCVT.D.W    f23,zero */
    .hword              0xD200
    .hword              0x0C53              /* FCVT.D.W    f24,zero */
    .hword              0xD200
    .hword              0x0CD3              /* FCVT.D.W    f25,zero */
    .hword              0xD200
    .hword              0x0D53              /* FCVT.D.W    f26,zero */
    .hword              0xD200
    .hword              0x0DD3              /* FCVT.D.W    f27,zero */
    .hword              0xD200
    .hword              0x0E53              /* FCVT.D.W    f28,zero */
    .hword              0xD200
    .hword              0x0ED3              /* FCVT.D.W    f29,zero */
    .hword              0xD200
    .hword              0x0F53              /* FCVT.D.W    f30,zero */
    .hword              0xD200
    .hword              0x0FD3              /* FCVT.D.W    f31,zero */
    .hword              0xD200
    RET
/* End Function:___RME_RV32P_Thd_Cop_Clear ***********************************/

/* Function:___RME_RV32P_Thd_Cop_Save *****************************************
Description : Save the coprocessor context on switch.
              Contains two versions for RVF and RVD.
Input       : A0 - The pointer to the coprocessor struct.
Output      : None.
Return      : None.
******************************************************************************/
/* RVF */
    .section            .text.___rme_rv32p_thd_cop_save_rvf
    .align              3

___RME_RV32P_Thd_Cop_Save_RVF:
    .hword              0x22F3              /* FRCSR   t0 */
    .hword              0x0030
    SW                  t0,(a0)
    ADDI                a0,a0,4
    .hword              0x2027              /* FSW     f0,0*4(a0) */
    .hword              0x0005
    .hword              0x2227              /* FSW     f1,1*4(a0) */
    .hword              0x0015
    .hword              0x2427              /* FSW     f2,2*4(a0) */
    .hword              0x0025
    .hword              0x2627              /* FSW     f3,3*4(a0) */
    .hword              0x0035
    .hword              0x2827              /* FSW     f4,4*4(a0) */
    .hword              0x0045
    .hword              0x2A27              /* FSW     f5,5*4(a0) */
    .hword              0x0055
    .hword              0x2C27              /* FSW     f6,6*4(a0) */
    .hword              0x0065
    .hword              0x2E27              /* FSW     f7,7*4(a0) */
    .hword              0x0075
    .hword              0x2027              /* FSW     f8,8*4(a0) */
    .hword              0x0285
    .hword              0x2227              /* FSW     f9,9*4(a0) */
    .hword              0x0295
    .hword              0x2427              /* FSW     f10,10*4(a0) */
    .hword              0x02A5
    .hword              0x2627              /* FSW     f11,11*4(a0) */
    .hword              0x02B5
    .hword              0x2827              /* FSW     f12,12*4(a0) */
    .hword              0x02C5
    .hword              0x2A27              /* FSW     f13,13*4(a0) */
    .hword              0x02D5
    .hword              0x2C27              /* FSW     f14,14*4(a0) */
    .hword              0x02E5
    .hword              0x2E27              /* FSW     f15,15*4(a0) */
    .hword              0x02F5
    .hword              0x2027              /* FSW     f16,16*4(a0) */
    .hword              0x0505
    .hword              0x2227              /* FSW     f17,17*4(a0) */
    .hword              0x0515
    .hword              0x2427              /* FSW     f18,18*4(a0) */
    .hword              0x0525
    .hword              0x2627              /* FSW     f19,19*4(a0) */
    .hword              0x0535
    .hword              0x2827              /* FSW     f20,20*4(a0) */
    .hword              0x0545
    .hword              0x2A27              /* FSW     f21,21*4(a0) */
    .hword              0x0555
    .hword              0x2C27              /* FSW     f22,22*4(a0) */
    .hword              0x0565
    .hword              0x2E27              /* FSW     f23,23*4(a0) */
    .hword              0x0575
    .hword              0x2027              /* FSW     f24,24*4(a0) */
    .hword              0x0785
    .hword              0x2227              /* FSW     f25,25*4(a0) */
    .hword              0x0795
    .hword              0x2427              /* FSW     f26,26*4(a0) */
    .hword              0x07A5
    .hword              0x2627              /* FSW     f27,27*4(a0) */
    .hword              0x07B5
    .hword              0x2827              /* FSW     f28,28*4(a0) */
    .hword              0x07C5
    .hword              0x2A27              /* FSW     f29,29*4(a0) */
    .hword              0x07D5
    .hword              0x2C27              /* FSW     f30,30*4(a0) */
    .hword              0x07E5
    .hword              0x2E27              /* FSW     f31,31*4(a0) */
    .hword              0x07F5
    RET

/* RVD */
    .section            .text.___rme_rv32p_thd_cop_save_rvd
    .align              3

___RME_RV32P_Thd_Cop_Save_RVD:
    .hword              0x22F3              /* FRCSR   t0 */
    .hword              0x0030
    SW                  t0,(a0)
    ADDI                a0,a0,4
    .hword              0x3027              /* FSD     f0,0*8(a0) */
    .hword              0x0005
    .hword              0x3427              /* FSD     f1,1*8(a0) */
    .hword              0x0015
    .hword              0x3827              /* FSD     f2,2*8(a0) */
    .hword              0x0025
    .hword              0x3C27              /* FSD     f3,3*8(a0) */
    .hword              0x0035
    .hword              0x3027              /* FSD     f4,4*8(a0) */
    .hword              0x0245
    .hword              0x3427              /* FSD     f5,5*8(a0) */
    .hword              0x0255
    .hword              0x3827              /* FSD     f6,6*8(a0) */
    .hword              0x0265
    .hword              0x3C27              /* FSD     f7,7*8(a0) */
    .hword              0x0275
    .hword              0x3027              /* FSD     f8,8*8(a0) */
    .hword              0x0485
    .hword              0x3427              /* FSD     f9,9*8(a0) */
    .hword              0x0495
    .hword              0x3827              /* FSD     f10,10*8(a0) */
    .hword              0x04A5
    .hword              0x3C27              /* FSD     f11,11*8(a0) */
    .hword              0x04B5
    .hword              0x3027              /* FSD     f12,12*8(a0) */
    .hword              0x06C5
    .hword              0x3427              /* FSD     f13,13*8(a0) */
    .hword              0x06D5
    .hword              0x3827              /* FSD     f14,14*8(a0) */
    .hword              0x06E5
    .hword              0x3C27              /* FSD     f15,15*8(a0) */
    .hword              0x06F5
    .hword              0x3027              /* FSD     f16,16*8(a0) */
    .hword              0x0905
    .hword              0x3427              /* FSD     f17,17*8(a0) */
    .hword              0x0915
    .hword              0x3827              /* FSD     f18,18*8(a0) */
    .hword              0x0925
    .hword              0x3C27              /* FSD     f19,19*8(a0) */
    .hword              0x0935
    .hword              0x3027              /* FSD     f20,20*8(a0) */
    .hword              0x0B45
    .hword              0x3427              /* FSD     f21,21*8(a0) */
    .hword              0x0B55
    .hword              0x3827              /* FSD     f22,22*8(a0) */
    .hword              0x0B65
    .hword              0x3C27              /* FSD     f23,23*8(a0) */
    .hword              0x0B75
    .hword              0x3027              /* FSD     f24,24*8(a0) */
    .hword              0x0D85
    .hword              0x3427              /* FSD     f25,25*8(a0) */
    .hword              0x0D95
    .hword              0x3827              /* FSD     f26,26*8(a0) */
    .hword              0x0DA5
    .hword              0x3C27              /* FSD     f27,27*8(a0) */
    .hword              0x0DB5
    .hword              0x3027              /* FSD     f28,28*8(a0) */
    .hword              0x0FC5
    .hword              0x3427              /* FSD     f29,29*8(a0) */
    .hword              0x0FD5
    .hword              0x3827              /* FSD     f30,30*8(a0) */
    .hword              0x0FE5
    .hword              0x3C27              /* FSD     f31,31*8(a0) */
    .hword              0x0FF5
    RET
/* End Function:___RME_RV32P_Thd_Cop_Save ************************************/

/* Function:___RME_RV32P_Thd_Cop_Load *****************************************
Description : Restore the coprocessor context on switch.
              Contains two versions for RVF and RVD.
Input       : A0 - The pointer to the coprocessor struct.
Output      : None.
Return      : None.
******************************************************************************/
/* RVF */
    .section            .text.___rme_rv32p_thd_cop_load_rvf
    .align              3

___RME_RV32P_Thd_Cop_Load_RVF:
    LW                  t0,(a0)
    .hword              0x9073              /* FSCSR   t0 */
    .hword              0x0032
    ADDI                a0,a0,4
    .hword              0x2007              /* FLW     f0,0*4(a0) */
    .hword              0x0005
    .hword              0x2087              /* FLW     f1,1*4(a0) */
    .hword              0x0045
    .hword              0x2107              /* FLW     f2,2*4(a0) */
    .hword              0x0085
    .hword              0x2187              /* FLW     f3,3*4(a0) */
    .hword              0x00C5
    .hword              0x2207              /* FLW     f4,4*4(a0) */
    .hword              0x0105
    .hword              0x2287              /* FLW     f5,5*4(a0) */
    .hword              0x0145
    .hword              0x2307              /* FLW     f6,6*4(a0) */
    .hword              0x0185
    .hword              0x2387              /* FLW     f7,7*4(a0) */
    .hword              0x01C5
    .hword              0x2407              /* FLW     f8,8*4(a0) */
    .hword              0x0205
    .hword              0x2487              /* FLW     f9,9*4(a0) */
    .hword              0x0245
    .hword              0x2507              /* FLW     f10,10*4(a0) */
    .hword              0x0285
    .hword              0x2587              /* FLW     f11,11*4(a0) */
    .hword              0x02C5
    .hword              0x2607              /* FLW     f12,12*4(a0) */
    .hword              0x0305
    .hword              0x2687              /* FLW     f13,13*4(a0) */
    .hword              0x0345
    .hword              0x2707              /* FLW     f14,14*4(a0) */
    .hword              0x0385
    .hword              0x2787              /* FLW     f15,15*4(a0) */
    .hword              0x03C5
    .hword              0x2807              /* FLW     f16,16*4(a0) */
    .hword              0x0405
    .hword              0x2887              /* FLW     f17,17*4(a0) */
    .hword              0x0445
    .hword              0x2907              /* FLW     f18,18*4(a0) */
    .hword              0x0485
    .hword              0x2987              /* FLW     f19,19*4(a0) */
    .hword              0x04C5
    .hword              0x2A07              /* FLW     f20,20*4(a0) */
    .hword              0x0505
    .hword              0x2A87              /* FLW     f21,21*4(a0) */
    .hword              0x0545
    .hword              0x2B07              /* FLW     f22,22*4(a0) */
    .hword              0x0585
    .hword              0x2B87              /* FLW     f23,23*4(a0) */
    .hword              0x05C5
    .hword              0x2C07              /* FLW     f24,24*4(a0) */
    .hword              0x0605
    .hword              0x2C87              /* FLW     f25,25*4(a0) */
    .hword              0x0645
    .hword              0x2D07              /* FLW     f26,26*4(a0) */
    .hword              0x0685
    .hword              0x2D87              /* FLW     f27,27*4(a0) */
    .hword              0x06C5
    .hword              0x2E07              /* FLW     f28,28*4(a0) */
    .hword              0x0705
    .hword              0x2E87              /* FLW     f29,29*4(a0) */
    .hword              0x0745
    .hword              0x2F07              /* FLW     f30,30*4(a0) */
    .hword              0x0785
    .hword              0x2F87              /* FLW     f31,31*4(a0) */
    .hword              0x07C5
    RET

/* RVD */
    .section            .text.___rme_rv32p_thd_cop_load_rvd
    .align              3

___RME_RV32P_Thd_Cop_Load_RVD:
    LW                  t0,(a0)
    .hword              0x9073              /* FSCSR   t0 */
    .hword              0x0032
    ADDI                a0,a0,4
    .hword              0x3007              /* FLD     f0,0*8(a0) */
    .hword              0x0005
    .hword              0x3087              /* FLD     f1,1*8(a0) */
    .hword              0x0085
    .hword              0x3107              /* FLD     f2,2*8(a0) */
    .hword              0x0105
    .hword              0x3187              /* FLD     f3,3*8(a0) */
    .hword              0x0185
    .hword              0x3207              /* FLD     f4,4*8(a0) */
    .hword              0x0205
    .hword              0x3287              /* FLD     f5,5*8(a0) */
    .hword              0x0285
    .hword              0x3307              /* FLD     f6,6*8(a0) */
    .hword              0x0305
    .hword              0x3387              /* FLD     f7,7*8(a0) */
    .hword              0x0385
    .hword              0x3407              /* FLD     f8,8*8(a0) */
    .hword              0x0405
    .hword              0x3487              /* FLD     f9,9*8(a0) */
    .hword              0x0485
    .hword              0x3507              /* FLD     f10,10*8(a0) */
    .hword              0x0505
    .hword              0x3587              /* FLD     f11,11*8(a0) */
    .hword              0x0585
    .hword              0x3607              /* FLD     f12,12*8(a0) */
    .hword              0x0605
    .hword              0x3687              /* FLD     f13,13*8(a0) */
    .hword              0x0685
    .hword              0x3707              /* FLD     f14,14*8(a0) */
    .hword              0x0705
    .hword              0x3787              /* FLD     f15,15*8(a0) */
    .hword              0x0785
    .hword              0x3807              /* FLD     f16,16*8(a0) */
    .hword              0x0805
    .hword              0x3887              /* FLD     f17,17*8(a0) */
    .hword              0x0885
    .hword              0x3907              /* FLD     f18,18*8(a0) */
    .hword              0x0905
    .hword              0x3987              /* FLD     f19,19*8(a0) */
    .hword              0x0985
    .hword              0x3A07              /* FLD     f20,20*8(a0) */
    .hword              0x0A05
    .hword              0x3A87              /* FLD     f21,21*8(a0) */
    .hword              0x0A85
    .hword              0x3B07              /* FLD     f22,22*8(a0) */
    .hword              0x0B05
    .hword              0x3B87              /* FLD     f23,23*8(a0) */
    .hword              0x0B85
    .hword              0x3C07              /* FLD     f24,24*8(a0) */
    .hword              0x0C05
    .hword              0x3C87              /* FLD     f25,25*8(a0) */
    .hword              0x0C85
    .hword              0x3D07              /* FLD     f26,26*8(a0) */
    .hword              0x0D05
    .hword              0x3D87              /* FLD     f27,27*8(a0) */
    .hword              0x0D85
    .hword              0x3E07              /* FLD     f28,28*8(a0) */
    .hword              0x0E05
    .hword              0x3E87              /* FLD     f29,29*8(a0) */
    .hword              0x0E85
    .hword              0x3F07              /* FLD     f30,30*8(a0) */
    .hword              0x0F05
    .hword              0x3F87              /* FLD     f31,31*8(a0) */
    .hword              0x0F85
    RET
/* End Function:___RME_RV32P_Thd_Cop_Save ************************************/

/* Function:___RME_RV32P_PMP_Set **********************************************
Description : Program the entire PMP array.
Input       : rme_ptr_t* CFG_Meta - The PMP metadata for PMPCFGs.
              rme_ptr_t* ADDR_Meta - The PMP metadata for PMPADDRs.
Output      : None.
Return      : None.
******************************************************************************/
    /* Configuration registers */
    .macro              PMPCFG_SET1
    LW                  t0, 0*4(a0)
    CSRW                pmpcfg0, t0
    .endm

    .macro              PMPCFG_SET2
    PMPCFG_SET1
    LW                  t0, 1*4(a0)
    CSRW                pmpcfg1, t0
    .endm

    .macro              PMPCFG_SET3
    PMPCFG_SET2
    LW                  t0, 2*4(a0)
    CSRW                pmpcfg2, t0
    .endm

    .macro              PMPCFG_SET4
    PMPCFG_SET3
    LW                  t0, 3*4(a0)
    CSRW                pmpcfg3, t0
    .endm

    /* Address registers */
    .macro              PMPADDR_SET1
    LW                  t0, 0*4(a1)
    CSRW                pmpaddr0, t0
    .endm

    .macro              PMPADDR_SET2
    PMPADDR_SET1
    LW                  t0, 1*4(a1)
    CSRW                pmpaddr1, t0
    .endm

    .macro              PMPADDR_SET3
    PMPADDR_SET2
    LW                  t0, 2*4(a1)
    CSRW                pmpaddr2, t0
    .endm

    .macro              PMPADDR_SET4
    PMPADDR_SET3
    LW                  t0, 3*4(a1)
    CSRW                pmpaddr3, t0
    .endm

    .macro              PMPADDR_SET5
    PMPADDR_SET4
    LW                  t0, 4*4(a1)
    CSRW                pmpaddr4, t0
    .endm

    .macro              PMPADDR_SET6
    PMPADDR_SET5
    LW                  t0, 5*4(a1)
    CSRW                pmpaddr5, t0
    .endm

    .macro              PMPADDR_SET7
    PMPADDR_SET6
    LW                  t0, 6*4(a1)
    CSRW                pmpaddr6, t0
    .endm

    .macro              PMPADDR_SET8
    PMPADDR_SET7
    LW                  t0, 7*4(a1)
    CSRW                pmpaddr7, t0
    .endm

    .macro              PMPADDR_SET9
    PMPADDR_SET8
    LW                  t0, 8*4(a1)
    CSRW                pmpaddr8, t0
    .endm

    .macro              PMPADDR_SET10
    PMPADDR_SET9
    LW                  t0, 9*4(a1)
    CSRW                pmpaddr9, t0
    .endm

    .macro              PMPADDR_SET11
    PMPADDR_SET10
    LW                  t0, 10*4(a1)
    CSRW                pmpaddr10, t0
    .endm

    .macro              PMPADDR_SET12
    PMPADDR_SET11
    LW                  t0, 11*4(a1)
    CSRW                pmpaddr11, t0
    .endm

    .macro              PMPADDR_SET13
    PMPADDR_SET12
    LW                  t0, 12*4(a1)
    CSRW                pmpaddr12, t0
    .endm

    .macro              PMPADDR_SET14
    PMPADDR_SET13
    LW                  t0, 13*4(a1)
    CSRW                pmpaddr13, t0
    .endm

    .macro              PMPADDR_SET15
    PMPADDR_SET14
    LW                  t0, 14*4(a1)
    CSRW                pmpaddr14, t0
    .endm

    .macro              PMPADDR_SET16
    PMPADDR_SET15
    LW                  t0, 15*4(a1)
    CSRW                pmpaddr15, t0
    .endm

/* 1-range version */
    .section            .text.___rme_rv32p_pmp_set1
    .align              3

___RME_RV32P_PMP_Set1:
    PMPCFG_SET1
    PMPADDR_SET1
    RET

/* 2-range version */
    .section            .text.___rme_rv32p_pmp_set2
    .align              3

___RME_RV32P_PMP_Set2:
    PMPCFG_SET1
    PMPADDR_SET2
    RET

/* 3-range version */
    .section            .text.___rme_rv32p_pmp_set3
    .align              3

___RME_RV32P_PMP_Set3:
    PMPCFG_SET1
    PMPADDR_SET3
    RET

/* 4-range version */
    .section            .text.___rme_rv32p_pmp_set4
    .align              3

___RME_RV32P_PMP_Set4:
    PMPCFG_SET1
    PMPADDR_SET4
    RET

/* 5-range version */
    .section            .text.___rme_rv32p_pmp_set5
    .align              3

___RME_RV32P_PMP_Set5:
    PMPCFG_SET2
    PMPADDR_SET5
    RET

/* 6-range version */
    .section            .text.___rme_rv32p_pmp_set6
    .align              3

___RME_RV32P_PMP_Set6:
    PMPCFG_SET2
    PMPADDR_SET6
    RET

/* 7-range version */
    .section            .text.___rme_rv32p_pmp_set7
    .align              3

___RME_RV32P_PMP_Set7:
    PMPCFG_SET2
    PMPADDR_SET7
    RET

/* 8-range version */
    .section            .text.___rme_rv32p_pmp_set8
    .align              3

___RME_RV32P_PMP_Set8:
    PMPCFG_SET2
    PMPADDR_SET8
    RET

/* 9-range version */
    .section            .text.___rme_rv32p_pmp_set9
    .align              3

___RME_RV32P_PMP_Set9:
    PMPCFG_SET3
    PMPADDR_SET9
    RET

/* 10-range version */
    .section            .text.___rme_rv32p_pmp_set10
    .align              3

___RME_RV32P_PMP_Set10:
    PMPCFG_SET3
    PMPADDR_SET10
    RET

/* 11-range version */
    .section            .text.___rme_rv32p_pmp_set11
    .align              3

___RME_RV32P_PMP_Set11:
    PMPCFG_SET3
    PMPADDR_SET11
    RET

/* 12-range version */
    .section            .text.___rme_rv32p_pmp_set12
    .align              3

___RME_RV32P_PMP_Set12:
    PMPCFG_SET3
    PMPADDR_SET12
    RET

/* 13-range version */
    .section            .text.___rme_rv32p_pmp_set13
    .align              3

___RME_RV32P_PMP_Set13:
    PMPCFG_SET4
    PMPADDR_SET13
    RET

/* 14-range version */
    .section            .text.___rme_rv32p_pmp_set14
    .align              3

___RME_RV32P_PMP_Set14:
    PMPCFG_SET4
    PMPADDR_SET14
    RET

/* 15-range version */
    .section            .text.___rme_rv32p_pmp_set15
    .align              3

___RME_RV32P_PMP_Set15:
    PMPCFG_SET4
    PMPADDR_SET15
    RET

/* 16-range version */
    .section            .text.___rme_rv32p_pmp_set16
    .align              3

___RME_RV32P_PMP_Set16:
    PMPCFG_SET4
    PMPADDR_SET16
    RET
/* End Function:___RME_RV32P_PMP_Set *****************************************/
    .end
/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/

