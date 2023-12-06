/******************************************************************************
Filename    : rme_platform_a6m.c
Author      : pry
Date        : 01/04/2017
Licence     : The Unlicense; see LICENSE for details.
Description : The hardware abstraction layer for ARMv6-M microcontrollers.

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
The page table conforms to page table implementation style I, which states that
the page table trees shall not share anything and each tree must be constructed
in a high-to-low order. This is due to the complete lack of possibility of error
handling on this processor. All pages must be STATIC.
******************************************************************************/

/* Include *******************************************************************/
#define __HDR_DEF__
#include "Platform/A6M/rme_platform_a6m.h"
#include "Kernel/rme_kernel.h"
#undef __HDR_DEF__

#define __HDR_STRUCT__
#include "Platform/A6M/rme_platform_a6m.h"
#include "Kernel/rme_kernel.h"
#undef __HDR_STRUCT__

/* Private include */
#include "Platform/A6M/rme_platform_a6m.h"

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
Description : Output a character to console. In ARMv6-M, under most circumstances, 
              we should use the ITM or serial for such outputs.
Input       : char Char - The character to print.
Output      : None.
Return      : rme_ptr_t - Always 0.
******************************************************************************/
#if(RME_DEBUG_PRINT==1U)
rme_ptr_t __RME_Putchar(char Char)
{
    RME_A6M_PUTCHAR(Char);
    return 0U;
}
#endif
/* End Function:__RME_Putchar ************************************************/

/* Function:__RME_CPUID_Get ***************************************************
Description : Get the CPUID. This is to identify where we are executing.
Input       : None.
Output      : None.
Return      : rme_ptr_t - The CPUID. On ARMv6-M, this is certainly always 0.
******************************************************************************/
rme_ptr_t __RME_CPUID_Get(void)
{
    return 0U;
}
/* End Function:__RME_CPUID_Get **********************************************/

/* Function:__RME_A6M_Exc_Handler *********************************************
Description : The fault handler of RME. All ARMv6-M's faults are considered
              fatal due to the lack of appropriate exception information.
Input       : struct RME_Reg_Struct* Reg - The register set.
Output      : struct RME_Reg_Struct* Reg - The updated register set.
Return      : None.
******************************************************************************/
void __RME_A6M_Exc_Handler(struct RME_Reg_Struct* Reg)
{
    struct RME_Thd_Struct* Thd_Cur;
    
    /* Is it a kernel-level fault? If yes, panic */
    RME_ASSERT((Reg->LR&RME_A6M_EXC_RET_RET_USER)!=0U);
    
    /* Get the address of this faulty address, and what caused this fault */
    Thd_Cur=RME_A6M_Local.Thd_Cur;
    
    /* Are we activating the NMI? If yes, we directly soft lockup */
    RME_ASSERT((RME_A6M_SCB_ICSR&RME_A6M_ICSR_NMIPENDSET)==0U);
    
    /* Clear SVC pend indicators, because the fault could have happened before
     * the SVC. The access to SHCSR is implementation defined (always a no in 
     * ARMv6-M0+, in fact), so if the processor does not support this, we're
     * left with nothing to help. This is not the case in ARMv6-M where it's 
     * guaranteed to be accessible by the processor. */
    RME_A6M_SCB_SHCSR&=~RME_A6M_SCB_SHCSR_SVCALLPENDED;
    
    /* ARMv6-M have no fault indicators. Any fault is fatal */
    Thd_Cur->Ctx.Reg->Exc.Cause=1U;
    _RME_Thd_Fatal(Reg);
    
    /* Make sure the LR returns to the user level */
    RME_A6M_EXC_RET_FIX(Reg);
}
/* End Function:__RME_A6M_Exc_Handler ****************************************/

/* Function:__RME_A6M_Flag_Fast ***********************************************
Description : Set a fast flag in a flag set. Works for timer interrupts only.
Input       : rme_ptr_t Base - The base address of the flagset.
              rme_ptr_t Size - The size of the flagset.
              rme_ptr_t Flag - The fast flagset to program.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_A6M_Flag_Fast(rme_ptr_t Base,
                         rme_ptr_t Size,
                         rme_ptr_t Flag)
{
    volatile struct __RME_RVM_Flag* Set;
    
    /* Choose a data structure that is not locked at the moment */
    Set=RME_RVM_FLAG_SET(Base, Size, 0U);
    if(Set->Lock!=0U)
        Set=RME_RVM_FLAG_SET(Base, Size, 1U);
    
    /* Set the flags for this interrupt source */
    Set->Fast|=Flag;
}
/* End Function:__RME_A6M_Flag_Fast ******************************************/

/* Function:__RME_A6M_Flag_Slow ***********************************************
Description : Set a slow flag in a flag set. Works for both vectors and events.
Input       : rme_ptr_t Base - The base address of the flagset.
              rme_ptr_t Size - The size of the flagset.
              rme_ptr_t Pos - The position in the flagset to set.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_A6M_Flag_Slow(rme_ptr_t Base,
                         rme_ptr_t Size,
                         rme_ptr_t Pos)
{
    volatile struct __RME_RVM_Flag* Set;
    
    /* Choose a data structure that is not locked at the moment */
    Set=RME_RVM_FLAG_SET(Base, Size, 0U);
    if(Set->Lock!=0U)
        Set=RME_RVM_FLAG_SET(Base, Size, 1U);
    
    /* Set the flags for this interrupt source */
    Set->Group|=RME_POW2(Pos>>RME_WORD_ORDER);
    Set->Flag[Pos>>RME_WORD_ORDER]|=RME_POW2(Pos&RME_MASK_END(RME_WORD_ORDER-1U));
}
/* End Function:__RME_A6M_Flag_Slow ******************************************/

/* Function:__RME_A6M_Vct_Handler *********************************************
Description : The generic interrupt handler of RME for ARMv6-M.
Input       : struct RME_Reg_Struct* Reg - The register set.
              rme_ptr_t Vct_Num - The vector number. For ARMv6-M, this is in accordance
                                   with the ARMv6-M architecture reference manual.
Output      : volatile struct RME_Reg_Struct* Reg - The update register set.
Return      : None.
******************************************************************************/
void __RME_A6M_Vct_Handler(struct RME_Reg_Struct* Reg, rme_ptr_t Vct_Num)
{
#if(RME_RVM_GEN_ENABLE==1U)
    /* If the user wants to bypass, we skip the flag marshalling & sending process */
    if(RME_Boot_Vct_Handler(Vct_Num)==0U)
        return;
    
    __RME_A6M_Flag_Slow(RME_RVM_PHYS_VCTF_BASE, RME_RVM_PHYS_VCTF_SIZE, Vct_Num);
#endif
    
    _RME_Kern_Snd(RME_A6M_Local.Sig_Vct);
    /* Remember to pick the guy with the highest priority after we did all sends */
    _RME_Kern_High(Reg, &RME_A6M_Local);
    
    /* Make sure the LR returns to the user level */
    RME_A6M_EXC_RET_FIX(Reg);
}
/* End Function:__RME_A6M_Vct_Handler ****************************************/

/* Function:__RME_A6M_Tim_Handler *********************************************
Description : The timer interrupt handler of RME for ARMv6-M.
Input       : struct RME_Reg_Struct* Reg - The register set.
Output      : struct RME_Reg_Struct* Reg - The update register set.
Return      : None.
******************************************************************************/
void __RME_A6M_Tim_Handler(struct RME_Reg_Struct* Reg)
{
    RME_A6M_Timestamp++;

#if(RME_RVM_GEN_ENABLE==1U)
    __RME_A6M_Flag_Fast(RME_RVM_PHYS_VCTF_BASE, RME_RVM_PHYS_VCTF_SIZE, 1U);
#endif
    
    _RME_Tim_Handler(Reg, 1U);
    
    /* Make sure the LR returns to the user level */
    RME_A6M_EXC_RET_FIX(Reg);
}
/* End Function:__RME_A6M_Tim_Handler ****************************************/

/* Function:__RME_A6M_Svc_Handler *********************************************
Description : The timer interrupt handler of RME for ARMv6-M.
Input       : struct RME_Reg_Struct* Reg - The register set.
Output      : struct RME_Reg_Struct* Reg - The update register set.
Return      : None.
******************************************************************************/
void __RME_A6M_Svc_Handler(struct RME_Reg_Struct* Reg)
{   
    _RME_Svc_Handler(Reg);
    
    /* Make sure the LR returns to the user level */
    RME_A6M_EXC_RET_FIX(Reg);
}
/* End Function:__RME_A6M_Svc_Handler ****************************************/

/* Function:__RME_A6M_Pgt_Entry_Mod *******************************************
Description : Consult or modify the page table attributes. ARMv6-M only allows 
              consulting page table attributes but does not allow modifying them,
              because there are no architecture-specific flags.
Input       : struct RME_Cap_Cpt* Cpt - The current capability table.
              rme_cid_t Cap_Pgt - The capability to the top-level page table to consult.
              rme_ptr_t Vaddr - The virtual address to consult.
              rme_ptr_t Type - The consult type, see manual for details.
Output      : None.
Return      : rme_ret_t - If successful, the flags; else RME_ERR_KFN_FAIL.
******************************************************************************/
#if(RME_PGT_RAW_USER==0U)
rme_ret_t __RME_A6M_Pgt_Entry_Mod(struct RME_Cap_Cpt* Cpt, 
                                  rme_cid_t Cap_Pgt,
                                  rme_ptr_t Vaddr,
                                  rme_ptr_t Type)
{
    struct RME_Cap_Pgt* Pgt_Op;
    rme_ptr_t Type_Stat;
    rme_ptr_t Size_Order;
    rme_ptr_t Num_Order;
    rme_ptr_t Flag;
    
    /* Get the capability slot */
    RME_CPT_GETCAP(Cpt,Cap_Pgt,RME_CAP_TYPE_PGT,struct RME_Cap_Pgt*,Pgt_Op,Type_Stat);
    
    if(__RME_Pgt_Walk(Pgt_Op,Vaddr,0U,0U,0U,&Size_Order,&Num_Order,&Flag)!=0U)
        return RME_ERR_KFN_FAIL;
    
    switch(Type)
    {
        case RME_A6M_KFN_PGT_ENTRY_MOD_FLAG_GET: return (rme_ret_t)Flag;
        case RME_A6M_KFN_PGT_ENTRY_MOD_SZORD_GET: return (rme_ret_t)Size_Order;
        case RME_A6M_KFN_PGT_ENTRY_MOD_NMORD_GET: return (rme_ret_t)Num_Order;
        default:break;
    }
    
    return RME_ERR_KFN_FAIL;
}
#endif
/* End Function:__RME_A6M_Pgt_Entry_Mod **************************************/

/* Function:__RME_A6M_Int_Local_Mod *******************************************
Description : Consult or modify the local interrupt controller's vector state.
Input       : rme_ptr_t Int_Num - The interrupt number to consult or modify.
              rme_ptr_t Operation - The operation to conduct.
              rme_ptr_t Param - The parameter, could be state or priority.
Output      : None.
Return      : rme_ret_t - If successful, 0 or the desired value; else RME_ERR_KFN_FAIL.
******************************************************************************/
rme_ret_t __RME_A6M_Int_Local_Mod(rme_ptr_t Int_Num,
                                  rme_ptr_t Operation,
                                  rme_ptr_t Param)
{
    if(Int_Num>=RME_RVM_PHYS_VCT_NUM)
        return RME_ERR_KFN_FAIL;
    
    switch(Operation)
    {
        case RME_A6M_KFN_INT_LOCAL_MOD_STATE_GET:
        {
            if((RME_A6M_NVIC_ISER(Int_Num)&RME_POW2(Int_Num&0x1FU))==0U)
                return 0U;
            else
                return 1U;
        }
        case RME_A6M_KFN_INT_LOCAL_MOD_STATE_SET:
        {
            if(Param==0U)
                RME_A6M_NVIC_ICER(Int_Num)=RME_POW2(Int_Num&0x1FU);
            else
                RME_A6M_NVIC_ISER(Int_Num)=RME_POW2(Int_Num&0x1FU);
            
            return 0U;
        }
        case RME_A6M_KFN_INT_LOCAL_MOD_PRIO_GET:
        {
            return RME_A6M_NVIC_IPR_GET(Int_Num);
        }
        case RME_A6M_KFN_INT_LOCAL_MOD_PRIO_SET:
        {
            if(Param>0xFFU)
                return RME_ERR_KFN_FAIL;
            
            RME_A6M_NVIC_IPR_SET(Int_Num, Param);
            return 0U;
        }
        default:break;
    }
    
    return RME_ERR_KFN_FAIL;
}
/* End Function:__RME_A6M_Int_Local_Mod **************************************/

/* Function:__RME_A6M_Int_Local_Trig ******************************************
Description : Trigger a CPU's local event source.
Input       : rme_ptr_t CPUID - The ID of the CPU. For ARMv6-M, this must be 0.
              rme_ptr_t Evt_Num - The event ID.
Output      : None.
Return      : rme_ret_t - If successful, 0; else RME_ERR_KFN_FAIL.
******************************************************************************/
rme_ret_t __RME_A6M_Int_Local_Trig(rme_ptr_t CPUID,
                                   rme_ptr_t Int_Num)
{
    if(CPUID!=0U)
        return RME_ERR_KFN_FAIL;

    if(Int_Num>=RME_RVM_PHYS_VCT_NUM)
        return RME_ERR_KFN_FAIL;
    
    /* Trigger the interrupt - have to use ISPR directly */
    RME_A6M_NVIC_ISPR|=RME_POW2(Int_Num);

    return 0U;
}
/* End Function:__RME_A6M_Int_Local_Trig *************************************/

/* Function:__RME_A6M_Evt_Local_Trig ******************************************
Description : Trigger a CPU's local event source.
Input       : struct RME_Reg_Struct* Reg - The register set.
              rme_ptr_t CPUID - The ID of the CPU. For ARMv6-M, this must be 0.
              rme_ptr_t Evt_Num - The event ID.
Output      : None.
Return      : rme_ret_t - If successful, 0; else RME_ERR_KFN_FAIL.
******************************************************************************/
rme_ret_t __RME_A6M_Evt_Local_Trig(struct RME_Reg_Struct* Reg,
                                   rme_ptr_t CPUID,
                                   rme_ptr_t Evt_Num)
{
    if(CPUID!=0U)
        return RME_ERR_KFN_FAIL;

    if(Evt_Num>=RME_RVM_VIRT_EVT_NUM)
        return RME_ERR_KFN_FAIL;

    __RME_A6M_Flag_Slow(RME_RVM_VIRT_EVTF_BASE, RME_RVM_VIRT_EVTF_SIZE, Evt_Num);
    
    if(_RME_Kern_Snd(RME_A6M_Local.Sig_Vct)!=0U)
        return RME_ERR_KFN_FAIL;
    
    /* Set return value first before we really do context switch */
    __RME_Svc_Retval_Set(Reg, 0);
    
    _RME_Kern_High(Reg, &RME_A6M_Local);

    return 0U;
}
/* End Function:__RME_A6M_Evt_Local_Trig *************************************/

/* Function:__RME_A6M_Prfth_Mod ***********************************************
Description : Modify prefetcher state. Due to the fact that the ARMv6-M architectural
              prefetch is usually permanently enabled, this only controls manufacturer
              specific Flash accelerators. The accelerator is always enabled at
              power-on, and the only reason you want to fiddle with it is to save power.
Input       : rme_ptr_t Prfth_ID - The ID of the prefetcher to enable, disable or consult.
              rme_ptr_t Operation - The operation to perform.
              rme_ptr_t Param - The parameter.
Output      : None.
Return      : If successful, 0; else RME_ERR_KFN_FAIL.
******************************************************************************/
rme_ret_t __RME_A6M_Prfth_Mod(rme_ptr_t Prfth_ID,
                              rme_ptr_t Operation,
                              rme_ptr_t Param)
{
    if(Prfth_ID!=0U)
        return RME_ERR_KFN_FAIL;
    
    if(Operation==RME_A6M_KFN_PRFTH_MOD_STATE_SET)
    {
        if(Param==RME_A6M_KFN_PRFTH_STATE_ENABLE)
            RME_A6M_PRFTH_STATE_SET(1U);
        else if(Param==RME_A6M_KFN_PRFTH_STATE_DISABLE)
            RME_A6M_PRFTH_STATE_SET(0U);
        else
            return RME_ERR_KFN_FAIL;
        
        __RME_A6M_Barrier();
        return 0U;
    }
    else if(Operation==RME_A6M_KFN_PRFTH_MOD_STATE_GET)
    {
        if(RME_A6M_PRFTH_STATE_GET()==0U)
            return RME_A6M_KFN_PRFTH_STATE_DISABLE;
        else
            return RME_A6M_KFN_PRFTH_STATE_ENABLE;
    }
    
    return RME_ERR_KFN_FAIL;
}
/* End Function:__RME_A6M_Prfth_Mod ******************************************/

/* Function:__RME_A6M_Perf_CPU_Func *******************************************
Description : CPU feature detection for ARMv6-M.
Input       : struct RME_Reg_Struct* Reg - The register set.
              rme_ptr_t Freg_ID - The capability to the thread to consult.
Output      : struct RME_Reg_Struct* Reg - The updated register set.
Return      : rme_ret_t - If successful, 0; if a negative value, failed.
******************************************************************************/
rme_ret_t __RME_A6M_Perf_CPU_Func(struct RME_Reg_Struct* Reg,
                                  rme_ptr_t Freg_ID)
{
    switch(Freg_ID)
    {
        case RME_A6M_KFN_CPU_FUNC_CPUID:            {Reg->R6=RME_A6M_SCB_CPUID;break;}
        case RME_A6M_KFN_CPU_FUNC_MPU_TYPE:         {Reg->R6=RME_A6M_MPU_CTRL;break;}
        case RME_A6M_KFN_CPU_FUNC_PID0:             {Reg->R6=RME_A6M_SCNSCB_PID0;break;}
        case RME_A6M_KFN_CPU_FUNC_PID1:             {Reg->R6=RME_A6M_SCNSCB_PID1;break;}
        case RME_A6M_KFN_CPU_FUNC_PID2:             {Reg->R6=RME_A6M_SCNSCB_PID2;break;}
        case RME_A6M_KFN_CPU_FUNC_PID3:             {Reg->R6=RME_A6M_SCNSCB_PID3;break;}
        case RME_A6M_KFN_CPU_FUNC_PID4:             {Reg->R6=RME_A6M_SCNSCB_PID4;break;}
        case RME_A6M_KFN_CPU_FUNC_PID5:             {Reg->R6=RME_A6M_SCNSCB_PID5;break;}
        case RME_A6M_KFN_CPU_FUNC_PID6:             {Reg->R6=RME_A6M_SCNSCB_PID6;break;}
        case RME_A6M_KFN_CPU_FUNC_PID7:             {Reg->R6=RME_A6M_SCNSCB_PID7;break;}
        case RME_A6M_KFN_CPU_FUNC_CID0:             {Reg->R6=RME_A6M_SCNSCB_CID0;break;}
        case RME_A6M_KFN_CPU_FUNC_CID1:             {Reg->R6=RME_A6M_SCNSCB_CID1;break;}
        case RME_A6M_KFN_CPU_FUNC_CID2:             {Reg->R6=RME_A6M_SCNSCB_CID2;break;}
        case RME_A6M_KFN_CPU_FUNC_CID3:             {Reg->R6=RME_A6M_SCNSCB_CID3;break;}
        default:                                    {return RME_ERR_KFN_FAIL;}
    }

    return 0U;
}
/* End Function:__RME_A6M_Perf_CPU_Func **************************************/

/* Function:__RME_A6M_Perf_Mon_Mod ********************************************
Description : Read or write performance monitor settings. This only works for
              a single performance counter, CYCCNT, and only works for enabling 
              or disabling operations.
Input       : rme_ptr_t Perf_ID - The performance monitor identifier.
              rme_ptr_t Operation - The operation to perform.
              rme_ptr_t Param - An extra parameter.
Output      : None.
Return      : rme_ret_t - If successful, the desired value; if a negative value, failed.
******************************************************************************/
rme_ret_t __RME_A6M_Perf_Mon_Mod(rme_ptr_t Perf_ID,
                                 rme_ptr_t Operation,
                                 rme_ptr_t Param)
{
    if(Perf_ID!=RME_A6M_KFN_PERF_CYCLE_CYCCNT)
        return RME_ERR_KFN_FAIL;

    /* Do we have this counter at all? */
    if((RME_A6M_DWT_CTRL&RME_A6M_DWT_CTRL_NOCYCCNT)!=0U)
        return RME_ERR_KFN_FAIL;
    
    if(Operation==RME_A6M_KFN_PERF_STATE_GET)
        return RME_A6M_DWT_CTRL&RME_A6M_DWT_CTRL_CYCCNTENA;
    else if(Operation==RME_A6M_KFN_PERF_STATE_SET)
    {
        if(Param==RME_A6M_KFN_PERF_STATE_DISABLE)
            RME_A6M_DWT_CTRL&=~RME_A6M_DWT_CTRL_CYCCNTENA;
        else if(Param==RME_A6M_KFN_PERF_STATE_ENABLE)
            RME_A6M_DWT_CTRL|=RME_A6M_DWT_CTRL_CYCCNTENA;
        else
            return RME_ERR_KFN_FAIL;

        return 0U;
    }
    
    return RME_ERR_KFN_FAIL;
}
/* End Function:__RME_A6M_Perf_Mon_Mod ***************************************/

/* Function:__RME_A6M_Perf_Cycle_Mod ******************************************
Description : Cycle performance counter read or write for ARMv6-M. Only supports
              CYCCNT register.
Input       : struct RME_Reg_Struct* Reg - The current register set.
              rme_ptr_t Cycle_ID - The performance counter to read or write.
Output      : struct RME_Reg_Struct* Reg - The register set when exiting the handler.
Return      : rme_ret_t - If successful, 0; if a negative value, failed.
******************************************************************************/
rme_ret_t __RME_A6M_Perf_Cycle_Mod(struct RME_Reg_Struct* Reg,
                                   rme_ptr_t Cycle_ID, 
                                   rme_ptr_t Operation,
                                   rme_ptr_t Value)
{
    if(Cycle_ID!=RME_A6M_KFN_PERF_CYCLE_CYCCNT)
        return RME_ERR_KFN_FAIL;
    
    /* Do we have this counter at all? */
    if((RME_A6M_DWT_CTRL&RME_A6M_DWT_CTRL_NOCYCCNT)!=0U)
        return RME_ERR_KFN_FAIL;

    /* We do have the counter, access it */
    if(Operation==RME_A6M_KFN_PERF_VAL_GET)
        Reg->R6=RME_A6M_DWT_CYCCNT;
    else if(Operation==RME_A6M_KFN_PERF_VAL_SET)
        RME_A6M_DWT_CYCCNT=Value;
    else
        return RME_ERR_KFN_FAIL;

    return 0U;
}
/* End Function:__RME_A6M_Perf_Cycle_Mod *************************************/

/* Function:__RME_A6M_Debug_Reg_Mod *******************************************
Description : Debug regular register modification implementation for ARMv6-M.
Input       : struct RME_Cap_Cpt* Cpt - The current capability table.
              struct RME_Reg_Struct* Reg - The current register set.
              rme_cid_t Cap_Thd - The capability to the thread to consult.
              rme_ptr_t Operation - The operation, e.g. which register to read or write.
              rme_ptr_t Value - The value to write into the register.
Output      : struct RME_Reg_Struct* Reg - The register set when exiting the handler.
Return      : rme_ret_t - If successful, 0; if a negative value, failed.
******************************************************************************/
rme_ret_t __RME_A6M_Debug_Reg_Mod(struct RME_Cap_Cpt* Cpt,
                                  struct RME_Reg_Struct* Reg, 
                                  rme_cid_t Cap_Thd,
                                  rme_ptr_t Operation,
                                  rme_ptr_t Value)
{
    struct RME_Cap_Thd* Thd_Op;
    struct RME_Thd_Struct* Thd_Struct;
    volatile struct RME_CPU_Local* Local;
    volatile rme_ptr_t* Position;
    rme_ptr_t Register;
    rme_ptr_t Type_Stat;
    
    /* Get the capability slot */
    RME_CPT_GETCAP(Cpt, Cap_Thd, RME_CAP_TYPE_THD, struct RME_Cap_Thd*, Thd_Op, Type_Stat);
    
    /* See if the target thread is already bound. If no or bound to other cores, we just quit */
    Local=RME_CPU_LOCAL();
    Thd_Struct=(struct RME_Thd_Struct*)Thd_Op->Head.Object;
    if(Thd_Struct->Sched.Local!=Local)
        return RME_ERR_PTH_INVSTATE;
    
    /* Register validity check */
    Register=Operation&(~RME_A6M_KFN_DEBUG_REG_MOD_SET);
    if(Register<=RME_A6M_KFN_DEBUG_REG_MOD_LR)
        Position=&(((volatile rme_ptr_t*)&(Thd_Struct->Ctx.Reg->Reg))[Register]);
    else
        return RME_ERR_KFN_FAIL;
    
    /* Perform read/write */
    if((Operation&RME_A6M_KFN_DEBUG_REG_MOD_SET)==0U)
        Reg->R6=Position[0];
    else
        Position[0]=Reg->R6;

    return 0;
}
/* End Function:__RME_A6M_Debug_Reg_Mod **************************************/

/* Function:__RME_A6M_Debug_Inv_Mod *******************************************
Description : Debug invocation register modification implementation for ARMv6-M.
Input       : struct RME_Cap_Cpt* Cpt - The current capability table.
              struct RME_Reg_Struct* Reg - The current register set.
              rme_cid_t Cap_Thd - The capability to the thread to consult.
              rme_ptr_t Operation - The operation, e.g. which register to read or write.
              rme_ptr_t Value - The value to write into the register.
Output      : struct RME_Reg_Struct* Reg - The register set when exiting the handler.
Return      : rme_ret_t - If successful, 0; if a negative value, failed.
******************************************************************************/
rme_ret_t __RME_A6M_Debug_Inv_Mod(struct RME_Cap_Cpt* Cpt,
                                  struct RME_Reg_Struct* Reg, 
                                  rme_cid_t Cap_Thd,
                                  rme_ptr_t Operation,
                                  rme_ptr_t Value)
{
    struct RME_Cap_Thd* Thd_Op;
    volatile struct RME_Thd_Struct* Thd_Struct;
    volatile struct RME_Inv_Struct* Inv_Struct;
    volatile struct RME_CPU_Local* Local;
    rme_ptr_t Type_Stat;
    rme_ptr_t Layer_Cnt;
    
    /* Get the capability slot */
    RME_CPT_GETCAP(Cpt, Cap_Thd, RME_CAP_TYPE_THD, struct RME_Cap_Thd*, Thd_Op, Type_Stat);
    
    /* See if the target thread is already bound. If no or bound to other cores, we just quit */
    Local=RME_CPU_LOCAL();
    Thd_Struct=(struct RME_Thd_Struct*)Thd_Op->Head.Object;
    if(Thd_Struct->Sched.Local!=Local)
        return RME_ERR_PTH_INVSTATE;
    
    /* Find whatever position we require - Layer 0 is the first layer (stack top), and so on */
    Layer_Cnt=RME_PARAM_D1(Operation);
    Inv_Struct=(volatile struct RME_Inv_Struct*)(Thd_Struct->Ctx.Invstk.Next);
    while(1U)
    {
        if(Inv_Struct==(volatile struct RME_Inv_Struct*)&(Thd_Struct->Ctx.Invstk))
            return RME_ERR_KFN_FAIL;
        
        if(Layer_Cnt==0U)
            break;
        
        Layer_Cnt--;
        Inv_Struct=(volatile struct RME_Inv_Struct*)(Inv_Struct->Head.Next);
    }

    /* D0 position is the operation */
    switch(RME_PARAM_D0(Operation))
    {
        /* Register read/write */
        case RME_A6M_KFN_DEBUG_INV_MOD_SP_GET:      {Reg->R6=Inv_Struct->Ret.SP;break;}
        case RME_A6M_KFN_DEBUG_INV_MOD_SP_SET:      {Inv_Struct->Ret.SP=Value;break;}
        default:                                    {return RME_ERR_KFN_FAIL;}
    }

    return 0;
}
/* End Function:__RME_A6M_Debug_Inv_Mod **************************************/

/* Function:__RME_A6M_Debug_Exc_Get *******************************************
Description : Debug exception register extraction implementation for ARMv6-M.
Input       : struct RME_Cap_Cpt* Cpt - The current capability table.
              struct RME_Reg_Struct* Reg - The current register set.
              rme_cid_t Cap_Thd - The capability to the thread to consult.
              rme_ptr_t Operation - The operation, e.g. which register to read.
Output      : struct RME_Reg_Struct* Reg - The register set when exiting the
                                           handler.
Return      : rme_ret_t - If successful, 0; if a negative value, failed.
******************************************************************************/
rme_ret_t __RME_A6M_Debug_Exc_Get(struct RME_Cap_Cpt* Cpt,
                                  struct RME_Reg_Struct* Reg, 
                                  rme_cid_t Cap_Thd,
                                  rme_ptr_t Operation)
{
    struct RME_Cap_Thd* Thd_Op;
    struct RME_Thd_Struct* Thd_Struct;
    volatile struct RME_CPU_Local* Local;
    rme_ptr_t Type_Stat;
    
    /* Get the capability slot */
    RME_CPT_GETCAP(Cpt, Cap_Thd, RME_CAP_TYPE_THD, struct RME_Cap_Thd*, Thd_Op, Type_Stat);
    
    /* See if the target thread is already bound. If no or bound to other cores, we just quit */
    Local=RME_CPU_LOCAL();
    Thd_Struct=(struct RME_Thd_Struct*)Thd_Op->Head.Object;
    if(Thd_Struct->Sched.Local!=Local)
        return RME_ERR_PTH_INVSTATE;
    
    switch(Operation)
    {
        /* Register read */
        case RME_A6M_KFN_DEBUG_EXC_CAUSE_GET:       {Reg->R6=Thd_Struct->Ctx.Reg->Exc.Cause;break;}
        default:                                    {return RME_ERR_KFN_FAIL;}
    }

    return 0U;
}
/* End Function:__RME_A6M_Debug_Exc_Get **************************************/

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
#if(RME_RVM_GEN_ENABLE==1U)
EXTERN rme_ret_t RME_Hook_Kfn_Handler(rme_ptr_t Func_ID,
                                      rme_ptr_t Sub_ID,
                                      rme_ptr_t Param1,
                                      rme_ptr_t Param2);
#endif
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
#if(RME_PGT_RAW_USER==0U)
        case RME_KFN_PGT_ENTRY_MOD:
        {
            Retval=__RME_A6M_Pgt_Entry_Mod(Cpt,
                                           (rme_cid_t)Sub_ID,
                                           Param1,
                                           Param2);
            break;
        }
#endif
/* Interrupt controller operations *******************************************/
        case RME_KFN_INT_LOCAL_MOD:
        {
            Retval=__RME_A6M_Int_Local_Mod(Sub_ID,
                                           Param1,
                                           Param2);
            break;
        }
        case RME_KFN_INT_GLOBAL_MOD:    {return RME_ERR_KFN_FAIL;}
        case RME_KFN_INT_LOCAL_TRIG:
        {
            Retval=__RME_A6M_Int_Local_Trig(Sub_ID,     /* No ctxsw, int ctxsw later */
                                            Param1);
            break;
        }
        case RME_KFN_EVT_LOCAL_TRIG:
        {
            return __RME_A6M_Evt_Local_Trig(Reg,        /* May ctxsw */
                                            Sub_ID,
                                            Param1);
        }
/* Cache operations **********************************************************/
        case RME_KFN_CACHE_MOD:         {return RME_ERR_KFN_FAIL;}
        case RME_KFN_CACHE_CONFIG:      {return RME_ERR_KFN_FAIL;}
        case RME_KFN_CACHE_MAINT:       {return RME_ERR_KFN_FAIL;}
        case RME_KFN_CACHE_LOCK:        {return RME_ERR_KFN_FAIL;}
        case RME_KFN_PRFTH_MOD:
        {
            Retval=__RME_A6M_Prfth_Mod(Sub_ID,
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
            __RME_A6M_Wait_Int();
            Retval=0;
            break;
        }
        case RME_KFN_SYS_REBOOT:
        {
            __RME_A6M_Reboot();
            while(1);
        }
        case RME_KFN_SYS_SHDN:          {return RME_ERR_KFN_FAIL;}
        case RME_KFN_VOLT_MOD:          {return RME_ERR_KFN_FAIL;}
        case RME_KFN_FREQ_MOD:          {return RME_ERR_KFN_FAIL;}
        case RME_KFN_PMOD_MOD:          {return RME_ERR_KFN_FAIL;}
        case RME_KFN_SAFETY_MOD:        {return RME_ERR_KFN_FAIL;}
/* Performance monitoring operations *****************************************/
        case RME_KFN_PERF_CPU_FUNC:
        {
            Retval=__RME_A6M_Perf_CPU_Func(Reg,         /* Value in R6 */
                                           Sub_ID);
            break;
        }
        case RME_KFN_PERF_MON_MOD:
        {
            Retval=__RME_A6M_Perf_Mon_Mod(Sub_ID,
                                          Param1,
                                          Param2);
            break;
        }
        case RME_KFN_PERF_CNT_MOD:      {return RME_ERR_KFN_FAIL;}
        case RME_KFN_PERF_CYCLE_MOD:
        {
            Retval=__RME_A6M_Perf_Cycle_Mod(Reg,        /* Value in R6 */
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
#if(RME_DEBUG_PRINT==1U)
        case RME_KFN_DEBUG_PRINT:
        {
            __RME_Putchar((rme_s8_t)Sub_ID);
            Retval=0;
            break;
        }
#endif
        case RME_KFN_DEBUG_REG_MOD:
        {
            Retval=__RME_A6M_Debug_Reg_Mod(Cpt,         /* Value in R6 */
                                           Reg,
                                           (rme_cid_t)Sub_ID,
                                           Param1,
                                           Param2);
            break;
        }
        case RME_KFN_DEBUG_INV_MOD:
        {
            Retval=__RME_A6M_Debug_Inv_Mod(Cpt,         /* Value in R6 */
                                           Reg,
                                           (rme_cid_t)Sub_ID,
                                           Param1,
                                           Param2);
            break;
        }
        case RME_KFN_DEBUG_EXC_GET:
        {
            Retval=__RME_A6M_Debug_Exc_Get(Cpt,         /* Value in R6 */
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
#if(RME_RVM_GEN_ENABLE==1U)
            Retval=RME_Hook_Kfn_Handler(Func_ID,
                                        Sub_ID,
                                        Param1,
                                        Param2);
#else
            return RME_ERR_KFN_FAIL;
#endif
        }
    }

    /* Only non-ctxsw kernel functions end up here, we need to set success
     * return value for them */
    if(Retval>=0)
        __RME_Svc_Retval_Set(Reg, Retval);
            
    return Retval;
}
/* End Function:__RME_Kfn_Handler ********************************************/

/* Function:__RME_A6M_Lowlvl_Preinit ******************************************
Description : Initialize the low-level hardware, before the loading of the kernel
              even takes place.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_A6M_Lowlvl_Preinit(void)
{
    RME_A6M_LOWLVL_PREINIT();
    
#if(RME_RVM_GEN_ENABLE==1U)
    RME_Boot_Pre_Init();
#endif
}
/* End Function:__RME_A6M_Lowlvl_Preinit *************************************/

/* Function:__RME_A6M_NVIC_Set_Exc_Prio ***************************************
Description : Set the system exception priority in ARMv6-M architecture.
Input       : rme_cnt_t Exc - The exception number, always nagative.
              rme_ptr_t Prio - The priority to directly write into the priority
                               register.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_A6M_NVIC_Set_Exc_Prio(rme_cnt_t Exc,
                                 rme_ptr_t Prio)
{
    RME_ASSERT(Exc<0);
    
    /* Consider 2's complement here, and SHPR1 logically starts from handler #4 */
    RME_A6M_SCB_SHPR_SET((((rme_ptr_t)Exc)&0x0FU)-4U, Prio);
}
/* End Function:__RME_A6M_NVIC_Set_Exc_Prio **********************************/

/* Function:__RME_Lowlvl_Init *************************************************
Description : Initialize the low-level hardware.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_Lowlvl_Init(void)
{
    RME_A6M_LOWLVL_INIT();
    
    /* Check the number of interrupt lines */
    RME_ASSERT(((RME_A6M_SCNSCB_ICTR+1U)<<5U)>=RME_RVM_PHYS_VCT_NUM);
    RME_ASSERT(RME_RVM_PHYS_VCT_NUM<=32U);

    /* Enable the MPU */
    RME_ASSERT(RME_A6M_REGION_NUM<=16U);
    RME_A6M_MPU_CTRL&=~RME_A6M_MPU_CTRL_ENABLE;
    RME_A6M_MPU_CTRL=RME_A6M_MPU_CTRL_PRIVDEF|RME_A6M_MPU_CTRL_ENABLE;
    
    /* Set priority grouping - not supported on ARMV6-M */
    
    /* Set the priority of timer and faults to the lowest, SVC is slightly higher.
     * If not, ARMv6-M may pend the asynchronous SVC, causing the following execution:
     * 1> SVC is triggered but not yet in execution.
     * 2> SysTick or some other random vector comes in.
     * 3> Processor choose to respond to SysTick first.
     * 4> There is a context switch in SysTick, we switch to another thread.
     * 5> SVC is now processed on the new thread rather than the calling thread.
     * This MUST be avoided, thus we place SVC at a higher priority.
     * All vectors that have a higher priority than SVC shall not call any RME
     * kernel functions; for those who can call kernel functions, they must be
     * placed at a priority smaller than 0x80 and the user needs to guarantee
     * that they never preempt each other. To make things 100% work, place all
     * those vectors that may send from kernel to priority 0xFF. */
    __RME_A6M_NVIC_Set_Exc_Prio(RME_A6M_IRQN_SVCALL, 0x80U);
    __RME_A6M_NVIC_Set_Exc_Prio(RME_A6M_IRQN_PENDSV, 0xFFU);
    __RME_A6M_NVIC_Set_Exc_Prio(RME_A6M_IRQN_SYSTICK, 0xFFU);

    /* Initialize CPU-local data structures */
    _RME_CPU_Local_Init(&RME_A6M_Local, 0U);
    
    /* Configure and turn on the systick */
    RME_A6M_SYSTICK_LOAD=RME_A6M_SYSTICK_VAL-1U;
    RME_A6M_SYSTICK_VALREG=0U;
    RME_A6M_SYSTICK_CTRL=RME_A6M_SYSTICK_CTRL_CLKSOURCE|
                         RME_A6M_SYSTICK_CTRL_TICKINT|
                         RME_A6M_SYSTICK_CTRL_ENABLE;
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
#if(RME_PGT_RAW_USER!=0U)
    /* Initial array for raw page table mode - generic for all ARMv6-M */
    static const rme_ptr_t RME_A6M_Raw_Pgt_Def[32U]=
    {
        0x00000010U,0x0303003FU,
        0x00000011U,0x00000000U,
        0x00000012U,0x00000000U,
        0x00000013U,0x00000000U,
        0x00000014U,0x00000000U,
        0x00000015U,0x00000000U,
        0x00000016U,0x00000000U,
        0x00000017U,0x00000000U,
        0x00000018U,0x00000000U,
        0x00000019U,0x00000000U,
        0x0000001AU,0x00000000U,
        0x0000001BU,0x00000000U,
        0x0000001CU,0x00000000U,
        0x0000001DU,0x00000000U,
        0x0000001EU,0x00000000U,
        0x0000001FU,0x00000000U
    };
#endif
    
    Cur_Addr=RME_KOM_VA_BASE;
    
    /* Create the capability table for the init process */
    RME_ASSERT(_RME_Cpt_Boot_Init(RME_BOOT_INIT_CPT, Cur_Addr, RME_RVM_INIT_CPT_SIZE)==0);
    Cur_Addr+=RME_KOM_ROUND(RME_CPT_SIZE(RME_RVM_INIT_CPT_SIZE));

    /* Create the page table for the init process, and map in the page alloted for it */
#if(RME_PGT_RAW_USER==0U)
    /* The top-level page table - covers 4G address range */
    RME_ASSERT(_RME_Pgt_Boot_Crt(RME_A6M_CPT, RME_BOOT_INIT_CPT, RME_BOOT_INIT_PGT, 
               Cur_Addr, 0x00000000U, RME_PGT_TOP, RME_PGT_SIZE_4G, RME_PGT_NUM_1)==0);
    Cur_Addr+=RME_KOM_ROUND(RME_PGT_SIZE_TOP(RME_PGT_NUM_1));
    /* Other memory regions will be directly added, because we do not protect them in the init process */
    RME_ASSERT(_RME_Pgt_Boot_Add(RME_A6M_CPT, RME_BOOT_INIT_PGT, 0x00000000U, 0U, RME_PGT_ALL_PERM)==0);

    /* Activate the first process - This process cannot be deleted */
    RME_ASSERT(_RME_Prc_Boot_Crt(RME_A6M_CPT, RME_BOOT_INIT_CPT, RME_BOOT_INIT_PRC, 
                                 RME_BOOT_INIT_CPT, RME_BOOT_INIT_PGT)==0U);
#else
    RME_ASSERT(_RME_Prc_Boot_Crt(RME_A6M_CPT, RME_BOOT_INIT_CPT, RME_BOOT_INIT_PRC, 
                                 RME_BOOT_INIT_CPT, (rme_ptr_t)RME_A6M_Raw_Pgt_Def)==0U);
#endif

    /* Create the initial kernel function capability, and kernel memory capability */
    RME_ASSERT(_RME_Kfn_Boot_Crt(RME_A6M_CPT, RME_BOOT_INIT_CPT, RME_BOOT_INIT_KFN)==0);
    RME_ASSERT(_RME_Kom_Boot_Crt(RME_A6M_CPT, 
                                 RME_BOOT_INIT_CPT, 
                                 RME_BOOT_INIT_KOM,
                                 RME_KOM_VA_BASE,
                                 RME_KOM_VA_BASE+RME_KOM_VA_SIZE-1U,
                                 RME_KOM_FLAG_ALL)==0U);
    
    /* Create the initial kernel endpoint for timer ticks and interrupts */
    RME_A6M_Local.Sig_Tim=(struct RME_Cap_Sig*)&(RME_A6M_CPT[RME_BOOT_INIT_VCT]);
    RME_A6M_Local.Sig_Vct=(struct RME_Cap_Sig*)&(RME_A6M_CPT[RME_BOOT_INIT_VCT]);
    RME_ASSERT(_RME_Sig_Boot_Crt(RME_A6M_CPT, RME_BOOT_INIT_CPT, RME_BOOT_INIT_VCT)==0);
    
    /* Clean up the region for vectors and events */
    _RME_Clear((void*)RME_RVM_PHYS_VCTF_BASE, RME_RVM_PHYS_VCTF_SIZE);
    _RME_Clear((void*)RME_RVM_VIRT_EVTF_BASE, RME_RVM_VIRT_EVTF_SIZE);
    
    /* Activate the first thread, and set its priority */
    RME_ASSERT(_RME_Thd_Boot_Crt(RME_A6M_CPT, RME_BOOT_INIT_CPT, RME_BOOT_INIT_THD,
                                 RME_BOOT_INIT_PRC, Cur_Addr, 0U, &RME_A6M_Local)==0);
    Cur_Addr+=RME_KOM_ROUND(RME_THD_SIZE(0U));
    
    /* Print the size of some kernel objects, only used in debugging
    Size=RME_CPT_SIZE(1U);
    Size=RME_PGT_SIZE_TOP(0U)-sizeof(rme_ptr_t);
    Size=RME_PGT_SIZE_NOM(0U)-sizeof(rme_ptr_t);
    Size=RME_INV_SIZE;
    Size=RME_HYP_SIZE;
    Size=RME_REG_SIZE(0U);
    Size=RME_THD_SIZE(0U); */
    
    /* If generator is enabled for this project, generate what is required by the generator */
#if(RME_RVM_GEN_ENABLE==1U)
    Cur_Addr=RME_Boot_Vct_Init(RME_A6M_CPT, RME_BOOT_INIT_VCT+1U, Cur_Addr);
#endif

    /* Before we go into user level, make sure that the kernel object allocation is within the limits */
#if(RME_RVM_GEN_ENABLE==1U)
    RME_ASSERT(Cur_Addr==(RME_KOM_VA_BASE+RME_RVM_KOM_BOOT_FRONT));
#else
    RME_ASSERT(Cur_Addr<(RME_KOM_VA_BASE+RME_RVM_KOM_BOOT_FRONT));
#endif

#if(RME_RVM_GEN_ENABLE==1U)
    /* Perform post initialization */
    RME_Boot_Post_Init();
#endif

    /* Enable the MPU & interrupt */
#if(RME_PGT_RAW_USER==0U)
    RME_ASSERT(RME_CAP_IS_ROOT(RME_A6M_Local.Thd_Cur->Sched.Prc->Pgt)!=0U);
#endif
    __RME_Pgt_Set(RME_A6M_Local.Thd_Cur->Sched.Prc->Pgt);
    __RME_Int_Enable();
    
    /* Boot into the init thread */
    __RME_User_Enter(RME_A6M_INIT_ENTRY, RME_A6M_INIT_STACK, 0U);
    
    /* Never reaches here */
    while(1U);
}
/* End Function:__RME_Boot ***************************************************/

/* Function:__RME_A6M_Reboot **************************************************
Description : Reboot the MCU, including all its peripherals.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_A6M_Reboot(void)
{
#if(RME_RVM_GEN_ENABLE==1U)
    RME_Reboot_Failsafe();
#endif

    /* Reboots the platform, which includes all peripherals. This is standard across all ARMv6-M */
    __RME_A6M_Reset();
}
/* End Function:__RME_A6M_Reboot *********************************************/

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
    *Svc=(Reg->R4)>>16;
    *Cid=(Reg->R4)&0xFFFFU;
    Param[0U]=Reg->R5;
    Param[1U]=Reg->R6;
    Param[2U]=Reg->R7;
}
/* End Function:__RME_Svc_Param_Get ******************************************/

/* Function:__RME_Svc_Retval_Set **********************************************
Description : Set the system call return value to the stack frame.
Input       : rme_ret_t Retval - The return value.
Output      : struct RME_Reg_Struct* Reg - The register set.
Return      : None.
******************************************************************************/
void __RME_Svc_Retval_Set(struct RME_Reg_Struct* Reg,
                          rme_ret_t Retval)
{
    Reg->R4=(rme_ptr_t)Retval;
}
/* End Function:__RME_Svc_Retval_Set *****************************************/

/* Function:__RME_Thd_Reg_Init ************************************************
Description : Initialize the register set for the thread.
Input       : rme_ptr_t Attr - Always RME_A6M_ATTR_NONE.
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
    RME_ASSERT(Attr==RME_A6M_ATTR_NONE);
    
    /* Set the LR to a valid value */
    Reg->LR=RME_A6M_EXC_RET_INIT;
    /* The entry point needs to have the last bit set to avoid ARM mode */
    Reg->R4=Entry|0x01U;
    /* Put something in the SP later */
    Reg->SP=Stack;
    /* Set the parameter */
    Reg->R5=Param;
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
    Dst->SP=Src->SP;
    Dst->R4=Src->R4;
    Dst->R5=Src->R5;
    Dst->R6=Src->R6;
    Dst->R7=Src->R7;
    Dst->R8=Src->R8;
    Dst->R9=Src->R9;
    Dst->R10=Src->R10;
    Dst->R11=Src->R11;
    Dst->LR=Src->LR;
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
    /* LR does not need to be preserved because ARMv6-M have no
     *  coprocessors, and LR have only one valid value. */
    Ret->SP=Reg->SP;
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
    Reg->SP=Ret->SP;
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
    Reg->R5=(rme_ptr_t)Retval;
}
/* End Function:__RME_Inv_Retval_Set *****************************************/

/* Function:__RME_Pgt_Kom_Init ************************************************
Description : Initialize the kernel mapping tables, so it can be added to all the
              top-level page tables. In ARMv6-M, we do not need to add such pages.
Input       : None.
Output      : None.
Return      : rme_ptr_t - 0, always successful.
******************************************************************************/
rme_ret_t __RME_Pgt_Kom_Init(void)
{
    /* Empty function, always immediately successful */
    return 0;
}
/* End Function:__RME_Pgt_Kom_Init *******************************************/

/* Function:__RME_Pgt_Init ****************************************************
Description : Initialize the page table data structure, according to the capability.
Input       : struct RME_Cap_Pgt* Pgt_Op - The page table to operate on.
Output      : None.
Return      : rme_ret_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
#if(RME_PGT_RAW_USER==0U)
rme_ret_t __RME_Pgt_Init(struct RME_Cap_Pgt* Pgt_Op)
{
    rme_ptr_t Count;
    rme_ptr_t* Ptr;
    
    /* Get the actual table */
    Ptr=RME_CAP_GETOBJ(Pgt_Op,rme_ptr_t*);
    
    /* Initialize the causal metadata */
    ((struct __RME_A6M_Pgt_Meta*)Ptr)->Base=Pgt_Op->Base;
    ((struct __RME_A6M_Pgt_Meta*)Ptr)->Toplevel=0U;
    ((struct __RME_A6M_Pgt_Meta*)Ptr)->Order=Pgt_Op->Order;
    Ptr+=sizeof(struct __RME_A6M_Pgt_Meta)/sizeof(rme_ptr_t);
    
    /* Is this a top-level? If it is, we need to clean up the MPU data. In MMU
     * environments, if it is top-level, we need to add kernel pages as well */
    if(((Pgt_Op->Base)&RME_PGT_TOP)!=0U)
    {
        for(Count=0;Count<RME_A6M_REGION_NUM;Count++)
        {
            ((struct __RME_A6M_MPU_Data*)Ptr)->Raw.Data[Count].RBAR=RME_A6M_MPU_VALID|Count;
            ((struct __RME_A6M_MPU_Data*)Ptr)->Raw.Data[Count].RASR=0U;
        }
        
        Ptr+=sizeof(struct __RME_A6M_MPU_Data)/sizeof(rme_ptr_t);
    }
    
    /* Clean up the table itself - This is could be virtually unbounded if the user
     * pass in some very large length value */
    for(Count=0U;Count<RME_POW2(RME_PGT_NMORD(Pgt_Op->Order));Count++)
        Ptr[Count]=0U;
    
    return 0;
}
#endif
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
#if(RME_PGT_RAW_USER==0U)
rme_ret_t __RME_Pgt_Check(rme_ptr_t Base_Addr,
                          rme_ptr_t Is_Top, 
                          rme_ptr_t Size_Order,
                          rme_ptr_t Num_Order,
                          rme_ptr_t Vaddr)
{
    if(Num_Order>RME_PGT_NUM_256)
        return RME_ERR_HAL_FAIL;
    if(Size_Order<RME_PGT_SIZE_32B)
        return RME_ERR_HAL_FAIL;
    if(Size_Order>RME_PGT_SIZE_4G)
        return RME_ERR_HAL_FAIL;
    if((Vaddr&0x03U)!=0U)
        return RME_ERR_HAL_FAIL;
    
    return 0U;
}
#endif
/* End Function:__RME_Pgt_Check **********************************************/

/* Function:__RME_Pgt_Del_Check ***********************************************
Description : Check if the page table can be deleted.
Input       : struct RME_Cap_Pgt Pgt_Op* - The page table to operate on.
Output      : None.
Return      : rme_ret_t - If can be deleted, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
#if(RME_PGT_RAW_USER==0U)
rme_ret_t __RME_Pgt_Del_Check(struct RME_Cap_Pgt* Pgt_Op)
{
    /* We don't need to check the directory mapping status (whether we are 
     * parent or children) anymore because this is done in the kernel */
    return 0;
}
#endif
/* End Function:__RME_Pgt_Del_Check ******************************************/

/* Function:___RME_A6M_MPU_RASR_Gen *******************************************
Description : Generate the RASR metadata for this level of page table.
Input       : rme_ptr_t* Table - The table to generate data for. This 
                                          is directly the raw page table itself,
                                          without accounting for metadata.
              rme_ptr_t Flag - The flags for each entry.
              rme_ptr_t Size_Order - The size order of the page directory.
              rme_ptr_t Num_Order - The number order of the page directory.
Output      : struct __RME_A6M_MPU_Entry* Entry - The data generated.
Return      : rme_ptr_t - The RASR value returned.
******************************************************************************/
#if(RME_PGT_RAW_USER==0U)
rme_ptr_t ___RME_A6M_MPU_RASR_Gen(rme_ptr_t* Table,
                                  rme_ptr_t Flag, 
                                  rme_ptr_t Size_Order,
                                  rme_ptr_t Num_Order)
{
    rme_ptr_t RASR;
    rme_ptr_t Count;
    rme_ptr_t SRD_Flag;
    
    /* Get the SRD part first */
    RASR=0U;
    SRD_Flag=0U;
    
    switch(Num_Order)
    {
        case RME_PGT_NUM_1:SRD_Flag=0xFFU;break;
        case RME_PGT_NUM_2:SRD_Flag=0x0FU;break;
        case RME_PGT_NUM_4:SRD_Flag=0x03U;break;
        case RME_PGT_NUM_8:SRD_Flag=0x01U;break;
        default:RME_ASSERT(0U);
    }
    
    for(Count=0U;Count<RME_POW2(Num_Order);Count++)
    {
        if(((Table[Count]&RME_A6M_PGT_PRESENT)!=0U)&&((Table[Count]&RME_A6M_PGT_TERMINAL)!=0U))
            RASR|=SRD_Flag<<Count*RME_POW2(RME_PGT_NUM_8-Num_Order);
    }
    
    if(RASR==0U)
        return 0U;
    
    RASR<<=8;
    
    RASR=RME_A6M_MPU_SRDCLR&(~RASR);
    RASR|=RME_A6M_MPU_SZENABLE;
    /* Is it read-only? - we do not care if the read bit is set, because it is always readable anyway */
    if((Flag&RME_PGT_WRITE)!=0U)
        RASR|=RME_A6M_MPU_RW;
    else
        RASR|=RME_A6M_MPU_RO;
    /* Can we fetch instructions from there? */
    if((Flag&RME_PGT_EXECUTE)==0U)
        RASR|=RME_A6M_MPU_XN;
    /* Is the area cacheable? */
    if((Flag&RME_PGT_CACHE)!=0U)
        RASR|=RME_A6M_MPU_CACHE;
    /* Is the area bufferable? */
    if((Flag&RME_PGT_BUFFER)!=0U)
        RASR|=RME_A6M_MPU_BUFFER;
    /* What is the region size? */
    RASR|=RME_A6M_MPU_REGIONSIZE(Size_Order+Num_Order);
    
    return RASR;
}
#endif
/* End Function:___RME_A6M_MPU_RASR_Gen **************************************/

/* Function:___RME_A6M_MPU_Clear **********************************************
Description : Clear the MPU setting of this directory. If it exists, clear it;
              If it does not exist, don't do anything.
Input       : struct __RME_A6M_MPU_Data* Top_MPU - The top-level MPU metadata
              rme_ptr_t Base_Addr - The start mapping address of the directory.
              rme_ptr_t Size_Order - The size order of the page directory.
              rme_ptr_t Num_Order - The number order of the page directory.
Output      : None.
Return      : rme_ret_t - Always 0.
******************************************************************************/
#if(RME_PGT_RAW_USER==0U)
rme_ret_t ___RME_A6M_MPU_Clear(struct __RME_A6M_MPU_Data* Top_MPU, 
                               rme_ptr_t Base_Addr,
                               rme_ptr_t Size_Order,
                               rme_ptr_t Num_Order)
{
    rme_ptr_t Count;
    
    for(Count=0;Count<RME_A6M_REGION_NUM;Count++)
    {
        if((Top_MPU->Raw.Data[Count].RASR&RME_A6M_MPU_SZENABLE)!=0U)
        {
            /* We got one MPU region valid here */
            if((RME_A6M_MPU_ADDR(Top_MPU->Raw.Data[Count].RBAR)==Base_Addr)&&
               (RME_A6M_MPU_SZORD(Top_MPU->Raw.Data[Count].RASR)==(Size_Order+Num_Order)))
            {
                /* Clean it up and return */
                Top_MPU->Raw.Data[Count].RBAR=RME_A6M_MPU_VALID|Count;
                Top_MPU->Raw.Data[Count].RASR=0U;
                return 0;
            }
        }
    }
    
    return 0;
}
#endif
/* End Function:___RME_A6M_MPU_Clear *****************************************/

/* Function:___RME_A6M_MPU_Add ************************************************
Description : Add or update the MPU entry in the top-level MPU table.
              ARMv6-M have no dynamic pages at all; all pages are static by default.
Input       : struct __RME_A6M_MPU_Data* Top_MPU - The top-level MPU metadata.
              rme_ptr_t Base_Addr - The start mapping address of the directory.
              rme_ptr_t Size_Order - The size order of the page directory.
              rme_ptr_t Num_Order - The number order of the page directory.
              rme_ptr_t RASR - The RASR register content, if set.
Output      : None.
Return      : rme_ret_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
#if(RME_PGT_RAW_USER==0U)
rme_ret_t ___RME_A6M_MPU_Add(struct __RME_A6M_MPU_Data* Top_MPU, 
                             rme_ptr_t Base_Addr,
                             rme_ptr_t Size_Order,
                             rme_ptr_t Num_Order,
                             rme_ptr_t RASR)
{
    rme_u8_t Count;
    /* The number of empty slots available */
    rme_ptr_t Empty_Cnt;
    /* The empty slots */
    rme_u8_t Empty[RME_A6M_REGION_NUM];
    
    /* Set these values to some overrange value */
    Empty_Cnt=0U;
    for(Count=0U;Count<RME_A6M_REGION_NUM;Count++)
    {
        if((Top_MPU->Raw.Data[Count].RASR&RME_A6M_MPU_SZENABLE)!=0U)
        {
            /* We got one MPU region valid here */
            if((RME_A6M_MPU_ADDR(Top_MPU->Raw.Data[Count].RBAR)==Base_Addr)&&
               (RME_A6M_MPU_SZORD(Top_MPU->Raw.Data[Count].RASR)==(Size_Order+Num_Order)))
            {
                /* Update the RASR - all flag changes are reflected here */
                Top_MPU->Raw.Data[Count].RASR=RASR;
                return 0U;
            }
        }
        else
        {
            Empty[Empty_Cnt]=Count;
            Empty_Cnt++;
        }
    }
    
    /* Update unsuccessful, we didn't find any match. We will need a new slot. */
    if(Empty_Cnt==0)
        return RME_ERR_HAL_FAIL;
    
    /* We may map in using an empty slot */
    Count=Empty[0];
    
    /* Put the data to this slot */
    Top_MPU->Raw.Data[Count].RBAR=RME_A6M_MPU_ADDR(Base_Addr)|RME_A6M_MPU_VALID|Count;
    Top_MPU->Raw.Data[Count].RASR=RASR;

    return 0;
}
#endif
/* End Function:___RME_A6M_MPU_Add *******************************************/

/* Function:___RME_A6M_MPU_Update *********************************************
Description : Update the top-level MPU metadata for this level of page table.
Input       : struct __RME_A6M_Pgt_Meta* Meta - This page table.
              rme_ptr_t Op_Flag - The operation flag. 1 for add, 0 for clean.
Output      : None.
Return      : rme_ret_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
#if(RME_PGT_RAW_USER==0U)
rme_ret_t ___RME_A6M_MPU_Update(struct __RME_A6M_Pgt_Meta* Meta,
                                rme_ptr_t Op_Flag)
{
    rme_ptr_t RASR;
    rme_ptr_t* Table;
    struct __RME_A6M_MPU_Data* Top_MPU;
    
    /* Is it possible for MPU to represent this? */
    if(RME_PGT_NMORD(Meta->Order)>RME_PGT_NUM_8)
        return RME_ERR_HAL_FAIL;
    
    /* Get the tables */
    if(Meta->Toplevel!=0U)
    {
        /* We have a top-level */
        Top_MPU=(struct __RME_A6M_MPU_Data*)(Meta->Toplevel+sizeof(struct __RME_A6M_Pgt_Meta));
        Table=RME_A6M_PGT_TBL_NOM(Meta);
    }
    else if(((Meta->Base)&RME_PGT_TOP)!=0U)
    {
        /* We don't have a top-level, but we are the top-level */
        Top_MPU=(struct __RME_A6M_MPU_Data*)(((rme_ptr_t)Meta)+sizeof(struct __RME_A6M_Pgt_Meta));
        Table=RME_A6M_PGT_TBL_TOP(Meta);
    }
    else
        return RME_ERR_HAL_FAIL;
    
    if(Op_Flag==RME_A6M_MPU_CLR)
    {
        /* Clear the metadata - this function will never fail */
        ___RME_A6M_MPU_Clear(Top_MPU,
                             RME_PGT_START(Meta->Base),
                             RME_PGT_SZORD(Meta->Order),
                             RME_PGT_NMORD(Meta->Order));
    }
    else
    {
        /* See if the RASR contains anything */
        RASR=___RME_A6M_MPU_RASR_Gen(Table, Meta->Page_Flag, 
                                     RME_PGT_SZORD(Meta->Order),
                                     RME_PGT_NMORD(Meta->Order));
        if(RASR==0U)
        {
            /* All pages are unmapped. Clear this from the MPU data */
            ___RME_A6M_MPU_Clear(Top_MPU,
                                 RME_PGT_START(Meta->Base),
                                 RME_PGT_SZORD(Meta->Order),
                                 RME_PGT_NMORD(Meta->Order));
        }
        else
        {
            /* At least one of the pages are there. Map it */
            if(___RME_A6M_MPU_Add(Top_MPU,
                                  RME_PGT_START(Meta->Base),
                                  RME_PGT_SZORD(Meta->Order),
                                  RME_PGT_NMORD(Meta->Order),
                                  RASR)!=0U)
                return RME_ERR_HAL_FAIL;
        }
    }
    
    return 0;
}
#endif
/* End Function:___RME_A6M_MPU_Update ****************************************/

/* Function:___RME_A6M_Pgt_Have_Page ******************************************
Description : See if there are pages mapped in a directory.
Input       : rme_ptr_t* Table - The table to detect.
              rme_ptr_t Num_Order - The number order.
Output      : None.
Return      : rme_ptr_t - If there are no pages mapped in, 0; else 1.
******************************************************************************/
#if(RME_PGT_RAW_USER==0U)
rme_ptr_t ___RME_A6M_Pgt_Have_Page(rme_ptr_t* Table,
                                   rme_ptr_t Num_Order)
{
    rme_ptr_t Count;
    
    for(Count=0U;Count<RME_POW2(Num_Order);Count++)
    {
        if(((Table[Count]&RME_A6M_PGT_PRESENT)!=0U)&&
           ((Table[Count]&RME_A6M_PGT_TERMINAL)!=0U))
            return 1U;
    }
    
    return 0U;
}
#endif
/* End Function:___RME_A6M_Pgt_Have_Page *************************************/

/* Function:___RME_A6M_Pgt_Have_Pgdir *****************************************
Description : See if there are page directories mapped mapped in a directory.
Input       : rme_ptr_t* Table - The table to detect.
              rme_ptr_t Num_Order - The number order.
Output      : None.
Return      : rme_ptr_t - If there are no pages mapped in, 0; else 1.
******************************************************************************/
#if(RME_PGT_RAW_USER==0U)
rme_ptr_t ___RME_A6M_Pgt_Have_Pgdir(rme_ptr_t* Table,
                                    rme_ptr_t Num_Order)
{
    rme_ptr_t Count;
    
    for(Count=0U;Count<RME_POW2(Num_Order);Count++)
    {
        if(((Table[Count]&RME_A6M_PGT_PRESENT)!=0U)&&
           ((Table[Count]&RME_A6M_PGT_TERMINAL)==0U))
            return 1U;
    }
    
    return 0;
}
#endif
/* End Function:___RME_A6M_Pgt_Have_Pgdir ************************************/

/* Function:__RME_Pgt_Set *****************************************************
Description : Set the processor's page table.
Input       : struct RME_Cap_Pgt* Pgt - The capability to the root page table.
              rme_ptr_t Pgt - The alternative raw page table.
Output      : None.
Return      : None.
******************************************************************************/
#if(RME_PGT_RAW_USER==0U)
void __RME_Pgt_Set(struct RME_Cap_Pgt* Pgt)
#else
void __RME_Pgt_Set(rme_ptr_t Pgt)
#endif
{
    struct __RME_A6M_Raw_Pgt* Raw_Pgt;
    
#if(RME_PGT_RAW_USER==0U)
    struct __RME_A6M_MPU_Data* MPU_Data;
    
    MPU_Data=(struct __RME_A6M_MPU_Data*)(RME_CAP_GETOBJ(Pgt, rme_ptr_t)+
                                          sizeof(struct __RME_A6M_Pgt_Meta));
    Raw_Pgt=&(MPU_Data->Raw);
#else
    Raw_Pgt=(struct __RME_A6M_Raw_Pgt*)Pgt;
#endif

    /* Get the physical address of the page table - here we do not need any 
     * conversion, because VA = PA as always. We just need to extract the MPU
     * metadata part and pass it down */
#if(RME_A6M_REGION_NUM==1U)
    ___RME_A6M_MPU_Set1(Raw_Pgt);
#elif(RME_A6M_REGION_NUM==2U)
    ___RME_A6M_MPU_Set2(Raw_Pgt);
#elif(RME_A6M_REGION_NUM==3U)
    ___RME_A6M_MPU_Set3(Raw_Pgt);
#elif(RME_A6M_REGION_NUM==4U)
    ___RME_A6M_MPU_Set4(Raw_Pgt);
#elif(RME_A6M_REGION_NUM==5U)
    ___RME_A6M_MPU_Set5(Raw_Pgt);
#elif(RME_A6M_REGION_NUM==6U)
    ___RME_A6M_MPU_Set6(Raw_Pgt);
#elif(RME_A6M_REGION_NUM==7U)
    ___RME_A6M_MPU_Set7(Raw_Pgt);
#elif(RME_A6M_REGION_NUM==8U)
    ___RME_A6M_MPU_Set8(Raw_Pgt);
#elif(RME_A6M_REGION_NUM==9U)
    ___RME_A6M_MPU_Set9(Raw_Pgt);
#elif(RME_A6M_REGION_NUM==10U)
    ___RME_A6M_MPU_Set10(Raw_Pgt);
#elif(RME_A6M_REGION_NUM==11U)
    ___RME_A6M_MPU_Set11(Raw_Pgt);
#elif(RME_A6M_REGION_NUM==12U)
    ___RME_A6M_MPU_Set12(Raw_Pgt);
#elif(RME_A6M_REGION_NUM==13U)
    ___RME_A6M_MPU_Set13(Raw_Pgt);
#elif(RME_A6M_REGION_NUM==14U)
    ___RME_A6M_MPU_Set14(Raw_Pgt);
#elif(RME_A6M_REGION_NUM==15U)
    ___RME_A6M_MPU_Set15(Raw_Pgt);
#elif(RME_A6M_REGION_NUM==16U)
    ___RME_A6M_MPU_Set16(Raw_Pgt);
#endif
}
/* End Function:__RME_Pgt_Set ************************************************/

/* Function:___RME_A6M_Pgt_Refresh ********************************************
Description : Refresh the processor's page table content to the latest.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
#if(RME_PGT_RAW_USER==0U)
void ___RME_A6M_Pgt_Refresh(void)
{
    struct RME_Thd_Struct* Thd_Cur;
    
    /* Get current thread; if we're booting, do nothing */
    Thd_Cur=RME_A6M_Local.Thd_Cur;
    if(Thd_Cur==RME_NULL)
        return;
    
    __RME_Pgt_Set(_RME_Thd_Pgt(Thd_Cur));
}
#endif
/* End Function:___RME_A6M_Pgt_Refresh ***************************************/

/* Function:__RME_Pgt_Page_Map ************************************************
Description : Map a page into the page table. If a page is mapped into the slot, the
              flags is actually placed on the metadata place because all pages are
              required to have the same flags. We take advantage of this to increase
              the page granularity. This architecture requires that the mapping is
              always at least readable.
Input       : struct RME_Cap_Pgt* - The cap ability to the page table to operate on.
              rme_ptr_t Paddr - The physical address to map to.
              rme_ptr_t Pos - The position in the page table.
              rme_ptr_t Flag - The RME standard page attributes. Need to
                               translate them into architecture specific ones.
Output      : None.
Return      : rme_ret_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
#if(RME_PGT_RAW_USER==0U)
rme_ret_t __RME_Pgt_Page_Map(struct RME_Cap_Pgt* Pgt_Op,
                             rme_ptr_t Paddr,
                             rme_ptr_t Pos,
                             rme_ptr_t Flag)
{
    rme_ptr_t* Table;
    struct __RME_A6M_Pgt_Meta* Meta;

    /* It should at least be readable */
    if((Flag&RME_PGT_READ)==0U)
        return RME_ERR_HAL_FAIL;

    /* ARMv6-M refuses all dynamic mappings - we don't even have fault handlers */
    if((Flag&RME_PGT_READ)==0U)
        return RME_ERR_HAL_FAIL;

    /* We are doing page-based operations on this, so the page directory should
     * be MPU-representable. Only page numbers of 1, 2, 4 & 8 are representable
     * for ARMv6-M */
    if(RME_PGT_NMORD(Pgt_Op->Order)>RME_PGT_NUM_8)
        return RME_ERR_HAL_FAIL;
    
    /* Get the metadata */
    Meta=RME_CAP_GETOBJ(Pgt_Op, struct __RME_A6M_Pgt_Meta*);
    
    /* Where is the entry slot */
    if(((Pgt_Op->Base)&RME_PGT_TOP)!=0U)
        Table=RME_A6M_PGT_TBL_TOP(Meta);
    else
        Table=RME_A6M_PGT_TBL_NOM(Meta);
    
    /* Check if we are trying to make duplicate mappings into the same location */
    if((Table[Pos]&RME_A6M_PGT_PRESENT)!=0U)
        return RME_ERR_HAL_FAIL;

    /* Trying to map something. Check if the pages flags are consistent. MPU
     * subregions shall share the same flags in ARMv6-M */
    if(___RME_A6M_Pgt_Have_Page(Table, RME_PGT_NMORD(Pgt_Op->Order))==0U)
        Meta->Page_Flag=Flag;
    else
    {
        if(Meta->Page_Flag!=Flag)
            return RME_ERR_HAL_FAIL;
    }

    /* Register into the page table */
    Table[Pos]=RME_A6M_PGT_PRESENT|RME_A6M_PGT_TERMINAL|
               RME_ROUND_DOWN(Paddr,RME_PGT_SZORD(Pgt_Op->Order));
   
    /* If we are the top level or we have a top level, and we have static pages mapped in, do MPU updates */
    if((Meta->Toplevel!=0U)||(((Pgt_Op->Base)&RME_PGT_TOP)!=0U))
    {
        if((Flag&RME_PGT_STATIC)!=0U)
        {
            /* Mapping static pages, update the MPU representation */
            if(___RME_A6M_MPU_Update(Meta, RME_A6M_MPU_UPD)==RME_ERR_HAL_FAIL)
            {
                /* MPU update failed. Revert operations */
                Table[Pos]=0U;
                return RME_ERR_HAL_FAIL;
            }
            /* Update MPU in case we're manipulating the current page table */
            ___RME_A6M_Pgt_Refresh();
        }
    }
    
    return 0;
}
#endif
/* End Function:__RME_Pgt_Page_Map *******************************************/

/* Function:__RME_Pgt_Page_Unmap **********************************************
Description : Unmap a page from the page table.
Input       : struct RME_Cap_Pgt* - The capability to the page table to operate on.
              rme_ptr_t Pos - The position in the page table.
Output      : None.
Return      : rme_ret_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
#if(RME_PGT_RAW_USER==0U)
rme_ret_t __RME_Pgt_Page_Unmap(struct RME_Cap_Pgt* Pgt_Op,
                               rme_ptr_t Pos)
{
    rme_ptr_t* Table;
    rme_ptr_t Temp;
    struct __RME_A6M_Pgt_Meta* Meta;
        
    /* We are doing page-based operations on this, so the page directory should
     * be MPU-representable. Only page numbers of 1, 2, 4 & 8 are representable
     * for ARMv6-M */
    if(RME_PGT_NMORD(Pgt_Op->Order)>RME_PGT_NUM_8)
        return RME_ERR_HAL_FAIL;
    
    /* Get the metadata */
    Meta=RME_CAP_GETOBJ(Pgt_Op,struct __RME_A6M_Pgt_Meta*);
    
    /* Where is the entry slot */
    if(((Pgt_Op->Base)&RME_PGT_TOP)!=0U)
        Table=RME_A6M_PGT_TBL_TOP(Meta);
    else
        Table=RME_A6M_PGT_TBL_NOM(Meta);

    /* Check if we are trying to remove something that does not exist, or trying to
     * remove a page directory */
    if(((Table[Pos]&RME_A6M_PGT_PRESENT)==0)||((Table[Pos]&RME_A6M_PGT_TERMINAL)==0U))
        return RME_ERR_HAL_FAIL;

    Temp=Table[Pos];
    Table[Pos]=0U;
    /* If we are top-level or we have a top-level, do MPU updates */
    if((Meta->Toplevel!=0U)||(((Pgt_Op->Base)&RME_PGT_TOP)!=0U))
    {
        /* Now we are unmapping the pages - Immediately update MPU representations */
        if(___RME_A6M_MPU_Update(Meta, RME_A6M_MPU_UPD)==RME_ERR_HAL_FAIL)
        {
            /* Revert operations */
            Table[Pos]=Temp;
            return RME_ERR_HAL_FAIL;
        }
        /* Update MPU in case we're manipulating the current page table */
        ___RME_A6M_Pgt_Refresh();
    }
    
    return 0;
}
#endif
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
#if(RME_PGT_RAW_USER==0U)
rme_ret_t __RME_Pgt_Pgdir_Map(struct RME_Cap_Pgt* Pgt_Parent,
                              rme_ptr_t Pos, 
                              struct RME_Cap_Pgt* Pgt_Child,
                              rme_ptr_t Flag)
{
    rme_ptr_t* Parent_Table;
    rme_ptr_t* Child_Table;
    struct __RME_A6M_Pgt_Meta* Parent_Meta;
    struct __RME_A6M_Pgt_Meta* Child_Meta;
    
    /* Get the metadata */
    Parent_Meta=RME_CAP_GETOBJ(Pgt_Parent,struct __RME_A6M_Pgt_Meta*);
    Child_Meta=RME_CAP_GETOBJ(Pgt_Child,struct __RME_A6M_Pgt_Meta*);
    
    /* The parent table must have or be a top-directory */
    if((Parent_Meta->Toplevel==0U)&&(((Parent_Meta->Base)&RME_PGT_TOP)==0U))
        return RME_ERR_HAL_FAIL;
    
    /* The child must not have a top-level and itself is not a top-level.
     * We don't need to check whether the child already have childs, because
     * this is impossible: trying to map into a pgdir without a top-level, and
     * trying to unmap a pgdir that still have childs will both be rejected. */
    if((Child_Meta->Toplevel!=0U)||(((Child_Meta->Base)&RME_PGT_TOP)!=0U))
        return RME_ERR_HAL_FAIL;
    
    /* Where is the entry slot? */
    if(((Parent_Meta->Base)&RME_PGT_TOP)!=0U)
        Parent_Table=RME_A6M_PGT_TBL_TOP(Parent_Meta);
    else
        Parent_Table=RME_A6M_PGT_TBL_NOM(Parent_Meta);
    
    /* Check if anything already mapped in */
    if((Parent_Table[Pos]&RME_A6M_PGT_PRESENT)!=0U)
        return RME_ERR_HAL_FAIL;
    
    /* The address must be aligned to a word */
    Parent_Table[Pos]=RME_A6M_PGT_PRESENT|RME_A6M_PGT_PGD_ADDR((rme_ptr_t)Child_Meta);
    
    /* Log the entry into the destination - if the parent is a top-level, then the top-level
     * is the parent; if the parent have a top-level, then the top-level is the parent's top-level */
    if(Parent_Meta->Toplevel==0U)
        Child_Meta->Toplevel=(rme_ptr_t)Parent_Meta;
    else
        Child_Meta->Toplevel=Parent_Meta->Toplevel;
    
    /* Update MPU settings if there are static pages mapped into the source.
     * If there are any, update the MPU settings. */
    Child_Table=RME_A6M_PGT_TBL_NOM(Child_Meta);
    if((___RME_A6M_Pgt_Have_Page(Child_Table, RME_PGT_NMORD(Pgt_Child->Order))!=0U)&&
       (((Child_Meta->Page_Flag)&RME_PGT_STATIC)!=0U))
    {
        if(___RME_A6M_MPU_Update(Child_Meta, RME_A6M_MPU_UPD)==RME_ERR_HAL_FAIL)
        {
            /* Mapping failed. Revert operations */
            Parent_Table[Pos]=0U;
            Child_Meta->Toplevel=0U;
            return RME_ERR_HAL_FAIL;
        }
        /* Update MPU in case we're manipulating the current page table */
        ___RME_A6M_Pgt_Refresh();
    }

    return 0;
}
#endif
/* End Function:__RME_Pgt_Pgdir_Map ******************************************/

/* Function:__RME_Pgt_Pgdir_Unmap *********************************************
Description : Unmap a page directory from the page table.
Input       : struct RME_Cap_Pgt* Pgt_Parent - The parent page table to unmap from.
              rme_ptr_t Pos - The position in the page table.
              struct RME_Cap_Pgt* Pgt_Child - The child page table to unmap.
Output      : None.
Return      : rme_ret_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
#if(RME_PGT_RAW_USER==0U)
rme_ret_t __RME_Pgt_Pgdir_Unmap(struct RME_Cap_Pgt* Pgt_Parent,
                                rme_ptr_t Pos, 
                                struct RME_Cap_Pgt* Pgt_Child)
{
    rme_ptr_t* Table;
    struct __RME_A6M_Pgt_Meta* Parent_Meta;
    struct __RME_A6M_Pgt_Meta* Child_Meta;
    
    /* Get the metadata */
    Parent_Meta=RME_CAP_GETOBJ(Pgt_Parent,struct __RME_A6M_Pgt_Meta*);
    
    /* Where is the entry slot */
    if(((Pgt_Parent->Base)&RME_PGT_TOP)!=0U)
        Table=RME_A6M_PGT_TBL_TOP(Parent_Meta);
    else
        Table=RME_A6M_PGT_TBL_NOM(Parent_Meta);

    /* Check if we try to remove something nonexistent, or a page */
    if(((Table[Pos]&RME_A6M_PGT_PRESENT)==0U)||((Table[Pos]&RME_A6M_PGT_TERMINAL)!=0U))
        return RME_ERR_HAL_FAIL;
    
    /* See if the child page table is actually mapped there */
    Child_Meta=(struct __RME_A6M_Pgt_Meta*)RME_A6M_PGT_PGD_ADDR(Table[Pos]);
    if(Child_Meta!=RME_CAP_GETOBJ(Pgt_Child,struct __RME_A6M_Pgt_Meta*))
        return RME_ERR_HAL_FAIL;

    /* Check if the directory still have child directories */
    if(___RME_A6M_Pgt_Have_Page(Table, RME_PGT_NMORD(Pgt_Parent->Order))!=0U)
        return RME_ERR_HAL_FAIL;
    
    /* We are removing a page directory. Do MPU updates if any page mapped in */
    if(___RME_A6M_Pgt_Have_Pgdir(Table, RME_PGT_NMORD(Pgt_Parent->Order))!=0U)
    {
        if(___RME_A6M_MPU_Update(Parent_Meta, RME_A6M_MPU_CLR)==RME_ERR_HAL_FAIL)
            return RME_ERR_HAL_FAIL;
        /* Update MPU in case we're manipulating the current page table */
        ___RME_A6M_Pgt_Refresh();
    }

    Table[Pos]=0U;
    Parent_Meta->Toplevel=0U;

    return 0;
}
#endif
/* End Function:__RME_Pgt_Pgdir_Unmap ****************************************/

/* Function:__RME_Pgt_Lookup **************************************************
Description : Lookup a page entry in a page directory.
Input       : struct RME_Cap_Pgt* Pgt_Op - The page directory to lookup.
              rme_ptr_t Pos - The position to look up.
Output      : rme_ptr_t* Paddr - The physical address of the page.
              rme_ptr_t* Flag - The RME standard flags of the page.
Return      : rme_ret_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
#if(RME_PGT_RAW_USER==0U)
rme_ret_t __RME_Pgt_Lookup(struct RME_Cap_Pgt* Pgt_Op,
                           rme_ptr_t Pos,
                           rme_ptr_t* Paddr,
                           rme_ptr_t* Flag)
{
    rme_ptr_t* Table;
    
    /* Check if this is the top-level page table. Get the table */
    if(((Pgt_Op->Base)&RME_PGT_TOP)!=0U)
        Table=RME_A6M_PGT_TBL_TOP(RME_CAP_GETOBJ(Pgt_Op,rme_ptr_t*));
    else
        Table=RME_A6M_PGT_TBL_NOM(RME_CAP_GETOBJ(Pgt_Op,rme_ptr_t*));
    
    /* Start lookup */
    if(((Table[Pos]&RME_A6M_PGT_PRESENT)==0U)||
       ((Table[Pos]&RME_A6M_PGT_TERMINAL)==0U))
        return RME_ERR_HAL_FAIL;
    
    /* This is a page. Return the physical address and flags */
    if(Paddr!=0)
        *Paddr=RME_A6M_PGT_PTE_ADDR(Table[Pos]);
    
    if(Flag!=0)
        *Flag=RME_CAP_GETOBJ(Pgt_Op,struct __RME_A6M_Pgt_Meta*)->Page_Flag;

    return 0;
}
#endif
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
#if(RME_PGT_RAW_USER==0U)
rme_ret_t __RME_Pgt_Walk(struct RME_Cap_Pgt* Pgt_Op,
                         rme_ptr_t Vaddr,
                         rme_ptr_t* Pgt,
                         rme_ptr_t* Map_Vaddr,
                         rme_ptr_t* Paddr,
                         rme_ptr_t* Size_Order,
                         rme_ptr_t* Num_Order,
                         rme_ptr_t* Flag)
{
    struct __RME_A6M_Pgt_Meta* Meta;
    rme_ptr_t* Table;
    rme_ptr_t Pos;
    rme_ptr_t Shift;
    
    /* This must the top-level page table */
    RME_ASSERT(((Pgt_Op->Base)&RME_PGT_TOP)!=0U);
    
    /* Get the table and start lookup */
    Meta=RME_CAP_GETOBJ(Pgt_Op, struct __RME_A6M_Pgt_Meta*);
    Table=RME_A6M_PGT_TBL_TOP(Meta);
    
    /* Do lookup recursively */
    while(1)
    {
        /* Check if the virtual address is in our range */
        if(Vaddr<RME_PGT_START(Meta->Base))
            return RME_ERR_HAL_FAIL;
        /* Calculate entry position - shifting by RME_WORD_BITS or more is UB */
        Shift=RME_PGT_SZORD(Meta->Order);
        if(Shift>=RME_WORD_BITS)
            Pos=0U;
        else
            Pos=(Vaddr-RME_PGT_START(Meta->Base))>>Shift;
        /* See if the entry is overrange */
        if((Pos>>RME_PGT_NMORD(Meta->Order))!=0U)
            return RME_ERR_HAL_FAIL;
        /* Find the position of the entry - Is there a page, a directory, or nothing? */
        if((Table[Pos]&RME_A6M_PGT_PRESENT)==0U)
            return RME_ERR_HAL_FAIL;
        if((Table[Pos]&RME_A6M_PGT_TERMINAL)!=0U)
        {
            /* This is a page - we found it */
            if(Pgt!=RME_NULL)
                *Pgt=(rme_ptr_t)Meta;
            if(Map_Vaddr!=RME_NULL)
            {
                if(Shift>=RME_WORD_BITS)
                    *Map_Vaddr=RME_PGT_START(Meta->Base);
                else
                    *Map_Vaddr=RME_PGT_START(Meta->Base)+(Pos<<Shift);
            }
            if(Paddr!=RME_NULL)
            {
                if(Shift>=RME_WORD_BITS)
                    *Paddr=RME_PGT_START(Meta->Base);
                else
                    *Paddr=RME_PGT_START(Meta->Base)+(Pos<<Shift);
            }
            if(Size_Order!=RME_NULL)
                *Size_Order=RME_PGT_SZORD(Meta->Order);
            if(Num_Order!=RME_NULL)
                *Num_Order=RME_PGT_NMORD(Meta->Order);
            if(Flag!=RME_NULL)
                *Flag=Meta->Page_Flag;
            
            break;
        }
        else
        {
            /* This is a directory, we goto that directory to continue walking */
            Meta=(struct __RME_A6M_Pgt_Meta*)RME_A6M_PGT_PGD_ADDR(Table[Pos]);
            Table=RME_A6M_PGT_TBL_NOM(Meta);
        }
    }
    
    return 0;
}
#endif
/* End Function:__RME_Pgt_Walk ***********************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/

