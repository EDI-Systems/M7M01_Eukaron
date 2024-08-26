/******************************************************************************
Filename    : rme_platform_rv32p.c
Author      : pry
Date        : 01/04/2017
Licence     : The Unlicense; see LICENSE for details.
Description : The hardware abstraction layer for RV32P PMP-based microcontrollers.
              This supports only one core, and with MPU. If you'd have a MMU,
              use the RV32GV port instead. This port is only aware of two modes,
              M and U. The S-mode which supports MMU is not honored.

* Generic Code Section ********************************************************
Small utility functions that can be either implemented with C or assembly, and 
the entry of the kernel. Also responsible for debug printing and CPUID getting.

* Handler Code Section ********************************************************
Contains fault handlers, generic interrupt handlers and kernel function handlers.

* Initialization Code Section *************************************************
Low-level initialization and booting. If we have multiple processors, the 
booting of all processors are dealt with here.

* Register Manipulation Section ***********************************************
Low-level register manipulations and parameter extractions.

* Page Table Section **********************************************************
Page table related operations are all here.
The page table implementation conforms to style II which allows sharing of page
tables and infinite number of dynamic/static pages. The STATIC attribute here
just denotes the reluctance for being evicted.
******************************************************************************/

/* Include *******************************************************************/
#define __HDR_DEF__
#include "Platform/RV32P/rme_platform_rv32p.h"
#include "Kernel/rme_kernel.h"
#undef __HDR_DEF__

#define __HDR_STRUCT__
#include "Platform/RV32P/rme_platform_rv32p.h"
#include "Kernel/rme_kernel.h"
#undef __HDR_STRUCT__

/* Private include */
#include "Platform/RV32P/rme_platform_rv32p.h"

#define __HDR_PUBLIC__
#include "Kernel/rme_kernel.h"
#undef __HDR_PUBLIC__
/* End Include ***************************************************************/

/* Function:main **************************************************************
Description : The entry of the operating system. This function is for 
              compatibility with the existing toolchains.
Input       : None.
Output      : None.
Return      : int - Dummy value, this function never returns.
******************************************************************************/
int main(void)
{
    /* The main function of the kernel - we will start our kernel boot here */
    RME_Kmain();
    return 0;
}
/* End Function:main *********************************************************/

/* Function:__RME_Putchar *****************************************************
Description : Output a character to console.
Input       : char Char - The character to print.
Output      : None.
Return      : rme_ptr_t - Always 0.
******************************************************************************/
#if(RME_DBGLOG_ENABLE!=0U)
rme_ptr_t __RME_Putchar(char Char)
{
    RME_RV32P_PUTCHAR(Char);
    return 0U;
}
#endif
/* End Function:__RME_Putchar ************************************************/

/* Function:__RME_CPUID_Get ***************************************************
Description : Get the CPUID. This is to identify where we are executing.
              Currently this only supports one core.
Input       : None.
Output      : None.
Return      : rme_ptr_t - The CPUID.
******************************************************************************/
rme_ptr_t __RME_CPUID_Get(void)
{
    return 0U;
}
/* End Function:__RME_CPUID_Get **********************************************/

/* Function:__RME_RV32P_Wait_Int **********************************************
Description : Wait until a new interrupt comes, to save power.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_RV32P_Wait_Int(void)
{
    /* Calling functions implemented by risc-v library */
    RME_RV32P_WAIT_INT();
}
/* End Function:__RME_RV32P_Wait_Int *****************************************/

/* Function:__RME_RV32P_Exc_Handler *******************************************
Description : The fault handler of RME.
Input       : struct RME_Reg_Struct* Reg - The register set.
              rme_ptr_t Mcause - The machine cause register.
Output      : struct RME_Reg_Struct* Reg - The updated register set.
Return      : None.
******************************************************************************/
void __RME_RV32P_Exc_Handler(struct RME_Reg_Struct* Reg,
                             rme_ptr_t Mcause)
{
    rme_ptr_t Mtval;
#if(RME_PGT_RAW_ENABLE==0U)
    rme_ptr_t Paddr;
    rme_ptr_t Size_Order;
    rme_ptr_t Flag;
    struct RME_Cap_Prc* Prc;
    struct RME_Inv_Struct* Inv_Top;
    struct __RME_RV32P_Pgt_Meta* Meta;
#endif
    struct RME_Thd_Struct* Thd_Cur;
    struct RME_Exc_Struct* Exc;

    /* Is it a kernel-level fault? If yes, panic */
    RME_ASSERT((Reg->MSTATUS&RME_RV32P_MSTATUS_RET_KERNEL)==0U);

    /* Get the address of this faulty address, and what caused this fault */
    Thd_Cur=RME_RV32P_Local.Thd_Cur;
    Exc=&Thd_Cur->Ctx.Reg->Exc;
    Mtval=___RME_RV32P_MTVAL_Get();

#if(RME_PGT_RAW_ENABLE==0U)
    /* What fault is it? */
    switch(Mcause)
    {
        /* We cannot recover from the following errors, have to kill the thread */
        default:
        case RME_RV32P_MCAUSE_IMALIGN:
        case RME_RV32P_MCAUSE_IUNDEF:
        case RME_RV32P_MCAUSE_IBRKPT:
        case RME_RV32P_MCAUSE_LALIGN:
        case RME_RV32P_MCAUSE_SALIGN:
        case RME_RV32P_MCAUSE_U_ECALL:
        case RME_RV32P_MCAUSE_S_ECALL:
        case RME_RV32P_MCAUSE_H_ECALL:
        case RME_RV32P_MCAUSE_M_ECALL:
        case RME_RV32P_MCAUSE_IPGFLT:
        case RME_RV32P_MCAUSE_LPGFLT:
        case RME_RV32P_MCAUSE_SPGFLT:
        {
            Exc->Cause=Mcause;
            Exc->Addr=Reg->PC;
            Exc->Value=Mtval;
            _RME_Thd_Fatal(Reg);
            return;
        }
        /* Try to recover from PMP issues - could be dynamic pages */
        case RME_RV32P_MCAUSE_IACCFLT:
        case RME_RV32P_MCAUSE_LACCFLT:
        case RME_RV32P_MCAUSE_SACCFLT: break;
    }

    Inv_Top=RME_INVSTK_TOP(Thd_Cur);
    if(Inv_Top==RME_NULL)
        Prc=Thd_Cur->Sched.Prc;
    else
        Prc=Inv_Top->Prc;

    /* The page is not found, kill the thread */
    if(__RME_Pgt_Walk(Prc->Pgt,Mtval,0U,0U,&Paddr,&Size_Order,0U,&Flag)!=0)
    {
        Exc->Cause=Mcause;
        Exc->Addr=Reg->PC;
        Exc->Value=Mtval;
        _RME_Thd_Fatal(Reg);
        return;
    }

    /* We found the page, and need to update the PMP cache. It could be:
     * (1) a dynamic page,
     * (2) a static page on first occurrence,
     * (3) a static page swapping out another static page. */
    Meta=RME_CAP_GETOBJ(Prc->Pgt,struct __RME_RV32P_Pgt_Meta*);
    if(___RME_RV32P_PMP_Update(Meta,Paddr,Size_Order,Flag)!=0)
    {
        Exc->Cause=Mcause;
        Exc->Addr=Reg->PC;
        Exc->Value=Mtval;
        _RME_Thd_Fatal(Reg);
        return;
    }

    /* Reload current PMP data */
    __RME_Pgt_Set(Prc->Pgt);
#else
    Exc->Cause=Mcause;
    Exc->Addr=Reg->PC;
    Exc->Value=Mtval;
    _RME_Thd_Fatal(Reg);
#endif
}
/* End Function:__RME_RV32P_Exc_Handler **************************************/

/* Function:__RME_RV32P_Flag_Fast *********************************************
Description : Set a fast flag in a flag set. Works for timer interrupts only.
Input       : rme_ptr_t Base - The base address of the flagset.
              rme_ptr_t Size - The size of the flagset.
              rme_ptr_t Flag - The fast flagset to program.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_RV32P_Flag_Fast(rme_ptr_t Base,
                           rme_ptr_t Size,
                           rme_ptr_t Flag)
{
    volatile struct __RME_RVM_Flag* Set;

    /* Choose a data structure that is not locked at the moment */
    Set=RME_RVM_FLAG_SET(Base,Size,0U);
    if(Set->Lock!=0U)
        Set=RME_RVM_FLAG_SET(Base,Size,1U);

    /* Set the flags for this interrupt source */
    Set->Fast|=Flag;
}
/* End Function:__RME_RV32P_Flag_Fast ****************************************/

/* Function:__RME_RV32P_Flag_Slow *********************************************
Description : Set a slow flag in a flag set. Works for both vectors and events.
Input       : rme_ptr_t Base - The base address of the flagset.
              rme_ptr_t Size - The size of the flagset.
              rme_ptr_t Pos - The position in the flagset to set.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_RV32P_Flag_Slow(rme_ptr_t Base,
                           rme_ptr_t Size,
                           rme_ptr_t Pos)
{
    volatile struct __RME_RVM_Flag* Set;
    
    /* Choose a data structure that is not locked at the moment */
    Set=RME_RVM_FLAG_SET(Base,Size,0U);
    if(Set->Lock!=0U)
        Set=RME_RVM_FLAG_SET(Base,Size,1U);
    
    /* Set the flags for this interrupt source */
    Set->Group|=RME_POW2(Pos>>RME_WORD_ORDER);
    Set->Flag[Pos>>RME_WORD_ORDER]|=RME_POW2(Pos&RME_MASK_END(RME_WORD_ORDER-1U));
}
/* End Function:__RME_RV32P_Flag_Slow ****************************************/

/* Function:_RME_RV32P_Handler ************************************************
Description : The generic interrupt handler of RME for RV32P, in C.
Input       : struct RME_Reg_Struct* Reg - The register set.
Output      : struct RME_Reg_Struct* Reg - The updated register set.
Return      : None.
******************************************************************************/
void _RME_RV32P_Handler(struct RME_Reg_Struct* Reg)
{
    rme_ptr_t Mcause;

    Mcause=___RME_RV32P_MCAUSE_Get();

    /* Timer handler */
    if(Mcause==(RME_RV32P_MCAUSE_TIM|RME_RV32P_MCAUSE_INT))
    {
        RME_RV32P_Timestamp++;

#if(RME_RVM_GEN_ENABLE!=0U)
        __RME_RV32P_Flag_Fast(RME_RVM_PHYS_VCTF_BASE,RME_RVM_PHYS_VCTF_SIZE,1U);
#endif

        _RME_Tim_Handler(Reg,1U);
    }
    /* Vector handler */
    else if((Mcause&RME_RV32P_MCAUSE_INT)!=0U)
    {
        Mcause&=0x7FFFFFFFU;

#if(RME_RVM_GEN_ENABLE!=0U)
        /* If the user wants to bypass, we skip the flag marshalling & sending process */
        if(RME_Boot_Vct_Handler(Reg,Mcause)!=0U)
        {
            /* Set the vector flag */
            __RME_RV32P_Flag_Slow(RME_RVM_PHYS_VCTF_BASE,RME_RVM_PHYS_VCTF_SIZE,Mcause);
#endif
            /* Send to the kernel endpoint */
            _RME_Kern_Snd(RME_RV32P_Local.Sig_Vct);
            /* Pick the highest priority thread after we did all sends */
            _RME_Kern_High(Reg,&RME_RV32P_Local);
#if(RME_RVM_GEN_ENABLE!=0U)
        }
#endif
    }
    /* System call handler */
    else if(Mcause==RME_RV32P_MCAUSE_U_ECALL)
    {
        /* Need to skip the ECALL */
        Reg->PC+=RME_WORD_BYTE;
        _RME_Svc_Handler(Reg);
    }
    /* Exception handler */
    else
    {
        /* Attempt to reexecute the faulting instruction */
        __RME_RV32P_Exc_Handler(Reg,Mcause);
    }

    /* Never ever allow any modifications to MSTATUS bypass checks: modifying
     * return states, or allow a FPU-disabled thread to access FPU */
    RME_RV32P_MSTATUS_FIX(Reg);
}
/* End Function:_RME_RV32P_Handler *******************************************/

/* Function:__RME_RV32P_Pgt_Entry_Mod *****************************************
Description : Consult or modify the page table attributes. RV32P only allows
              consulting page table attributes but does not allow modifying them,
              because there are no architecture-specific flags.
Input       : struct RME_Cap_Cpt* Cpt - The current capability table.
              rme_cid_t Cap_Pgt - The capability to the top-level page table to consult.
              rme_tid_t Vaddr - The virtual address to consult.
              rme_tid_t Type - The consult type, see manual for details.
Output      : None.
Return      : rme_ret_t - If successful, the flags; else RME_ERR_KFN_FAIL.
******************************************************************************/
#if(RME_PGT_RAW_ENABLE==0U)
rme_ret_t __RME_RV32P_Pgt_Entry_Mod(struct RME_Cap_Cpt* Cpt,
                                    rme_cid_t Cap_Pgt,
                                    rme_ptr_t Vaddr,
                                    rme_ptr_t Type)
{
    struct RME_Cap_Pgt* Pgt_Op;
    rme_ptr_t Type_Stat;
    rme_ptr_t Size_Order;
    rme_ptr_t Num_Order;
    rme_ptr_t Flags;
    
    /* Get the capability slot */
    RME_CPT_GETCAP(Cpt,Cap_Pgt,RME_CAP_TYPE_PGT,
                   struct RME_Cap_Pgt*,Pgt_Op,Type_Stat);
    
    Size_Order=0U;
    Num_Order=0U;
    Flags=0U;
    if(__RME_Pgt_Walk(Pgt_Op,Vaddr,0U,0U,0U,&Size_Order,&Num_Order,&Flags)!=0U)
        return RME_ERR_KFN_FAIL;
    
    switch(Type)
    {
        case RME_RV32P_KFN_PGT_ENTRY_MOD_FLAG_GET: return (rme_ret_t)Flags;
        case RME_RV32P_KFN_PGT_ENTRY_MOD_SZORD_GET: return (rme_ret_t)Size_Order;
        case RME_RV32P_KFN_PGT_ENTRY_MOD_NMORD_GET: return (rme_ret_t)Num_Order;
        default:break;
    }
    
    return RME_ERR_KFN_FAIL;
}
#endif
/* End Function:__RME_RV32P_Pgt_Entry_Mod ************************************/

/* Function:__RME_RV32P_Int_Local_Mod *****************************************
Description : Consult or modify the local interrupt controller's vector state.
Input       : rme_tid_t Int_Num - The interrupt number to consult or modify.
              rme_tid_t Operation - The operation to conduct.
              rme_tid_t Param - The parameter, could be state or priority.
Output      : None.
Return      : rme_ret_t - If successful, 0 or the desired value; else RME_ERR_KFN_FAIL.
******************************************************************************/
rme_ret_t __RME_RV32P_Int_Local_Mod(rme_ptr_t Int_Num,
                                    rme_ptr_t Operation,
                                    rme_ptr_t Param)
{
    if(Int_Num>=RME_RVM_PHYS_VCT_NUM)
        return RME_ERR_KFN_FAIL;
    
    /* Call header for chip-specific operations */
    switch(Operation)
    {
        case RME_RV32P_KFN_INT_LOCAL_MOD_STATE_GET:
        {
            return RME_RV32P_INT_STATE_GET(Int_Num);
        }
        case RME_RV32P_KFN_INT_LOCAL_MOD_STATE_SET:
        {
            if(Param==0U)
                RME_RV32P_INT_STATE_DISABLE(Int_Num);
            else
                RME_RV32P_INT_STATE_ENABLE(Int_Num);
            break;
        }
        case RME_RV32P_KFN_INT_LOCAL_MOD_PRIO_GET:
        {
            RME_RV32P_INT_PRIO_GET(Int_Num);
            break;
        }
        case RME_RV32P_KFN_INT_LOCAL_MOD_PRIO_SET:
        {
            RME_RV32P_INT_PRIO_SET(Int_Num,Param);
            break;
        }
        default:return RME_ERR_KFN_FAIL;
    }

    return 0;
}
/* End Function:__RME_RV32P_Int_Local_Mod ************************************/

/* Function:__RME_RV32P_Int_Local_Trig ****************************************
Description : Trigger a CPU's local event source.
Input       : rme_ptr_t CPUID - The ID of the CPU.
              rme_ptr_t Evt_Num - The event ID.
Output      : None.
Return      : rme_ret_t - If successful, 0; else RME_ERR_KFN_FAIL.
******************************************************************************/
rme_ret_t __RME_RV32P_Int_Local_Trig(rme_ptr_t CPUID,
                                     rme_ptr_t Int_Num)
{
    if(Int_Num>=RME_RVM_PHYS_VCT_NUM)
        return RME_ERR_KFN_FAIL;

    /* Have to call the header - this is chip specific */
    RME_RV32P_INT_LOCAL_TRIG(Int_Num);

    return 0;
}
/* End Function:__RME_RV32P_Int_Local_Trig ***********************************/

/* Function:__RME_RV32P_Evt_Local_Trig ****************************************
Description : Trigger a CPU's local event source.
Input       : struct RME_Reg_Struct* Reg - The register set.
              rme_ptr_t CPUID - The ID of the CPU.
              rme_ptr_t Evt_Num - The event ID.
Output      : None.
Return      : rme_ret_t - If successful, 0; else RME_ERR_KFN_FAIL.
******************************************************************************/
rme_ret_t __RME_RV32P_Evt_Local_Trig(struct RME_Reg_Struct* Reg,
                                     rme_ptr_t CPUID,
                                     rme_ptr_t Evt_Num)
{
    if(Evt_Num>=RME_RVM_VIRT_EVT_NUM)
        return RME_ERR_KFN_FAIL;

    __RME_RV32P_Flag_Slow(RME_RVM_VIRT_EVTF_BASE,RME_RVM_VIRT_EVTF_SIZE,Evt_Num);
    
    if(_RME_Kern_Snd(RME_RV32P_Local.Sig_Vct)!=0U)
        return RME_ERR_KFN_FAIL;
    
    /* Set return value first before we really do context switch */
    __RME_Svc_Retval_Set(Reg,0);
    
    _RME_Kern_High(Reg,&RME_RV32P_Local);

    return 0U;
}
/* End Function:__RME_RV32P_Evt_Local_Trig ***********************************/

/* Function:__RME_RV32P_Cache_Mod *********************************************
Description : Modify cache state. We do not make assumptions about cache contents.
Input       : rme_ptr_t Cache_ID - The ID of the cache to enable, disable or consult.
              rme_ptr_t Operation - The operation to perform.
              rme_ptr_t Param - The parameter.
Output      : None.
Return      : If successful, 0; else RME_ERR_KFN_FAIL.
******************************************************************************/
rme_ret_t __RME_RV32P_Cache_Mod(rme_ptr_t Cache_ID,
                                rme_ptr_t Operation,
                                rme_ptr_t Param)
{
    RME_RV32P_CACHE_MOD(Cache_ID, Operation, Param);

    return 0;
}
/* End Function:__RME_RV32P_Cache_Mod ****************************************/

/* Function:__RME_RV32P_Cache_Maint *******************************************
Description : Do cache maintenance. Integrity of the data is always protected.
Input       : rme_ptr_t Cache_ID - The ID of the cache to do maintenance on.
              rme_ptr_t Operation - The maintenance operation to perform.
              rme_ptr_t Param - The parameter for that operation.
Output      : None.
Return      : If successful, 0; else RME_ERR_KFN_FAIL.
******************************************************************************/
rme_ret_t __RME_RV32P_Cache_Maint(rme_ptr_t Cache_ID,
                                  rme_ptr_t Operation,
                                  rme_ptr_t Param)
{
    RME_RV32P_CACHE_MAINT(Cache_ID,Operation,Param);

    return 0;
}
/* End Function:__RME_RV32P_Cache_Maint **************************************/

/* Function:__RME_RV32P_Prfth_Mod *********************************************
Description : Modify prefetcher state.
Input       : rme_ptr_t Prfth_ID - The ID of the prefetcher to enable, disable or consult.
              rme_ptr_t Operation - The operation to perform.
              rme_ptr_t Param - The parameter.
Output      : None.
Return      : If successful, 0; else RME_ERR_KFN_FAIL.
******************************************************************************/
rme_ret_t __RME_RV32P_Prfth_Mod(rme_ptr_t Prfth_ID,
                                rme_ptr_t Operation,
                                rme_ptr_t Param)
{
    RME_RV32P_PRFTH_MOD(Prfth_ID,Operation,Param);

    return  0;
}
/* End Function:__RME_RV32P_Prfth_Mod ****************************************/

/* Function:__RME_RV32P_Perf_CPU_Func *****************************************
Description : CPU feature detection for RV32P.
Input       : struct RME_Reg_Struct* Reg - The register set.
              rme_ptr_t Freg_ID - The capability to the thread to consult.
Output      : struct RME_Reg_Struct* Reg - The updated register set.
Return      : rme_ret_t - If successful, 0; if a negative value, failed.
******************************************************************************/
rme_ret_t __RME_RV32P_Perf_CPU_Func(struct RME_Reg_Struct* Reg,
                                    rme_ptr_t Freg_ID)
{
    /* Read dedicated feature register, but musk out bit width - we already know */
    return ___RME_RV32P_MISA_Get()&0x3FFFFFFFU;
}
/* End Function:__RME_RV32P_Perf_CPU_Func ************************************/

/* Function:__RME_RV32P_Perf_Mon_Mod ******************************************
Description : Read or write performance monitor settings.
Input       : rme_ptr_t Perf_ID - The performance monitor identifier.
              rme_ptr_t Operation - The operation to perform.
              rme_ptr_t Param - An extra parameter.
Output      : None.
Return      : rme_ret_t - If successful, the desired value; if a negative value, failed.
******************************************************************************/
rme_ret_t __RME_RV32P_Perf_Mon_Mod(rme_ptr_t Perf_ID,
                                   rme_ptr_t Operation,
                                   rme_ptr_t Param)
{
    RME_RV32P_PERF_MON_MOD(Perf_ID,Operation,Param);

    return 0;
}
/* End Function:__RME_RV32P_Perf_Mon_Mod *************************************/

/* Function:__RME_RV32P_Perf_Cycle_Mod ****************************************
Description : Cycle performance counter read or write.
Input       : struct RME_Reg_Struct* Reg - The current register set.
              rme_ptr_t Cycle_ID - The performance counter to read or write.
Output      : struct RME_Reg_Struct* Reg - The register set when exiting the handler.
Return      : rme_ret_t - If successful, 0; if a negative value, failed.
******************************************************************************/
rme_ret_t __RME_RV32P_Perf_Cycle_Mod(struct RME_Reg_Struct* Reg,
                                     rme_ptr_t Cycle_ID,
                                     rme_ptr_t Operation,
                                     rme_ptr_t Value)
{
    RME_RV32P_PERF_CYCLE_MOD(Cycle_ID,Operation,Value);

    return 0;
}
/* End Function:__RME_RV32P_Perf_Cycle_Mod ***********************************/

/* Function:__RME_RV32P_Debug_Reg_Mod *****************************************
Description : Debug regular register modification implementation for RV32P.
Input       : struct RME_Cap_Cpt* Cpt - The current capability table.
              struct RME_Reg_Struct* Reg - The current register set.
              rme_cid_t Cap_Thd - The capability to the thread to consult.
              rme_ptr_t Operation - The operation, e.g. which register to read or write.
              rme_ptr_t Value - The value to write into the register.
Output      : struct RME_Reg_Struct* Reg - The register set when exiting the handler.
Return      : rme_ret_t - If successful, 0; if a negative value, failed.
******************************************************************************/
rme_ret_t __RME_RV32P_Debug_Reg_Mod(struct RME_Cap_Cpt* Cpt,
                                    struct RME_Reg_Struct* Reg,
                                    rme_cid_t Cap_Thd,
                                    rme_ptr_t Operation,
                                    rme_ptr_t Value)
{
    struct RME_Cap_Thd* Thd_Op;
    struct RME_Thd_Struct* Thd_Struct;
    struct RME_CPU_Local* Local;
    rme_ptr_t* Position;
    rme_ptr_t Register;
    rme_ptr_t Type_Stat;

    /* Get the capability slot */
    RME_CPT_GETCAP(Cpt,Cap_Thd,RME_CAP_TYPE_THD,
                   struct RME_Cap_Thd*,Thd_Op,Type_Stat);

    /* See if the target thread is already bound. If no or bound to other cores, we just quit */
    Local=RME_CPU_LOCAL();
    Thd_Struct=(struct RME_Thd_Struct*)Thd_Op->Head.Object;
    if(Thd_Struct->Sched.Local!=Local)
        return RME_ERR_PTH_INVSTATE;

    /* Register validity check */
    Register=Operation&(~RME_RV32P_KFN_DEBUG_REG_MOD_SET);
    if(Register<=RME_RV32P_KFN_DEBUG_REG_MOD_X31_T6)
    {
        Position=(rme_ptr_t*)&(Thd_Struct->Ctx.Reg->Reg);
        Position=&(Position[Register]);
    }
    /* See if this thread have FPU context. */
#if(RME_COP_NUM!=0U)
    else if((RME_THD_ATTR(Thd_Struct->Ctx.Hyp_Attr)!=0U)&&
            (Register<=RME_RV32P_KFN_DEBUG_REG_MOD_F31))
    {
        Position=(rme_ptr_t*)&(Thd_Struct->Ctx.Reg->Cop);
#if(RME_RV32P_COP_RVD!=0U)
        /* Double-precision FPU */
        if(Register!=RME_RV32P_KFN_DEBUG_REG_MOD_FCSR)
            Position=&(Position[(Register-RME_RV32P_KFN_DEBUG_REG_MOD_FCSR)*2U-1U]);
#else
        /* Single-precision FPU */
        Position=&(Position[Register-RME_RV32P_KFN_DEBUG_REG_MOD_FCSR]);
#endif
    }
#endif
    else
        return RME_ERR_KFN_FAIL;

    /* Perform read/write */
    if((Operation&RME_RV32P_KFN_DEBUG_REG_MOD_SET)==0U)
    {
        Reg->X11_A1=Position[0];
#if((RME_COP_NUM!=0U)&&(RME_RV32P_COP_RVD!=0U))
        if(Register>=RME_RV32P_KFN_DEBUG_REG_MOD_F0)
            Reg->X12_A2=Position[1];
#endif
    }
    else
    {
        Position[0]=Reg->X11_A1;
#if((RME_COP_NUM!=0U)&&(RME_RV32P_COP_RVD!=0U))
        if(Register>=RME_RV32P_KFN_DEBUG_REG_MOD_F0)
            Position[1]=Reg->X12_A2;
#endif
    }

    return 0;
}
/* End Function:__RME_RV32P_Debug_Reg_Mod ************************************/

/* Function:__RME_RV32P_Debug_Inv_Mod *****************************************
Description : Debug invocation register modification implementation for RV32P.
Input       : struct RME_Cap_Cpt* Cpt - The current capability table.
              struct RME_Reg_Struct* Reg - The current register set.
              rme_cid_t Cap_Thd - The capability to the thread to consult.
              rme_ptr_t Operation - The operation, e.g. which register to read or write.
              rme_ptr_t Value - The value to write into the register.
Output      : struct RME_Reg_Struct* Reg - The register set when exiting the handler.
Return      : rme_ret_t - If successful, 0; if a negative value, failed.
******************************************************************************/
rme_ret_t __RME_RV32P_Debug_Inv_Mod(struct RME_Cap_Cpt* Cpt,
                                    struct RME_Reg_Struct* Reg,
                                    rme_cid_t Cap_Thd,
                                    rme_ptr_t Operation,
                                    rme_ptr_t Value)
{
    struct RME_Cap_Thd* Thd_Op;
    struct RME_Thd_Struct* Thd_Struct;
    struct RME_Inv_Struct* Inv_Struct;
    struct RME_CPU_Local* Local;
    rme_ptr_t Type_Stat;
    rme_ptr_t Layer_Cnt;

    /* Get the capability slot */
    RME_CPT_GETCAP(Cpt,Cap_Thd,RME_CAP_TYPE_THD,
                   struct RME_Cap_Thd*,Thd_Op,Type_Stat);

    /* See if the target thread is already binded. If no or binded to other cores, we just quit */
    Local=RME_CPU_LOCAL();
    Thd_Struct=(struct RME_Thd_Struct*)Thd_Op->Head.Object;
    if(Thd_Struct->Sched.Local!=Local)
        return RME_ERR_PTH_INVSTATE;

    /* Find whatever position we require - Layer 0 is the first layer (stack top), and so on */
    Layer_Cnt=RME_PARAM_D1(Operation);
    Inv_Struct=(struct RME_Inv_Struct*)(Thd_Struct->Ctx.Invstk.Next);
    while(1U)
    {
        if(Inv_Struct==(struct RME_Inv_Struct*)&(Thd_Struct->Ctx.Invstk))
            return RME_ERR_KFN_FAIL;

        if(Layer_Cnt==0U)
            break;

        Layer_Cnt--;
        Inv_Struct=(struct RME_Inv_Struct*)(Inv_Struct->Head.Next);
    }

    /* D0 position is the operation */
    switch(RME_PARAM_D0(Operation))
    {
        /* Register read/write */
        case RME_RV32P_KFN_DEBUG_INV_MOD_SP_GET:    {Reg->X11_A1=Inv_Struct->Ret.X2_SP;break;}
        case RME_RV32P_KFN_DEBUG_INV_MOD_SP_SET:    {Inv_Struct->Ret.X2_SP=Value;break;}
        case RME_RV32P_KFN_DEBUG_INV_MOD_PC_GET:    {Reg->X11_A1=Inv_Struct->Ret.PC;break;}
        case RME_RV32P_KFN_DEBUG_INV_MOD_PC_SET:    {Inv_Struct->Ret.PC=Reg->X11_A1;break;}
        default:                                    {return RME_ERR_KFN_FAIL;}
    }

    return 0;
}
/* End Function:__RME_RV32P_Debug_Inv_Mod ************************************/

/* Function:__RME_RV32P_Debug_Exc_Get *****************************************
Description : Debug exception register extraction implementation for RV32P.
Input       : struct RME_Cap_Cpt* Cpt - The current capability table.
              struct RME_Reg_Struct* Reg - The current register set.
              rme_cid_t Cap_Thd - The capability to the thread to consult.
              rme_ptr_t Operation - The operation, e.g. which register to read.
Output      : struct RME_Reg_Struct* Reg - The register set when exiting the
                                           handler.
Return      : rme_ret_t - If successful, 0; if a negative value, failed.
******************************************************************************/
rme_ret_t __RME_RV32P_Debug_Exc_Get(struct RME_Cap_Cpt* Cpt,
                                    struct RME_Reg_Struct* Reg,
                                    rme_cid_t Cap_Thd,
                                    rme_ptr_t Operation)
{
    struct RME_Cap_Thd* Thd_Op;
    struct RME_Thd_Struct* Thd_Struct;
    struct RME_CPU_Local* Local;
    rme_ptr_t Type_Stat;

    /* Get the capability slot */
    RME_CPT_GETCAP(Cpt,Cap_Thd,RME_CAP_TYPE_THD,
                   struct RME_Cap_Thd*,Thd_Op,Type_Stat);

    /* See if the target thread is already binded. If no or binded to other cores, we just quit */
    Local=RME_CPU_LOCAL();
    Thd_Struct=(struct RME_Thd_Struct*)Thd_Op->Head.Object;
    if(Thd_Struct->Sched.Local!=Local)
        return RME_ERR_PTH_INVSTATE;

    switch(Operation)
    {
        /* Register read */
        case RME_RV32P_KFN_DEBUG_EXC_CAUSE_GET:     {Reg->X12_A2=Thd_Struct->Ctx.Reg->Exc.Cause;break;}
        case RME_RV32P_KFN_DEBUG_EXC_ADDR_GET:      {Reg->X12_A2=Thd_Struct->Ctx.Reg->Exc.Addr;break;}
        case RME_RV32P_KFN_DEBUG_EXC_VALUE_GET:     {Reg->X12_A2=Thd_Struct->Ctx.Reg->Exc.Value;break;}
        default:                                    {return RME_ERR_KFN_FAIL;}
    }

    return 0U;
}
/* End Function:__RME_RV32P_Debug_Exc_Get ************************************/

/* Function:__RME_Kfn_Handler *************************************************
Description : Handle kernel function calls.
Input       : struct RME_Cap_Cpt* Cpt - The current capability table.
              struct RME_Reg_Struct* Reg - The current register set.
              rme_ptr_t Func_ID - The function ID.
              rme_ptr_t Sub_ID - The subfunction ID.
              rme_ptr_t Param1 - The first parameter.
              rme_ptr_t Param2 - The second parameter.
Output      : None.
Return      : rme_ret_t - The value that the function returned.
******************************************************************************/
rme_ret_t __RME_Kfn_Handler(struct RME_Cap_Cpt* Cpt,
                            struct RME_Reg_Struct* Reg,
                            rme_ptr_t Func_ID,
                            rme_ptr_t Sub_ID,
                            rme_ptr_t Param1,
                            rme_ptr_t Param2)
{
    rme_ret_t Retval;

    /* Standard kernel function implmentations */
    switch(Func_ID)
    {
/* Page table operations *****************************************************/
        case RME_KFN_PGT_CACHE_CLR:     {return RME_ERR_KFN_FAIL;}
        case RME_KFN_PGT_LINE_CLR:      {return RME_ERR_KFN_FAIL;}
        case RME_KFN_PGT_ASID_SET:      {return RME_ERR_KFN_FAIL;}
        case RME_KFN_PGT_TLB_LOCK:      {return RME_ERR_KFN_FAIL;}
#if(RME_PGT_RAW_ENABLE==0U)
        case RME_KFN_PGT_ENTRY_MOD:
        {
            Retval=__RME_RV32P_Pgt_Entry_Mod(Cpt,
                                             (rme_cid_t)Sub_ID,
                                             Param1,
                                             Param2);
            break;
        }
#endif
/* Interrupt controller operations *******************************************/
        case RME_KFN_INT_LOCAL_MOD:
        {
            Retval=__RME_RV32P_Int_Local_Mod(Sub_ID,
                                             Param1,
                                             Param2);
            break;
        }
        case RME_KFN_INT_GLOBAL_MOD:    {return RME_ERR_KFN_FAIL;}
        case RME_KFN_INT_LOCAL_TRIG:
        {
            Retval=__RME_RV32P_Int_Local_Trig(Sub_ID,   /* No ctxsw, int ctxsw later */
                                              Param1);
            break;
        }
        case RME_KFN_EVT_LOCAL_TRIG:
        {
            return __RME_RV32P_Evt_Local_Trig(Reg,      /* May ctxsw */
                                              Sub_ID,
                                              Param1);
        }
/* Cache operations **********************************************************/
        case RME_KFN_CACHE_MOD:
        {
            Retval=__RME_RV32P_Cache_Mod(Sub_ID,
                                         Param1,
                                         Param2);
            break;
        }
        case RME_KFN_CACHE_CONFIG:      {return RME_ERR_KFN_FAIL;}
        case RME_KFN_CACHE_MAINT:
        {
            Retval=__RME_RV32P_Cache_Maint(Sub_ID,
                                           Param1,
                                           Param2);
            break;
        }
        case RME_KFN_CACHE_LOCK:        {return RME_ERR_KFN_FAIL;}
        case RME_KFN_PRFTH_MOD:
        {
            Retval=__RME_RV32P_Prfth_Mod(Sub_ID,
                                         Param1,
                                         Param2);
            break;
        }
/* Hot plug and pull operations **********************************************/
        case RME_KFN_HPNP_PCPU_MOD:     {return RME_ERR_KFN_FAIL;}
        case RME_KFN_HPNP_LCPU_MOD:     {return RME_ERR_KFN_FAIL;}
        case RME_KFN_HPNP_PMEM_MOD:     {return RME_ERR_KFN_FAIL;}
/* Power and frequency adjustment operations *********************************/
        case RME_KFN_IDLE_SLEEP:
        {
            __RME_RV32P_Wait_Int();
            Retval=0;
            break;
        }
        case RME_KFN_SYS_REBOOT:
        {
            __RME_RV32P_Reboot();
            while(1);
            break;
        }
        case RME_KFN_SYS_SHDN:          {return RME_ERR_KFN_FAIL;}
        case RME_KFN_VOLT_MOD:          {return RME_ERR_KFN_FAIL;}
        case RME_KFN_FREQ_MOD:          {return RME_ERR_KFN_FAIL;}
        case RME_KFN_PMOD_MOD:          {return RME_ERR_KFN_FAIL;}
        case RME_KFN_SAFETY_MOD:        {return RME_ERR_KFN_FAIL;}
/* Performance monitoring operations *****************************************/
        case RME_KFN_PERF_CPU_FUNC:
        {
            Retval=__RME_RV32P_Perf_CPU_Func(Reg,       /* Value in a3 */
                                             Sub_ID);
            break;
        }
        case RME_KFN_PERF_MON_MOD:
        {
            Retval=__RME_RV32P_Perf_Mon_Mod(Sub_ID,
                                            Param1,
                                            Param2);
            break;
        }
        case RME_KFN_PERF_CNT_MOD:      {return RME_ERR_KFN_FAIL;}
        case RME_KFN_PERF_CYCLE_MOD:
        {
            Retval=__RME_RV32P_Perf_Cycle_Mod(Reg,      /* Value in a3 */
                                              Sub_ID,
                                              Param1,
                                              Param2);
            break;
        }
        case RME_KFN_PERF_DATA_MOD:     {return RME_ERR_KFN_FAIL;}
        case RME_KFN_PERF_PHYS_MOD:     {return RME_ERR_KFN_FAIL;}
        case RME_KFN_PERF_CUMUL_MOD:    {return RME_ERR_KFN_FAIL;}
/* Hardware virtualization operations ****************************************/
        case RME_KFN_VM_CRT:            {return RME_ERR_KFN_FAIL;}
        case RME_KFN_VM_DEL:            {return RME_ERR_KFN_FAIL;}
        case RME_KFN_VM_PGT:            {return RME_ERR_KFN_FAIL;}
        case RME_KFN_VM_MOD:            {return RME_ERR_KFN_FAIL;}
        case RME_KFN_VCPU_CRT:          {return RME_ERR_KFN_FAIL;}
        case RME_KFN_VCPU_BIND:         {return RME_ERR_KFN_FAIL;}
        case RME_KFN_VCPU_FREE:         {return RME_ERR_KFN_FAIL;}
        case RME_KFN_VCPU_DEL:          {return RME_ERR_KFN_FAIL;}
        case RME_KFN_VCPU_MOD:          {return RME_ERR_KFN_FAIL;}
        case RME_KFN_VCPU_RUN:          {return RME_ERR_KFN_FAIL;}
/* Security monitor operations ***********************************************/
        case RME_KFN_ECLV_CRT:          {return RME_ERR_KFN_FAIL;}
        case RME_KFN_ECLV_MOD:          {return RME_ERR_KFN_FAIL;}
        case RME_KFN_ECLV_DEL:          {return RME_ERR_KFN_FAIL;}
        case RME_KFN_ECLV_ACT:          {return RME_ERR_KFN_FAIL;}
        case RME_KFN_ECLV_RET:          {return RME_ERR_KFN_FAIL;}
/* Debugging operations ******************************************************/
#if(RME_DBGLOG_ENABLE!=0U)
        case RME_KFN_DEBUG_PRINT:
        {
            __RME_Putchar((rme_s8_t)Sub_ID);
            Retval=0;
            break;
        }
#endif
        case RME_KFN_DEBUG_REG_MOD:
        {
            Retval=__RME_RV32P_Debug_Reg_Mod(Cpt,       /* Value in a3 */
                                             Reg,
                                             (rme_cid_t)Sub_ID,
                                             Param1,
                                             Param2);
            break;
        }
        case RME_KFN_DEBUG_INV_MOD:
        {
            Retval=__RME_RV32P_Debug_Inv_Mod(Cpt,       /* Value in a3 */
                                             Reg,
                                             (rme_cid_t)Sub_ID,
                                             Param1,
                                             Param2);
            break;
        }
        case RME_KFN_DEBUG_EXC_GET:
        {
            Retval=__RME_RV32P_Debug_Exc_Get(Cpt,       /* Value in a3 */
                                             Reg,
                                             (rme_cid_t)Sub_ID,
                                             Param1);
            break;
        }
        case RME_KFN_DEBUG_MODE_MOD:    {return RME_ERR_KFN_FAIL;}
        case RME_KFN_DEBUG_IBP_MOD:     {return RME_ERR_KFN_FAIL;}
        case RME_KFN_DEBUG_DBP_MOD:     {return RME_ERR_KFN_FAIL;}
/* User-defined operations ***************************************************/
        default:
        {
#if(RME_RVM_GEN_ENABLE!=0U)
            Retval=RME_Hook_Kfn_Handler(Func_ID,
                                        Sub_ID,
                                        Param1,
                                        Param2);
#else
            return RME_ERR_KFN_FAIL;
#endif
        }
    }

    /* If it gets here, we must have failed */
    if(Retval>=0)
        __RME_Svc_Retval_Set(Reg,0);
            
    return Retval;
}
/* End Function:__RME_Kfn_Handler ********************************************/

/* Function:__RME_RV32P_Lowlvl_Preinit ****************************************
Description : Initialize the low-level hardware, before the loading of the kernel
              even takes place.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_RV32P_Lowlvl_Preinit(void)
{
    RME_RV32P_LOWLVL_PREINIT();
    
#if(RME_RVM_GEN_ENABLE!=0U)
    RME_Boot_Pre_Init();
#endif
}
/* End Function:__RME_RV32P_Lowlvl_Preinit ***********************************/

/* Function:__RME_Lowlvl_Init *************************************************
Description : Initialize the low-level hardware.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_Lowlvl_Init(void)
{
    RME_RV32P_LOWLVL_INIT();

    /* Initialize CPU-local data structures */
    _RME_CPU_Local_Init(&RME_RV32P_Local,__RME_CPUID_Get());
}
/* End Function:__RME_Lowlvl_Init ********************************************/

/* Function:__RME_Boot ********************************************************
Description : Boot the first process in the system.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_Boot(void)
{
    /* volatile rme_ptr_t Size; */
    rme_ptr_t Cur_Addr;
#if(RME_PGT_RAW_ENABLE!=0U)
    /* Initial array for raw page table mode - generic for all RV32 */
#if(RME_RV32_REGION_NUM<=4U)
    static const rme_ptr_t RME_RV32P_Raw_Pgt_Def[5U]=
    {
        0x1818181FU,
        0xFFFFFFFFU,0xFFFFFFFFU,0xFFFFFFFFU,0xFFFFFFFFU
    };
#elif(RME_RV32_REGION_NUM<=8U)
    static const rme_ptr_t RME_RV32P_Raw_Pgt_Def[10U]=
    {
        0x1818181FU,0x18181818U,
        0xFFFFFFFFU,0xFFFFFFFFU,0xFFFFFFFFU,0xFFFFFFFFU,
        0xFFFFFFFFU,0xFFFFFFFFU,0xFFFFFFFFU,0xFFFFFFFFU
    };
#elif(RME_RV32_REGION_NUM<=12U)
    static const rme_ptr_t RME_RV32P_Raw_Pgt_Def[15U]=
    {
        0x1818181FU,0x18181818U,0x18181818U,
        0xFFFFFFFFU,0xFFFFFFFFU,0xFFFFFFFFU,0xFFFFFFFFU,
        0xFFFFFFFFU,0xFFFFFFFFU,0xFFFFFFFFU,0xFFFFFFFFU,
        0xFFFFFFFFU,0xFFFFFFFFU,0xFFFFFFFFU,0xFFFFFFFFU
    };
#else
    static const rme_ptr_t RME_RV32P_Raw_Pgt_Def[20U]=
    {
        0x1818181FU,0x18181818U,0x18181818U,0x18181818U,
        0xFFFFFFFFU,0xFFFFFFFFU,0xFFFFFFFFU,0xFFFFFFFFU,
        0xFFFFFFFFU,0xFFFFFFFFU,0xFFFFFFFFU,0xFFFFFFFFU,
        0xFFFFFFFFU,0xFFFFFFFFU,0xFFFFFFFFU,0xFFFFFFFFU,
        0xFFFFFFFFU,0xFFFFFFFFU,0xFFFFFFFFU,0xFFFFFFFFU
    };
#endif
#endif

    Cur_Addr=RME_KOM_VA_BASE;

    /* Create the capability table for the init process */
    RME_ASSERT(_RME_Cpt_Boot_Init(RME_BOOT_INIT_CPT,
                                  Cur_Addr,
                                  RME_RVM_INIT_CPT_SIZE)==0);
    Cur_Addr+=RME_KOM_ROUND(RME_CPT_SIZE(RME_RVM_INIT_CPT_SIZE));

    /* Create the page table for the init process, and map in the page alloted for it */
#if(RME_PGT_RAW_ENABLE==0U)
    /* The top-level page table - covers 4G address range */
    RME_ASSERT(_RME_Pgt_Boot_Crt(RME_RV32P_CPT,
                                 RME_BOOT_INIT_CPT,
                                 RME_BOOT_INIT_PGT,
                                 Cur_Addr,
                                 0x00000000U,
                                 RME_PGT_TOP,
                                 RME_PGT_SIZE_4G,
                                 RME_PGT_NUM_1)==0);
    Cur_Addr+=RME_KOM_ROUND(RME_PGT_SIZE_TOP(RME_PGT_NUM_1));
    /* Other memory regions will be directly added, because we do not protect them in the init process */
    RME_ASSERT(_RME_Pgt_Boot_Add(RME_RV32P_CPT,
                                 RME_BOOT_INIT_PGT,
                                 0x00000000U,
                                 0U,
                                 RME_PGT_ALL_PERM)==0);

    /* Activate the first process - This process cannot be deleted */
    RME_ASSERT(_RME_Prc_Boot_Crt(RME_RV32P_CPT,
                                 RME_BOOT_INIT_CPT,
                                 RME_BOOT_INIT_PRC,
                                 RME_BOOT_INIT_CPT,
                                 RME_BOOT_INIT_PGT)==0U);
#else
    RME_ASSERT(_RME_Prc_Boot_Crt(RME_RV32P_CPT,
                                 RME_BOOT_INIT_CPT,
                                 RME_BOOT_INIT_PRC,
                                 RME_BOOT_INIT_CPT,
                                 (rme_ptr_t)RME_RV32P_Raw_Pgt_Def)==0U);
#endif

    /* Create the initial kernel function capability, and kernel memory capability */
    RME_ASSERT(_RME_Kfn_Boot_Crt(RME_RV32P_CPT,
                                 RME_BOOT_INIT_CPT,
                                 RME_BOOT_INIT_KFN)==0);
    RME_ASSERT(_RME_Kom_Boot_Crt(RME_RV32P_CPT,
                                 RME_BOOT_INIT_CPT,
                                 RME_BOOT_INIT_KOM,
                                 RME_KOM_VA_BASE,
                                 RME_KOM_VA_BASE+RME_KOM_VA_SIZE-1U,
                                 RME_KOM_FLAG_ALL)==0U);

    /* Create the initial kernel endpoint for timer ticks and interrupts */
    RME_RV32P_Local.Sig_Tim=(struct RME_Cap_Sig*)&(RME_RV32P_CPT[RME_BOOT_INIT_VCT]);
    RME_RV32P_Local.Sig_Vct=(struct RME_Cap_Sig*)&(RME_RV32P_CPT[RME_BOOT_INIT_VCT]);
    RME_ASSERT(_RME_Sig_Boot_Crt(RME_RV32P_CPT,
                                 RME_BOOT_INIT_CPT,
                                 RME_BOOT_INIT_VCT)==0);

    /* Clean up the region for vectors and events */
    _RME_Clear((void*)RME_RVM_PHYS_VCTF_BASE,RME_RVM_PHYS_VCTF_SIZE);
    _RME_Clear((void*)RME_RVM_VIRT_EVTF_BASE,RME_RVM_VIRT_EVTF_SIZE);

    /* Activate the first thread, and set its priority */
    RME_ASSERT(_RME_Thd_Boot_Crt(RME_RV32P_CPT,
                                 RME_BOOT_INIT_CPT,
                                 RME_BOOT_INIT_THD,
                                 RME_BOOT_INIT_PRC,
                                 Cur_Addr,
                                 0U,
                                 &RME_RV32P_Local)==0);
    Cur_Addr+=RME_KOM_ROUND(RME_THD_SIZE(0U));

    /* Print the size of some kernel objects, only used in debugging
    Size=RME_CPT_SIZE(1U);
    Size=RME_PGT_SIZE_TOP(0U)-2U*RME_WORD_BYTE;
    Size=RME_PGT_SIZE_NOM(0U)-2U*RME_WORD_BYTE;
    Size=RME_INV_SIZE;
    Size=RME_HYP_SIZE;
    Size=RME_REG_SIZE(0U);
    Size=RME_REG_SIZE(0U)+sizeof(struct RME_RV32P_Cop_Struct);
    Size=RME_THD_SIZE(0U); */

    /* If generator is enabled for this project, generate what is required by the generator */
#if(RME_RVM_GEN_ENABLE!=0U)
    Cur_Addr=RME_Boot_Vct_Init(RME_RV32P_CPT,
                               RME_BOOT_INIT_VCT+1U,
                               Cur_Addr);
#endif

    /* Before we go into user level, make sure that the kernel object allocation is within the limits */
#if(RME_RVM_GEN_ENABLE!=0U)
    RME_ASSERT(Cur_Addr==(RME_KOM_VA_BASE+RME_RVM_KOM_BOOT_FRONT));
#else
    RME_ASSERT(Cur_Addr<(RME_KOM_VA_BASE+RME_RVM_KOM_BOOT_FRONT));
#endif

#if(RME_RVM_GEN_ENABLE!=0U)
    /* Perform post initialization */
    RME_Boot_Post_Init();
#endif

    /* Enable the PMP & interrupt */
#if(RME_PGT_RAW_ENABLE==0U)
    RME_ASSERT(RME_CAP_IS_ROOT(RME_RV32P_Local.Thd_Cur->Sched.Prc->Pgt)!=0U);
#endif
    __RME_Pgt_Set(RME_RV32P_Local.Thd_Cur->Sched.Prc->Pgt);
    __RME_Int_Enable();

    /* Boot into the init thread */
    __RME_User_Enter(RME_RV32P_INIT_ENTRY,RME_RV32P_INIT_STACK,0U);

    /* Should never reach here */
    while(1);
}
/* End Function:__RME_Boot ***************************************************/

/* Function:__RME_RV32P_Reboot ***********************************************
Description : Reboot the MCU, including all its peripherals.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_RV32P_Reboot(void)
{
    /* Reboots the platform, which includes all peripherals. */
    RME_RV32P_REBOOT();
}
/* End Function:__RME_RV32P_Reboot ******************************************/

/* Function:__RME_Svc_Param_Get ***********************************************
Description : Get the system call parameters from the stack frame.
Input       : struct RME_Reg_Struct* Reg - The register set.
Output      : rme_ptr_t* Svc - The system service number.
              rme_ptr_t* Cid - The capability ID number.
              rme_ptr_t* Param - The parameters.
Return      : None.
******************************************************************************/
void __RME_Svc_Param_Get(struct RME_Reg_Struct* Reg,
                         rme_ptr_t* Svc,
                         rme_ptr_t* Cid,
                         rme_ptr_t* Param)
{
    *Svc=(Reg->X10_A0)>>16;
    *Cid=(Reg->X10_A0)&0xFFFFU;
    Param[0]=Reg->X11_A1;
    Param[1]=Reg->X12_A2;
    Param[2]=Reg->X13_A3;
}
/* End Function:__RME_Svc_Param_Get ******************************************/

/* Function:__RME_Svc_Retval_Set **********************************************
Description : Set the system call return value to the stack frame. This function 
              may carry up to 4 return values. If the last 3 is not needed, just set
              them to zero.
Input       : rme_ret_t Retval - The return value.
Output      : struct RME_Reg_Struct* Reg - The register set.
Return      : None.
******************************************************************************/
void __RME_Svc_Retval_Set(struct RME_Reg_Struct* Reg,
                          rme_ret_t Retval)
{
    Reg->X10_A0=(rme_ptr_t)Retval;
}
/* End Function:__RME_Svc_Retval_Set *****************************************/

/* Function:__RME_Thd_Reg_Init ************************************************
Description : Initialize the register set for the thread.
Input       : rme_ptr_t Attr - The context attributes.
              rme_ptr_t Entry - The thread entry address.
              rme_ptr_t Stack - The thread stack address.
              rme_ptr_t Param - The parameter to pass.
Output      : struct RME_Reg_Struct* Reg - The register set content generated.
Return      : None.
******************************************************************************/
void __RME_Thd_Reg_Init(rme_ptr_t Attr,
                        rme_ptr_t Entry,
                        rme_ptr_t Stack,
                        rme_ptr_t Param,
                        struct RME_Reg_Struct* Reg)
{
    /* Decide FPU state */
#if(RME_COP_NUM==0U)
    Reg->MSTATUS=RME_RV32P_MSTATUS_INIT;
#else
    if(Attr==RME_RV32P_ATTR_NONE)
        Reg->MSTATUS=RME_RV32P_MSTATUS_INIT;
    else
        Reg->MSTATUS=RME_RV32P_MSTATUS_INIT|RME_RV32P_MSTATUS_FPU_INIT;
#endif
    /* The entry point */
    Reg->PC=Entry;
    /* The stack */
    Reg->X2_SP=Stack;
    /* Set the parameter */
    Reg->X10_A0=Param;
}
/* End Function:__RME_Thd_Reg_Init *******************************************/

/* Function:__RME_Thd_Reg_Copy ************************************************
Description : Copy one set of registers into another.
Input       : struct RME_Reg_Struct* Src - The source register set.
Output      : struct RME_Reg_Struct* Dst - The destination register set.
Return      : None.
******************************************************************************/
void __RME_Thd_Reg_Copy(struct RME_Reg_Struct* Dst,
                        struct RME_Reg_Struct* Src)
{
    /* Make sure that the ordering is the same so the compiler can optimize */
    Dst->MSTATUS=Src->MSTATUS;
    Dst->PC=Src->PC;
    Dst->X1_RA=Src->X1_RA;
    Dst->X2_SP=Src->X2_SP;
    Dst->X3_GP=Src->X3_GP;
    Dst->X4_TP=Src->X4_TP;
    Dst->X5_T0=Src->X5_T0;
    Dst->X6_T1=Src->X6_T1;
    Dst->X7_T2=Src->X7_T2;
    Dst->X8_S0_FP=Src->X8_S0_FP;
    Dst->X9_S1=Src->X9_S1;
    Dst->X10_A0=Src->X10_A0;
    Dst->X11_A1=Src->X11_A1;
    Dst->X12_A2=Src->X12_A2;
    Dst->X13_A3=Src->X13_A3;
    Dst->X14_A4=Src->X14_A4;
    Dst->X15_A5=Src->X15_A5;
    Dst->X16_A6=Src->X16_A6;
    Dst->X17_A7=Src->X17_A7;
    Dst->X18_S2=Src->X18_S2;
    Dst->X19_S3=Src->X19_S3;
    Dst->X20_S4=Src->X20_S4;
    Dst->X21_S5=Src->X21_S5;
    Dst->X22_S6=Src->X22_S6;
    Dst->X23_S7=Src->X23_S7;
    Dst->X24_S8=Src->X24_S8;
    Dst->X25_S9=Src->X25_S9;
    Dst->X26_S10=Src->X26_S10;
    Dst->X27_S11=Src->X27_S11;
    Dst->X28_T3=Src->X28_T3;
    Dst->X29_T4=Src->X29_T4;
    Dst->X30_T5=Src->X30_T5;
    Dst->X31_T6=Src->X31_T6;
}
/* End Function:__RME_Thd_Reg_Copy *******************************************/

/* Function:__RME_Inv_Reg_Save ************************************************
Description : Save the necessary registers on invocation for returning. Only the
              registers that will influence program control flow will be saved.
Input       : struct RME_Reg_Struct* Reg - The register set.
Output      : struct RME_Iret_Struct* Ret - The invocation return register context.
Return      : None.
******************************************************************************/
void __RME_Inv_Reg_Save(struct RME_Iret_Struct* Ret,
                        struct RME_Reg_Struct* Reg)
{
    Ret->PC=Reg->PC;
    Ret->X2_SP=Reg->X2_SP;
}
/* End Function:__RME_Inv_Reg_Save *******************************************/

/* Function:__RME_Inv_Reg_Restore *********************************************
Description : Restore the necessary registers for returning from an invocation.
Input       : struct RME_Iret_Struct* Ret - The invocation return register context.
Output      : struct RME_Reg_Struct* Reg - The register set.
Return      : None.
******************************************************************************/
void __RME_Inv_Reg_Restore(struct RME_Reg_Struct* Reg,
                           struct RME_Iret_Struct* Ret)
{
    Reg->PC=Ret->PC;
    Reg->X2_SP=Ret->X2_SP;
}
/* End Function:__RME_Inv_Reg_Restore ****************************************/

/* Function:__RME_Inv_Retval_Set **********************************************
Description : Set the invocation return value to the stack frame.
Input       : rme_ret_t Retval - The return value.
Output      : struct RME_Reg_Struct* Reg - The register set.
Return      : None.
******************************************************************************/
void __RME_Inv_Retval_Set(struct RME_Reg_Struct* Reg,
                          rme_ret_t Retval)
{
    Reg->X11_A1=(rme_ptr_t)Retval;
}
/* End Function:__RME_Inv_Retval_Set *****************************************/

/* Function:__RME_Thd_Cop_Check ***********************************************
Description : Check if this CPU is compatible with this coprocessor attribute.
Input       : rme_ptr_t Attr - The thread context attributes.
Output      : None.
Return      : rme_ret_t - If 0, compatible; if RME_ERR_HAL_FAIL, incompatible.
******************************************************************************/
#if(RME_COP_NUM!=0U)
rme_ret_t __RME_Thd_Cop_Check(rme_ptr_t Attr)
{
#if(RME_RV32P_COP_RVD!=0U)
    /* No checks necessary */
#elif(RME_RV32P_COP_RVF!=0U)
    /* If only RVF is selected, no RVD allowed */
    if((Attr&RME_RV32P_ATTR_RVD)!=0U)
        return RME_ERR_HAL_FAIL;
#else
    /* Can't happen, wrong macro configs */
#error No coprocessor selected but RME_COP_NUM is nonzero.
#endif

    return 0;
}
#endif
/* End Function:__RME_Thd_Cop_Check ******************************************/

/* Function:__RME_Thd_Cop_Size ************************************************
Description : Query coprocessor register size for this CPU.
Input       : rme_ptr_t Attr - The thread context attributes.
Output      : None.
Return      : rme_ptr_t - 0 - This does not have any coprocessors.
******************************************************************************/
#if(RME_COP_NUM!=0U)
rme_ptr_t __RME_Thd_Cop_Size(rme_ptr_t Attr)
{
    /* Even if the thread would specify only RVF when RVD is present, the
     * context is still as large as RVD because they use the same coprocessor */
    if(Attr!=RME_RV32P_ATTR_NONE)
        return sizeof(struct RME_RV32P_Cop_Struct);

    return 0U;
}
#endif
/* End Function:__RME_Thd_Cop_Size *******************************************/

/* Function:__RME_Thd_Cop_Init ************************************************
Description : Initialize the coprocessor register set for the thread.
Input       : rme_ptr_t Attr - The coprocessor context attributes.
              struct RME_Reg_Struct* Reg - The register struct to help
                                           initialize the coprocessor.
Output      : void* Cop - The register set content generated.
Return      : None.
******************************************************************************/
#if(RME_COP_NUM!=0U)
void __RME_Thd_Cop_Init(rme_ptr_t Attr,
                        struct RME_Reg_Struct* Reg,
                        void* Cop)
{
    struct RME_RV32P_Cop_Struct* RV32P_Cop;

    if(Attr==RME_RV32P_ATTR_NONE)
        return;

    RV32P_Cop=Cop;

    RV32P_Cop->FCSR=0U;
#if(RME_RV32P_COP_RVD==0U)
    RV32P_Cop->F0=0U;
    RV32P_Cop->F1=0U;
    RV32P_Cop->F2=0U;
    RV32P_Cop->F3=0U;
    RV32P_Cop->F4=0U;
    RV32P_Cop->F5=0U;
    RV32P_Cop->F6=0U;
    RV32P_Cop->F7=0U;
    RV32P_Cop->F8=0U;
    RV32P_Cop->F9=0U;
    RV32P_Cop->F10=0U;
    RV32P_Cop->F11=0U;
    RV32P_Cop->F12=0U;
    RV32P_Cop->F13=0U;
    RV32P_Cop->F14=0U;
    RV32P_Cop->F15=0U;
    RV32P_Cop->F16=0U;
    RV32P_Cop->F17=0U;
    RV32P_Cop->F18=0U;
    RV32P_Cop->F19=0U;
    RV32P_Cop->F20=0U;
    RV32P_Cop->F21=0U;
    RV32P_Cop->F22=0U;
    RV32P_Cop->F23=0U;
    RV32P_Cop->F24=0U;
    RV32P_Cop->F25=0U;
    RV32P_Cop->F26=0U;
    RV32P_Cop->F27=0U;
    RV32P_Cop->F28=0U;
    RV32P_Cop->F29=0U;
    RV32P_Cop->F30=0U;
    RV32P_Cop->F31=0U;
#else
    RV32P_Cop->F0[0]=0U;
    RV32P_Cop->F0[1]=0U;
    RV32P_Cop->F1[0]=0U;
    RV32P_Cop->F1[1]=0U;
    RV32P_Cop->F2[0]=0U;
    RV32P_Cop->F2[1]=0U;
    RV32P_Cop->F3[0]=0U;
    RV32P_Cop->F3[1]=0U;
    RV32P_Cop->F4[0]=0U;
    RV32P_Cop->F4[1]=0U;
    RV32P_Cop->F5[0]=0U;
    RV32P_Cop->F5[1]=0U;
    RV32P_Cop->F6[0]=0U;
    RV32P_Cop->F6[1]=0U;
    RV32P_Cop->F7[0]=0U;
    RV32P_Cop->F7[1]=0U;
    RV32P_Cop->F8[0]=0U;
    RV32P_Cop->F8[1]=0U;
    RV32P_Cop->F9[0]=0U;
    RV32P_Cop->F9[1]=0U;
    RV32P_Cop->F10[0]=0U;
    RV32P_Cop->F10[1]=0U;
    RV32P_Cop->F11[0]=0U;
    RV32P_Cop->F11[1]=0U;
    RV32P_Cop->F12[0]=0U;
    RV32P_Cop->F12[1]=0U;
    RV32P_Cop->F13[0]=0U;
    RV32P_Cop->F13[1]=0U;
    RV32P_Cop->F14[0]=0U;
    RV32P_Cop->F14[1]=0U;
    RV32P_Cop->F15[0]=0U;
    RV32P_Cop->F15[1]=0U;
    RV32P_Cop->F16[0]=0U;
    RV32P_Cop->F16[1]=0U;
    RV32P_Cop->F17[0]=0U;
    RV32P_Cop->F17[1]=0U;
    RV32P_Cop->F18[0]=0U;
    RV32P_Cop->F18[1]=0U;
    RV32P_Cop->F19[0]=0U;
    RV32P_Cop->F19[1]=0U;
    RV32P_Cop->F20[0]=0U;
    RV32P_Cop->F20[1]=0U;
    RV32P_Cop->F21[0]=0U;
    RV32P_Cop->F21[1]=0U;
    RV32P_Cop->F22[0]=0U;
    RV32P_Cop->F22[1]=0U;
    RV32P_Cop->F23[0]=0U;
    RV32P_Cop->F23[1]=0U;
    RV32P_Cop->F24[0]=0U;
    RV32P_Cop->F24[1]=0U;
    RV32P_Cop->F25[0]=0U;
    RV32P_Cop->F25[1]=0U;
    RV32P_Cop->F26[0]=0U;
    RV32P_Cop->F26[1]=0U;
    RV32P_Cop->F27[0]=0U;
    RV32P_Cop->F27[1]=0U;
    RV32P_Cop->F28[0]=0U;
    RV32P_Cop->F28[1]=0U;
    RV32P_Cop->F29[0]=0U;
    RV32P_Cop->F29[1]=0U;
    RV32P_Cop->F30[0]=0U;
    RV32P_Cop->F30[1]=0U;
    RV32P_Cop->F31[0]=0U;
    RV32P_Cop->F31[1]=0U;
#endif
}
#endif
/* End Function:__RME_Thd_Cop_Init *******************************************/

/* Function:__RME_Thd_Cop_Swap ************************************************
Description : Swap the cop register sets. This operation is flexible - If the
              program does not use the FPU, we do not save/restore its context.
Input       : rme_ptr_t Attr_New - The attribute of the context to switch to.
              rme_ptr_t Is_Hyp_New - Whether the context to switch to is a
                                     hypervisor dedicated one.
              struct RME_Reg_Struct* Reg_New - The context to switch to.
              rme_ptr_t Attr_Cur - The attribute of the context to switch from.
              rme_ptr_t Is_Hyp_Cur - Whether the context to switch from is a
                                     hypervisor dedicated one.
              struct RME_Reg_Struct* Reg_Cur - The context to switch from.
Output      : void* Cop_New - The coprocessor context to switch to.
              void* Cop_Cur - The coprocessor context to switch from.
Return      : None.
******************************************************************************/
#if(RME_COP_NUM!=0U)
void __RME_Thd_Cop_Swap(rme_ptr_t Attr_New,
                        rme_ptr_t Is_Hyp_New,
                        struct RME_Reg_Struct* Reg_New,
                        void* Cop_New,
                        rme_ptr_t Attr_Cur,
                        rme_ptr_t Is_Hyp_Cur,
                        struct RME_Reg_Struct* Reg_Cur,
                        void* Cop_Cur)
{
    rme_ptr_t State_Cur;
    rme_ptr_t State_New;

    State_Cur=Reg_Cur->MSTATUS&RME_RV32P_MSTATUS_FPU_MASK;
    State_New=Reg_New->MSTATUS&RME_RV32P_MSTATUS_FPU_MASK;

    /* Fix the coprocessor enabling status reported by the HYP threads */
    if(Is_Hyp_Cur!=0U)
    {
        if(Attr_Cur!=RME_RV32P_ATTR_NONE)
        {
            /* If the current thread uses the coprocessor but reports not using
             * it, treat the coprocessor as CLEAN, and enable the coprocessor */
            if(State_Cur==RME_RV32P_MSTATUS_FPU_OFF)
            {
                State_Cur=RME_RV32P_MSTATUS_FPU_CLEAN;
                ___RME_RV32P_MSTATUS_Set(RME_RV32P_MSTATUS_INIT|RME_RV32P_MSTATUS_FPU_CLEAN);
                Reg_Cur->MSTATUS=RME_RV32P_MSTATUS_INIT|RME_RV32P_MSTATUS_FPU_CLEAN;
            }
        }
        else
        {
            /* If the current thread does not use the coprocessor but reports using
             * it, treat the coprocessor as OFF, and disable the coprocessor; this in
             * theory does not happen because the mstatus will be fixed on the handler
             * return path, and we only have a single core thus it will not be modified
             * by another core after we fix it; on multi-core processors (which is out
             *  of the scope of this port), the fix must be on the assembly exit anyway */
            if((Attr_Cur==RME_RV32P_ATTR_NONE)&&(State_Cur!=RME_RV32P_MSTATUS_FPU_OFF))
            {
                State_Cur=RME_RV32P_MSTATUS_FPU_OFF;
                ___RME_RV32P_MSTATUS_Set(RME_RV32P_MSTATUS_INIT);
                Reg_Cur->MSTATUS=RME_RV32P_MSTATUS_INIT;
            }
        }
    }

    if(Is_Hyp_New!=0U)
    {
        if(Attr_New!=RME_RV32P_ATTR_NONE)
        {
            /* If the new thread uses the coprocessor but reports not using
             * it, treat the coprocessor as CLEAN */
            if(State_New==RME_RV32P_MSTATUS_FPU_OFF)
            {
                State_New=RME_RV32P_MSTATUS_FPU_CLEAN;
                Reg_New->MSTATUS=RME_RV32P_MSTATUS_INIT|RME_RV32P_MSTATUS_FPU_CLEAN;
            }
        }
        else
        {
            /* If the current thread does not use the coprocessor but reports using
             * it, treat the coprocessor as OFF */
            if((Attr_New==RME_RV32P_ATTR_NONE)&&(State_New!=RME_RV32P_MSTATUS_FPU_OFF))
            {
                State_New=RME_RV32P_MSTATUS_FPU_OFF;
                Reg_New->MSTATUS=RME_RV32P_MSTATUS_INIT;
            }
        }
    }

    /* The current thread does not use the coprocessor */
    if(Attr_Cur==RME_RV32P_ATTR_NONE)
    {
        /* The new thread does not use the coprocessor */
        if(Attr_New==RME_RV32P_ATTR_NONE)
        {
            /* No action required */
        }
        /* The new thread did not touch the coprocessor */
        else if(State_New==RME_RV32P_MSTATUS_FPU_INIT)
        {
            /* Turn the coprocessor on */
            Reg_New->MSTATUS=RME_RV32P_MSTATUS_INIT|RME_RV32P_MSTATUS_FPU_INIT;
            ___RME_RV32P_MSTATUS_Set(RME_RV32P_MSTATUS_INIT|RME_RV32P_MSTATUS_FPU_INIT);
        }
        /* The new thread touched the coprocessor and its context was saved */
        else
        {
            /* Turn the coprocessor on */
            Reg_New->MSTATUS=RME_RV32P_MSTATUS_INIT|RME_RV32P_MSTATUS_FPU_CLEAN;
            ___RME_RV32P_MSTATUS_Set(RME_RV32P_MSTATUS_INIT|RME_RV32P_MSTATUS_FPU_CLEAN);
            /* Load the new coprocessor context */
            RME_RV32P_THD_COP_LOAD(Cop_New);
        }
    }
    /* The current thread did not touch the coprocessor, and is not a HYP one */
    else if((State_Cur==RME_RV32P_MSTATUS_FPU_INIT)&&(Is_Hyp_Cur==0U))
    {
        /* The new thread does not use the coprocessor */
        if(Attr_New==RME_RV32P_ATTR_NONE)
        {
            /* Turn the coprocessor off */
            Reg_New->MSTATUS=RME_RV32P_MSTATUS_INIT;
            ___RME_RV32P_MSTATUS_Set(RME_RV32P_MSTATUS_INIT);
        }
        /* The new thread did not touch the coprocessor */
        else if(State_New==RME_RV32P_MSTATUS_FPU_INIT)
        {
            /* No action required */
        }
        /* The new thread touched the coprocessor, and was saved */
        else
        {
            /* Load the new coprocessor context */
            RME_RV32P_THD_COP_LOAD(Cop_New);
        }
    }
    /* The current thread touched the coprocessor and its context was saved, or it is a HYP one;
     * for HYP threads, the INIT status is not trustworthy and must always be treated as CLEAN */
    else if((State_Cur==RME_RV32P_MSTATUS_FPU_INIT)||(State_Cur==RME_RV32P_MSTATUS_FPU_CLEAN))
    {
        /* The new thread does not use the coprocessor */
        if(Attr_New==RME_RV32P_ATTR_NONE)
        {
            /* Reinitialize the coprocessor context */
            RME_RV32P_THD_COP_CLEAR();
            /* Turn the coprocessor off */
            Reg_New->MSTATUS=RME_RV32P_MSTATUS_INIT;
            ___RME_RV32P_MSTATUS_Set(RME_RV32P_MSTATUS_INIT);
        }
        /* The new thread did not touch the coprocessor */
        else if(State_New==RME_RV32P_MSTATUS_FPU_INIT)
        {
            /* Reinitialize the coprocessor context */
            RME_RV32P_THD_COP_CLEAR();
        }
        /* The new thread touched the coprocessor, and was saved */
        else
        {
            /* Load the new coprocessor context */
            RME_RV32P_THD_COP_LOAD(Cop_New);
        }
    }
    /* The current thread touched the coprocessor and its context was not saved */
    else
    {
        /* Save the current coprocessor context */
        RME_RV32P_THD_COP_SAVE(Cop_Cur);
        /* Set the current thread to clean */
        Reg_Cur->MSTATUS=RME_RV32P_MSTATUS_INIT|RME_RV32P_MSTATUS_FPU_CLEAN;

        /* The new thread does not use the coprocessor */
        if(Attr_New==RME_RV32P_ATTR_NONE)
        {
            /* Reinitialize the coprocessor context */
            RME_RV32P_THD_COP_CLEAR();
            /* Turn the coprocessor off */
            Reg_New->MSTATUS=RME_RV32P_MSTATUS_INIT;
            ___RME_RV32P_MSTATUS_Set(RME_RV32P_MSTATUS_INIT);
        }
        /* The new thread did not touch the coprocessor */
        else if(State_New==RME_RV32P_MSTATUS_FPU_INIT)
        {
            /* Reinitialize the coprocessor context */
            RME_RV32P_THD_COP_CLEAR();
        }
        /* The new thread touched the coprocessor, and was saved */
        else
        {
            /* Load the new coprocessor context */
            RME_RV32P_THD_COP_LOAD(Cop_New);
        }
    }
}
#endif
/* End Function:__RME_Thd_Cop_Swap *******************************************/

/* Function:__RME_Pgt_Kom_Init ************************************************
Description : Initialize the kernel mapping tables, so it can be added to all the
              top-level page tables. In RV32P, we do not need to add such pages.
Input       : None.
Output      : None.
Return      : rme_ret_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
rme_ret_t __RME_Pgt_Kom_Init(void)
{
    /* Empty function, always immediately successful */
    return 0;
}
/* End Function:__RME_Pgt_Kom_Init *******************************************/

/* Function:__RME_Pgt_Set *****************************************************
Description : Set the processor's page table.
Input       : struct RME_Cap_Pgt* Pgt - The capability to the root page table.
              struct RME_Raw_Pgt* Pgt - The alternative raw page table.
Output      : None.
Return      : None.
******************************************************************************/
#if(RME_PGT_RAW_ENABLE==0U)
void __RME_Pgt_Set(struct RME_Cap_Pgt* Pgt)
#else
void __RME_Pgt_Set(rme_ptr_t Pgt)
#endif
{
    struct __RME_RV32P_Raw_Pgt* Raw_Pgt;

#if(RME_PGT_RAW_ENABLE==0U)
    struct __RME_RV32P_PMP_Data* PMP_Data;

    PMP_Data=(struct __RME_RV32P_PMP_Data*)(RME_CAP_GETOBJ(Pgt, rme_ptr_t)+
                                            sizeof(struct __RME_RV32P_Pgt_Meta));
    Raw_Pgt=&(PMP_Data->Raw);
#else
    Raw_Pgt=(struct __RME_RV32P_Raw_Pgt*)Pgt;
#endif

    /* Get the physical address of the page table - here we do not need any
     * conversion, because VA = PA as always. We just need to extract the MPU
     * metadata part and pass it down */
#if(RME_RV32P_REGION_NUM==1U)
    ___RME_RV32P_PMP_Set1(Raw_Pgt);
#elif(RME_RV32P_REGION_NUM==2U)
    ___RME_RV32P_PMP_Set2(Raw_Pgt);
#elif(RME_RV32P_REGION_NUM==3U)
    ___RME_RV32P_PMP_Set3(Raw_Pgt);
#elif(RME_RV32P_REGION_NUM==4U)
    ___RME_RV32P_PMP_Set4(Raw_Pgt);
#elif(RME_RV32P_REGION_NUM==5U)
    ___RME_RV32P_PMP_Set5(Raw_Pgt);
#elif(RME_RV32P_REGION_NUM==6U)
    ___RME_RV32P_PMP_Set6(Raw_Pgt);
#elif(RME_RV32P_REGION_NUM==7U)
    ___RME_RV32P_PMP_Set7(Raw_Pgt);
#elif(RME_RV32P_REGION_NUM==8U)
    ___RME_RV32P_PMP_Set8(Raw_Pgt);
#elif(RME_RV32P_REGION_NUM==9U)
    ___RME_RV32P_PMP_Set9(Raw_Pgt);
#elif(RME_RV32P_REGION_NUM==10U)
    ___RME_RV32P_PMP_Set10(Raw_Pgt);
#elif(RME_RV32P_REGION_NUM==11U)
    ___RME_RV32P_PMP_Set11(Raw_Pgt);
#elif(RME_RV32P_REGION_NUM==12U)
    ___RME_RV32P_PMP_Set12(Raw_Pgt);
#elif(RME_RV32P_REGION_NUM==13U)
    ___RME_RV32P_PMP_Set13(Raw_Pgt);
#elif(RME_RV32P_REGION_NUM==14U)
    ___RME_RV32P_PMP_Set14(Raw_Pgt);
#elif(RME_RV32P_REGION_NUM==15U)
    ___RME_RV32P_PMP_Set15(Raw_Pgt);
#elif(RME_RV32P_REGION_NUM==16U)
    ___RME_RV32P_PMP_Set16(Raw_Pgt);
#endif
}
/* End Function:__RME_Pgt_Set ************************************************/

/* Function:__RME_Pgt_Init ****************************************************
Description : Initialize the page table data structure, according to the capability.
Input       : struct RME_Cap_Pgt* Pgt_Op - The page table to operate on.
Output      : None.
Return      : rme_ret_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
#if(RME_PGT_RAW_ENABLE==0U)
rme_ret_t __RME_Pgt_Init(struct RME_Cap_Pgt* Pgt_Op)
{
    rme_ptr_t Count;
    rme_ptr_t* Ptr;
    rme_u8_t* Cfg;

    /* Get the actual table */
    Ptr=RME_CAP_GETOBJ(Pgt_Op, rme_ptr_t*);

    /* Initialize the causal metadata */
    ((struct __RME_RV32P_Pgt_Meta*)Ptr)->Base=Pgt_Op->Base;
    ((struct __RME_RV32P_Pgt_Meta*)Ptr)->Order=Pgt_Op->Order;
    Ptr+=sizeof(struct __RME_RV32P_Pgt_Meta)/sizeof(rme_ptr_t);

    /* Is this a top-level? If it is, we need to clean up the PMP data */
    if(((Pgt_Op->Base)&RME_PGT_TOP)!=0U)
    {
        ((struct __RME_RV32P_PMP_Data*)Ptr)->Static=0U;
        for(Count=0U;Count<RME_RV32P_PMPCFG_NUM;Count++)
            ((struct __RME_RV32P_PMP_Data*)Ptr)->Raw.Cfg[Count]=0U;
        for(Count=0U;Count<RME_RV32P_REGION_NUM;Count++)
            ((struct __RME_RV32P_PMP_Data*)Ptr)->Raw.Addr[Count]=0U;

        /* "Nonexistent" entries appear as enabled but not allowing access for workaround */
        Cfg=(rme_u8_t*)(((struct __RME_RV32P_PMP_Data*)Ptr)->Raw.Cfg);
        for(Count=RME_RV32P_REGION_NUM;Count<RME_RV32P_PMPCFG_NUM*sizeof(rme_ptr_t);Count++)
            Cfg[Count]=0x18U;

        Ptr+=sizeof(struct __RME_RV32P_PMP_Data)/sizeof(rme_ptr_t);
    }

    /* Clean up the table itself - This is could be virtually unbounded if the user
     * pass in some very large length value. Need to restrict this. */
    for(Count=0U;Count<RME_POW2(RME_PGT_NMORD(Pgt_Op->Order));Count++)
        Ptr[Count]=0U;

    return 0;
}
/* End Function:__RME_Pgt_Init ***********************************************/

/* Function:__RME_Pgt_Check ***************************************************
Description : Check if the page table parameters are feasible, according to the
              parameters. This is only used in page table creation.
Input       : rme_ptr_t Base_Addr - The start mapping address.
              rme_ptr_t Is_Top - The top-level flag,
              rme_ptr_t Size_Order - The size order of the page directory.
              rme_ptr_t Num_Order - The number order of the page directory.
              rme_ptr_t Vaddr - The virtual address of the page directory.
Output      : None.
Return      : rme_ret_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
rme_ret_t __RME_Pgt_Check(rme_ptr_t Base_Addr,
                          rme_ptr_t Is_Top,
                          rme_ptr_t Size_Order,
                          rme_ptr_t Num_Order,
                          rme_ptr_t Vaddr)
{
    if(Num_Order>RME_PGT_NUM_256)
        return RME_ERR_HAL_FAIL;
    if(Size_Order<RME_PGT_SIZE_4B)
        return RME_ERR_HAL_FAIL;
    if(Size_Order>RME_PGT_SIZE_4G)
        return RME_ERR_HAL_FAIL;
    if((Vaddr&0x03U)!=0U)
        return RME_ERR_HAL_FAIL;

    return 0U;
}
/* End Function:__RME_Pgt_Check **********************************************/

/* Function:__RME_Pgt_Del_Check ***********************************************
Description : Check if the page table can be deleted. The table can only be
              deleted when there are no down- or up- mappings.
Input       : struct RME_Cap_Pgt Pgt_Op* - The page table to operate on.
Output      : None.
Return      : rme_ret_t - If can be deleted, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
rme_ret_t __RME_Pgt_Del_Check(struct RME_Cap_Pgt* Pgt_Op)
{
    /* No special property to check */
    return 0;
}
/* End Function:__RME_Pgt_Del_Check ******************************************/

/* Function:__RME_RV32P_Rand **************************************************
Description : The random number generator used for random replacement policy.
              RV32P have only one core, thus we make the LFSR local.
Input       : None.
Output      : None.
Return      : rme_ptr_t - The random number returned.
******************************************************************************/
rme_ptr_t __RME_RV32P_Rand(void)
{   
    static rme_ptr_t LFSR=0xACE1ACE1U;
    
    if((LFSR&0x01U)!=0U)
    {
        LFSR>>=1;
        LFSR^=0xB400B400U;
    }
    else
        LFSR>>=1;
    
    return LFSR;
}
/* End Function:__RME_RV32P_Rand *********************************************/

/* Function:___RME_RV32P_PMP_Decode *******************************************
Description : Decode PMP register data into stuff easier for processing.
Input       : struct __RME_RV32P_PMP_Data* Top_Data - The PMP data.
Output      : struct __RME_RV32P_PMP_Range* Range - The decoded ranges.
Return      : rme_ptr_t - The number of regions that are present.
******************************************************************************/
rme_ptr_t ___RME_RV32P_PMP_Decode(struct __RME_RV32P_PMP_Data* Top_Data,
                                  struct __RME_RV32P_PMP_Range* Range)
{
    rme_ptr_t Data_Cnt;
    rme_ptr_t Range_Cnt;
    rme_u8_t* Cfg;

    Data_Cnt=0U;
    Range_Cnt=0U;
    Cfg=(rme_u8_t*)(Top_Data->Raw.Cfg);

    while(Data_Cnt<RME_RV32P_REGION_NUM)
    {
        /* This region itself contains data - NA4 won't be used */
        if(Cfg[Data_Cnt]!=0U)
        {
            RME_ASSERT(RME_RV32P_PMP_MODE(Cfg[Data_Cnt])==RME_RV32P_PMP_NAPOT);
            Range[Range_Cnt].Flag=RME_RV32P_PMP_PERM(Cfg[Data_Cnt]);
            Range[Range_Cnt].Order_Div4=_RME_LSB_Generic(~Top_Data->Raw.Addr[Data_Cnt])+1U;
            Range[Range_Cnt].Start_Div4=Top_Data->Raw.Addr[Data_Cnt]&RME_MASK_BEGIN(Range[Range_Cnt].Order_Div4);
            /* Can't be UB here, all address [34:2] */
            Range[Range_Cnt].End_Div4=Range[Range_Cnt].Start_Div4+RME_POW2(Range[Range_Cnt].Order_Div4);
            Range_Cnt++;
            Data_Cnt++;
        }
        /* The region itself is empty, but what it follows may contain data in TOR mode */
        else
        {
            if((Data_Cnt<(RME_RV32P_REGION_NUM-1U))&&
               (RME_RV32P_PMP_MODE(Cfg[Data_Cnt+1U])==RME_RV32P_PMP_TOR))
            {
                    Range[Range_Cnt].Flag=RME_RV32P_PMP_PERM(Cfg[Data_Cnt+1U]);
                    Range[Range_Cnt].Start_Div4=Top_Data->Raw.Addr[Data_Cnt];
                    Range[Range_Cnt].End_Div4=Top_Data->Raw.Addr[Data_Cnt+1U];
                    Range[Range_Cnt].Order_Div4=0U;
                    Range_Cnt++;
                    Data_Cnt+=2U;
            }
            else
                break;
        }
    }

    return Range_Cnt;
}
/* End Function:___RME_RV32P_PMP_Decode **************************************/

/* Function:___RME_RV32P_PMP_Range_Ins ****************************************
Description : Make room for range insertion at a certain point.
Input       : struct __RME_RV32P_PMP_Range* Range - The memory ranges.
              rme_ptr_t Number - The number of memory ranges.
              rme_ptr_t Pos - The position to insert before.
Output      : struct __RME_RV32P_PMP_Range* Range - The changed ranges.
Return      : None.
******************************************************************************/
void ___RME_RV32P_PMP_Range_Ins(struct __RME_RV32P_PMP_Range* Range,
                                rme_ptr_t Number,
                                rme_ptr_t Pos)
{
    rme_ptr_t Count;

    for(Count=Number;Count>Pos;Count--)
    {
        Range[Count]=Range[Count-1U];
    }
}
/* End Function:___RME_RV32P_PMP_Range_Ins ***********************************/

/* Function:___RME_RV32P_PMP_Range_Del ****************************************
Description : Delete a range at a certain point.
Input       : struct __RME_RV32P_PMP_Range* Range - The memory ranges.
              rme_ptr_t Number - The number of memory ranges.
              rme_ptr_t Pos - The position to delete.
Output      : struct __RME_RV32P_PMP_Range* Range - The changed ranges.
Return      : None.
******************************************************************************/
void ___RME_RV32P_PMP_Range_Del(struct __RME_RV32P_PMP_Range* Range,
                                rme_ptr_t Number,
                                rme_ptr_t Pos)
{
    rme_ptr_t Count;

    for(Count=Pos;Count<Number-1U;Count++)
    {
        Range[Count]=Range[Count+1U];
    }
}
/* End Function:___RME_RV32P_PMP_Range_Del ***********************************/

/* Function:___RME_RV32P_PMP_Range_Entry **************************************
Description : Check the number of entries used with the regions.
Input       : struct __RME_RV32P_PMP_Range* Range - The memory ranges.
              rme_ptr_t Number - The number of memory ranges.
Output      : None.
Return      : rme_ptr_t - The number of entries used.
******************************************************************************/
rme_ptr_t ___RME_RV32P_PMP_Range_Entry(struct __RME_RV32P_PMP_Range* Range,
                                       rme_ptr_t Number)
{
    rme_ptr_t Count;
    rme_ptr_t Total;

    Total=0U;

    for(Count=0U;Count<Number;Count++)
    {
        /* TOR ranges use two entries */
        if(Range[Count].Order_Div4==0U)
        {
            Total+=2U;
        }
        /* NAPOT ranges use one entry */
        else
        {
            Total++;
        }
    }

    return Total;
}
/* End Function:___RME_RV32P_PMP_Range_Entry *********************************/

/* Function:___RME_RV32P_PMP_Range_Kick ***************************************
Description : Find an entry that could be kicked out, except for the entry we
              just added.
Input       : struct __RME_RV32P_PMP_Range* Range - The memory ranges.
              rme_ptr_t Number - The number of memory ranges.
              rme_ptr_t Add - The position of entry just added.
Output      : None.
Return      : rme_ptr_t - The position to kick out.
******************************************************************************/
rme_ptr_t ___RME_RV32P_PMP_Range_Kick(struct __RME_RV32P_PMP_Range* Range,
                                      rme_ptr_t Number,
                                      rme_ptr_t Add)
{
    rme_ptr_t Rand;
    rme_ptr_t Count;
    rme_ptr_t Entry;

    /* Try to kick a dynamic range */
    Rand=__RME_RV32P_Rand();
    for(Count=0U;Count<Number;Count++)
    {
        Entry=(Rand+Count)%Number;
        if(Entry==Add)
        {
            continue;
        }
        else if((Range[Entry].Flag&RME_PGT_STATIC)==0U)
        {
            return Entry;
        }
    }

    /* Cannot find a dynamic one, kick a static range */
    Entry=Rand%Number;
    if(Entry==Add)
        Entry=(Entry+1U)%Number;

    return Entry;
}
/* End Function:___RME_RV32P_PMP_Range_Kick **********************************/

/* Function:___RME_RV32P_PMP_Add **********************************************
Description : Add an entry into the ranges.
Input       : struct __RME_RV32P_PMP_Range* Range - The memory ranges.
              rme_ptr_t Number - The number of memory ranges.
              rme_ptr_t Paddr - The physical address of the page to add.
              rme_ptr_t Size_Order - The page size order.
              rme_ptr_t Flag - The flags.
Output      : struct __RME_RV32P_PMP_Range* Range - The changed ranges.
Return      : rme_ret_t - If successful, the current number of ranges; else
                          RME_ERR_HAL_FAIL.
******************************************************************************/
rme_ret_t ___RME_RV32P_PMP_Add(struct __RME_RV32P_PMP_Range* Range,
                               rme_ptr_t Number,
                               rme_ptr_t Paddr,
                               rme_ptr_t Size_Order,
                               rme_ptr_t Flag)
{
    rme_ptr_t Count;
    rme_ptr_t Start_Div4;
    rme_ptr_t End_Div4;
    rme_ptr_t Size_Div4;
    rme_ptr_t Order_Div4;
    rme_ptr_t Left;
    rme_ptr_t Right;
    rme_ptr_t Number_New;

    /* All PMP addresses are [33:2], thus no UB */
    Start_Div4=Paddr>>2U;
    Order_Div4=Size_Order-2U;
    End_Div4=Start_Div4+RME_POW2(Order_Div4);

    /* There are existing entries, look them up */
    Left=0U;
    Right=0U;
    if(Number!=0U)
    {
        /* Mapping in a 4GiB page. nothing must be existent, or we have a fault */
        if(Size_Order>=RME_WORD_BIT)
            return RME_ERR_HAL_FAIL;
        /* Mapping in a page smaller than 4GiB */
        else
        {
            /* Is this ever in the ranges? If yes, then there must be a permission conflict */
            for(Count=0U;Count<Number;Count++)
            {
                if(((Start_Div4>=Range[Number].Start_Div4)&&(Start_Div4<Range[Number].End_Div4))||
                   ((End_Div4>=Range[Number].Start_Div4)&&(End_Div4<Range[Number].End_Div4)))
                {
                    return RME_ERR_HAL_FAIL;
                }
            }

            /* No. Find the possible position of this page between the ranges */
            for(Count=0U;Count<Number;Count++)
            {
                if(Start_Div4<Range[Number].Start_Div4)
                {
                    break;
                }
            }

            /* Check possible adjacent ranges for possible mergers */
            if((Count>0U)&&
               (Range[Count-1U].End_Div4==Start_Div4)&&
               (RME_RV32P_PGT_MERGE(Range[Count-1U].Flag)==RME_RV32P_PGT_MERGE(Flag)))
            {
                Left=1U;
            }
            else if((Count<Number)&&
                    (Range[Count].Start_Div4==End_Div4)&&
                    (RME_RV32P_PGT_MERGE(Range[Count].Flag)==RME_RV32P_PGT_MERGE(Flag)))
            {
                Right=1U;
            }
        }
    }
    /* If there are no existing ranges, Don't bother */
    else
    {
        Count=0U;
    }

    /* Merge with both sides */
    if((Left!=0U)&&(Right!=0U))
    {
        /* Concatenate all ranges */
        Range[Count-1U].End_Div4=Range[Count].End_Div4;
        /* Should we use NAPOT or TOR? */
        RME_RV32P_PGT_MODE(Range[Count-1U]);
        /* Use aggregated flags from both sides */
        Range[Count-1U].Flag|=Flag|Range[Count].Flag;
        /* Clear the region that follow, range # decreases */
        ___RME_RV32P_PMP_Range_Del(Range,Number,Count);
        Number_New=Number-1U;
        Count--;
    }
    /* Merge with left side only */
    else if(Left!=0U)
    {
        Range[Count-1U].End_Div4=End_Div4;
        /* Should we use NAPOT or TOR? */
        RME_RV32P_PGT_MODE(Range[Count-1U]);
        /* Use aggregated flags from left */
        Range[Count-1U].Flag|=Flag;
        /* Range # doesn't change */
        Number_New=Number;
        Count--;
    }
    /* Merge with right side only, range # may increase */
    else if(Right!=0U)
    {
        Range[Count].Start_Div4=Start_Div4;
        /* Should we use NAPOT or TOR? */
        RME_RV32P_PGT_MODE(Range[Count]);
        /* Use aggregated flags from right */
        Range[Count].Flag|=Flag;
        /* Range # doesn't change */
        Number_New=Number;
    }
    /* Consider adding a new entry, range # will increase */
    else
    {
        /* Make room for the new entry, with NAPOT */
        ___RME_RV32P_PMP_Range_Ins(Range,Number,Count);
        Range[Count].Start_Div4=Start_Div4;
        Range[Count].End_Div4=End_Div4;
        Range[Count].Order_Div4=Order_Div4;
        Range[Count].Flag=Flag;
        /* Range # increases */
        Number_New=Number+1U;
    }

    /* We've exceeded the PMP entry capacity, need to kick someone out */
    if(___RME_RV32P_PMP_Range_Entry(Range,Number_New)>RME_RV32P_REGION_NUM)
    {
        Count=___RME_RV32P_PMP_Range_Kick(Range,Number_New,Count);
        ___RME_RV32P_PMP_Range_Del(Range,Number_New,Count);
        Number_New--;
    }

    return (rme_ret_t)Number_New;
}
/* End Function:___RME_RV32P_PMP_Add *****************************************/

/* Function:___RME_RV32P_PMP_Encode *******************************************
Description : Encode the entries back to PMP representation.
Input       : struct __RME_RV32P_PMP_Data* Top_Data - The top-level page data.
              struct __RME_RV32P_PMP_Range* Range - The memory ranges.
              rme_ptr_t Number - The number of memory ranges.
              rme_ptr_t Paddr - The physical address of the page to add.
              rme_ptr_t Size_Order - The page size order.
              rme_ptr_t Flag - The flags.
Output      : struct __RME_RV32P_PMP_Data* Top_Data - The top-level page data.
Return      : None.
******************************************************************************/
void ___RME_RV32P_PMP_Encode(struct __RME_RV32P_PMP_Data* Top_Data,
                             struct __RME_RV32P_PMP_Range* Range,
                             rme_ptr_t Number)
{
    rme_ptr_t Data_Cnt;
    rme_ptr_t Range_Cnt;
    rme_u8_t* Cfg;

    Data_Cnt=0U;
    Cfg=(rme_u8_t*)(Top_Data->Raw.Cfg);

    /* The "nonexistent" entries always default to "enable but do not allow
     * any access", for compatibility with some of the nonstandard PMP
     * implementations. These implementations allow U-mode accesses by default
     * so we have to sacrifice one region for them, and report one less region
     * to the kernel config. All PMP implementations have a even number of
     * entries, so the unused space in PMPCFG array can be used for this purpose. */
    Top_Data->Raw.Cfg[RME_RV32P_PMPCFG_NUM-1U]=0x18181818U;

    /* Decide best use of the entries */
    for(Range_Cnt=0U;Range_Cnt<Number;Range_Cnt++)
    {
        /* Using TOR - using 2 entries */
        if(Range[Range_Cnt].Order_Div4==0U)
        {
            Cfg[Data_Cnt]=0U;
            Top_Data->Raw.Addr[Data_Cnt]=Range[Range_Cnt].Start_Div4;
            Cfg[Data_Cnt+1U]=RME_RV32P_PGT_MERGE(Range[Range_Cnt].Flag)|RME_RV32P_PMP_TOR;
            Top_Data->Raw.Addr[Data_Cnt+1U]=Range[Range_Cnt].End_Div4;
            Data_Cnt+=2;
        }
        /* Using NAPOT - using 1 entry */
        else
        {
            Cfg[Data_Cnt]=RME_RV32P_PGT_MERGE(Range[Range_Cnt].Flag)|RME_RV32P_PMP_NAPOT;
            Top_Data->Raw.Addr[Data_Cnt]=Range[Range_Cnt].Start_Div4|RME_MASK_END(Range[Range_Cnt].Order_Div4-2U);
            Data_Cnt++;
        }
    }

    RME_ASSERT(Data_Cnt<=RME_RV32P_REGION_NUM);

    /* Disable the rest of entries */
    while(Data_Cnt<RME_RV32P_REGION_NUM)
    {
        Cfg[Data_Cnt]=0U;
        Data_Cnt++;
    }
}
/* End Function:___RME_RV32P_PMP_Encode **************************************/

/* Function:___RME_RV32P_PMP_Update *******************************************
Description : Update the top-level MPU metadata for this page.
              This always does addition, and does not do removal of mappings;
              For any removal, a full flush of PMP registers is required.
Input       : struct __RME_RV32P_Pgt_Meta* Top_Meta - The top-level page table.
              rme_ptr_t Paddr - The address of the page.
              rme_ptr_t Size_Order - The size order of the page.
              rme_ptr_t Flag - The flag of the page.
Output      : struct __RME_RV32P_Pgt_Meta* Top_Meta - The top-level page table.
Return      : rme_ret_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
rme_ret_t ___RME_RV32P_PMP_Update(struct __RME_RV32P_Pgt_Meta* Top_Meta,
                                  rme_ptr_t Paddr,
                                  rme_ptr_t Size_Order,
                                  rme_ptr_t Flag)
{
    rme_ret_t Number;
    struct __RME_RV32P_PMP_Data* Top_Data;
    struct __RME_RV32P_PMP_Range Range[RME_RV32P_REGION_NUM+1U];

    Top_Data=(struct __RME_RV32P_PMP_Data*)(Top_Meta+1U);

    /* Decode the PMP stuff into start/end */
    Number=___RME_RV32P_PMP_Decode(Top_Data,Range);

    /* Try to add the page into these ranges */
    Number=___RME_RV32P_PMP_Add(Range,Number,Paddr,Size_Order,Flag);
    if(Number<0)
        return RME_ERR_HAL_FAIL;

    /* Encode things back, kicking out */
    ___RME_RV32P_PMP_Encode(Top_Data,Range,Number);

    return 0;
}
/* End Function:___RME_RV32P_PMP_Update **************************************/

/* Function:__RME_Pgt_Page_Map ************************************************
Description : Map a page into the page table.
Input       : struct RME_Cap_Pgt* - The cap ability to the page table to operate on.
              rme_ptr_t Paddr - The physical address to map to.
              rme_ptr_t Pos - The position in the page table.
              rme_ptr_t Flag - The RME standard page attributes. Need to
                               translate them into architecture specific ones.
Output      : None.
Return      : rme_ret_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
rme_ret_t __RME_Pgt_Page_Map(struct RME_Cap_Pgt* Pgt_Op,
                             rme_ptr_t Paddr,
                             rme_ptr_t Pos,
                             rme_ptr_t Flag)
{
    rme_u8_t* Flagtbl;
    rme_ptr_t* Table;
    struct __RME_RV32P_Pgt_Meta* Meta;

    /* It should at least have some access permission */
    if((Flag&(RME_PGT_READ|RME_PGT_WRITE|RME_PGT_EXECUTE))==0U)
        return RME_ERR_HAL_FAIL;

    /* Get the metadata */
    Meta=RME_CAP_GETOBJ(Pgt_Op,struct __RME_RV32P_Pgt_Meta*);

    /* Where is the entry slot */
    if(((Pgt_Op->Base)&RME_PGT_TOP)!=0U)
        Table=RME_RV32P_PGT_TBL_TOP(Meta);
    else
        Table=RME_RV32P_PGT_TBL_NOM(Meta);

    /* Check if we are trying to make duplicate mappings into the same location */
    if((Table[Pos]&RME_RV32P_PGT_PRESENT)!=0U)
        return RME_ERR_HAL_FAIL;

    /* Register into the page table - PMP updated by page faults */
    Table[Pos]=RME_RV32P_PGT_PRESENT|RME_RV32P_PGT_TERMINAL|
               RME_ROUND_DOWN(Paddr,RME_PGT_SZORD(Pgt_Op->Order));
    Flagtbl=(rme_u8_t*)&Table[RME_POW2(RME_PGT_NMORD(Pgt_Op->Order))];
    Flagtbl[Pos]=Flag;

    return 0;
}
/* End Function:__RME_Pgt_Page_Map *******************************************/

/* Function:__RME_Pgt_Page_Unmap **********************************************
Description : Unmap a page from the page table.
Input       : struct RME_Cap_Pgt* - The capability to the page table to operate on.
              rme_ptr_t Pos - The position in the page table.
Output      : None.
Return      : rme_ret_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
rme_ret_t __RME_Pgt_Page_Unmap(struct RME_Cap_Pgt* Pgt_Op,
                               rme_ptr_t Pos)
{
    rme_ptr_t* Table;
    struct __RME_RV32P_Pgt_Meta* Meta;

    /* Get the metadata */
    Meta=RME_CAP_GETOBJ(Pgt_Op,struct __RME_RV32P_Pgt_Meta*);

    /* Where is the entry slot */
    if(((Pgt_Op->Base)&RME_PGT_TOP)!=0U)
        Table=RME_RV32P_PGT_TBL_TOP(Meta);
    else
        Table=RME_RV32P_PGT_TBL_NOM(Meta);

    /* Check if we are trying to remove something that does not exist, or trying to
     * remove a page directory */
    if(((Table[Pos]&RME_RV32P_PGT_PRESENT)==0)||
       ((Table[Pos]&RME_RV32P_PGT_TERMINAL)==0U))
        return RME_ERR_HAL_FAIL;

    /* We don't update the PMP: if mapping removal is needed, do a manual flush */
    Table[Pos]=0U;

    return 0;
}
/* End Function:__RME_Pgt_Page_Unmap *****************************************/

/* Function:__RME_Pgt_Pgdir_Map ***********************************************
Description : Map a page directory into the page table. This architecture does not
              support page directory flags.
Input       : struct RME_Cap_Pgt* Pgt_Parent - The parent page table.
              struct RME_Cap_Pgt* Pgt_Child - The child page table.
              rme_ptr_t Pos - The position in the destination page table.
              rme_ptr_t Flag - This have no effect for MPU-based architectures
                               (because page table addresses use up the whole word).
Output      : None.
Return      : rme_ret_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
rme_ret_t __RME_Pgt_Pgdir_Map(struct RME_Cap_Pgt* Pgt_Parent,
                              rme_ptr_t Pos,
                              struct RME_Cap_Pgt* Pgt_Child,
                              rme_ptr_t Flag)
{
    rme_u8_t* Flagtbl;
    rme_ptr_t* Table;
    struct __RME_RV32P_Pgt_Meta* Parent_Meta;
    struct __RME_RV32P_Pgt_Meta* Child_Meta;

    /* Get the metadata */
    Parent_Meta=RME_CAP_GETOBJ(Pgt_Parent,struct __RME_RV32P_Pgt_Meta*);
    Child_Meta=RME_CAP_GETOBJ(Pgt_Child,struct __RME_RV32P_Pgt_Meta*);

    /* The child must not be a top-level */
    if(((Child_Meta->Base)&RME_PGT_TOP)!=0U)
        return RME_ERR_HAL_FAIL;

    /* Where is the entry slot? */
    if(((Parent_Meta->Base)&RME_PGT_TOP)!=0U)
        Table=RME_RV32P_PGT_TBL_TOP(Parent_Meta);
    else
        Table=RME_RV32P_PGT_TBL_NOM(Parent_Meta);

    /* Check if anything already mapped in */
    if((Table[Pos]&RME_RV32P_PGT_PRESENT)!=0U)
        return RME_ERR_HAL_FAIL;

    /* Register into the page table - PMP updated by page faults */
    Table[Pos]=RME_RV32P_PGT_PRESENT|RME_RV32P_PGT_PGD_ADDR((rme_ptr_t)Child_Meta);
    Flagtbl=(rme_u8_t*)&Table[RME_POW2(RME_PGT_NMORD(Pgt_Parent->Order))];
    Flagtbl[Pos]=Flag;

    return 0;
}
/* End Function:__RME_Pgt_Pgdir_Map ******************************************/

/* Function:__RME_Pgt_Pgdir_Unmap *********************************************
Description : Unmap a page directory from the page table.
Input       : struct RME_Cap_Pgt* Pgt_Parent - The parent page table to unmap from.
              rme_ptr_t Pos - The position in the page table.
              struct RME_Cap_Pgt* Pgt_Child - The child page table to unmap.
Output      : None.
Return      : rme_ret_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
rme_ret_t __RME_Pgt_Pgdir_Unmap(struct RME_Cap_Pgt* Pgt_Parent,
                                rme_ptr_t Pos,
                                struct RME_Cap_Pgt* Pgt_Child)
{
    rme_ptr_t* Table;
    struct __RME_RV32P_Pgt_Meta* Parent_Meta;
    struct __RME_RV32P_Pgt_Meta* Child_Meta;

    /* Get the metadata */
    Parent_Meta=RME_CAP_GETOBJ(Pgt_Parent,struct __RME_RV32P_Pgt_Meta*);

    /* Where is the entry slot */
    if(((Pgt_Parent->Base)&RME_PGT_TOP)!=0U)
        Table=RME_RV32P_PGT_TBL_TOP(Parent_Meta);
    else
        Table=RME_RV32P_PGT_TBL_NOM(Parent_Meta);

    /* Check if we try to remove something nonexistent, or a page */
    if(((Table[Pos]&RME_RV32P_PGT_PRESENT)==0U)||
       ((Table[Pos]&RME_RV32P_PGT_TERMINAL)!=0U))
        return RME_ERR_HAL_FAIL;

    /* See if the child page table is actually mapped there */
    Child_Meta=(struct __RME_RV32P_Pgt_Meta*)RME_RV32P_PGT_PGD_ADDR(Table[Pos]);
    if(Child_Meta!=RME_CAP_GETOBJ(Pgt_Child,struct __RME_RV32P_Pgt_Meta*))
        return RME_ERR_HAL_FAIL;

    /* We don't update the PMP: if mapping removal is needed, do a manual flush */
    Table[Pos]=0U;

    return 0;
}
/* End Function:__RME_Pgt_Pgdir_Unmap ****************************************/

/* Function:__RME_Pgt_Lookup **************************************************
Description : Lookup a page entry in a page directory.
Input       : struct RME_Cap_Pgt* Pgt_Op - The page directory to lookup.
              rme_ptr_t Pos - The position to look up.
Output      : rme_ptr_t* Paddr - The physical address of the page.
              rme_ptr_t* Flag - The RME standard flags of the page.
Return      : rme_ret_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
rme_ret_t __RME_Pgt_Lookup(struct RME_Cap_Pgt* Pgt_Op,
                           rme_ptr_t Pos,
                           rme_ptr_t* Paddr,
                           rme_ptr_t* Flag)
{
    rme_u8_t* Flagtbl;
    rme_ptr_t* Table;

    /* Check if this is the top-level page table. Get the table */
    if(((Pgt_Op->Base)&RME_PGT_TOP)!=0U)
        Table=RME_RV32P_PGT_TBL_TOP(RME_CAP_GETOBJ(Pgt_Op,rme_ptr_t*));
    else
        Table=RME_RV32P_PGT_TBL_NOM(RME_CAP_GETOBJ(Pgt_Op,rme_ptr_t*));

    /* Start lookup */
    if(((Table[Pos]&RME_RV32P_PGT_PRESENT)==0U)||
       ((Table[Pos]&RME_RV32P_PGT_TERMINAL)==0U))
        return RME_ERR_HAL_FAIL;

    /* This is a page. Return the physical address and flags */
    if(Paddr!=RME_NULL)
        *Paddr=RME_RV32P_PGT_PTE_ADDR(Table[Pos]);

    /* The flags follow the pages */
    if(Flag!=RME_NULL)
    {
        Flagtbl=(rme_u8_t*)&Table[RME_POW2(RME_PGT_NMORD(Pgt_Op->Order))];
        *Flag=Flagtbl[Pos];
    }

    return 0;
}
/* End Function:__RME_Pgt_Lookup *********************************************/

/* Function:__RME_Pgt_Walk ****************************************************
Description : Walking function for the page table. This function just does page
              table lookups. The page table that is being walked must be the top-
              level page table. The output values are optional; only pass in pointers
              when you need that value.
Input       : struct RME_Cap_Pgt* Pgt_Op - The page table to walk.
              rme_ptr_t Vaddr - The virtual address to look up.
Output      : rme_ptr_t* Pgt - The pointer to the page table level.
              rme_ptr_t* Map_Vaddr - The virtual address that starts mapping.
              rme_ptr_t* Paddr - The physical address of the page.
              rme_ptr_t* Size_Order - The size order of the page.
              rme_ptr_t* Num_Order - The entry order of the page.
              rme_ptr_t* Flags - The RME standard flags of the page.
Return      : rme_ret_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
rme_ret_t __RME_Pgt_Walk(struct RME_Cap_Pgt* Pgt_Op,
                         rme_ptr_t Vaddr,
                         rme_ptr_t* Pgt,
                         rme_ptr_t* Map_Vaddr,
                         rme_ptr_t* Paddr,
                         rme_ptr_t* Size_Order,
                         rme_ptr_t* Num_Order,
                         rme_ptr_t* Flag)
{
    struct __RME_RV32P_Pgt_Meta* Meta;
    rme_ptr_t* Table;
    rme_u8_t* Flagtbl;
    rme_ptr_t Pos;
    rme_ptr_t Shift;
    rme_ptr_t Num;
    rme_u8_t Flag_Final;

    /* This must the top-level page table */
    RME_ASSERT(((Pgt_Op->Base)&RME_PGT_TOP)!=0U);

    /* Get the table and start lookup */
    Meta=RME_CAP_GETOBJ(Pgt_Op,struct __RME_RV32P_Pgt_Meta*);
    Table=RME_RV32P_PGT_TBL_TOP(Meta);

    /* Do lookup recursively */
    Flag_Final=RME_PGT_ALL_PERM;
    while(1)
    {
        /* Check if the virtual address is in our range */
        if(Vaddr<RME_PGT_BASE(Meta->Base))
            return RME_ERR_HAL_FAIL;
        /* Calculate entry position - shifting by RME_WORD_BIT or more is UB */
        Shift=RME_PGT_SZORD(Meta->Order);
        if(Shift>=RME_WORD_BIT)
            Pos=0U;
        else
            Pos=(Vaddr-RME_PGT_BASE(Meta->Base))>>Shift;
        /* See if the entry is overrange */
        Num=RME_POW2(RME_PGT_NMORD(Meta->Order));
        if(Pos>=Num)
            return RME_ERR_HAL_FAIL;
        /* See if the entry exists */
        if((Table[Pos]&RME_RV32P_PGT_PRESENT)==0U)
            return RME_ERR_HAL_FAIL;
        /* Find the position of the entry - Is there a page, a directory, or nothing? */
        Flagtbl=(rme_u8_t*)(&Table[Num]);
        if((Table[Pos]&RME_RV32P_PGT_TERMINAL)!=0U)
        {
            /* This is a page - we found it */
            if(Pgt!=RME_NULL)
                *Pgt=(rme_ptr_t)Meta;
            if(Map_Vaddr!=RME_NULL)
            {
                if(Shift>=RME_WORD_BIT)
                    *Map_Vaddr=RME_PGT_BASE(Meta->Base);
                else
                    *Map_Vaddr=RME_PGT_BASE(Meta->Base)+(Pos<<Shift);
            }
            if(Paddr!=RME_NULL)
            {
                if(Shift>=RME_WORD_BIT)
                    *Paddr=RME_PGT_BASE(Meta->Base);
                else
                    *Paddr=RME_PGT_BASE(Meta->Base)+(Pos<<Shift);
            }
            if(Size_Order!=RME_NULL)
                *Size_Order=RME_PGT_SZORD(Meta->Order);
            if(Num_Order!=RME_NULL)
                *Num_Order=RME_PGT_NMORD(Meta->Order);
            if(Flag!=RME_NULL)
                *Flag=Flag_Final&Flagtbl[Pos];

            break;
        }
        else
        {
            /* Accumulate flags on the way, if needed */
            if(Flag!=RME_NULL)
                Flag_Final&=Flagtbl[Pos];
            /* This is a directory, we goto that directory to continue walking */
            Meta=(struct __RME_RV32P_Pgt_Meta*)RME_RV32P_PGT_PGD_ADDR(Table[Pos]);
            Table=RME_RV32P_PGT_TBL_NOM(Meta);
        }
    }

    return 0;
}
#endif
/* End Function:__RME_Pgt_Walk ***********************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
