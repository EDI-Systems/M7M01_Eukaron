/******************************************************************************
Filename    : platform_template.c
Author      : pry
Date        : 01/04/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The hardware abstraction layer for the template.
******************************************************************************/

/* Includes ******************************************************************/
#define __HDR_DEFS__
#include "Kernel/kernel.h"
#include "Kernel/kotbl.h"
#include "Kernel/captbl.h"
#include "Kernel/pgtbl.h"
#include "Kernel/prcthd.h"
#include "Kernel/siginv.h"
#include "Platform/Template/platform_template.h"
#undef __HDR_DEFS__

#define __HDR_STRUCTS__
#include "Platform/Template/platform_template.h"
#include "Kernel/captbl.h"
#include "Kernel/pgtbl.h"
#include "Kernel/prcthd.h"
#include "Kernel/siginv.h"
#undef __HDR_STRUCTS__

/* Private include */
#include "Platform/Template/platform_template.h"

#define __HDR_PUBLIC_MEMBERS__
#include "Kernel/kernel.h"
#include "Kernel/captbl.h"
#include "Kernel/pgtbl.h"
#include "Kernel/prcthd.h"
#include "Kernel/siginv.h"
#undef __HDR_PUBLIC_MEMBERS__
/* End Includes **************************************************************/

/* Begin Function:__RME_Disable_Int *******************************************
Description     : The function for disabling all interrupts.
Input           : None.
Output          : None.    
Return          : None.                                  
******************************************************************************/    
void __RME_Disable_Int(void)
{
    
}                                               
/* End Function:__RME_Disable_Int ********************************************/

/* Begin Function:__RME_Enable_Int ********************************************
Description     : The Function For Enabling all interrupts.
Input           : None.
Output          : None.   
Return          : None.                                  
******************************************************************************/
void __RME_Disable_Int(void)
{
    
}
/* End Function:__RME_Enable_Int *********************************************/

/* Begin Function:__RME_Enable_Int ********************************************
Description     : The Function For Enabling all interrupts.
Input           : None.
Output          : None.   
Return          : None.                                  
******************************************************************************/
void __RME_Disable_Int(void)
{
    
}
/* End Function:__RME_Enable_Int *********************************************/

/* Begin Function:__RME_MSB_Get ***********************************************
Description     : Get the MSB of the word.
Input           : ptr_t Val - The value.
Output          : None.
Return          : ptr_t - The MSB position.
******************************************************************************/
ptr_t __RME_MSB_Get(ptr_t Val)
{
    
}
/* End Function:__RME_MSB_Get ************************************************/

/* Begin Function:_RME_Kmain **************************************************
Description     : The entry address of the kernel. Never returns.
Input           : ptr_t Stack - The stack address to set SP to.
Output          : None.
Return          : None. 
******************************************************************************/
void _RME_Kmain(ptr_t Stack)
{

}
/* End Function:_RME_Kmain ***************************************************/

/* Begin Function:__RME_Enter_User_Mode ***************************************
Description  : Entering of the user mode, after the system finish its preliminary
               booting. The function shall never return. This function should only
               be used to boot the first process in the system.
Input        : ptr_t Entry_Addr - The user execution startpoint.
               ptr_t Stack_Addr - The user stack.
Output       : None.
Return       : None.                      
;*****************************************************************************/
void __RME_Enter_User_Mode(ptr_t Entry_Addr, ptr_t Stack_Addr)
{
    
}
/* End Function:__RME_Enter_User_Mode ****************************************/

/* Begin Function:__RME_Comp_Swap *********************************************
Description : The compare-and-swap atomic instruction. If the *Old value is equal to
              *Ptr, then set the *Ptr as New and return 1; else set the *Old as *Ptr,
              and return 0.
Input       : ptr_t* Ptr - The pointer to the data.
              ptr_t* Old - The old value.
              ptr_t New - The new value.
Output      : ptr_t* Ptr - The pointer to the data.
              ptr_t* Old - The old value.
Return      : ptr_t - If successful, 1; else 0.
******************************************************************************/
ptr_t __RME_Comp_Swap(ptr_t* Ptr, ptr_t* Old, ptr_t New)
{
    return 0;
}
/* End Function:__RME_Comp_Swap **********************************************/

/* Begin Function:__RME_Fetch_Add *********************************************
Description : The fetch-and-add atomic instruction. Increase the value that is 
              pointed to by the pointer, and return the value before addition.
Input       : ptr_t* Ptr - The pointer to the data.
              cnt_t Addend - The number to add.
Output      : ptr_t* Ptr - The pointer to the data.
Return      : ptr_t - The value before the addition.
******************************************************************************/
ptr_t __RME_Fetch_Add(ptr_t* Ptr, cnt_t Addend)
{
    return 0;
}
/* End Function:__RME_Fetch_Add **********************************************/

/* Begin Function:__RME_Fetch_And *********************************************
Description : The fetch-and-logic-and atomic instruction. Logic AND the pointer
              value with the operand, and return the value before logic AND.
              On Cortex-M there is only one core. There's basically no need to do
              anything special.
Input       : ptr_t* Ptr - The pointer to the data.
              cnt_t Operand - The number to logic AND with the destination.
Output      : ptr_t* Ptr - The pointer to the data.
Return      : ptr_t - The value before the AND operation.
******************************************************************************/
ptr_t __RME_Fetch_And(ptr_t* Ptr, ptr_t Operand)
{
    return 0;
}
/* End Function:__RME_Fetch_And **********************************************/

/* Begin Function:__RME_Putchar ***********************************************
Description : Output a character to console.
Input       : char Char - The character to print.
Output      : None.
Return      : ptr_t - Always 0.
******************************************************************************/
ptr_t __RME_Putchar(char Char)
{
    return 0;
}
/* End Function:__RME_Putchar ************************************************/

/* Begin Function:__RME_Low_Level_Init ****************************************
Description : Initialize the low-level hardware. 
Input       : None.
Output      : None.
Return      : ptr_t - Always 0.
******************************************************************************/
ptr_t __RME_Low_Level_Init(void)
{
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
Return      : ptr_t - Always 0.
******************************************************************************/
ptr_t __RME_Boot(void)
{
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

}
/* End Function:__RME_Shutdown ***********************************************/

/* Begin Function:__RME_CPUID_Get *********************************************
Description : Get the CPUID. This is to identify where we are executing.
Input       : None.
Output      : None.
Return      : ptr_t - The CPUID. On Cortex-M, this is certainly always 0.
******************************************************************************/
ptr_t __RME_CPUID_Get(void)
{

}
/* End Function:__RME_CPUID_Get **********************************************/

/* Begin Function:__RME_Get_Syscall_Param *************************************
Description : Get the system call parameters from the stack frame.
Input       : struct RME_Reg_Struct* Reg - The register set.
Output      : ptr_t* Svc - The system service number.
              ptr_t* Capid - The capability ID number.
              ptr_t* Param - The parameters.
Return      : ptr_t - Always 0.
******************************************************************************/
ptr_t __RME_Get_Syscall_Param(struct RME_Reg_Struct* Reg, ptr_t* Svc, ptr_t* Capid, ptr_t* Param)
{
    return 0;
}
/* End Function:__RME_Get_Syscall_Param **************************************/

/* Begin Function:__RME_Set_Syscall_Retval ************************************
Description : Set the system call return value to the stack frame.
Input       : ret_t Retval - The return value.
Output      : struct RME_Reg_Struct* Reg - The register set.
Return      : ptr_t - Always 0.
******************************************************************************/
ptr_t __RME_Set_Syscall_Retval(struct RME_Reg_Struct* Reg, ret_t Retval)
{
    return 0;
}
/* End Function:__RME_Set_Syscall_Retval *************************************/

/* Begin Function:__RME_Get_Inv_Retval ****************************************
Description : Get the invocation return value to the stack frame.
Input       : struct RME_Reg_Struct* Reg - The register set.
Output      : None.
Return      : ptr_t - The return value.
******************************************************************************/
ptr_t __RME_Get_Inv_Retval(struct RME_Reg_Struct* Reg)
{
    return 0;
}
/* End Function:__RME_Get_Inv_Retval *****************************************/

/* Begin Function:__RME_Set_Inv_Retval ****************************************
Description : Set the invocation return value to the stack frame.
Input       : ret_t Retval - The return value.
Output      : struct RME_Reg_Struct* Reg - The register set.
Return      : ptr_t - Always 0.
******************************************************************************/
ptr_t __RME_Set_Inv_Retval(struct RME_Reg_Struct* Reg, ret_t Retval)
{
    return 0;
}
/* End Function:__RME_Set_Inv_Retval *****************************************/

/* Begin Function:__RME_Thd_Reg_Init ******************************************
Description : Initialize the register set for the thread.
Input       : ptr_t Entry_Addr - The thread entry address.
              ptr_t Stack_Addr - The thread stack address.
Output      : struct RME_Reg_Struct* Reg - The register set content generated.
Return      : ptr_t - Always 0.
******************************************************************************/
ptr_t __RME_Thd_Reg_Init(ptr_t Entry_Addr, ptr_t Stack_Addr, struct RME_Reg_Struct* Reg)
{
    return 0;
}
/* End Function:__RME_Thd_Reg_Init *******************************************/

/* Begin Function:__RME_Thd_Reg_Copy ******************************************
Description : Copy one set of registers into another.
Input       : struct RME_Reg_Struct* Src - The source register set.
Output      : struct RME_Reg_Struct* Dst - The destination register set.
Return      : ptr_t - Always 0.
******************************************************************************/
ptr_t __RME_Thd_Reg_Copy(struct RME_Reg_Struct* Dst, struct RME_Reg_Struct* Src)
{
    return 0;
}
/* End Function:__RME_Thd_Reg_Copy *******************************************/

/* Begin Function:__RME_Thd_Cop_Init ******************************************
Description : Initialize the coprocessor register set for the thread.
Input       : ptr_t Entry_Addr - The thread entry address.
              ptr_t Stack_Addr - The thread stack address.
Output      : struct RME_Reg_Cop_Struct* Cop_Reg - The register set content generated.
Return      : ptr_t - Always 0.
******************************************************************************/
ptr_t __RME_Thd_Cop_Init(ptr_t Entry_Addr, ptr_t Stack_Addr, struct RME_Cop_Struct* Cop_Reg)
{
    return 0;
}
/* End Function:__RME_Thd_Cop_Reg_Init ***************************************/

/* Begin Function:__RME_Thd_Cop_Save ******************************************
Description : Save the co-op register sets. This operation is flexible - If the
              program does not use the FPU, we do not save its context.
Input       : struct RME_Reg_Struct* Reg - The context, used to decide whether
                                           to save the context of the coprocessor.
Output      : s struct RME_Cop_Struct* Cop_Reg - The pointer to the coprocessor contents.
Return      : ptr_t - Always 0.
******************************************************************************/
ptr_t __RME_Thd_Cop_Save(struct RME_Reg_Struct* Reg, struct RME_Cop_Struct* Cop_Reg)
{
    return 0;
}
/* End Function:__RME_Thd_Cop_Save *******************************************/

/* Begin Function:__RME_Thd_Cop_Restore ***************************************
Description : Restore the co-op register sets. This operation is flexible - If the
              FPU is not used, we do not restore its context.
Input       : struct RME_Reg_Struct* Reg - The context, used to decide whether
                                           to save the context of the coprocessor.
Output      : s struct RME_Cop_Struct* Cop_Reg - The pointer to the coprocessor contents.
Return      : ptr_t - Always 0.
******************************************************************************/
ptr_t __RME_Thd_Cop_Restore(struct RME_Reg_Struct* Reg, struct RME_Cop_Struct* Cop_Reg)
{
    return 0;
}
/* End Function:__RME_Thd_Cop_Restore ****************************************/

/* Begin Function:__RME_Inv_Reg_Init ******************************************
Description : Initialize the register set for the invocation.
Input       : ptr_t Param - The parameter.
Output      : struct RME_Reg_Struct* Reg - The register set content generated.
Return      : ptr_t - Always 0.
******************************************************************************/
ptr_t __RME_Inv_Reg_Init(ptr_t Param, struct RME_Reg_Struct* Reg)
{
    return 0;
}
/* End Function:__RME_Inv_Reg_Init *******************************************/

/* Begin Function:__RME_Inv_Cop_Init ******************************************
Description : Initialize the coprocessor register set for the invocation.
Input       : ptr_t Param - The parameter.
Output      : struct RME_Reg_Struct* Reg - The register set content generated.
Return      : ptr_t - Always 0.
******************************************************************************/
ptr_t __RME_Inv_Cop_Init(ptr_t Param, struct RME_Cop_Struct* Cop_Reg)
{
    return 0;
}
/* End Function:__RME_Inv_Cop_Init *******************************************/

/* Begin Function:__RME_Kern_Func_Handler *************************************
Description : Initialize the coprocessor register set for the invocation.
Input       : struct RME_Reg_Struct* Reg - The current register set.
              ptr_t Func_ID - The function ID.
              ptr_t Param1 - The first parameter.
              ptr_t Param2 - The second parameter.
Output      : None.
Return      : ptr_t - The value that the function returned.
******************************************************************************/
ptr_t __RME_Kern_Func_Handler(struct RME_Reg_Struct* Reg, ptr_t Func_ID,
                              ptr_t Param1, ptr_t Param2)
{
    return 0;
}
/* End Function:__RME_Kern_Func_Handler **************************************/

/* Begin Function:__RME_Pgtbl_Set *********************************************
Description : Set the processor's page table.
Input       : ptr_t Pgtbl - The virtual address of the page table.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_Pgtbl_Set(ptr_t Pgtbl)
{

}
/* End Function:__RME_Pgtbl_Set **********************************************/

/* Begin Function:__RME_Pgtbl_Kmem_Init ***************************************
Description : Initialize the kernel mapping tables, so it can be added to all the
              top-level page tables.
Input       : None.
Output      : None.
Return      : ptr_t - If successful, 0; else RME_ERR_PGT_OPFAIL.
******************************************************************************/
ptr_t __RME_Pgtbl_Kmem_Init(void)
{
    return 0;
}
/* End Function:__RME_Pgtbl_Kmem_Init ****************************************/

/* Begin Function:__RME_Pgtbl_Check *******************************************
Description : Check if the page table parameters are feasible, according to the
              parameters. This is only used in page table creation.
Input       : ptr_t Start_Addr - The start mapping address.
              ptr_t Top_Flag - The top-level flag,
              ptr_t Size_Order - The size order of the page directory.
              ptr_t Num_Order - The number order of the page directory.
Output      : None.
Return      : ptr_t - If successful, 0; else RME_ERR_PGT_OPFAIL.
******************************************************************************/
ptr_t __RME_Pgtbl_Check(ptr_t Start_Addr, ptr_t Top_Flag, ptr_t Size_Order, ptr_t Num_Order)
{   
    return 0;
}
/* End Function:__RME_Pgtbl_Check ********************************************/

/* Begin Function:__RME_Pgtbl_Init ********************************************
Description : Initialize the page table data structure, according to the capability.
Input       : struct RME_Cap_Pgtbl* - The capability to the page table to operate on.
Output      : None.
Return      : ptr_t - If successful, 0; else RME_ERR_PGT_OPFAIL.
******************************************************************************/
ptr_t __RME_Pgtbl_Init(struct RME_Cap_Pgtbl* Pgtbl_Op)
{   
    return 0;
}
/* End Function:__RME_Pgtbl_Init *********************************************/

/* Begin Function:__RME_Pgtbl_Del_Check ***************************************
Description : Check if the page table can be deleted.
Input       : struct RME_Cap_Pgtbl Pgtbl_Op* - The capability to the page table to operate on.
Output      : None.
Return      : ptr_t - If can be deleted, 0; else RME_ERR_PGT_OPFAIL.
******************************************************************************/
ptr_t __RME_Pgtbl_Del_Check(struct RME_Cap_Pgtbl* Pgtbl_Op)
{
    return 0;
}
/* End Function:__RME_Pgtbl_Del_Check ****************************************/

/* Begin Function:__RME_Pgtbl_Page_Map ****************************************
Description : Map a page into the page table.
Input       : struct RME_Cap_Pgtbl* - The capability to the page table to operate on.
              ptr_t Paddr - The physical address to map to. If we are unmapping, this have no effect.
              ptr_t Pos - The position in the page table.
              ptr_t Flags - The RME standard page attributes. Need to translate them into 
                            architecture specific page table's settings.
Output      : None.
Return      : ptr_t - If successful, 0; else RME_ERR_PGT_OPFAIL.
******************************************************************************/
ptr_t __RME_Pgtbl_Page_Map(struct RME_Cap_Pgtbl* Pgtbl_Op, ptr_t Paddr, ptr_t Pos, ptr_t Flags)
{
    return 0;
}
/* End Function:__RME_Pgtbl_Page_Map *****************************************/

/* Begin Function:__RME_Pgtbl_Page_Unmap **************************************
Description : Unmap a page from the page table.
Input       : struct RME_Cap_Pgtbl* - The capability to the page table to operate on.
              ptr_t Pos - The position in the page table.
Output      : None.
Return      : ptr_t - If successful, 0; else RME_ERR_PGT_OPFAIL.
******************************************************************************/
ptr_t __RME_Pgtbl_Page_Unmap(struct RME_Cap_Pgtbl* Pgtbl_Op, ptr_t Pos)
{
    return 0;
}
/* End Function:__RME_Pgtbl_Page_Unmap ***************************************/

/* Begin Function:__RME_Pgtbl_Pgdir_Map ***************************************
Description : Map a page directory into the page table.
Input       : struct RME_Cap_Pgtbl* Pgtbl_Parent - The parent page table.
              struct RME_Cap_Pgtbl* Pgtbl_Child - The child page table.
              ptr_t Pos - The position in the destination page table.
Output      : None.
Return      : ptr_t - If successful, 0; else RME_ERR_PGT_OPFAIL.
******************************************************************************/
ptr_t __RME_Pgtbl_Pgdir_Map(struct RME_Cap_Pgtbl* Pgtbl_Parent, ptr_t Pos, 
                            struct RME_Cap_Pgtbl* Pgtbl_Child)
{
    return 0;
}
/* End Function:__RME_Pgtbl_Pgdir_Map ****************************************/

/* Begin Function:__RME_Pgtbl_Pgdir_Unmap *************************************
Description : Unmap a page directory from the page table.
Input       : struct RME_Cap_Pgtbl* Pgtbl_Op - The page table to operate on.
              ptr_t Pos - The position in the page table.
Output      : None.
Return      : ptr_t - If successful, 0; else RME_ERR_PGT_OPFAIL.
******************************************************************************/
ptr_t __RME_Pgtbl_Pgdir_Unmap(struct RME_Cap_Pgtbl* Pgtbl_Op, ptr_t Pos)
{
    return 0;
}
/* End Function:__RME_Pgtbl_Pgdir_Unmap **************************************/

/* Begin Function:__RME_Pgtbl_Lookup ********************************************
Description : Lookup a page entry in a page directory.
Input       : struct RME_Cap_Pgtbl* Pgtbl_Op - The page directory to lookup.
              ptr_t Pos - The position to look up.
Output      : ptr_t* Paddr - The physical address of the page.
              ptr_t* Flags - The RME standard flags of the page.
Return      : ptr_t - If successful, 0; else RME_ERR_PGT_OPFAIL.
******************************************************************************/
ptr_t __RME_Pgtbl_Lookup(struct RME_Cap_Pgtbl* Pgtbl_Op, ptr_t Pos, ptr_t* Paddr, ptr_t* Flags)
{
    return 0;
}
/* End Function:__RME_Pgtbl_Lookup *******************************************/

/* Begin Function:__RME_Pgtbl_Walk ********************************************
Description : Walking function for the page table. This function just does page
              table lookups. The page table that is being walked must be the top-
              level page table. The output values are optional; only pass in pointers
              when you need that value.
Input       : struct RME_Cap_Pgtbl* Pgtbl_Op - The page table to walk.
              ptr_t Vaddr - The virtual address to look up.
Output      : ptr_t* Pgtbl - The pointer to the page table level.
              ptr_t* Map_Vaddr - The virtual address that starts mapping.
              ptr_t* Paddr - The physical address of the page.
              ptr_t* Size_Order - The size order of the page.
              ptr_t* Num_Order - The entry order of the page.
              ptr_t* Flags - The RME standard flags of the page.
Return      : ptr_t - If successful, 0; else RME_ERR_PGT_OPFAIL.
******************************************************************************/
ptr_t __RME_Pgtbl_Walk(struct RME_Cap_Pgtbl* Pgtbl_Op, ptr_t Vaddr, ptr_t* Pgtbl,
                       ptr_t* Map_Vaddr, ptr_t* Paddr, ptr_t* Size_Order, ptr_t* Num_Order, ptr_t* Flags)
{
    return 0;
}
/* End Function:__RME_Pgtbl_Walk *********************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
