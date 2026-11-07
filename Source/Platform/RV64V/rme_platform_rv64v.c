/******************************************************************************
Filename    : rme_platform_rv64v.c
Author      : yhy
Date        : 29/09/2026
Licence     : The Unlicense; see LICENSE for details.
Description : The RV64V hardware abstraction layer implementation.

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
#include "Platform/RV64V/rme_platform_rv64v.h"
#include "Kernel/rme_kernel.h"
#undef __HDR_DEF__

#define __HDR_STRUCT__
#include "Platform/RV64V/rme_platform_rv64v.h"
#include "Kernel/rme_kernel.h"
#undef __HDR_STRUCT__

/* Private include */
#include "Platform/RV64V/rme_platform_rv64v.h"

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
    RME_RV64V_PUTCHAR(Char);
    return 0U;
}
#endif
/* End Function:__RME_Putchar ************************************************/

/* Function:___RME_RV64V_Sbi_Console_Putchar **********************************
Description : Output one character via the OpenSBI DBCN extension. The byte is
              passed through unchanged; newline translation is the caller's
              responsibility.
Input       : rme_ptr_t Char - The character to output.
Output      : None.
Return      : rme_ret_t - SBI error code, RME_RV64V_SBI_SUCCESS on success.
******************************************************************************/
rme_ret_t ___RME_RV64V_Sbi_Console_Putchar(rme_ptr_t Char)
{
    return ___RME_RV64V_Sbi_Call(RME_RV64V_SBI_EXT_DBCN,
                                 RME_RV64V_SBI_DBCN_CONSOLE_WRITE_BYTE,
                                 Char,0U,0U,0U,0U,0U).Error;
}
/* End Function:___RME_RV64V_Sbi_Console_Putchar *****************************/

/* Function:___RME_RV64V_Sbi_Timer_Set ****************************************
Description : Set the next S-mode timer event via the OpenSBI TIME extension.
              The timer does not auto-reload, so it must be re-armed on every
              timer interrupt.
Input       : rme_ptr_t Next_Time - The absolute time (in time CSR ticks) of
                                    the next event.
Output      : None.
Return      : rme_ret_t - SBI error code, RME_RV64V_SBI_SUCCESS on success.
******************************************************************************/
rme_ret_t ___RME_RV64V_Sbi_Timer_Set(rme_ptr_t Next_Time)
{
    return ___RME_RV64V_Sbi_Call(RME_RV64V_SBI_EXT_TIME,
                                 RME_RV64V_SBI_TIME_SET_TIMER,
                                 Next_Time,0U,0U,0U,0U,0U).Error;
}
/* End Function:___RME_RV64V_Sbi_Timer_Set ***********************************/

/* Function:___RME_RV64V_Sbi_Reset ********************************************
Description : Reset the system via the OpenSBI SRST extension. A cold reboot is
              requested, so this function should never return.
Input       : None.
Output      : None.
Return      : rme_ret_t - SBI error code if the reset request fails.
******************************************************************************/
rme_ret_t ___RME_RV64V_Sbi_Reset(void)
{
    return ___RME_RV64V_Sbi_Call(RME_RV64V_SBI_EXT_SRST,
                                 RME_RV64V_SBI_SRST_SYSTEM_RESET,
                                 RME_RV64V_SBI_SRST_TYPE_COLD_REBOOT,
                                 0U,0U,0U,0U,0U).Error;
}
/* End Function:___RME_RV64V_Sbi_Reset ***************************************/

/* Function:__RME_RV64V_CPU_Local_Get ****************************************
Description : Get the CPU-local data structure of the running hart. The tp
              register points at this hart's RME_RV64V_CPU_Data record while
              the kernel is executing. This must only be called after
              __RME_Lowlvl_Init() has installed tp for this hart.
Input       : None.
Output      : None.
Return      : struct RME_CPU_Local* - The CPU-local data structure.
******************************************************************************/
struct RME_CPU_Local* __RME_RV64V_CPU_Local_Get(void)
{
    return ((struct RME_RV64V_CPU_Data*)__RME_RV64V_CPU_Data_Get_Raw())->Local;
}
/* End Function:__RME_RV64V_CPU_Local_Get ************************************/

/* Function:__RME_CPUID_Get ***************************************************
Description : Get the CPUID. This is to identify where we are executing.
              CPUID is the logical (dense) CPU index, not the hardware hart ID;
              the hardware ID is kept in RME_RV64V_CPU_Info[CPUID].Hart_ID.
              Logical CPU IDs are contiguous and the boot hart is always
              CPUID 0, so before tp is installed this returns 0.
Input       : None.
Output      : None.
Return      : rme_ptr_t - The CPUID.
******************************************************************************/
rme_ptr_t __RME_CPUID_Get(void)
{
    /* Before tp is installed the boot hart is the only running CPU, and it
     * is always logical CPU 0. */
    if(__RME_RV64V_CPU_Data_Get_Raw()==0U)
        return 0U;

    return RME_CPU_LOCAL()->CPUID;
}
/* End Function:__RME_CPUID_Get **********************************************/

/* Function:___RME_RV64V_CPU_Set **********************************************
Description : Install the current hart's CPU data pointer into tp/sscratch.
Input       : rme_ptr_t CPUID - The logical CPU index.
Output      : None.
Return      : None.
******************************************************************************/
void ___RME_RV64V_CPU_Set(rme_ptr_t CPUID)
{
    rme_ptr_t Data;

    Data=(rme_ptr_t)&(RME_RV64V_CPU_Data[CPUID]);
    __asm__ __volatile__("mv tp,%0\n\t"
                         "csrw sscratch,%0"
                         ::"r"(Data):"memory");
}
/* End Function:___RME_RV64V_CPU_Set *****************************************/

/* Function:___RME_RV64V_Sbi_Hart_Start ***************************************
Description : Start one hart through the SBI HSM extension. A hart that is
              already started is treated as success.
Input       : rme_ptr_t Hart_ID - The hardware hart ID.
              rme_ptr_t Start - The physical start address.
              rme_ptr_t Opaque - The value passed to the hart in a2.
Output      : None.
Return      : rme_ret_t - RME_RV64V_SBI_SUCCESS on success, or an SBI error.
******************************************************************************/
rme_ret_t ___RME_RV64V_Sbi_Hart_Start(rme_ptr_t Hart_ID,
                                      rme_ptr_t Start,
                                      rme_ptr_t Opaque)
{
    struct RME_RV64V_SBI_Ret Ret;

    Ret=___RME_RV64V_Sbi_Call(RME_RV64V_SBI_EXT_HSM,
                              RME_RV64V_SBI_HSM_HART_START,
                              Hart_ID,Start,Opaque,0U,0U,0U);

    if(Ret.Error==RME_RV64V_SBI_ERR_ALREADY_AVAILABLE)
        return RME_RV64V_SBI_SUCCESS;

    return Ret.Error;
}
/* End Function:___RME_RV64V_Sbi_Hart_Start **********************************/

/* Function:__RME_RV64V_Exc_Handler *******************************************
Description : The fault handler of RME.
Input       : struct RME_Reg_Struct* Reg - The register set.
              rme_ptr_t Scause - The supervisor cause register.
Output      : struct RME_Reg_Struct* Reg - The updated register set.
Return      : None.
******************************************************************************/
void __RME_RV64V_Exc_Handler(struct RME_Reg_Struct* Reg,
                             rme_ptr_t Scause)
{
    rme_ptr_t Stval;
#if(RME_PGT_RAW_ENABLE==0U)
    rme_ptr_t Paddr;
    rme_ptr_t Size_Order;
    rme_ptr_t Flag;
    struct RME_Cap_Prc* Prc;
    struct RME_Inv_Struct* Inv_Top;
#endif
    struct RME_Thd_Struct* Thd_Cur;
    struct RME_Exc_Struct* Exc;

    /* Is it a kernel-level fault? If yes, panic */
    RME_ASSERT((Reg->SSTATUS&RME_RV64V_SSTATUS_RET_KERNEL)==0U);

    /* Get the address of this faulty address, and what caused this fault */
    Thd_Cur=RME_CPU_LOCAL()->Thd_Cur;
    Exc=&Thd_Cur->Ctx.Reg->Exc;
    Stval=___RME_RV64V_STVAL_Get();

#if(RME_PGT_RAW_ENABLE==0U)
    /* What fault is it? */
    switch(Scause)
    {
        /* We cannot recover from the following errors, have to kill the thread */
        default:
        case RME_RV64V_SCAUSE_IMALIGN:
        case RME_RV64V_SCAUSE_IUNDEF:
        case RME_RV64V_SCAUSE_IBRKPT:
        case RME_RV64V_SCAUSE_LALIGN:
        case RME_RV64V_SCAUSE_SALIGN:
        case RME_RV64V_SCAUSE_U_ECALL:
        case RME_RV64V_SCAUSE_S_ECALL:
        case RME_RV64V_SCAUSE_H_ECALL:
        case RME_RV64V_SCAUSE_M_ECALL:
        /* Sv39 access faults are genuine hardware violations - the page
         * tables are walked by the hardware itself, so there is no
         * software cache left to lazily populate, and they are not
         * recoverable */
        case RME_RV64V_SCAUSE_IACCFLT:
        case RME_RV64V_SCAUSE_LACCFLT:
        case RME_RV64V_SCAUSE_SACCFLT:
        {
            Exc->Cause=Scause;
            Exc->Addr=Reg->PC;
            Exc->Value=Stval;
            _RME_Thd_Fatal(Reg);
            return;
        }
        /* Page faults - the page may have just been mapped and the TLB still
         * holds a stale entry. Try to recover by walking the page table. */
        case RME_RV64V_SCAUSE_IPGFLT:
        case RME_RV64V_SCAUSE_LPGFLT:
        case RME_RV64V_SCAUSE_SPGFLT: break;
    }

    Inv_Top=RME_INVSTK_TOP(Thd_Cur);
    if(Inv_Top==RME_NULL)
        Prc=Thd_Cur->Sched.Prc;
    else
        Prc=Inv_Top->Prc;

    /* The page is not found, kill the thread */
    if(__RME_Pgt_Walk(Prc->Pgt,Stval,0U,0U,&Paddr,&Size_Order,0U,&Flag)!=0)
    {
        Exc->Cause=Scause;
        Exc->Addr=Reg->PC;
        Exc->Value=Stval;
        _RME_Thd_Fatal(Reg);
        return;
    }

    /* The page is mapped. Under Sv39 the hardware walks the table itself, so
     * a fault on an existing page means a stale TLB entry - flush and retry
     * the faulting instruction. */
    ___RME_RV64V_TLB_Flush();
#else
    Exc->Cause=Scause;
    Exc->Addr=Reg->PC;
    Exc->Value=Stval;
    _RME_Thd_Fatal(Reg);
#endif
}
/* End Function:__RME_RV64V_Exc_Handler **************************************/

/* Function:_RME_RV64V_Handler ************************************************
Description : The generic interrupt handler of RME for RV64V, in C.
Input       : struct RME_Reg_Struct* Reg - The register set.
Output      : struct RME_Reg_Struct* Reg - The updated register set.
Return      : None.
******************************************************************************/
void _RME_RV64V_Handler(struct RME_Reg_Struct* Reg)
{
    rme_ptr_t Scause;
    struct RME_CPU_Local* Local;

    Scause=___RME_RV64V_SCAUSE_Get();
    Local=RME_CPU_LOCAL();

    /* Timer handler */
    if(Scause==RME_RV64V_SCAUSE_S_TIM)
    {
        /* Re-arm this hart's next SBI timer event - it does not auto-reload.
         * Every hart owns its timer, so no cross-core IPI is needed. */
        ___RME_RV64V_Sbi_Timer_Set(___RME_RV64V_Time_Get()+RME_RV64V_OSTIM_VAL);

        _RME_Tim_Handler(Reg,1U);
    }
    /* External/software interrupt handler */
    else if((Scause&RME_RV64V_SCAUSE_INT)!=0U)
    {

        /* Send to the kernel endpoint */
        _RME_Kern_Snd(Local->Sig_Vct,1U);
        /* Pick the highest priority thread after we did all sends */
        _RME_Kern_High(Reg,Local);
    }
    /* System call handler */
    else if(Scause==RME_RV64V_SCAUSE_U_ECALL)
    {
        /* Need to skip the ECALL - the ECALL instruction is always 4 bytes
         * long (it is never a compressed instruction), NOT the 8-byte data
         * word size */
        Reg->PC+=4U;
        _RME_Svc_Handler(Reg);
    }
    /* Exception handler */
    else
    {
        /* Attempt to reexecute the faulting instruction */
        __RME_RV64V_Exc_Handler(Reg,Scause);
    }

    /* Never ever allow any modifications to SSTATUS bypass checks: modifying
     * return states, or allow a FPU-disabled thread to access FPU */
    RME_RV64V_SSTATUS_FIX(Reg);
}
/* End Function:_RME_RV64V_Handler *******************************************/

/* Function:__RME_RV64V_Pgt_Entry_Mod *****************************************
Description : Consult or modify the page table attributes. RV64V only allows
              consulting page table attributes but does not allow modifying them,
              because there are no architecture-specific flags.
Input       : struct RME_Cap_Cpt* Cpt - The current capability table.
              rme_cid_t Cap_Pgt - The capability to the top-level page table to consult.
              rme_ptr_t Vaddr - The virtual address to consult.
              rme_ptr_t Type - The consult type, see manual for details.
Output      : None.
Return      : rme_ret_t - If successful, the flags; else RME_ERR_KFN_FAIL.
******************************************************************************/
#if(RME_PGT_RAW_ENABLE==0U)
rme_ret_t __RME_RV64V_Pgt_Entry_Mod(struct RME_Cap_Cpt* Cpt,
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
        case RME_RV64V_KFN_PGT_ENTRY_MOD_FLAG_GET: return (rme_ret_t)Flags;
        case RME_RV64V_KFN_PGT_ENTRY_MOD_SZORD_GET: return (rme_ret_t)Size_Order;
        case RME_RV64V_KFN_PGT_ENTRY_MOD_NMORD_GET: return (rme_ret_t)Num_Order;
        default:break;
    }
    
    return RME_ERR_KFN_FAIL;
}
#endif
/* End Function:__RME_RV64V_Pgt_Entry_Mod ************************************/

/* Function:__RME_RV64V_Debug_Reg_Mod *****************************************
Description : Debug regular register modification implementation for RV64V.
Input       : struct RME_Cap_Cpt* Cpt - The current capability table.
              struct RME_Reg_Struct* Reg - The current register set.
              rme_cid_t Cap_Thd - The capability to the thread to consult.
              rme_ptr_t Operation - The operation, e.g. which register to read or write.
              rme_ptr_t Value - Unused - the value to write is taken from the
                                second return register (X11_A1) of Reg.
Output      : struct RME_Reg_Struct* Reg - The register set when exiting the handler.
Return      : rme_ret_t - If successful, 0; if a negative value, failed.
******************************************************************************/
rme_ret_t __RME_RV64V_Debug_Reg_Mod(struct RME_Cap_Cpt* Cpt,
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
    Register=Operation&(~RME_RV64V_KFN_DEBUG_REG_MOD_SET);
    if(Register<=RME_RV64V_KFN_DEBUG_REG_MOD_X31_T6)
    {
        Position=(rme_ptr_t*)&(Thd_Struct->Ctx.Reg->Reg);
        Position=&(Position[Register]);
    }
    /* See if this thread have FPU context. */
#if(RME_COP_NUM!=0U)
    else if((RME_THD_ATTR(Thd_Struct->Ctx.Hyp_Attr)!=0U)&&
            (Register<=RME_RV64V_KFN_DEBUG_REG_MOD_F31))
    {
        /* One rme_ptr_t slot per FP register; RVF uses the low 32 bits. */
        Position=(rme_ptr_t*)&(Thd_Struct->Ctx.Reg->Cop);
        Position=&(Position[Register-RME_RV64V_KFN_DEBUG_REG_MOD_FCSR]);
    }
#endif
    else
        return RME_ERR_KFN_FAIL;

    /* Perform read/write */
    if((Operation&RME_RV64V_KFN_DEBUG_REG_MOD_SET)==0U)
    {
        Reg->X11_A1=Position[0];
#if(RME_RV64V_COP_RVD==0U)
        Reg->X11_A1&=0xFFFFFFFFU;
#endif
    }
    else
    {
        Position[0]=Reg->X11_A1;
#if(RME_RV64V_COP_RVD==0U)
        Position[0]&=0xFFFFFFFFU;
#endif
    }

    return 0;
}
/* End Function:__RME_RV64V_Debug_Reg_Mod ************************************/

/* Function:__RME_RV64V_Debug_Inv_Mod *****************************************
Description : Debug invocation register modification implementation for RV64V.
Input       : struct RME_Cap_Cpt* Cpt - The current capability table.
              struct RME_Reg_Struct* Reg - The current register set.
              rme_cid_t Cap_Thd - The capability to the thread to consult.
              rme_ptr_t Operation - The operation, e.g. which register to read or write.
              rme_ptr_t Value - The value to write into the SP. The PC is
                                written from the second return register
                                (X11_A1) of Reg, like the general registers.
Output      : struct RME_Reg_Struct* Reg - The register set when exiting the handler.
Return      : rme_ret_t - If successful, 0; if a negative value, failed.
******************************************************************************/
rme_ret_t __RME_RV64V_Debug_Inv_Mod(struct RME_Cap_Cpt* Cpt,
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
        case RME_RV64V_KFN_DEBUG_INV_MOD_SP_GET:    {Reg->X11_A1=Inv_Struct->Ret.X2_SP;break;}
        case RME_RV64V_KFN_DEBUG_INV_MOD_SP_SET:    {Inv_Struct->Ret.X2_SP=Value;break;}
        case RME_RV64V_KFN_DEBUG_INV_MOD_PC_GET:    {Reg->X11_A1=Inv_Struct->Ret.PC;break;}
        case RME_RV64V_KFN_DEBUG_INV_MOD_PC_SET:    {Inv_Struct->Ret.PC=Reg->X11_A1;break;}
        default:                                    {return RME_ERR_KFN_FAIL;}
    }

    return 0;
}
/* End Function:__RME_RV64V_Debug_Inv_Mod ************************************/

/* Function:__RME_RV64V_Debug_Exc_Get *****************************************
Description : Debug exception register extraction implementation for RV64V.
Input       : struct RME_Cap_Cpt* Cpt - The current capability table.
              struct RME_Reg_Struct* Reg - The current register set.
              rme_cid_t Cap_Thd - The capability to the thread to consult.
              rme_ptr_t Operation - The operation, e.g. which register to read.
Output      : struct RME_Reg_Struct* Reg - The register set when exiting the
                                           handler.
Return      : rme_ret_t - If successful, 0; if a negative value, failed.
******************************************************************************/
rme_ret_t __RME_RV64V_Debug_Exc_Get(struct RME_Cap_Cpt* Cpt,
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
        case RME_RV64V_KFN_DEBUG_EXC_CAUSE_GET:     {Reg->X12_A2=Thd_Struct->Ctx.Reg->Exc.Cause;break;}
        case RME_RV64V_KFN_DEBUG_EXC_ADDR_GET:      {Reg->X12_A2=Thd_Struct->Ctx.Reg->Exc.Addr;break;}
        case RME_RV64V_KFN_DEBUG_EXC_VALUE_GET:     {Reg->X12_A2=Thd_Struct->Ctx.Reg->Exc.Value;break;}
        default:                                    {return RME_ERR_KFN_FAIL;}
    }

    return 0U;
}
/* End Function:__RME_RV64V_Debug_Exc_Get ************************************/

/* Function:__RME_RV64V_Cache_Maint *******************************************
Description : Do cache maintenance. RV64V has no architected whole-cache
              operation; only address-based Zicbom operations and fence.i are
              supported. The raw .insn encoding is used because -march=rv64gc
              does not enable the Zicbom assembler extension.
Input       : Cache_ID - RME_RV64V_KFN_CACHE_ICACHE or DCACHE.
              Operation - CLEAN_ADDR / INV_ADDR / CLEAN_INV_ADDR, or INV_ALL
                          for the instruction cache.
              Param - The virtual address (for address operations).
Output      : None.
Return      : If successful, 0; else RME_ERR_KFN_FAIL.
******************************************************************************/
rme_ret_t __RME_RV64V_Cache_Maint(rme_ptr_t Cache_ID,
                                  rme_ptr_t Operation,
                                  rme_ptr_t Param)
{
    rme_ptr_t Addr;
    rme_ptr_t Sstatus;
    rme_ret_t Retval;

    if(Cache_ID==RME_RV64V_KFN_CACHE_ICACHE)
    {
        /* fence.i is a full instruction-cache sync; the address is ignored. */
        if((Operation==RME_RV64V_KFN_CACHE_INV_ALL)||
           (Operation==RME_RV64V_KFN_CACHE_INV_ADDR))
        {
            __asm__ volatile("fence.i" : : : "memory");
            return 0;
        }

        return RME_ERR_KFN_FAIL;
    }

    if(Cache_ID==RME_RV64V_KFN_CACHE_DCACHE)
    {
        /* Zicbom operates on one cache block (64B on QEMU virt and K3). */
        Addr=Param&(~((rme_ptr_t)RME_RV64V_CACHE_BLOCK_SIZE-1U));

        /* The address is a user VA, but the KFN runs in S-mode. Enable SUM
         * while the CBO instruction probes the user page, then restore it. */
        Sstatus=___RME_RV64V_SSTATUS_Get();
        ___RME_RV64V_SSTATUS_Set(Sstatus|RME_RV64V_SSTATUS_SUM);

        switch(Operation)
        {
            case RME_RV64V_KFN_CACHE_CLEAN_ADDR:
            {
                /* cbo.clean: funct3=2, imm=1 */
                __asm__ volatile(".insn i 0x0F, 2, x0, %0, 0x001" : : "r"(Addr) : "memory");
                Retval=0;
                break;
            }
            case RME_RV64V_KFN_CACHE_INV_ADDR:
            {
                /* cbo.inval: funct3=2, imm=0 */
                __asm__ volatile(".insn i 0x0F, 2, x0, %0, 0x000" : : "r"(Addr) : "memory");
                Retval=0;
                break;
            }
            case RME_RV64V_KFN_CACHE_CLEAN_INV_ADDR:
            {
                /* cbo.flush: funct3=2, imm=2 (clean + invalidate) */
                __asm__ volatile(".insn i 0x0F, 2, x0, %0, 0x002" : : "r"(Addr) : "memory");
                Retval=0;
                break;
            }
            default:
            {
                Retval=RME_ERR_KFN_FAIL;
                break;
            }
        }

        __asm__ volatile("fence" : : : "memory");
        ___RME_RV64V_SSTATUS_Set(Sstatus);
        return Retval;
    }

    return RME_ERR_KFN_FAIL;
}
/* End Function:__RME_RV64V_Cache_Maint **************************************/

/* Function:__RME_RV64V_Pgt_Asid_Set *****************************************
Description : Set the ASID of a top-level page table. The old and new ASID
              TLB entries are flushed before the change, so an ASID can be
              reused safely. ASID 0 is left untagged for the boot mapping.
Input       : struct RME_Cap_Cpt* Cpt - The current capability table.
              rme_cid_t Cap_Pgt - The capability to the top-level page table.
              rme_ptr_t Asid - The new ASID (0..0xFFFF).
Output      : None.
Return      : rme_ret_t - If successful, 0; else RME_ERR_KFN_FAIL.
******************************************************************************/
rme_ret_t __RME_RV64V_Pgt_Asid_Set(struct RME_Cap_Cpt* Cpt,
                                   rme_cid_t Cap_Pgt,
                                   rme_ptr_t Asid)
{
    struct RME_Cap_Pgt* Pgt_Op;
    rme_ptr_t Type_Stat;

    /* Get the capability slot */
    RME_CPT_GETCAP(Cpt,Cap_Pgt,RME_CAP_TYPE_PGT,
                   struct RME_Cap_Pgt*,Pgt_Op,Type_Stat);

    /* Only a top-level table represents an address space */
    if((Pgt_Op->Base&RME_PGT_TOP)==0U)
        return RME_ERR_KFN_FAIL;

    if(Asid>RME_RV64V_SATP_ASID_MASK)
        return RME_ERR_KFN_FAIL;

    if(Pgt_Op->ASID==Asid)
        return 0;

    /* Drop stale translations for both the old and the new ASID before the
     * change. ASID 0 is shared by the boot/unassigned tables, so it is never
     * flushed here. */
    if(Pgt_Op->ASID!=0U)
        __asm__ volatile("sfence.vma x0, %0" : : "r"(Pgt_Op->ASID) : "memory");
    if(Asid!=0U)
        __asm__ volatile("sfence.vma x0, %0" : : "r"(Asid) : "memory");

    RME_WRITE_RELEASE((rme_ptr_t*)&(Pgt_Op->ASID),Asid);

    return 0;
}
/* End Function:__RME_RV64V_Pgt_Asid_Set ************************************/

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
        case RME_KFN_PGT_ASID_SET:
        {
            Retval=__RME_RV64V_Pgt_Asid_Set(Cpt,
                                            (rme_cid_t)Sub_ID,
                                            Param1);
            break;
        }
        case RME_KFN_PGT_TLB_LOCK:      {return RME_ERR_KFN_FAIL;}
#if(RME_PGT_RAW_ENABLE==0U)
        case RME_KFN_PGT_ENTRY_MOD:
        {
            Retval=__RME_RV64V_Pgt_Entry_Mod(Cpt,
                                             (rme_cid_t)Sub_ID,
                                             Param1,
                                             Param2);
            break;
        }
#endif
/* Interrupt controller operations *******************************************/
        /* QEMU virt has no PFIC - interrupt management is handled by OpenSBI
         * (PLIC/CLINT) at the trap level, so these are unsupported. */
        case RME_KFN_INT_LOCAL_MOD:     {return RME_ERR_KFN_FAIL;}
        case RME_KFN_INT_GLOBAL_MOD:    {return RME_ERR_KFN_FAIL;}
        case RME_KFN_INT_LOCAL_TRIG:    {return RME_ERR_KFN_FAIL;}
        case RME_KFN_EVT_LOCAL_TRIG:    {return RME_ERR_KFN_FAIL;}
/* Cache and prefetcher operations *******************************************/
        /* CACHE_MAINT = Zicbom. No architected cache config/lock or prefetcher
         * control; the rest stay unsupported. */
        case RME_KFN_CACHE_MOD:         {return RME_ERR_KFN_FAIL;}
        case RME_KFN_CACHE_CONFIG:      {return RME_ERR_KFN_FAIL;}
        case RME_KFN_CACHE_MAINT:
        {
            Retval=__RME_RV64V_Cache_Maint(Sub_ID,
                                           Param1,
                                           Param2);
            break;
        }
        case RME_KFN_CACHE_LOCK:        {return RME_ERR_KFN_FAIL;}
        case RME_KFN_PRFTH_MOD:         {return RME_ERR_KFN_FAIL;}
/* Hot plug and pull operations **********************************************/
        case RME_KFN_HPNP_PCPU_MOD:     {return RME_ERR_KFN_FAIL;}
        case RME_KFN_HPNP_LCPU_MOD:     {return RME_ERR_KFN_FAIL;}
        case RME_KFN_HPNP_PMEM_MOD:     {return RME_ERR_KFN_FAIL;}
/* Power and frequency adjustment operations *********************************/
        case RME_KFN_IDLE_SLEEP:
        {
            RME_RV64V_WAIT_INT_PRE();
            __RME_RV64V_Wait_Int();
            RME_RV64V_WAIT_INT_POST();
            Retval=0;
            break;
        }
        case RME_KFN_SYS_REBOOT:
        {
            __RME_RV64V_Reboot();
            while(1);
            break;
        }
        case RME_KFN_SYS_SHDN:          {return RME_ERR_KFN_FAIL;}
        case RME_KFN_VOLT_MOD:          {return RME_ERR_KFN_FAIL;}
        case RME_KFN_FREQ_MOD:          {return RME_ERR_KFN_FAIL;}
        case RME_KFN_PMOD_MOD:          {return RME_ERR_KFN_FAIL;}
        case RME_KFN_SAFETY_MOD:        {return RME_ERR_KFN_FAIL;}
/* Performance monitoring operations *****************************************/
        /* misa is a machine-level CSR and the performance counters have no
         * S-mode control interface, so every performance KFN is unsupported. */
        case RME_KFN_PERF_CPU_FUNC:     {return RME_ERR_KFN_FAIL;}
        case RME_KFN_PERF_MON_MOD:      {return RME_ERR_KFN_FAIL;}
        case RME_KFN_PERF_CNT_MOD:      {return RME_ERR_KFN_FAIL;}
        case RME_KFN_PERF_CYCLE_MOD:    {return RME_ERR_KFN_FAIL;}
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
            Retval=__RME_RV64V_Debug_Reg_Mod(Cpt,       /* Value in a3 */
                                             Reg,
                                             (rme_cid_t)Sub_ID,
                                             Param1,
                                             Param2);
            break;
        }
        case RME_KFN_DEBUG_INV_MOD:
        {
            Retval=__RME_RV64V_Debug_Inv_Mod(Cpt,       /* Value in a3 */
                                             Reg,
                                             (rme_cid_t)Sub_ID,
                                             Param1,
                                             Param2);
            break;
        }
        case RME_KFN_DEBUG_EXC_GET:
        {
            Retval=__RME_RV64V_Debug_Exc_Get(Cpt,       /* Value in a3 */
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
            return RME_ERR_KFN_FAIL;
        }
    }

    /* The operation succeeded - report a zero return value to the caller */
    if(Retval>=0)
        __RME_Svc_Retval_Set(Reg,0);

    return Retval;
}
/* End Function:__RME_Kfn_Handler ********************************************/

/* Function:__RME_RV64V_Pgt_Boot_Init ******************************************
Description : Build the bootstrap root page table and turn the MMU on. This is
              called from the assembly startup stub with the MMU still off,
              so it must not rely on any virtual mapping: every PC-relative
              address it computes is a physical one.
              The table holds two 1GB leaves: the identity map (so the code
              keeps executing right after the satp write) and the same window
              at the Sv39 high half. The chip's physical window base is the
              constant RME_RV64V_KPGT_PA from the chip header.
Input       : None.
Output      : None.
Return      : None.
*******************************************************************************/
void __RME_RV64V_Pgt_Boot_Init(void)
{
    rme_ptr_t* Root;
    rme_ptr_t Pte;
    rme_ptr_t Satp;
    rme_cnt_t Count;

    /* Build the bootstrap root page table. The MMU is still off, so the
       PC-relative address of the table is its physical address. */
    Root=(rme_ptr_t*)__RME_Boot_Root;
    for(Count=0;Count<RME_POW2(RME_PGT_NUM_512);Count++)
        Root[Count]=0;
    /* The leaf PTE: PPN|V|R|W|X|A|D. The A (accessed) and D (dirty) bits are
       set up front - a physical board (SpacemiT K3) checks these strictly,
       unlike QEMU which tolerates A=D=0 leaves. The U bit stays 0 (kernel
       only). */
    Pte=RME_RV64V_MMU_PPN(RME_RV64V_KPGT_PA)|
         RME_RV64V_MMU_V|RME_RV64V_MMU_R|RME_RV64V_MMU_W|
         RME_RV64V_MMU_X|RME_RV64V_MMU_A|RME_RV64V_MMU_D;
    /* The identity map, needed for the few instructions right after the
       satp write, and the same window at the Sv39 high half. */
    Root[(RME_RV64V_KPGT_PA)>>RME_PGT_SIZE_1G]=Pte;
    Root[256U+((RME_RV64V_KPGT_PA)>>RME_PGT_SIZE_1G)]=Pte;

    /* satp = MODE(Sv39=8)<<60 | PPN(root) */
    Satp=(8UL<<60)|(((rme_ptr_t)Root)>>12);

    /* Turn the MMU on - the same helpers that __RME_Pgt_Set uses. */
    ___RME_RV64V_SATP_Set(Satp);
    ___RME_RV64V_TLB_Flush();
}
/* End Function:__RME_RV64V_Pgt_Boot_Init **********************************/

/* Function:__RME_RV64V_Lowlvl_Preinit ****************************************
Description : Initialize the low-level hardware, before the loading of the kernel
              even takes place.
Input       : None.
Output      : None.
Return      : None.
*******************************************************************************/
void __RME_RV64V_Lowlvl_Preinit(void)
{
    RME_RV64V_LOWLVL_PREINIT();
}
/* End Function:__RME_RV64V_Lowlvl_Preinit ***********************************/

/* Function:__RME_Lowlvl_Init *************************************************
Description : Initialize the low-level hardware and the boot hart's per-CPU
              record. Secondary harts are brought up later by
              __RME_RV64V_SMP_Init() once the kernel can start them.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_Lowlvl_Init(void)
{
    RME_RV64V_LOWLVL_INIT();

    /* Boot CPU always uses logical CPUID 0. It traps on its own entry of the
     * per-CPU kernel stack array - the linker-reserved boot stack is only
     * used by the early startup path, before the first trap. */
    RME_RV64V_CPU_Data[0].Kernel_SP=
        (rme_ptr_t)&(RME_RV64V_KStack[0][0])+RME_RV64V_KSTACK_SIZE;
    RME_RV64V_CPU_Data[0].Local=&RME_RV64V_CPU_Local[0];
    RME_RV64V_CPU_Data[0].User_SP=0U;
    RME_RV64V_CPU_Data[0].User_TP=0U;
    RME_RV64V_CPU_Data[0].User_T6=0U;
    RME_RV64V_CPU_Data_Ptr[0]=(rme_ptr_t)&RME_RV64V_CPU_Data[0];

    RME_RV64V_CPU_Info[0].Hart_ID=RME_RV64V_Boot_Hart;
    RME_RV64V_CPU_Info[0].Boot_Done=1U;

    _RME_CPU_Local_Init(&RME_RV64V_CPU_Local[0],0U);
    ___RME_RV64V_CPU_Set(0U);

    RME_RV64V_CPU_Cnt=0U;
    RME_RV64V_Num_CPU=1U;
}
/* End Function:__RME_Lowlvl_Init ********************************************/

/* Function:__RME_RV64V_SMP_Init **********************************************
Description : Start all secondary harts through SBI HSM, one by one. The global
              RME_RV64V_CPU_Cnt is both the next logical CPUID and the AP boot
              gate. Each AP initializes its CPU-local data, sets Boot_Done, and
              then spins on RME_RV64V_CPU_Cnt until the boot hart has created
              all kernel objects.
              The number of harts is a build time configuration
              (RME_RV64V_CPU_NUM). A single core configuration has no secondary
              hart, so the loop below starts nothing and the boot hart remains
              the only CPU.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_RV64V_SMP_Init(void)
{
    rme_ptr_t Hart;
    rme_ptr_t CPUID;
    rme_ret_t Ret;
    rme_ptr_t Start_Addr;
    rme_ptr_t Start_Time;

    Start_Addr=RME_RV64V_VA2PA((rme_ptr_t)__RME_RV64V_SMP_Boot);

    /* Nonzero keeps every AP waiting. During the loop the value is also the
     * logical CPUID of the AP being started. __RME_Boot() clears it to zero
     * once all per-CPU objects exist. */
    RME_WRITE_RELEASE(&RME_RV64V_CPU_Cnt,1U);

    for(Hart=0U;Hart<RME_RV64V_CPU_NUM;Hart++)
    {
        /* Never start ourselves again */
        if(Hart==RME_RV64V_Boot_Hart)
            continue;

        /* The counter is the logical CPUID for this AP */
        CPUID=RME_READ_ACQUIRE(&RME_RV64V_CPU_Cnt);
        if(CPUID>=RME_RV64V_CPU_NUM)
            break;

        /* Prepare this CPU's record before releasing it */
        RME_RV64V_CPU_Data[CPUID].Kernel_SP=
            (rme_ptr_t)&(RME_RV64V_KStack[CPUID][0])+RME_RV64V_KSTACK_SIZE;
        RME_RV64V_CPU_Data[CPUID].Local=&RME_RV64V_CPU_Local[CPUID];
        RME_RV64V_CPU_Data[CPUID].User_SP=0U;
        RME_RV64V_CPU_Data[CPUID].User_TP=0U;
        RME_RV64V_CPU_Data[CPUID].User_T6=0U;
        RME_RV64V_CPU_Data_Ptr[CPUID]=(rme_ptr_t)&RME_RV64V_CPU_Data[CPUID];
        RME_RV64V_CPU_Info[CPUID].Hart_ID=Hart;
        RME_RV64V_CPU_Info[CPUID].Boot_Done=0U;

        /* The AP starts with the MMU off and reads these records through
         * physical addresses. Order the stores before the SBI call. */
        __RME_RV64V_Barrier();

        Ret=___RME_RV64V_Sbi_Hart_Start(Hart,Start_Addr,CPUID);
        if(Ret!=RME_RV64V_SBI_SUCCESS)
        {
            /* SBI reports failure only when the hart was not started, so the
             * same CPUID slot can be reused by the next hart. */
            RME_DBG_S(" SBI HSM failed: ");
            RME_DBG_I((rme_cnt_t)Ret);
            continue;
        }

        /* Wait for the AP to finish its own low-level initialization. Use a
         * cycle-counter timeout so a broken hart cannot hang the boot CPU. */
        Start_Time=___RME_RV64V_CYCLE_Get();
        while(RME_READ_ACQUIRE(&(RME_RV64V_CPU_Info[CPUID].Boot_Done))==0U)
        {
            if((___RME_RV64V_CYCLE_Get()-Start_Time)>1000000000UL)
                break;
        }

        if(RME_READ_ACQUIRE(&(RME_RV64V_CPU_Info[CPUID].Boot_Done))==0U)
        {
            /* The timed-out hart may still be alive. Reusing its CPUID would
             * make two harts share one CPU-local record and kernel stack, so
             * stop the boot instead of continuing with a broken slot. */
            RME_DBG_S(" boot timeout\r\n");
            RME_ASSERT(0);
            while(1);
        }

        /* Advance to the next logical CPUID. The value stays nonzero, so the
         * APs keep waiting until __RME_Boot() releases them. */
        RME_WRITE_RELEASE(&RME_RV64V_CPU_Cnt,CPUID+1U);
    }

    /* CPU_Cnt is still nonzero here; __RME_Boot() clears it after creating
     * every per-CPU object. */
    RME_RV64V_Num_CPU=RME_READ_ACQUIRE(&RME_RV64V_CPU_Cnt);
}
/* End Function:__RME_RV64V_SMP_Init *****************************************/

/* Function:__RME_SMP_Low_Level_Init ******************************************
Description : Low-level initialization for secondary harts. Called from the
              AP trampoline with the logical CPU index and hardware hart ID.
              Never returns: the AP enters its initial user thread.
Input       : rme_ptr_t CPUID - The logical CPU index.
              rme_ptr_t Hart_ID - The hardware hart ID.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_SMP_Low_Level_Init(rme_ptr_t CPUID,
                              rme_ptr_t Hart_ID)
{
    struct RME_CPU_Local* Local;

    RME_ASSERT(CPUID<RME_RV64V_CPU_NUM);

    /* Initialize this hart's CPU-local data. tp/sscratch/stvec have already
     * been installed by the trampoline. */
    _RME_CPU_Local_Init(&RME_RV64V_CPU_Local[CPUID],CPUID);
    RME_RV64V_CPU_Data[CPUID].Local=&RME_RV64V_CPU_Local[CPUID];
    RME_RV64V_CPU_Info[CPUID].Hart_ID=Hart_ID;

    /* Tell the boot hart that our low-level setup is complete */
    RME_WRITE_RELEASE(&(RME_RV64V_CPU_Info[CPUID].Boot_Done),1U);
    __RME_RV64V_Barrier();

    /* Wait until the boot hart has created all kernel objects and releases
     * the barrier. Interrupts are still disabled, so do not WFI here. */
    while(RME_READ_ACQUIRE(&RME_RV64V_CPU_Cnt)!=0U)
        __RME_RV64V_Barrier();

    Local=RME_CPU_LOCAL();
    RME_ASSERT(Local->Thd_Cur!=RME_NULL);
    RME_ASSERT(Local->Sig_Tim!=RME_NULL);
    RME_ASSERT(Local->Sig_Vct!=RME_NULL);

    /* Switch to the initial process page table */
    __RME_Pgt_Set(Local->Thd_Cur->Sched.Prc->Pgt);

    /* Every hart owns its timer: arm it and let the user thread run */
    ___RME_RV64V_Sie_Timer_Enable();
    ___RME_RV64V_Sbi_Timer_Set(___RME_RV64V_Time_Get()+RME_RV64V_OSTIM_VAL);
    ___RME_RV64V_SCOUNTEREN_Set(0x7U);

    __RME_User_Enter(RME_RV64V_INIT_ENTRY,
                     RME_RV64V_INIT_STACK(CPUID),
                     CPUID);

    /* Should never reach here */
    while(1);
}
/* End Function:__RME_SMP_Low_Level_Init *************************************/

/* Function:__RME_Boot ********************************************************
Description : Boot the first process in the system and, on SMP systems, the
              initial thread of every secondary hart.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_Boot(void)
{
    rme_ptr_t Cur_Addr;
    rme_ptr_t Stack_Pa;
    rme_cnt_t Count;
    rme_cnt_t Page;
    struct RME_CPU_Local* CPU_Local;

    Cur_Addr=RME_KOM_VA_BASE;

    /* Start the secondary harts first; they will spin on RME_RV64V_CPU_Cnt
     * until all kernel objects below have been created. */
    __RME_RV64V_SMP_Init();

    /* Create the capability table for the init process - RME_RV64V_INIT_CPT_SIZE
     * entries. Boot consumes slots 0..9; the benchmark's objects start at 10. */
    RME_ASSERT(_RME_Cpt_Boot_Init(RME_BOOT_INIT_CPT,Cur_Addr,RME_RV64V_INIT_CPT_SIZE)==RME_BOOT_INIT_CPT);
    Cur_Addr+=RME_KOM_ROUND(RME_CPT_SIZE(RME_RV64V_INIT_CPT_SIZE));

    /* Sv39 locates a page table through its PPN, so every table object must be
     * 4KB aligned (the KOM allocation granularity is only 16 bytes) */
    Cur_Addr=RME_ROUND_UP(Cur_Addr,12);

    /* The top-level (root) page table - one entry per 1GB. Its high half is
     * pre-filled with the shared kernel mappings by __RME_Pgt_Init. */
    RME_ASSERT(_RME_Pgt_Boot_Crt(RME_RV64V_CPT,
                                 RME_BOOT_INIT_CPT,
                                 RME_BOOT_INIT_PGT,
                                 Cur_Addr,
                                 0x00000000U,
                                 RME_PGT_TOP,
                                 RME_PGT_SIZE_1G,
                                 RME_PGT_NUM_512)==0);
    Cur_Addr+=RME_KOM_ROUND(RME_PGT_SIZE_TOP(RME_PGT_NUM_512));

    /* The second-level page table for the init process's user address space.
     * It covers [0, 1GB); only the user window below is populated, the rest of
     * the low half stays unmapped so user mode cannot reach kernel memory. */
    RME_ASSERT(_RME_Pgt_Boot_Crt(RME_RV64V_CPT,
                                 RME_BOOT_INIT_CPT,
                                 RME_BOOT_INIT_PGT_L1,
                                 Cur_Addr,
                                 0x00000000U,
                                 RME_PGT_NOM,
                                 RME_PGT_SIZE_2M,
                                 RME_PGT_NUM_512)==0);
    Cur_Addr+=RME_KOM_ROUND(RME_PGT_SIZE_NOM(RME_PGT_NUM_512));

    /* The third-level page table for the user window [0x20000000,0x20200000)
     * at 4KB granularity. */
    RME_ASSERT(_RME_Pgt_Boot_Crt(RME_RV64V_CPT,
                                 RME_BOOT_INIT_CPT,
                                 RME_BOOT_INIT_PGT_L2,
                                 Cur_Addr,
                                 0x20000000U,
                                 RME_PGT_NOM,
                                 RME_PGT_SIZE_4K,
                                 RME_PGT_NUM_512)==0);
    Cur_Addr+=RME_KOM_ROUND(RME_PGT_SIZE_NOM(RME_PGT_NUM_512));

    /* Attach the second-level table to top entry 0 (0x20000000>>30 = 0) */
    RME_ASSERT(_RME_Pgt_Boot_Con(RME_RV64V_CPT,
                                 RME_BOOT_INIT_PGT,
                                 0U,
                                 RME_BOOT_INIT_PGT_L1,
                                 RME_PGT_ALL_PERM)==0);

    /* Attach the third-level table to L1 entry 256 (0x20000000>>21) */
    RME_ASSERT(_RME_Pgt_Boot_Con(RME_RV64V_CPT,
                                 RME_BOOT_INIT_PGT_L1,
                                 256U,
                                 RME_BOOT_INIT_PGT_L2,
                                 RME_PGT_ALL_PERM)==0);

    /* The init thread's user code at VA 0x20000000, backed by physical
     * RME_RV64V_INIT_PA where the loader (QEMU -device loader) placed the
     * image or the linker embedded it (SpacemiT K3). Map 16 pages (64KB) so
     * the init program's .data/.bss beyond the first page is reachable. */
    for(Count=0;Count<16;Count++)
    {
        RME_ASSERT(_RME_Pgt_Boot_Add(RME_RV64V_CPT,
                                     RME_BOOT_INIT_PGT_L2,
                                     RME_RV64V_INIT_PA+(((rme_ptr_t)Count)<<RME_PGT_SIZE_4K),
                                     (rme_ptr_t)Count,
                                     RME_PGT_ALL_PERM)==0);
    }

    /* Map one 64KB user stack per CPU. The stacks live in the kernel .bss and
     * are mapped into the low user window, so every hart has a private stack
     * and no fixed bootloader physical address is required. */
    for(Count=0;Count<(rme_cnt_t)RME_RV64V_Num_CPU;Count++)
    {
        Stack_Pa=RME_RV64V_VA2PA((rme_ptr_t)&(RME_RV64V_UStack[Count][0]));
        for(Page=0;Page<16;Page++)
        {
            RME_ASSERT(_RME_Pgt_Boot_Add(RME_RV64V_CPT,
                                         RME_BOOT_INIT_PGT_L2,
                                         Stack_Pa+(((rme_ptr_t)Page)<<RME_PGT_SIZE_4K),
                                         (((RME_RV64V_INIT_STACK(Count)-RME_RV64V_USTACK_SIZE)-0x20000000U)>>RME_PGT_SIZE_4K)+
                                         (rme_ptr_t)Page,
                                         RME_PGT_ALL_PERM)==0);
        }
    }

    /* Activate the first process - This process cannot be deleted */
    RME_ASSERT(_RME_Prc_Boot_Crt(RME_RV64V_CPT,
                                 RME_BOOT_INIT_CPT,
                                 RME_BOOT_INIT_PRC,
                                 RME_BOOT_INIT_CPT,
                                 RME_BOOT_INIT_PGT)==0U);

    /* Create the initial kernel function capability, and kernel memory capability */
    RME_ASSERT(_RME_Kfn_Boot_Crt(RME_RV64V_CPT,
                                 RME_BOOT_INIT_CPT,
                                 RME_BOOT_INIT_KFN)==0);
    RME_ASSERT(_RME_Kom_Boot_Crt(RME_RV64V_CPT,
                                 RME_BOOT_INIT_CPT,
                                 RME_BOOT_INIT_KOM,
                                 RME_KOM_VA_BASE,
                                 RME_KOM_VA_BASE+RME_KOM_VA_SIZE-1U,
                                 RME_KOM_FLAG_ALL)==0U);

    /* Create the per-CPU timer endpoint table and endpoints */
    RME_ASSERT(_RME_Cpt_Boot_Crt(RME_RV64V_CPT,
                                 RME_BOOT_INIT_CPT,
                                 RME_BOOT_TBL_TIMER,
                                 Cur_Addr,
                                 RME_RV64V_Num_CPU)==0);
    Cur_Addr+=RME_KOM_ROUND(RME_CPT_SIZE(RME_RV64V_Num_CPU));
    for(Count=0;Count<(rme_cnt_t)RME_RV64V_Num_CPU;Count++)
    {
        CPU_Local=&RME_RV64V_CPU_Local[Count];
        CPU_Local->Sig_Tim=(struct RME_Cap_Sig*)
            &(RME_CAP_GETOBJ(&(RME_RV64V_CPT[RME_BOOT_TBL_TIMER]),struct RME_Cap_Sig*)[Count]);
        RME_ASSERT(_RME_Sig_Boot_Crt(RME_RV64V_CPT,
                                     RME_BOOT_TBL_TIMER,
                                     Count)==0);
    }

    /* Create the per-CPU general interrupt endpoint table and endpoints */
    RME_ASSERT(_RME_Cpt_Boot_Crt(RME_RV64V_CPT,
                                 RME_BOOT_INIT_CPT,
                                 RME_BOOT_TBL_INT,
                                 Cur_Addr,
                                 RME_RV64V_Num_CPU)==0);
    Cur_Addr+=RME_KOM_ROUND(RME_CPT_SIZE(RME_RV64V_Num_CPU));
    for(Count=0;Count<(rme_cnt_t)RME_RV64V_Num_CPU;Count++)
    {
        CPU_Local=&RME_RV64V_CPU_Local[Count];
        CPU_Local->Sig_Vct=(struct RME_Cap_Sig*)
            &(RME_CAP_GETOBJ(&(RME_RV64V_CPT[RME_BOOT_TBL_INT]),struct RME_Cap_Sig*)[Count]);
        RME_ASSERT(_RME_Sig_Boot_Crt(RME_RV64V_CPT,
                                     RME_BOOT_TBL_INT,
                                     Count)==0);
    }

    /* Create the per-CPU initial-thread table and one thread per CPU. All
     * threads belong to the same init process and share its address space. */
    RME_ASSERT(_RME_Cpt_Boot_Crt(RME_RV64V_CPT,
                                 RME_BOOT_INIT_CPT,
                                 RME_BOOT_TBL_THD,
                                 Cur_Addr,
                                 RME_RV64V_Num_CPU)==0);
    Cur_Addr+=RME_KOM_ROUND(RME_CPT_SIZE(RME_RV64V_Num_CPU));
    for(Count=0;Count<(rme_cnt_t)RME_RV64V_Num_CPU;Count++)
    {
        CPU_Local=&RME_RV64V_CPU_Local[Count];
        RME_ASSERT(_RME_Thd_Boot_Crt(RME_RV64V_CPT,
                                     RME_BOOT_TBL_THD,
                                     Count,
                                     RME_BOOT_INIT_PRC,
                                     Cur_Addr,
                                     0U,
                                     CPU_Local)==0);
        Cur_Addr+=RME_KOM_ROUND(RME_THD_SIZE(0U));
    }

    /* Switch to the init process page table */
#if(RME_PGT_RAW_ENABLE==0U)
    RME_ASSERT(RME_CAP_IS_ROOT(RME_CPU_LOCAL()->Thd_Cur->Sched.Prc->Pgt)!=0U);
#endif
    __RME_Pgt_Set(RME_CPU_LOCAL()->Thd_Cur->Sched.Prc->Pgt);

    /* Let the user mode read the cycle/time/instret counters - the benchmark
     * uses the cycle counter as its timestamp counter. */
    ___RME_RV64V_SCOUNTEREN_Set(0x7U);

    /* Release all secondary harts now that every CPU has its thread and
     * endpoints. They proceed into their own initial user thread. */
    RME_WRITE_RELEASE(&RME_RV64V_CPU_Cnt,0U);

    /* Boot into this CPU's initial thread */
    __RME_User_Enter(RME_RV64V_INIT_ENTRY,RME_RV64V_INIT_STACK(0U),0U);

    /* Should never reach here */
    while(1);
}
/* End Function:__RME_Boot ***************************************************/

/* Function:__RME_RV64V_Reboot ***********************************************
Description : Reboot the MCU, including all its peripherals.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_RV64V_Reboot(void)
{
    /* Reboots the platform, which includes all peripherals. */
    RME_RV64V_REBOOT();
}
/* End Function:__RME_RV64V_Reboot ******************************************/

/* Function:__RME_Svc_Param_Get ***********************************************
Description : Get the system call parameters from the stack frame.
Input       : struct RME_Reg_Struct* Reg - The register set.
Output      : rme_ptr_t* Svc - The system service number.
              rme_ptr_t* Cid - The capability ID number.
              rme_ptr_t* Param - The parameters.
Return      : None.
Notes       : The register layout is the same as the X64 port, so that the
              64-bit capability ID encoding (which needs 31 bits) is fully
              available in the top-level capability ID field:
                [63:48] Extended parameter (PC)  - PGT_CRT's Num_Order
                [47:39] Thread attribute (Attr)  - THD_CRT's Attr
                [ 38  ] Hypervisor flag         - THD_CRT's Is_Hyp
                [37:32] Service number          - Svc_Num
                [31: 0] Capability ID            - Cid (1-level or 2-level)
              The kernel derives Svc_Num from the low 6 bits of Svc, Is_Hyp
              from bit 6, Attr from bits 7 and above, and the extended
              parameter from bits 16 and above. Therefore the extended
              parameter and the thread attribute must never be used by the
              same system call - they share the same bits.
******************************************************************************/
void __RME_Svc_Param_Get(struct RME_Reg_Struct* Reg,
                         rme_ptr_t* Svc,
                         rme_ptr_t* Cid,
                         rme_ptr_t* Param)
{
    *Svc=(Reg->X10_A0)>>32;
    *Cid=(Reg->X10_A0)&0xFFFFFFFFU;
    Param[0]=Reg->X11_A1;
    Param[1]=Reg->X12_A2;
    Param[2]=Reg->X13_A3;
}
/* End Function:__RME_Svc_Param_Get ******************************************/

/* Function:__RME_Svc_Retval_Set **********************************************
Description : Set the system call return value to the register frame. Only one
              return value is supported - it is carried in A0.
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
    Reg->SSTATUS=RME_RV64V_SSTATUS_INIT;
#else
    if(Attr==RME_RV64V_ATTR_NONE)
        Reg->SSTATUS=RME_RV64V_SSTATUS_INIT;
    else
        Reg->SSTATUS=RME_RV64V_SSTATUS_INIT|RME_RV64V_SSTATUS_FPU_INIT;
#endif
    /* The entry point */
    Reg->PC=Entry;
    /* The stack */
    Reg->X2_SP=Stack;
    /* User thread pointer starts at zero; the kernel keeps its per-CPU
     * pointer in tp only while executing in S-mode. */
    Reg->X4_TP=0U;
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
    Dst->SSTATUS=Src->SSTATUS;
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

/* Function:__RME_Thd_Reg_Print ***********************************************
Description : Print thread registers. This is used exclusively for debugging.
Input       : struct RME_Reg_Struct* Reg - The register set.
Output      : None.
Return      : None.
******************************************************************************/
#if(RME_DBGLOG_ENABLE!=0U)
void __RME_Thd_Reg_Print(struct RME_Reg_Struct* Reg)
{
    RME_DBG_SHS("SSTATUS: 0x",Reg->SSTATUS,"\r\n");
    RME_DBG_SHS("PC: 0x",Reg->PC,"\r\n");
    RME_DBG_SHS("X1_RA: 0x",Reg->X1_RA,"\r\n");
    RME_DBG_SHS("X2_SP: 0x",Reg->X2_SP,"\r\n");
    RME_DBG_SHS("X3_GP: 0x",Reg->X3_GP,"\r\n");
    RME_DBG_SHS("X4_TP: 0x",Reg->X4_TP,"\r\n");
    RME_DBG_SHS("X5_T0: 0x",Reg->X5_T0,"\r\n");
    RME_DBG_SHS("X6_T1: 0x",Reg->X6_T1,"\r\n");
    RME_DBG_SHS("X7_T2: 0x",Reg->X7_T2,"\r\n");
    RME_DBG_SHS("X8_S0_FP: 0x",Reg->X8_S0_FP,"\r\n");
    RME_DBG_SHS("X9_S1: 0x",Reg->X9_S1,"\r\n");
    RME_DBG_SHS("X10_A0: 0x",Reg->X10_A0,"\r\n");
    RME_DBG_SHS("X11_A1: 0x",Reg->X11_A1,"\r\n");
    RME_DBG_SHS("X12_A2: 0x",Reg->X12_A2,"\r\n");
    RME_DBG_SHS("X13_A3: 0x",Reg->X13_A3,"\r\n");
    RME_DBG_SHS("X14_A4: 0x",Reg->X14_A4,"\r\n");
    RME_DBG_SHS("X15_A5: 0x",Reg->X15_A5,"\r\n");
    RME_DBG_SHS("X16_A6: 0x",Reg->X16_A6,"\r\n");
    RME_DBG_SHS("X17_A7: 0x",Reg->X17_A7,"\r\n");
    RME_DBG_SHS("X18_S2: 0x",Reg->X18_S2,"\r\n");
    RME_DBG_SHS("X19_S3: 0x",Reg->X19_S3,"\r\n");
    RME_DBG_SHS("X20_S4: 0x",Reg->X20_S4,"\r\n");
    RME_DBG_SHS("X21_S5: 0x",Reg->X21_S5,"\r\n");
    RME_DBG_SHS("X22_S6: 0x",Reg->X22_S6,"\r\n");
    RME_DBG_SHS("X23_S7: 0x",Reg->X23_S7,"\r\n");
    RME_DBG_SHS("X24_S8: 0x",Reg->X24_S8,"\r\n");
    RME_DBG_SHS("X25_S9: 0x",Reg->X25_S9,"\r\n");
    RME_DBG_SHS("X26_S10: 0x",Reg->X26_S10,"\r\n");
    RME_DBG_SHS("X27_S11: 0x",Reg->X27_S11,"\r\n");
    RME_DBG_SHS("X28_T3: 0x",Reg->X28_T3,"\r\n");
    RME_DBG_SHS("X29_T4: 0x",Reg->X29_T4,"\r\n");
    RME_DBG_SHS("X30_T5: 0x",Reg->X30_T5,"\r\n");
    RME_DBG_SHS("X31_T6: 0x",Reg->X31_T6,"\r\n");
}
#endif
/* End Function:__RME_Thd_Reg_Print ******************************************/

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
#if(RME_RV64V_COP_RVD!=0U)
    /* No checks necessary */
#elif(RME_RV64V_COP_RVF!=0U)
    /* If only RVF is selected, no RVD allowed */
    if((Attr&RME_RV64V_ATTR_RVD)!=0U)
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
Return      : rme_ptr_t - The context size in bytes; 0 if the thread does
              not use the FPU.
******************************************************************************/
#if(RME_COP_NUM!=0U)
rme_ptr_t __RME_Thd_Cop_Size(rme_ptr_t Attr)
{
    /* Even if the thread would specify only RVF when RVD is present, the
     * context is still as large as RVD because they use the same coprocessor */
    if(Attr!=RME_RV64V_ATTR_NONE)
        return sizeof(struct RME_RV64V_Cop_Struct);

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
    struct RME_RV64V_Cop_Struct* RV64V_Cop;

    if(Attr==RME_RV64V_ATTR_NONE)
        return;

    RV64V_Cop=Cop;

    /* The assembly save/load hard-codes FCSR at offset 0 and F0 at
     * offset 8, so the whole context is one 8-byte slot plus the FP
     * registers. */
    RV64V_Cop->FCSR=0U;
    RV64V_Cop->F0=0U;
    RV64V_Cop->F1=0U;
    RV64V_Cop->F2=0U;
    RV64V_Cop->F3=0U;
    RV64V_Cop->F4=0U;
    RV64V_Cop->F5=0U;
    RV64V_Cop->F6=0U;
    RV64V_Cop->F7=0U;
    RV64V_Cop->F8=0U;
    RV64V_Cop->F9=0U;
    RV64V_Cop->F10=0U;
    RV64V_Cop->F11=0U;
    RV64V_Cop->F12=0U;
    RV64V_Cop->F13=0U;
    RV64V_Cop->F14=0U;
    RV64V_Cop->F15=0U;
    RV64V_Cop->F16=0U;
    RV64V_Cop->F17=0U;
    RV64V_Cop->F18=0U;
    RV64V_Cop->F19=0U;
    RV64V_Cop->F20=0U;
    RV64V_Cop->F21=0U;
    RV64V_Cop->F22=0U;
    RV64V_Cop->F23=0U;
    RV64V_Cop->F24=0U;
    RV64V_Cop->F25=0U;
    RV64V_Cop->F26=0U;
    RV64V_Cop->F27=0U;
    RV64V_Cop->F28=0U;
    RV64V_Cop->F29=0U;
    RV64V_Cop->F30=0U;
    RV64V_Cop->F31=0U;
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

    State_Cur=Reg_Cur->SSTATUS&RME_RV64V_SSTATUS_FPU_MASK;
    State_New=Reg_New->SSTATUS&RME_RV64V_SSTATUS_FPU_MASK;

    /* Fix the coprocessor enabling status reported by the HYP threads */
    if(Is_Hyp_Cur!=0U)
    {
        if(Attr_Cur!=RME_RV64V_ATTR_NONE)
        {
            /* If the current thread uses the coprocessor but reports not using
             * it, treat the coprocessor as CLEAN, and enable the coprocessor */
            if(State_Cur==RME_RV64V_SSTATUS_FPU_OFF)
            {
                State_Cur=RME_RV64V_SSTATUS_FPU_CLEAN;
                ___RME_RV64V_SSTATUS_Set(RME_RV64V_SSTATUS_INIT|RME_RV64V_SSTATUS_FPU_CLEAN);
                Reg_Cur->SSTATUS=RME_RV64V_SSTATUS_INIT|RME_RV64V_SSTATUS_FPU_CLEAN;
            }
        }
        else
        {
            /* If the current thread does not use the coprocessor but reports using
             * it, treat the coprocessor as OFF, and disable the coprocessor; this in
             * theory does not happen because the sstatus will be fixed on the handler
             * return path, and the fix is done by the hart that owns the thread;
             * on multi-core processors, the fix must be on the assembly exit anyway */
            if((Attr_Cur==RME_RV64V_ATTR_NONE)&&(State_Cur!=RME_RV64V_SSTATUS_FPU_OFF))
            {
                State_Cur=RME_RV64V_SSTATUS_FPU_OFF;
                ___RME_RV64V_SSTATUS_Set(RME_RV64V_SSTATUS_INIT);
                Reg_Cur->SSTATUS=RME_RV64V_SSTATUS_INIT;
            }
        }
    }

    if(Is_Hyp_New!=0U)
    {
        if(Attr_New!=RME_RV64V_ATTR_NONE)
        {
            /* If the new thread uses the coprocessor but reports not using
             * it, treat the coprocessor as CLEAN */
            if(State_New==RME_RV64V_SSTATUS_FPU_OFF)
            {
                State_New=RME_RV64V_SSTATUS_FPU_CLEAN;
                Reg_New->SSTATUS=RME_RV64V_SSTATUS_INIT|RME_RV64V_SSTATUS_FPU_CLEAN;
            }
        }
        else
        {
            /* If the current thread does not use the coprocessor but reports using
             * it, treat the coprocessor as OFF */
            if((Attr_New==RME_RV64V_ATTR_NONE)&&(State_New!=RME_RV64V_SSTATUS_FPU_OFF))
            {
                State_New=RME_RV64V_SSTATUS_FPU_OFF;
                Reg_New->SSTATUS=RME_RV64V_SSTATUS_INIT;
            }
        }
    }

    /* The current thread does not use the coprocessor */
    if(Attr_Cur==RME_RV64V_ATTR_NONE)
    {
        /* The new thread does not use the coprocessor */
        if(Attr_New==RME_RV64V_ATTR_NONE)
        {
            /* No action required */
        }
        /* The new thread did not touch the coprocessor */
        else if(State_New==RME_RV64V_SSTATUS_FPU_INIT)
        {
            /* Turn the coprocessor on */
            Reg_New->SSTATUS=RME_RV64V_SSTATUS_INIT|RME_RV64V_SSTATUS_FPU_INIT;
            ___RME_RV64V_SSTATUS_Set(RME_RV64V_SSTATUS_INIT|RME_RV64V_SSTATUS_FPU_INIT);
        }
        /* The new thread touched the coprocessor and its context was saved */
        else
        {
            /* Turn the coprocessor on */
            Reg_New->SSTATUS=RME_RV64V_SSTATUS_INIT|RME_RV64V_SSTATUS_FPU_CLEAN;
            ___RME_RV64V_SSTATUS_Set(RME_RV64V_SSTATUS_INIT|RME_RV64V_SSTATUS_FPU_CLEAN);
            /* Load the new coprocessor context */
            RME_RV64V_THD_COP_LOAD(Cop_New);
        }
    }
    /* The current thread did not touch the coprocessor, and is not a HYP one */
    else if((State_Cur==RME_RV64V_SSTATUS_FPU_INIT)&&(Is_Hyp_Cur==0U))
    {
        /* The new thread does not use the coprocessor */
        if(Attr_New==RME_RV64V_ATTR_NONE)
        {
            /* Turn the coprocessor off */
            Reg_New->SSTATUS=RME_RV64V_SSTATUS_INIT;
            ___RME_RV64V_SSTATUS_Set(RME_RV64V_SSTATUS_INIT);
        }
        /* The new thread did not touch the coprocessor */
        else if(State_New==RME_RV64V_SSTATUS_FPU_INIT)
        {
            /* No action required */
        }
        /* The new thread touched the coprocessor, and was saved */
        else
        {
            /* Load the new coprocessor context */
            RME_RV64V_THD_COP_LOAD(Cop_New);
        }
    }
    /* The current thread touched the coprocessor and its context was saved, or it is a HYP one;
     * for HYP threads, the INIT status is not trustworthy and must always be treated as CLEAN */
    else if((State_Cur==RME_RV64V_SSTATUS_FPU_INIT)||(State_Cur==RME_RV64V_SSTATUS_FPU_CLEAN))
    {
        /* The new thread does not use the coprocessor */
        if(Attr_New==RME_RV64V_ATTR_NONE)
        {
            /* Reinitialize the coprocessor context */
            RME_RV64V_THD_COP_CLEAR();
            /* Turn the coprocessor off */
            Reg_New->SSTATUS=RME_RV64V_SSTATUS_INIT;
            ___RME_RV64V_SSTATUS_Set(RME_RV64V_SSTATUS_INIT);
        }
        /* The new thread did not touch the coprocessor */
        else if(State_New==RME_RV64V_SSTATUS_FPU_INIT)
        {
            /* Reinitialize the coprocessor context - untrustworthy "did not touch" */
            RME_RV64V_THD_COP_CLEAR();
        }
        /* The new thread touched the coprocessor, and its context was saved */
        else
        {
            /* Load the new coprocessor context */
            RME_RV64V_THD_COP_LOAD(Cop_New);
        }
    }
    /* The current thread touched the coprocessor and its context was not saved */
    else
    {
        /* Save the current coprocessor context */
        RME_RV64V_THD_COP_SAVE(Cop_Cur);
        /* Set the current thread to clean */
        Reg_Cur->SSTATUS=RME_RV64V_SSTATUS_INIT|RME_RV64V_SSTATUS_FPU_CLEAN;

        /* The new thread does not use the coprocessor */
        if(Attr_New==RME_RV64V_ATTR_NONE)
        {
            /* Reinitialize the coprocessor context */
            RME_RV64V_THD_COP_CLEAR();
            /* Turn the coprocessor off */
            Reg_New->SSTATUS=RME_RV64V_SSTATUS_INIT;
            ___RME_RV64V_SSTATUS_Set(RME_RV64V_SSTATUS_INIT);
        }
        /* The new thread did not touch the coprocessor */
        else if(State_New==RME_RV64V_SSTATUS_FPU_INIT)
        {
            /* Reinitialize the coprocessor context */
            RME_RV64V_THD_COP_CLEAR();
        }
        /* The new thread touched the coprocessor, and its context was saved */
        else
        {
            /* Load the new coprocessor context */
            RME_RV64V_THD_COP_LOAD(Cop_New);
        }
    }
}
#endif
/* End Function:__RME_Thd_Cop_Swap *******************************************/

/* Sv39 maps the RME flags directly onto the PTE bits, so the X64-style lookup
 * tables are gone - see RME_RV64V_PGFLG_RME2NAT/NAT2RME in the header. */


/* Function:__RME_Pgt_Kom_Init ************************************************
Description : Stub retained for the common kernel interface. The Sv39 kernel
              mapping (one 1GB leaf in the high half covering physical
              [0x80000000,0xC0000000)) is hardcoded directly in __RME_Pgt_Init,
              so there is no template left to build here.
Input       : None.
Output      : None.
Return      : rme_ret_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
rme_ret_t __RME_Pgt_Kom_Init(void)
{
     return 0;
}
/* End Function:__RME_Pgt_Kom_Init *******************************************/

/* Function:__RME_Pgt_Set ***************************************************
Description : Set the processor's page table.
Input       : struct RME_Cap_Pgt* Pgt - The capability to the root page table.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_Pgt_Set(struct RME_Cap_Pgt* Pgt)
{
    rme_ptr_t Satp;
    rme_ptr_t Asid;
    rme_ptr_t Old;
    rme_ptr_t New;

    Asid=Pgt->ASID&RME_RV64V_SATP_ASID_MASK;

    /* First switch: take the next ASID; exhaustion falls back to 0. */
    if(Asid==0U)
    {
        do
        {
            Old=RME_RV64V_ASID_Next;
            New=(Old==0U)?2U:(Old+1U);
        }
        while(RME_COMP_SWAP(&RME_RV64V_ASID_Next,Old,New)==RME_CASFAIL);

        Asid=(Old==0U)?1U:Old;

        if(Asid<=RME_RV64V_SATP_ASID_MASK)
        {
            /* Make sure the reused ASID has no stale translations. */
            __asm__ volatile("sfence.vma x0, %0" : : "r"(Asid) : "memory");
            if(RME_COMP_SWAP((volatile rme_ptr_t*)&(Pgt->ASID),0U,Asid)==RME_CASFAIL)
                Asid=Pgt->ASID&RME_RV64V_SATP_ASID_MASK;
        }
        else
        {
            Asid=0U;
        }
    }

    /* The page table object lives in the kernel object memory (high half), so
     * translate its VA to PA first. Build satp: [63:60] MODE = 8 (Sv39),
     * [59:44] ASID, [43:0] PPN = root_pa >> 12 (4KB aligned). Sv39 supports
     * a 56-bit physical address, so the PPN field is 44 bits. */
    Satp=(8UL<<60)|(Asid<<RME_RV64V_SATP_ASID_SHIFT)|
         ((RME_RV64V_VA2PA(RME_CAP_GETOBJ(Pgt,rme_ptr_t))>>12)&RME_RV64V_SATP_PPN_MASK);

    ___RME_RV64V_SATP_Set(Satp);

    /* Non-zero ASID was flushed at allocation; ASID 0 needs a full flush. */
    if(Asid==0U)
        ___RME_RV64V_TLB_Flush();
}
/* End Function:__RME_Pgt_Set **********************************************/

/* Function:__RME_Pgt_Check *************************************************
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
rme_ret_t __RME_Pgt_Check(rme_ptr_t Base_Addr, rme_ptr_t Is_Top,
                            rme_ptr_t Size_Order, rme_ptr_t Num_Order, rme_ptr_t Vaddr)
{
    /* Is the table address aligned to 4kB? Sv39 finds the table through a PPN */
    if((Vaddr&0xFFF)!=0)
        return RME_ERR_HAL_FAIL;

    /* Sv39 has exactly three levels: 1GB (root), 2MB, then 4KB */
    if((Size_Order!=RME_PGT_SIZE_1G)&&(Size_Order!=RME_PGT_SIZE_2M)&&
       (Size_Order!=RME_PGT_SIZE_4K))
        return RME_ERR_HAL_FAIL;

    /* Only the top-level table uses 1GB entries, and it must use them */
    if(((Size_Order==RME_PGT_SIZE_1G)^(Is_Top!=0))!=0)
        return RME_ERR_HAL_FAIL;

    /* Every Sv39 table has exactly 512 entries */
    if(Num_Order!=RME_PGT_NUM_512)
        return RME_ERR_HAL_FAIL;

    return 0;
}
/* End Function:__RME_Pgt_Check ********************************************/

/* Function:__RME_Pgt_Init **************************************************
Description : Initialize the page table data structure, according to the capability.
Input       : struct RME_Cap_Pgt* - The capability to the page table to operate on.
Output      : None.
Return      : rme_ret_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
rme_ret_t __RME_Pgt_Init(struct RME_Cap_Pgt* Pgt_Op)
{
    rme_cnt_t Count;
    rme_ptr_t* Ptr;

    /* Get the actual table - the object memory IS the hardware table */
    Ptr=RME_CAP_GETOBJ(Pgt_Op,rme_ptr_t*);

    /* Every Sv39 table has 512 entries and starts empty. A top-level table
     * also gets a single hardcoded kernel entry in the high half (below) so a
     * satp switch keeps the kernel mapped. */
    for(Count=0;Count<RME_POW2(RME_PGT_NUM_512);Count++)
        Ptr[Count]=0;

    if((Pgt_Op->Base&RME_PGT_TOP)!=0)
    {
        /* Top-level index of the high-half address that maps the kernel's
         * physical direct-map window. A and D are pre-set because some
         * hardware checks these strictly. */
        Ptr[256U+((RME_RV64V_KPGT_PA)>>RME_PGT_SIZE_1G)]=RME_RV64V_MMU_PPN(RME_RV64V_KPGT_PA)|RME_RV64V_MMU_V|
                 RME_RV64V_MMU_R|RME_RV64V_MMU_W|RME_RV64V_MMU_X|RME_RV64V_MMU_A|RME_RV64V_MMU_D;
    }

    return 0;
}
/* End Function:__RME_Pgt_Init *********************************************/

/* Function:__RME_Pgt_Del_Check *********************************************
Description : Check if the page table can be deleted.
Input       : struct RME_Cap_Pgt Pgt_Op* - The capability to the page table to operate on.
Output      : None.
Return      : rme_ret_t - If can be deleted, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
rme_ret_t __RME_Pgt_Del_Check(struct RME_Cap_Pgt* Pgt_Op)
{

    return 0;
}
/* End Function:__RME_Pgt_Del_Check ****************************************/

/* Function:__RME_Pgt_Page_Map **********************************************
Description : Map a page into the page table. This architecture requires that the mapping is
              always at least readable.
Input       : struct RME_Cap_Pgt* - The cap ability to the page table to operate on.
              rme_ptr_t Paddr - The physical address to map to. If we are unmapping, this have no effect.
              rme_ptr_t Pos - The position in the page table.
              rme_ptr_t Flags - The RME standard page attributes. Need to translate them into
                                architecture specific page table's settings.
Output      : None.
Return      : rme_ret_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
rme_ret_t __RME_Pgt_Page_Map(struct RME_Cap_Pgt* Pgt_Op, rme_ptr_t Paddr, rme_ptr_t Pos, rme_ptr_t Flags)
{
    rme_ptr_t* Table;
    rme_ptr_t RV64V_Flags;
    rme_ptr_t Szord;
    rme_ptr_t Vaddr;

    /* It should at least be readable (Sv39 also requires R=1 whenever W=1) */
    if((Flags&RME_PGT_READ)==0)
        return RME_ERR_HAL_FAIL;

    /* Are we trying to map into the kernel space on the top level? */
    if(((Pgt_Op->Base&RME_PGT_TOP)!=0)&&(Pos>=256))
        return RME_ERR_HAL_FAIL;

    /* The position must fit in this table and the address must be aligned to
     * the page size this table manages */
    Szord=RME_PGT_SZORD(Pgt_Op->Order);
    if((Pos>=RME_POW2(RME_PGT_NMORD(Pgt_Op->Order)))||
       ((Paddr&RME_MASK_END(Szord-1U))!=0U))
        return RME_ERR_HAL_FAIL;

    /* Get the table */
    Table=RME_CAP_GETOBJ(Pgt_Op,rme_ptr_t*);

    /* A Sv39 leaf has the same encoding at every level - the level is implied
     * by Size_Order, not by a PTE bit. The kernel half is refused above and is
     * pre-built from the template, so anything mapped here is a user page. */
    RV64V_Flags=RME_RV64V_MMU_PPN(Paddr)|RME_RV64V_PGFLG_RME2NAT(Flags)|RME_RV64V_MMU_U;

    /* Try to map it in */
    if(RME_COMP_SWAP(&(Table[Pos]),0,RV64V_Flags)==0)
        return RME_ERR_HAL_FAIL;

    /* Drop the old translation for this VA (all ASIDs) so the new mapping
     * takes effect on this hart immediately. */
    Vaddr=(Pgt_Op->Base&(~RME_PGT_TOP))+
          (((rme_ptr_t)Pos)<<RME_PGT_SZORD(Pgt_Op->Order));
    __asm__ volatile("sfence.vma %0, x0" : : "r"(Vaddr) : "memory");

    return 0;
}
/* End Function:__RME_Pgt_Page_Map *****************************************/

/* Function:__RME_Pgt_Page_Unmap ********************************************
Description : Unmap a page from the page table.
Input       : struct RME_Cap_Pgt* - The capability to the page table to operate on.
              rme_ptr_t Pos - The position in the page table.
Output      : None.
Return      : rme_ret_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
rme_ret_t __RME_Pgt_Page_Unmap(struct RME_Cap_Pgt* Pgt_Op, rme_ptr_t Pos)
{
    rme_ptr_t* Table;
    rme_ptr_t Temp;
    rme_ptr_t Vaddr;

    /* Are we trying to unmap the kernel space on the top level? */
    if(((Pgt_Op->Base&RME_PGT_TOP)!=0)&&(Pos>=256))
        return RME_ERR_HAL_FAIL;

    if(Pos>=RME_POW2(RME_PGT_NMORD(Pgt_Op->Order)))
        return RME_ERR_HAL_FAIL;

    /* Get the table */
    Table=RME_CAP_GETOBJ(Pgt_Op,rme_ptr_t*);

    /* Make sure that there is something */
    Temp=Table[Pos];
    if(Temp==0)
        return RME_ERR_HAL_FAIL;

    /* At the 4KB level every valid entry is a leaf. At the 1GB/2MB levels an
     * entry without R/W/X is a pointer to the next level table, which must not
     * be removed through the page interface. */
    if((RME_PGT_SZORD(Pgt_Op->Order)!=RME_PGT_SIZE_4K)&&((Temp&RME_RV64V_MMU_LEAF)==0))
        return RME_ERR_HAL_FAIL;

    /* Try to unmap it. Use CAS just in case */
    if(RME_COMP_SWAP(&(Table[Pos]),Temp,0)==0)
        return RME_ERR_HAL_FAIL;

    /* Drop the stale translation for this VA (all ASIDs). */
    Vaddr=(Pgt_Op->Base&(~RME_PGT_TOP))+
          (((rme_ptr_t)Pos)<<RME_PGT_SZORD(Pgt_Op->Order));
    __asm__ volatile("sfence.vma %0, x0" : : "r"(Vaddr) : "memory");

    return 0;
}
/* End Function:__RME_Pgt_Page_Unmap ***************************************/

/* Function:__RME_Pgt_Pgdir_Map *********************************************
Description : Map a page directory into the page table.
Input       : struct RME_Cap_Pgt* Pgt_Parent - The parent page table.
              struct RME_Cap_Pgt* Pgt_Child - The child page table.
              rme_ptr_t Pos - The position in the destination page table.
              rme_ptr_t Flags - The RME standard flags for the child page table.
Output      : None.
Return      : rme_ret_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
rme_ret_t __RME_Pgt_Pgdir_Map(struct RME_Cap_Pgt* Pgt_Parent, rme_ptr_t Pos,
                                struct RME_Cap_Pgt* Pgt_Child, rme_ptr_t Flags)
{
    rme_ptr_t* Parent_Table;
    rme_ptr_t* Child_Table;
    rme_ptr_t RV64V_Flags;

    /* It should at least be readable */
    if((Flags&RME_PGT_READ)==0)
        return RME_ERR_HAL_FAIL;

    /* Are we trying to map into the kernel space on the top level? */
    if(((Pgt_Parent->Base&RME_PGT_TOP)!=0)&&(Pos>=256))
        return RME_ERR_HAL_FAIL;

    if(Pos>=RME_POW2(RME_PGT_NMORD(Pgt_Parent->Order)))
        return RME_ERR_HAL_FAIL;

    /* Get the table */
    Parent_Table=RME_CAP_GETOBJ(Pgt_Parent,rme_ptr_t*);
    Child_Table=RME_CAP_GETOBJ(Pgt_Child,rme_ptr_t*);

    /* A Sv39 non-leaf entry carries V plus the PPN of the child table and MUST
     * NOT carry R/W/X (nor U - non-leaf U is reserved and must be zero),
     * otherwise hardware decodes it as a huge page leaf. The RME flags are
     * applied to the individual leaf pages instead. */
    RV64V_Flags=RME_RV64V_MMU_V|RME_RV64V_MMU_PPN(RME_RV64V_VA2PA(Child_Table));

    /* Try to map it in - may need to increase some count */
    if(RME_COMP_SWAP(&(Parent_Table[Pos]),0,RV64V_Flags)==0)
        return RME_ERR_HAL_FAIL;

    /* The child pointer covers a whole subtree; a full flush is the safe
     * conservative choice for this rare operation. */
    ___RME_RV64V_TLB_Flush();

    return 0;
}
/* End Function:__RME_Pgt_Pgdir_Map ****************************************/

/* Function:__RME_Pgt_Pgdir_Unmap *******************************************
Description : Unmap a page directory from the page table.
Input       : struct RME_Cap_Pgt* Pgt_Parent - The parent page table to unmap from.
              rme_ptr_t Pos - The position in the page table.
              struct RME_Cap_Pgt* Pgt_Child - The child page table to unmap.
Output      : None.
Return      : rme_ret_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
rme_ret_t __RME_Pgt_Pgdir_Unmap(struct RME_Cap_Pgt* Pgt_Parent, rme_ptr_t Pos,
                                  struct RME_Cap_Pgt* Pgt_Child)
{
    rme_ptr_t* Parent_Table;
    rme_ptr_t Temp;

    /* Are we trying to unmap the kernel space on the top level? */
    if(((Pgt_Parent->Base&RME_PGT_TOP)!=0)&&(Pos>=256))
        return RME_ERR_HAL_FAIL;

    if(Pos>=RME_POW2(RME_PGT_NMORD(Pgt_Parent->Order)))
        return RME_ERR_HAL_FAIL;

    /* Get the table */
    Parent_Table=RME_CAP_GETOBJ(Pgt_Parent,rme_ptr_t*);

    /* Make sure that there is something */
    Temp=Parent_Table[Pos];
    if(Temp==0)
        return RME_ERR_HAL_FAIL;

    /* The 4KB level cannot hold child tables, and a 1GB/2MB entry with R/W/X
     * set is a leaf page, not a child table pointer. */
    if((RME_PGT_SZORD(Pgt_Parent->Order)==RME_PGT_SIZE_4K)||((Temp&RME_RV64V_MMU_LEAF)!=0))
        return RME_ERR_HAL_FAIL;

    /* Is this child table the one mapped here? */
    if(RME_RV64V_MMU_ADDR(Temp)!=RME_RV64V_VA2PA(RME_CAP_GETOBJ(Pgt_Child,rme_ptr_t*)))
        return RME_ERR_HAL_FAIL;

    /* Try to unmap it. Use CAS just in case */
    if(RME_COMP_SWAP(&(Parent_Table[Pos]),Temp,0)==0)
        return RME_ERR_HAL_FAIL;

    /* The child pointer covers a whole subtree; a full flush is the safe
     * conservative choice for this rare operation. */
    ___RME_RV64V_TLB_Flush();

    return 0;
}
/* End Function:__RME_Pgt_Pgdir_Unmap **************************************/

/* Function:__RME_Pgt_Lookup **************************************************
Description : Lookup a page entry in a page directory.
Input       : struct RME_Cap_Pgt* Pgt_Op - The page directory to lookup.
              rme_ptr_t Pos - The position to look up.
Output      : rme_ptr_t* Paddr - The physical address of the page.
              rme_ptr_t* Flags - The RME standard flags of the page.
Return      : rme_ret_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
rme_ret_t __RME_Pgt_Lookup(struct RME_Cap_Pgt* Pgt_Op, rme_ptr_t Pos, rme_ptr_t* Paddr, rme_ptr_t* Flags)
{
    rme_ptr_t* Table;
    rme_ptr_t Temp;

    /* Check if the position is within the range of this page table */
    if((Pos>>RME_PGT_NMORD(Pgt_Op->Order))!=0)
        return RME_ERR_HAL_FAIL;

    /* Get the table */
    Table=RME_CAP_GETOBJ(Pgt_Op,rme_ptr_t*);
    /* Get the position requested - atomic read */
    Temp=Table[Pos];

    /* The entry must be valid. At the 4KB level every valid entry is a leaf;
     * at the 1GB/2MB levels it must carry R/W/X to be a huge page rather than
     * a pointer to the next level table. */
    if((Temp&RME_RV64V_MMU_V)==0)
        return RME_ERR_HAL_FAIL;

    if((RME_PGT_SZORD(Pgt_Op->Order)!=RME_PGT_SIZE_4K)&&((Temp&RME_RV64V_MMU_LEAF)==0))
        return RME_ERR_HAL_FAIL;

    /* This is a page. Return the physical address and flags */
    if(Paddr!=0)
        *Paddr=RME_RV64V_MMU_ADDR(Temp);

    if(Flags!=0)
        *Flags=RME_RV64V_PGFLG_NAT2RME(Temp);

    return 0;
}
/* End Function:__RME_Pgt_Lookup *******************************************/

/* Function:__RME_Pgt_Walk **************************************************
Description : Walking function for the page table. This function just does page
              table lookups. The page table that is being walked must be the top-
              level page table. The output values are optional; only pass in pointers
              when you need that value.
              Walking kernel page tables is prohibited.
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
rme_ret_t __RME_Pgt_Walk(struct RME_Cap_Pgt* Pgt_Op, rme_ptr_t Vaddr, rme_ptr_t* Pgt,
                           rme_ptr_t* Map_Vaddr, rme_ptr_t* Paddr, rme_ptr_t* Size_Order, rme_ptr_t* Num_Order, rme_ptr_t* Flags)
{
    rme_ptr_t* Table;
    rme_ptr_t Pos;
    rme_ptr_t Temp;
    rme_ptr_t Size_Cnt;

    /* Check if this is the top-level page table */
    if(((Pgt_Op->Base)&RME_PGT_TOP)==0)
        return RME_ERR_HAL_FAIL;

    /* Sv39's low canonical region ends at 2^38; this port never uses the
     * supervisor high half, so reject anything outside the low region. */
    if(Vaddr>=0x4000000000UL)
        return RME_ERR_HAL_FAIL;

    /* Get the table and start lookup - the root entry covers 1GB */
    Table=RME_CAP_GETOBJ(Pgt_Op, rme_ptr_t*);

    Size_Cnt=RME_PGT_SIZE_1G;

    while(1)
    {
        /* Calculate where is the entry - always 0 to 512 */
        Pos=(Vaddr>>Size_Cnt)&0x1FF;
        /* Atomic read */
        Temp=Table[Pos];
        /* Is the entry valid at all? */
        if((Temp&RME_RV64V_MMU_V)==0)
            return RME_ERR_HAL_FAIL;

        if((Temp&RME_RV64V_MMU_LEAF)!=0)
        {
            /* This is a page/huge page - we found it. Sv39 keeps the
             * permissions only in the leaf entry, there is nothing to
             * accumulate from the upper levels. */
            if(Pgt!=0)
                *Pgt=(rme_ptr_t)Table;
            if(Map_Vaddr!=0)
                *Map_Vaddr=RME_ROUND_DOWN(Vaddr,Size_Cnt);
            if(Paddr!=0)
                *Paddr=RME_RV64V_MMU_ADDR(Temp);
            if(Size_Order!=0)
                *Size_Order=Size_Cnt;
            if(Num_Order!=0)
                *Num_Order=RME_PGT_NUM_512;
            if(Flags!=0)
                *Flags=RME_RV64V_PGFLG_NAT2RME(Temp);

            break;
        }

        /* At the last level a non-leaf entry is malformed - fail rather than
         * reporting an unmapped page as present. */
        if(Size_Cnt==RME_PGT_SIZE_4K)
            return RME_ERR_HAL_FAIL;

        /* This is a directory, go to the child table to continue walking */
        Table=(rme_ptr_t*)RME_RV64V_PA2VA(RME_RV64V_MMU_ADDR(Temp));

        /* Each level down covers 512x less address space (1GB -> 2MB -> 4KB) */
        Size_Cnt-=RME_PGT_SIZE_512B;
    }

    return 0;
}
/* End Function:__RME_Pgt_Walk *********************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
