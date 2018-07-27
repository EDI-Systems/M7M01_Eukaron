/******************************************************************************
Filename    : rme_kernel.h
Author      : pry
Date        : 08/04/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The header of kernel system call path.
******************************************************************************/

/* Defines *******************************************************************/
#ifdef __HDR_DEFS__
#ifndef __RME_KERNEL_H_DEFS__
#define __RME_KERNEL_H_DEFS__
/*****************************************************************************/
/* Generic definitions */
#define RME_TRUE                        1
#define RME_FALSE                       0
#define RME_NULL                        0
#define RME_EXIST                       1
#define RME_EMPTY                       0

/* Bit mask/address operations */
#define RME_ALLBITS                     ((rme_ptr_t)(-1))
#define RME_WORD_BITS                   (sizeof(rme_ptr_t)*8)
#define RME_POSITIVE_BITS               (RME_ALLBITS>>1)
/* Apply this mask to keep START to MSB bits */
#define RME_MASK_START(START)           ((RME_ALLBITS)<<(START))
/* Apply this mask to keep LSB to END bits */
#define RME_MASK_END(END)               ((RME_ALLBITS)>>(RME_WORD_BITS-1-(END)))
/* Apply this mask to keep START to END bits, START < END */
#define RME_MASK(START,END)             ((RME_MASK_START(START))&(RME_MASK_END(END)))
/* Round the number down & up to a power of 2, or get the power of 2 */
#define RME_ROUND_DOWN(NUM,POW)         ((NUM)&(RME_MASK_START(POW)))
#define RME_ROUND_UP(NUM,POW)           RME_ROUND_DOWN((NUM)+RME_MASK_END(POW-1),POW)
#define RME_POW2(POW)                   (((rme_ptr_t)1)<<(POW))
/* Check if address is aligned on word boundary */
#define RME_IS_ALIGNED(ADDR)    (((ADDR)&RME_MASK_END(RME_WORD_ORDER-4))==0)
/* Bit field extraction macros for easy extraction of parameters
[MSB                                 PARAMS                                 LSB]
[                  D1                  ][                  D0                  ]
[        Q3        ][        Q2        ][        Q1        ][        Q0        ]
[   O7   ][   O6   ][   O5   ][   O4   ][   O3   ][   O2   ][   O1   ][   O0   ] 
*/
/* Cut in half */
#define RME_PARAM_D1(X)                 ((X)>>(sizeof(rme_ptr_t)*4))
#define RME_PARAM_D0(X)                 ((X)&RME_MASK_END((sizeof(rme_ptr_t)*4)-1))
/* Cut into 4 parts */
#define RME_PARAM_Q3(X)                 ((X)>>(sizeof(rme_ptr_t)*6))
#define RME_PARAM_Q2(X)                 (((X)>>(sizeof(rme_ptr_t)*4))&RME_MASK_END((sizeof(rme_ptr_t)*2)-1))
#define RME_PARAM_Q1(X)                 (((X)>>(sizeof(rme_ptr_t)*2))&RME_MASK_END((sizeof(rme_ptr_t)*2)-1))
#define RME_PARAM_Q0(X)                 ((X)&RME_MASK_END((sizeof(rme_ptr_t)*2)-1))
/* Cut into 8 parts */
#define RME_PARAM_O7(X)                 ((X)>>(sizeof(rme_ptr_t)*7))
#define RME_PARAM_O6(X)                 (((X)>>(sizeof(rme_ptr_t)*6))&RME_MASK_END(sizeof(rme_ptr_t)-1))
#define RME_PARAM_O5(X)                 (((X)>>(sizeof(rme_ptr_t)*5))&RME_MASK_END(sizeof(rme_ptr_t)-1))
#define RME_PARAM_O4(X)                 (((X)>>(sizeof(rme_ptr_t)*4))&RME_MASK_END(sizeof(rme_ptr_t)-1))
#define RME_PARAM_O3(X)                 (((X)>>(sizeof(rme_ptr_t)*3))&RME_MASK_END(sizeof(rme_ptr_t)-1))
#define RME_PARAM_O2(X)                 (((X)>>(sizeof(rme_ptr_t)*2))&RME_MASK_END(sizeof(rme_ptr_t)-1))
#define RME_PARAM_O1(X)                 (((X)>>(sizeof(rme_ptr_t)*1))&RME_MASK_END(sizeof(rme_ptr_t)-1))
#define RME_PARAM_O0(X)                 ((X)&RME_MASK_END(sizeof(rme_ptr_t)-1))
    
/* This is the special one used for delegation, and used for kernel memory
 * capability only because it is very complicated. Other capabilities will not use this */
#define RME_PARAM_KM(SVC,CAPID)         (((SVC)<<(sizeof(rme_ptr_t)*4))|(CAPID))
/* This is the special one used for page table top-level flags */
#define RME_PARAM_PT(X)                 ((X)&0x01)

/* Kernel function capability flag arrangement
* 32-bit systems: Maximum kernel function number 2^16
* [31        High Limit        16] [15        Low Limit        0]
* 64-bit systems: Maximum kernel function number 2^32
* [63        High Limit        32] [31        Low Limit        0] */
#define RME_KERN_FLAG_HIGH(X)           ((X)>>(sizeof(rme_ptr_t)*4))
#define RME_KERN_FLAG_LOW(X)            ((X)&RME_MASK_END((sizeof(rme_ptr_t)*4)-1))
#define RME_KERN_FLAG_FULL_RANGE        (((rme_ptr_t)(-1))&RME_MASK_START(sizeof(rme_ptr_t)*4))
    
/* Kernel memory function capability flag arrangement - extended flags used, Granularity always 64 bytes min,
* because the reserved bits is always 6 bits, and the flags field in the Ext_Flag is always 6 bits too.
* 32-bit systems:
* [31          High Limit[31:16]         16] [15       Low Limit[31:16]       0]  Flags
* [31 High Limit[15: 6] 22] [21 Reserved 16] [15 Low Limit[15: 6] 6] [5 Flags 0]  Ext_Flags
* 64-bit systems:
* [63          High Limit[64:32]         32] [31       Low Limit[64:32]       0]  Flags
* [63 High Limit[31: 6] 38] [37 Reserved 32] [31 Low Limit[31: 6] 6] [5 Flags 0]  Ext_Flags
*/
#define RME_KMEM_FLAG_HIGH_F(FLAGS)         ((FLAGS)&RME_MASK_START(sizeof(rme_ptr_t)*4))
#define RME_KMEM_FLAG_HIGH_E(EFLAGS)        (((EFLAGS)>>(sizeof(rme_ptr_t)*4))&RME_MASK_START(6))
#define RME_KMEM_FLAG_HIGH(FLAGS,EFLAGS)    (RME_KMEM_FLAG_HIGH_F(FLAGS)|RME_KMEM_FLAG_HIGH_E(EFLAGS))
#define RME_KMEM_FLAG_LOW_F(FLAGS)          ((FLAGS)<<(sizeof(rme_ptr_t)*4))
#define RME_KMEM_FLAG_LOW_E(EFLAGS)         ((EFLAGS)&RME_MASK(sizeof(rme_ptr_t)*4-1,6))
#define RME_KMEM_FLAG_LOW(FLAGS,EFLAGS)     (RME_KMEM_FLAG_LOW_F(FLAGS)|RME_KMEM_FLAG_LOW_E(EFLAGS))
#define RME_KMEM_FLAG_FLAGS(EFLAGS)         ((EFLAGS)&RME_MASK(5,0))

/* The return procedure of a possible context switch - If successful, the function itself
 * is responsible for setting the parameters; If failed, we set the parameters for it.
 * Possible categories of context switch includes synchronous invocation and thread switch. */
#define RME_SWITCH_RETURN(REG,RETVAL) \
{ \
    if(RME_UNLIKELY((RETVAL)<0)) \
        __RME_Set_Syscall_Retval((REG),(RETVAL)); \
    \
    return; \
}

/* The system service numbers are defined here. This is included in both user level 
 * and kernel level */
#include "rme.h"

/* Debugging */
#define RME_KERNEL_DEBUG_MAX_STR        128
/* Printk macros */
#define RME_PRINTK_I(INT)               RME_Print_Int((INT))
#define RME_PRINTK_U(UINT)              RME_Print_Uint((UINT))
#define RME_PRINTK_S(STR)               RME_Print_String((rme_s8_t*)(STR))
    
/* Assert macro */
#define RME_ASSERT(X) \
do \
{ \
    if(RME_UNLIKELY((X)==0)) \
    { \
        RME_PRINTK_S((rme_s8_t*)"\r\n***\r\nKernel panic - not syncing:\r\n"); \
        RME_PRINTK_S((rme_s8_t*)__FILE__); \
        RME_PRINTK_S((rme_s8_t*)" , Line "); \
        RME_PRINTK_I(__LINE__); \
        RME_PRINTK_S((rme_s8_t*)"\r\n"); \
        RME_PRINTK_S((rme_s8_t*)__DATE__); \
        RME_PRINTK_S((rme_s8_t*)" , "); \
        RME_PRINTK_S((rme_s8_t*)__TIME__); \
        RME_PRINTK_S((rme_s8_t*)"\r\n"); \
        while(1); \
    } \
} \
while(0)

/* __RME_KERNEL_H_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Structs *******************************************************************/
#ifdef __HDR_STRUCTS__
#ifndef __RME_KERNEL_H_STRUCTS__
#define __RME_KERNEL_H_STRUCTS__

/* Use defines in these headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__

/*****************************************************************************/
/* The kernel function capability */
struct RME_Cap_Kern
{
    struct RME_Cap_Head Head;
    rme_ptr_t Info[3];
};

/* The kernel memory capability - This is for preventing denial of service */
struct RME_Cap_Kmem
{
    struct RME_Cap_Head Head;
    /* The start address of the allowed kernel memory */
    rme_ptr_t Start;
    /* The end address of the allowed kernel memory */
    rme_ptr_t End;
    rme_ptr_t Info[1];
};
/*****************************************************************************/
/* __RME_KERNEL_H_STRUCTS__ */
#endif
/* __HDR_STRUCTS__ */
#endif
/* End Structs ***************************************************************/

/* Private Global Variables **************************************************/
#if(!(defined __HDR_DEFS__||defined __HDR_STRUCTS__))
#ifndef __RME_KERNEL_MEMBERS__
#define __RME_KERNEL_MEMBERS__

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
static rme_ret_t _RME_Syscall_Init(void);
static rme_ret_t __RME_Low_Level_Check(void);
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
/* Current timestamp counter */
__EXTERN__ rme_ptr_t RME_Timestamp;
/*****************************************************************************/

/* End Public Global Variables ***********************************************/

/* Public C Function Prototypes **********************************************/
/*****************************************************************************/
/* Kernel entry */
__EXTERN__ rme_ret_t RME_Kmain(void);
/* Increase counter */
__EXTERN__ rme_ptr_t _RME_Timestamp_Inc(rme_cnt_t Value);
/* Memory helpers */
__EXTERN__ void _RME_Clear(void* Addr, rme_ptr_t Size);
__EXTERN__ rme_ret_t _RME_Memcmp(const void* Ptr1, const void* Ptr2, rme_ptr_t Num);
__EXTERN__ void _RME_Memcpy(void* Dst, void* Src, rme_ptr_t Num);
/* Kernel capability */
__EXTERN__ rme_ret_t _RME_Kern_Boot_Crt(struct RME_Cap_Captbl* Captbl, rme_cid_t Cap_Captbl, rme_cid_t Cap_Kern);
__EXTERN__ rme_ret_t _RME_Kern_Act(struct RME_Cap_Captbl* Captbl, struct RME_Reg_Struct* Reg,
                                   rme_cid_t Cap_Kern, rme_ptr_t Func_ID, rme_ptr_t Sub_ID, rme_ptr_t Param1, rme_ptr_t Param2);
/* Kernel memory capability */
__EXTERN__ rme_ret_t _RME_Kmem_Boot_Crt(struct RME_Cap_Captbl* Captbl, rme_cid_t Cap_Captbl,
                                        rme_cid_t Cap_Kmem, rme_ptr_t Start, rme_ptr_t End, rme_ptr_t Flags);
/* System call handler */
__EXTERN__ void _RME_Svc_Handler(struct RME_Reg_Struct* Reg);
/* Timer interrupt handler */
__EXTERN__ void _RME_Tick_SMP_Handler(struct RME_Reg_Struct* Reg);
__EXTERN__ void _RME_Tick_Handler(struct RME_Reg_Struct* Reg);
/* Debugging helpers */
__EXTERN__ rme_cnt_t RME_Print_Uint(rme_ptr_t Uint);
__EXTERN__ rme_cnt_t RME_Print_Int(rme_cnt_t Int);
__EXTERN__ rme_cnt_t RME_Print_String(rme_s8_t* String);
/*****************************************************************************/
/* Undefine "__EXTERN__" to avoid redefinition */
#undef __EXTERN__
/* __RME_KERNEL_MEMBERS__ */
#endif
/* !(defined __HDR_DEFS__||defined __HDR_STRUCTS__) */
#endif
/* End Public C Function Prototypes ******************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
