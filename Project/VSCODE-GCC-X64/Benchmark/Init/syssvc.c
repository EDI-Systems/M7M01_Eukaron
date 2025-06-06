/******************************************************************************
Filename    : syssvc.c
Author      : pry
Date        : 11/06/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The system call wrapper of UVM VMM.
******************************************************************************/

/* Includes ******************************************************************/
#include "rme.h"

#define __HDR_DEFS__
#include "Platform/UVM_platform.h"
#include "Init/syssvc.h"
#undef __HDR_DEFS__

#define __HDR_STRUCTS__
#include "Platform/UVM_platform.h"
#include "Init/syssvc.h"
#undef __HDR_STRUCTS__

/* Private include */
#include "Init/syssvc.h"

#define __HDR_PUBLIC_MEMBERS__
#include "Platform/UVM_platform.h"
#undef __HDR_PUBLIC_MEMBERS__
/* End Includes **************************************************************/

/* Begin Function:UVM_List_Crt ************************************************
Description : Create a doubly linkled list.
Input       : volatile struct UVM_List* Head - The pointer to the list head.
Output      : None.
Return      : None.
******************************************************************************/
void UVM_List_Crt(volatile struct UVM_List* Head)
{
    Head->Prev=(struct UVM_List*)Head;
    Head->Next=(struct UVM_List*)Head;
}
/* End Function:UVM_List_Crt *************************************************/

/* Begin Function:UVM_List_Del ************************************************
Description : Delete a node from the doubly-linked list.
Input       : volatile struct UVM_List* Prev - The prevoius node of the target node.
              volatile struct UVM_List* Next - The next node of the target node.
Output      : None.
Return      : None.
******************************************************************************/
void UVM_List_Del(volatile struct UVM_List* Prev,volatile struct UVM_List* Next)
{
    Next->Prev=(struct UVM_List*)Prev;
    Prev->Next=(struct UVM_List*)Next;
}
/* End Function:UVM_List_Del *************************************************/

/* Begin Function:UVM_List_Ins ************************************************
Description : Insert a node to the doubly-linked list.
Input       : volatile struct UVM_List* New - The new node to insert.
              volatile struct UVM_List* Prev - The previous node.
              volatile struct UVM_List* Next - The next node.
Output      : None.
Return      : None.
******************************************************************************/
void UVM_List_Ins(volatile struct UVM_List* New,
                  volatile struct UVM_List* Prev,
                  volatile struct UVM_List* Next)
{
    Next->Prev=(struct UVM_List*)New;
    New->Next=(struct UVM_List*)Next;
    New->Prev=(struct UVM_List*)Prev;
    Prev->Next=(struct UVM_List*)New;
}
/* End Function:UVM_List_Ins *************************************************/

/* Begin Function:UVM_Print_Int ***********************************************
Description : Print a signed integer on the debugging console. This integer is
              printed as decimal with sign.
Input       : cnt_t Int - The integer to print.
Output      : None.
Return      : cnt_t - The length of the string printed.
******************************************************************************/
cnt_t UVM_Print_Int(cnt_t Int)
{
    ptr_t Iter;
    cnt_t Count;
    cnt_t Num;
    ptr_t Div;

    /* how many digits are there? */
    if(Int==0)
    {
        UVM_Putchar('0');
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
        
        UVM_Putchar('-');
        Iter=-Int;
        Num=Count+1;
        
        while(Count>0)
        {
            Count--;
            UVM_Putchar(Iter/Div+'0');
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
            UVM_Putchar(Iter/Div+'0');
            Iter=Iter%Div;
            Div/=10;
        }
    }
    
    return Num;
}
/* End Function:UVM_Print_Int ************************************************/

/* Begin Function:UVM_Print_Uint **********************************************
Description : Print a unsigned integer on the debugging console. This integer is
              printed as hexadecimal.
Input       : ptr_t Uint - The unsigned integer to print.
Output      : None.
Return      : cnt_t - The length of the string printed.
******************************************************************************/
cnt_t UVM_Print_Uint(ptr_t Uint)
{
    ptr_t Iter;
    cnt_t Count;
    cnt_t Num;
    
    /* how many digits are there? */
    if(Uint==0)
    {
        UVM_Putchar('0');
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
                UVM_Putchar('0'+Iter);
            else
                UVM_Putchar('A'+Iter-10);
        }
    }
    
    return Num;
}
/* End Function:UVM_Print_Uint ***********************************************/

/* Begin Function:UVM_Print_String ********************************************
Description : Print a string on the debugging console.
              This is only used for user-level debugging.
Input       : s8* String - The string to print.
Output      : None.
Return      : cnt_t - The length of the string printed, the '\0' is not included.
******************************************************************************/
cnt_t UVM_Print_String(s8* String)
{
    cnt_t Count;
    
    Count=0;
    while(Count<UVM_USER_DEBUG_MAX_STR)
    {
        if(String[Count]=='\0')
            break;
        
        UVM_Putchar(String[Count++]);
    }

    return Count;
}
/* End Function:UVM_Print_String *********************************************/

/* Begin Function:UVM_Captbl_Crt **********************************************
Description : Create a capability table.
Input       : cid_t Cap_Captbl_Crt - The capability table that contains the newly 
                                     created cap to captbl. 2-Level.
              cid_t Cap_Kmem - The kernel memory capability. 2-Level.
              cid_t Cap_Crt - The capability slot that you want this newly created
                              capability table capability to be in. 1-Level.
              ptr_t Vaddr - The physical address to store the kernel data.
              ptr_t Entry_Num - The number of entries in that capability table.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Captbl_Crt(cid_t Cap_Captbl_Crt, cid_t Cap_Kmem, 
                     cid_t Cap_Captbl, ptr_t Vaddr, ptr_t Entry_Num)
{
    return UVM_CAP_OP(RME_SVC_CPT_CRT, Cap_Captbl_Crt,
                      UVM_PARAM_D1(Cap_Kmem)|UVM_PARAM_D0(Cap_Captbl),
                      Vaddr,
                      Entry_Num);
}
/* End Function:UVM_Captbl_Crt ***********************************************/

/* Begin Function:UVM_Captbl_Del **********************************************
Description : Delete a layer of capability table. The table will be deleted regardless
              of whether there are still capabilities in it.
Input       : cid_t Cap_Captbl_Del - The capability table that contains the cap to 
                                     captbl. 2-Level.
              cid_t Cap_Del - The capability to the capability table to be deleted. 1-Level.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Captbl_Del(cid_t Cap_Captbl_Del, cid_t Cap_Del)
{
    return UVM_CAP_OP(RME_SVC_CPT_DEL, Cap_Captbl_Del,
                      Cap_Del,
                      0,
                      0);
}
/* End Function:UVM_Captbl_Del ***********************************************/

/* Begin Function:UVM_Captbl_Frz **********************************************
Description : Freeze a capability in the capability table.
Input       : cid_t Cap_Captbl_Frz  - The capability table containing the cap to
                                      captbl for this operation. 2-Level.
              cid_t Cap_Frz - The cap to the kernel object being freezed. 1-Level.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Captbl_Frz(cid_t Cap_Captbl_Frz, cid_t Cap_Frz)
{
        return UVM_CAP_OP(RME_SVC_CPT_FRZ, Cap_Captbl_Frz,
                          Cap_Frz,
                          0,
                          0);
}
/* End Function:UVM_Captbl_Frz ***********************************************/

/* Begin Function:UVM_Captbl_Add **********************************************
Description : Add one capability into the capability table. This is doing a 
              capability delegation. This function should only be used to delegate
              capabilities other than kernel memory, kernel function and page
              tables.
Input       : cid_t Cap_Captbl_Dst - The capability to the destination capability table.
                                     2-Level.
              cid_t Cap_Dst - The capability slot you want to add to. 1-Level.
              cid_t Cap_Captbl_Src - The capability to the source capability table.
                                     2-Level.
              cid_t Cap_Src - The capability in the source capability table to delegate.
                              1-Level.
              ptr_t Flags - The flags for the capability.
Output      : None.
Return      : ret_t - If the mapping is successful, it will return 0; else error code.
******************************************************************************/
ret_t UVM_Captbl_Add(cid_t Cap_Captbl_Dst, cid_t Cap_Dst, 
                     cid_t Cap_Captbl_Src, cid_t Cap_Src, ptr_t Flags)
{
    return UVM_CAP_OP(RME_SVC_CPT_ADD, 0,
                      UVM_PARAM_D1(Cap_Captbl_Dst)|UVM_PARAM_D0(Cap_Dst),
                      UVM_PARAM_D1(Cap_Captbl_Src)|UVM_PARAM_D0(Cap_Src),Flags);
}
/* End Function:UVM_Captbl_Add ***********************************************/

/* Begin Function:UVM_Captbl_Pgtbl ********************************************
Description : Add one capability into the capability table. This is doing a 
              capability delegation. This function should only be used to delegate
              page tables.
Input       : cid_t Cap_Captbl_Dst - The capability to the destination capability table.
                                     2-Level.
              cid_t Cap_Dst - The capability slot you want to add to. 1-Level.
              cid_t Cap_Captbl_Src - The capability to the source capability table.
                                     2-Level.
              cid_t Cap_Src - The capability in the source capability table to delegate.
                              1-Level.
              ptr_t Start - The start position of the page table entry.
              ptr_t End - The end position of the page table entry.
              ptr_t Flags - The flags for the page table capability.
Output      : None.
Return      : ret_t - If the mapping is successful, it will return 0; else error code.
******************************************************************************/
ret_t UVM_Captbl_Pgtbl(cid_t Cap_Captbl_Dst, cid_t Cap_Dst, 
                       cid_t Cap_Captbl_Src, cid_t Cap_Src,
                       ptr_t Start, ptr_t End, ptr_t Flags)
{
    return UVM_CAP_OP(RME_SVC_CPT_ADD, 0,
                      UVM_PARAM_D1(Cap_Captbl_Dst)|UVM_PARAM_D0(Cap_Dst),
                      UVM_PARAM_D1(Cap_Captbl_Src)|UVM_PARAM_D0(Cap_Src),
                      UVM_PGTBL_FLAG(End,Start,Flags));
}
/* End Function:UVM_Captbl_Pgtbl *********************************************/

/* Begin Function:UVM_Captbl_Kern *********************************************
Description : Add one capability into the capability table. This is doing a 
              capability delegation. This function should only be used to delegate
              kernel functions.
Input       : cid_t Cap_Captbl_Dst - The capability to the destination capability table.
                                     2-Level.
              cid_t Cap_Dst - The capability slot you want to add to. 1-Level.
              cid_t Cap_Captbl_Src - The capability to the source capability table.
                                     2-Level.
              cid_t Cap_Src - The capability in the source capability table to delegate.
                              1-Level.
              ptr_t Start - The start of the kernel function ID.
              ptr_t End - The end of the kernel function ID.
Output      : None.
Return      : ret_t - If the mapping is successful, it will return 0; else error code.
******************************************************************************/
ret_t UVM_Captbl_Kern(cid_t Cap_Captbl_Dst, cid_t Cap_Dst, 
                      cid_t Cap_Captbl_Src, cid_t Cap_Src,
                      ptr_t Start, ptr_t End)
{
    return UVM_CAP_OP(RME_SVC_CPT_ADD, 0,
                      UVM_PARAM_D1(Cap_Captbl_Dst)|UVM_PARAM_D0(Cap_Dst),
                      UVM_PARAM_D1(Cap_Captbl_Src)|UVM_PARAM_D0(Cap_Src),
                      UVM_KERN_FLAG(End,Start));
}
/* End Function:UVM_Captbl_Kern **********************************************/

/* Begin Function:UVM_Captbl_Kmem *********************************************
Description : Add one capability into the capability table. This is doing a 
              capability delegation. This function should only be used to delegate
              kernel memory.
Input       : cid_t Cap_Captbl_Dst - The capability to the destination capability table.
                                     2-Level.
              cid_t Cap_Dst - The capability slot you want to add to. 1-Level.
              cid_t Cap_Captbl_Src - The capability to the source capability table.
                                     2-Level.
              cid_t Cap_Src - The capability in the source capability table to delegate.
                              1-Level.
              ptr_t Start - The start address of the kernel memory.
              ptr_t End - The end address of the kernel memory.
              ptr_t Flags - The flags for the kernel memory capability.
Output      : None.
Return      : ret_t - If the mapping is successful, it will return 0; else error code.
******************************************************************************/
ret_t UVM_Captbl_Kmem(cid_t Cap_Captbl_Dst, cid_t Cap_Dst, 
                      cid_t Cap_Captbl_Src, cid_t Cap_Src,
                      ptr_t Start, ptr_t End, ptr_t Flags)
{
    return UVM_CAP_OP(UVM_KMEM_SVC(End,RME_SVC_CPT_ADD), UVM_KMEM_CAPID(Start,Flags),
                      UVM_PARAM_D1(Cap_Captbl_Dst)|UVM_PARAM_D0(Cap_Dst),
                      UVM_PARAM_D1(Cap_Captbl_Src)|UVM_PARAM_D0(Cap_Src),
                      UVM_KMEM_FLAG(End,Start));
}
/* End Function:UVM_Captbl_Kmem **********************************************/

/* Begin Function:UVM_Captbl_Rem **********************************************
Description : Remove one capability from the capability table. This function reverts
              the delegation.
Input       : cid_t Cap_Captbl_Rem - The capability to the capability table to remove from.
                                     2-Level.
              cid_t Cap_Rem - The capability slot you want to remove. 1-Level.
Output      : None.
Return      : ret_t - If the mapping is successful, it will return 0; else error code.
******************************************************************************/
ret_t UVM_Captbl_Rem(cid_t Cap_Captbl_Rem, cid_t Cap_Rem)
{
    return UVM_CAP_OP(RME_SVC_CPT_REM, Cap_Captbl_Rem,
                      Cap_Rem,
                      0,
                      0);
}
/* End Function:_UVM_Captbl_Rem **********************************************/

/* Begin Function:UVM_Kern_Act ************************************************
Description : Activate a kernel function.
Input       : cid_t Cap_Kern - The capability to the kernel capability. 2-Level.
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
ret_t UVM_Kern_Act(cid_t Cap_Kern, ptr_t Func_ID, ptr_t Sub_ID, ptr_t Param1, ptr_t Param2)
{
    return UVM_CAP_OP(RME_SVC_KFN, Cap_Kern,
                      UVM_PARAM_D1(Sub_ID)|UVM_PARAM_D0(Func_ID),
                      Param1,
                      Param2);
}
/* End Function:UVM_Kern_Act *************************************************/

/* Begin Function:UVM_Pgtbl_Crt ***********************************************
Description : Create a layer of page table, and put that capability into a designated
              capability table. The function will check if the memory region is large
              enough, and how it has been typed; if the function found out that it is
              impossible to create a page table due to some error, it will return an
              error.
Input       : cid_t Cap_Captbl - The capability table that contains the newly created 
                                 cap to pgtbl. 2-Level.
              cid_t Cap_Kmem - The kernel memory capability. 2-Level.
              cid_t Cap_Pgtbl - The capability slot that you want this newly created
                                page table capability to be in. 1-Level.
              ptr_t Vaddr - The physical address to store the page table. This must fall
                            within the kernel virtual address.
              ptr_t Start_Addr - The virtual address to start mapping for this page table.  
                                This address must be aligned to the total size of the table.
              ptr_t Top_Flag - Whether this page table is the top-level. If it is, we will
                               map all the kernel page directories into this one.
              ptr_t Size_Order - The size order of the page table. The size refers to
                                 the size of each page in the page directory.
              ptr_t Num_Order - The number order of entries in the page table.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Pgtbl_Crt(cid_t Cap_Captbl, cid_t Cap_Kmem, cid_t Cap_Pgtbl, 
                    ptr_t Vaddr, ptr_t Start_Addr, ptr_t Top_Flag,
                    ptr_t Size_Order, ptr_t Num_Order)
{
    
    return UVM_CAP_OP(UVM_PGTBL_SVC(Num_Order,RME_SVC_PGT_CRT), Cap_Captbl,
                      UVM_PARAM_D1(Cap_Kmem)|UVM_PARAM_Q1(Cap_Pgtbl)|UVM_PARAM_Q0(Size_Order),
                      Vaddr, 
                      Start_Addr|Top_Flag);
}
/* End Function:UVM_Pgtbl_Crt ************************************************/

/* Begin Function:UVM_Pgtbl_Del ***********************************************
Description : Delete a layer of page table. We do not care if the childs are all
              deleted. For MPU based environments, it is required that all the 
              mapped child page tables are deconstructed from the master table
              before we can destroy the master table.
Input       : cid_t Cap_Captbl - The capability to the captbl that may contain the cap
                                 to new captbl. 2-Level.
              cid_t Cap_Pgtbl - The capability slot that you want this newly created
                                page table capability to be in. 1-Level.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Pgtbl_Del(cid_t Cap_Captbl, cid_t Cap_Pgtbl)
{
    return UVM_CAP_OP(RME_SVC_PGT_DEL, Cap_Captbl,
                      Cap_Pgtbl,
                      0,
                      0);
}
/* End Function:UVM_Pgtbl_Del ************************************************/

/* Begin Function:UVM_Pgtbl_Add ***********************************************
Description : Delegate a page from one page table to another. This is the only way
              to add pages to new page tables after the system boots.
Input       : cid_t Cap_Pgtbl_Dst - The capability to the destination page directory. 2-Level.
              ptr_t Pos_Dst - The position to delegate to in the destination page directory.
              ptr_t Flags_Dst - The page access permission for the destination page. This is
                                not to be confused with the flags for the capabilities for
                                page tables!
              cid_t Cap_Pgtbl_Src - The capability to the source page directory. 2-Level.
              ptr_t Pos_Dst - The position to delegate from in the source page directory.
              ptr_t Index - The index of the physical address frame to delegate.
                            For example, if the destination directory's page size is 1/4
                            of that of the source directory, index=0 will delegate the first
                            1/4, index=1 will delegate the second 1/4, index=2 will delegate
                            the third 1/4, and index=3 will delegate the last 1/4.
                            All other index values are illegal.
Output      : None.
Return      : ret_t - If the unmapping is successful, it will return 0; else error code.
******************************************************************************/
ret_t UVM_Pgtbl_Add(cid_t Cap_Pgtbl_Dst, ptr_t Pos_Dst, ptr_t Flags_Dst,
                    cid_t Cap_Pgtbl_Src, ptr_t Pos_Src, ptr_t Index)
{
    return UVM_CAP_OP(RME_SVC_PGT_ADD, Flags_Dst,
                      UVM_PARAM_D1(Cap_Pgtbl_Dst)|UVM_PARAM_D0(Pos_Dst),
                      UVM_PARAM_D1(Cap_Pgtbl_Src)|UVM_PARAM_D0(Pos_Src),
                      Index);
}
/* End Function:UVM_Pgtbl_Add ************************************************/

/* Begin Function:UVM_Pgtbl_Rem ***********************************************
Description : Remove a page from the page table. We are doing unmapping of a page.
Input       : cid_t Cap_Pgtbl - The capability to the page table. 2-Level.
              ptr_t Pos - The virtual address position to unmap from.
Output      : None.
Return      : ret_t - If the unmapping is successful, it will return 0; else error code.
******************************************************************************/
ret_t UVM_Pgtbl_Rem(cid_t Cap_Pgtbl, ptr_t Pos)
{
    return UVM_CAP_OP(RME_SVC_PGT_REM, 0,
                      Cap_Pgtbl,
                      Pos,
                      0);
}
/* End Function:UVM_Pgtbl_Rem ************************************************/

/* Begin Function:UVM_Pgtbl_Con ***********************************************
Description : Map a child page table from the parent page table. Basically, we 
              are doing the construction of a page table.
Input       : cid_t Cap_Pgtbl_Parent - The capability to the parent page table. 2-Level.
              ptr_t Pos - The virtual address to position map the child page table to.
              cid_t Cap_Pgtbl_Child - The capability to the child page table. 2-Level.
              ptr_t Flags_Child - This have no effect on MPU-based architectures.
Output      : None.
Return      : ret_t - If the mapping is successful, it will return 0; else error code.
******************************************************************************/
ret_t UVM_Pgtbl_Con(cid_t Cap_Pgtbl_Parent, ptr_t Pos, cid_t Cap_Pgtbl_Child, ptr_t Flags_Child)
{
    return UVM_CAP_OP(RME_SVC_PGT_CON, 0,
                      UVM_PARAM_D1(Cap_Pgtbl_Parent)|UVM_PARAM_D0(Cap_Pgtbl_Child),
                      Pos,
                      Flags_Child);
}
/* End Function:UVM_Pgtbl_Con ************************************************/

/* Begin Function:UVM_Pgtbl_Des ***********************************************
Description : Unmap a child page table from the parent page table. Basically, we 
              are doing the destruction of a page table.
Input       : cid_t Cap_Pgtbl - The capability to the page table. 2-Level.
              ptr_t Pos - The virtual address to position unmap the child page
                          table from.
Output      : None.
Return      : ret_t - If the mapping is successful, it will return 0; else error code.
******************************************************************************/
ret_t UVM_Pgtbl_Des(cid_t Cap_Pgtbl, ptr_t Pos)
{
    return UVM_CAP_OP(RME_SVC_PGT_DES, 0,
                      Cap_Pgtbl,
                      Pos,
                      0);
}
/* End Function:UVM_Pgtbl_Des ************************************************/

/* Begin Function:UVM_Proc_Crt ************************************************
Description : Create a process. A process is in fact a protection domain associated
              with a set of capabilities.
Input       : cid_t Cap_Captbl_Crt - The capability to the capability table to put
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
ret_t UVM_Proc_Crt(cid_t Cap_Captbl_Crt, cid_t Cap_Kmem, cid_t Cap_Proc,
                   cid_t Cap_Captbl, cid_t Cap_Pgtbl, ptr_t Vaddr)
{
    return UVM_CAP_OP(RME_SVC_PRC_CRT, Cap_Captbl_Crt,
                      Cap_Proc,
                      Cap_Captbl,
                      Cap_Pgtbl);
}
/* End Function:UVM_Proc_Crt *************************************************/

/* Begin Function:UVM_Proc_Del ************************************************
Description : Delete a process.
Input       : cid_t Cap_Captbl - The capability to the capability table. 2-Level.
              cid_t Cap_Proc - The capability to the process. 1-Level.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Proc_Del(cid_t Cap_Captbl, cid_t Cap_Proc)
{
    return UVM_CAP_OP(RME_SVC_PRC_DEL, Cap_Captbl,
                      Cap_Proc,
                      0,
                      0);
}
/* End Function:UVM_Proc_Del *************************************************/

/* Begin Function:UVM_Proc_Cpt ***********************************************
Description : Assign or change a process's capability table.
Input       : cid_t Cap_Proc - The capability to the process that have been created
                               already. 2-Level.
              cid_t Cap_Captbl - The capability to the capability table to use for
                                 this process. 2-Level.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Proc_Cpt(cid_t Cap_Proc, cid_t Cap_Captbl)
{
    return UVM_CAP_OP(RME_SVC_PRC_CPT, 0,
                      Cap_Proc,
                      Cap_Captbl,
                      0);
}
/* End Function:UVM_Proc_Cpt *************************************************/

/* Begin Function:UVM_Proc_Pgt ************************************************
Description : Assign or change a process's page table.
Input       : cid_t Cap_Proc - The capability slot that you want this newly created
                               process capability to be in. 2-Level.
              cid_t Cap_Pgtbl - The capability to the page table to use for this
                                process. 2-Level.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Proc_Pgt(cid_t Cap_Proc, cid_t Cap_Pgtbl)
{
    return UVM_CAP_OP(RME_SVC_PRC_PGT, 0,
                      Cap_Proc,
                      Cap_Pgtbl,
                      0);
}
/* End Function:UVM_Proc_Pgt *************************************************/

/* Begin Function:UVM_Thd_Crt *************************************************
Description : Create a thread. A thread is the minimal kernel-level execution unit.
Input       : cid_t Cap_Captbl - The capability to the capability table. 2-Level.
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
ret_t UVM_Thd_Crt(cid_t Cap_Captbl, cid_t Cap_Kmem, cid_t Cap_Thd,
                  cid_t Cap_Proc, ptr_t Max_Prio, ptr_t Vaddr)
{
    return UVM_CAP_OP(RME_SVC_THD_CRT, Cap_Captbl, 
                      UVM_PARAM_D1(Cap_Kmem)|UVM_PARAM_D0(Cap_Thd),
                      UVM_PARAM_D1(Cap_Proc)|UVM_PARAM_D0(Max_Prio),
                      Vaddr);
}
/* End Function:UVM_Thd_Crt **************************************************/

/* Begin Function:UVM_Thd_Del *************************************************
Description : Delete a thread.
Input       : cid_t Cap_Captbl - The capability to the capability table. 2-Level.
              cid_t Cap_Thd - The capability to the thread in the captbl. 1-Level.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Thd_Del(cid_t Cap_Captbl, cid_t Cap_Thd)
{
    return UVM_CAP_OP(RME_SVC_THD_DEL, Cap_Captbl,
                      Cap_Thd,
                      0,
                      0);
}
/* End Function:UVM_Thd_Del **************************************************/

/* Begin Function:UVM_Thd_Exec_Set ********************************************
Description : Set a thread's entry point and stack. The registers will be initialized
              with these contents.
Input       : cid_t Cap_Thd - The capability to the thread. 2-Level.
              ptr_t Entry - The entry of the thread. An address.
              ptr_t Stack - The stack address to use for execution. An address.
              ptr_t Param - The parameter of the thread.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Thd_Exec_Set(cid_t Cap_Thd, ptr_t Entry, ptr_t Stack, ptr_t Param)
{
    return UVM_CAP_OP(RME_SVC_THD_EXEC_SET, Cap_Thd,
                      Entry, 
                      Stack,
                      Param);
}
/* End Function:UVM_Thd_Exec_Set *********************************************/

/* Begin Function:UVM_Thd_Hyp_Set *********************************************
Description : Set a thread's entry point and stack. The registers will be initialized
              with these contents.
Input       : cid_t Cap_Thd - The capability to the thread. 2-Level.
              ptr_t Kaddr - The kernel-accessible virtual memory address for this
                            thread's register sets.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Thd_Hyp_Set(cid_t Cap_Thd, ptr_t Kaddr)
{
    return UVM_CAP_OP(RME_SVC_THD_EXEC_SET, 0,
                      Cap_Thd,
                      Kaddr,
                      0);
}
/* End Function:UVM_Thd_Hyp_Set **********************************************/

/* Begin Function:UVM_Thd_Sched_Bind ******************************************
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
Input       : cid_t Cap_Thd - The capability to the thread. 2-Level.
              cid_t Cap_Thd_Sched - The scheduler thread. 2-Level.
              cid_t Cap_Sig - The signal endpoint for scheduler notifications. This signal
                              endpoint will be sent to whenever this thread has a fault, or
                              timeouts. This is purely optional; if it is not needed, pass
                              in RME_CAPID_NULL.
              tid_t TID - The thread ID. This is user-supplied, and the kernel will not
                          check whether there are two threads that have the same TID.
              ptr_t Prio - The priority level, higher is more critical.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Thd_Sched_Bind(cid_t Cap_Thd, cid_t Cap_Thd_Sched, cid_t Cap_Sig, tid_t TID, ptr_t Prio)
{
    return UVM_CAP_OP(RME_SVC_THD_SCHED_BIND, Cap_Thd,
                      UVM_PARAM_D1(Cap_Thd_Sched)|UVM_PARAM_D0(Cap_Sig),
                      TID, 
                      Prio);
}
/* End Function:UVM_Thd_Sched_Bind *******************************************/

/* Begin Function:UVM_Thd_Sched_Rcv *******************************************
Description : Try to receive a notification from the scheduler queue. This
              can only be called from the same core the thread is on.
Input       : cid_t Cap_Thd - The capability to the scheduler thread. We are going
                              to get timeout notifications for the threads that
                              it is responsible for scheduling. This capability must
                              point to the calling thread itself, or the receiving
                              is simply not allowed. 2-Level.
Output      : None.
Return      : ret_t - If successful, the thread ID; or an error code.
******************************************************************************/
ret_t UVM_Thd_Sched_Rcv(cid_t Cap_Thd)
{
    return UVM_CAP_OP(RME_SVC_THD_SCHED_RCV, 0,
                      Cap_Thd,
                      0, 
                      0);
}
/* End Function:UVM_Thd_Sched_Rcv ********************************************/

/* Begin Function:UVM_Thd_Sched_Prio ******************************************
Description : Change a thread's priority level. This can only be called from the
              core that have the thread bonded.
              This system call can cause a potential context switch.
              It is impossible to set a thread's priority beyond its maximum priority. 
Input       : cid_t Cap_Thd - The capability to the thread. 2-Level.
              ptr_t Prio - The priority level, higher is more critical.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Thd_Sched_Prio(cid_t Cap_Thd, ptr_t Prio)
{
    return UVM_CAP_OP(RME_SVC_THD_SCHED_PRIO, 1,
                     UVM_PARAM_D0(Cap_Thd),
                      0,
                     UVM_PARAM_Q0(Prio));
}
/* End Function:UVM_Thd_Sched_Prio *******************************************/

/* Begin Function:UVM_Thd_Sched_Free ******************************************
Description : Free a thread from its current bonding. This function can only be
              executed from the same core on with the thread.
              This system call can cause a potential context switch.
Input       : cid_t Cap_Thd - The capability to the thread. 2-Level.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Thd_Sched_Free(cid_t Cap_Thd)
{
    return UVM_CAP_OP(RME_SVC_THD_SCHED_FREE, 0,
                      Cap_Thd,
                      0, 
                      0);
}
/* End Function:UVM_Thd_Sched_Free *******************************************/

/* Begin Function:UVM_Thd_Time_Xfer *******************************************
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
Input       : cid_t Cap_Thd_Dst - The destination thread. 2-Level.
              cid_t Cap_Thd_Src - The source thread. 2-Level.
              ptr_t Time - The time to transfer, in slices. A slice is the minimal
                           amount of time transfered in the system usually on the
                           order of 100us or 1ms.
Output      : None.
Return      : ret_t - If successful, the destination time amount; or an error code.
******************************************************************************/
ret_t UVM_Thd_Time_Xfer(cid_t Cap_Thd_Dst, cid_t Cap_Thd_Src, ptr_t Time)
{
    return UVM_CAP_OP(RME_SVC_THD_TIME_XFER, 0,
                      Cap_Thd_Dst,
                      Cap_Thd_Src, 
                      Time);
}
/* End Function:UVM_Thd_Time_Xfer ********************************************/

/* Begin Function:UVM_Thd_Swt *************************************************
Description : Switch to another thread. The thread to switch to must have the same
              preemptive priority as this thread, and have time, and not blocked.
              If trying to switch to a higher priority thread, this is impossible
              because whenever a thread with higher priority exists in the system,
              the kernel wiull let it preempt the current thread. 
              If trying to switch to a lower priority thread, this is impossible
              because the current thread just preempts it after the thread switch.
Input       : cid_t Cap_Thd - The capability to the thread. 2-Level. If this is
                              -1, the kernel will pickup whatever thread that have
                              the highest priority and have time to run. 
              ptr_t Full_Yield - This is a flag to indicate whether this is a full yield.
                                 If it is, the kernel will kill all the time alloted on 
                                 this thread.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Thd_Swt(cid_t Cap_Thd, ptr_t Full_Yield)
{
    return UVM_CAP_OP(RME_SVC_THD_SWT, 0,
                      Cap_Thd,
                      Full_Yield, 
                      0);
}
/* End Function:UVM_Thd_Swt **************************************************/

/* Begin Function:UVM_Sig_Crt *************************************************
Description : Create a signal capability.
Input       : cid_t Cap_Captbl - The capability to the capability table to use
                                 for this signal. 2-Level.
              cid_t Cap_Kmem - The kernel memory capability. 2-Level.
              cid_t Cap_Inv - The capability slot that you want this newly created
                              signal capability to be in. 1-Level.
              ptr_t Vaddr - The physical address to store the kernel data. This must fall
                            within the kernel virtual address.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Sig_Crt(cid_t Cap_Captbl, cid_t Cap_Kmem, cid_t Cap_Sig, ptr_t Vaddr)
{
    return UVM_CAP_OP(RME_SVC_SIG_CRT, Cap_Captbl,
                      Cap_Sig,
                      Cap_Kmem,
                      Vaddr);
}
/* End Function:UVM_Sig_Crt **************************************************/

/* Begin Function:UVM_Sig_Del *************************************************
Description : Delete a signal capability.
Input       : cid_t Cap_Captbl - The capability to the capability table to delete from.
                                 2-Level.
              cid_t Cap_Sig - The capability to the signal. 1-Level.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Sig_Del(cid_t Cap_Captbl, cid_t Cap_Sig)
{
    return UVM_CAP_OP(RME_SVC_SIG_DEL, Cap_Captbl,
                      Cap_Sig,
                      0, 
                      0);
}
/* End Function:UVM_Sig_Del **************************************************/

/* Begin Function:UVM_Sig_Snd ************************************************
Description : Try to send a signal from user level. This system call can cause
              a potential context switch.
Input       : cid_t Cap_Sig - The capability to the signal. 2-Level.
              ptr_t Number- The Number of signals to send.
Output      : None.
Return      : ret_t - If successful, 0, or an error code.
******************************************************************************/
ret_t UVM_Sig_Snd(cid_t Cap_Sig,ptr_t Number)
{
    return UVM_CAP_OP(RME_SVC_SIG_SND, 0,
                      Cap_Sig,
                      Number, 
                      0);
}
/* End Function:UVM_Sig_Snd **************************************************/

/* Begin Function:UVM_Sig_Rcv *************************************************
Description : Try to receive a signal capability. The rules for the signal capability
              is:
              1.If a receive endpoint have many send endpoints, everyone can send to it,
                and sending to it will increase the signal count by 1.
              2.If some thread blocks on a receive endpoint, the wakeup is only possible
                from the same core that thread is on.
              3.It is not recommended to let 2 cores operate on the rcv endpoint simutaneously.
              This system call can potentially trigger a context switch.
Input       : cid_t Cap_Sig - The capability to the signal. 2-Level.
              ptr_t Option - The receive option.
Output      : None.
Return      : ret_t - If successful, a non-negative number containing the number of signals
                      received will be returned; else an error code.
******************************************************************************/
ret_t UVM_Sig_Rcv(cid_t Cap_Sig, ptr_t Option)
{
    return UVM_CAP_OP(RME_SVC_SIG_RCV, 0,
                      Cap_Sig,
                      Option, 
                      0);
}
/* End Function:UVM_Sig_Rcv **************************************************/

/* Begin Function:UVM_Inv_Crt *************************************************
Description : Create a invocation capability.
Input       : cid_t Cap_Captbl - The capability to the capability table to use
                                 for this process. 2-Level.
              cid_t Cap_Inv - The capability slot that you want this newly created
                              invocation capability to be in. 1-Level.
              cid_t Cap_Proc - The capability to the process that it is in. 2-Level.
              ptr_t Vaddr - The physical address to store the kernel data. This must fall
                            within the kernel virtual address.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Inv_Crt(cid_t Cap_Captbl, cid_t Cap_Kmem, 
                  cid_t Cap_Inv, cid_t Cap_Proc, ptr_t Vaddr)
{
    return UVM_CAP_OP(RME_SVC_INV_CRT, Cap_Captbl,
                      UVM_PARAM_D1(Cap_Kmem)|UVM_PARAM_D0(Cap_Inv),
                      Cap_Proc, 
                      Vaddr);
}
/* End Function:UVM_Inv_Crt **************************************************/

/* Begin Function:UVM_Inv_Del *************************************************
Description : Delete an invocation capability.
Input       : cid_t Cap_Captbl - The capability to the capability table to delete from.
                                 2-Level.
              cid_t Cap_Inv - The capability to the invocation stub. 1-Level.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Inv_Del(cid_t Cap_Captbl, cid_t Cap_Inv)
{
    return UVM_CAP_OP(RME_SVC_INV_DEL, Cap_Captbl,
                      Cap_Inv,
                      0, 
                      0);
}
/* End Function:UVM_Inv_Del **************************************************/

/* Begin Function:UVM_Inv_Set *************************************************
Description : Set an invocation stub's entry point and stack. The registers will
              be initialized with these contents.
Input       : cid_t Cap_Inv - The capability to the invocation stub. 2-Level.
              ptr_t Entry - The entry of the thread.
              ptr_t Stack - The stack address to use for execution.
              ptr_t Fault_Ret_Flag - If there is an error in this invocation, we return
                                     immediately, or we wait for fault handling?
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Inv_Set(cid_t Cap_Inv, ptr_t Entry, ptr_t Stack, ptr_t Fault_Ret_Flag)
{
    return UVM_CAP_OP(RME_SVC_INV_SET, 0,
                      UVM_PARAM_D1(Fault_Ret_Flag)|UVM_PARAM_D0(Cap_Inv),
                      Entry, 
                      Stack);
}
/* End Function:UVM_Inv_Set **************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/

