/******************************************************************************
Filename    : rme_platform_rv64v_gcc.s
Author      : yhy
Date        : 29/09/2026
Description : The GCC assembly stubs for the RV64V platform.
******************************************************************************/

/* The RISC-V RV64V Architecture **********************************************
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
    .extern             __RME_Zero_Start
    .extern             __RME_Zero_End
    /* Nonstandard low-level preinit that some platforms require */
    .extern             __RME_RV64V_Lowlvl_Preinit
    /* Kernel entry */
    .extern             main
/* End Import ****************************************************************/

/* Export ********************************************************************/
    /* The kernel entry point - jumped to by OpenSBI at 0x80200000 */
    .global             _start
    /* Disable all interrupts */
    .global             __RME_Int_Disable
    /* Enable all interrupts */
    .global             __RME_Int_Enable
    /* Entering of the user mode */
    .global             __RME_User_Enter
    /* Wait for interrupts */
    .global             __RME_RV64V_Wait_Int
    /* A full barrier */
    .global             __RME_RV64V_Barrier
    /* CSR manipulations */
    .global             ___RME_RV64V_SCAUSE_Get
    .global             ___RME_RV64V_STVAL_Set
    .global             ___RME_RV64V_STVAL_Get
    .global             ___RME_RV64V_CYCLE_Get
    .global             ___RME_RV64V_MISA_Get
    .global             ___RME_RV64V_SSTATUS_Get
    .global             ___RME_RV64V_SSTATUS_Set
    /* MMU (satp) manipulations */
    .global             ___RME_RV64V_SATP_Set
    .global             ___RME_RV64V_TLB_Flush
    /* Handler for everything */
    .global             __RME_RV64V_Handler
    /* Coprocessor save/load */
    .global             ___RME_RV64V_Thd_Cop_Clear_RVF
    .global             ___RME_RV64V_Thd_Cop_Clear_RVD
    .global             ___RME_RV64V_Thd_Cop_Save_RVF
    .global             ___RME_RV64V_Thd_Cop_Save_RVD
    .global             ___RME_RV64V_Thd_Cop_Load_RVF
    .global             ___RME_RV64V_Thd_Cop_Load_RVD
    /* OpenSBI (SBI) service helpers */
    .global             ___RME_RV64V_Sbi_Call
    .global             ___RME_RV64V_Time_Get
    .global             ___RME_RV64V_Sie_Timer_Enable
/* End Export ****************************************************************/

/* Entry *********************************************************************/
    .section            .text._start
    .align              3

_start:
    /* OpenSBI enters at the physical load address with the MMU off. Build a
     * bootstrap root table with two 1GB leaves:
     *   top[2]   - identity map of physical [0x80000000,0xC0000000), needed
     *              for the few instructions right after satp is enabled;
     *   top[258] - the same range at the high-half kernel VA.
     * PC-relative LA yields the physical address here and the high VA later,
     * so the same code works before and after the switch. */
    LA                  t0,__RME_Boot_Root
    LI                  t1,512
    MV                  t2,t0
__RME_Boot_Root_Clear:
    SD                  zero,(t2)
    ADDI                t2,t2,8
    ADDI                t1,t1,-1
    BNEZ                t1,__RME_Boot_Root_Clear

    /* PTE = PPN(0x80000000)<<10 | V|R|W|X (U must stay 0 for the kernel) */
    LI                  t3,0x2000000F
    SD                  t3,16(t0)               /* top[2], identity */
    LI                  t2,2048
    ADD                 t2,t0,t2
    SD                  t3,16(t2)               /* top[258], high half */

    /* satp = MODE(Sv39=8)<<60 | PPN(root) */
    SRLI                t4,t0,12
    LI                  t5,8
    SLLI                t5,t5,60
    OR                  t4,t4,t5
    CSRW                satp,t4
    SFENCE.VMA          x0,x0

    /* Jump into the high half (LA gives the physical address here) */
    LA                  t0,__RME_RV64V_Boot_High
    LI                  t1,0xFFFFFFC000000000
    ADD                 t0,t0,t1
    JR                  t0

__RME_RV64V_Boot_High:
    /* Now executing at the high-half VA */
    .option             push
    .option             norelax
    LA                  gp,__RME_Global
    .option             pop
    LA                  sp,__RME_Stack
    CSRW                sscratch,sp
    LA                  t0,__RME_RV64V_Handler
    CSRW                stvec,t0

    /* Clear the bss section. The data section needs no copy: the high-half
     * mapping points straight at the physical load address. */
    LA                  a0,__RME_Zero_Start
    LA                  a1,__RME_Zero_End
__RME_Zero_Clear:
    BEQ                 a0,a1,__RME_Zero_Done
    SD                  zero,(a0)
    ADDI                a0,a0,8
    J                   __RME_Zero_Clear
__RME_Zero_Done:

    /* Preinitialize hardware before any initialization */
    CALL                __RME_RV64V_Lowlvl_Preinit
    /* Branch to main function */
    J                   main

/* Bootstrap root page table - the loader does not zero it, _start programs it
 * explicitly, and it must survive the .bss clear below. */
    .section            .bootpgt, "aw", @nobits
    .align              12

__RME_Boot_Root:
    .space              4096
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
    CSRCI               sstatus, 2
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
    CSRSI               sstatus, 2
    RET
/* End Function:__RME_Int_Enable *********************************************/

/* Function:__RME_RV64V_Barrier ***********************************************
Description : A full data/instruction barrier.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
    .section            .text.__rme_rv64v_barrier
    .align              3

___RME_RV64V_Barrier:
    FENCE
    FENCE.I
    RET
/* End Function:__RME_RV64V_Barrier ******************************************/

/* Function:__RME_RV64V_Wait_Int **********************************************
Description : Wait until a new interrupt comes, to save power.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
    .section            .text.__rme_rv64v_wait_int
    .align              3

__RME_RV64V_Wait_Int:
    WFI
    RET
/* End Function:__RME_RV64V_Wait_Int *****************************************/

/* Function:___RME_RV64V_SCAUSE_Get *******************************************
Description : Get the SCAUSE register content.
Input       : None.
Output      : None.
Return      : a0 - SCAUSE value.
******************************************************************************/
    .section            .text.___rme_rv64v_mcause_get
    .align              3

___RME_RV64V_SCAUSE_Get:
    CSRR                a0, scause
    RET
/* End Function:___RME_RV64V_SCAUSE_Get **************************************/

/* Function:___RME_RV64V_STVAL_Get ********************************************
Description : Get the STVAL register content.
Input       : None.
Output      : None.
Return      : a0 - SCAUSE value.
******************************************************************************/
    .section            .text.___rme_rv64v_mtval_get
    .align              3

___RME_RV64V_STVAL_Get:
    CSRR                a0, stval
    RET
/* End Function:___RME_RV64V_STVAL_Get ***************************************/

/* Function:___RME_RV64V_CYCLE_Get *******************************************
Description : Get the CYCLE register content.
Input       : None.
Output      : None.
Return      : a0 - CYCLE value.
******************************************************************************/
    .section            .text.___rme_rv64v_mcycle_get
    .align              3

___RME_RV64V_CYCLE_Get:
    CSRR                a0, cycle
    RET
/* End Function:___RME_RV64V_CYCLE_Get **************************************/

/* Function:___RME_RV64V_MISA_Get *********************************************
Description : Get the MISA register content.
Input       : None.
Output      : None.
Return      : a0 - MISA value.
******************************************************************************/
    .section            .text.___rme_rv64v_misa_get
    .align              3

___RME_RV64V_MISA_Get:
    CSRR                a0, misa
    RET
/* End Function:___RME_RV64V_MISA_Get ****************************************/

/* Function:___RME_RV64V_SSTATUS_Get ******************************************
Description : Get the SSTATUS register content.
Input       : None.
Output      : None.
Return      : $a0 - SSTATUS value.
******************************************************************************/
    .section            .text.___rme_rv64v_mstatus_get
    .align              3

___RME_RV64V_SSTATUS_Get:
    CSRR                a0, sstatus
    RET
/* End Function:___RME_RV64V_SSTATUS_Get *************************************/

/* Function:___RME_RV64V_SSTATUS_Set ******************************************
Description : Set the SSTATUS register content.
Input       : $a0 - SSTATUS value.
Output      : None.
Return      : None.
******************************************************************************/
    .section            .text.___rme_rv64v_mstatus_set
    .align              3

___RME_RV64V_SSTATUS_Set:
    CSRW                sstatus, a0
    RET
/* End Function:___RME_RV64V_SSTATUS_Set *************************************/

/* Function:___RME_RV64V_SATP_Set *********************************************
Description : Write the satp register to enable/switch the Sv39 MMU.
              satp layout: [63:60] MODE = 8 (Sv39), [59:44] ASID,
              [43:0] PPN = root page table physical address >> 12.
Input       : a0 - The satp value.
Output      : None.
Return      : None.
******************************************************************************/
    .section            .text.___rme_rv64v_satp_set
    .align              3

___RME_RV64V_SATP_Set:
    CSRW                satp,a0
    RET
/* End Function:___RME_RV64V_SATP_Set ****************************************/

/* Function:___RME_RV64V_TLB_Flush ********************************************
Description : Flush all address-translation cache (TLB) entries.
              SFENCE.VMA with rs1=rs2=x0 flushes the whole TLB; on a
              single-core system this is all we ever need.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
    .section            .text.___rme_rv64v_tlb_flush
    .align              3

___RME_RV64V_TLB_Flush:
    SFENCE.VMA          x0,x0
    RET
/* End Function:___RME_RV64V_TLB_Flush ****************************************/

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
    /* FS clear, previous mode = U, previous M-mode interrupt enabled */
    LI                  t0,0x00000020
    CSRW                sstatus,t0
    CSRW                sepc,a0
    MV                  sp,a1
    MV                  a0,a2
    SRET
/* End Function:__RME_User_Enter *********************************************/

/* Function:__RME_RV64V_Handler ***********************************************
Description : The generic handler routine. Assuming no vector mode is available
              regardless. The C program instead of this is responsible for
              software PC adjustments before and after an exception.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
    .section            .text.__rme_rv64v_handler
    .align              3

__RME_RV64V_Handler:
    CSRRW               sp,sscratch,sp
    /* Allocate stack space for context saving */
    ADDI                sp,sp,-33*8
    /* Save all general-purpose registers - RV32G does not have any flag registers */
    SD                  x31,32*8(sp)
    SD                  x30,31*8(sp)
    SD                  x29,30*8(sp)
    SD                  x28,29*8(sp)
    SD                  x27,28*8(sp)
    SD                  x26,27*8(sp)
    SD                  x25,26*8(sp)
    SD                  x24,25*8(sp)
    SD                  x23,24*8(sp)
    SD                  x22,23*8(sp)
    SD                  x21,22*8(sp)
    SD                  x20,21*8(sp)
    SD                  x19,20*8(sp)
    SD                  x18,19*8(sp)
    SD                  x17,18*8(sp)
    SD                  x16,17*8(sp)
    SD                  x15,16*8(sp)
    SD                  x14,15*8(sp)
    SD                  x13,14*8(sp)
    SD                  x12,13*8(sp)
    SD                  x11,12*8(sp)
    SD                  x10,11*8(sp)
    SD                  x9,10*8(sp)
    SD                  x8,9*8(sp)
    SD                  x7,8*8(sp)
    SD                  x6,7*8(sp)
    SD                  x5,6*8(sp)
    SD                  x4,5*8(sp)
    SD                  x3,4*8(sp)
    /* Save sp (x2) */
    CSRR                a0,sscratch
    SD                  a0,3*8(sp)
    /* Save x1 */
    SD                  x1,2*8(sp)
    /* Save pc (sepc) */
    CSRR                a0,sepc
    SD                  a0,1*8(sp)
    /* Save sstatus */
    CSRR                a0,sstatus
    SD                  a0,0*8(sp)

    /* Load gp for kernel - defined by linker script */
    .option push
    .option norelax
    LA                  gp,__RME_Global
    .option pop
    /* Call system main interrupt handler with regs */
    MV                  a0,sp
    CALL                _RME_RV64V_Handler

    /* Load sstatus */
    LD                  a0,0*8(sp)
    CSRW                sstatus,a0
    /* Load pc (into sepc) */
    LD                  a0,1*8(sp)
1:
    BEQZ                a0,1b
    CSRW                sepc,a0
    /* Load x1 */
    LD                  x1,2*8(sp)
    /* Load sp (x2, into sscratch) */
    LD                  a0,3*8(sp)
    CSRW                sscratch,a0
    /* Load all general-purpose registers - RV32G does not have any flag registers */
    LD                  x3,4*8(sp)
    LD                  x4,5*8(sp)
    LD                  x5,6*8(sp)
    LD                  x6,7*8(sp)
    LD                  x7,8*8(sp)
    LD                  x8,9*8(sp)
    LD                  x9,10*8(sp)
    LD                  x10,11*8(sp)
    LD                  x11,12*8(sp)
    LD                  x12,13*8(sp)
    LD                  x13,14*8(sp)
    LD                  x14,15*8(sp)
    LD                  x15,16*8(sp)
    LD                  x16,17*8(sp)
    LD                  x17,18*8(sp)
    LD                  x18,19*8(sp)
    LD                  x19,20*8(sp)
    LD                  x20,21*8(sp)
    LD                  x21,22*8(sp)
    LD                  x22,23*8(sp)
    LD                  x23,24*8(sp)
    LD                  x24,25*8(sp)
    LD                  x25,26*8(sp)
    LD                  x26,27*8(sp)
    LD                  x27,28*8(sp)
    LD                  x28,29*8(sp)
    LD                  x29,30*8(sp)
    LD                  x30,31*8(sp)
    LD                  x31,32*8(sp)
    ADDI                sp,sp,33*8
    /* Return to user mode */
    CSRRW               sp,sscratch,sp
    SRET
/* End Function:_RME_RV64V_Handler *******************************************/

/* Function:___RME_RV64V_Thd_Cop_Clear ****************************************
Description : Clean up the coprocessor state so that the FP information is not
              leaked when switching from a FPU-enabled thread to a FPU-disabled
              thread. Contains two versions for RVF and RVD.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
/* RVF ***********************************************************************/
    .section            .text.___rme_rv64v_thd_cop_clear_rvf
    .align              3

___RME_RV64V_Thd_Cop_Clear_RVF:
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
    .section            .text.___rme_rv64v_thd_cop_clear_rvd
    .align              3

___RME_RV64V_Thd_Cop_Clear_RVD:
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
/* End Function:___RME_RV64V_Thd_Cop_Clear ***********************************/

/* Function:___RME_RV64V_Thd_Cop_Save *****************************************
Description : Save the coprocessor context on switch.
              Contains two versions for RVF and RVD.
Input       : A0 - The pointer to the coprocessor struct.
Output      : None.
Return      : None.
******************************************************************************/
/* RVF */
    .section            .text.___rme_rv64v_thd_cop_save_rvf
    .align              3

___RME_RV64V_Thd_Cop_Save_RVF:
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
    .section            .text.___rme_rv64v_thd_cop_save_rvd
    .align              3

___RME_RV64V_Thd_Cop_Save_RVD:
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
/* End Function:___RME_RV64V_Thd_Cop_Save ************************************/

/* Function:___RME_RV64V_Thd_Cop_Load *****************************************
Description : Restore the coprocessor context on switch.
              Contains two versions for RVF and RVD.
Input       : A0 - The pointer to the coprocessor struct.
Output      : None.
Return      : None.
******************************************************************************/
/* RVF */
    .section            .text.___rme_rv64v_thd_cop_load_rvf
    .align              3

___RME_RV64V_Thd_Cop_Load_RVF:
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
    .section            .text.___rme_rv64v_thd_cop_load_rvd
    .align              3

___RME_RV64V_Thd_Cop_Load_RVD:
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
/* End Function:___RME_RV64V_Thd_Cop_Load ************************************/

/* Function:___RME_RV64V_Sbi_Call *********************************************
Description : Generic OpenSBI call. SBI convention: a7 = extension/function ID,
              a6 = function ID (v0.2, zeroed here), a0-a1 = arguments.
              C signature: rme_ptr_t ___RME_RV64V_Sbi_Call(A7, A0, A1)
              On entry (C ABI): a0=A7, a1=A0, a2=A1.
Input       : a0 - A7 (SBI ID).
              a1 - A0 (SBI argument 0).
              a2 - A1 (SBI argument 1).
Output      : None.
Return      : a0 - SBI return value (ignored by callers).
******************************************************************************/
    .section            .text.___rme_rv64v_sbi_call
    .align              3

___RME_RV64V_Sbi_Call:
    MV                  a7,a0        /* a7 = extension/function ID */
    LI                  a6,0         /* v0.2 function ID (SRST needs 0; legacy ignores) */
    MV                  a0,a1        /* a0 = argument 0 */
    MV                  a1,a2        /* a1 = argument 1 */
    ECALL
    RET
/* End Function:___RME_RV64V_Sbi_Call ****************************************/

/* Function:___RME_RV64V_Time_Get *********************************************
Description : Read the time CSR (0xC01). The time base is 10MHz on QEMU virt.
Input       : None.
Output      : None.
Return      : a0 - The current time.
******************************************************************************/
    .section            .text.___rme_rv64v_time_get
    .align              3

___RME_RV64V_Time_Get:
    CSRR                a0,time
    RET
/* End Function:___RME_RV64V_Time_Get ****************************************/

/* Function:___RME_RV64V_Sie_Timer_Enable *************************************
Description : Enable the S-mode timer interrupt (sie.STIE, bit 5).
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
    .section            .text.___rme_rv64v_sie_timer_enable
    .align              3

___RME_RV64V_Sie_Timer_Enable:
    LI                  t0,32
    CSRRS               x0,sie,t0    /* sie |= STIE */
    RET
/* End Function:___RME_RV64V_Sie_Timer_Enable ********************************/
    .end
/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/

