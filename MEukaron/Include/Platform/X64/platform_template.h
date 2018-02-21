/******************************************************************************
Filename    : platform_template.h
Author      : pry
Date        : 01/04/2017
Licence     : LGPL v3+; see COPYING for details.
Description: The header of "platform_template.c".
******************************************************************************/

/* Defines *******************************************************************/
#ifdef __HDR_DEFS__
#ifndef __PLATFORM_TEMPLATE_H_DEFS__
#define __PLATFORM_TEMPLATE_H_DEFS__
/*****************************************************************************/
/* Basic Types ***************************************************************/
#if(DEFINE_BASIC_TYPES==TRUE)

#ifndef __S32__
#define __S32__
typedef Dummy  s32;
#endif

#ifndef __S16__
#define __S16__
typedef Dummy s16;
#endif

#ifndef __S8__
#define __S8__
typedef Dummy  s8;
#endif

#ifndef __U32__
#define __U32__
typedef Dummy  u32;
#endif

#ifndef __U16__
#define __U16__
typedef Dummy u16;
#endif

#ifndef __U8__
#define __U8__
typedef Dummy  u8;
#endif

#endif
/* End Basic Types ***********************************************************/

/* Begin Extended Types ******************************************************/
#ifndef __TID_T__
#define __TID_T__
/* The typedef for the Thread ID */
typedef Dummy tid_t;
#endif

#ifndef __PTR_T__
#define __PTR_T__
/* The typedef for the pointers - This is the raw style. Pointers must be unsigned */
typedef Dummy ptr_t;
#endif

#ifndef __CNT_T__
#define __CNT_T__
/* The typedef for the count variables */
typedef Dummy cnt_t;
#endif

#ifndef __CID_T__
#define __CID_T__
/* The typedef for capability ID */
typedef Dummy cid_t;
#endif

#ifndef __RET_T__
#define __RET_T__
/* The type for process return value */
typedef Dummy ret_t;
#endif
/* End Extended Types ********************************************************/

/* System macros *************************************************************/
/* Compiler "extern" keyword setting */
#define EXTERN                  Dummy
/* Compiler "inline" keyword setting */
#define INLINE                  Dummy
/* Number of CPUs in the system */
#define RME_CPU_NUM             Dummy
/* The order of bits in one CPU machine word */
#define RME_WORD_ORDER          Dummy
/* Forcing VA=PA in user memory segments */
#define RME_VA_EQU_PA           Dummy
/* Quiescence timeslice value */
#define RME_QUIE_TIME           Dummy
/* Normal page directory size calculation macro */
#define RME_PGTBL_SIZE_NOM(NUM_ORDER)   Dummy
/* Top-level page directory size calculation macro */
#define RME_PGTBL_SIZE_TOP(NUM_ORDER)   Dummy

/* The virtual memory start address for the kernel objects */
#define RME_KMEM_VA_START            Dummy
/* The size of the kernel object virtual memory */
#define RME_KMEM_SIZE                Dummy
/* The granularity of kernel memory allocation, in bytes */
#define RME_KMEM_SLOT_ORDER          Dummy
/* Kernel stack size and address */
#define RME_KMEM_STACK_ADDR          Dummy
/* The maximum number of preemption priority levels in the system.
 * This parameter must be divisible by the word length - 32 is usually sufficient */
#define RME_MAX_PREEMPT_PRIO         Dummy
/* End System macros *********************************************************/

/* __PLATFORM_TEMPLATE_H_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Structs *******************************************************************/
#ifdef __HDR_STRUCTS__
#ifndef __PLATFORM_TEMPLATE_H_STRUCTS__
#define __PLATFORM_TEMPLATE_H_STRUCTS__
/* We used structs in the header */

/* Use defines in these headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__
/*****************************************************************************/
/* The register set struct - R0-R3, R12, PC, LR, xPSR is automatically pushed.
 * Here we need LR to decide EXC_RETURN, that's why it is here */
struct RME_Reg_Struct
{
    ptr_t Dummy;
};

/* The coprocessor register set structure. In Cortex-M, if there is a 
 * single-precision FPU, then the FPU S0-S15 is automatically pushed */
struct RME_Cop_Struct
{
    ptr_t Dummy;
};
/*****************************************************************************/
/* __PLATFORM_TEMPLATE_H_STRUCTS__ */
#endif
/* __HDR_STRUCTS__ */
#endif
/* End Structs ***************************************************************/

/* Private Global Variables **************************************************/
#if(!(defined __HDR_DEFS__||defined __HDR_STRUCTS__))
#ifndef __PLATFORM_TEMPLATE_MEMBERS__
#define __PLATFORM_TEMPLATE_MEMBERS__

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
/* Interrupts */
EXTERN void __RME_Disable_Int(void);
EXTERN void __RME_Enable_Int(void);
/* Atomics */
__EXTERN__ ptr_t __RME_Comp_Swap(ptr_t* Ptr, ptr_t* Old, ptr_t New);
__EXTERN__ ptr_t __RME_Fetch_Add(ptr_t* Ptr, cnt_t Addend);
__EXTERN__ ptr_t __RME_Fetch_And(ptr_t* Ptr, ptr_t Operand);
/* MSB counting */
EXTERN ptr_t __RME_MSB_Get(ptr_t Val);
/* Debugging */
__EXTERN__ ptr_t __RME_Putchar(char Char);
/* Booting */
EXTERN void _RME_Kmain(ptr_t Stack);
EXTERN void __RME_Enter_User_Mode(ptr_t Entry_Addr, ptr_t Stack_Addr);
__EXTERN__ ptr_t __RME_Low_Level_Init(void);
__EXTERN__ ptr_t __RME_Boot(void);
__EXTERN__ void __RME_Reboot(void);
__EXTERN__ void __RME_Shutdown(void);
/* Syscall & invocation */
__EXTERN__ ptr_t __RME_CPUID_Get(void);
__EXTERN__ ptr_t __RME_Get_Syscall_Param(struct RME_Reg_Struct* Reg, ptr_t* Svc,
                                         ptr_t* Capid, ptr_t* Param);
__EXTERN__ ptr_t __RME_Set_Syscall_Retval(struct RME_Reg_Struct* Reg, ret_t Retval);
__EXTERN__ ptr_t __RME_Get_Inv_Retval(struct RME_Reg_Struct* Reg);
__EXTERN__ ptr_t __RME_Set_Inv_Retval(struct RME_Reg_Struct* Reg, ret_t Retval);
/* Thread register sets */
__EXTERN__ ptr_t __RME_Thd_Reg_Init(ptr_t Entry_Addr, ptr_t Stack_Addr, struct RME_Reg_Struct* Reg);
__EXTERN__ ptr_t __RME_Thd_Reg_Copy(struct RME_Reg_Struct* Dst, struct RME_Reg_Struct* Src);
__EXTERN__ ptr_t __RME_Thd_Cop_Init(ptr_t Entry_Addr, ptr_t Stack_Addr, struct RME_Cop_Struct* Cop_Reg);
__EXTERN__ ptr_t __RME_Thd_Cop_Save(struct RME_Reg_Struct* Reg, struct RME_Cop_Struct* Cop_Reg);
__EXTERN__ ptr_t __RME_Thd_Cop_Restore(struct RME_Reg_Struct* Reg, struct RME_Cop_Struct* Cop_Reg);
/* Invocation register sets */
__EXTERN__ ptr_t __RME_Inv_Reg_Init(ptr_t Param, struct RME_Reg_Struct* Reg);
__EXTERN__ ptr_t __RME_Inv_Cop_Init(ptr_t Param, struct RME_Cop_Struct* Cop_Reg);
/* Kernel function handler */
__EXTERN__ ptr_t __RME_Kern_Func_Handler(struct RME_Reg_Struct* Reg, ptr_t Func_ID, 
                                         ptr_t Param1, ptr_t Param2);
/* Page table operations */
__EXTERN__ void __RME_Pgtbl_Set(ptr_t Pgtbl);
__EXTERN__ ptr_t __RME_Pgtbl_Kmem_Init(void);
__EXTERN__ ptr_t __RME_Pgtbl_Check(ptr_t Start_Addr, ptr_t Top_Flag, ptr_t Size_Order, ptr_t Num_Order);
__EXTERN__ ptr_t __RME_Pgtbl_Init(struct RME_Cap_Pgtbl* Pgtbl_Op);
__EXTERN__ ptr_t __RME_Pgtbl_Del_Check(struct RME_Cap_Pgtbl* Pgtbl_Op);
__EXTERN__ ptr_t __RME_Pgtbl_Page_Map(struct RME_Cap_Pgtbl* Pgtbl_Op, ptr_t Paddr, ptr_t Pos, ptr_t Flags);
__EXTERN__ ptr_t __RME_Pgtbl_Page_Unmap(struct RME_Cap_Pgtbl* Pgtbl_Op, ptr_t Pos);
__EXTERN__ ptr_t __RME_Pgtbl_Pgdir_Map(struct RME_Cap_Pgtbl* Pgtbl_Parent, ptr_t Pos, 
                                       struct RME_Cap_Pgtbl* Pgtbl_Child);
__EXTERN__ ptr_t __RME_Pgtbl_Pgdir_Unmap(struct RME_Cap_Pgtbl* Pgtbl_Op, ptr_t Pos);
__EXTERN__ ptr_t __RME_Pgtbl_Lookup(struct RME_Cap_Pgtbl* Pgtbl_Op, ptr_t Pos, ptr_t* Paddr, ptr_t* Flags);
__EXTERN__ ptr_t __RME_Pgtbl_Walk(struct RME_Cap_Pgtbl* Pgtbl_Op, ptr_t Vaddr, ptr_t* Pgtbl,
                                  ptr_t* Map_Vaddr, ptr_t* Paddr, ptr_t* Size_Order, ptr_t* Num_Order, ptr_t* Flags);
/*****************************************************************************/
/* Undefine "__EXTERN__" to avoid redefinition */
#undef __EXTERN__
/* __PLATFORM_TEMPLATE_MEMBERS__ */
#endif
/* !(defined __HDR_DEFS__||defined __HDR_STRUCTS__) */
#endif
/* End Public C Function Prototypes ******************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
