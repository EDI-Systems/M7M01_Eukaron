/******************************************************************************
Filename    : uvmlib_x64.c
Author      : pry
Date        : 26/06/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The x86-64 system library platform specific header.
******************************************************************************/

/* Includes ******************************************************************/
#include "rme.h"

#define __HDR_DEFS__
#include "Platform/X64/uvmlib_x64.h"
#include "Init/syssvc.h"
#undef __HDR_DEFS__

#define __HDR_STRUCTS__
#include "Platform/X64/uvmlib_x64.h"
#include "Init/syssvc.h"
#undef __HDR_STRUCTS__

/* Private include */
#include "Platform/X64/uvmlib_x64.h"

#define __HDR_PUBLIC_MEMBERS__
#include "Init/syssvc.h"
#undef __HDR_PUBLIC_MEMBERS__
/* End Includes **************************************************************/

/* Begin Function:UVM_Putchar *************************************************
Description : Output a character to console. This is for user-level debugging
              only. This only works when IOPL=3.
Input       : char Char - The character to print.
Output      : None.
Return      : ptr_t - Always 0.
******************************************************************************/
ptr_t UVM_Putchar(char Char)
{
    /* Wait until we have transmitted */
    while((__UVM_X64_In(UVM_X64_COM1+5)&0x20)==0);
    __UVM_X64_Out(UVM_X64_COM1, Char);
    UVM_Kern_Act(UVM_BOOT_INIT_KERN,0,0,(ptr_t)Char,0);
    return 0;
}
/* End Function:_UVM_Putchar *************************************************/

/* Begin Function:_UVM_Stack_Init *********************************************
Description : Initialize a thread's stack for synchronous invocation or thread
              creation.
Input       : ptr_t Stack - The start(lower) address of the stub.
              ptr_t Size  - The size of the stack.
              ptr_t Stub  - The address of the stub.
              ptr_t Entry - The entry of the thread/invocation.
              ptr_t Param1 - The parameter 1 to pass to the thread.
              ptr_t Param2 - The parameter 2 to pass to the thread.
              ptr_t Param3 - The parameter 3 to pass to the thread.
              ptr_t Param4 - The parameter 4 to pass to the thread.
Output      : None.
Return      : ptr_t - The actual stack address to use for system call.
******************************************************************************/
ptr_t _UVM_Stack_Init(ptr_t Stack, ptr_t Size, ptr_t Stub, ptr_t Entry,
                      ptr_t Param1, ptr_t Param2, ptr_t Param3, ptr_t Param4)
{
	ptr_t* Stack_Ptr;

    Stack_Ptr=(ptr_t*)(Stack+Size-UVM_STACK_SAFE_SIZE*sizeof(ptr_t));
    Stack_Ptr[0]=Param1;
    Stack_Ptr[1]=Param2;
    Stack_Ptr[2]=Param3;
    Stack_Ptr[3]=Param4;
    Stack_Ptr[4]=Entry;

    return (ptr_t)Stack_Ptr;
}
/* End Function:_RVM_Stack_Init **********************************************/

/* Begin Function:_UVM_Idle ***************************************************
Description : Put the processor into idle state.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void _UVM_Idle(void)
{
    /* Do nothing. In the future we may call a kernel function to put us to sleep */
}
/* End Function:_UVM_Idle ****************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
