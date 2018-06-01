/******************************************************************************
Filename    : kernel.c
Author      : pry
Date        : 23/03/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The system call processing path, debugging primitives and kernel
              capability implementation for the RME system.
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
#include "Kernel/kernel.h"

#define __HDR_PUBLIC_MEMBERS__
#include "Platform/RME_platform.h"
#include "Kernel/captbl.h"
#include "Kernel/pgtbl.h"
#include "Kernel/kotbl.h"
#include "Kernel/prcthd.h"
#include "Kernel/siginv.h"
#undef __HDR_PUBLIC_MEMBERS__
/* End Includes **************************************************************/

/* Begin Function:_RME_Clear **************************************************
Description : Memset a memory area to zero. This is not fast due to byte operations;
              this is not meant for large memory.
Input       : void* Addr - The address to clear.
              ptr_t Size - The size to clear.
Output      : None.
Return      : None.
******************************************************************************/
void _RME_Clear(void* Addr, ptr_t Size)
{
	cnt_t Count;

    for(Count=0;Count<Size;Count++)
    	((u8*)Addr)[Count]=0;
}
/* End Function:_RME_Clear ***************************************************/

/* Begin Function:_RME_Memcmp *************************************************
Description : Compare two memory segments to see if they are equal. This is not
              fast due to byte operations; this is not meant for large memory.
Input       : const void* Ptr1 - The first memory region.
              const void* Ptr2 - The second memory region.
              ptr_t Num - The number of bytes to compare.
Output      : None.
Return      : ret_t - If Ptr1>Ptr2, then return a positive value; else a negative
                      value. If Ptr1==Ptr2, then return 0;
******************************************************************************/
ret_t _RME_Memcmp(const void* Ptr1, const void* Ptr2, ptr_t Num)
{
    u8* Dst;
    u8* Src;
    cnt_t Count;

    Dst=(u8*)Ptr1;
    Src=(u8*)Ptr2;

    for(Count=0;Count<Num;Count++)
    {
    	if(Dst[Count]!=Src[Count])
    		return Dst[Count]-Src[Count];
    }

    return 0;
}
/* End Function:_RME_Memcmp **************************************************/

/* Begin Function:_RME_Memcpy *************************************************
Description : Copy one segment of memory to another segment. This is not fast
              due to byte operations; this is not meant for large memory.
Input       : void* Dst - The first memory region.
              void* Src - The second memory region.
              ptr_t Num - The number of bytes to compare.
              ptr_t Size - The size to clear.
Output      : None.
Return      : None.
******************************************************************************/
void _RME_Memcpy(void* Dst, void* Src, ptr_t Num)
{
    cnt_t Count;

    for(Count=0;Count<Num;Count++)
        ((u8*)Dst)[Count]=((u8*)Src)[Count];
}
/* End Function:_RME_Memcpy **************************************************/

/* Begin Function:_RME_Timestamp_Inc ******************************************
Description : This function is used in the drivers to update the timestamp value.
Input       : cnt_t Value - The value to increase.
Output      : None.
Return      : ptr_t - The timestamp value before the increment.
******************************************************************************/
ptr_t _RME_Timestamp_Inc(cnt_t Value)
{
    /* The incremental value cannot be smaller than zero or equal to zero */
    RME_ASSERT(Value>0);
    return __RME_Fetch_Add(&RME_Timestamp,Value);
}
/* End Function:_RME_Timestamp_Inc *******************************************/

/* Begin Function:_RME_Kern_Boot_Crt ******************************************
Description : This function is used to create boot-time kernel call capability.
              This kind of capability that does not have a kernel object. Kernel
              function capabilities are the capabilities that allow you to execute
              functions in kernel mode. These functions must be defined in the 
              CPU driver platform file.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              cid_t Cap_Captbl - The capability to the captbl that may contain the cap
                                 to kernel function. 2-Level.
              cid_t Cap_Kern - The capability to the kernel function. 1-Level.
Output      : None.
Return      : ret_t - If the mapping is successful, it will return 0; else error code.
******************************************************************************/
ret_t _RME_Kern_Boot_Crt(struct RME_Cap_Captbl* Captbl, cid_t Cap_Captbl, cid_t Cap_Kern)
{
    struct RME_Cap_Captbl* Captbl_Op;
    struct RME_Cap_Kern* Kern_Crt;
    ptr_t Type_Ref;
    
    /* Get the cap location that we care about */
    RME_CAPTBL_GETCAP(Captbl,Cap_Captbl,RME_CAP_CAPTBL,struct RME_Cap_Captbl*,Captbl_Op);
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Captbl_Op,RME_CAPTBL_FLAG_CRT);
    
    /* Get the cap slot */
    RME_CAPTBL_GETSLOT(Captbl_Op,Cap_Kern,struct RME_Cap_Kern*,Kern_Crt);
    /* Take the slot if possible */
    RME_CAPTBL_OCCUPY(Kern_Crt,Type_Ref);
    
    Kern_Crt->Head.Parent=0;
    /* The kernel capability does not have an object */
    Kern_Crt->Head.Object=0;
    Kern_Crt->Head.Flags=RME_KERN_FLAG_FULL_RANGE;
    
    /* Creation complete, and make it undeletable */
    Kern_Crt->Head.Type_Ref=RME_CAP_TYPEREF(RME_CAP_KERN,1);
    return 0;
}
/* End Function:_RME_Kern_Boot_Crt *******************************************/

/* Begin Function:_RME_Kern_Act ***********************************************
Description : Activate a kernel function.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              struct RME_Reg_Struct* Reg - The current register set.
              cid_t Cap_Kern - The capability to the kernel capability. 2-Level.
              ptr_t Func_ID - The function ID to invoke.
              ptr_t Sub_ID - The subfunction ID to invoke.
              ptr_t Param1 - The first parameter.
              ptr_t Param2 - The second parameter
Output      : None.
Return      : ret_t - If the call is successful, it will return whatever the 
                      function returned(It is expected that these functions shall
                      never return an negative value); else error code. If the 
                      kernel function ever causes a context switch, it is responsible
                      for setting the return value. On failure, a context switch 
                      in the kernel fucntion is banned.
******************************************************************************/
ret_t _RME_Kern_Act(struct RME_Cap_Captbl* Captbl, struct RME_Reg_Struct* Reg,
                    cid_t Cap_Kern, ptr_t Func_ID, ptr_t Sub_ID, ptr_t Param1, ptr_t Param2)
{
    struct RME_Cap_Kern* Kern_Op;
    
    /* Get the capability slot */
    RME_CAPTBL_GETCAP(Captbl,Cap_Kern,RME_CAP_KERN,struct RME_Cap_Kern*,Kern_Op);    
    /* Check if the target cap is not frozen */
    if((Kern_Op->Head.Type_Ref&RME_CAP_FROZEN)!=0)
        return RME_ERR_CAP_FROZEN;
    /* Check if the range of calling is allowed - This is kernel function specific */
    if((Func_ID>RME_KERN_FLAG_HIGH(Kern_Op->Head.Flags))||
       (Func_ID<RME_KERN_FLAG_LOW(Kern_Op->Head.Flags)))
        return RME_ERR_CAP_FLAG;
    /* Return whatever the function returns */
    return __RME_Kern_Func_Handler(Reg,Func_ID,Sub_ID,Param1,Param2);
}
/* End Function:_RME_Kern_Act ************************************************/

/* Begin Function:_RME_Kmem_Boot_Crt ******************************************
Description : This function is used to create boot-time kernel memory capability.
              This kind of capability that does not have a kernel object. Kernel
              memory capabilities are the capabilities that allow you to create
              specific types of kernel objects in a specific kernel memory range.
              The initial kernel memory capability's content is supplied by the
              kernel according to the initial memory setup macros.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              cid_t Cap_Captbl - The capability to the captbl that may contain the cap
                                 to kernel function. 2-Level.
              cid_t Cap_Kmem - The capability to the kernel memory. 1-Level.
              ptr_t Start - The start address of the kernel memory, aligned to kotbl granularity.
              ptr_t End - The end address of the kernel memory, aligned to kotbl granularity, then -1.
              ptr_t Flags - The operation flags for this kernel memory.
Output      : None.
Return      : ret_t - If the mapping is successful, it will return 0; else error code.
******************************************************************************/
ret_t _RME_Kmem_Boot_Crt(struct RME_Cap_Captbl* Captbl, cid_t Cap_Captbl,
		                 cid_t Cap_Kmem, ptr_t Start, ptr_t End, ptr_t Flags)
{
    struct RME_Cap_Captbl* Captbl_Op;
    struct RME_Cap_Kmem* Kmem_Crt;
    ptr_t Kmem_Start;
    ptr_t Kmem_End;
    ptr_t Type_Ref;
    
    /* Get the cap location that we care about */
    RME_CAPTBL_GETCAP(Captbl,Cap_Captbl,RME_CAP_CAPTBL,struct RME_Cap_Captbl*,Captbl_Op);
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Captbl_Op,RME_CAPTBL_FLAG_CRT);
    
    /* Get the cap slot */
    RME_CAPTBL_GETSLOT(Captbl_Op,Cap_Kmem,struct RME_Cap_Kmem*,Kmem_Crt);
    /* Take the slot if possible */
    RME_CAPTBL_OCCUPY(Kmem_Crt,Type_Ref);
    
    /* Align addresses */
#if(RME_KMEM_SLOT_ORDER>6)
    Kmem_End=RME_ROUND_DOWN(End+1,RME_KMEM_SLOT_ORDER);
    Kmem_Start=RME_ROUND_UP(Start,RME_KMEM_SLOT_ORDER);
#else
    Kmem_End=RME_ROUND_DOWN(End+1,6);
    Kmem_Start=RME_ROUND_UP(Start,6);
#endif
    /* Must at least allow creation of something */
    RME_ASSERT(Flags!=0);

    Kmem_Crt->Head.Parent=0;
    /* The kernel memory capability does not have an object */
    Kmem_Crt->Head.Object=0;
    /* Fill in the flags, start and end */
    Kmem_Crt->Head.Flags=Flags;
    /* Extra flags */
    Kmem_Crt->Start=Kmem_Start;
    Kmem_Crt->End=Kmem_End-1;
    /* Creation complete */
    Kmem_Crt->Head.Type_Ref=RME_CAP_TYPEREF(RME_CAP_KMEM,1);
    return 0;
}
/* End Function:_RME_Kmem_Boot_Crt *******************************************/

/* Begin Function:_RME_Syscall_Init *******************************************
Description : The initialization function of system calls.
Input       : None.
Output      : None.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t _RME_Syscall_Init(void)
{
    cnt_t Count;
    
    /* Set it to 0x00..FF.. */
    RME_Timestamp=(~((ptr_t)(0)))>>(sizeof(ptr_t)*4);
    
    for(Count=0;Count<RME_CPU_NUM;Count++)
        RME_Cur_Thd[Count]=0;
    
    return 0;
}
/* End Function:_RME_Syscall_Init ********************************************/

/* Begin Function:_RME_Svc_Handler ********************************************
Description : The system call handler of the operating system. The register set 
              of the current thread shall be passed in as a parameter.
Input       : struct RME_Reg_Struct* Reg - The register set when entering the handler.
Output      : struct RME_Reg_Struct* Reg - The register set when exiting the handler.
Return      : None.
******************************************************************************/
void _RME_Svc_Handler(struct RME_Reg_Struct* Reg)
{
    /* What's the system call number and major capability ID? */
    ptr_t Svc;
    ptr_t Capid;
    ptr_t Param[3];
    ret_t Retval;
    ptr_t CPUID;
    ptr_t Svc_Num;
    struct RME_Inv_Struct* Inv_Top;
    struct RME_Cap_Captbl* Captbl;

    /* Get the system call parameters from the system call */
    __RME_Get_Syscall_Param(Reg, &Svc, &Capid, Param);
    Svc_Num=Svc&0x3F;
    
    /* Fast path - synchronous invocation returning */
    if(Svc_Num==RME_SVC_INV_RET)
    {
        Retval=_RME_Inv_Ret(Reg      /* struct RME_Reg_Struct* Reg */,
        		            Param[0] /* ptr_t Retval */,
                            0        /* ptr_t Fault_Flag */);
        RME_SWITCH_RETURN(Reg,Retval);
    }
    
    /* Get our current capability table. No need to check whether it is frozen
     * because it can't be deleted anyway */
    CPUID=RME_CPUID();
    Inv_Top=RME_INVSTK_TOP(RME_Cur_Thd[CPUID]);
    if(Inv_Top==0)
        Captbl=RME_Cur_Thd[CPUID]->Sched.Proc->Captbl;
    else
        Captbl=Inv_Top->Proc->Captbl;
    
    /* Fast path - synchronous invocation activation */
    if(Svc_Num==RME_SVC_INV_ACT)
    {
        Retval=_RME_Inv_Act(Captbl, Reg      /* struct RME_Reg_Struct* Reg */,
                                    Param[0] /* cid_t Cap_Inv */,
                                    Param[1] /* ptr_t Param */);
        RME_SWITCH_RETURN(Reg,Retval);
    }
     
    /* See if this operation can potentially cause a register set switch. All the 
     * functions that may cause a register set switch is listed here. The behavior
     * of these functions shall be: If the function is successful, they shall
     * perform the return value saving on proper register stacks by themselves;
     * if the function fails, it should not conduct such return value saving. These
     * paths are less optimized than synchronous invocation, but are still optimized */
    switch(Svc_Num)
    {
        /* Send to a signal endpoint */
        case RME_SVC_SIG_SND:
        {
            Retval=_RME_Sig_Snd(Captbl, Reg      /* struct RME_Reg_Struct* Reg */,
                                        Param[0] /* cid_t Cap_Sig */);
            RME_SWITCH_RETURN(Reg,Retval);
        }
        /* Receive from a signal endpoint */
        case RME_SVC_SIG_RCV:
        {
            Retval=_RME_Sig_Rcv(Captbl, Reg      /* struct RME_Reg_Struct* Reg */,
                                        Param[0] /* cid_t Cap_Sig */);
            RME_SWITCH_RETURN(Reg,Retval);
        }
        /* Call kernel functions */
        case RME_SVC_KERN:
        {
            Retval=_RME_Kern_Act(Captbl, Reg                    /* struct RME_Reg_Struct* Reg */,
                                         Capid                  /* cid_t Cap_Kern */,
                                         RME_PARAM_D0(Param[0]) /* ptr_t Func_ID */,
                                         RME_PARAM_D1(Param[0]) /* ptr_t Sub_ID */,
                                         Param[1]               /* ptr_t Param1 */,
                                         Param[2]               /* ptr_t Param2 */);
            RME_SWITCH_RETURN(Reg,Retval);
        }
        /* Changing thread priority */
        case RME_SVC_THD_SCHED_PRIO:
        {
            Retval=_RME_Thd_Sched_Prio(Captbl, Reg      /* struct RME_Reg_Struct* Reg */,
                                               Param[0] /* cid_t Cap_Thd */,
                                               Param[1] /* ptr_t Prio */);
            RME_SWITCH_RETURN(Reg,Retval);
        }
        /* Free a thread from some core */
        case RME_SVC_THD_SCHED_FREE:
        {
            Retval=_RME_Thd_Sched_Free(Captbl, Reg      /* struct RME_Reg_Struct* Reg */,
                                               Param[0] /* cid_t Cap_Thd */);
            RME_SWITCH_RETURN(Reg,Retval);
        }
        /* Transfer time to a thread */
        case RME_SVC_THD_TIME_XFER:
        {
            Retval=_RME_Thd_Time_Xfer(Captbl, Reg      /* struct RME_Reg_Struct* Reg */,
                                              Param[0] /* cid_t Cap_Thd_Dst */,
                                              Param[1] /* cid_t Cap_Thd_Src */, 
                                              Param[2] /* ptr_t Time */);
            RME_SWITCH_RETURN(Reg,Retval);
        }
        /* Switch to another thread */
        case RME_SVC_THD_SWT:
        {
            Retval=_RME_Thd_Swt(Captbl, Reg      /* struct RME_Reg_Struct* Reg */,
                                        Param[0] /* cid_t Cap_Thd */,
                                        Param[1] /* ptr_t Full_Yield */);
            RME_SWITCH_RETURN(Reg, Retval);
        }
        default:break;
    } 

    /* It is guaranteed that these functions will never cause a context switch */
    switch(Svc_Num)
    {
        /* Capability table */
        case RME_SVC_CAPTBL_CRT:
        { 
            Retval=_RME_Captbl_Crt(Captbl, Capid                  /* cid_t Cap_Captbl_Crt */,
                                           RME_PARAM_D1(Param[0]) /* cid_t Cap_Kmem */,
                                           RME_PARAM_D0(Param[0]) /* cid_t Cap_Crt */,
                                           Param[1]               /* ptr_t Vaddr */,
                                           Param[2]               /* ptr_t Entry_Num */);
            break;
        }
        case RME_SVC_CAPTBL_DEL:
        {
            Retval=_RME_Captbl_Del(Captbl, Capid    /* cid_t Cap_Captbl_Del */,
                                           Param[0] /* cid_t Cap_Captbl */);
            break;
        }
        case RME_SVC_CAPTBL_FRZ:
        {
            Retval=_RME_Captbl_Frz(Captbl, Capid    /* cid_t Cap_Captbl_Frz */,
                                           Param[0] /* cid_t Cap_Frz */);
            break;
        }
        case RME_SVC_CAPTBL_ADD:
        {
            Retval=_RME_Captbl_Add(Captbl, RME_PARAM_D1(Param[0])  /* cid_t Cap_Captbl_Dst */,
                                           RME_PARAM_D0(Param[0])  /* cid_t Cap_Dst */,
                                           RME_PARAM_D1(Param[1])  /* cid_t Cap_Captbl_Src */,
                                           RME_PARAM_D0(Param[1])  /* cid_t Cap_Src */,
                                           Param[2]                /* ptr_t Flags */,
                                           RME_PARAM_KM(Svc,Capid) /* ptr_t Ext_Flags */);
            break;
        }
        case RME_SVC_CAPTBL_REM:
        {
            Retval=_RME_Captbl_Rem(Captbl, Capid    /* cid_t Cap_Captbl_Rem */,
                                           Param[0] /* cid_t Cap_Rem */);
            break;
        }
        /* Page table */
        case RME_SVC_PGTBL_CRT:
        {
            Retval=_RME_Pgtbl_Crt(Captbl, Capid                  /* cid_t Cap_Captbl */,
                                          RME_PARAM_D1(Param[0]) /* cid_t Cap_Kmem */,
                                          RME_PARAM_Q1(Param[0]) /* cid_t Cap_Pgtbl */,
                                          Param[1]               /* ptr_t Vaddr */,
                                          Param[2]               /* ptr_t Start_Addr */,
                                          RME_PARAM_PT(Param[2]) /* ptr_t Top_Flag */,
                                          RME_PARAM_Q0(Param[0]) /* ptr_t Size_Order */,
                                          RME_PARAM_PC(Svc)      /* ptr_t Num_Order */);
            break;
        }
        case RME_SVC_PGTBL_DEL:
        {
            Retval=_RME_Pgtbl_Del(Captbl, Capid    /* cid_t Cap_Captbl */,
                                          Param[0] /* cid_t Cap_Pgtbl */);
            break;
        }
        case RME_SVC_PGTBL_ADD:
        {
            Retval=_RME_Pgtbl_Add(Captbl, RME_PARAM_D1(Param[0]) /* cid_t Cap_Pgtbl_Dst */,
                                          RME_PARAM_D0(Param[0]) /* ptr_t Pos_Dst */,
                                          RME_PARAM_D1(Param[2]) /* ptr_t Flags_Dst */,
                                          RME_PARAM_D1(Param[1]) /* cid_t Cap_Pgtbl_Src */,
                                          RME_PARAM_D0(Param[1]) /* ptr_t Pos_Src */,
                                          RME_PARAM_D0(Param[2]) /* ptr_t Index */);
            break;
        }
        case RME_SVC_PGTBL_REM:
        {
            Retval=_RME_Pgtbl_Rem(Captbl, Param[0] /* cid_t Cap_Pgtbl */,
                                          Param[1] /* ptr_t Pos */);
            break;
        }
        case RME_SVC_PGTBL_CON:
        {
            Retval=_RME_Pgtbl_Con(Captbl, RME_PARAM_D1(Param[0]) /* cid_t Cap_Pgtbl_Parent */,
                                          Param[1]               /* ptr_t Pos */,
                                          RME_PARAM_D0(Param[0]) /* cid_t Cap_Pgtbl_Child */,
                                          Param[2]               /* ptr_t Flags_Child */);
            break;
        }
        case RME_SVC_PGTBL_DES:
        {
            Retval=_RME_Pgtbl_Des(Captbl, Param[0] /* cid_t Cap_Pgtbl */,
                                          Param[1] /* ptr_t Pos */);
            break;
        }
        /* Process */
        case RME_SVC_PROC_CRT:
        {
            Retval=_RME_Proc_Crt(Captbl, Capid                  /* cid_t Cap_Captbl_Crt */,
                                         RME_PARAM_D1(Param[0]) /* cid_t Cap_Kmem */,
                                         RME_PARAM_D0(Param[0]) /* cid_t Cap_Proc */,
                                         RME_PARAM_D1(Param[1]) /* cid_t Cap_Captbl */,
                                         RME_PARAM_D0(Param[1]) /* cid_t Cap_Pgtbl */,
                                         Param[2]               /* ptr_t Vaddr */);
            break;
        }
        case RME_SVC_PROC_DEL:
        {
            Retval=_RME_Proc_Del(Captbl, Capid    /* cid_t Cap_Captbl */,
                                         Param[0] /* cid_t Cap_Proc */);
            break;
        }
        case RME_SVC_PROC_CPT:
        {
            Retval=_RME_Proc_Cpt(Captbl, Param[0] /* cid_t Cap_Proc */,
                                         Param[1] /* cid_t Cap_Captbl */);
            break;
        }
        case RME_SVC_PROC_PGT:
        {
            Retval=_RME_Proc_Pgt(Captbl, Param[0] /* cid_t Cap_Proc */,
                                         Param[1] /* cid_t Cap_Pgtbl */);
            break;
        }
        /* Thread */
        case RME_SVC_THD_CRT:
        {
            Retval=_RME_Thd_Crt(Captbl, Capid                  /* cid_t Cap_Captbl */,
                                        RME_PARAM_D1(Param[0]) /* cid_t Cap_Kmem */,
                                        RME_PARAM_D0(Param[0]) /* cid_t Cap_Thd */,
                                        RME_PARAM_D1(Param[1]) /* cid_t Cap_Proc */,
                                        RME_PARAM_D0(Param[1]) /* ptr_t Max_Prio */,
                                        Param[2]               /* ptr_t Vaddr */);
            break;
        }
        case RME_SVC_THD_DEL:
        {
            Retval=_RME_Thd_Del(Captbl, Capid    /* cid_t Cap_Captbl */,
                                        Param[0] /* cid_t Cap_Thd */);
            break;
        }
        case RME_SVC_THD_EXEC_SET:
        {
            Retval=_RME_Thd_Exec_Set(Captbl, Capid    /* cid_t Cap_Thd */,
                                             Param[0] /* ptr_t Entry */,
                                             Param[1] /* ptr_t Stack */,
                                             Param[2] /* ptr_t Param */);
            break;
        }
        case RME_SVC_THD_HYP_SET:
        {
            Retval=_RME_Thd_Hyp_Set(Captbl, Param[0] /* cid_t Cap_Thd */,
                                            Param[1] /* ptr_t Kaddr */);
            break;
        }
        case RME_SVC_THD_SCHED_BIND:
        {
            Retval=_RME_Thd_Sched_Bind(Captbl, Param[0] /* cid_t Cap_Thd */,
                                               Param[1] /* cid_t Cap_Thd_Sched */, 
                                               Param[2] /* ptr_t Prio */);
            break;
        }
        case RME_SVC_THD_SCHED_RCV:
        {
            Retval=_RME_Thd_Sched_Rcv(Captbl, Param[0] /* cid_t Cap_Thd */);
            break;
        }
        /* Signal */
        case RME_SVC_SIG_CRT:
        {
            Retval=_RME_Sig_Crt(Captbl, Capid    /* cid_t Cap_Captbl */,
                                        Param[0] /* cid_t Cap_Kmem */,
                                        Param[1] /* cid_t Cap_Sig */, 
                                        Param[2] /* ptr_t Vaddr */);
            break;
        }
        case RME_SVC_SIG_DEL:
        {
            Retval=_RME_Sig_Del(Captbl, Capid    /* cid_t Cap_Captbl */,
                                        Param[0] /* cid_t Cap_Sig */);
            break;
        }
        /* Invocation */
        case RME_SVC_INV_CRT:
        {
            Retval=_RME_Inv_Crt(Captbl, Capid                  /* cid_t Cap_Captbl */,
                                        RME_PARAM_D1(Param[0]) /* cid_t Cap_Kmem */,
                                        RME_PARAM_D0(Param[0]) /* cid_t Cap_Inv */,
                                        Param[1]               /* cid_t Cap_Proc */,
                                        Param[2]               /* ptr_t Vaddr */);
            break;
        }
        case RME_SVC_INV_DEL:
        {
            Retval=_RME_Inv_Del(Captbl, Capid    /* cid_t Cap_Captbl */,
                                        Param[0] /* cid_t Cap_Inv */);
            break;
        }
        case RME_SVC_INV_SET:
        {
            Retval=_RME_Inv_Set(Captbl, RME_PARAM_D0(Param[0]) /* cid_t Cap_Inv */,
                                        Param[1]               /* ptr_t Entry */,
                                        Param[2]               /* ptr_t Stack */,
                                        RME_PARAM_D1(Param[0]) /* ptr_t Fault_Ret_Flag */);
            break;
        }
        /* This is an error */
        default: 
        {
            Retval=RME_ERR_CAP_NULL;
            break;
        }
    }
    /* We set the registers and return */
    __RME_Set_Syscall_Retval(Reg, Retval);
}
/* End Function:_RME_Svc_Handler *********************************************/

/* Begin Function:_RME_Tick_SMP_Handler ***************************************
Description : The system tick timer handler of RME, on all processors except for
              the main processor.
Input       : struct RME_Reg_Struct* Reg - The register set when entering the handler.
Output      : struct RME_Reg_Struct* Reg - The register set when exiting the handler.
Return      : None.
******************************************************************************/
void _RME_Tick_SMP_Handler(struct RME_Reg_Struct* Reg)
{
	ptr_t CPUID;
	struct RME_Thd_Struct* Next_Thd;

	CPUID=RME_CPUID();
	if(RME_Cur_Thd[CPUID]->Sched.Slices<RME_THD_INF_TIME)
	{
		/* Decrease timeslice count */
		RME_Cur_Thd[CPUID]->Sched.Slices--;
		/* See if the current thread's timeslice is used up */
		if(RME_Cur_Thd[CPUID]->Sched.Slices==0)
		{
			/* Running out of time. Kick this guy out and pick someone else */
			RME_Cur_Thd[CPUID]->Sched.State=RME_THD_TIMEOUT;
			/* Send a scheduler notification to its parent */
			_RME_Run_Notif(RME_Cur_Thd[CPUID]);
			_RME_Run_Del(RME_Cur_Thd[CPUID]);
			Next_Thd=_RME_Run_High(CPUID);
			RME_ASSERT(Next_Thd!=0);
			Next_Thd->Sched.State=RME_THD_RUNNING;
			/* Do a solid context switch, to the new guy */
			_RME_Run_Swt(Reg, RME_Cur_Thd[CPUID],Next_Thd);
			RME_Cur_Thd[CPUID]=Next_Thd;
		}
	}

	/* Send a signal to the kernel system ticker receive endpoint. This endpoint
	 * is per-core */
	_RME_Kern_Snd(Reg, RME_Tick_Sig[CPUID]);
}
/* End Function:_RME_Tick_SMP_Handler ****************************************/

/* Begin Function:_RME_Tick_Handler *******************************************
Description : The system tick timer handler of RME.
Input       : struct RME_Reg_Struct* Reg - The register set when entering the handler.
Output      : struct RME_Reg_Struct* Reg - The register set when exiting the handler.
Return      : None.
******************************************************************************/
void _RME_Tick_Handler(struct RME_Reg_Struct* Reg)
{
    /* Increase the tick count */
    RME_Timestamp++;
    /* Call generic handler */
    _RME_Tick_SMP_Handler(Reg);
}
/* End Function:_RME_Tick_Handler ********************************************/

/* Begin Function:__RME_Low_Level_Check ***************************************
Description : Do some low-level checking for the operating system.
Input       : None.
Output      : None.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t __RME_Low_Level_Check(void)
{
    /* Make sure the machine is more than 32-bit */
    RME_ASSERT(RME_WORD_ORDER>=5);
    /* Check if the word order setting is correct */
    RME_ASSERT(RME_WORD_BITS==RME_POW2(RME_WORD_ORDER));
    /* Check if the struct sizes are correct */
    RME_ASSERT(sizeof(struct RME_Cap_Struct)==RME_CAP_SIZE);
    RME_ASSERT(sizeof(struct RME_Cap_Captbl)==RME_CAP_SIZE);
    RME_ASSERT(sizeof(struct RME_Cap_Pgtbl)==RME_CAP_SIZE);
    RME_ASSERT(sizeof(struct RME_Cap_Proc)==RME_CAP_SIZE);
    RME_ASSERT(sizeof(struct RME_Cap_Thd)==RME_CAP_SIZE);
    RME_ASSERT(sizeof(struct RME_Cap_Sig)==RME_CAP_SIZE);
    RME_ASSERT(sizeof(struct RME_Cap_Inv)==RME_CAP_SIZE);
    RME_ASSERT(sizeof(struct RME_Cap_Kern)==RME_CAP_SIZE);
    RME_ASSERT(sizeof(struct RME_Cap_Kmem)==RME_CAP_SIZE);
    /* Check if the other configurations are correct */
    /* Kernel memory allocation minimal size aligned to word boundary */
    RME_ASSERT(RME_KMEM_SLOT_ORDER>=RME_WORD_ORDER-3);
    /* Make sure the number of priorities does not exceed half-word boundary */
    RME_ASSERT(RME_MAX_PREEMPT_PRIO<=RME_POW2(RME_WORD_BITS>>1));
    return 0;
}
/* End Function:__RME_Low_Level_Check ****************************************/

/* Begin Function:RME_Kmain ***************************************************
Description : The entrance of the operating system.
Input       : None.
Output      : None.
Return      : ret_t - This function never returns.
******************************************************************************/
ret_t RME_Kmain(void)
{
    /* Disable all interrupts first */
    __RME_Disable_Int();
    /* Some low-level checks to make sure the correctness of the core */
    __RME_Low_Level_Check();
    /* Hardware low-level init */
    __RME_Low_Level_Init();
    /* Initialize the kernel page tables */
    __RME_Pgtbl_Kmem_Init();
    
    /* Initialize the kernel object allocation table - default init */
    _RME_Kotbl_Init(RME_KOTBL_WORD_NUM);
    /* Initialize system calls, and kernel timestamp counter */
    _RME_Syscall_Init();
    /* Initialize process/threads control module */
    _RME_Prcthd_Init();
    
    /* Boot into the first process, and handle it all the other cases&enable the interrupt */
    __RME_Boot();
    
    /* Should never reach here. If it reached here, we reboot */
    __RME_Reboot();
    
    return 0;
}
/* End Function:RME_Kmain ****************************************************/

/* Begin Function:RME_Print_Int ***********************************************
Description : Print a signed integer on the debugging console. This integer is
              printed as decimal with sign.
Input       : cnt_t Int - The integer to print.
Output      : None.
Return      : cnt_t - The length of the string printed.
******************************************************************************/
cnt_t RME_Print_Int(cnt_t Int)
{
    ptr_t Iter;
    cnt_t Count;
    cnt_t Num;
    ptr_t Div;
    
    /* how many digits are there? */
    if(Int==0)
    {
        __RME_Putchar('0');
        return 1;
    }
    else if(Int<0)
    {
        /* How many digits are there? */
        Count=0;
        Div=1;
        Iter=-Int;
        while(Iter!=0)
        {
            Iter/=10;
            Count++;
            Div*=10;
        }
        Div/=10;
        
        __RME_Putchar('-');
        Iter=-Int;
        Num=Count+1;
        
        while(Count>0)
        {
            Count--;
            __RME_Putchar(Iter/Div+'0');
            Iter=Iter%Div;
            Div/=10;
        }
    }
    else
    {
        /* How many digits are there? */
        Count=0;
        Div=1;
        Iter=Int;
        while(Iter!=0)
        {
            Iter/=10;
            Count++;
            Div*=10;
        }
        Div/=10;
        
        Iter=Int;
        Num=Count;
        
        while(Count>0)
        {
            Count--;
            __RME_Putchar(Iter/Div+'0');
            Iter=Iter%Div;
            Div/=10;
        }
    }
    
    return Num;
}
/* End Function:RME_Print_Int ************************************************/

/* Begin Function:RME_Print_Uint **********************************************
Description : Print a unsigned integer on the debugging console. This integer is
              printed as hexadecimal.
Input       : ptr_t Uint - The unsigned integer to print.
Output      : None.
Return      : cnt_t - The length of the string printed.
******************************************************************************/
cnt_t RME_Print_Uint(ptr_t Uint)
{
    ptr_t Iter;
    cnt_t Count;
    cnt_t Num;
    
    /* how many digits are there? */
    if(Uint==0)
    {
        __RME_Putchar('0');
        return 1;
    }
    else
    {
        /* Filter out all the zeroes */
        Count=0;
        Iter=Uint;
        while((Iter>>((sizeof(ptr_t)*8)-4))==0)
        {
            Iter<<=4;
            Count++;
        }
        /* Count is the number of pts to print */
        Count=sizeof(ptr_t)*2-Count;
        Num=Count;
        while(Count>0)
        {
            Count--;
            Iter=(Uint>>(Count*4))&0x0F;
            if(Iter<10)
                __RME_Putchar('0'+Iter);
            else
                __RME_Putchar('A'+Iter-10);
        }
    }
    
    return Num;
}
/* End Function:RME_Print_Uint ***********************************************/

/* Begin Function:RME_Print_String ********************************************
Description : Print a string the kernel console.
              This is only used for kernel-level debugging.
Input       : s8* String - The string to print
Output      : None.
Return      : cnt_t - The length of the string printed, the '\0' is not included.
******************************************************************************/
cnt_t RME_Print_String(s8* String)
{
    cnt_t Count;
    
    Count=0;
    while(Count<RME_KERNEL_DEBUG_MAX_STR)
    {
        if(String[Count]=='\0')
            break;
        
        __RME_Putchar(String[Count++]);
    }
    
    return Count;
}
/* End Function:RME_Print_String *********************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
