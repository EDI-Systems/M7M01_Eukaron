/******************************************************************************
Filename    : prcthd.c
Author      : pry
Date        : 11/06/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The process and thread management code of RME RTOS.
******************************************************************************/

/* Includes ******************************************************************/
#define __HDR_DEFS__
#include "Platform/RME_platform.h"
#include "Kernel/kernel.h"
#include "Kernel/captbl.h"
#include "Kernel/pgtbl.h"
#include "Kernel/siginv.h"
#include "Kernel/kotbl.h"
#include "Kernel/prcthd.h"
#undef __HDR_DEFS__

#define __HDR_STRUCTS__
#include "Platform/RME_platform.h"
#include "Kernel/captbl.h"
#include "Kernel/kernel.h"
#include "Kernel/pgtbl.h"
#include "Kernel/prcthd.h"
#include "Kernel/siginv.h"
#include "Kernel/kotbl.h"
#undef __HDR_STRUCTS__

/* Private include */
#include "Kernel/prcthd.h"

#define __HDR_PUBLIC_MEMBERS__
#include "Platform/RME_platform.h"
#include "Kernel/kernel.h"
#include "Kernel/captbl.h"
#include "Kernel/pgtbl.h"
#include "Kernel/kotbl.h"
#include "Kernel/siginv.h"
#undef __HDR_PUBLIC_MEMBERS__
/* End Includes **************************************************************/

/* Begin Function:__RME_List_Crt **********************************************
Description : Create a doubly linkled list.
Input       : volatile struct RME_List* Head - The pointer to the list head.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_List_Crt(volatile struct RME_List* Head)
{
    Head->Prev=(struct RME_List*)Head;
    Head->Next=(struct RME_List*)Head;
}
/* End Function:__RME_List_Crt ***********************************************/

/* Begin Function:__RME_List_Del **********************************************
Description : Delete a node from the doubly-linked list.
Input       : volatile struct RME_List* Prev - The prevoius node of the target node.
              volatile struct RME_List* Next - The next node of the target node.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_List_Del(volatile struct RME_List* Prev,volatile struct RME_List* Next)
{
    Next->Prev=(struct RME_List*)Prev;
    Prev->Next=(struct RME_List*)Next;
}
/* End Function:__RME_List_Del ***********************************************/

/* Begin Function:__RME_List_Ins **********************************************
Description : Insert a node to the doubly-linked list.
Input       : volatile struct RME_List* New - The new node to insert.
              volatile struct RME_List* Prev - The previous node.
              volatile struct RME_List* Next - The next node.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_List_Ins(volatile struct RME_List* New,
                    volatile struct RME_List* Prev,
                    volatile struct RME_List* Next)
{
    Next->Prev=(struct RME_List*)New;
    New->Next=(struct RME_List*)Next;
    New->Prev=(struct RME_List*)Prev;
    Prev->Next=(struct RME_List*)New;
}
/* End Function:__RME_List_Ins ***********************************************/

/* Begin Function:__RME_Thd_Fatal *********************************************
Description : The fatal fault handler of RME. This handler will be called by the 
              ISR that handles the faults. This indicates that a fatal fault
              has happenened and we need to see if this thread is in a synchronous
              invocation. If yes, we stop the synchronous invocation immediately,
              and return a fault value to the old register set. If not, we just
              kill the thread. If the thread is killed, a timeout notification will
              be sent to its parent, and if we try to delegate time to it, the time
              delegation will just fail. A thread execution set is required to clear
              the fatal fault status of the thread.
Input       : struct RME_Reg_Struct* Reg - The register set when entering the handler.
Output      : struct RME_Reg_Struct* Reg - The register set when exiting the handler.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t __RME_Thd_Fatal(struct RME_Reg_Struct* Reg)
{
    struct RME_Thd_Struct* Thd;
    ptr_t CPUID;
    
    /* Attempt to return from the invocation, from fault */
    if(_RME_Inv_Ret(Reg, 1)!=0)
    {
        CPUID=RME_CPUID();
        /* Return failure, we are not in an invocation. Kill the thread */
        Thd=RME_Cur_Thd[CPUID];
        /* Are we attempting to kill the init threads? If yes, panic */
        RME_ASSERT(Thd->Sched.Slices!=RME_THD_INIT_TIME);
        Thd->Sched.Slices=0;
        Thd->Sched.State=RME_THD_FAULT;
        _RME_Run_Del(Thd);
        /* Finally, pick up something else to run */
        RME_Cur_Thd[CPUID]=_RME_Run_High(CPUID);
        RME_Cur_Thd[CPUID]->Sched.State=RME_THD_RUNNING;
        /* A solid context switch */
        _RME_Run_Swt(Reg, Thd, RME_Cur_Thd[CPUID]);
    
        /* Send a signal to the fault receive endpoint. This endpoint is per-core */
        _RME_Kern_Snd(Reg, RME_Fault_Sig[CPUID]);
    }
        
    return 0;
}
/* End Function:__RME_Thd_Fatal **********************************************/

/* Begin Function:_RME_Run_Ins ************************************************
Description : Insert a thread into the runqueue. In this function we do not check
              if the thread is on the current core, or is runnable, because it 
              should have been checked by someone else.
Input       : struct RME_Thd_Struct* Thd - The thread to insert.
              ptr_t CPUID - The cpu to consult.
Output      : None.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t _RME_Run_Ins(struct RME_Thd_Struct* Thd)
{
    ptr_t Prio;
    ptr_t CPUID;
    
    Prio=Thd->Sched.Prio;
    CPUID=Thd->Sched.CPUID_Bind;
    
    /* Insert this thread into the runqueue */
    __RME_List_Ins(&(Thd->Sched.Run),RME_Run[CPUID].List[Prio].Prev,&(RME_Run[CPUID].List[Prio]));
    /* Set the bit in the bitmap */
    RME_Run[CPUID].Bitmap[Prio>>RME_WORD_ORDER]|=1<<(Prio&RME_MASK_END(RME_WORD_ORDER-1));
    
    return 0;
}
/* End Function:_RME_Run_Ins *************************************************/

/* Begin Function:_RME_Run_Del ************************************************
Description : Delete a thread from the runqueue.
Input       : struct RME_Thd_Struct* Thd - The thread to delete.
Output      : None.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t _RME_Run_Del(struct RME_Thd_Struct* Thd)
{
    ptr_t Prio;
    ptr_t CPUID;
    
    Prio=Thd->Sched.Prio;
    CPUID=Thd->Sched.CPUID_Bind;
    
    /* Delete this thread from the runqueue */
    __RME_List_Del(Thd->Sched.Run.Prev,Thd->Sched.Run.Next);
    /* __RME_List_Crt(&(Thd->Sched.Run)); */
    
    /* See if there are any thread on this peiority level. If no, clear the bit */
    if(RME_Run[CPUID].List[Prio].Next==&(RME_Run[CPUID].List[Prio]))
        RME_Run[CPUID].Bitmap[Prio>>RME_WORD_ORDER]&=~(1<<(Prio&RME_MASK_END(RME_WORD_ORDER-1)));
    
    return 0;
}
/* End Function:_RME_Run_Del *************************************************/

/* Begin Function:_RME_Run_High ***********************************************
Description : Find the thread with the highest priority on the core.
Input       : ptr_t CPUID - The CPUID of the queue.
Output      : None.
Return      : struct RME_Thd_Struct* - The thread returned.
******************************************************************************/
struct RME_Thd_Struct* _RME_Run_High(ptr_t CPUID)
{
    cnt_t Count;
    ptr_t Prio;
    
    /* We start looking for preemption priority levels from the highest */
    for(Count=RME_PRIO_WORD_NUM-1;Count>=0;Count--)
    {
        if(RME_Run[CPUID].Bitmap[Count]!=0)
            break;
    }
    /* It must be possible to find one thread per core */
    RME_ASSERT(Count>=0);
    /* Get the first "1"'s position in the word */
    Prio=__RME_MSB_Get(RME_Run[CPUID].Bitmap[Count]);
    Prio+=Count<<RME_WORD_ORDER;
    /* Now there is something at this priority level. Get it and start to run */
    return (struct RME_Thd_Struct*)RME_Run[CPUID].List[Prio].Next;
}
/* End Function:_RME_Run_High ************************************************/

/* Begin Function:_RME_Run_Notif **********************************************
Description : Send a notification to the thread's parent, to notify that this 
              thread is currently out of time.
Input       : struct RME_Thd_Struct* Thd - The thread to send notification for.
Output      : None.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t _RME_Run_Notif(struct RME_Thd_Struct* Thd)
{
    /* See if there is already a notification. If yes, do not do the send again */
    if(Thd->Sched.Notif.Next==&(Thd->Sched.Notif))
    {
        __RME_List_Ins(&(Thd->Sched.Notif),
                       Thd->Sched.Parent->Sched.Event.Prev,
                       &(Thd->Sched.Parent->Sched.Event));
    }
    
    return 0;
}
/* End Function:_RME_Run_Notif ***********************************************/

/* Begin Function:_RME_Run_Swt ************************************************
Description : Switch the register set and page table to another thread. 
Input       : struct RME_Reg_Struct* Reg - The current register set.
              struct RME_Thd_Struct* Curr_Thd - The current thread.
              struct RME_Thd_Struct* Next_Thd - The next thread.
Output      : None.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t _RME_Run_Swt(struct RME_Reg_Struct* Reg,
                   struct RME_Thd_Struct* Curr_Thd, 
                   struct RME_Thd_Struct* Next_Thd)
{
    struct RME_Inv_Struct* Curr_Inv_Top;
    struct RME_Cap_Pgtbl* Curr_Pgtbl;
    struct RME_Inv_Struct* Next_Inv_Top;
    struct RME_Cap_Pgtbl* Next_Pgtbl;
    
    /* Save current context */
    __RME_Thd_Reg_Copy(&(Curr_Thd->Cur_Reg->Reg), Reg);
    __RME_Thd_Cop_Save(Reg, &(Curr_Thd->Cur_Reg->Cop_Reg));
    /* Restore next context */
    __RME_Thd_Reg_Copy(Reg, &(Next_Thd->Cur_Reg->Reg));
    __RME_Thd_Cop_Restore(Reg, &(Next_Thd->Cur_Reg->Cop_Reg));

    /* Are we going to switch page tables? If yes, we change it now */
    Curr_Inv_Top=RME_INVSTK_TOP(Curr_Thd);
    Next_Inv_Top=RME_INVSTK_TOP(Next_Thd);
    
    if(Curr_Inv_Top==0)
        Curr_Pgtbl=Curr_Thd->Sched.Proc->Pgtbl;
    else
        Curr_Pgtbl=Curr_Inv_Top->Proc->Pgtbl;
    
    if(Next_Inv_Top==0)
        Next_Pgtbl=Next_Thd->Sched.Proc->Pgtbl;
    else
        Next_Pgtbl=Next_Inv_Top->Proc->Pgtbl;
    
    if(RME_CAP_GETOBJ(Curr_Pgtbl,ptr_t)!=RME_CAP_GETOBJ(Next_Pgtbl,ptr_t))
        __RME_Pgtbl_Set(RME_CAP_GETOBJ(Next_Pgtbl,ptr_t));
    
    return 0;
}
/* End Function:_RME_Run_Swt *************************************************/

/* Begin Function:_RME_Prcthd_Init ********************************************
Description : The system scheduling primitive initialization function.
Input       : None.
Output      : None.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t _RME_Prcthd_Init(void)
{
    cnt_t CPU_Cnt;
    cnt_t Prio_Cnt;
    
    /* Initialize counters */
    RME_TID_Inc=0;
    
    /* Initialize the per-CPU run-queue and bitmap */
    for(CPU_Cnt=0;CPU_Cnt<RME_CPU_NUM;CPU_Cnt++)
    {
        for(Prio_Cnt=0;Prio_Cnt<RME_MAX_PREEMPT_PRIO;Prio_Cnt++)
        {
            RME_Run[CPU_Cnt].Bitmap[Prio_Cnt>>RME_WORD_ORDER]=0;
            __RME_List_Crt(&(RME_Run[CPU_Cnt].List[Prio_Cnt]));
        }
    }
    return 0;
}
/* End Function:_RME_Prcthd_Init *********************************************/

/* Begin Function:_RME_Proc_Boot_Crt ******************************************
Description : Create a process. A process is in fact a protection domain associated
              with a set of capabilities.
              This function will not ask for a kernel memory capability.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              cid_t Cap_Captbl_Crt - The capability to the capability table to use
                                     for this process. 2-Level.
              cid_t Cap_Proc - The capability slot that you want this newly created
                               process capability to be in. 1-Level.
              cid_t Cap_Captbl - The capability to the capability table to use for
                                 this process. 2-Level.
              cid_t Cap_Pgtbl - The capability to the page table to use for this process.
                                2-Level.
              ptr_t Vaddr - The physical address to store the kernel data. This must fall
                            within the kernel virtual address.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t _RME_Proc_Boot_Crt(struct RME_Cap_Captbl* Captbl, cid_t Cap_Captbl_Crt,
                         cid_t Cap_Proc, cid_t Cap_Captbl, cid_t Cap_Pgtbl, ptr_t Vaddr)
{
    struct RME_Cap_Captbl* Captbl_Crt;
    struct RME_Cap_Captbl* Captbl_Op;
    struct RME_Cap_Pgtbl* Pgtbl_Op;
    struct RME_Cap_Proc* Proc_Crt;
    struct RME_Proc_Struct* Proc_Struct;
    ptr_t Type_Ref;
    
    /* Get the capability slots */
    RME_CAPTBL_GETCAP(Captbl,Cap_Captbl_Crt,RME_CAP_CAPTBL,struct RME_Cap_Captbl*,Captbl_Crt);
    RME_CAPTBL_GETCAP(Captbl,Cap_Captbl,RME_CAP_CAPTBL,struct RME_Cap_Captbl*,Captbl_Op);
    RME_CAPTBL_GETCAP(Captbl,Cap_Pgtbl,RME_CAP_PGTBL,struct RME_Cap_Pgtbl*,Pgtbl_Op);
    /* Check if the captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Captbl_Crt,RME_CAPTBL_FLAG_CRT);
    RME_CAP_CHECK(Captbl_Op,RME_CAPTBL_FLAG_PROC_CRT);
    RME_CAP_CHECK(Pgtbl_Op,RME_PGTBL_FLAG_PROC_CRT);
    
    /* Get the cap slot */
    RME_CAPTBL_GETSLOT(Captbl_Crt,Cap_Proc,struct RME_Cap_Proc*,Proc_Crt);
    /* Take the slot if possible */
    RME_CAPTBL_OCCUPY(Proc_Crt,Type_Ref);
    
    /* Try to populate the area */
    if(_RME_Kotbl_Mark(Vaddr, RME_PROC_SIZE)!=0)
    {
        Proc_Crt->Head.Type_Ref=0;
        return RME_ERR_CAP_KOTBL;
    }
    
    Proc_Crt->Head.Parent=0;
    Proc_Crt->Head.Object=Vaddr;
    /* Does not allow changing page tables and capability tables for it */
    Proc_Crt->Head.Flags=RME_PROC_FLAG_INV|RME_PROC_FLAG_THD;
    Proc_Struct=((struct RME_Proc_Struct*)Vaddr);
    /* Reference it to make the process undeletable */
    Proc_Struct->Refcnt=1;
    /* Set the capability table, reference it and check for overflow */
    Proc_Struct->Captbl=Captbl_Op;
    Type_Ref=__RME_Fetch_Add(&(Captbl_Op->Head.Type_Ref), 1);
    if(RME_CAP_REF(Type_Ref)>=RME_CAP_MAXREF)
    {
        __RME_Fetch_Add(&(Captbl_Op->Head.Type_Ref), -1);
        RME_ASSERT(_RME_Kotbl_Erase(Vaddr, RME_PROC_SIZE)==0);
        Proc_Crt->Head.Type_Ref=0;
        return RME_ERR_CAP_REFCNT;
    }
    /* Set the page table, reference it and check for overflow */
    Proc_Struct->Pgtbl=Pgtbl_Op;
    Type_Ref=__RME_Fetch_Add(&(Pgtbl_Op->Head.Type_Ref), 1);
    if(RME_CAP_REF(Type_Ref)>=RME_CAP_MAXREF)
    {
        __RME_Fetch_Add(&(Captbl_Op->Head.Type_Ref), -1);
        __RME_Fetch_Add(&(Pgtbl_Op->Head.Type_Ref), -1);
        RME_ASSERT(_RME_Kotbl_Erase(Vaddr, RME_PROC_SIZE)==0);
        Proc_Crt->Head.Type_Ref=0;
        return RME_ERR_CAP_REFCNT;
    }
    
    /* Creation complete */
    Proc_Crt->Head.Type_Ref=RME_CAP_TYPEREF(RME_CAP_PROC,0);
    
    return 0;
}
/* End Function:_RME_Proc_Boot_Crt *******************************************/

/* Begin Function:_RME_Proc_Crt ***********************************************
Description : Create a process. A process is in fact a protection domain associated
              with a set of capabilities.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              cid_t Cap_Captbl_Crt - The capability to the capability table to place
                                     this process capability in. 2-Level.
              cid_t Cap_Kmem - The kernel memory capability. 2-Level.
              cid_t Cap_Proc - The capability slot that you want this newly created
                               process capability to be in. 1-Level.
              cid_t Cap_Captbl - The capability to the capability table to use for
                                 this process. 2-Level.
              cid_t Cap_Pgtbl - The capability to the page table to use for this process.
                                2-Level.
              ptr_t Vaddr - The physical address to store the kernel data. This must fall
                            within the kernel virtual address.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t _RME_Proc_Crt(struct RME_Cap_Captbl* Captbl, cid_t Cap_Captbl_Crt, cid_t Cap_Kmem,
                    cid_t Cap_Proc, cid_t Cap_Captbl, cid_t Cap_Pgtbl, ptr_t Vaddr)
{
    struct RME_Cap_Captbl* Captbl_Crt;
    struct RME_Cap_Captbl* Captbl_Op;
    struct RME_Cap_Pgtbl* Pgtbl_Op;
    struct RME_Cap_Kmem* Kmem_Op;
    struct RME_Cap_Proc* Proc_Crt;
    struct RME_Proc_Struct* Proc_Struct;
    ptr_t Type_Ref;
    
    /* Get the capability slots */
    RME_CAPTBL_GETCAP(Captbl,Cap_Captbl_Crt,RME_CAP_CAPTBL,struct RME_Cap_Captbl*,Captbl_Crt);
    RME_CAPTBL_GETCAP(Captbl,Cap_Captbl,RME_CAP_CAPTBL,struct RME_Cap_Captbl*,Captbl_Op);
    RME_CAPTBL_GETCAP(Captbl,Cap_Pgtbl,RME_CAP_PGTBL,struct RME_Cap_Pgtbl*,Pgtbl_Op);
    RME_CAPTBL_GETCAP(Captbl,Cap_Kmem,RME_CAP_KMEM,struct RME_Cap_Kmem*,Kmem_Op);
    /* Check if the captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Captbl_Crt,RME_CAPTBL_FLAG_CRT);
    RME_CAP_CHECK(Captbl_Op,RME_CAPTBL_FLAG_PROC_CRT);
    RME_CAP_CHECK(Pgtbl_Op,RME_PGTBL_FLAG_PROC_CRT);
    /* See if the creation is valid for this kmem range */
    RME_KMEM_CHECK(Kmem_Op,RME_KMEM_FLAG_PROC,Vaddr,RME_PROC_SIZE);
    
    /* Get the cap slot */
    RME_CAPTBL_GETSLOT(Captbl_Crt,Cap_Proc,struct RME_Cap_Proc*,Proc_Crt);
    /* Take the slot if possible */
    RME_CAPTBL_OCCUPY(Proc_Crt,Type_Ref);
    
    /* Try to populate the area */
    if(_RME_Kotbl_Mark(Vaddr, RME_PROC_SIZE)!=0)
    {
        Proc_Crt->Head.Type_Ref=0;
        return RME_ERR_CAP_KOTBL;
    }
    
    Proc_Crt->Head.Parent=0;
    Proc_Crt->Head.Object=Vaddr;
    Proc_Crt->Head.Flags=RME_PROC_FLAG_INV|RME_PROC_FLAG_THD|
                         RME_PROC_FLAG_CPT|RME_PROC_FLAG_PGT;
    Proc_Struct=((struct RME_Proc_Struct*)Vaddr);
    /* Set the capability table, reference it and check for overflow */
    Proc_Struct->Captbl=Captbl_Op;
    Proc_Struct->Refcnt=0;
    Type_Ref=__RME_Fetch_Add(&(Captbl_Op->Head.Type_Ref), 1);
    if(RME_CAP_REF(Type_Ref)>=RME_CAP_MAXREF)
    {
        __RME_Fetch_Add(&(Captbl_Op->Head.Type_Ref), -1);
        RME_ASSERT(_RME_Kotbl_Erase(Vaddr, RME_PROC_SIZE)==0);
        Proc_Crt->Head.Type_Ref=0;
        return RME_ERR_CAP_REFCNT;
    }
    /* Set the page table, reference it and check for overflow */
    Proc_Struct->Pgtbl=Pgtbl_Op;
    Type_Ref=__RME_Fetch_Add(&(Pgtbl_Op->Head.Type_Ref), 1);
    if(RME_CAP_REF(Type_Ref)>=RME_CAP_MAXREF)
    {
        __RME_Fetch_Add(&(Captbl_Op->Head.Type_Ref), -1);
        __RME_Fetch_Add(&(Pgtbl_Op->Head.Type_Ref), -1);
        RME_ASSERT(_RME_Kotbl_Erase(Vaddr, RME_PROC_SIZE)==0);
        Proc_Crt->Head.Type_Ref=0;
        return RME_ERR_CAP_REFCNT;
    }
    
    /* Creation complete */
    Proc_Crt->Head.Type_Ref=RME_CAP_TYPEREF(RME_CAP_PROC,0);
    
    return 0;
}
/* End Function:_RME_Proc_Crt ************************************************/

/* Begin Function:_RME_Proc_Del ***********************************************
Description : Delete a process.
Input       : struct RME_Cap_Captbl* Captbl - The capability table.
              cid_t Cap_Captbl - The capability to the capability table. 2-Level.
              cid_t Cap_Proc - The capability to the process. 1-Level.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t _RME_Proc_Del(struct RME_Cap_Captbl* Captbl, cid_t Cap_Captbl, cid_t Cap_Proc)
{
    struct RME_Cap_Captbl* Captbl_Op;
    struct RME_Cap_Proc* Proc_Del;
    ptr_t Type_Ref;

    /* Used for deletion */
    struct RME_Proc_Struct* Object;
    
    /* Get the capability slot */
    RME_CAPTBL_GETCAP(Captbl,Cap_Captbl,RME_CAP_CAPTBL,struct RME_Cap_Captbl*,Captbl_Op);    
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Captbl_Op,RME_CAPTBL_FLAG_DEL);
    
    /* Get the cap slot */
    RME_CAPTBL_GETSLOT(Captbl_Op,Cap_Proc,struct RME_Cap_Proc*,Proc_Del);
    /* Delete check */
    RME_CAP_DEL_CHECK(Proc_Del,Type_Ref,RME_CAP_PROC);
    
    /* Remember the object location for deletion */
    Object=RME_CAP_GETOBJ(Proc_Del,struct RME_Proc_Struct*);
    
    /* See if the object is referenced by another thread or invocation kernel
     * object. If yes, cannot delete */
     if(Object->Refcnt!=0)
     {
         RME_CAP_DEFROST(Proc_Del,Type_Ref);
         return RME_ERR_PTH_REFCNT;
     }
    
    /* Now we can safely delete the cap */
    RME_CAP_REMDEL(Proc_Del,Type_Ref);
    
    /* Decrease the refcnt for the two caps */
    __RME_Fetch_Add(&(Object->Captbl->Head.Type_Ref), -1);
    __RME_Fetch_Add(&(Object->Pgtbl->Head.Type_Ref), -1);
        
    /* Try to depopulate the area - this must be successful */
    RME_ASSERT(_RME_Kotbl_Erase((ptr_t)Object, RME_PROC_SIZE)!=0);
    
    return 0;
}
/* End Function:_RME_Proc_Del ************************************************/

/* Begin Function:_RME_Proc_Cpt ***********************************************
Description : Change a process's capability table.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              cid_t Cap_Proc - The capability to the process that have been created
                               already. 2-Level.
              cid_t Cap_Captbl - The capability to the capability table to use for
                                 this process. 2-Level.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t _RME_Proc_Cpt(struct RME_Cap_Captbl* Captbl, cid_t Cap_Proc, cid_t Cap_Captbl)
{
    struct RME_Cap_Proc* Proc_Op;
    struct RME_Cap_Captbl* Captbl_New;
    struct RME_Cap_Captbl* Captbl_Old;
    struct RME_Proc_Struct* Proc_Struct;
    ptr_t Type_Ref;
    
    /* Get the capability slot */
    RME_CAPTBL_GETCAP(Captbl,Cap_Proc,RME_CAP_PROC,struct RME_Cap_Proc*,Proc_Op); 
    RME_CAPTBL_GETCAP(Captbl,Cap_Captbl,RME_CAP_CAPTBL,struct RME_Cap_Captbl*,Captbl_New);     
    /* Check if the target caps is not frozen and allows such operations */
    RME_CAP_CHECK(Proc_Op,RME_PROC_FLAG_CPT);
    RME_CAP_CHECK(Captbl_New,RME_CAPTBL_FLAG_PROC_CPT);
    
    /* Increase the reference count of the new cap first - If that fails, we can revert easily */
    Type_Ref=__RME_Fetch_Add(&(Captbl_New->Head.Type_Ref), 1);
    if(RME_CAP_REF(Type_Ref)>=RME_CAP_MAXREF)
    {
        __RME_Fetch_Add(&(Captbl_New->Head.Type_Ref), -1);
        return RME_ERR_CAP_REFCNT;
    }
    
    /* Read the old captbl, and do CAS here. If we fail, revert the refcnt */
    Proc_Struct=RME_CAP_GETOBJ(Proc_Op,struct RME_Proc_Struct*);
    Captbl_Old=Proc_Struct->Captbl;
    /* Actually commit the change */
    if(__RME_Comp_Swap((ptr_t*)(&(Proc_Struct->Captbl)),
                              (ptr_t*)(&Captbl_Old),
                              (ptr_t)Captbl_New)==0)
    {
        __RME_Fetch_Add(&(Captbl_New->Head.Type_Ref), -1);
        return RME_ERR_PTH_CONFLICT;
    }
    /* Release the old table */
    __RME_Fetch_Add(&(Captbl_Old->Head.Type_Ref), -1);
    
    return 0;
}
/* End Function:_RME_Proc_Cpt ************************************************/

/* Begin Function:_RME_Proc_Pgt ***********************************************
Description : Change a process's page table.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              cid_t Cap_Proc - The capability slot that you want this newly created
                               process capability to be in. 2-Level.
              cid_t Cap_Pgtbl - The capability to the page table to use for this
                                process. 2-Level.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t _RME_Proc_Pgt(struct RME_Cap_Captbl* Captbl, cid_t Cap_Proc, cid_t Cap_Pgtbl)
{
    struct RME_Cap_Proc* Proc_Op;
    struct RME_Cap_Pgtbl* Pgtbl_New;
    struct RME_Cap_Pgtbl* Pgtbl_Old;
    struct RME_Proc_Struct* Proc_Struct;
    ptr_t Type_Ref;
    
    /* Get the capability slot */
    RME_CAPTBL_GETCAP(Captbl,Cap_Proc,RME_CAP_PROC,struct RME_Cap_Proc*,Proc_Op); 
    RME_CAPTBL_GETCAP(Captbl,Cap_Pgtbl,RME_CAP_PGTBL,struct RME_Cap_Pgtbl*,Pgtbl_New);     
    /* Check if the target caps is not frozen and allows such operations */
    RME_CAP_CHECK(Proc_Op,RME_PROC_FLAG_PGT);
    RME_CAP_CHECK(Pgtbl_New,RME_PGTBL_FLAG_PROC_PGT);
    
    /* Increase the reference count of the new cap first - If that fails, we can revert easily */
    Type_Ref=__RME_Fetch_Add(&(Pgtbl_New->Head.Type_Ref), 1);
    if(RME_CAP_REF(Type_Ref)>=RME_CAP_MAXREF)
    {
        __RME_Fetch_Add(&(Pgtbl_New->Head.Type_Ref), -1);
        return RME_ERR_CAP_REFCNT;
    }
    
    /* Read the old captbl, and do CAS here. If we fail, revert the refcnt */
    Proc_Struct=RME_CAP_GETOBJ(Proc_Op,struct RME_Proc_Struct*);
    Pgtbl_Old=Proc_Struct->Pgtbl;
    /* Actually commit the change */
    if(__RME_Comp_Swap((ptr_t*)(&(Proc_Struct->Captbl)),
                              (ptr_t*)(&Pgtbl_Old),
                              (ptr_t)Pgtbl_New)==0)
    {
        __RME_Fetch_Add(&(Pgtbl_New->Head.Type_Ref), -1);
        return RME_ERR_PTH_CONFLICT;
    }
    /* Release the old table */
    __RME_Fetch_Add(&(Pgtbl_Old->Head.Type_Ref), -1);
    
    return 0;
}
/* End Function:_RME_Proc_Pgt ************************************************/

/* Begin Function:_RME_Thd_Boot_Crt *******************************************
Description : Create a boot-time thread. The boot-time thread is per-core, and
              will have infinite budget, and has no parent. This function
              allows creation of a thread on behalf of other processors,
              by passing a CPUID parameter. Therefore, it is recommended to
              create the threads sequentially during boot-up; if you create threads
              in parallel, be sure to only create the thread on your local core.
              This function will not ask for a kernel memory capability, and 
              the initial threads' maximum priority will be set by the system.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              cid_t Cap_Captbl - The capability to the capability table. 2-Level.
              cid_t Cap_Thd - The capability slot that you want this newly created
                              thread capability to be in. 1-Level.
              cid_t Cap_Proc - The capability to the process that it is in. 2-Level.
              ptr_t Vaddr - The physical address to store the kernel object.
              ptr_t Prio - The priority level of the thread.
              ptr_t CPUID - The CPU to bind this thread to.
Output      : None.
Return      : ret_t - If successful, the Thread ID; or an error code.
******************************************************************************/
ret_t _RME_Thd_Boot_Crt(struct RME_Cap_Captbl* Captbl, cid_t Cap_Captbl, cid_t Cap_Thd,
		                cid_t Cap_Proc, ptr_t Vaddr, ptr_t Prio, ptr_t CPUID)
{
    struct RME_Cap_Captbl* Captbl_Op;
    struct RME_Cap_Proc* Proc_Op;
    struct RME_Cap_Thd* Thd_Crt;
    struct RME_Thd_Struct* Thd_Struct;
    ptr_t Type_Ref;
    
    /* Check whether the priority level is allowed */
    if(Prio>=RME_MAX_PREEMPT_PRIO)
        return RME_ERR_PTH_PRIO;
    
    /* Get the capability slots */
    RME_CAPTBL_GETCAP(Captbl,Cap_Captbl,RME_CAP_CAPTBL,struct RME_Cap_Captbl*,Captbl_Op); 
    RME_CAPTBL_GETCAP(Captbl,Cap_Proc,RME_CAP_PROC,struct RME_Cap_Proc*,Proc_Op);   
    /* Check if the target caps is not frozen and allows such operations */
    RME_CAP_CHECK(Captbl_Op,RME_CAPTBL_FLAG_CRT);
    RME_CAP_CHECK(Proc_Op,RME_PROC_FLAG_THD);
    
    /* Get the cap slot */
    RME_CAPTBL_GETSLOT(Captbl_Op,Cap_Thd,struct RME_Cap_Thd*,Thd_Crt);
    /* Take the slot if possible */
    RME_CAPTBL_OCCUPY(Thd_Crt,Type_Ref);
     
    /* Try to populate the area */
    if(_RME_Kotbl_Mark(Vaddr, RME_THD_SIZE)!=0)
    {
        Thd_Crt->Head.Type_Ref=0;
        return RME_ERR_CAP_KOTBL;
    }
    
    /* Get the thread, and start creation */
    Thd_Struct=(struct RME_Thd_Struct*)Vaddr;
    Thd_Struct->Sched.TID=__RME_Fetch_Add(&RME_TID_Inc, 1);
    /* Set this initially to 1 to make it virtually unfreeable & undeletable */
    Thd_Struct->Sched.Refcnt=1;
    Thd_Struct->Sched.Slices=RME_THD_INIT_TIME;
    Thd_Struct->Sched.State=RME_THD_RUNNING;
    Thd_Struct->Sched.Signal=0;
    Thd_Struct->Sched.Prio=Prio;
    Thd_Struct->Sched.Max_Prio=RME_MAX_PREEMPT_PRIO-1;
    /* Bind the thread to the current CPU */
    Thd_Struct->Sched.CPUID_Bind=CPUID;
    /* This is a marking that this thread haven't sent any notifications */
    __RME_List_Crt(&(Thd_Struct->Sched.Notif));
    __RME_List_Crt(&(Thd_Struct->Sched.Event));
    /* RME_List_Crt(&(Thd_Struct->Sched.Run)); */
    Thd_Struct->Sched.Proc=RME_CAP_GETOBJ(Proc_Op,struct RME_Proc_Struct*);
    /* Point its pointer to itself - this will never be a hypervisor thread */
    Thd_Struct->Cur_Reg=&(Thd_Struct->Def_Reg);
    /* Initialize the invocation stack */
    __RME_List_Crt(&(Thd_Struct->Inv_Stack));
    
    /* Increase the reference count of the process structure(Not the process capability) */
    __RME_Fetch_Add(&(RME_CAP_GETOBJ(Proc_Op, struct RME_Proc_Struct*)->Refcnt), 1);
    
    /* Set the cap's parameters according to what we have just created */
    Thd_Crt->Head.Parent=0;
    Thd_Crt->Head.Object=Vaddr;
    /* This can only be a parent, and not a child, and cannot be freed. Additionally,
     * this should not be blocked on any endpoint. Any attempt to block this thread will fail.
     * Setting execution information for this is also prohibited. */
    Thd_Crt->Head.Flags=RME_THD_FLAG_SCHED_PRIO|RME_THD_FLAG_SCHED_PARENT|
                        RME_THD_FLAG_XFER_DST|RME_THD_FLAG_XFER_SRC|
                        RME_THD_FLAG_SCHED_RCV|RME_THD_FLAG_SWT;
    Thd_Crt->TID=Thd_Struct->Sched.TID;
    
    /* Insert this into the runqueue, and set current thread to it */
    _RME_Run_Ins(Thd_Struct);
    RME_Cur_Thd[Thd_Struct->Sched.CPUID_Bind]=Thd_Struct;
    
    /* Creation complete */
    Thd_Crt->Head.Type_Ref=RME_CAP_TYPEREF(RME_CAP_THD,0);
    
    return Thd_Crt->TID;
}
/* End Function:_RME_Thd_Boot_Crt ********************************************/

/* Begin Function:_RME_Thd_Crt ************************************************
Description : Create a thread. A thread is the minimal kernel-aware execution unit.
              When the thread is created, there are no time associated with it.
              This can be called on any core.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              cid_t Cap_Captbl - The capability to the capability table. 2-Level.
              cid_t Cap_Kmem - The kernel memory capability. 2-Level.
              cid_t Cap_Thd - The capability slot that you want this newly created
                              thread capability to be in. 1-Level.
              cid_t Cap_Proc - The capability to the process that it is in. 2-Level.
              ptr_t Max_Prio - The maximum priority allowed for this thread. Once set,
                               this cannot be changed.
              ptr_t Vaddr - The physical address to store the kernel object.
Output      : None.
Return      : ret_t - If successful, the Thread ID; or an error code.
******************************************************************************/
ret_t _RME_Thd_Crt(struct RME_Cap_Captbl* Captbl, cid_t Cap_Captbl, cid_t Cap_Kmem,
                   cid_t Cap_Thd, cid_t Cap_Proc, ptr_t Max_Prio, ptr_t Vaddr)
{
    struct RME_Cap_Captbl* Captbl_Op;
    struct RME_Cap_Proc* Proc_Op;
    struct RME_Cap_Kmem* Kmem_Op;
    struct RME_Cap_Thd* Thd_Crt;
    struct RME_Thd_Struct* Thd_Struct;
    ptr_t Type_Ref;
    
    /* See if the maximum priority relationship is correct - a thread can never create
     * a thread with higher maximum priority */
    if(RME_Cur_Thd[RME_CPUID()]->Sched.Max_Prio<Max_Prio)
        return RME_ERR_PTH_PRIO;

    /* Get the capability slots */
    RME_CAPTBL_GETCAP(Captbl,Cap_Captbl,RME_CAP_CAPTBL,struct RME_Cap_Captbl*,Captbl_Op); 
    RME_CAPTBL_GETCAP(Captbl,Cap_Proc,RME_CAP_PROC,struct RME_Cap_Proc*,Proc_Op);   
    RME_CAPTBL_GETCAP(Captbl,Cap_Kmem,RME_CAP_KMEM,struct RME_Cap_Kmem*,Kmem_Op);
    /* Check if the target caps is not frozen and allows such operations */
    RME_CAP_CHECK(Captbl_Op,RME_CAPTBL_FLAG_CRT);
    RME_CAP_CHECK(Proc_Op,RME_PROC_FLAG_THD);
    /* See if the creation is valid for this kmem range */
    RME_KMEM_CHECK(Kmem_Op,RME_KMEM_FLAG_THD,Vaddr,RME_THD_SIZE);
    
    /* Get the cap slot */
    RME_CAPTBL_GETSLOT(Captbl_Op,Cap_Thd,struct RME_Cap_Thd*,Thd_Crt);
    /* Take the slot if possible */
    RME_CAPTBL_OCCUPY(Thd_Crt,Type_Ref);
     
    /* Try to populate the area */
    if(_RME_Kotbl_Mark(Vaddr, RME_THD_SIZE)!=0)
    {
        Thd_Crt->Head.Type_Ref=0;
        return RME_ERR_CAP_KOTBL;
    }

    /* Get the thread, and start creation */
    Thd_Struct=(struct RME_Thd_Struct*)Vaddr;
    Thd_Struct->Sched.TID=__RME_Fetch_Add(&RME_TID_Inc, 1);
    Thd_Struct->Sched.Refcnt=0;
    Thd_Struct->Sched.Slices=0;
    Thd_Struct->Sched.State=RME_THD_TIMEOUT;
    Thd_Struct->Sched.Signal=0;
    Thd_Struct->Sched.Max_Prio=Max_Prio;
    /* Currently the thread is not binded to any particular CPU */
    Thd_Struct->Sched.CPUID_Bind=RME_THD_UNBIND;
    /* This is a marking that this thread haven't sent any notifications */
    __RME_List_Crt(&(Thd_Struct->Sched.Notif));
    __RME_List_Crt(&(Thd_Struct->Sched.Event));
    /* RME_List_Crt(&(Thd_Struct->Sched.Run)); */
    Thd_Struct->Sched.Proc=RME_CAP_GETOBJ(Proc_Op,struct RME_Proc_Struct*);
    /* Point its pointer to itself - this is not a hypervisor thread yet */
    Thd_Struct->Cur_Reg=&(Thd_Struct->Def_Reg);
    /* Initialize the invocation stack */
    __RME_List_Crt(&(Thd_Struct->Inv_Stack));
    
    /* Increase the reference count of the process structure(Not the process capability) */
    __RME_Fetch_Add(&(RME_CAP_GETOBJ(Proc_Op, struct RME_Proc_Struct*)->Refcnt), 1);
    
    /* Set the cap's parameters according to what we have just created */
    Thd_Crt->Head.Parent=0;
    Thd_Crt->Head.Object=Vaddr;
    Thd_Crt->Head.Flags=RME_THD_FLAG_EXEC_SET|RME_THD_FLAG_HYP_SET|
                        RME_THD_FLAG_SCHED_CHILD|RME_THD_FLAG_SCHED_PARENT|
                        RME_THD_FLAG_SCHED_PRIO|RME_THD_FLAG_SCHED_FREE|
                        RME_THD_FLAG_SCHED_RCV|RME_THD_FLAG_SWT|
                        RME_THD_FLAG_XFER_SRC|RME_THD_FLAG_XFER_DST;
    Thd_Crt->TID=Thd_Struct->Sched.TID;
    
    /* Creation complete */
    Thd_Crt->Head.Type_Ref=RME_CAP_TYPEREF(RME_CAP_THD,0);

    return Thd_Crt->TID;
}
/* End Function:_RME_Thd_Crt *************************************************/

/* Begin Function:_RME_Thd_Del ************************************************
Description : Delete a thread. This can be called on any core.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table. 
              cid_t Cap_Captbl - The capability to the capability table. 2-Level.
              cid_t Cap_Thd - The capability to the thread in the captbl. 1-Level.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t _RME_Thd_Del(struct RME_Cap_Captbl* Captbl, cid_t Cap_Captbl, cid_t Cap_Thd)
{
    struct RME_Cap_Captbl* Captbl_Op;
    struct RME_Cap_Thd* Thd_Del;
    ptr_t Type_Ref;
    /* These are for deletion */
    struct RME_Thd_Struct* Thd_Struct;
    struct RME_Inv_Struct* Inv_Struct;
    
    /* Get the capability slot */
    RME_CAPTBL_GETCAP(Captbl,Cap_Captbl,RME_CAP_CAPTBL,struct RME_Cap_Captbl*,Captbl_Op);    
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Captbl_Op,RME_CAPTBL_FLAG_DEL);
    
    /* Get the cap slot */
    RME_CAPTBL_GETSLOT(Captbl_Op,Cap_Thd,struct RME_Cap_Thd*,Thd_Del);
    /* Delete check */
    RME_CAP_DEL_CHECK(Thd_Del,Type_Ref,RME_CAP_THD);
    
    /* Get the thread */
    Thd_Struct=RME_CAP_GETOBJ(Thd_Del,struct RME_Thd_Struct*);
    
    /* See if the thread is unbinded. If not, we cannot proceed to deletion */
    if(Thd_Struct->Sched.CPUID_Bind!=RME_THD_UNBIND)
    {
        RME_CAP_DEFROST(Thd_Del,Type_Ref);
        return RME_ERR_PTH_INVSTATE;
    }
    
    /* Now we can safely delete the cap */
    RME_CAP_REMDEL(Thd_Del,Type_Ref);
    
    /* Is the thread using any invocation? If yes, just pop the invocation
     * stack to empty, and free all the invocation stubs. This can be virtually
     * unbounded if the invocation stack is just too deep. This is left to the
     * user; if this is what he or she wants, be our guest. */
    while(Thd_Struct->Inv_Stack.Next!=&(Thd_Struct->Inv_Stack))
    {
        Inv_Struct=(struct RME_Inv_Struct*)(Thd_Struct->Inv_Stack.Next);
        __RME_List_Del(Inv_Struct->Head.Prev,Inv_Struct->Head.Next);
        Inv_Struct->Active=0;
    }
    
    /* Dereference the process */
    __RME_Fetch_Add(&(Thd_Struct->Sched.Proc->Refcnt), -1);
    
    /* Try to depopulate the area - this must be successful */
    RME_ASSERT(_RME_Kotbl_Erase((ptr_t)Thd_Struct,RME_THD_SIZE)!=0);
    
    return 0;
}
/* End Function:_RME_Thd_Del *************************************************/

/* Begin Function:_RME_Thd_Exec_Set *******************************************
Description : Set a thread's entry point and stack. The registers will be initialized
              with these contents. Only when the thread has exited, or just after
              created should we change these values.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              cid_t Cap_Thd - The capability to the thread. 2-Level.
              ptr_t Entry - The entry of the thread. An address.
              ptr_t Stack - The stack address to use for execution. An address.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t _RME_Thd_Exec_Set(struct RME_Cap_Captbl* Captbl, cid_t Cap_Thd, ptr_t Entry, ptr_t Stack)
{
    struct RME_Cap_Thd* Thd_Op;
    struct RME_Thd_Struct* Thd_Struct;
    
    /* Get the capability slot */
    RME_CAPTBL_GETCAP(Captbl,Cap_Thd,RME_CAP_THD,struct RME_Cap_Thd*,Thd_Op);
    /* Check if the target cap is not frozen and allows such operations */
    RME_CAP_CHECK(Thd_Op,RME_THD_FLAG_EXEC_SET);
    
    /* See if the target thread is already binded. If no or incorrect, we just quit */
    Thd_Struct=RME_CAP_GETOBJ(Thd_Op,struct RME_Thd_Struct*);
    if(Thd_Struct->Sched.CPUID_Bind!=RME_CPUID())
        return RME_ERR_PTH_INVSTATE;
    
    /* See if there is a fault pending. If yes, we clear it */
    if(Thd_Struct->Sched.State==RME_THD_FAULT)
        Thd_Struct->Sched.State=RME_THD_TIMEOUT;
    
    /* Commit the change if both values are non-zero. If both are zero we are just
     * clearing the error flag and continue execution from where it faulted */
    if((Entry!=0)&&(Stack!=0))
    {
        __RME_Thd_Reg_Init(Entry, Stack, &(Thd_Struct->Cur_Reg->Reg));
        __RME_Thd_Cop_Init(Entry, Stack, &(Thd_Struct->Cur_Reg->Cop_Reg));
    }
    
    return 0;
}
/* End Function:_RME_Thd_Exec_Set ********************************************/

/* Begin Function:_RME_Thd_Hyp_Set ********************************************
Description : Set the thread as hypervisor-managed. This means that the thread's
              register set will be saved to somewhere that is indicated by the user,
              instead of in the kernel data structures.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              cid_t Cap_Thd - The capability to the thread. 2-Level.
              ptr_t Kaddr - The kernel-accessible virtual address to save the register set to.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t _RME_Thd_Hyp_Set(struct RME_Cap_Captbl* Captbl, cid_t Cap_Thd, ptr_t Kaddr)
{
    struct RME_Cap_Thd* Thd_Op;
    struct RME_Thd_Struct* Thd_Struct;
    
    /* Get the capability slot */
    RME_CAPTBL_GETCAP(Captbl,Cap_Thd,RME_CAP_THD,struct RME_Cap_Thd*,Thd_Op);
    /* Check if the target cap is not frozen and allows such operations */
    RME_CAP_CHECK(Thd_Op,RME_THD_FLAG_HYP_SET);
    
    /* See if the target thread is already binded. If no or incorrect, we just quit */
    Thd_Struct=RME_CAP_GETOBJ(Thd_Op,struct RME_Thd_Struct*);
    if(Thd_Struct->Sched.CPUID_Bind!=RME_CPUID())
        return RME_ERR_PTH_INVSTATE;
    
    /* Set the thread's register storage back to default if the address passed in is null */
    if(Kaddr==0)
        Thd_Struct->Cur_Reg=&(Thd_Struct->Def_Reg);
    else
    {
        /* Register external save area must be aligned to word boundary and accessible to the kernel */
        if(RME_IS_ALIGNED(Kaddr)&&(Kaddr>=RME_HYP_VA_START)&&
           ((Kaddr+sizeof(struct RME_Thd_Regs))<(RME_HYP_VA_START+RME_HYP_SIZE)))
            Thd_Struct->Cur_Reg=(struct RME_Thd_Regs*)Kaddr;
        else
            return RME_ERR_PTH_PGTBL;
    }
    
    return 0;
}
/* End Function:_RME_Thd_Hyp_Set *********************************************/

/* Begin Function:_RME_Thd_Sched_Bind *****************************************
Description : Set a thread's priority level, and its scheduler thread. When there
              is any state change on this thread, a notification will be sent to
              scheduler thread. If the state of the thread changes for multiple
              times, then only the most recent state will be in the scheduler's 
              receive queue.
              The scheduler and the threads that it schedule must be on the same
              core. When a thread wants to go from one core to another, its notification
              to the scheduler must all be processed, and it must have no scheduler
              notifications in itself. 
              This must be called on the same core with the Cap_Thd_Sched, and the
              Cap_Thd itself must be free.
              It is impossible to set a thread's priority beyond its maximum priority. 
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              cid_t Cap_Thd - The capability to the thread. 2-Level.
              cid_t Cap_Thd_Sched - The scheduler thread. 2-Level.
              ptr_t Prio - The priority level, higher is more critical.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t _RME_Thd_Sched_Bind(struct RME_Cap_Captbl* Captbl, 
                          cid_t Cap_Thd, cid_t Cap_Thd_Sched, ptr_t Prio)
{
    struct RME_Cap_Thd* Thd_Op;
    struct RME_Cap_Thd* Thd_Sched;
    struct RME_Thd_Struct* Thd_Op_Struct;
    struct RME_Thd_Struct* Thd_Sched_Struct;
    ptr_t Old_CPUID;
    ptr_t CPUID;
    
    /* Get the capability slot */
    RME_CAPTBL_GETCAP(Captbl,Cap_Thd,RME_CAP_THD,struct RME_Cap_Thd*,Thd_Op);
    RME_CAPTBL_GETCAP(Captbl,Cap_Thd_Sched,RME_CAP_THD,struct RME_Cap_Thd*,Thd_Sched);
    /* Check if the target cap is not frozen and allows such operations */
    RME_CAP_CHECK(Thd_Op,RME_THD_FLAG_SCHED_CHILD);
    RME_CAP_CHECK(Thd_Sched,RME_THD_FLAG_SCHED_PARENT);
    
    /* See if the target thread is already binded. If yes, we just quit */
    Thd_Op_Struct=RME_CAP_GETOBJ(Thd_Op,struct RME_Thd_Struct*);
    Old_CPUID=Thd_Op_Struct->Sched.CPUID_Bind;
    if((Old_CPUID&RME_THD_UNBIND)==0)
        return RME_ERR_PTH_INVSTATE;
    
    /* See if the parent thread is on the same core with the current processor */
    CPUID=RME_CPUID();
    Thd_Sched_Struct=RME_CAP_GETOBJ(Thd_Sched,struct RME_Thd_Struct*);
    if(Thd_Sched_Struct->Sched.CPUID_Bind!=CPUID)
        return RME_ERR_PTH_INVSTATE;

    /* See if we are trying to bind to ourself. This is prohibited */
    if(Thd_Op_Struct==Thd_Sched_Struct)
        return RME_ERR_PTH_NOTIF;
    
    /* See if the priority relationship is correct */
    if(Thd_Sched_Struct->Sched.Max_Prio<Prio)
        return RME_ERR_PTH_PRIO;
    
    /* Yes, it is on the current processor. Try to bind the thread */
    if(__RME_Comp_Swap(&(Thd_Op_Struct->Sched.CPUID_Bind), &Old_CPUID, CPUID)==0)
        return RME_ERR_PTH_CONFLICT;
    
    /* Binding successful. Do operations to finish this. There's no need to worry about
     * other cores' operations on this thread because this thread is already binded
     * to this core */
    Thd_Op_Struct->Sched.Parent=Thd_Sched_Struct;
    Thd_Op_Struct->Sched.Prio=Prio;
    Thd_Sched_Struct->Sched.Refcnt++;
    
    return 0;
}
/* End Function:_RME_Thd_Sched_Bind ******************************************/

/* Begin Function:_RME_Thd_Sched_Prio *****************************************
Description : Change a thread's priority level. This can only be called from the
              core that have the thread binded.
              This system call can cause a potential context switch.
              It is impossible to set a thread's priority beyond its maximum priority. 
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              struct RME_Reg_Struct* Reg - The current register set.
              cid_t Cap_Thd - The capability to the thread. 2-Level.
              ptr_t Prio - The priority level, higher is more critical.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t _RME_Thd_Sched_Prio(struct RME_Cap_Captbl* Captbl,
                          struct RME_Reg_Struct* Reg,
                          cid_t Cap_Thd, ptr_t Prio)
{
    struct RME_Cap_Thd* Thd_Op;
    struct RME_Thd_Struct* Thd_Struct;
    ptr_t CPUID;
    
    /* Get the capability slot */
    RME_CAPTBL_GETCAP(Captbl,Cap_Thd,RME_CAP_THD,struct RME_Cap_Thd*,Thd_Op);
    /* Check if the target cap is not frozen and allows such operations */
    RME_CAP_CHECK(Thd_Op,RME_THD_FLAG_SCHED_PRIO);
    
    /* See if the target thread is already binded to this core. If no, we just quit */
    CPUID=RME_CPUID();
    Thd_Struct=(struct RME_Thd_Struct*)Thd_Op->Head.Object;
    if(Thd_Struct->Sched.CPUID_Bind!=CPUID)
        return RME_ERR_PTH_INVSTATE;
    
    /* See if the priority relationship is correct */
    if(Thd_Struct->Sched.Max_Prio<Prio)
        return RME_ERR_PTH_PRIO;
    
    /* Now save the system call return value to the caller stack */
    __RME_Set_Syscall_Retval(Reg,0);
    
    /* See if this thread is currently running, or is runnable. If yes, it must be
     * in the run queue. Remove it from there and change priority, after changing
     * priority, put it back, and see if we need a reschedule. */
    if((Thd_Struct->Sched.State==RME_THD_RUNNING)||(Thd_Struct->Sched.State==RME_THD_READY))
    {
        _RME_Run_Del(Thd_Struct);
        Thd_Struct->Sched.Prio=Prio;
        _RME_Run_Ins(Thd_Struct);
        
        /* Get the current highest-priority running thread */
        Thd_Struct=_RME_Run_High(CPUID);
        /* See if we need a context seitch */
        if(Thd_Struct!=RME_Cur_Thd[CPUID])
        {
            /* This will cause a solid context switch - The current thread will be set to ready,
             * and we will set the thread that we switch to to be running. */
            _RME_Run_Swt(Reg,RME_Cur_Thd[CPUID],Thd_Struct);
            RME_Cur_Thd[CPUID]->Sched.State=RME_THD_READY;
            Thd_Struct->Sched.State=RME_THD_RUNNING;
            RME_Cur_Thd[CPUID]=Thd_Struct;
        }
    }
    else
        Thd_Struct->Sched.Prio=Prio;
    
    return 0;
}
/* End Function:_RME_Thd_Sched_Prio ******************************************/

/* Begin Function:_RME_Thd_Sched_Free *****************************************
Description : Free a thread from its current binding. This function can only be
              executed from the same core on with the thread.
              This system call can cause a potential context switch.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              struct RME_Reg_Struct* Reg - The current register set.
              cid_t Cap_Thd - The capability to the thread. 2-Level.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t _RME_Thd_Sched_Free(struct RME_Cap_Captbl* Captbl, 
                          struct RME_Reg_Struct* Reg, cid_t Cap_Thd)
{
    struct RME_Cap_Thd* Thd_Op;
    struct RME_Thd_Struct* Thd_Struct;
    /* These are used to free the thread */
    ptr_t CPUID;
    
    /* Get the capability slot */
    RME_CAPTBL_GETCAP(Captbl,Cap_Thd,RME_CAP_THD,struct RME_Cap_Thd*,Thd_Op);
    /* Check if the target cap is not frozen and allows such operations */
    RME_CAP_CHECK(Thd_Op,RME_THD_FLAG_SCHED_FREE);
    
    /* See if the target thread is already binded. If no or binded to other cores, we just quit */
    Thd_Struct=(struct RME_Thd_Struct*)Thd_Op->Head.Object;
    if((Thd_Struct->Sched.CPUID_Bind&RME_THD_UNBIND)!=0)
        return RME_ERR_PTH_INVSTATE;
    if(Thd_Struct->Sched.CPUID_Bind!=RME_CPUID())
        return RME_ERR_PTH_INVSTATE;
    
    /* Am I referenced by someone as a scheduler? If yes, we cannot unbind. Because
     * boot-time thread's refcnt will never be 0, thus they will never pass this checking */
    if(Thd_Struct->Sched.Refcnt!=0)
        return RME_ERR_PTH_REFCNT;
    
    /* Decrease the parent's reference count */
    Thd_Struct->Sched.Parent->Sched.Refcnt--;
    
    /* See if we have any events sent to the parent. If yes, remove that event */
    if(Thd_Struct->Sched.Notif.Next!=&(Thd_Struct->Sched.Notif))
    {
        __RME_List_Del(Thd_Struct->Sched.Notif.Prev,Thd_Struct->Sched.Notif.Next);
        __RME_List_Crt(&(Thd_Struct->Sched.Notif));
    }
    
    /* Now save the system call return value to the caller stack */
    __RME_Set_Syscall_Retval(Reg,0);  
    
    /* If the thread is running, or ready to run, kick it out of the run queue.
     * If it is blocked on some endpoint, end the blocking and set the return
     * value to RME_ERR_SIV_FREE. If the thread is killed due to a fault, we will
     * not clear the fault here */
    if(Thd_Struct->Sched.State!=RME_THD_BLOCKED)
    {
        if((Thd_Struct->Sched.State==RME_THD_RUNNING)||(Thd_Struct->Sched.State==RME_THD_READY))
        {
            _RME_Run_Del(Thd_Struct);
            Thd_Struct->Sched.State=RME_THD_TIMEOUT;
        }
    }
    else
    {
        /* If it got here, the thread that is operated on cannot be the current thread, so
         * we are not overwriting the return value of the caller thread */
        __RME_Set_Syscall_Retval(&(Thd_Struct->Cur_Reg->Reg),RME_ERR_SIV_FREE);
        Thd_Struct->Sched.Signal->Thd=0;
        Thd_Struct->Sched.Signal=0;
        Thd_Struct->Sched.State=RME_THD_TIMEOUT;
    }
    /* Delete all slices on it */
    Thd_Struct->Sched.Slices=0;
    
    CPUID=RME_CPUID();
    /* See if this thread is the current thread. If yes, then there will be a context switch */
    if(RME_Cur_Thd[CPUID]==Thd_Struct)
    {
        RME_Cur_Thd[CPUID]=_RME_Run_High(CPUID);
        _RME_Run_Ins(RME_Cur_Thd[CPUID]);
        RME_Cur_Thd[CPUID]->Sched.State=RME_THD_RUNNING;
        _RME_Run_Swt(Reg,Thd_Struct,RME_Cur_Thd[CPUID]);
    }
    
    /* Set the state to unbinded so other cores can bind */
    Thd_Struct->Sched.CPUID_Bind=RME_THD_UNBIND;
    return 0;
}
/* End Function:_RME_Thd_Sched_Free ******************************************/

/* Begin Function:_RME_Thd_Sched_Rcv ******************************************
Description : Try to receive a notification from the scheduler queue. This
              can only be called from the same core the thread is on.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              cid_t Cap_Thd - The capability to the scheduler thread. We are going
                              to get timeout notifications for the threads that
                              it is responsible for scheduling. This capability must
                              point to the calling thread itself, or the receiving
                              is simply not allowed. 2-Level.
Output      : None.
Return      : ret_t - If successful, the thread ID; or an error code.
******************************************************************************/
ret_t _RME_Thd_Sched_Rcv(struct RME_Cap_Captbl* Captbl, cid_t Cap_Thd)
{
    struct RME_Cap_Thd* Thd_Op;
    struct RME_Thd_Struct* Thd_Struct;
    struct RME_Thd_Struct* Thd_Child;
    
    /* Get the capability slot */
    RME_CAPTBL_GETCAP(Captbl,Cap_Thd,RME_CAP_THD,struct RME_Cap_Thd*,Thd_Op);
    /* Check if the target cap is not frozen and allows such operations */
    RME_CAP_CHECK(Thd_Op,RME_THD_FLAG_SCHED_RCV);
    
    /* Check if the CPUID is correct. Only if yes can we proceed */
    Thd_Struct=(struct RME_Thd_Struct*)Thd_Op->Head.Object;
    if(Thd_Struct->Sched.CPUID_Bind!=RME_CPUID())
        return RME_ERR_PTH_INVSTATE;
    
    /* Are there any notifications? */
    if(Thd_Struct->Sched.Event.Next==&(Thd_Struct->Sched.Event))
    {
        /* Check the blocking flag to see whether we need to block the thread */
        return RME_ERR_PTH_NOTIF;
    }
    
    /* Return one notification and delete it from the notification list */
    Thd_Child=(struct RME_Thd_Struct*)(Thd_Struct->Sched.Event.Next-1);
    __RME_List_Del(Thd_Child->Sched.Notif.Prev,Thd_Child->Sched.Notif.Next);
    /* We need to do this because we are using this to detect whether the notification is sent */
    __RME_List_Crt(&(Thd_Child->Sched.Notif));
    
    /* See if the child is in a faulty state. If yes, we return a fault notification with that TID */
    if(Thd_Child->Sched.State==RME_THD_FAULT)
        return Thd_Child->Sched.TID|RME_THD_FAULT_FLAG;

    /* Return the notification TID */
    return Thd_Child->Sched.TID;
}
/* End Function:_RME_Thd_Sched_Rcv *******************************************/

/* Begin Function:_RME_Thd_Time_Xfer ******************************************
Description : Transfer time from one thread to another. This can only be called
              from the core that the thread is on, and the the two threads involved
              in the time transfer must be on the same core.
              If the time transfered is more than or equal to what the source have,
              the source will be out of time or blocked. If the source is both out of
              time and blocked, we do not send the notification; Instead, we send the
              notification when the receive endpoint actually receive something.
              It is possible to transfer time to threads have a lower priority, and it
              is also possible to transfer time to threads that have a higher priority.
              In the latter case, and if the source is currently running, a preemption
              will directly occur.
              There are 3 kinds of threads in the system:
              1. Init threads - They are created at boot-time and have infinite budget.
              2. Infinite threads - They are created later but have infinite budget.
              3. Normal threads - They are created later and have a finite budget.
              There are 3 kinds of transfer in the system:
              1. Normal transfers - They transfer a finite budget.
              2. Infinite transfers - They attempt to transfer an infinite budget but will
                 not revoke the timeslices of the source if the source have infinite budget.
              3. Revoking transfers - They attempt to transfer an infinite budget but will
                 revoke the timeslices of the source if the source is an infinite thread.
              Normal budget transferring rules:
              -----------------------------------------------------------------
                Nom   |   From   |     Init     |   Infinite   |    Normal
              -----------------------------------------------------------------
                 To   |   Init   |      --      |      --      |      T-
              -----------------------------------------------------------------
                      | Infinite |      --      |      --      |      T-
              -----------------------------------------------------------------
                      |  Normal  |      -A      |      TA      |      TA
              -----------------------------------------------------------------
              Infinite budget transferring rules:
              -----------------------------------------------------------------
                Nom   |   From   |     Init     |   Infinite   |    Normal
              -----------------------------------------------------------------
                 To   |   Init   |      --      |      --      |      S-
              -----------------------------------------------------------------
                      | Infinite |      --      |      --      |      S-
              -----------------------------------------------------------------
                      |  Normal  |      -I      |      -I      |      TA
              -----------------------------------------------------------------
              Revoking budget transferring rules:
              -----------------------------------------------------------------
                Rev   |   From   |     Init     |   Infinite   |    Normal
              -----------------------------------------------------------------
                 To   |   Init   |      --      |      S-      |      S-
              -----------------------------------------------------------------
                      | Infinite |      --      |      S-      |      S-
              -----------------------------------------------------------------
                      |  Normal  |      -I      |      SI      |      TA
              -----------------------------------------------------------------
              Notations:
              -:Nothing will happen on source/destination.
              T:Source timeout if all transferred.
              S:Source will definitely timeout.
              A:Destination accept if not overflow.
              I:Destination will definitely become an infinite thread.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              struct RME_Reg_Struct* Reg - The current register set.
              cid_t Cap_Thd_Dst - The destination thread. 2-Level.
              cid_t Cap_Thd_Src - The source thread. 2-Level.
              ptr_t Time - The time to transfer, in slices, for normal transfers.
                           A slice is the minimal amount of time transfered in the
                           system usually on the order of 100us or 1ms.
                           Use RME_THD_INIT_TIME for revoking transfer.
                           Use RME_THD_INF_TIME for infinite trasnfer.
Output      : None.
Return      : ret_t - If successful, the destination time amount; or an error code.
******************************************************************************/
ret_t _RME_Thd_Time_Xfer(struct RME_Cap_Captbl* Captbl, struct RME_Reg_Struct* Reg,
                         cid_t Cap_Thd_Dst, cid_t Cap_Thd_Src, ptr_t Time)
{
    struct RME_Cap_Thd* Thd_Dst;
    struct RME_Cap_Thd* Thd_Src;
    struct RME_Thd_Struct* Thd_Dst_Struct;
    struct RME_Thd_Struct* Thd_Src_Struct;
    ptr_t CPUID;
    ptr_t Time_Xfer;
    
    /* We may allow transferring infinite time here */
    if(Time==0)
        return RME_ERR_PTH_INVSTATE;
    
    /* Get the capability slot */
    RME_CAPTBL_GETCAP(Captbl,Cap_Thd_Dst,RME_CAP_THD,struct RME_Cap_Thd*,Thd_Dst);
    RME_CAPTBL_GETCAP(Captbl,Cap_Thd_Src,RME_CAP_THD,struct RME_Cap_Thd*,Thd_Src);
    /* Check if the target cap is not frozen and allows such operations */
    RME_CAP_CHECK(Thd_Dst,RME_THD_FLAG_XFER_DST);
    RME_CAP_CHECK(Thd_Src,RME_THD_FLAG_XFER_SRC);

    /* Check if the two threads are on the core that is accordance with what we are on */
    CPUID=RME_CPUID();
    Thd_Src_Struct=RME_CAP_GETOBJ(Thd_Src,struct RME_Thd_Struct*);
    if(Thd_Src_Struct->Sched.CPUID_Bind!=CPUID)
        return RME_ERR_PTH_INVSTATE;
    /* Do we have slices to transfer? - slices == 0 implies TIMEOUT, or BLOCKED */
    if(Thd_Src_Struct->Sched.Slices==0)
        return RME_ERR_PTH_INVSTATE;
    Thd_Dst_Struct=RME_CAP_GETOBJ(Thd_Dst,struct RME_Thd_Struct*);
    if(Thd_Dst_Struct->Sched.CPUID_Bind!=CPUID)
        return RME_ERR_PTH_INVSTATE;
    /* See if the destination is in a fault. If yes, cancel the transfer */
    if(Thd_Dst_Struct->Sched.State==RME_THD_FAULT)
        return RME_ERR_PTH_FAULT;
    
    /* Delegating from a normal thread */
    if(Thd_Src_Struct->Sched.Slices<RME_THD_INF_TIME)
    {
        /* Delegate all our time */
        if(Time>=RME_THD_INF_TIME)
            Time_Xfer=Thd_Src_Struct->Sched.Slices;
        /* Delegate some time, if not sufficient, clean up the source time */
        else
        {
            if(Thd_Src_Struct->Sched.Slices>Time)
                Time_Xfer=Time;
            else
                Time_Xfer=Thd_Src_Struct->Sched.Slices;
        }
        
        /* See if we are transferring to an infinite budget thread. If yes, we
         * are revoking timeslices; If not, this is a finite transfer */
        if(Thd_Dst_Struct->Sched.Slices<RME_THD_INF_TIME)
        {
            RME_TIME_CHECK(Thd_Dst_Struct->Sched.Slices,Time_Xfer);
            Thd_Dst_Struct->Sched.Slices+=Time_Xfer;
        }
        
        Thd_Src_Struct->Sched.Slices-=Time_Xfer;
    }
    /* Delegating from init or infinite thread */
    else
    {
        /* Infinite transfer to the destination */
        if(Time>=RME_THD_INF_TIME)
        {
            /* This transfer will revoke the infinite budget */
            if(Time==RME_THD_INIT_TIME)
            {
                /* Will not revoke, source is an init thread */
                if(Thd_Src_Struct->Sched.Slices!=RME_THD_INIT_TIME)
                    Thd_Src_Struct->Sched.Slices=0;
            }
            /* Set destination to infinite if it is not an init thread */
            if(Thd_Dst_Struct->Sched.Slices<RME_THD_INF_TIME)
                Thd_Dst_Struct->Sched.Slices=RME_THD_INF_TIME;
        }
        else
        {
            /* Just increase the budget of the other thread - check first */
            RME_TIME_CHECK(Thd_Dst_Struct->Sched.Slices,Time);
            Thd_Dst_Struct->Sched.Slices+=Time;
        }
    }
    
    /* Is the source time used up? If yes, delete it from the run queue, and notify its 
     * parent. If it is not in the run queue, The state of the source must be BLOCKED. We
     * notify its parent when we are waking it up in the future, so do nothing here */
    if(Thd_Src_Struct->Sched.Slices==0)
    {
        if((Thd_Src_Struct->Sched.State==RME_THD_RUNNING)||(Thd_Src_Struct->Sched.State==RME_THD_READY))
        {
            _RME_Run_Del(Thd_Src_Struct);
            Thd_Src_Struct->Sched.State=RME_THD_TIMEOUT;
            /* Notify the parent about this */
            _RME_Run_Notif(Thd_Src_Struct);
        }
    }
    
    /* Now save the system call return value to the caller stack - how much time the destination have now */
    __RME_Set_Syscall_Retval(Reg,Thd_Dst_Struct->Sched.Slices);  
    
    /* See what was the state of the destination thread. If it is timeout, then
     * activate it. If it is other state, then leave it alone */
    if(Thd_Dst_Struct->Sched.State==RME_THD_TIMEOUT)
    {
        Thd_Dst_Struct->Sched.State=RME_THD_READY;
        _RME_Run_Ins(Thd_Dst_Struct);
    }
    
    /* See we are timeout because we did this delegation(If the current thread
     * is timeout, it is sure that it became timeout in this function). It is not
     * possible that the current thread be BLOCKED here */
    if(RME_Cur_Thd[CPUID]->Sched.State==RME_THD_TIMEOUT)
    {
        Thd_Dst_Struct=_RME_Run_High(CPUID);
        _RME_Run_Swt(Reg, RME_Cur_Thd[CPUID], Thd_Dst_Struct);
        Thd_Dst_Struct->Sched.State=RME_THD_RUNNING;
        RME_Cur_Thd[CPUID]=Thd_Dst_Struct;
    }
    /* See if the delegated thread have a higher priority and is ready, thus it
     * will preempt us */
    else if((Thd_Dst_Struct->Sched.State==RME_THD_READY)&&
            (Thd_Dst_Struct->Sched.Prio>RME_Cur_Thd[CPUID]->Sched.Prio))
    {
        _RME_Run_Swt(Reg, RME_Cur_Thd[CPUID], Thd_Dst_Struct);
        Thd_Dst_Struct->Sched.State=RME_THD_RUNNING;
        RME_Cur_Thd[CPUID]->Sched.State=RME_THD_READY;
        RME_Cur_Thd[CPUID]=Thd_Dst_Struct;
    }
    
    return 0;
}
/* End Function:_RME_Thd_Time_Xfer *******************************************/

/* Begin Function:_RME_Thd_Swt ************************************************
Description : Switch to another thread. The thread to switch to must have the same
              preemptive priority as this thread, and have time, and not blocked.
              If trying to switch to a higher priority thread, this is impossible
              because whenever a thread with higher priority exists in the system,
              the kernel wiull let it preempt the current thread. 
              If trying to switch to a lower priority thread, this is impossible
              because the current thread just preempts it after the thread switch.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table. 
              struct RME_Reg_Struct* Reg - The register set structure.
              cid_t Cap_Thd - The capability to the thread. 2-Level. If this is
                              -1, the kernel will pickup whatever thread that have
                              the highest priority and have time to run. 
              ptr_t Full_Yield - This is a flag to indicate whether this is a full yield.
                                 If it is, the kernel will kill all the time allocated for 
                                 this thread.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t _RME_Thd_Swt(struct RME_Cap_Captbl* Captbl, struct RME_Reg_Struct* Reg,
                   cid_t Cap_Thd, ptr_t Full_Yield)
{
    struct RME_Cap_Thd* Next_Thd_Cap;
    struct RME_Thd_Struct* Next_Thd;
    ptr_t CPUID;
    
    /* See if the scheduler is given the right to pick a thread to run */
    CPUID=RME_CPUID();                                                   
    if(Cap_Thd!=RME_THD_ARBITRARY)
    {
        RME_CAPTBL_GETCAP(Captbl,Cap_Thd,RME_CAP_THD,struct RME_Cap_Thd*,Next_Thd_Cap);
        /* Check if the target cap is not frozen and allows such operations */
        RME_CAP_CHECK(Next_Thd_Cap,RME_THD_FLAG_SWT);
        /* See if we can do operation on this core */
        Next_Thd=RME_CAP_GETOBJ(Next_Thd_Cap, struct RME_Thd_Struct*);
        if(Next_Thd->Sched.CPUID_Bind!=CPUID)
            return RME_ERR_PTH_INVSTATE;
        /* See if we can yield to the thread */
        if(RME_Cur_Thd[CPUID]->Sched.Prio!=Next_Thd->Sched.Prio)
            return RME_ERR_PTH_PRIO;
        /* See if the state will allow us to do this */
        if((Next_Thd->Sched.State==RME_THD_BLOCKED)||
           (Next_Thd->Sched.State==RME_THD_TIMEOUT))
            return RME_ERR_PTH_INVSTATE;
        /* See if the target is in a faulty state */
        if(Next_Thd->Sched.State==RME_THD_FAULT)
            return RME_ERR_PTH_FAULT;
        
        /* See if we need to give up all our timeslices in this yield */
        if((Full_Yield!=0)&&(RME_Cur_Thd[CPUID]->Sched.Slices!=RME_THD_INIT_TIME))
        {
            _RME_Run_Del(RME_Cur_Thd[CPUID]);
            RME_Cur_Thd[CPUID]->Sched.Slices=0;
            RME_Cur_Thd[CPUID]->Sched.State=RME_THD_TIMEOUT;
            /* Notify the parent about this */
            _RME_Run_Notif(RME_Cur_Thd[CPUID]);
            /* See if it is the current thread. If yes, we choose another guy */
            if(RME_Cur_Thd[CPUID]==Next_Thd)
                Next_Thd=_RME_Run_High(CPUID);
        }
        else
            RME_Cur_Thd[CPUID]->Sched.State=RME_THD_READY;
    }
    else
    {
        /* See if we need to give up all our timeslices in this yield */
        if((Full_Yield!=0)&&(RME_Cur_Thd[CPUID]->Sched.Slices!=RME_THD_INIT_TIME))
        {
            _RME_Run_Del(RME_Cur_Thd[CPUID]);
            RME_Cur_Thd[CPUID]->Sched.Slices=0;
            RME_Cur_Thd[CPUID]->Sched.State=RME_THD_TIMEOUT;
            /* Notify the parent about this */
            _RME_Run_Notif(RME_Cur_Thd[CPUID]);
        }
        else
        {
            /* This operation is just to make sure that there are any other thread
             * at the same priviledge level, we're not switching to ourself */
            _RME_Run_Del(RME_Cur_Thd[CPUID]);
            _RME_Run_Ins(RME_Cur_Thd[CPUID]);
            RME_Cur_Thd[CPUID]->Sched.State=RME_THD_READY;
        }
        Next_Thd=_RME_Run_High(CPUID);
    }
    
    /* Now save the system call return value to the caller stack */
    __RME_Set_Syscall_Retval(Reg,0);
    
    /* Set the next thread's state first */
    Next_Thd->Sched.State=RME_THD_RUNNING;
    /* Are we switching to ourself? If yes, skip all the next operations */
    if(RME_Cur_Thd[CPUID]==Next_Thd)
        return 0;

    /* We have a solid context switch */
    _RME_Run_Swt(Reg, RME_Cur_Thd[CPUID], Next_Thd);
    RME_Cur_Thd[CPUID]=Next_Thd;

    return 0;
}
/* End Function:_RME_Thd_Swt *************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
