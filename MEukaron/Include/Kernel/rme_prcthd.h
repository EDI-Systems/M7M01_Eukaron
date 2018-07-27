/******************************************************************************
Filename    : rme_prcthd.h
Author      : pry
Date        : 08/04/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The header of page table.
******************************************************************************/

/* Defines *******************************************************************/
#ifdef __HDR_DEFS__
#ifndef __RME_PRCTHD_H_DEFS__
#define __RME_PRCTHD_H_DEFS__
/*****************************************************************************/
/* Thread states */
/* The thread is currently running */
#define RME_THD_RUNNING            (0)
/* The thread is currently ready for scheduling */
#define RME_THD_READY              (1)
/* The thread is currently blocked on a asynchronous send endpoint */
#define RME_THD_BLOCKED            (2)
/* The thread just ran out of time */
#define RME_THD_TIMEOUT            (3)
/* The thread is blocked on the sched rcv endpoint */
#define RME_THD_SCHED_BLOCKED      (4)
/* The thread is stopped due to a fault */
#define RME_THD_FAULT              (5)

/* Priority level bitmap */
#define RME_PRIO_WORD_NUM          (RME_MAX_PREEMPT_PRIO>>RME_WORD_ORDER)

/* Thread binding state */
#define RME_THD_UNBINDED           ((struct RME_CPU_Local*)((rme_ptr_t)(-1)))
/* Thread sched rcv faulty state */
#define RME_THD_FAULT_FLAG         (((rme_ptr_t)1)<<(sizeof(rme_ptr_t)*8-2))
/* Init thread infinite time marker */
#define RME_THD_INIT_TIME          (((rme_ptr_t)(-1))>>1)
/* Other thread infinite time marker */
#define RME_THD_INF_TIME           (RME_THD_INIT_TIME-1)
/* Thread time upper limit - always ths infinite time */
#define RME_THD_MAX_TIME           (RME_THD_INF_TIME)
/* Get the size of kernel objects */
#define RME_PROC_SIZE              sizeof(struct RME_Proc_Struct)
#define RME_THD_SIZE               sizeof(struct RME_Thd_Struct)
    
/* Time checking macro */
#define RME_TIME_CHECK(DST,AMOUNT) \
do \
{ \
    /* Check if exceeded maximum time or overflowed */ \
    if(RME_UNLIKELY((((DST)+(AMOUNT))>=RME_THD_MAX_TIME)||(((DST)+(AMOUNT))<(DST)))) \
        return RME_ERR_PTH_OVERFLOW; \
} \
while(0);
/*****************************************************************************/
/* __RME_PRCTHD_H_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Structs *******************************************************************/
#ifdef __HDR_STRUCTS__
#ifndef __RME_PRCTHD_H_STRUCTS__
#define __RME_PRCTHD_H_STRUCTS__

/* Use defines in these headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__

/* The list head structure */
struct RME_List
{
    volatile struct RME_List* Prev;
    volatile struct RME_List* Next;
};

/* The per-CPU run queue */
struct RME_Run_Struct
{
    /* The bitmap marking to show if there are active threads at a run level */
    rme_ptr_t Bitmap[RME_PRIO_WORD_NUM];
    /* The actual RME running list */
    struct RME_List List[RME_MAX_PREEMPT_PRIO];
};

/* The process capability structure */
struct RME_Cap_Proc
{
    struct RME_Cap_Head Head;
    rme_ptr_t Info[3];
};

/* The process struct - may contain more data later on */
struct RME_Proc_Struct
{
    /* How many threads/invocation stubs are there in this process? */
    rme_ptr_t Refcnt;
    /* The capability table struct */
    struct RME_Cap_Captbl* Captbl;
    /* The page table struct */
    struct RME_Cap_Pgtbl* Pgtbl;
};

/* The thread capability structure */
struct RME_Cap_Thd
{
    struct RME_Cap_Head Head;
    /* The thread ID of the process */
    rme_ptr_t TID;
    rme_ptr_t Info[2];
};

/* The thread scheduling state structure */
struct RME_Thd_Sched
{
    /* The runnable thread header - This will be inserted into the per-core
     * run queue */
    struct RME_List Run;
    /* The list head for notifications - This will be inserted into scheduler
     * threads' event list */
    struct RME_List Notif; 
    /* What's the TID of the thread? */
    rme_ptr_t TID;
    /* What is the CPU-local data structure that this thread is on? If this is
     * 0xFF....FF, then this is not binded to any core. The compiler should be able
     * to take this even if at this point struct RME_CPU_Local is undefined. */
    struct RME_CPU_Local* CPU_Local;
    /* How much time slices is left for this thread? */
    rme_ptr_t Slices;
    /* What is the current state of the thread? */
    rme_ptr_t State;
    /* How many children refered to it as the scheduler thread? */
    rme_ptr_t Refcnt;
    /* What's the priority of the thread? */
    rme_ptr_t Prio;
    /* What's the maximum priority allowed for this thread? */
    rme_ptr_t Max_Prio;
    /* What signal does this thread block on? */
    struct RME_Sig_Struct* Signal;
    /* Which process is it created in? Reference the process structure */
    struct RME_Proc_Struct* Proc; 
    /* What is its parent thread? Reference the parent structure */
    struct RME_Thd_Struct* Parent;
    /* What is the signal endpoint to send to if we have scheduler notifications? (optional) */
    struct RME_Sig_Struct* Sched_Sig;
    /* The event list for the thread */
    struct RME_List Event;
};

/* The thread register set structure on hypervisor accessible memory */
struct RME_Thd_Regs 
{
    /* The register set - architectural specific */
    struct RME_Reg_Struct Reg;
    /* The co-processor/peripheral context - architecture specific.
     * This usually contains the FPU data */
    struct RME_Cop_Struct Cop_Reg;
};

/* The thread structure */
struct RME_Thd_Struct
{
    /* The thread scheduling struct */
    struct RME_Thd_Sched Sched;
    /* The pointer to current register set */
    struct RME_Thd_Regs* Cur_Reg;
    /* The default register storage area - may be used or not */
    struct RME_Thd_Regs Def_Reg;
    /* The thread synchronous invocation stack */
    struct RME_List Inv_Stack;
};

/* The CPU-local data structure */
struct RME_CPU_Local
{
    /* The CPUID of the CPU */
    rme_ptr_t CPUID;
    /* The current thread on the CPU */
    struct RME_Thd_Struct* Cur_Thd;
    /* The tick timer signal endpoint */
    struct RME_Sig_Struct* Tick_Sig;
    /* The interrupt signal endpoint */
    struct RME_Sig_Struct* Int_Sig;
    /* The runqueue and bitmap */
    struct RME_Run_Struct Run;
};
/*****************************************************************************/

/*****************************************************************************/
/* __RME_PRCTHD_H_STRUCTS__ */
#endif
/* __HDR_STRUCTS__ */
#endif
/* End Structs ***************************************************************/

/* Private Global Variables **************************************************/
#if(!(defined __HDR_DEFS__||defined __HDR_STRUCTS__))
#ifndef __RME_PRCTHD_MEMBERS__
#define __RME_PRCTHD_MEMBERS__

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
/* Initialize per-CPU data structures */
__EXTERN__ void _RME_CPU_Local_Init(struct RME_CPU_Local* CPU_Local, rme_ptr_t CPUID);
/* Linked list operations */
__EXTERN__ void __RME_List_Crt(volatile struct RME_List* Head);
__EXTERN__ void __RME_List_Del(volatile struct RME_List* Prev,
                               volatile struct RME_List* Next);
__EXTERN__ void __RME_List_Ins(volatile struct RME_List* New,
                               volatile struct RME_List* Prev,
                               volatile struct RME_List* Next);
/* Helper functions */
__EXTERN__ rme_ret_t __RME_Thd_Fatal(struct RME_Reg_Struct* Regs);
/* In-kernel ready-queue primitives */
__EXTERN__ rme_ret_t _RME_Run_Ins(struct RME_Thd_Struct* Thd);
__EXTERN__ rme_ret_t _RME_Run_Del(struct RME_Thd_Struct* Thd);
__EXTERN__ struct RME_Thd_Struct* _RME_Run_High(struct RME_CPU_Local* CPU_Local);
__EXTERN__ rme_ret_t _RME_Run_Notif(struct RME_Reg_Struct* Reg, struct RME_Thd_Struct* Thd);
__EXTERN__ rme_ret_t _RME_Run_Swt(struct RME_Reg_Struct* Reg,
                                  struct RME_Thd_Struct* Curr_Thd, 
                                  struct RME_Thd_Struct* Next_Thd);
/* Process system calls */
__EXTERN__ rme_ret_t _RME_Proc_Boot_Crt(struct RME_Cap_Captbl* Captbl, rme_cid_t Cap_Captbl_Crt,
                                        rme_cid_t Cap_Proc, rme_cid_t Cap_Captbl, rme_cid_t Cap_Pgtbl, rme_ptr_t Vaddr);
__EXTERN__ rme_ret_t _RME_Proc_Crt(struct RME_Cap_Captbl* Captbl, rme_cid_t Cap_Captbl_Crt, rme_cid_t Cap_Kmem,
                                   rme_cid_t Cap_Proc, rme_cid_t Cap_Captbl, rme_cid_t Cap_Pgtbl, rme_ptr_t Vaddr);
__EXTERN__ rme_ret_t _RME_Proc_Del(struct RME_Cap_Captbl* Captbl, rme_cid_t Cap_Captbl, rme_cid_t Cap_Proc);
__EXTERN__ rme_ret_t _RME_Proc_Cpt(struct RME_Cap_Captbl* Captbl, rme_cid_t Cap_Proc, rme_cid_t Cap_Captbl);
__EXTERN__ rme_ret_t _RME_Proc_Pgt(struct RME_Cap_Captbl* Captbl, rme_cid_t Cap_Proc, rme_cid_t Cap_Pgtbl);
/* Thread system calls */
__EXTERN__ rme_ret_t _RME_Thd_Boot_Crt(struct RME_Cap_Captbl* Captbl, rme_cid_t Cap_Captbl,
                                       rme_cid_t Cap_Thd, rme_cid_t Cap_Proc, rme_ptr_t Vaddr,
                                       rme_ptr_t Prio, struct RME_CPU_Local* CPU_Local);
__EXTERN__ rme_ret_t _RME_Thd_Crt(struct RME_Cap_Captbl* Captbl, rme_cid_t Cap_Captbl, rme_cid_t Cap_Kmem,
                                  rme_cid_t Cap_Thd, rme_cid_t Cap_Proc, rme_ptr_t Max_Prio, rme_ptr_t Vaddr);
__EXTERN__ rme_ret_t _RME_Thd_Del(struct RME_Cap_Captbl* Captbl, rme_cid_t Cap_Captbl, rme_cid_t Cap_Thd);
__EXTERN__ rme_ret_t _RME_Thd_Exec_Set(struct RME_Cap_Captbl* Captbl,
                                       rme_cid_t Cap_Thd, rme_ptr_t Entry, rme_ptr_t Stack, rme_ptr_t Param);
__EXTERN__ rme_ret_t _RME_Thd_Hyp_Set(struct RME_Cap_Captbl* Captbl, rme_cid_t Cap_Thd, rme_ptr_t Kaddr);
__EXTERN__ rme_ret_t _RME_Thd_Sched_Bind(struct RME_Cap_Captbl* Captbl, rme_cid_t Cap_Thd,
                                         rme_cid_t Cap_Thd_Sched, rme_cid_t Cap_Sig, rme_tid_t TID, rme_ptr_t Prio);
__EXTERN__ rme_ret_t _RME_Thd_Sched_Prio(struct RME_Cap_Captbl* Captbl,
                                         struct RME_Reg_Struct* Reg, rme_cid_t Cap_Thd, rme_ptr_t Prio);
__EXTERN__ rme_ret_t _RME_Thd_Sched_Free(struct RME_Cap_Captbl* Captbl, 
                                         struct RME_Reg_Struct* Reg, rme_cid_t Cap_Thd);
__EXTERN__ rme_ret_t _RME_Thd_Sched_Rcv(struct RME_Cap_Captbl* Captbl, rme_cid_t Cap_Thd);
__EXTERN__ rme_ret_t _RME_Thd_Time_Xfer(struct RME_Cap_Captbl* Captbl, struct RME_Reg_Struct* Reg,
                                        rme_cid_t Cap_Thd_Dst, rme_cid_t Cap_Thd_Src, rme_ptr_t Time);
__EXTERN__ rme_ret_t _RME_Thd_Swt(struct RME_Cap_Captbl* Captbl,
                                  struct RME_Reg_Struct* Reg,
                                  rme_cid_t Cap_Thd, rme_ptr_t Yield);
/*****************************************************************************/
/* Undefine "__EXTERN__" to avoid redefinition */
#undef __EXTERN__
/* __RME_PRCTHD_MEMBERS__ */
#endif
/* !(defined __HDR_DEFS__||defined __HDR_STRUCTS__) */
#endif
/* End Public C Function Prototypes ******************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
