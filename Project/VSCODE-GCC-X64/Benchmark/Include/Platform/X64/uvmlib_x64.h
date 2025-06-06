/******************************************************************************
Filename    : uvmlib_x64.h
Author      : pry
Date        : 25/06/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The header of the platform dependent part.
******************************************************************************/

/* Defines *******************************************************************/
#ifdef __HDR_DEFS__
#ifndef __UVMLIB_X64_H_DEFS__
#define __UVMLIB_X64_H_DEFS__
/*****************************************************************************/
/* Definitions of basic types */
/* Basic Types ***************************************************************/
#ifndef __S64__
#define __S64__
typedef signed long long s64;
#endif

#ifndef __S32__
#define __S32__
typedef signed int s32;
#endif

#ifndef __S16__
#define __S16__
typedef signed short s16;
#endif

#ifndef __S8__
#define __S8__
typedef signed char s8;
#endif

#ifndef __U64__
#define __U64__
typedef unsigned long long u64;
#endif

#ifndef __U32__
#define __U32__
typedef unsigned int u32;
#endif

#ifndef __U16__
#define __U16__
typedef unsigned short u16;
#endif

#ifndef __U8__
#define __U8__
typedef unsigned char u8;
#endif
/* End Basic Types ***********************************************************/
/* EXTERN keyword definition */
#define EXTERN                              extern
/* The order of bits in one CPU machine word */
#define UVM_WORD_ORDER                      6
/* The stack safe redundancy size in words - set to 0x20 words */
#define UVM_STACK_SAFE_SIZE                 0x20
/* The COM1 I/O address */
#define UVM_X64_COM1                        0x3F8
/**/

/* Platform-specific includes */
#include "Platform/X64/uvmlib_x64_conf.h"
/* Begin Extended Types ******************************************************/
#ifndef __PID_T__
#define __PID_T__
/* The typedef for the Process ID */
typedef s64 pid_t;
#endif

#ifndef __TID_T__
#define __TID_T__
/* The typedef for the Thread ID */
typedef s64 tid_t;
#endif

#ifndef __PTR_T__
#define __PTR_T__
/* The typedef for the pointers - This is the raw style. Pointers must be unsigned */
typedef u64 ptr_t;
#endif

#ifndef __CNT_T__
#define __CNT_T__
/* The typedef for the count variables */
typedef s64 cnt_t;
#endif

#ifndef __CID_T__
#define __CID_T__
/* The typedef for capability ID */
typedef s64 cid_t;
#endif

#ifndef __RET_T__
#define __RET_T__
/* The type for process return value */
typedef s64 ret_t;
#endif
/* End Extended Types ********************************************************/
/*****************************************************************************/
/* __UVMLIB_X64_H_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Structs *******************************************************************/
#ifdef __HDR_STRUCTS__
#ifndef __UVMLIB_X64_H_STRUCTS__
#define __UVMLIB_X64_H_STRUCTS__
/* We used structs in the header */

/* Use defines in these headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__
/*****************************************************************************/
struct UVM_Reg_Struct
{
    ptr_t SP;
    ptr_t R4;
    ptr_t R5;
    ptr_t R6;
    ptr_t R7;
    ptr_t R8;
    ptr_t R9;
    ptr_t R10;
    ptr_t R11;
    ptr_t LR;
};

/* The coprocessor register set structure. In Cortex-M, if there is a 
 * single-precision FPU, then the FPU S0-S15 is automatically pushed */
struct UVM_Cop_Struct
{
    ptr_t S16;
    ptr_t S17;
    ptr_t S18;
    ptr_t S19;
    ptr_t S20;
    ptr_t S21;
    ptr_t S22;
    ptr_t S23;
    ptr_t S24;
    ptr_t S25;
    ptr_t S26;
    ptr_t S27;
    ptr_t S28;
    ptr_t S29;
    ptr_t S30;
    ptr_t S31;
};
/*****************************************************************************/
/* __UVMLIB_X64_H_STRUCTS__ */
#endif
/* __HDR_STRUCTS__ */
#endif
/* End Structs ***************************************************************/

/* Private Global Variables **************************************************/
#if(!(defined __HDR_DEFS__||defined __HDR_STRUCTS__))
#ifndef __UVMLIB_X64_MEMBERS__
#define __UVMLIB_X64_MEMBERS__

/* In this way we can use the data structures and definitions in the headers */
#define __HDR_DEFS__

#undef __HDR_DEFS__

#define __HDR_STRUCTS__

#undef __HDR_STRUCTS__

/* If the header is not used in the public mode */
#ifndef __HDR_PUBLIC_MEMBERS__
/*****************************************************************************/

/*****************************************************************************/
/* End Private Global Variables **********************************************/

/* Private C Function Prototypes *********************************************/ 
/*****************************************************************************/

/*****************************************************************************/
#define __EXTERN__
/* End Private C Function Prototypes *****************************************/

/* Public Global Variables ***************************************************/
/* __HDR_PUBLIC_MEMBERS__ */
#else
#define __EXTERN__ EXTERN 
/* __HDR_PUBLIC_MEMBERS__ */
#endif

/*****************************************************************************/

/*****************************************************************************/

/* End Public Global Variables ***********************************************/

/* Public C Function Prototypes **********************************************/
/*****************************************************************************/
/* I/O ports */
EXTERN ptr_t __UVM_X64_In(ptr_t Port);
EXTERN void __UVM_X64_Out(ptr_t Port, ptr_t Data);
EXTERN ptr_t __UVM_X64_Read_TSC(void);
/* Stubs in assembly */
EXTERN void _UVM_Entry(void);
EXTERN ptr_t* _UVM_Get_TLS_Pos(ptr_t Mask);
EXTERN void _UVM_Thd_Stub(void);
EXTERN void _UVM_Inv_Stub(void);
EXTERN ptr_t _UVM_MSB_Get(ptr_t Val);
EXTERN ret_t UVM_Svc(ptr_t Op_Capid, ptr_t Arg1, ptr_t Arg2, ptr_t Arg3);
/* Printing */
__EXTERN__ ptr_t UVM_Putchar(char Char);
/* Stack initialization */
__EXTERN__ ptr_t _UVM_Stack_Init(ptr_t Stack, ptr_t Size, ptr_t Stub, ptr_t Entry,
                                 ptr_t Param1, ptr_t Param2, ptr_t Param3, ptr_t Param4);
/* Idle function */
__EXTERN__ void _UVM_Idle(void);
/*****************************************************************************/
/* Undefine "__EXTERN__" to avoid redefinition */
#undef __EXTERN__
/* __UVMLIB_X64_MEMBERS__ */
#endif
/* !(defined __HDR_DEFS__||defined __HDR_STRUCTS__) */
#endif
/* End Public C Function Prototypes ******************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
