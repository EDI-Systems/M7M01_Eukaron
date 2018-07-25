/******************************************************************************
Filename    : rme_platform_c66x.c
Author      : pry
Date        : 01/04/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The hardware abstraction layer for TMS320C66X DSPs.
******************************************************************************/

/* Includes ******************************************************************/
#define __HDR_DEFS__
#include "Kernel/rme_kernel.h"
#include "Kernel/rme_kotbl.h"
#include "Kernel/rme_captbl.h"
#include "Kernel/rme_pgtbl.h"
#include "Kernel/rme_prcthd.h"
#include "Kernel/rme_siginv.h"
#include "Platform/C66X/rme_platform_c66x.h"
#undef __HDR_DEFS__

#define __HDR_STRUCTS__
#include "Platform/C66X/rme_platform_c66x.h"
#include "Kernel/rme_captbl.h"
#include "Kernel/rme_pgtbl.h"
#include "Kernel/rme_prcthd.h"
#include "Kernel/rme_siginv.h"
#undef __HDR_STRUCTS__

/* Private include */
#include "Platform/C66X/rme_platform_c66x.h"

#define __HDR_PUBLIC_MEMBERS__
#include "Kernel/rme_kernel.h"
#include "Kernel/rme_captbl.h"
#include "Kernel/rme_pgtbl.h"
#include "Kernel/rme_prcthd.h"
#include "Kernel/rme_siginv.h"
#undef __HDR_PUBLIC_MEMBERS__
/* End Includes **************************************************************/

/* Begin Function:__RME_Comp_Swap *********************************************
Description : The compare-and-swap atomic instruction. If the *Old value is equal to
              *Ptr, then set the *Ptr as New and return 1; else set the *Old as *Ptr,
              and return 0.
              TMS320C6678 does not have atomic instructions. Instead, it have something
              called hardware semaphore. We then are forced to use this as a memory
              monitor, try to take ticket locks to certain kernel memory segments
              when we access memory. This is how exactly the LDREX/STREX instructions
              get implemented in the CPU, so this should be fine, and we can still
              guarantee progress somewhere.
Input       : rme_ptr_t* Ptr - The pointer to the data.
              rme_ptr_t* Old - The old value.
              rme_ptr_t New - The new value.
Output      : rme_ptr_t* Ptr - The pointer to the data.
              rme_ptr_t* Old - The old value.
Return      : rme_ptr_t - If successful, 1; else 0.
******************************************************************************/
rme_ptr_t __RME_Comp_Swap(rme_ptr_t* Ptr, rme_ptr_t* Old, rme_ptr_t New)
{
    rme_ptr_t Pos;
    rme_ptr_t Ret;

    /* What semaphore is responsible for this range? */
    RME_C66X_SEM_POS((rme_ptr_t)Ptr,Pos);

    /* Indirect request */
    if(RME_READ_ACQUIRE((rme_ptr_t*)&RME_C66X_SEM_INDIRECT(Pos))!=1)
    {
        /* We have made the request, loop here until we are granted access.
         * The queue is a FIFO, thus the execution is still  */
        while(RME_C66X_SEM_DIRECT(Pos)!=1);
    }

    /* Granted. We proceed to compare-and-swap */
    if(*Ptr==*Old)
    {
        *Ptr=New;
        Ret=1;
    }
    else
    {
        *Old=*Ptr;
        Ret=0;
    }

    /* Release the semaphore as we are finished */
    RME_WRITE_RELEASE((rme_ptr_t*)&RME_C66X_SEM_DIRECT(Pos),1);
    return Ret;
}
/* End Function:__RME_Comp_Swap **********************************************/

/* Begin Function:__RME_Fetch_Add *********************************************
Description : The fetch-and-add atomic instruction. Increase the value that is 
              pointed to by the pointer, and return the value before addition.
              TMS320C6678 does not have atomic instructions. Instead, it have something
              called hardware semaphore. We then are forced to use this as a memory
              monitor, try to take ticket locks to certain kernel memory segments
              when we access memory. This is how exactly the LDREX/STREX instructions
              get implemented in the CPU, so this should be fine, and we can still
              guarantee progress somewhere.
Input       : rme_ptr_t* Ptr - The pointer to the data.
              rme_cnt_t Addend - The number to add.
Output      : rme_ptr_t* Ptr - The pointer to the data.
Return      : rme_ptr_t - The value before the addition.
******************************************************************************/
rme_ptr_t __RME_Fetch_Add(rme_ptr_t* Ptr, rme_cnt_t Addend)
{
    rme_ptr_t Pos;
    rme_ptr_t Old;

    /* What semaphore is responsible for this range? */
    RME_C66X_SEM_POS((rme_ptr_t)Ptr,Pos);

    /* Indirect request */
    if(RME_READ_ACQUIRE((rme_ptr_t*)&RME_C66X_SEM_INDIRECT(Pos))!=1)
    {
        /* We have made the request, loop here until we are granted access.
         * The queue is a FIFO, thus the execution is still  */
        while(RME_C66X_SEM_DIRECT(Pos)!=1);
    }

    /* Actual operation */
    Old=*Ptr;
    *Ptr+=Addend;

    /* Release the semaphore as we are finished */
    RME_WRITE_RELEASE((rme_ptr_t*)&RME_C66X_SEM_DIRECT(Pos),1);
    return Old;
}
/* End Function:__RME_Fetch_Add **********************************************/

/* Begin Function:__RME_Fetch_And *********************************************
Description : The fetch-and-logic-and atomic instruction. Logic AND the pointer
              value with the operand, and return the value before logic AND.
              TMS320C6678 does not have atomic instructions. Instead, it have something
              called hardware semaphore. We then are forced to use this as a memory
              monitor, try to take ticket locks to certain kernel memory segments
              when we access memory. This is how exactly the LDREX/STREX instructions
              get implemented in the CPU, so this should be fine, and we can still
              guarantee progress somewhere.
Input       : rme_ptr_t* Ptr - The pointer to the data.
              rme_cnt_t Operand - The number to logic AND with the destination.
Output      : rme_ptr_t* Ptr - The pointer to the data.
Return      : rme_ptr_t - The value before the AND operation.
******************************************************************************/
rme_ptr_t __RME_Fetch_And(rme_ptr_t* Ptr, rme_ptr_t Operand)
{
    rme_ptr_t Pos;
    rme_ptr_t Old;

    /* What semaphore is responsible for this range? */
    RME_C66X_SEM_POS((rme_ptr_t)Ptr,Pos);

    /* Indirect request */
    if(RME_READ_ACQUIRE((rme_ptr_t*)&RME_C66X_SEM_INDIRECT(Pos))!=1)
    {
        /* We have made the request, loop here until we are granted access.
         * The queue is a FIFO, thus the execution is still  */
        while(RME_C66X_SEM_DIRECT(Pos)!=1);
    }

    /* Actual operation */
    Old=*Ptr;
    *Ptr&=Operand;

    /* Release the semaphore as we are finished */
    RME_WRITE_RELEASE((rme_ptr_t*)&RME_C66X_SEM_DIRECT(Pos),1);
    return Old;
}
/* End Function:__RME_Fetch_And **********************************************/

/* Begin Function:__RME_Putchar ***********************************************
Description : Output a character to console. In Cortex-M, under most circumstances, 
              we should use the ITM for such outputs.
Input       : char Char - The character to print.
Output      : None.
Return      : rme_ptr_t - Always 0.
******************************************************************************/
rme_ptr_t __RME_Putchar(char Char)
{
    RME_C66X_PUTCHAR(Char);
    return 0;
}
/* End Function:__RME_Putchar ************************************************/

/* Begin Function:__RME_Low_Level_Init ****************************************
Description : Initialize the low-level hardware.
Input       : None.
Output      : None.
Return      : rme_ptr_t - Always 0.
******************************************************************************/
rme_ptr_t __RME_Low_Level_Init(void)
{
    rme_ptr_t Count;
    RME_C66X_LOW_LEVEL_INIT();

    /* UART initialization - Word length 8 */
    RME_C66X_UART_LCR=RME_C66X_UART_LCR_WLS8;
    /* Set the serial for 115200 - this should be good */
    RME_C66X_UART_DLL=RME_C66X_UART_BAUD&0xFF;
    RME_C66X_UART_DLH=RME_C66X_UART_BAUD>>8;
    /* Disable all interrupts, and no flow control */
    RME_C66X_UART_IER=0;
    RME_C66X_UART_MCR=0;
    /* Enable transmitter and free-running mode. receiver not enabled */
    RME_C66X_UART_PWREMU_MGMT=RME_C66X_MGMT_UTRST|RME_C66X_MGMT_FREE;
    /* Cleanup previous data (rx trigger is also set to 0) */
    RME_C66X_UART_FCR=RME_C66X_UART_FCR_FIFOEN|RME_C66X_UART_FCR_TXCLR;

    /* Core-local interrupt controller initialization - clear all flags first */
    RME_C66X_LIC_EVTCLR(0)=0xFFFFFFFFUL;
    RME_C66X_LIC_EVTCLR(1)=0xFFFFFFFFUL;
    RME_C66X_LIC_EVTCLR(2)=0xFFFFFFFFUL;
    RME_C66X_LIC_EVTCLR(3)=0xFFFFFFFFUL;
    /* Disable all interrupts permanently by setting the MUX to 0, which is never used */
    RME_C66X_LIC_INTMUX(1)=0;
    RME_C66X_LIC_INTMUX(2)=0;
    RME_C66X_LIC_INTMUX(3)=0;
    /* Clear all exception masks so that all of the events generate an exception */
    RME_C66X_LIC_EXPMASK(0)=0;
    RME_C66X_LIC_EXPMASK(1)=0;
    RME_C66X_LIC_EXPMASK(2)=0;
    RME_C66X_LIC_EXPMASK(3)=0;

    /* Initialize interrupt vector table */
    __RME_C66X_Set_ISTP((rme_ptr_t)__RME_C66X_Vector_Table);

    /* Program the timer 0 - we assume that it is always Fcorepac/6 */
    RME_C66X_TIM_CNTLO(0)=0;
    RME_C66X_TIM_CNTHI(0)=0;
    RME_C66X_TIM_PRDLO(0)=RME_C66X_SYSTICK_VAL;
    RME_C66X_TIM_PRDHI(0)=0;
    RME_C66X_TIM_RELLO(0)=RME_C66X_SYSTICK_VAL;
    RME_C66X_TIM_RELHI(0)=0;
    /* Take timer out of reset */
    RME_C66X_TIM_TGCR(0)=RME_C66X_TIM_TGCR_TIMHIRS|RME_C66X_TIM_TGCR_TIMLORS;
    RME_C66X_TIM_INTCTL(0)=RME_C66X_TIM_INTCTL_PRDINTEN_LO;
    /* Start to run the timer */
    RME_C66X_TIM_TCR(0)=RME_C66X_TIM_TCR_CONT;

    /* System interrupt controller initialization - disable all of them */
    for(Count=0;Count<4;Count++)
        RME_C66X_CIC_GER(Count)=0x00;

    /* Cache configuration - L1 preconfigured on some devices but we will configure again */
    RME_C66X_CACHE_L1PCFG=0x07;
    RME_C66X_CACHE_L1DCFG=0x07;
    RME_C66X_CACHE_L2CFG=0x07;
    /* All device memory is not cacheable not prefetchable - we can modify this later with kernel functions */
    for(Count=12;Count<128;Count++)
        RME_C66X_CACHE_L2MAR(Count)=0;
    /* All external memor is cacheable and prefetchable - we can modify this later with kernel functions */
    for(Count=128;Count<256;Count++)
        RME_C66X_CACHE_L2MAR(Count)=RME_C66X_CACHE_L2MAR_PFX|RME_C66X_CACHE_L2MAR_PC;

    return 0;
}
/* End Function:__RME_Low_Level_Init *****************************************/

/* Begin Function:main ********************************************************
Description : The entrance of the operating system. This function is for compatibility
              with the ARM toolchain.
Input       : None.
Output      : None.
Return      : int - This function never returns.
******************************************************************************/
int main(void)
{
    /* The main function of the kernel - we will start our kernel boot here */
    _RME_Kmain(RME_KMEM_STACK_ADDR);
}
/* End Function:main *********************************************************/

/* Begin Function:__RME_Boot **************************************************
Description : Boot the first process in the system.
Input       : None.
Output      : None.
Return      : rme_ptr_t - Always 0.
******************************************************************************/
rme_ptr_t __RME_Boot(void)
{
    rme_ptr_t Cur_Addr;
    

    return 0;
}
/* End Function:__RME_Boot ***************************************************/

/* Begin Function:__RME_Reboot ************************************************
Description : Reboot the machine, abandon all operating system states.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_Reboot(void)
{
    /* While(1) loop */
    RME_ASSERT(RME_WORD_BITS!=RME_POW2(RME_WORD_ORDER));
}
/* End Function:__RME_Reboot *************************************************/

/* Begin Function:__RME_Shutdown **********************************************
Description : Shutdown the machine, abandon all operating system states.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_Shutdown(void)
{
    /* While(1)loop */
    RME_ASSERT(RME_WORD_BITS!=RME_POW2(RME_WORD_ORDER));
}
/* End Function:__RME_Shutdown ***********************************************/

/* Begin Function:__RME_CPUID_Get *********************************************
Description : Get the CPUID. This is to identify where we are executing.
Input       : None.
Output      : None.
Return      : rme_ptr_t - The CPUID.
******************************************************************************/
rme_ptr_t __RME_CPUID_Get(void)
{
    return 0;
}
/* End Function:__RME_CPUID_Get **********************************************/

/* Begin Function:__RME_Get_Syscall_Param *************************************
Description : Get the system call parameters from the stack frame.
Input       : struct RME_Reg_Struct* Reg - The register set.
Output      : rme_ptr_t* Svc - The system service number.
              rme_ptr_t* Capid - The capability ID number.
              rme_ptr_t* Param - The parameters.
Return      : None.
******************************************************************************/
void __RME_Get_Syscall_Param(struct RME_Reg_Struct* Reg, rme_ptr_t* Svc, rme_ptr_t* Capid, rme_ptr_t* Param)
{
    *Svc=(Reg->A4)>>16;
    *Capid=(Reg->A4)&0xFFFF;
    Param[0]=Reg->B4;
    Param[1]=Reg->A6;
    Param[2]=Reg->B6;
}
/* End Function:__RME_Get_Syscall_Param **************************************/

/* Begin Function:__RME_Set_Syscall_Retval ************************************
Description : Set the system call return value to the stack frame. This function 
              may carry up to 4 return values. If the last 3 is not needed, just set
              them to zero.
Input       : rme_ret_t Retval - The return value.
Output      : struct RME_Reg_Struct* Reg - The register set.
Return      : None.
******************************************************************************/
void __RME_Set_Syscall_Retval(struct RME_Reg_Struct* Reg, rme_ret_t Retval)
{
    Reg->A4=(rme_ptr_t)Retval;
}
/* End Function:__RME_Set_Syscall_Retval *************************************/

/* Begin Function:__RME_Thd_Reg_Init ******************************************
Description : Initialize the register set for the thread.
Input       : rme_ptr_t Entry - The thread entry address.
              rme_ptr_t Stack - The thread stack address.
              rme_ptr_t Param - The parameter to pass.
Output      : struct RME_Reg_Struct* Reg - The register set content generated.
Return      : None.
******************************************************************************/
void __RME_Thd_Reg_Init(rme_ptr_t Entry, rme_ptr_t Stack, rme_ptr_t Param, struct RME_Reg_Struct* Reg)
{
    /* Entry point in NRP */
    Reg->NRP=Entry;
    /* Put something in the SP later */
    Reg->B15=Stack;
    /* Set the parameter */
    Reg->A4=Param;
}
/* End Function:__RME_Thd_Reg_Init *******************************************/

/* Begin Function:__RME_Thd_Reg_Copy ******************************************
Description : Copy one set of registers into another.
Input       : struct RME_Reg_Struct* Src - The source register set.
Output      : struct RME_Reg_Struct* Dst - The destination register set.
Return      : None.
******************************************************************************/
void __RME_Thd_Reg_Copy(struct RME_Reg_Struct* Dst, struct RME_Reg_Struct* Src)
{
    /* Don't worry, the TI compiler will optimize this into parallel operations */
    Dst->RILC=Src->RILC;
    Dst->NTSR=Src->NTSR;
    Dst->ILC=Src->ILC;
    Dst->CSR=Src->CSR;
    Dst->SSR=Src->SSR;
    Dst->NRP=Src->NRP;

    Dst->A0=Src->A0;
    Dst->A1=Src->A1;
    Dst->A2=Src->A2;
    Dst->A3=Src->A3;
    Dst->A4=Src->A4;
    Dst->A5=Src->A5;
    Dst->A6=Src->A6;
    Dst->A7=Src->A7;
    Dst->A8=Src->A8;
    Dst->A9=Src->A9;
    Dst->A10=Src->A10;
    Dst->A11=Src->A11;
    Dst->A12=Src->A12;
    Dst->A13=Src->A13;
    Dst->A16=Src->A16;
    Dst->A17=Src->A17;
    Dst->A18=Src->A18;
    Dst->A19=Src->A19;
    Dst->A20=Src->A20;
    Dst->A21=Src->A21;
    Dst->A22=Src->A22;
    Dst->A23=Src->A23;
    Dst->A24=Src->A24;
    Dst->A25=Src->A25;
    Dst->A26=Src->A26;
    Dst->A27=Src->A27;
    Dst->A28=Src->A28;
    Dst->A29=Src->A29;
    Dst->A30=Src->A30;
    Dst->A31=Src->A31;

    Dst->B0=Src->B0;
    Dst->B1=Src->B1;
    Dst->B2=Src->B2;
    Dst->B3=Src->B3;
    Dst->B4=Src->B4;
    Dst->B5=Src->B5;
    Dst->B6=Src->B6;
    Dst->B7=Src->B7;
    Dst->B8=Src->B8;
    Dst->B9=Src->B9;
    Dst->B10=Src->B10;
    Dst->B11=Src->B11;
    Dst->B12=Src->B12;
    Dst->B13=Src->B13;
    Dst->B16=Src->B16;
    Dst->B17=Src->B17;
    Dst->B18=Src->B18;
    Dst->B19=Src->B19;
    Dst->B20=Src->B20;
    Dst->B21=Src->B21;
    Dst->B22=Src->B22;
    Dst->B23=Src->B23;
    Dst->B24=Src->B24;
    Dst->B25=Src->B25;
    Dst->B26=Src->B26;
    Dst->B27=Src->B27;
    Dst->B28=Src->B28;
    Dst->B29=Src->B29;
    Dst->B30=Src->B30;
    Dst->B31=Src->B31;

    /* This is the data pointer */
    Dst->B14=Src->B14;
    /* This is the stack pointer */
    Dst->B15=Src->B15;
    Dst->A14=Src->A14;
    /* This is the frame pointer */
    Dst->A15=Src->A15;
}
/* End Function:__RME_Thd_Reg_Copy *******************************************/

/* Begin Function:__RME_Thd_Cop_Init ******************************************
Description : Initialize the coprocessor register set for the thread.
Input       : struct RME_Reg_Struct* Reg - The register struct to help initialize the coprocessor.
Output      : struct RME_Reg_Cop_Struct* Cop_Reg - The register set content generated.
Return      : None.
******************************************************************************/
void __RME_Thd_Cop_Init(struct RME_Reg_Struct* Reg, struct RME_Cop_Struct* Cop_Reg)
{
    Cop_Reg->AMR=0;
    Cop_Reg->GFPGFR=(0x07<<23)|(0x1D);
    Cop_Reg->GPLYA=0;
    Cop_Reg->GPLYB=0;
    Cop_Reg->FADCR=0;
    Cop_Reg->FAUCR=0;
    Cop_Reg->FMCR=0;
}
/* End Function:__RME_Thd_Cop_Reg_Init ***************************************/

/* Begin Function:__RME_Thd_Cop_Save ******************************************
Description : Save the co-op register sets. This operation is flexible - If the
              program does not use the FPU, we do not save its context.
Input       : struct RME_Reg_Struct* Reg - The context, used to decide whether
                                           to save the context of the coprocessor.
Output      : struct RME_Cop_Struct* Cop_Reg - The pointer to the coprocessor contents.
Return      : None.
******************************************************************************/
void __RME_Thd_Cop_Save(struct RME_Reg_Struct* Reg, struct RME_Cop_Struct* Cop_Reg)
{
    ___RME_C66X_Thd_Cop_Save(Cop_Reg);
}
/* End Function:__RME_Thd_Cop_Save *******************************************/

/* Begin Function:__RME_Thd_Cop_Restore ***************************************
Description : Restore the co-op register sets. This operation is flexible - If the
              FPU is not used, we do not restore its context.
Input       : struct RME_Reg_Struct* Reg - The context, used to decide whether
                                           to save the context of the coprocessor.
Output      : struct RME_Cop_Struct* Cop_Reg - The pointer to the coprocessor contents.
Return      : None.
******************************************************************************/
void __RME_Thd_Cop_Restore(struct RME_Reg_Struct* Reg, struct RME_Cop_Struct* Cop_Reg)
{    
    ___RME_C66X_Thd_Cop_Restore(Cop_Reg);
}
/* End Function:__RME_Thd_Cop_Restore ****************************************/

/* Begin Function:__RME_Inv_Reg_Save ******************************************
Description : Save the necessary registers on invocation for returning. Only the
              registers that will influence program control flow will be saved.
Input       : struct RME_Reg_Struct* Reg - The register set.
Output      : struct RME_Iret_Struct* Ret - The invocation return register context.
Return      : None.
******************************************************************************/
void __RME_Inv_Reg_Save(struct RME_Iret_Struct* Ret, struct RME_Reg_Struct* Reg)
{
    Ret->NRP=Reg->NRP;
    Ret->B15=Reg->B15;
}
/* End Function:__RME_Inv_Reg_Save *******************************************/

/* Begin Function:__RME_Inv_Reg_Restore ***************************************
Description : Restore the necessary registers for returning from an invocation.
Input       : struct RME_Iret_Struct* Ret - The invocation return register context.
Output      : struct RME_Reg_Struct* Reg - The register set.
Return      : None.
******************************************************************************/
void __RME_Inv_Reg_Restore(struct RME_Reg_Struct* Reg, struct RME_Iret_Struct* Ret)
{
    Reg->NRP=Ret->NRP;
    Reg->B15=Ret->B15;
}
/* End Function:__RME_Inv_Reg_Restore ****************************************/

/* Begin Function:__RME_Set_Inv_Retval ****************************************
Description : Set the invocation return value to the stack frame.
Input       : rme_ret_t Retval - The return value.
Output      : struct RME_Reg_Struct* Reg - The register set.
Return      : None.
******************************************************************************/
void __RME_Set_Inv_Retval(struct RME_Reg_Struct* Reg, rme_ret_t Retval)
{
    Reg->B4=Retval;
}
/* End Function:__RME_Set_Inv_Retval *****************************************/

/* Begin Function:__RME_Kern_Func_Handler *************************************
Description : Handle kernel function calls.
Input       : struct RME_Reg_Struct* Reg - The current register set.
              rme_ptr_t Func_ID - The function ID.
              rme_ptr_t Sub_ID - The subfunction ID.
              rme_ptr_t Param1 - The first parameter.
              rme_ptr_t Param2 - The second parameter.
Output      : None.
Return      : rme_ptr_t - The value that the function returned.
******************************************************************************/
rme_ptr_t __RME_Kern_Func_Handler(struct RME_Reg_Struct* Reg, rme_ptr_t Func_ID,
                                  rme_ptr_t Sub_ID, rme_ptr_t Param1, rme_ptr_t Param2)
{
    /* If it gets here, we must have failed */
    return RME_ERR_PGT_OPFAIL;
}
/* End Function:__RME_Kern_Func_Handler **************************************/

/* Begin Function:__RME_Pgtbl_Set *********************************************
Description : Set the processor's page table.
Input       : rme_ptr_t Pgtbl - The virtual address of the page table.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_Pgtbl_Set(rme_ptr_t Pgtbl)
{
    struct __RME_C66X_MMU_Data* MMU_Data;
    struct __RME_C66X_MMU_Entry* Region_Data;

    /* Get the address of the data section of this core */
    MMU_Data=(struct __RME_C66X_MMU_Data*)(Pgtbl+sizeof(struct __RME_C66X_Pgtbl_Meta));
    Region_Data=(struct __RME_C66X_MMU_Entry*)MMU_Data->Data[RME_CPUID()];

    /* The address to start to write into - the last region is always kernel entry.
     * The compiler will optimize this into highly efficient parallel assembly */
    RME_C66X_MMU_XMPAL(0)=Region_Data[0].MPAXL;
    RME_C66X_MMU_XMPAH(0)=Region_Data[0].MPAXH;
    RME_C66X_MMU_XMPAL(1)=Region_Data[1].MPAXL;
    RME_C66X_MMU_XMPAH(1)=Region_Data[1].MPAXH;
    RME_C66X_MMU_XMPAL(2)=Region_Data[2].MPAXL;
    RME_C66X_MMU_XMPAH(2)=Region_Data[2].MPAXH;
    RME_C66X_MMU_XMPAL(3)=Region_Data[3].MPAXL;
    RME_C66X_MMU_XMPAH(3)=Region_Data[3].MPAXH;
    RME_C66X_MMU_XMPAL(4)=Region_Data[4].MPAXL;
    RME_C66X_MMU_XMPAH(4)=Region_Data[4].MPAXH;
    RME_C66X_MMU_XMPAL(5)=Region_Data[5].MPAXL;
    RME_C66X_MMU_XMPAH(5)=Region_Data[5].MPAXH;
    RME_C66X_MMU_XMPAL(6)=Region_Data[6].MPAXL;
    RME_C66X_MMU_XMPAH(6)=Region_Data[6].MPAXH;
    RME_C66X_MMU_XMPAL(7)=Region_Data[7].MPAXL;
    RME_C66X_MMU_XMPAH(7)=Region_Data[7].MPAXH;
    RME_C66X_MMU_XMPAL(8)=Region_Data[8].MPAXL;
    RME_C66X_MMU_XMPAH(8)=Region_Data[8].MPAXH;
    RME_C66X_MMU_XMPAL(9)=Region_Data[9].MPAXL;
    RME_C66X_MMU_XMPAH(9)=Region_Data[9].MPAXH;
    RME_C66X_MMU_XMPAL(10)=Region_Data[10].MPAXL;
    RME_C66X_MMU_XMPAH(10)=Region_Data[10].MPAXH;
    RME_C66X_MMU_XMPAL(11)=Region_Data[11].MPAXL;
    RME_C66X_MMU_XMPAH(11)=Region_Data[11].MPAXH;
    RME_C66X_MMU_XMPAL(12)=Region_Data[12].MPAXL;
    RME_C66X_MMU_XMPAH(12)=Region_Data[12].MPAXH;
    RME_C66X_MMU_XMPAL(13)=Region_Data[13].MPAXL;
    RME_C66X_MMU_XMPAH(13)=Region_Data[13].MPAXH;
    RME_C66X_MMU_XMPAL(14)=Region_Data[14].MPAXL;
    RME_C66X_MMU_XMPAH(14)=Region_Data[14].MPAXH;
}
/* End Function:__RME_Pgtbl_Set **********************************************/

/* Begin Function:__RME_C66X_Fault_Handler ************************************
Description : The fault handler of RME. In C66X, this is used to handle multiple
              faults.
Input       : struct RME_Reg_Struct* Reg - The register set when entering the handler.
              rme_ptr_t Cause - The causing event of this fault.
Output      : struct RME_Reg_Struct* Reg - The register set when exiting the handler.
Return      : None.
******************************************************************************/
void __RME_C66X_Fault_Handler(struct RME_Reg_Struct* Reg, rme_ptr_t Cause)
{
    rme_ptr_t IERR_Reg;

    switch(Cause)
    {
        /* Internal exception, kill the thread now */
        case 0:
        {
            IERR_Reg=__RME_C66X_Get_IERR();
            RME_ASSERT(IERR_Reg==0);
            __RME_Thd_Fatal(Reg);
            break;
        }
        /* Memory management unit fault */
        case RME_C66X_EVT_MDMAERREVT:
        {
            /* Walk the page table to see if this address is allowed */
            break;
        }
        /* Memory protection unit faults - we kill the thread */
        default:
        {
            __RME_Thd_Fatal(Reg);
            break;
        }
    }
}
/* End Function:__RME_C66X_Fault_Handler *************************************/

/* Begin Function:__RME_C66X_Generic_Handler **********************************
Description : The generic interrupt handler of RME for C66X. In C66X, all interrupts are
              always disabled, and we will only use the NMI vector to do our job. This
              is due to the restriction that C66X allows user-level to disable interrupts,
              which makes the system totally unsafe. Also, we read the interrupt number
              when we are in this.
Input       : struct RME_Reg_Struct* Reg - The register set when entering the handler.
Output      : struct RME_Reg_Struct* Reg - The register set when exiting the handler.
Return      : None.
******************************************************************************/
void __RME_C66X_Generic_Handler(struct RME_Reg_Struct* Reg)
{
    rme_ptr_t EFR_Reg;
    rme_ptr_t Event_ID;
    rme_ptr_t Count;

    /* Is this a system call or something else? */
    EFR_Reg=__RME_C66X_Get_EFR();

    /* Is it a NMI? If yes, we have a hardware failure and cannot proceed anymore */
    RME_ASSERT((EFR_Reg&RME_C66X_EFR_NXF)==0);

    /* We service external interrupt first */
    if((EFR_Reg&RME_C66X_EFR_EXF)!=0)
    {
        /* What interrupt is it? */
        Event_ID=RME_C66X_LIC_INTXSTAT>>24;
        switch(Event_ID)
        {
            case RME_C66X_EVT_SYSTICK:
            {
                if(RME_CPUID()==0)
                {
                    /* Increase the tick */
                    _RME_Tick_Handler(Reg);
                    /* Send IPI to all other processors */
                    for(Count=1;Count<RME_CPU_NUM;Count++)
                        RME_C66X_DSC_IPCGR(Count)=0x01;
                }
                else
                    _RME_Tick_SMP_Handler(Reg);

                break;
            }
            /* MMU error */
            case RME_C66X_EVT_MDMAERREVT:
            /* Internal MPU errors */
            case RME_C66X_EVT_L1P_ED:
            case RME_C66X_EVT_L2_ED1:
            case RME_C66X_EVT_L2_ED2:
            case RME_C66X_EVT_PDC_INT:
            case RME_C66X_EVT_SYS_CMPA:
            case RME_C66X_EVT_L1P_CMPA:
            case RME_C66X_EVT_L1P_DMPA:
            case RME_C66X_EVT_L1D_CMPA:
            case RME_C66X_EVT_L1D_DMPA:
            case RME_C66X_EVT_L2_CMPA:
            case RME_C66X_EVT_L2_DMPA:
            case RME_C66X_EVT_EMC_CMPA:
            case RME_C66X_EVT_EMC_BUSERR:
            /* System MPU errors are disabled, and they silently ban any read/writes */
            {
                __RME_C66X_Fault_Handler(Reg,Event_ID);
                break;
            }
            /* Handle all other cases */
            default:
            {
                /* Unhandled for now */
                break;
            }
        }

        /* Clear the event flag */
        RME_C66X_LIC_EVTCLR(RME_C66X_EVT_SYSTICK/32)=(rme_ptr_t)1<<(RME_C66X_EVT_SYSTICK&0x1F);
        __RME_C66X_Set_ECR(EFR_Reg&(~RME_C66X_EFR_EXF));
        return;
    }

    /* Then check for internal errors */
    if((EFR_Reg&RME_C66X_EFR_IXF)!=0)
    {
        /* Whatever the error is, throw it towards the error handler */
        __RME_C66X_Fault_Handler(Reg,0);
        __RME_C66X_Set_ECR(EFR_Reg&(~RME_C66X_EFR_IXF));
        return;
    }

    /* We have a system call */
    if((EFR_Reg&RME_C66X_EFR_SXF)!=0)
    {
        _RME_Svc_Handler(Reg);
        __RME_C66X_Set_ECR(EFR_Reg&(~RME_C66X_EFR_SXF));
        return;
    }

    /* Can't have this situation, one of the bits must be set */
    RME_ASSERT(0);
}
/* End Function:__RME_C66X_Generic_Handler ***********************************/

/* Begin Function:__RME_Pgtbl_Kmem_Init ***************************************
Description : Initialize the kernel mapping tables, so it can be added to all the
              top-level page tables. In C66X, we do not need to add such pages.
Input       : None.
Output      : None.
Return      : rme_ptr_t - If successful, 0; else RME_ERR_PGT_OPFAIL.
******************************************************************************/
rme_ptr_t __RME_Pgtbl_Kmem_Init(void)
{
    /* Empty function, always immediately successful */
    return 0;
}
/* End Function:__RME_Pgtbl_Kmem_Init ****************************************/

/* Begin Function:__RME_Pgtbl_Check *******************************************
Description : Check if the page table parameters are feasible, according to the
              parameters. This is only used in page table creation. We will not
              check the start address because this is actually a MMU, but other
              restrictions when constructing page tables still apply.
Input       : rme_ptr_t Start_Addr - The start mapping address.
              rme_ptr_t Top_Flag - The top-level flag,
              rme_ptr_t Size_Order - The size order of the page directory.
              rme_ptr_t Num_Order - The number order of the page directory.
              rme_ptr_t Vaddr - The virtual address of the page directory.
Output      : None.
Return      : rme_ptr_t - If successful, 0; else RME_ERR_PGT_OPFAIL.
******************************************************************************/
rme_ptr_t __RME_Pgtbl_Check(rme_ptr_t Start_Addr, rme_ptr_t Top_Flag,
                            rme_ptr_t Size_Order, rme_ptr_t Num_Order, rme_ptr_t Vaddr)
{

    if(Num_Order<RME_PGTBL_NUM_2)
        return RME_ERR_PGT_OPFAIL;
    /* This is a policy in the kernel: we never allow creation of page tables of
     * this length because it can be disruptive to have such a big array */
    if(Num_Order>RME_PGTBL_NUM_1K)
        return RME_ERR_PGT_OPFAIL;
    /* Pages must be bigger than or equal to 4kB */
    if(Size_Order<RME_PGTBL_SIZE_4K)
        return RME_ERR_PGT_OPFAIL;
    /* Pages must be smaller than or equal to 1GB because we do not support PAE */
    if(Size_Order>RME_PGTBL_SIZE_1G)
        return RME_ERR_PGT_OPFAIL;
    /* Page directories must be aligned to a word */
    if((Vaddr&0x03)!=0)
        return RME_ERR_PGT_OPFAIL;
    /* If this is a top-level, it must cover 4G address range */
    if((Top_Flag!=0)&&((Num_Order+Size_Order)!=RME_PGTBL_SIZE_4G))
       return RME_ERR_PGT_OPFAIL;

    return 0;
}
/* End Function:__RME_Pgtbl_Check ********************************************/

/* Begin Function:__RME_Pgtbl_Init ********************************************
Description : Initialize the page table data structure, according to the capability.
Input       : struct RME_Cap_Pgtbl* - The capability to the page table to operate on.
Output      : None.
Return      : rme_ptr_t - If successful, 0; else RME_ERR_PGT_OPFAIL.
******************************************************************************/
rme_ptr_t __RME_Pgtbl_Init(struct RME_Cap_Pgtbl* Pgtbl_Op)
{
    rme_ptr_t* Ptr;
    rme_ptr_t CPU_Cnt;
    rme_ptr_t Count;

    /* Get the actual table */
    Ptr=RME_CAP_GETOBJ(Pgtbl_Op,rme_ptr_t*);

    /* Initialize the causal metadata */
    ((struct __RME_C66X_Pgtbl_Meta*)Ptr)->Size_Num_Order=Pgtbl_Op->Size_Num_Order;
    ((struct __RME_C66X_Pgtbl_Meta*)Ptr)->Parent_Cnt=0;
    ((struct __RME_C66X_Pgtbl_Meta*)Ptr)->Child_Cnt=0;
    Ptr+=sizeof(struct __RME_C66X_Pgtbl_Meta)/sizeof(rme_ptr_t);
    /* Is this a top-level? If it is, we need to clean up the MMU data */
    if(((Pgtbl_Op->Start_Addr)&RME_PGTBL_TOP)!=0)
    {
        ((struct __RME_C66X_MMU_Data*)Ptr)->State=0;

        for(CPU_Cnt=0;CPU_Cnt<RME_CPU_NUM;CPU_Cnt++)
        {
            for(Count=0;Count<15;Count++)
            {
                ((struct __RME_C66X_MMU_Data*)Ptr)->Data[CPU_Cnt][Count].MPAXH=0;
                ((struct __RME_C66X_MMU_Data*)Ptr)->Data[CPU_Cnt][Count].MPAXL=0;
            }
        }

        Ptr+=sizeof(struct __RME_C66X_MMU_Data)/sizeof(rme_ptr_t);
    }

    /* Clean up the table itself - This is could be virtually unbounded if the user
     * pass in some very large length value */
    for(Count=0;Count<RME_POW2(RME_PGTBL_NUMORD(Pgtbl_Op->Size_Num_Order));Count++)
        Ptr[Count]=0;

    return 0;
}
/* End Function:__RME_Pgtbl_Init *********************************************/

/* Begin Function:__RME_Pgtbl_Del_Check ***************************************
Description : Check if the page table can be deleted.
Input       : struct RME_Cap_Pgtbl Pgtbl_Op* - The capability to the page table to operate on.
Output      : None.
Return      : rme_ptr_t - If can be deleted, 0; else RME_ERR_PGT_OPFAIL.
******************************************************************************/
rme_ptr_t __RME_Pgtbl_Del_Check(struct RME_Cap_Pgtbl* Pgtbl_Op)
{
    struct __RME_C66X_Pgtbl_Meta* Meta;

    Meta=RME_CAP_GETOBJ(Pgtbl_Op,struct __RME_C66X_Pgtbl_Meta*);
    /* Check if it is mapped into other page tables. If yes, then it cannot be deleted.
     * also, it must not contain mappings of lower levels, or it is not deletable. */
    if((Meta->Parent_Cnt==0)&&(Meta->Child_Cnt==0))
        return 0;

    return RME_ERR_PGT_OPFAIL;
}
/* End Function:__RME_Pgtbl_Del_Check ****************************************/

/* Begin Function:__RME_Pgtbl_Page_Map ****************************************
Description : Map a page into the page table. If a page is mapped into the slot, the
              flags is actually placed on the metadata place because all pages are
              required to have the same flags. We take advantage of this to increase
              the page granularity.
              The C66X MPAX behaves like an MMU. It have physical address extension,
              however we will not seek to support that functionality here. For manually
              filled MMU, we do not pose any construction and destruction restrictions;
              however, the static pages are only guaranteed to be mapped in after a MMU
              miss. When pages are unmapped or page table deconstructed, the MMU metadata
              must be flushed, and it is the user-level's duty to do so.
Input       : struct RME_Cap_Pgtbl* - The cap ability to the page table to operate on.
              rme_ptr_t Paddr - The physical address to map to. If we are unmapping, this have no effect.
              rme_ptr_t Pos - The position in the page table.
              rme_ptr_t Flags - The RME standard page attributes. Need to translate them into
                            architecture specific page table's settings.
Output      : None.
Return      : rme_ptr_t - If successful, 0; else RME_ERR_PGT_OPFAIL.
******************************************************************************/
rme_ptr_t __RME_Pgtbl_Page_Map(struct RME_Cap_Pgtbl* Pgtbl_Op, rme_ptr_t Paddr, rme_ptr_t Pos, rme_ptr_t Flags)
{
    rme_ptr_t Temp;
    rme_ptr_t Entry;
    rme_ptr_t* Table;
    struct __RME_C66X_Pgtbl_Meta* Meta;

    /* Get the metadata */
    Meta=RME_CAP_GETOBJ(Pgtbl_Op,struct __RME_C66X_Pgtbl_Meta*);

    /* Where is the entry slot? */
    if(((Pgtbl_Op->Start_Addr)&RME_PGTBL_TOP)!=0)
        Table=RME_C66X_PGTBL_TBL_TOP((rme_ptr_t*)Meta);
    else
        Table=RME_C66X_PGTBL_TBL_NOM((rme_ptr_t*)Meta);

    /* Check if we are trying to make duplicate mappings into the same location */
    Temp=Table[Pos];
    if((Temp&RME_C66X_PGTBL_PRESENT)!=0)
        return RME_ERR_PGT_OPFAIL;

    /* Generate the contents of this entry */
    Entry=RME_C66X_PGTBL_PRESENT|RME_C66X_PGTBL_TERMINAL|(Flags<<2)|
          RME_ROUND_DOWN(Paddr,RME_PGTBL_SIZEORD(Pgtbl_Op->Size_Num_Order));

    /* Register into the page table¡£ We need a compare-and-swap here */
    if(__RME_Comp_Swap(&Table[Pos],&Temp,Entry)==0)
        return RME_ERR_PGT_OPFAIL;

    return 0;
}
/* End Function:__RME_Pgtbl_Page_Map *****************************************/

/* Begin Function:__RME_Pgtbl_Page_Unmap **************************************
Description : Unmap a page from the page table.
Input       : struct RME_Cap_Pgtbl* - The capability to the page table to operate on.
              rme_ptr_t Pos - The position in the page table.
Output      : None.
Return      : rme_ptr_t - If successful, 0; else RME_ERR_PGT_OPFAIL.
******************************************************************************/
rme_ptr_t __RME_Pgtbl_Page_Unmap(struct RME_Cap_Pgtbl* Pgtbl_Op, rme_ptr_t Pos)
{
    rme_ptr_t Temp;
    rme_ptr_t* Table;
    struct __RME_C66X_Pgtbl_Meta* Meta;

    /* Get the metadata */
    Meta=RME_CAP_GETOBJ(Pgtbl_Op,struct __RME_C66X_Pgtbl_Meta*);

    /* Where is the entry slot? */
    if(((Pgtbl_Op->Start_Addr)&RME_PGTBL_TOP)!=0)
        Table=RME_C66X_PGTBL_TBL_TOP((rme_ptr_t*)Meta);
    else
        Table=RME_C66X_PGTBL_TBL_NOM((rme_ptr_t*)Meta);

    /* Check if we are trying to remove air */
    Temp=Table[Pos];
    if((Temp&RME_C66X_PGTBL_PRESENT)==0)
        return RME_ERR_PGT_OPFAIL;

    /* Check if we are trying to remove a page directory */
    if((Temp&RME_C66X_PGTBL_TERMINAL)==0)
        return RME_ERR_PGT_OPFAIL;

    /* Use compare-and swap to evict the entry - the metadata problem is the user-level's,
     * not the kernel, and the kernel does not check quiescence at all here */
    if(__RME_Comp_Swap(&Table[Pos],&Temp,0)==0)
        return RME_ERR_PGT_OPFAIL;

    return 0;
}
/* End Function:__RME_Pgtbl_Page_Unmap ***************************************/

/* Begin Function:__RME_Pgtbl_Pgdir_Map ***************************************
Description : Map a page directory into the page table.
Input       : struct RME_Cap_Pgtbl* Pgtbl_Parent - The parent page table.
              struct RME_Cap_Pgtbl* Pgtbl_Child - The child page table.
              rme_ptr_t Pos - The position in the destination page table.
              rme_ptr_t Flags - This have no effect for manually filled MMU-based
                            architectures(because page table addresses use up
                            the whole word).
Output      : None.
Return      : rme_ptr_t - If successful, 0; else RME_ERR_PGT_OPFAIL.
******************************************************************************/
rme_ptr_t __RME_Pgtbl_Pgdir_Map(struct RME_Cap_Pgtbl* Pgtbl_Parent, rme_ptr_t Pos,
                                struct RME_Cap_Pgtbl* Pgtbl_Child, rme_ptr_t Flags)
{
    rme_ptr_t Temp;
    rme_ptr_t Entry;
    rme_ptr_t* Table;
    struct __RME_C66X_Pgtbl_Meta* Parent_Meta;
    struct __RME_C66X_Pgtbl_Meta* Child_Meta;

    /* Get the metadata */
    Parent_Meta=RME_CAP_GETOBJ(Pgtbl_Parent,struct __RME_C66X_Pgtbl_Meta*);
    Child_Meta=RME_CAP_GETOBJ(Pgtbl_Child,struct __RME_C66X_Pgtbl_Meta*);

    /* Where is the entry slot for the parent? */
    if(((Pgtbl_Parent->Start_Addr)&RME_PGTBL_TOP)!=0)
        Table=RME_C66X_PGTBL_TBL_TOP((rme_ptr_t*)Parent_Meta);
    else
        Table=RME_C66X_PGTBL_TBL_NOM((rme_ptr_t*)Parent_Meta);

    /* Check if we are trying to make duplicate mappings into the same location */
    Temp=Table[Pos];
    if((Temp&RME_C66X_PGTBL_PRESENT)!=0)
        return RME_ERR_PGT_OPFAIL;

    /* Generate the contents of this entry - what gets filled in here is kernel
     * virtual address, this is very different from hardware-lookup page tables, where
     * this is physical address! */
    Entry=RME_C66X_PGTBL_PRESENT|((rme_ptr_t)Child_Meta);

    /* Register into the page table¡£ We need a compare-and-swap here */
    if(__RME_Comp_Swap(&Table[Pos],&Temp,Entry)==0)
        return RME_ERR_PGT_OPFAIL;

    /* Fetch-and-add to parent&child counters */
    __RME_Fetch_Add(&(Parent_Meta->Child_Cnt),1);
    __RME_Fetch_Add(&(Child_Meta->Parent_Cnt),1);

    return 0;
}
/* End Function:__RME_Pgtbl_Pgdir_Map ****************************************/

/* Begin Function:__RME_Pgtbl_Pgdir_Unmap *************************************
Description : Unmap a page directory from the page table.
Input       : struct RME_Cap_Pgtbl* Pgtbl_Op - The page table to operate on.
              rme_ptr_t Pos - The position in the page table.
Output      : None.
Return      : rme_ptr_t - If successful, 0; else RME_ERR_PGT_OPFAIL.
******************************************************************************/
rme_ptr_t __RME_Pgtbl_Pgdir_Unmap(struct RME_Cap_Pgtbl* Pgtbl_Op, rme_ptr_t Pos)
{
    rme_ptr_t Temp;
    rme_ptr_t* Table;
    struct __RME_C66X_Pgtbl_Meta* Parent_Meta;
    struct __RME_C66X_Pgtbl_Meta* Child_Meta;

    /* Get the metadata */
    Parent_Meta=RME_CAP_GETOBJ(Pgtbl_Op,struct __RME_C66X_Pgtbl_Meta*);

    /* Where is the entry slot for the parent? */
    if(((Pgtbl_Op->Start_Addr)&RME_PGTBL_TOP)!=0)
        Table=RME_C66X_PGTBL_TBL_TOP((rme_ptr_t*)Parent_Meta);
    else
        Table=RME_C66X_PGTBL_TBL_NOM((rme_ptr_t*)Parent_Meta);

    /* Check if we are trying to remove air */
    Temp=Table[Pos];
    if((Temp&RME_C66X_PGTBL_PRESENT)==0)
        return RME_ERR_PGT_OPFAIL;

    /* Check if we are trying to remove a page */
    if((Temp&RME_C66X_PGTBL_TERMINAL)!=0)
        return RME_ERR_PGT_OPFAIL;

    /* Get the address of this entry */
    Child_Meta=(struct __RME_C66X_Pgtbl_Meta*)RME_C66X_PGTBL_PGD_ADDR(Temp);

    /* Use compare-and swap to evict the entry - the metadata problem is the user-level's,
     * not the kernel, and the kernel does not check quiescence at all here */
    if(__RME_Comp_Swap(&Table[Pos],&Temp,0)==0)
        return RME_ERR_PGT_OPFAIL;

    /* Fetch-and-add to parent&child counters */
    __RME_Fetch_Add(&(Parent_Meta->Child_Cnt),-1);
    __RME_Fetch_Add(&(Child_Meta->Parent_Cnt),-1);

    return 0;
}
/* End Function:__RME_Pgtbl_Pgdir_Unmap **************************************/

/* Begin Function:__RME_Pgtbl_Lookup ******************************************
Description : Lookup a page entry in a page directory.
Input       : struct RME_Cap_Pgtbl* Pgtbl_Op - The page directory to lookup.
              rme_ptr_t Pos - The position to look up.
Output      : rme_ptr_t* Paddr - The physical address of the page.
              rme_ptr_t* Flags - The RME standard flags of the page.
Return      : rme_ptr_t - If successful, 0; else RME_ERR_PGT_OPFAIL.
******************************************************************************/
rme_ptr_t __RME_Pgtbl_Lookup(struct RME_Cap_Pgtbl* Pgtbl_Op, rme_ptr_t Pos, rme_ptr_t* Paddr, rme_ptr_t* Flags)
{
    rme_ptr_t* Table;
    rme_ptr_t Temp;

    /* Check if the position is within the range of this page table */
    if((Pos>>RME_PGTBL_NUMORD(Pgtbl_Op->Size_Num_Order))!=0)
        return RME_ERR_PGT_OPFAIL;

    /* Check if this is the top-level page table. Get the table */
    if(((Pgtbl_Op->Start_Addr)&RME_PGTBL_TOP)!=0)
        Table=RME_C66X_PGTBL_TBL_TOP(RME_CAP_GETOBJ(Pgtbl_Op,rme_ptr_t*));
    else
        Table=RME_C66X_PGTBL_TBL_NOM(RME_CAP_GETOBJ(Pgtbl_Op,rme_ptr_t*));
    Temp=Table[Pos];

    /* Start lookup */
    if(((Temp&RME_C66X_PGTBL_PRESENT)==0)||((Temp&RME_C66X_PGTBL_TERMINAL)==0))
        return RME_ERR_PGT_OPFAIL;

    /* This is a page. Return the physical address and flags */
    if(Paddr!=0)
        *Paddr=RME_C66X_PGTBL_PTE_ADDR(Temp);

    if(Flags!=0)
        *Flags=RME_C66X_PGTBL_FLAG(Temp);

    return 0;
}
/* End Function:__RME_Pgtbl_Lookup *******************************************/

/* Begin Function:__RME_Pgtbl_Walk ********************************************
Description : Walking function for the page table. This function just does page
              table lookups. The page table that is being walked must be the top-
              level page table. The output values are optional; only pass in pointers
              when you need that value.
Input       : struct RME_Cap_Pgtbl* Pgtbl_Op - The page table to walk.
              rme_ptr_t Vaddr - The virtual address to look up.
Output      : rme_ptr_t* Pgtbl - The pointer to the page table level.
              rme_ptr_t* Map_Vaddr - The virtual address that starts mapping.
              rme_ptr_t* Paddr - The physical address of the page.
              rme_ptr_t* Size_Order - The size order of the page.
              rme_ptr_t* Num_Order - The entry order of the page.
              rme_ptr_t* Flags - The RME standard flags of the page.
Return      : rme_ptr_t - If successful, 0; else RME_ERR_PGT_OPFAIL.
******************************************************************************/
rme_ptr_t __RME_Pgtbl_Walk(struct RME_Cap_Pgtbl* Pgtbl_Op, rme_ptr_t Vaddr, rme_ptr_t* Pgtbl,
                           rme_ptr_t* Map_Vaddr, rme_ptr_t* Paddr, rme_ptr_t* Size_Order, rme_ptr_t* Num_Order, rme_ptr_t* Flags)
{
    struct __RME_C66X_Pgtbl_Meta* Meta;
    rme_ptr_t* Table;
    rme_ptr_t Pos;
    rme_ptr_t Temp;
    rme_ptr_t Size_Cnt;

    /* Check if this is the top-level page table */
    if(((Pgtbl_Op->Start_Addr)&RME_PGTBL_TOP)==0)
        return RME_ERR_PGT_OPFAIL;

    /* Are we attempting a kernel or peripheral lookup? If yes, stop immediately */
    if(Vaddr<RME_KMEM_VA_START+RME_KMEM_SIZE)
        return RME_ERR_PGT_OPFAIL;

    /* Get the table and start lookup */
    Meta=RME_CAP_GETOBJ(Pgtbl_Op,struct __RME_C66X_Pgtbl_Meta*);
    Table=RME_C66X_PGTBL_TBL_TOP((rme_ptr_t*)Meta);

    Size_Cnt=RME_C66X_PGTBL_SIZEORD(Meta->Size_Num_Order);
    /* Do lookup recursively */
    while(1)
    {
        /* Calculate where is the entry */
        Pos=(Vaddr>>Size_Cnt)&RME_MASK_END(RME_C66X_PGTBL_NUMORD(Meta->Size_Num_Order)-1);
        /* Atomic read */
        Temp=Table[Pos];
        /* Find the position of the entry - Is there a page, a directory, or nothing? */
        if((Temp&RME_C66X_PGTBL_PRESENT)==0)
            return RME_ERR_PGT_OPFAIL;
        if((Temp&RME_C66X_PGTBL_TERMINAL)!=0)
        {
            /* This is a page - we found it */
            if(Pgtbl!=0)
                *Pgtbl=(rme_ptr_t)Meta;
            if(Map_Vaddr!=0)
                *Map_Vaddr=RME_ROUND_DOWN(Vaddr,Size_Cnt);
            if(Paddr!=0)
                *Paddr=RME_C66X_PGTBL_PTE_ADDR(Temp);
            if(Size_Order!=0)
                *Size_Order=RME_C66X_PGTBL_SIZEORD(Meta->Size_Num_Order);
            if(Num_Order!=0)
                *Num_Order=RME_C66X_PGTBL_NUMORD(Meta->Size_Num_Order);
            if(Flags!=0)
                *Flags=RME_C66X_PGTBL_FLAG(Temp);

            break;
        }
        else
        {
            /* This is a directory, we goto that directory to continue walking */
            Meta=(struct __RME_C66X_Pgtbl_Meta*)RME_C66X_PGTBL_PGD_ADDR(Table[Pos]);
            Table=(rme_ptr_t*)RME_C66X_PGTBL_TBL_NOM((rme_ptr_t*)Meta);
            Size_Cnt-=RME_C66X_PGTBL_NUMORD(Meta->Size_Num_Order);
        }
    }

    return 0;
}
/* End Function:__RME_Pgtbl_Walk *********************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
