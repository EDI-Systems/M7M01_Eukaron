/******************************************************************************
Filename    : rme_platform_rv32gp_gcc.s
Author      : pry
Date        : 19/01/2017
Description : The GCC assembly stubs for RV32G PMP-based microcontrollers.
              This only uses two modes: The U mode, and the M mode.
******************************************************************************/

/* The RISC-V RV32GP Architecture *********************************************
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

/* Begin Header **************************************************************/
    .section            .start,"ax",@progbits
    .align              1
/* End Header ****************************************************************/

/* Begin Exports *************************************************************/
    /* Entry of the operating system */
    .global             _start
    /* Disable all interrupts */
    .global             __RME_Int_Disable
    /* Enable all interrupts */
    .global             __RME_Int_Enable
    /* Kernel main function wrapper */
    .global             _RME_Kmain
    /* Entering of the user mode */
    .global             __RME_User_Enter
    /* The compare-and-swap atomic instruction */
    .global             __RME_RV32GP_Comp_Swap
    /* The fetch-and-add atomic instruction. */
    .global             __RME_RV32GP_Fetch_Add
    /* The fetch-and-logic-and atomic instruction. */
    .global             __RME_RV32GP_Fetch_And
    /* A full barrier */
    .global             __RME_RV32GP_Barrier
    /* Get the MSB in a word */
    .global             __RME_RV32GP_MSB_Get
    /* Get the MSB in a word, with CTZ bit manipulation */
    .global             __RME_RV32GP_MSB_Get_CTZ
    /* CSR manipulations */
    .global             __RME_RV32GP_MCAUSE_Get
    .global             __RME_RV32GP_MTVEC_Set
    .global             __RME_RV32GP_MCYCLE_Get
    .global             __RME_RV32GP_MISA_Get
    .global             __RME_RV32GP_MSTATUS_Get
    .global             __RME_RV32GP_MSTATUS_Set
    /* Handler for everything */
    .global             __RME_RV32GP_Handler
    /* Coprocessor save/load */
    .global             ___RME_RV32GP_Cop_Clear_RVF
    .global             ___RME_RV32GP_Cop_Clear_RVD
    .global             ___RME_RV32GP_Cop_Save_RVF
    .global             ___RME_RV32GP_Cop_Save_RVD
    .global             ___RME_RV32GP_Cop_Load_RVF
    .global             ___RME_RV32GP_Cop_Load_RVD
    /* The MPU setup routine */
    .global             ___RME_RV32GP_PMP_Set1
    .global             ___RME_RV32GP_PMP_Set2
    .global             ___RME_RV32GP_PMP_Set3
    .global             ___RME_RV32GP_PMP_Set4
    .global             ___RME_RV32GP_PMP_Set5
    .global             ___RME_RV32GP_PMP_Set6
    .global             ___RME_RV32GP_PMP_Set7
    .global             ___RME_RV32GP_PMP_Set8
    .global             ___RME_RV32GP_PMP_Set9
    .global             ___RME_RV32GP_PMP_Set10
    .global             ___RME_RV32GP_PMP_Set11
    .global             ___RME_RV32GP_PMP_Set12
    .global             ___RME_RV32GP_PMP_Set13
    .global             ___RME_RV32GP_PMP_Set14
    .global             ___RME_RV32GP_PMP_Set15
    .global             ___RME_RV32GP_PMP_Set16
/* End Exports ***************************************************************/

/* Begin Imports *************************************************************/
    .extern              __RME_RV32GP_Low_Level_Preinit
    /* The kernel entry of RME. This will be defined in C language. */
    .extern             RME_Kmain
    /* The classical C main function */
    .extern             main
/* End Imports ***************************************************************/

/* Reset Vector **************************************************************/
RME_Start:
    /* Set correct gp and sp */
    .option             push
    .option             norelax
    LA                  gp, __global_pointer$
    .option pop
    LA                  sp, __initial_stack$

    /* Low-level initialization */
    CALL                __RME_RV32GP_Low_Level_Preinit

    /* Load data section from flash to RAM - GCC-based Clib does not do this */
    LA                  a0, __data_start_flash$
    LA                  a1, __data_start_ram$
    LA                  a2, __data_end_ram$
_RME_Start_Copy:
    LW                  t0, (a0)
    SW                  t0, (a1)
    ADDI                a0, a0, 4
    ADDI                a1, a1, 4
    BLTU                a1, a2, _RME_Start_Copy

    /* Clear bss section in ram */
    LA                  a0, __bss_start_ram$
    LA                  a1, __bss_end_ram$
_RME_Start_Clear:
    SW                  zero, (a0)
    ADDI                a0, a0, 4
    BLTU                a0, a1, _RME_Start_Clear

    /* Enable nothing yet */
    LI                  t0, 0x1000
    CSRS                mstatus, t0
    /* Fill interrupt handler */
    LA                  t0, _RME_RV32GP_Handler
    ORI                 t0, t0, 3
    CSRW                mtvec, t0

    /* Jump to main function */
    J                   main
/* End Reset Vector **********************************************************/

/* Begin Function:__RME_Int_Disable *******************************************
Description : The function for disabling all interrupts.
Input       : None.
Output      : None. 
Return      : None.                                
******************************************************************************/    
__RME_Int_Disable:
    /* Disable all interrupts */
    CSRCI               mstatus, 8
    RET
/* End Function:__RME_Int_Disable ********************************************/

/* Begin Function:__RME_Int_Enable ********************************************
Description : The function for enabling all interrupts.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
__RME_Int_Enable:
    /* Enable all interrupts. */
    CSRSI               mstatus, 8
    RET
/* End Function:__RME_Int_Enable *********************************************/

/* Begin Function:__RME_RV32GP_Barrier ****************************************
Description : A full data/instruction barrier.
Input       : None.
Output      : None.
Return      : None.
*****************************************************************************/
__RME_RV32GP_Barrier:
      FENCE
      FENCE.I
      RET
/* End Function:__RME_RV32GP_Barrier ****************************************/

/* Begin Function:__RME_RV32GP_Comp_Swap *************************************
Description : The compare-and-swap atomic instruction. If the Old value is
              equal to *Ptr, then set the *Ptr as New and return 1; else return
              0.
Input       : volatile rme_ptr_t* Ptr - The pointer to the data.
              rme_ptr_t Old - The old value.
              rme_ptr_t New - The new value.
Output      : volatile rme_ptr_t* Ptr - The pointer to the data.
Return      : rme_ptr_t - If successful, 1; else 0.
******************************************************************************/
__RME_RV32GP_Comp_Swap:
    LR.W.AQ            t0, (a0)
    BNE                t0, a1, __RME_RV32GP_Comp_Swap_Fail
    SC.W.RL            t0, a2, (a0)
    LI                 a0, 1
    JR                 ra
__RME_RV32GP_Comp_Swap_Fail:
    LI                 a0, 0
    JR                 ra
/* End Function:__RME_RV32GP_Comp_Swap ***************************************/

/* Begin Function:__RME_RV32GP_Fetch_Add **************************************
Description : The fetch-and-add atomic instruction. Increase the value that is
              pointed to by the pointer, and return the value before addition.
Input       : volatile rme_ptr_t* Ptr - The pointer to the data.
              rme_cnt_t Addend - The number to add.
Output      : volatile rme_ptr_t* Ptr - The pointer to the data.
Return      : rme_ptr_t - The value before the addition.
******************************************************************************/
__RME_RV32GP_Fetch_Add:
    AMOADD.W.AQRL      a0, a1, (a0)
    JR                 ra
/* End Function:__RME_RV32GP_Fetch_Add ***************************************/

/* Begin Function:__RME_RV32GP_Fetch_And **************************************
Description : The fetch-and-logic-and atomic instruction. Logic AND the pointer
              value with the operand, and return the value before logic AND.
Input       : volatile rme_ptr_t* Ptr - The pointer to the data.
              rme_cnt_t Operand - The number to logic AND with the destination.
Output      : volatile rme_ptr_t* Ptr - The pointer to the data.
Return      : rme_ptr_t - The value before the AND operation.
******************************************************************************/
__RME_RV32GP_Fetch_And:
    AMOAND.W.AQRL      a0, a1,(a0)
    JR                 ra
/* End Function:__RME_RV32GP_Fetch_And ***************************************/

/* Begin Function:_RME_Kmain **************************************************
Description : The entry address of the kernel. Never returns.
Input       : rme_ptr_t Stack - The stack address to set SP to.
Output      : None.
Return      : None.
******************************************************************************/
_RME_Kmain:
    ADD                 sp, a0, x0
    JAL                 RME_Kmain
/* End Function:_RME_Kmain ***************************************************/

/* Begin Function:__RME_RV32GP_MSB_Get ****************************************
Description : Get the MSB of the word. RV32GP does not necessarily support bit
              manipulation so it is a must to handle things in this way.
Input       : rme_ptr_t Val - The value.
Output      : None.
Return      : rme_ptr_t - The MSB position.
******************************************************************************/
.macro CHECK_BITS BITS LABEL
    SRL                 a2, a1, \BITS
    BEQ                 a2, x0, \LABEL
    ADDI                a0, a0, \BITS
    ADD                 a1, a2, x0
\LABEL:
.endm

    /* Always 21 instructions no matter what */
__RME_RV32G_MSB_Get:
    BEQ                 a0, x0, ZERO
    ADD                 a1, a0, x0
    LI                  a0, 0
    CHECK_BITS          BITS=16 LABEL=HEX
    CHECK_BITS          BITS=8  LABEL=OCT
    CHECK_BITS          BITS=4  LABEL=QUAD
    CHECK_BITS          BITS=2  LABEL=BIN
    CHECK_BITS          BITS=1  LABEL=ONE
    RET
ZERO:
    ADDI                a0, a0, -1
    RET
/* End Function:__RME_RV32GP_MSB_Get *****************************************/

/* Begin Function:__RME_RV32GP_MSB_Get_CTZ ************************************
Description : Get the MSB of the word. Use this if the processor happen to have
              CTZ. Written in raw machine code to circumvent certain assembler
              checks.
Input       : rme_ptr_t Val - The value.
Output      : None.
Return      : rme_ptr_t - The MSB position.
******************************************************************************/
__RME_RV32GP_MSB_Get_CTZ:
    /* CTZ                 a0, a0 */
    RET
/* End Function:__RME_RV32GP_MSB_Get_CTZ *************************************/

/* Begin Function:_RME_RV32GP_MCAUSE_Get **************************************
Description : Get the MCAUSE register content.
Input       : None.
Output      : None.
Return      : $a0 - MCAUSE value.
******************************************************************************/
__RME_RV32GP_MCAUSE_Get:
    CSRR                a0, mcause
    RET
/* End Function:_RME_RV32GP_MCAUSE_Get ***************************************/

/* Begin Function:_RME_RV32GP_MTVEC_Set ***************************************
Description : Set the MTVEC register content.
Input       : $a0 - MTVEC value.
Output      : None.
Return      : None.
******************************************************************************/
__RME_RV32GP_MTVEC_Set:
    CSRW                mtvec, a0
    RET
/* End Function:_RME_RV32GP_MTVEC_Set ****************************************/

/* Begin Function:_RME_RV32GP_MCYCLE_Get **************************************
Description : Get the MCYCLE register content.
Input       : None.
Output      : None.
Return      : $a0 - MCYCLE value.
******************************************************************************/
__RME_RV32GP_MCYCLE_Get:
    CSRR                a0, mcycle
    RET
/* End Function:_RME_RV32GP_MCYCLE_Get ***************************************/

/* Begin Function:_RME_RV32GP_MISA_Get ****************************************
Description : Get the MISA register content.
Input       : None.
Output      : None.
Return      : $a0 - MISA value.
******************************************************************************/
__RME_RV32GP_MISA_Get:
    CSRR                a0, misa
    RET
/* End Function:_RME_RV32GP_MISA_Get *****************************************/

/* Begin Function:_RME_RV32GP_MSTATUS_Get *************************************
Description : Get the MSTATUS register content.
Input       : None.
Output      : None.
Return      : $a0 - MSTATUS value.
******************************************************************************/
_RME_RV32GP_MSTATUS_Get:
    CSRR                a0, mstatus
    RET
/* End Function:_RME_RV32GP_MSTATUS_Get **************************************/

/* Begin Function:_RME_RV32GP_MSTATUS_Set *************************************
Description : Set the MSTATUS register content.
Input       : $a0 - MSTATUS value.
Output      : None.
Return      : None.
******************************************************************************/
_RME_RV32GP_MSTATUS_Set:
    CSRW                mstatus, a0
    RET
/* End Function:_RME_RV32GP_MSTATUS_Set **************************************/

/* Begin Function:__RME_User_Enter ********************************************
Description : Entering of the user mode, after the system finish its preliminary
              booting. The function shall never return. This function should only
              be used to boot the first process in the system.

Input       : rme_ptr_t Entry - The user execution startpoint.
              rme_ptr_t Stack - The user stack.
              rme_ptr_t CPUID - The CPUID.
Output      : None.
Return      : None.
******************************************************************************/
__RME_User_Enter:
    /* MSTATUS_MPP = 0x00001800: FS clear, previous mode = U, previous interrupt enabled */
    LI                  t0, 0x00001800
    CSRW                mstatus, t0
    CSRW                mepc, a0
    MV                  sp, a1
    MV                  a0, a2
    /* Load sp for kernel - defined by linker script */
    LA                  t0, __initial_stack$
    CSRW                mscratch, t0
    MRET
/* End Function:__RME_User_Enter *********************************************/

/* Begin Function:__RME_RV32GP_Handler ****************************************
Description : The SVC handler routine. This will in fact call a C function to resolve
              the system service routines.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
__RME_RV32GP_Handler:
    CSRRW               sp, mscratch, sp
    /* Allocate stack space for context saving */
    ADDI                sp, sp, -33*4
    /* Save all general-purpose registers - RV32G does not have any flag registers */
    SW                  x31, 32*4(sp)
    SW                  x30, 31*4(sp)
    SW                  x29, 30*4(sp)
    SW                  x28, 29*4(sp)
    SW                  x27, 28*4(sp)
    SW                  x26, 27*4(sp)
    SW                  x25, 26*4(sp)
    SW                  x24, 25*4(sp)
    SW                  x23, 24*4(sp)
    SW                  x22, 23*4(sp)
    SW                  x21, 22*4(sp)
    SW                  x20, 21*4(sp)
    SW                  x19, 20*4(sp)
    SW                  x18, 19*4(sp)
    SW                  x17, 18*4(sp)
    SW                  x16, 17*4(sp)
    SW                  x15, 16*4(sp)
    SW                  x14, 15*4(sp)
    SW                  x13, 14*4(sp)
    SW                  x12, 13*4(sp)
    SW                  x11, 12*4(sp)
    SW                  x10, 11*4(sp)
    SW                  x9, 10*4(sp)
    SW                  x8, 9*4(sp)
    SW                  x7, 8*4(sp)
    SW                  x6, 7*4(sp)
    SW                  x5, 6*4(sp)
    SW                  x4, 5*4(sp)
    SW                  x3, 4*4(sp)
    /* Save sp (x2) */
    CSRR                x3, mscratch
    SW                  x3, 3*4(sp)
    /* Save x1 */
    SW                  x1, 2*4(sp)
    /* Save pc (mepc) */
    CSRR                x3, mepc
    SW                  x3, 1*4(sp)
    /* Save mstatus */
    CSRR                x3, mstatus
    SW                  x3, 0*4(sp)

    /* Load gp for kernel - defined by linker script */
    .option push
    .option norelax
    LA                  gp, __global_pointer$
    .option pop
    /* Call system main interrupt handler */
    CALL                _RME_RV32GP_Handler

    /* Load mstatus */
    LW                  x3, 0*4(sp)
    CSRW                mstatus, x3
    /* Load pc (into mepc) */
    LW                  x3, 1*4(sp)
    CSRW                mepc, x3
    /* Load x1 */
    LW                  x1, 2*4(sp)
    /* Load sp (x2, into mscratch) */
    LW                  x3, 3*4(sp)
    CSRW                mscratch, x3
    /* Load all general-purpose registers - RV32G does not have any flag registers */
    LW                  x3, 4*4(sp)
    LW                  x4, 5*4(sp)
    LW                  x5, 6*4(sp)
    LW                  x6, 7*4(sp)
    LW                  x7, 8*4(sp)
    LW                  x8, 9*4(sp)
    LW                  x9, 10*4(sp)
    LW                  x10, 11*4(sp)
    LW                  x11, 12*4(sp)
    LW                  x12, 13*4(sp)
    LW                  x13, 14*4(sp)
    LW                  x14, 15*4(sp)
    LW                  x15, 16*4(sp)
    LW                  x16, 17*4(sp)
    LW                  x17, 18*4(sp)
    LW                  x18, 19*4(sp)
    LW                  x19, 20*4(sp)
    LW                  x20, 21*4(sp)
    LW                  x21, 22*4(sp)
    LW                  x22, 23*4(sp)
    LW                  x23, 24*4(sp)
    LW                  x24, 25*4(sp)
    LW                  x25, 26*4(sp)
    LW                  x26, 27*4(sp)
    LW                  x27, 28*4(sp)
    LW                  x28, 29*4(sp)
    LW                  x29, 30*4(sp)
    LW                  x30, 31*4(sp)
    LW                  x31, 32*4(sp)
    ADDI                sp, sp, 33*4
    /* Return to user mode */
    CSRRW               sp, mscratch, sp
    MRET
/* End Function:_RME_RV32GP_Handler ******************************************/

/* Begin Function:___RME_RV32GP_Cop_Clear *************************************
Description : Clean up the coprocessor state so that the FP information is not
              leaked when switching from a FPU-enabled thread to a FPU-disabled
              thread. Contains two versions for RVF and RVD.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
/* RVF */
    .section            .text.rvf.clear,"ax"
    .align              1
___RME_RV32GP_Cop_Clear_RVF:
    RET

/* RVD */
    .section            .text.rvd.clear,"ax"
    .align              1
___RME_RV32GP_Cop_Clear_RVD:
    RET
/* End Function:___RME_RV32GP_Cop_Clear **************************************/

/* Begin Function:___RME_RV32GP_Cop_Save **************************************
Description : Save the coprocessor context on switch.
              Contains two versions for RVF and RVD.
Input       : A0 - The pointer to the coprocessor struct.
Output      : None.
Return      : None.
******************************************************************************/
/* RVF */
    .section            .text.rvf.save,"ax"
    .align              1
___RME_RV32GP_Cop_Save_RVF:
    RET

/* RVD */
    .section            .text.rvd.save,"ax"
    .align              1
___RME_RV32GP_Cop_Save_RVD:
    RET
/* End Function:___RME_RV32GP_Cop_Save ***************************************/

/* Begin Function:___RME_RV32GP_Cop_Load **************************************
Description : Restore the coprocessor context on switch.
              Contains two versions for RVF and RVD.
Input       : A0 - The pointer to the coprocessor struct.
Output      : None.
Return      : None.
******************************************************************************/
/* RVF */
    .section            .text.rvf.load,"ax"
    .align              1
___RME_RV32GP_Cop_Load_RVF:
    RET

/* RVD */
    .section            .text.rvd.load,"ax"
    .align              1
___RME_RV32GP_Cop_Load_RVD:
    RET
/* End Function:___RME_RV32GP_Cop_Save ***************************************/

/* Begin Function:___RME_RV32GP_PMP_Set ***************************************
Description : Program the entire PMP array.
Input       : rme_ptr_t* CFG_Meta - The PMP metadata for PMPCFGs.
              rme_ptr_t* ADDR_Meta - The PMP metadata for PMPADDRs.
Output      : None.
Return      : None.
******************************************************************************/
/* Configuration registers */
.macro PMPCFG_SET1
    LW                  t0, 0*4(a0)
    CSRW                pmpcfg0, t0
.endm

.macro PMPCFG_SET2
    PMPCFG_SET1
    LW                  t0, 1*4(a0)
    CSRW                pmpcfg1, t0
.endm

.macro PMPCFG_SET3
    PMPCFG_SET2
    LW                  t0, 2*4(a0)
    CSRW                pmpcfg2, t0
.endm

.macro PMPCFG_SET4
    PMPCFG_SET3
    LW                  t0, 3*4(a0)
    CSRW                pmpcfg3, t0
.endm

/* Address registers */
.macro PMPADDR_SET1
    LW                  t0, 0*4(a1)
    CSRW                pmpaddr0, t0
.endm

.macro PMPADDR_SET2
    PMPADDR_SET1
    LW                  t0, 1*4(a1)
    CSRW                pmpaddr1, t0
.endm

.macro PMPADDR_SET3
    PMPADDR_SET2
    LW                  t0, 2*4(a1)
    CSRW                pmpaddr2, t0
.endm

.macro PMPADDR_SET4
    PMPADDR_SET3
    LW                  t0, 3*4(a1)
    CSRW                pmpaddr3, t0
.endm

.macro PMPADDR_SET5
    PMPADDR_SET4
    LW                  t0, 4*4(a1)
    CSRW                pmpaddr4, t0
.endm

.macro PMPADDR_SET6
    PMPADDR_SET5
    LW                  t0, 5*4(a1)
    CSRW                pmpaddr5, t0
.endm

.macro PMPADDR_SET7
    PMPADDR_SET6
    LW                  t0, 6*4(a1)
    CSRW                pmpaddr6, t0
.endm

.macro PMPADDR_SET8
    PMPADDR_SET7
    LW                  t0, 7*4(a1)
    CSRW                pmpaddr7, t0
.endm

.macro PMPADDR_SET9
    PMPADDR_SET8
    LW                  t0, 8*4(a1)
    CSRW                pmpaddr8, t0
.endm

.macro PMPADDR_SET10
    PMPADDR_SET9
    LW                  t0, 9*4(a1)
    CSRW                pmpaddr9, t0
.endm

.macro PMPADDR_SET11
    PMPADDR_SET10
    LW                  t0, 10*4(a1)
    CSRW                pmpaddr10, t0
.endm

.macro PMPADDR_SET12
    PMPADDR_SET11
    LW                  t0, 11*4(a1)
    CSRW                pmpaddr11, t0
.endm

.macro PMPADDR_SET13
    PMPADDR_SET12
    LW                  t0, 12*4(a1)
    CSRW                pmpaddr12, t0
.endm

.macro PMPADDR_SET14
    PMPADDR_SET13
    LW                  t0, 13*4(a1)
    CSRW                pmpaddr13, t0
.endm

.macro PMPADDR_SET15
    PMPADDR_SET14
    LW                  t0, 14*4(a1)
    CSRW                pmpaddr14, t0
.endm

.macro PMPADDR_SET16
    PMPADDR_SET15
    LW                  t0, 15*4(a1)
    CSRW                pmpaddr15, t0
.endm

/* Functions - in separate sections so they are stripped if not needed */
    .section            .text.pmp1,"ax"
    .align              1
___RME_RV32GP_PMP_Set1:
    PMPCFG_SET1
    PMPADDR_SET1
    RET

    .section            .text.pmp2,"ax"
    .align              1
___RME_RV32GP_PMP_Set2:
    PMPCFG_SET1
    PMPADDR_SET2
    RET

    .section            .text.pmp3,"ax"
    .align              1
___RME_RV32GP_PMP_Set3:
    PMPCFG_SET1
    PMPADDR_SET3
    RET

    .section            .text.pmp4,"ax"
    .align              1
___RME_RV32GP_PMP_Set4:
    PMPCFG_SET1
    PMPADDR_SET4
    RET

    .section            .text.pmp5,"ax"
    .align              1
___RME_RV32GP_PMP_Set5:
    PMPCFG_SET2
    PMPADDR_SET5
    RET

    .section            .text.pmp6,"ax"
    .align              1
___RME_RV32GP_PMP_Set6:
    PMPCFG_SET2
    PMPADDR_SET6
    RET

    .section            .text.pmp7,"ax"
    .align              1
___RME_RV32GP_PMP_Set7:
    PMPCFG_SET2
    PMPADDR_SET7
    RET

    .section            .text.pmp8,"ax"
    .align              1
___RME_RV32GP_PMP_Set8:
    PMPCFG_SET2
    PMPADDR_SET8
    RET

    .section            .text.pmp9,"ax"
    .align              1
___RME_RV32GP_PMP_Set9:
    PMPCFG_SET3
    PMPADDR_SET9
    RET

    .section            .text.pmp10,"ax"
    .align              1
___RME_RV32GP_PMP_Set10:
    PMPCFG_SET3
    PMPADDR_SET10
    RET

    .section            .text.pmp11,"ax"
    .align              1
___RME_RV32GP_PMP_Set11:
    PMPCFG_SET3
    PMPADDR_SET11
    RET

    .section            .text.pmp12,"ax"
    .align              1
___RME_RV32GP_PMP_Set12:
    PMPCFG_SET3
    PMPADDR_SET12
    RET

    .section            .text.pmp13,"ax"
    .align              1
___RME_RV32GP_PMP_Set13:
    PMPCFG_SET4
    PMPADDR_SET13
    RET

    .section            .text.pmp14,"ax"
    .align              1
___RME_RV32GP_PMP_Set14:
    PMPCFG_SET4
    PMPADDR_SET14
    RET

    .section            .text.pmp15,"ax"
    .align              1
___RME_RV32GP_PMP_Set15:
    PMPCFG_SET4
    PMPADDR_SET15
    RET

    .section            .text.pmp16,"ax"
    .align              1
___RME_RV32GP_PMP_Set16:
    PMPCFG_SET4
    PMPADDR_SET16
    RET
/* End Function:___RME_RV32GP_PMP_Set ****************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
