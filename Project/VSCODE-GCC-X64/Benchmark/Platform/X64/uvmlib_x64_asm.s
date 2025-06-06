/****************************************************************************
Filename    : uvmlib_x64_asm.S
Author      : pry
Date        : 03/05/2018
Description : The x86-64 user-level assembly scheduling support of the UVM RTOS.
*****************************************************************************/

/* Begin Exports ************************************************************/
    /* Entry of the image */
    .global             _UVM_Entry
    /* Set the position of thread local storage */
    .global             _UVM_Set_TLS_Pos
    /* Get the position of thread local storage */
    .global             _UVM_Get_TLS_Pos
    /* Activate an invocation */
    .global             UVM_Inv_Act
    /* Return from an invocation */
    .global             UVM_Inv_Ret
    /* System call */
    .global             UVM_Svc
    /* Get the MSB of the word */
    .global             _UVM_MSB_Get
    /* Thread stub and invocation stub */
    .global             _UVM_Thd_Stub
    .global             _UVM_Inv_Stub
    /* Port operations */
    .global             __UVM_X64_In
    .global             __UVM_X64_Out
    /* Read timestamp */
    .global             __UVM_X64_Read_TSC
/* End Exports **************************************************************/

/* Begin Imports ************************************************************/
    /* The entry of user-level library */
    .global             _UVM_Entry
/* End Imports **************************************************************/

/* Begin Memory Init ********************************************************/

/* End Memory Init **********************************************************/

/* Begin Function:_UVM_Entry *************************************************
Description : The entry of the process. This should always start at address 0.
              The stack for the init process is always 2kB. Thus, booting 4096
              processors use up to 8MB of memory, which fits into the 16MB reserved
              space very well.
Input       : None.
Output      : None.
*****************************************************************************/
_UVM_Entry:
    JMP                 main
/* End Function:_UVM_Entry **************************************************/

/* Begin Function:__UVM_X64_In ***********************************************
Description    : The function for outputting something to an I/O port.
Input          : ptr_t Port - The port to output to.
Output         : None.
Return         : ptr_t - The data received from that port.
Register Usage : None.
*****************************************************************************/
__UVM_X64_In:
    PUSHQ               %RDX
    MOVQ                %RDI,%RDX
    MOVQ                %RAX,%RAX
    INB                 (%DX),%AL
    POPQ                %RDX
    RETQ
/* End Function:__UVM_X64_In ************************************************/

/* Begin Function:__UVM_X64_Out **********************************************
Description    : The function for outputting something to an I/O port.
Input          : ptr_t Port - The port to output to.
                 ptr_t Data - The data to send to that port.
Output         : None.
Return         : None.
Register Usage : None.
*****************************************************************************/
__UVM_X64_Out:
    PUSHQ               %RDX
    PUSHQ               %RAX
    MOVQ                %RDI,%RDX
    MOVQ                %RSI,%RAX
    OUTB                %AL,(%DX)
    POPQ                %RAX
    POPQ                %RDX
    RETQ
/* End Function:__UVM_X64_Out ***********************************************/

/* Begin Function:__UVM_X64_Read_TSC *****************************************
Description    : The function for reading the timestamp counter of x64.
Input          : None.
Output         : None.
Return         : ptr_t - The timestamp value returned.
Register Usage : None.
******************************************************************************/
__UVM_X64_Read_TSC:
    PUSHQ               %RDX
    RDTSC
    SHL                 $32,%RDX
    ORQ                 %RDX,%RAX
    POPQ                %RDX
    RETQ
/* End Function:__UVM_X64_Read_TSC ******************************************/

/* Begin Function:_UVM_Set_TLS_Pos *******************************************
Description : Set the TLS location.
Input       : ptr_t Mask - The alignment mask.
              ptr_t TLS - The TLS location.
Output      : None.
Return      : None.
*****************************************************************************/
_UVM_Set_TLS_Pos:
    /* The alignment mask is not used in x86-64 */
    WRFSBASE            %RSI
    RETQ
/* End Function:_UVM_Set_TLS_Pos ********************************************/

/* Begin Function:_UVM_Get_TLS_Pos *******************************************
Description : Get the TLS location.
Input       : ptr_t Mask - The alignment mask.
Output      : None.
Return      : ptr_t* - The thread local storage position.
*****************************************************************************/
_UVM_Get_TLS_Pos:
    /* The alignment mask is not used in x86-64 */
    RDFSBASE            %RAX
    RETQ
/* End Function:_UVM_Get_TLS_Pos ********************************************/

/* Begin Function:_UVM_Thd_Stub **********************************************
Description : The user level stub for thread creation.
Input       : RDI - The entry address.
              RSI - The stack address that we are using now.
Output      : None.
*****************************************************************************/
_UVM_Thd_Stub:
    MOVQ                (%RSP),%RDI
    MOVQ                8(%RSP),%RSI
    MOVQ                16(%RSP),%RDX
    MOVQ                24(%RSP),%RCX
    JMP                 *32(%RSP)           /* Jump to the actual entry address */
/* End Function:_UVM_Thd_Stub ***********************************************/

/* Begin Function:_UVM_Inv_Stub **********************************************
Description : The user level stub for synchronous invocation. This stub will
              do a return automatically when the function exits.
Input       : R4 - The entry address.
              R5 - The stack address that we are using now.
Output      : None.
*****************************************************************************/
_UVM_Inv_Stub:
    MOVQ                %RSI,%RDI           /* Pass the parameter */
    CALLQ               *32(%RSP)           /* Branch to the actual entry address */

    XORQ                %RDI,%RDI           /* UVM_SVC_INV_RET */
    MOVQ                %RAX,%RSI           /* return value in RSI */
    SYSCALL                                 /* System call */
/* End Function:_UVM_Inv_Stub ***********************************************/

/* Begin Function:UVM_Inv_Act ************************************************
Description : Activate an invocation. If the return value is not desired, pass
              0 into RDX. This is a default implementation that saves all general
              purpose registers and doesn't save FPU context. If you need a faster
              version, consider inline functions; if you need to save FPU contexts,
              please DIY.
Input       : RDI - cid_t Cap_Inv - The capability slot to the invocation stub. 2-Level.
              RSI - ptr_t Param - The parameter for the call.
Output      : RDX - ptr_t* Retval - The return value from the call.
Return      : RAX - ptr_t - The return value of the system call itself.
*****************************************************************************/
UVM_Inv_Act:
    PUSHQ               %RBX                /* The user-level should push all context */
    PUSHQ               %RCX
    PUSHQ               %RDX
    PUSHQ               %RBP
    PUSHQ               %R8
    PUSHQ               %R9
    PUSHQ               %R10
    PUSHQ               %R11
    PUSHQ               %R12
    PUSHQ               %R13
    PUSHQ               %R14
    PUSHQ               %R15
    PUSHFQ

    MOVQ                %RSI,%RDX           /* Param */
    MOVQ                %RDI,%RSI           /* Cap_Inv */
    MOVQ                $0x100000000,%RDI   /* UVM_SVC_INV_ACT */
    SYSCALL

    POPFQ
    POPQ                %R15
    POPQ                %R14
    POPQ                %R13
    POPQ                %R12
    POPQ                %R11
    POPQ                %R10
    POPQ                %R9
    POPQ                %R8
    POPQ                %RBP
    POPQ                %RDX
    POPQ                %RCX
    POPQ                %RBX                /* POP all saved registers now */

    CMPQ                $0,%RDX             /* See if this return value is desired */
    JZ                  No_Retval
    MOVQ                %RSI,(%RDX)
No_Retval:
    RETQ

UVM_Inv_Act_Dummy:
    .global UVM_Inv_Act_Dummy
    PUSHQ               %RBX                /* The user-level should push all context */
    PUSHQ               %RCX
    PUSHQ               %RDX
    PUSHQ               %RBP
    PUSHQ               %R8
    PUSHQ               %R9
    PUSHQ               %R10
    PUSHQ               %R11
    PUSHQ               %R12
    PUSHQ               %R13
    PUSHQ               %R14
    PUSHQ               %R15
    PUSHFQ

    MOVQ                %RSI,%RDX           /* Param */
    MOVQ                %RDI,%RSI           /* Cap_Inv */
    MOVQ                $0x100000000,%RDI   /* UVM_SVC_INV_ACT */

    POPFQ
    POPQ                %R15
    POPQ                %R14
    POPQ                %R13
    POPQ                %R12
    POPQ                %R11
    POPQ                %R10
    POPQ                %R9
    POPQ                %R8
    POPQ                %RBP
    POPQ                %RDX
    POPQ                %RCX
    POPQ                %RBX                /* POP all saved registers now */

    CMPQ                $0,%RDX             /* See if this return value is desired */
    JZ                  No_Retval_Dummy
    MOVQ                %RSI,(%RDX)
No_Retval_Dummy:
    RETQ

UVM_cret:
    .global     UVM_cret
    RETQ
/* End Function:UVM_Inv_Act *************************************************/

/* Begin Function:UVM_Inv_Ret ************************************************
Description : Manually return from an invocation, and set the return value to
              the old register set. This function does not need a capability
              table to work, and never returns.
Input       : RDI - The returning result from the invocation.
Output      : None.
Return      : None.
*****************************************************************************/
UVM_Inv_Ret:
    MOVQ                %RDI,%RSI           /* Set return value to the register */
    XORQ                %RDI,%RDI           /* UVM_SVC_INV_RET */
    SYSCALL                                 /* System call */
    RETQ
;/* End Function:UVM_Inv_Ret *************************************************/

/* Begin Function:UVM_Svc ****************************************************
Description : Trigger a system call.
Input       : RDI - The system call number/other information.
              RSI - Argument 1.
              RDX - Argument 2.
              RCX - Argument 3. We need to move this to R8 because SYSCALL will use RCX.
Output      : None.
Retun       : RAX - The return value.
*****************************************************************************/
UVM_Svc:
    MOV                 %RCX,%R8
    PUSH                %R11
    SYSCALL                                 /* Do the system call directly */
    POP                 %R11
    RETQ
;/* End Function:UVM_Svc *****************************************************/

;/* Begin Function:_UVM_MSB_Get ***********************************************
;Description    : Get the MSB of the word.
;Input          : ptr_t Val - The value.
;Output         : None.
;Return         : ptr_t - The MSB position.   
;Register Usage : None. 
;*****************************************************************************/
_UVM_MSB_Get:
    LZCNTQ              %RDI,%RDI
    MOVQ                $63,%RAX
    SUBQ                %RDI,%RAX
    RETQ
;/* End Function:_UVM_MSB_Get ************************************************/

/* End Of File **************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved **********************/
