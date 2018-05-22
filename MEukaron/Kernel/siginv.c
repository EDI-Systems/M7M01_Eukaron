/******************************************************************************
Filename    : siginv.c
Author      : pry
Date        : 11/06/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The signal and invocation management code of RME RTOS.
******************************************************************************/

/* Includes ******************************************************************/
#define __HDR_DEFS__
#include "Platform/RME_platform.h"
#include "Kernel/kernel.h"
#include "Kernel/captbl.h"
#include "Kernel/pgtbl.h"
#include "Kernel/kotbl.h"
#include "Kernel/prcthd.h"
#include "Kernel/siginv.h"
#undef __HDR_DEFS__

#define __HDR_STRUCTS__
#include "Platform/RME_platform.h"
#include "Kernel/captbl.h"
#include "Kernel/kernel.h"
#include "Kernel/pgtbl.h"
#include "Kernel/kotbl.h"
#include "Kernel/prcthd.h"
#include "Kernel/siginv.h"
#undef __HDR_STRUCTS__

/* Private include */
#include "Kernel/siginv.h"

#define __HDR_PUBLIC_MEMBERS__
#include "Platform/RME_platform.h"
#include "Kernel/kernel.h"
#include "Kernel/captbl.h"
#include "Kernel/pgtbl.h"
#include "Kernel/kotbl.h"
#include "Kernel/prcthd.h"
#undef __HDR_PUBLIC_MEMBERS__
/* End Includes **************************************************************/

/* Begin Function:_RME_Sig_Boot_Crt *******************************************
Description : Create a boot-time kernel signal capability. This is not a system 
              call, and is only used at boot-time to create endpoints that are
              related directly to hardware interrupts.
              This function will not ask for a kernel memory capability.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              cid_t Cap_Captbl - The capability to the capability table to use
                                 for this signal. 2-Level.
              cid_t Cap_Inv - The capability slot that you want this newly created
                              signal capability to be in. 1-Level.
              ptr_t Vaddr - The physical address to store the kernel data. This must fall
                            within the kernel virtual address.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t _RME_Sig_Boot_Crt(struct RME_Cap_Captbl* Captbl, cid_t Cap_Captbl,
                        cid_t Cap_Sig, ptr_t Vaddr)
{
    struct RME_Cap_Captbl* Captbl_Crt;
    struct RME_Cap_Sig* Sig_Crt;
    struct RME_Sig_Struct* Sig_Struct;
    ptr_t Type_Ref;
    
    /* Get the capability slots */
    RME_CAPTBL_GETCAP(Captbl,Cap_Captbl,RME_CAP_CAPTBL,struct RME_Cap_Captbl*,Captbl_Crt);
    /* Check if the captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Captbl_Crt,RME_CAPTBL_FLAG_CRT);
    
    /* Get the cap slot */
    RME_CAPTBL_GETSLOT(Captbl_Crt,Cap_Sig,struct RME_Cap_Sig*,Sig_Crt);
    /* Take the slot if possible */
    RME_CAPTBL_OCCUPY(Sig_Crt,Type_Ref);
    
    /* Try to populate the area */
    if(_RME_Kotbl_Mark(Vaddr, RME_SIG_SIZE)!=0)
    {
        Sig_Crt->Head.Type_Ref=0;
        return RME_ERR_CAP_KOTBL;
    }
    
    /* Fill in the structure */
    Sig_Struct=(struct RME_Sig_Struct*)Vaddr;
    Sig_Struct->Kernel_Flag=1;
    Sig_Struct->Signal_Num=0;
    Sig_Struct->Thd=0;
    
    /* Fill in the header part */
    Sig_Crt->Head.Parent=0;
    Sig_Crt->Head.Object=Vaddr;
    /* Receive only because this is from kernel */
    Sig_Crt->Head.Flags=RME_SIG_FLAG_RCV;
    
    /* Creation complete */
    Sig_Crt->Head.Type_Ref=RME_CAP_TYPEREF(RME_CAP_SIG,0);
    
    return 0;
}
/* End Function:_RME_Sig_Boot_Crt ********************************************/

/* Begin Function:_RME_Sig_Crt ************************************************
Description : Create a signal capability.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              cid_t Cap_Captbl - The capability to the capability table to use
                                 for this signal. 2-Level.
              cid_t Cap_Kmem - The kernel memory capability. 2-Level.
              cid_t Cap_Inv - The capability slot that you want this newly created
                              signal capability to be in. 1-Level.
              ptr_t Vaddr - The physical address to store the kernel data. This must fall
                            within the kernel virtual address.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t _RME_Sig_Crt(struct RME_Cap_Captbl* Captbl, cid_t Cap_Captbl,
                   cid_t Cap_Kmem, cid_t Cap_Sig, ptr_t Vaddr)
{
    struct RME_Cap_Captbl* Captbl_Op;
    struct RME_Cap_Kmem* Kmem_Op;
    struct RME_Cap_Sig* Sig_Crt;
    struct RME_Sig_Struct* Sig_Struct;
    ptr_t Type_Ref;
    
    /* Get the capability slots */
    RME_CAPTBL_GETCAP(Captbl,Cap_Captbl,RME_CAP_CAPTBL,struct RME_Cap_Captbl*,Captbl_Op);
    RME_CAPTBL_GETCAP(Captbl,Cap_Kmem,RME_CAP_KMEM,struct RME_Cap_Kmem*,Kmem_Op);
    /* Check if the captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Captbl_Op,RME_CAPTBL_FLAG_CRT);
    /* See if the creation is valid for this kmem range */
    RME_KMEM_CHECK(Kmem_Op,RME_KMEM_FLAG_SIG,Vaddr,RME_SIG_SIZE);
    
    /* Get the cap slot */
    RME_CAPTBL_GETSLOT(Captbl_Op,Cap_Sig,struct RME_Cap_Sig*,Sig_Crt);
    /* Take the slot if possible */
    RME_CAPTBL_OCCUPY(Sig_Crt,Type_Ref);
    
    /* Try to populate the area */
    if(_RME_Kotbl_Mark(Vaddr, RME_SIG_SIZE)!=0)
    {
        Sig_Crt->Head.Type_Ref=0;
        return RME_ERR_CAP_KOTBL;
    }
    
    /* Fill in the structure */
    Sig_Struct=(struct RME_Sig_Struct*)Vaddr;
    Sig_Struct->Kernel_Flag=0;
    Sig_Struct->Signal_Num=0;
    Sig_Struct->Thd=0;
    
    /* Fill in the header part */
    Sig_Crt->Head.Parent=0;
    Sig_Crt->Head.Object=Vaddr;
    Sig_Crt->Head.Flags=RME_SIG_FLAG_SND|RME_SIG_FLAG_RCV;
    
    /* Creation complete */
    Sig_Crt->Head.Type_Ref=RME_CAP_TYPEREF(RME_CAP_SIG,0);
    
    return 0;
}
/* End Function:_RME_Sig_Crt *************************************************/

/* Begin Function:_RME_Sig_Del ************************************************
Description : Delete a signal capability.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              cid_t Cap_Captbl - The capability to the capability table to delete from.
                                 2-Level.
              cid_t Cap_Sig - The capability to the signal. 1-Level.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t _RME_Sig_Del(struct RME_Cap_Captbl* Captbl, cid_t Cap_Captbl, cid_t Cap_Sig)
{
    struct RME_Cap_Captbl* Captbl_Op;
    struct RME_Cap_Sig* Sig_Del;
    ptr_t Type_Ref;
    /* These are for deletion */
    struct RME_Sig_Struct* Sig_Struct;
    
    /* Get the capability slot */
    RME_CAPTBL_GETCAP(Captbl,Cap_Captbl,RME_CAP_CAPTBL,struct RME_Cap_Captbl*,Captbl_Op);    
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Captbl_Op,RME_CAPTBL_FLAG_DEL);
    
    /* Get the cap slot */
    RME_CAPTBL_GETSLOT(Captbl_Op,Cap_Sig,struct RME_Cap_Sig*,Sig_Del);
    /* Delete check */
    RME_CAP_DEL_CHECK(Sig_Del,Type_Ref,RME_CAP_SIG);
    
    /* Get the thread */
    Sig_Struct=RME_CAP_GETOBJ(Sig_Del,struct RME_Sig_Struct*);
    
    /* See if the signal endpoint is currently used. If yes, we cannot delete it */
    if(Sig_Struct->Thd!=0)
    {
        RME_CAP_DEFROST(Sig_Del,Type_Ref);
        return RME_ERR_SIV_ACT;
    }
    
    /* See if this is a kernel endpoint. If yes, we cannot delete it */
    if(Sig_Struct->Kernel_Flag!=0)
    {
        RME_CAP_DEFROST(Sig_Del,Type_Ref);
        return RME_ERR_SIV_CONFLICT;
    }
    
    /* Now we can safely delete the cap */
    RME_CAP_REMDEL(Sig_Del,Type_Ref);
    /* Try to depopulate the area - this must be successful */
    RME_ASSERT(_RME_Kotbl_Erase((ptr_t)Sig_Struct,RME_SIG_SIZE)!=0);
    
    return 0;
}
/* End Function:_RME_Sig_Del *************************************************/

/* Begin Function:_RME_Kern_Snd ***********************************************
Description : Try to send a signal to an endpoint from kernel. This is intended to
              be called in the interrupt routines in the kernel, and this is not a
              system call.
Input       : struct RME_Reg_Struct* Reg - The register set.
              struct RME_Sig_Struct* Sig - The signal structure.
Output      : None.
Return      : ret_t - If successful, 0, or an error code.
******************************************************************************/
ret_t _RME_Kern_Snd(struct RME_Reg_Struct* Reg, struct RME_Sig_Struct* Sig_Struct)
{
    struct RME_Thd_Struct* Thd_Struct;
    ptr_t Unblock;
    ptr_t CPUID;
    
    /* Cannot send to a user endpoint in the kernel */
    if(Sig_Struct->Kernel_Flag==0)
        return RME_ERR_SIV_CONFLICT;
    /* See if we can receive on that endpoint - if someone blocks, we must
     * wait for it to unblock before we can proceed */
    CPUID=RME_CPUID();
    Thd_Struct=Sig_Struct->Thd;
    /* If and only if we are calling from the same core as the blocked thread do
     * we actually unblock. Use an intermediate variable Unblock to avoid optimizations */
    if(Thd_Struct!=0)
    {
        if(Thd_Struct->Sched.CPUID_Bind==CPUID)
            Unblock=1;
        else
            Unblock=0;
    }
    else
        Unblock=0;
    
    if(Unblock!=0)
    {
        /* The thread is blocked, and it is on our core. Unblock it, and
         * set the return value, then see if we need a preemption */
        __RME_Set_Syscall_Retval(&(Thd_Struct->Cur_Reg->Reg), Sig_Struct->Signal_Num);
        /* See if the thread still have time left */
        if(Thd_Struct->Sched.Slices!=0)
        {
            /* Put this into the runqueue */
            _RME_Run_Ins(Thd_Struct);
            /* See if it will preempt us */
            if(Thd_Struct->Sched.Prio>RME_Cur_Thd[CPUID]->Sched.Prio)
            {
                /* Yes. Do a context switch */
                _RME_Run_Swt(Reg,RME_Cur_Thd[CPUID],Thd_Struct);
                RME_Cur_Thd[CPUID]->Sched.State=RME_THD_READY;
                Thd_Struct->Sched.State=RME_THD_RUNNING;
                RME_Cur_Thd[CPUID]=Thd_Struct;
            }
            else
                Thd_Struct->Sched.State=RME_THD_READY;
        }
        else
        {
            /* No slices left. This is because we delegated all of its time
             * to someone else. Notify the parent, and change the state of this
             * thread to TIMEOUT */
            Thd_Struct->Sched.State=RME_THD_TIMEOUT;
            /* Notify the parent about this */
            _RME_Run_Notif(Thd_Struct);
        }
        
        /* Clear the blocking status of the endpoint up */
        Sig_Struct->Thd=0;
    }
    else
    {
        /* The guy who blocked on it is not on our core, or nobody blocked.
         * We just faa the counter value and return */
        if(__RME_Fetch_Add(&(Sig_Struct->Signal_Num),1)>RME_MAX_SIG_NUM)
        {
            __RME_Fetch_Add(&(Sig_Struct->Signal_Num),-1);
            return RME_ERR_SIV_FULL;
        }
    }

    return 0;
}
/* End Function:_RME_Kern_Snd ************************************************/

/* Begin Function:_RME_Sig_Snd ************************************************
Description : Try to send a signal from user level. This system call can cause
              a potential context switch.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              struct RME_Reg_Struct* Reg - The register set.
              cid_t Cap_Sig - The capability to the signal. 2-Level.
Output      : None.
Return      : ret_t - If successful, 0, or an error code.
******************************************************************************/
ret_t _RME_Sig_Snd(struct RME_Cap_Captbl* Captbl, struct RME_Reg_Struct* Reg, cid_t Cap_Sig)
{
    struct RME_Cap_Sig* Sig_Op;
    struct RME_Sig_Struct* Sig_Struct;
    struct RME_Thd_Struct* Thd_Struct;
    ptr_t Unblock;
    ptr_t CPUID;
    
    /* Get the capability slot */
    RME_CAPTBL_GETCAP(Captbl,Cap_Sig,RME_CAP_SIG,struct RME_Cap_Sig*,Sig_Op);    
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Sig_Op,RME_SIG_FLAG_SND);
    
    /* See if we can receive on that endpoint - if someone blocks, we must
     * wait for it to unblock before we can proceed */
    CPUID=RME_CPUID();
    Sig_Struct=RME_CAP_GETOBJ(Sig_Op,struct RME_Sig_Struct*);
    Thd_Struct=Sig_Struct->Thd;
    /* If and only if we are calling from the same core as the blocked thread do
     * we actually unblock. Use an intermediate variable Unblock to avoid optimizations */
    if(Thd_Struct!=0)
    {
        if(Thd_Struct->Sched.CPUID_Bind==CPUID)
            Unblock=1;
        else
            Unblock=0;
    }
    else
        Unblock=0;
    
    if(Unblock!=0)
    {
        /* Now save the system call return value to the caller stack */
        __RME_Set_Syscall_Retval(Reg,0);
        /* The thread is blocked, and it is on our core. Unblock it, and
         * set the return value, then see if we need a preemption */
        __RME_Set_Syscall_Retval(&(Thd_Struct->Cur_Reg->Reg), Sig_Struct->Signal_Num);
        /* See if the thread still have time left */
        if(Thd_Struct->Sched.Slices!=0)
        {
            /* Put this into the runqueue */
            _RME_Run_Ins(Thd_Struct);
            /* See if it will preempt us */
            if(Thd_Struct->Sched.Prio>RME_Cur_Thd[CPUID]->Sched.Prio)
            {
                /* Yes. Do a context switch */
                _RME_Run_Swt(Reg,RME_Cur_Thd[CPUID],Thd_Struct);
                RME_Cur_Thd[CPUID]->Sched.State=RME_THD_READY;
                Thd_Struct->Sched.State=RME_THD_RUNNING;
                RME_Cur_Thd[CPUID]=Thd_Struct;
            }
            else
                Thd_Struct->Sched.State=RME_THD_READY;
        }
        else
        {
            /* No slices left. This is because we delegated all of its time
             * to someone else. Notify the parent, and change the state of this
             * thread to TIMEOUT */
            Thd_Struct->Sched.State=RME_THD_TIMEOUT;
            /* Notify the parent about this */
            _RME_Run_Notif(Thd_Struct);
        }
        
        /* Clear the blocking status of the endpoint up */
        Sig_Struct->Thd=0;
    }
    else
    {
        /* The guy who blocked on it is not on our core, we just faa and return */
        if(__RME_Fetch_Add(&(Sig_Struct->Signal_Num),1)>RME_MAX_SIG_NUM)
        {
            __RME_Fetch_Add(&(Sig_Struct->Signal_Num),-1);
            return RME_ERR_SIV_FULL;
        }
        /* Now save the system call return value to the caller stack */
        __RME_Set_Syscall_Retval(Reg,0);
    }

    return 0;
}
/* End Function:_RME_Sig_Snd *************************************************/

/* Begin Function:_RME_Sig_Rcv ************************************************
Description : Try to receive a signal capability. The rules for the signal capability
              is:
              1.If a receive endpoint have many send endpoints, everyone can send to it,
                and sending to it will increase the signal count by 1.
              2.If some thread blocks on a receive endpoint, the wakeup is only possible
                from the same core that thread is on.
              3.It is not recommended to let 2 cores operate on the rcv endpoint simutaneously.
              This system call can potentially trigger a context switch.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              struct RME_Reg_Struct* Reg - The register set.
              cid_t Cap_Sig - The capability to the signal. 2-Level.
Output      : None.
Return      : ret_t - If successful, a non-negative number containing the current
                      counter value will be returned; else an error code.
******************************************************************************/
ret_t _RME_Sig_Rcv(struct RME_Cap_Captbl* Captbl, struct RME_Reg_Struct* Reg, cid_t Cap_Sig)
{
    struct RME_Cap_Sig* Sig_Op;
    struct RME_Sig_Struct* Sig_Struct;
    struct RME_Thd_Struct* Thd_Struct;
    ptr_t Old_Value;
    ptr_t CPUID;
    
    /* Get the capability slot */
    RME_CAPTBL_GETCAP(Captbl,Cap_Sig,RME_CAP_SIG,struct RME_Cap_Sig*,Sig_Op);    
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Sig_Op,RME_SIG_FLAG_RCV);
    
    /* See if we can receive on that endpoint - if someone blocks, we must
     * wait for it to unblock before we can proceed */
    Sig_Struct=RME_CAP_GETOBJ(Sig_Op,struct RME_Sig_Struct*);
    Thd_Struct=Sig_Struct->Thd;
    if(Thd_Struct!=0)
        return RME_ERR_SIV_ACT;
    
    /* Are we trying to let a boot-time thread block on a signal? This is NOT allowed */
    CPUID=RME_CPUID();
    Thd_Struct=RME_Cur_Thd[CPUID];
    if(Thd_Struct->Sched.Slices==RME_THD_INIT_TIME)
        return RME_ERR_SIV_BOOT;
    
    /* Are there any counts available? If yes, just take one and return. We cannot
     * use faa here because we don't know if we will get it below zero */
    Old_Value=Sig_Struct->Signal_Num;
    if(Old_Value>0)
    {
        /* Try to take it */
        if(__RME_Comp_Swap(&(Sig_Struct->Signal_Num),&Old_Value,Old_Value-1)==0)
            return RME_ERR_SIV_CONFLICT;
        /* We have taken it, now return the number of remaining signals */
        __RME_Set_Syscall_Retval(Reg, Old_Value-1);
        return 0;
    }
    else
    {
        /* Try to take it */
        Old_Value=0;
        if(__RME_Comp_Swap((ptr_t*)(&(Sig_Struct->Thd)),&Old_Value,(ptr_t)Thd_Struct)==0)
            return RME_ERR_SIV_CONFLICT;
        /* We have taken it, now we block our current thread. No need to set any
         * return value to the register set here, because we do not yet know how
         * many signals will be there when the thread unblocks */
        Thd_Struct->Sched.State=RME_THD_BLOCKED;
        Thd_Struct->Sched.Signal=Sig_Struct;
        _RME_Run_Del(Thd_Struct);
        RME_Cur_Thd[CPUID]=_RME_Run_High(CPUID);
        _RME_Run_Swt(Reg,Thd_Struct,RME_Cur_Thd[CPUID]);
        RME_Cur_Thd[CPUID]->Sched.State=RME_THD_RUNNING;
    }
    
    return 0;
}
/* End Function:_RME_Sig_Rcv *************************************************/

/* Begin Function:_RME_Inv_Crt ************************************************
Description : Create an invocation capability.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              cid_t Cap_Captbl - The capability to the capability table to use
                                 for this process. 2-Level.
              cid_t Cap_Kmem - The kernel memory capability. 2-Level.
              cid_t Cap_Inv - The capability slot that you want this newly created
                              invocation capability to be in. 1-Level.
              cid_t Cap_Proc - The capability to the process that it is in. 2-Level.
              ptr_t Vaddr - The physical address to store the kernel data. This must fall
                            within the kernel virtual address.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t _RME_Inv_Crt(struct RME_Cap_Captbl* Captbl, cid_t Cap_Captbl,
                   cid_t Cap_Kmem, cid_t Cap_Inv, cid_t Cap_Proc, ptr_t Vaddr)
{
    struct RME_Cap_Captbl* Captbl_Op;
    struct RME_Cap_Proc* Proc_Op;
    struct RME_Cap_Kmem* Kmem_Op;
    struct RME_Cap_Inv* Inv_Crt;
    struct RME_Inv_Struct* Inv_Struct;
    ptr_t Type_Ref;
    
    /* Get the capability slots */
    RME_CAPTBL_GETCAP(Captbl,Cap_Captbl,RME_CAP_CAPTBL,struct RME_Cap_Captbl*,Captbl_Op);
    RME_CAPTBL_GETCAP(Captbl,Cap_Proc,RME_CAP_PROC,struct RME_Cap_Proc*,Proc_Op);
    RME_CAPTBL_GETCAP(Captbl,Cap_Kmem,RME_CAP_KMEM,struct RME_Cap_Kmem*,Kmem_Op);
    /* Check if the captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Captbl_Op,RME_CAPTBL_FLAG_CRT);
    RME_CAP_CHECK(Proc_Op,RME_PROC_FLAG_INV);
    /* See if the creation is valid for this kmem range */
    RME_KMEM_CHECK(Kmem_Op,RME_KMEM_FLAG_INV,Vaddr,RME_INV_SIZE);
    
    /* Get the cap slot */
    RME_CAPTBL_GETSLOT(Captbl_Op,Cap_Inv,struct RME_Cap_Inv*,Inv_Crt);
    /* Take the slot if possible */
    RME_CAPTBL_OCCUPY(Inv_Crt,Type_Ref);
    
    /* Try to populate the area */
    if(_RME_Kotbl_Mark(Vaddr, RME_INV_SIZE)!=0)
    {
        Inv_Crt->Head.Type_Ref=0;
        return RME_ERR_CAP_KOTBL;
    }
    
    /* Fill in the structure */
    Inv_Struct=(struct RME_Inv_Struct*)Vaddr;
    Inv_Struct->Proc=RME_CAP_GETOBJ(Proc_Op,struct RME_Proc_Struct*);
    /* Increase the reference count of the process structure(Not the process capability) */
    __RME_Fetch_Add(&(RME_CAP_GETOBJ(Proc_Op, struct RME_Proc_Struct*)->Refcnt), 1);
    
    /* Fill in the header part */
    Inv_Crt->Head.Parent=0;
    Inv_Crt->Head.Object=Vaddr;
    Inv_Crt->Head.Flags=RME_INV_FLAG_SET|RME_INV_FLAG_ACT;
    
    /* Creation complete */
    Inv_Crt->Head.Type_Ref=RME_CAP_TYPEREF(RME_CAP_INV,0);
    
    return 0;
}
/* End Function:_RME_Inv_Crt *************************************************/

/* Begin Function:_RME_Inv_Del ************************************************
Description : Delete an invocation capability.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              cid_t Cap_Captbl - The capability to the capability table to delete from.
                                 2-Level.
              cid_t Cap_Inv - The capability to the invocation stub. 1-Level.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t _RME_Inv_Del(struct RME_Cap_Captbl* Captbl, cid_t Cap_Captbl, cid_t Cap_Inv)
{
    struct RME_Cap_Captbl* Captbl_Op;
    struct RME_Cap_Inv* Inv_Del;
    ptr_t Type_Ref;
    /* These are for deletion */
    struct RME_Inv_Struct* Inv_Struct;
    
    /* Get the capability slot */
    RME_CAPTBL_GETCAP(Captbl,Cap_Captbl,RME_CAP_CAPTBL,struct RME_Cap_Captbl*,Captbl_Op);    
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Captbl_Op,RME_CAPTBL_FLAG_DEL);
    
    /* Get the cap slot */
    RME_CAPTBL_GETSLOT(Captbl_Op,Cap_Inv,struct RME_Cap_Inv*,Inv_Del);
    /* Delete check */
    RME_CAP_DEL_CHECK(Inv_Del,Type_Ref,RME_CAP_INV);
    
    /* Get the thread */
    Inv_Struct=RME_CAP_GETOBJ(Inv_Del,struct RME_Inv_Struct*);
    
    /* See if the invocation is currently used. If yes, we cannot delete it */
    if(Inv_Struct->Active!=0)
    {
        RME_CAP_DEFROST(Inv_Del,Type_Ref);
        return RME_ERR_SIV_ACT;
    }
    
    /* Now we can safely delete the cap */
    RME_CAP_REMDEL(Inv_Del,Type_Ref);
    /* Dereference the process */
    __RME_Fetch_Add(&(Inv_Struct->Proc->Refcnt), -1);
    /* Try to depopulate the area - this must be successful */
    RME_ASSERT(_RME_Kotbl_Erase((ptr_t)Inv_Struct,RME_INV_SIZE)!=0);
    
    return 0;
}
/* End Function:_RME_Inv_Del *************************************************/

/* Begin Function:_RME_Inv_Set ************************************************
Description : Set an invocation stub's entry point and stack. The registers will
              be initialized with these contents.
Input       : struct RME_Cap_Captbl* Captbl - The capability table.
              cid_t Cap_Inv - The capability to the invocation stub. 2-Level.
              ptr_t Entry - The entry of the thread.
              ptr_t Stack - The stack address to use for execution.
              ptr_t Fault_Ret_Flag - If there is an error in this invocation, we return
                                     immediately, or we wait for fault handling?
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t _RME_Inv_Set(struct RME_Cap_Captbl* Captbl, cid_t Cap_Inv,
                   ptr_t Entry, ptr_t Stack, ptr_t Fault_Ret_Flag)
{
    struct RME_Cap_Inv* Inv_Op;
    struct RME_Inv_Struct* Inv_Struct;
    
    /* Get the capability slot */
    RME_CAPTBL_GETCAP(Captbl,Cap_Inv,RME_CAP_INV,struct RME_Cap_Inv*,Inv_Op);
    /* Check if the target cap is not frozen and allows such operations */
    RME_CAP_CHECK(Inv_Op,RME_INV_FLAG_SET);
    
    /* Commit the change - we do not care if the invocation is in use */
    Inv_Struct=RME_CAP_GETOBJ(Inv_Op,struct RME_Inv_Struct*);
    Inv_Struct->Entry=Entry;
    Inv_Struct->Stack=Stack;
    Inv_Struct->Fault_Ret_Flag=Fault_Ret_Flag;
    
    return 0;
}
/* End Function:_RME_Inv_Set *************************************************/

/* Begin Function:_RME_Inv_Act ************************************************
Description : Activate an invocation capability. That means, do the invocation.
Input       : struct RME_Cap_Captbl* Captbl - The capability table.
              struct RME_Reg_Struct* Reg - The register set for this thread.
              cid_t Cap_Inv - The capability slot to the invocation stub. 2-Level.
              ptr_t Param - The parameter for the call.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t _RME_Inv_Act(struct RME_Cap_Captbl* Captbl, 
                   struct RME_Reg_Struct* Reg,
                   cid_t Cap_Inv, ptr_t Param)
{
    struct RME_Cap_Inv* Inv_Op;
    struct RME_Inv_Struct* Inv_Struct;
    struct RME_Inv_Struct* Inv_Top;
    struct RME_Thd_Struct* Thd_Struct;
    ptr_t Active;
    
    /* Get the capability slot */
    RME_CAPTBL_GETCAP(Captbl,Cap_Inv,RME_CAP_INV,struct RME_Cap_Inv*,Inv_Op);
    /* Check if the target cap is not frozen and allows such operations */
    RME_CAP_CHECK(Inv_Op,RME_INV_FLAG_ACT);
    
    /* Get the invocation struct */
    Inv_Struct=RME_CAP_GETOBJ(Inv_Op,struct RME_Inv_Struct*);
    /* See if we are currently active - If yes, we can activate it again */
    Active=Inv_Struct->Active;
    if(Active!=0)
        return RME_ERR_SIV_ACT;
    
    /* Push this invocation stub capability into the current thread's invocation stack */
    Thd_Struct=RME_Cur_Thd[RME_CPUID()];
    /* Try to do CAS and activate it */
    if(__RME_Comp_Swap(&(Inv_Struct->Active),&Active,1)==0)
        return RME_ERR_SIV_ACT;
    
    /* Save whatever is needed to return to the point - normally only SP and IP needed
     * because all other registers, including the coprocessor registers, are saved at
     * user-level. We do not set the return value because it will be saved by SRET.
     * The coprocessor state will be consistent across the call */
    __RME_Inv_Reg_Save(&(Inv_Struct->Ret), Reg);
    Inv_Top=RME_INVSTK_TOP(Thd_Struct);
    /* Push this into the stack: insert after the thread list header */
    __RME_List_Ins(&(Inv_Struct->Head),&(Thd_Struct->Inv_Stack),Thd_Struct->Inv_Stack.Next);
    /* Setup the register contents, and do the invocation */
    __RME_Thd_Reg_Init(Inv_Struct->Entry, Inv_Struct->Stack, Reg);
    __RME_Inv_Reg_Init(Param, Reg);
    
    /* Are we invoking into a new process? If yes, switch the page table */
    if(Inv_Top!=0)
    {
        if(RME_CAP_GETOBJ(Inv_Top->Proc->Pgtbl,ptr_t)!=RME_CAP_GETOBJ(Inv_Struct->Proc->Pgtbl,ptr_t))
            __RME_Pgtbl_Set(RME_CAP_GETOBJ(Inv_Struct->Proc->Pgtbl,ptr_t));
    }
    else
    {
        if(RME_CAP_GETOBJ(Thd_Struct->Sched.Proc->Pgtbl,ptr_t)!=RME_CAP_GETOBJ(Inv_Struct->Proc->Pgtbl,ptr_t))
            __RME_Pgtbl_Set(RME_CAP_GETOBJ(Inv_Struct->Proc->Pgtbl,ptr_t));
    }
    
    return 0;
}
/* End Function:_RME_Inv_Act *************************************************/

/* Begin Function:_RME_Inv_Ret ************************************************
Description : Return from the invocation function, and set the return value to
              the old register set. This function does not need a capability
              table to work.
Input       : struct RME_Reg_Struct* Reg - The register set for this thread.
              ptr_t Fault_Flag - Are we attempting a return from fault?
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t _RME_Inv_Ret(struct RME_Reg_Struct* Reg, ptr_t Fault_Flag)
{
    struct RME_Thd_Struct* Thd_Struct;
    struct RME_Inv_Struct* Inv_Struct;
    struct RME_Inv_Struct* Inv_Top;
    ptr_t Retval;
    
    /* See if we can return; If we can, get the structure */
    Thd_Struct=RME_Cur_Thd[RME_CPUID()];
    Inv_Struct=RME_INVSTK_TOP(Thd_Struct);
    if(Inv_Struct==0)
        return RME_ERR_SIV_EMPTY;
    
    /* Is this return forced by a fault? If yes, check if we allow that */
    if((Fault_Flag!=0)&&(Inv_Struct->Fault_Ret_Flag==0))
        return RME_ERR_SIV_FAULT;
    
    /* Pop it from the stack */
    __RME_List_Del(Inv_Struct->Head.Prev,Inv_Struct->Head.Next);
    /* Get the return value from the register set */
    Retval=__RME_Get_Inv_Retval(Reg);

    /* Restore the register contents, and set return value. We need to set
     * the return value of the invocation system call itself as well */
    __RME_Inv_Reg_Restore(Reg, &(Inv_Struct->Ret));
    __RME_Set_Inv_Retval(Reg, Retval);
    /* Decide the system call's return value */
    if(Fault_Flag!=0)
        __RME_Set_Syscall_Retval(Reg, RME_ERR_SIV_FAULT);
    else
        __RME_Set_Syscall_Retval(Reg, 0);
    
    /* Are we returning to a new process? If yes, switch the page table */
    Inv_Top=RME_INVSTK_TOP(Thd_Struct);
    if(Inv_Top!=0)
    {
        if(RME_CAP_GETOBJ(Inv_Top->Proc->Pgtbl,ptr_t)!=RME_CAP_GETOBJ(Inv_Struct->Proc->Pgtbl,ptr_t))
            __RME_Pgtbl_Set(RME_CAP_GETOBJ(Inv_Struct->Proc->Pgtbl,ptr_t));
    }
    else
    {
        if(RME_CAP_GETOBJ(Thd_Struct->Sched.Proc->Pgtbl,ptr_t)!=RME_CAP_GETOBJ(Inv_Struct->Proc->Pgtbl,ptr_t))
            __RME_Pgtbl_Set(RME_CAP_GETOBJ(Inv_Struct->Proc->Pgtbl,ptr_t));
    }
    
    /* We have successfully returned, set the invocation as inactive */
    Inv_Struct->Active=0;
    
    return 0;
}
/* End Function:_RME_Inv_Ret *************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/

