/******************************************************************************
Filename    : syssvc.h
Author      : pry
Date        : 25/06/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The header of MMU-based user level low-level library.
******************************************************************************/

/* Defines *******************************************************************/
#ifdef __HDR_DEFS__
#ifndef __SYSSVC_H_DEFS__
#define __SYSSVC_H_DEFS__
/*****************************************************************************/
/* Generic definitions */
#define UVM_TRUE                            1
#define UVM_FALSE                           0
#define UVM_NULL                            0
#define UVM_EXIST                           1
#define UVM_EMPTY                           0

/* Debug string length */
#define UVM_USER_DEBUG_MAX_STR              128
/* Magic number for UVM interrupt header */
#define UVM_INT_MAGIC                      (0x49535953)
/* Magic number for RTD image header */
#define UVM_VM_MAGIC                       (0x56495254)

/* Assertion */
#define UVM_PRINTU_I(INT)                  UVM_Print_Int((INT))
#define UVM_PRINTU_U(UINT)                 UVM_Print_Uint((UINT))
#define UVM_PRINTU_S(STR)                  UVM_Print_String((s8*)(STR))

#define UVM_PRINTU_SIS(STR1,INT,STR2) \
do \
{ \
    UVM_PRINTU_S(STR1); \
    UVM_PRINTU_I(INT); \
    UVM_PRINTU_S(STR2); \
} \
while(0)
    
#define UVM_PRINTU_SUS(STR1,UINT,STR2) \
do \
{ \
    UVM_PRINTU_S(STR1); \
    UVM_PRINTU_U(UINT); \
    UVM_PRINTU_S(STR2); \
} \
while(0)
    
#define UVM_PRINTU_SISUS(STR1,INT,STR2,UINT,STR3) \
do \
{ \
    UVM_PRINTU_S(STR1); \
    UVM_PRINTU_I(INT); \
    UVM_PRINTU_S(STR2); \
    UVM_PRINTU_U(UINT); \
    UVM_PRINTU_S(STR3); \
} \
while(0)
    
#if(UVM_DEBUG_LOG==UVM_TRUE)
#define UVM_LOG_I(INT)                           UVM_PRINTU_I(INT)
#define UVM_LOG_U(UINT)                          UVM_PRINTU_U(UINT)
#define UVM_LOG_S(STR)                           UVM_PRINTU_S(STR)
#define UVM_LOG_SIS(STR1,INT,STR2)               UVM_PRINTU_SIS(STR1,INT,STR2)
#define UVM_LOG_SUS(STR1,UINT,STR2)              UVM_PRINTU_SUS(STR1,UINT,STR2)
#define UVM_LOG_SISUS(STR1,INT,STR2,UINT,STR3)   UVM_PRINTU_SISUS(STR1,INT,STR2,UINT,STR3)
#else
#define UVM_LOG_I(INT)                           while(0)
#define UVM_LOG_U(UINT)                          while(0)
#define UVM_LOG_S(STR)                           while(0)
#define UVM_LOG_SIS(STR1,INT,STR2)               while(0)
#define UVM_LOG_SUS(STR1,UINT,STR2)              while(0)
#define UVM_LOG_SISUS(STR1,INT,STR2,UINT,STR3)   while(0)
#endif

/* Assert macro */
#define UVM_ASSERT(X) \
do \
{ \
    if((X)==0) \
    { \
        UVM_PRINTU_S("\r\n***\r\nUser-level library panic - not syncing:\r\n"); \
        UVM_PRINTU_S(__FILE__); \
        UVM_PRINTU_S(", Line "); \
        UVM_PRINTU_I(__LINE__); \
        UVM_PRINTU_S("\r\n"); \
        UVM_PRINTU_S(__DATE__); \
        UVM_PRINTU_S(", "); \
        UVM_PRINTU_S(__TIME__); \
        UVM_PRINTU_S("\r\n"); \
        while(1); \
    } \
} \
while(0)
/* Word size settings */
#define UVM_WORD_SIZE                       (((ptr_t)1)<<UVM_WORD_ORDER)
#define UVM_WORD_MASK                       (~(((ptr_t)(-1))<<(UVM_WORD_ORDER-1)))
/* Bit mask/address operations */
#define UVM_ALLBITS                         ((ptr_t)(-1))
/* Apply this mask to keep START to MSB bits */
#define UVM_MASK_START(START)               ((UVM_ALLBITS)<<(START))
/* Apply this mask to keep LSB to END bits */
#define UVM_MASK_END(END)                   ((UVM_ALLBITS)>>(UVM_WORD_SIZE-1-(END)))
/* Apply this mask to keep START to END bits, START < END */
#define UVM_MASK(START,END)                 ((UVM_MASK_START(START))&(UVM_MASK_END(END)))
/* Round the number down & up to a power of 2, or get the power of 2 */
#define UVM_ROUND_DOWN(NUM,POW)             ((NUM)&(UVM_MASK_START(POW)))
#define UVM_ROUND_UP(NUM,POW)               UVM_ROUND_DOWN((NUM)+UVM_MASK_END(POW-1),POW)
#define UVM_POW2(POW)                       (((ptr_t)1)<<(POW))

/* System service stub */
#define UVM_CAP_OP(OP,CAPID,ARG1,ARG2,ARG3) UVM_Svc((((ptr_t)(OP))<<(sizeof(ptr_t)*4)|(CAPID)),ARG1,ARG2,ARG3)
#define UVM_PARAM_D_MASK                    ((UVM_ALLBITS)>>(sizeof(ptr_t)*4))
#define UVM_PARAM_Q_MASK                    ((UVM_ALLBITS)>>(sizeof(ptr_t)*6))
#define UVM_PARAM_O_MASK                    ((UVM_ALLBITS)>>(sizeof(ptr_t)*7))
/* The parameter passing - not to be confused with kernel macros. These macros just place the parameters */
#define UVM_PARAM_D1(X)                     (((X)&UVM_PARAM_D_MASK)<<(sizeof(ptr_t)*4))
#define UVM_PARAM_D0(X)                     ((X)&UVM_PARAM_D_MASK)

#define UVM_PARAM_Q3(X)                     (((X)&UVM_PARAM_Q_MASK)<<(sizeof(ptr_t)*6))
#define UVM_PARAM_Q2(X)                     (((X)&UVM_PARAM_Q_MASK)<<(sizeof(ptr_t)*4))
#define UVM_PARAM_Q1(X)                     (((X)&UVM_PARAM_Q_MASK)<<(sizeof(ptr_t)*2))
#define UVM_PARAM_Q0(X)                     ((X)&UVM_PARAM_Q_MASK)

#define UVM_PARAM_O7(X)                     (((X)&UVM_PARAM_O_MASK)<<(sizeof(ptr_t)*7))
#define UVM_PARAM_O6(X)                     (((X)&UVM_PARAM_O_MASK)<<(sizeof(ptr_t)*6))
#define UVM_PARAM_O5(X)                     (((X)&UVM_PARAM_O_MASK)<<(sizeof(ptr_t)*5))
#define UVM_PARAM_O4(X)                     (((X)&UVM_PARAM_O_MASK)<<(sizeof(ptr_t)*4))
#define UVM_PARAM_O3(X)                     (((X)&UVM_PARAM_O_MASK)<<(sizeof(ptr_t)*3))
#define UVM_PARAM_O2(X)                     (((X)&UVM_PARAM_O_MASK)<<(sizeof(ptr_t)*2))
#define UVM_PARAM_O1(X)                     (((X)&UVM_PARAM_O_MASK)<<(sizeof(ptr_t)*1))
#define UVM_PARAM_O0(X)                     ((X)&UVM_PARAM_O_MASK)

/* CID synthesis */
#define UVM_CAPID_NULL                      (((cid_t)1)<<(sizeof(ptr_t)*4-1))
#define UVM_CAPID_2L                        (((cid_t)1)<<(sizeof(ptr_t)*2-1))
#define UVM_CAPID(X,Y)                      (((X)<<(sizeof(ptr_t)*2))|(Y)|UVM_CAPID_2L)

/* Flag synthesis */
/* Kernel function */
#define UVM_KERN_FLAG(HIGH,LOW)             (((HIGH)<<(sizeof(ptr_t)*4))|(LOW))
/* Kernel memory */
#define UVM_KMEM_FLAG(HIGH,LOW)             ((((HIGH)>>(sizeof(ptr_t)*4))<<(sizeof(ptr_t)*4))| \
                                            ((LOW)>>(sizeof(ptr_t)*4)))
#define UVM_KMEM_SVC(HIGH,SVC)              (((((HIGH)>>6)<<(sizeof(ptr_t)*4+6))>>(sizeof(ptr_t)*4))|(SVC))
#define UVM_KMEM_CAPID(LOW,FLAGS)           (((((LOW)>>6)<<(sizeof(ptr_t)*4+6))>>(sizeof(ptr_t)*4))|(FLAGS))
/* Page table */
#define UVM_PGTBL_SVC(NUM_ORDER,SVC)        (((NUM_ORDER)<<(sizeof(ptr_t)<<1))|(SVC))
#define UVM_PGTBL_FLAG(HIGH,LOW,FLAGS)      (((HIGH)<<(sizeof(ptr_t)*4+4))|((LOW)<<8)|(FLAGS))
/* Page table size and number order */
#define UVM_PGTBL(SIZE,NUM)                 (((SIZE)<<(sizeof(ptr_t)<<2))|(NUM))
#define UVM_PGTBL_SIZE(X)                   ((X)>>(sizeof(ptr_t)<<2))
#define UVM_PGTBL_NUM(X)                    ((X)&(UVM_ALLBITS>>(sizeof(ptr_t)<<2)))
/* Thread time delegation */
/* Init thread infinite time marker */
#define UVM_THD_INIT_TIME                   (UVM_ALLBITS>>1)
/* Other thread infinite time marker */
#define UVM_THD_INF_TIME                    (UVM_THD_INIT_TIME-1)
/* Thread time upper limit - always ths infinite time */
#define UVM_THD_MAX_TIME                    (UVM_THD_INF_TIME)
/* Sched rcv return value's fault flag */
#define UVM_THD_FAULT_FLAG                   (((ptr_t)1)<<(sizeof(ptr_t)*8-2))
    
/* Size of kernel objects */
/* Capability table */
#define UVM_CAPTBL_WORD_SIZE(NUM)            (((ptr_t)(NUM))<<3)
/* Process */
#define UVM_PROC_WORD_SIZE                   3
/* Thread */
#define UVM_THD_WORD_SIZE                    314
/* Signal */
#define UVM_SIG_WORD_SIZE                    3
/* Invocation */
#define UVM_INV_WORD_SIZE                    9

/* Rounded size of each object */
#define UVM_ROUNDED(X)                       UVM_ROUND_UP(((ptr_t)(X))*sizeof(ptr_t),UVM_KMEM_SLOT_ORDER)
/* Capability table */
#define UVM_CAPTBL_SIZE(NUM)                 UVM_ROUNDED(UVM_CAPTBL_WORD_SIZE(NUM))
/* Normal page directory */
#define UVM_PGTBL_SIZE_NOM(NUM_ORDER)        UVM_ROUNDED(UVM_PGTBL_WORD_SIZE_NOM(NUM_ORDER))
/* Top-level page directory */
#define UVM_PGTBL_SIZE_TOP(NUM_ORDER)        UVM_ROUNDED(UVM_PGTBL_WORD_SIZE_TOP(NUM_ORDER))
/* Page size*/
#define UVM_PGTBL_WORD_SIZE_NOM(NUM_ORDER)   (1<<(NUM_ORDER))*sizeof(ptr_t)
/* Process */
#define UVM_PROC_SIZE                        UVM_ROUNDED(UVM_PROC_WORD_SIZE)
/* Thread */
#define UVM_THD_SIZE                         UVM_ROUNDED(UVM_THD_WORD_SIZE)
/* Signal */                           
#define UVM_SIG_SIZE                         UVM_ROUNDED(UVM_SIG_WORD_SIZE)
/* Invocation */
#define UVM_INV_SIZE                         UVM_ROUNDED(UVM_INV_WORD_SIZE)

/* The TLS masks */ 
#define UVM_TLS_MASK_128B                    UVM_ROUND_DOWN(UVM_ALLBITS,7)
#define UVM_TLS_MASK_256B                    UVM_ROUND_DOWN(UVM_ALLBITS,8)
#define UVM_TLS_MASK_512B                    UVM_ROUND_DOWN(UVM_ALLBITS,9)
#define UVM_TLS_MASK_1KB                     UVM_ROUND_DOWN(UVM_ALLBITS,10)
#define UVM_TLS_MASK_2KB                     UVM_ROUND_DOWN(UVM_ALLBITS,11)
#define UVM_TLS_MASK_4KB                     UVM_ROUND_DOWN(UVM_ALLBITS,12)
#define UVM_TLS_MASK_8KB                     UVM_ROUND_DOWN(UVM_ALLBITS,13)
#define UVM_TLS_MASK_16KB                    UVM_ROUND_DOWN(UVM_ALLBITS,14)
#define UVM_TLS_MASK_32KB                    UVM_ROUND_DOWN(UVM_ALLBITS,15)
#define UVM_TLS_MASK_64KB                    UVM_ROUND_DOWN(UVM_ALLBITS,16)

/* Initial capability layout - same across all architectures */
/* The capability table of the init process */
#define UVM_BOOT_CAPTBL                      0
/* The top-level page table of the init process - an array */
#define UVM_BOOT_TBL_PGTBL                   1
/* The init process */
#define UVM_BOOT_INIT_PROC                   2
/* The init thread - this is a per-core array */
#define UVM_BOOT_TBL_THD                     3
/* The initial kernel function capability */
#define UVM_BOOT_INIT_KERN                   4
/* The initial kernel memory capability - this is a per-NUMA node array */
#define UVM_BOOT_TBL_KMEM                    5
/* The initial timer endpoint - this is a per-core array */
#define UVM_BOOT_TBL_TIMER                   6
/* The initial default endpoint for all other interrupts - this is a per-core array */
#define UVM_BOOT_TBL_INT                     7

/* Helper capability definitions */
/* The capability to its capability table */
#define UVM_VIRT_CAPTBL(X)                  UVM_CAPID(UVM_VIRT_TBL_CAPPROC,((X)*2))
/* The capability to its process */
#define UVM_VIRT_PROC(X)                    UVM_CAPID(UVM_VIRT_TBL_CAPPROC,((X)*2+1))
/* The capability to its user thread */
#define UVM_VIRT_USERTHD(X)                 UVM_CAPID(UVM_VIRT_TBL_THDSIG,((X)*3))
/* The capability to its interrupt thread */
#define UVM_VIRT_INTTHD(X)                  UVM_CAPID(UVM_VIRT_TBL_THDSIG,((X)*3)+1)
/* The capability to its interrupt thread signal endpoint */
#define UVM_VIRT_INTSIG(X)                  UVM_CAPID(UVM_VIRT_TBL_THDSIG,((X)*3)+2)
/*****************************************************************************/
/* __SYSSVC_H_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Structs *******************************************************************/
#ifdef __HDR_STRUCTS__
#ifndef __SYSSVC_H_STRUCTS__
#define __SYSSVC_H_STRUCTS__
/* We used structs in the header */

/* Use defines in these headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__
/*****************************************************************************/
/* Doubly-linked list */
struct UVM_List
{
    struct UVM_List* Prev;
    struct UVM_List* Next;
};
/*****************************************************************************/
/* __SYSSVC_H_STRUCTS__ */
#endif
/* __HDR_STRUCTS__ */
#endif
/* End Structs ***************************************************************/

/* Private Global Variables **************************************************/
#if(!(defined __HDR_DEFS__||defined __HDR_STRUCTS__))
#ifndef __SYSSVC_MEMBERS__
#define __SYSSVC_MEMBERS__

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
/* Doubly linked list */
__EXTERN__ void UVM_List_Crt(volatile struct UVM_List* Head);
__EXTERN__ void UVM_List_Del(volatile struct UVM_List* Prev,volatile struct UVM_List* Next);
__EXTERN__ void UVM_List_Ins(volatile struct UVM_List* New,
                             volatile struct UVM_List* Prev,
                             volatile struct UVM_List* Next);
/* Thread local storage */
__EXTERN__ void UVM_Set_TLS(ptr_t TLS, ptr_t Mask, ptr_t Offset);
__EXTERN__ ptr_t UVM_Get_TLS(ptr_t Mask, ptr_t Offset);
/* Helper functions */
__EXTERN__ ptr_t UVM_Thd_Stack_Init(ptr_t Stack, ptr_t Size, ptr_t Entry, ptr_t Param1,
                                    ptr_t Param2, ptr_t Param3, ptr_t Param4);
__EXTERN__ ptr_t UVM_Inv_Stack_Init(ptr_t Stack, ptr_t Size, ptr_t Entry);
__EXTERN__ void UVM_Idle(void);
/* Capability table operations */
__EXTERN__ ret_t UVM_Captbl_Crt(cid_t Cap_Captbl_Crt, cid_t Cap_Kmem,
                                cid_t Cap_Captbl, ptr_t Vaddr, ptr_t Entry_Num);
__EXTERN__ ret_t UVM_Captbl_Del(cid_t Cap_Captbl_Del, cid_t Cap_Del);
__EXTERN__ ret_t UVM_Captbl_Frz(cid_t Cap_Captbl_Frz, cid_t Cap_Frz);
__EXTERN__ ret_t UVM_Captbl_Add(cid_t Cap_Captbl_Dst, cid_t Cap_Dst,
                                cid_t Cap_Captbl_Src, cid_t Cap_Src, ptr_t Flags);
__EXTERN__ ret_t UVM_Captbl_Pgtbl(cid_t Cap_Captbl_Dst, cid_t Cap_Dst,
                                  cid_t Cap_Captbl_Src, cid_t Cap_Src,
                                  ptr_t Start, ptr_t End, ptr_t Flags);
__EXTERN__ ret_t UVM_Captbl_Kern(cid_t Cap_Captbl_Dst, cid_t Cap_Dst,
                                 cid_t Cap_Captbl_Src, cid_t Cap_Src,
                                 ptr_t Start, ptr_t End);
__EXTERN__ ret_t UVM_Captbl_Kmem(cid_t Cap_Captbl_Dst, cid_t Cap_Dst,
                                 cid_t Cap_Captbl_Src, cid_t Cap_Src,
                                 ptr_t Start, ptr_t End, ptr_t Flags);
__EXTERN__ ret_t UVM_Captbl_Rem(cid_t Cap_Captbl_Rem, cid_t Cap_Rem);
/* Kernel function operations */
__EXTERN__ ret_t UVM_Kern_Act(cid_t Cap_Kern, ptr_t Func_ID, ptr_t Sub_ID, ptr_t Param1, ptr_t Param2);
/* Page table operations */
__EXTERN__ ret_t UVM_Pgtbl_Crt(cid_t Cap_Captbl, cid_t Cap_Kmem, cid_t Cap_Pgtbl,
                               ptr_t Vaddr, ptr_t Start_Addr, ptr_t Top_Flag,
                               ptr_t Size_Order, ptr_t Num_Order);
__EXTERN__ ret_t UVM_Pgtbl_Del(cid_t Cap_Captbl, cid_t Cap_Pgtbl);
__EXTERN__ ret_t UVM_Pgtbl_Add(cid_t Cap_Pgtbl_Dst, ptr_t Pos_Dst, ptr_t Flags_Dst,
                               cid_t Cap_Pgtbl_Src, ptr_t Pos_Src, ptr_t Index);
__EXTERN__ ret_t UVM_Pgtbl_Rem(cid_t Cap_Pgtbl, ptr_t Pos);
__EXTERN__ ret_t UVM_Pgtbl_Con(cid_t Cap_Pgtbl_Parent, ptr_t Pos, cid_t Cap_Pgtbl_Child, ptr_t Flags_Child);
__EXTERN__ ret_t UVM_Pgtbl_Des(cid_t Cap_Pgtbl, ptr_t Pos);
/* Process operations */
__EXTERN__ ret_t UVM_Proc_Crt(cid_t Cap_Captbl_Crt, cid_t Cap_Kmem, cid_t Cap_Proc,
                              cid_t Cap_Captbl, cid_t Cap_Pgtbl, ptr_t Vaddr);
__EXTERN__ ret_t UVM_Proc_Del(cid_t Cap_Captbl, cid_t Cap_Proc);
__EXTERN__ ret_t UVM_Proc_Cpt(cid_t Cap_Proc, cid_t Cap_Captbl);
__EXTERN__ ret_t UVM_Proc_Pgt(cid_t Cap_Proc, cid_t Cap_Pgtbl);
/* Thread operations */
__EXTERN__ ret_t UVM_Thd_Crt(cid_t Cap_Captbl, cid_t Cap_Kmem, cid_t Cap_Thd,
                             cid_t Cap_Proc, ptr_t Max_Prio, ptr_t Vaddr);
__EXTERN__ ret_t UVM_Thd_Del(cid_t Cap_Captbl, cid_t Cap_Thd);
__EXTERN__ ret_t UVM_Thd_Exec_Set(cid_t Cap_Thd, ptr_t Entry, ptr_t Stack, ptr_t Param);
__EXTERN__ ret_t UVM_Thd_Hyp_Set(cid_t Cap_Thd, ptr_t Kaddr);
__EXTERN__ ret_t UVM_Thd_Sched_Bind(cid_t Cap_Thd, cid_t Cap_Thd_Sched, cid_t Cap_Sig, tid_t TID, ptr_t Prio);
__EXTERN__ ret_t UVM_Thd_Sched_Rcv(cid_t Cap_Thd);
__EXTERN__ ret_t UVM_Thd_Sched_Prio(cid_t Cap_Thd, ptr_t Prio);
__EXTERN__ ret_t UVM_Thd_Sched_Free(cid_t Cap_Thd);
__EXTERN__ ret_t UVM_Thd_Time_Xfer(cid_t Cap_Thd_Dst, cid_t Cap_Thd_Src, ptr_t Time);
__EXTERN__ ret_t UVM_Thd_Swt(cid_t Cap_Thd, ptr_t Full_Yield);
/* Signal operations */
__EXTERN__ ret_t UVM_Sig_Crt(cid_t Cap_Captbl, cid_t Cap_Kmem, cid_t Cap_Sig, ptr_t Vaddr);
__EXTERN__ ret_t UVM_Sig_Del(cid_t Cap_Captbl, cid_t Cap_Sig);
__EXTERN__ ret_t UVM_Sig_Snd(cid_t Cap_Sig,ptr_t Number);
__EXTERN__ ret_t UVM_Sig_Rcv(cid_t Cap_Sig, ptr_t Option);
/* Invocation operations */
__EXTERN__ ret_t UVM_Inv_Crt(cid_t Cap_Captbl, cid_t Cap_Kmem,
                             cid_t Cap_Inv, cid_t Cap_Proc, ptr_t Vaddr);
__EXTERN__ ret_t UVM_Inv_Del(cid_t Cap_Captbl, cid_t Cap_Inv);
__EXTERN__ ret_t UVM_Inv_Set(cid_t Cap_Inv, ptr_t Entry, ptr_t Stack, ptr_t Fault_Ret_Flag);
EXTERN ret_t UVM_Inv_Act(cid_t Cap_Inv, ptr_t Param, ptr_t* Retval);
EXTERN ret_t UVM_Inv_Ret(ptr_t Retval);

/* Debugging helpers */
__EXTERN__ cnt_t UVM_Print_Int(cnt_t Int);
__EXTERN__ cnt_t UVM_Print_Uint(ptr_t Uint);
__EXTERN__ cnt_t UVM_Print_String(s8* String);
/*****************************************************************************/
/* Undefine "__EXTERN__" to avoid redefinition */
#undef __EXTERN__
/* __SYSSVC_MEMBERS__ */
#endif
/* !(defined __HDR_DEFS__||defined __HDR_STRUCTS__) */
#endif
/* End Public C Function Prototypes ******************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
