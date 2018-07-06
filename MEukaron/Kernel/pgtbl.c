/******************************************************************************
Filename    : pgtbl.c
Author      : pry
Date        : 01/04/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The page table operations for RME OS. 
              The operations included:
              1> Creation of the page table;
              2> Deletion of the page table;
              3> Adding(mapping) pages into a page table;
              4> Deleting(unmapping) pages from a capability table.
              5> Constructing hierachical page tables;
              6> Destructing hierachical page tables.
******************************************************************************/

/* Includes ******************************************************************/
#define __HDR_DEFS__
#include "Platform/RME_platform.h"
#include "Kernel/kernel.h"
#include "Kernel/captbl.h"
#include "Kernel/kotbl.h"
#include "Kernel/pgtbl.h"
#undef __HDR_DEFS__

#define __HDR_STRUCTS__
#include "Platform/RME_platform.h"
#include "Kernel/captbl.h"
#include "Kernel/kernel.h"
#include "Kernel/kotbl.h"
#include "Kernel/pgtbl.h"
#undef __HDR_STRUCTS__

/* Private include */
#include "Kernel/pgtbl.h"

#define __HDR_PUBLIC_MEMBERS__
#include "Platform/RME_platform.h"
#include "Kernel/kernel.h"
#include "Kernel/captbl.h"
#include "Kernel/kotbl.h"
#undef __HDR_PUBLIC_MEMBERS__
/* End Includes **************************************************************/

/* Begin Function:_RME_Pgtbl_Boot_Crt *****************************************
Description : Create a boot-time page table, and put that capability into a designated
              capability table. The function will check if the memory region is large
              enough, and how it has been typed; if the function found out that it is
              impossible to create a page table due to some error, it will return an
              error.
              This function will not ask for a kernel memory capability.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              cid_t Cap_Captbl - The capability to the captbl that may contain the cap
                                 to new captbl. 2-Level.
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
ret_t _RME_Pgtbl_Boot_Crt(struct RME_Cap_Captbl* Captbl, cid_t Cap_Captbl,
                          cid_t Cap_Pgtbl, ptr_t Vaddr, ptr_t Start_Addr,
                          ptr_t Top_Flag, ptr_t Size_Order, ptr_t Num_Order)
{
    struct RME_Cap_Captbl* Captbl_Op;
    struct RME_Cap_Pgtbl* Pgtbl_Crt;
    ptr_t Type_Ref;
    
    /* Check if the total representable memory exceeds our maximum possible
     * addressible memory under the machine word length */
    if((Size_Order+Num_Order)>RME_POW2(RME_WORD_ORDER))
        return RME_ERR_PGT_HW;
    
    /* Check if these parameters are feasible */
    if(__RME_Pgtbl_Check(Start_Addr, Top_Flag, Size_Order, Num_Order, Vaddr)!=0)
        return RME_ERR_PGT_HW;
    
    /* Get the cap location that we care about */
    RME_CAPTBL_GETCAP(Captbl,Cap_Captbl,RME_CAP_CAPTBL,struct RME_Cap_Captbl*,Captbl_Op);
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Captbl_Op,RME_CAPTBL_FLAG_CRT);
    
    /* Get the cap slot */
    RME_CAPTBL_GETSLOT(Captbl_Op,Cap_Pgtbl,struct RME_Cap_Pgtbl*,Pgtbl_Crt);
    /* Take the slot if possible */
    RME_CAPTBL_OCCUPY(Pgtbl_Crt,Type_Ref);

    /* Try to populate the area - Are we creating the top level? */
    if(Top_Flag!=0)
    {  
        if(_RME_Kotbl_Mark(Vaddr, RME_PGTBL_SIZE_TOP(Num_Order))!=0)
        {
            Pgtbl_Crt->Head.Type_Ref=0;
            return RME_ERR_CAP_KOTBL;
        }
    }
    else
    {
        if(_RME_Kotbl_Mark(Vaddr, RME_PGTBL_SIZE_NOM(Num_Order))!=0)
        {
            Pgtbl_Crt->Head.Type_Ref=0;
            return RME_ERR_CAP_KOTBL;
        }
    }

    Pgtbl_Crt->Head.Parent=0;
    Pgtbl_Crt->Head.Object=Vaddr;
    /* Set the property of the page table to only act as source and creating process */
    Pgtbl_Crt->Head.Flags=RME_PGTBL_FLAG_FULL_RANGE|
                          RME_PGTBL_FLAG_ADD_SRC|
                          RME_PGTBL_FLAG_PROC_CRT;
    Pgtbl_Crt->Start_Addr=Start_Addr|Top_Flag;
    /* These two variables are directly placed here. Checks will be done by the driver */
    Pgtbl_Crt->Size_Num_Order=RME_PGTBL_ORDER(Size_Order,Num_Order);
    /* Done. We start initialization of the page table, and we also add all 
     * kernel pages to them. If unsuccessful, we revert operations. At here, 
     * all the information of the page table should be filled in, except for
     * its header */
    if(__RME_Pgtbl_Init(Pgtbl_Crt)!=0)
    {
        /* This must be successful */
        if(Top_Flag!=0)
            RME_ASSERT(_RME_Kotbl_Erase(Vaddr, RME_PGTBL_SIZE_TOP(Num_Order))==0);
        else
            RME_ASSERT(_RME_Kotbl_Erase(Vaddr, RME_PGTBL_SIZE_NOM(Num_Order))==0);
        
        /* Unsuccessful. Revert operations */
        Pgtbl_Crt->Head.Type_Ref=0;
        return RME_ERR_PGT_HW;
    }

    /* Creation complete */
    RME_WRITE_RELEASE();
    Pgtbl_Crt->Head.Type_Ref=RME_CAP_TYPEREF(RME_CAP_PGTBL,0);
    return 0;
}
/* End Function:_RME_Pgtbl_Boot_Crt ******************************************/

/* Begin Function:_RME_Pgtbl_Boot_Con *****************************************
Description : At boot-time, map a child page table from the parent page table. 
              Basically, we are doing the construction of a page table.
              This operation is only used at boot-time and does not check for flags.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              cid_t Cap_Pgtbl_Parent - The capability to the parent page table. 2-Level.
              ptr_t Pos - The virtual address to position map the child page table to.
              cid_t Cap_Pgtbl_Child - The capability to the child page table. 2-Level.
              ptr_t Flags_Child - The flags for the child page table mapping. This restricts
                                  the access permissions of all the memory under this mapping.
Output      : None.
Return      : ret_t - If the mapping is successful, it will return 0; else error code.
******************************************************************************/
ret_t _RME_Pgtbl_Boot_Con(struct RME_Cap_Captbl* Captbl,
                          cid_t Cap_Pgtbl_Parent, ptr_t Pos,
                          cid_t Cap_Pgtbl_Child, ptr_t Flags_Child)
{
    struct RME_Cap_Pgtbl* Pgtbl_Parent;
    struct RME_Cap_Pgtbl* Pgtbl_Child;

    /* The total size order of the child table */
    ptr_t Child_Size_Ord;
#if(RME_VA_EQU_PA==RME_TRUE)
    /* The start mapping address in the parent */
    ptr_t Parent_Map_Addr;
#endif
    
    /* Get the capability slots */
    RME_CAPTBL_GETCAP(Captbl,Cap_Pgtbl_Parent,RME_CAP_PGTBL,struct RME_Cap_Pgtbl*,Pgtbl_Parent);
    RME_CAPTBL_GETCAP(Captbl,Cap_Pgtbl_Child,RME_CAP_PGTBL,struct RME_Cap_Pgtbl*,Pgtbl_Child);
    /* Check if both page table caps are not frozen but don't check flags */
    RME_CAP_CHECK(Pgtbl_Parent, 0);
    RME_CAP_CHECK(Pgtbl_Child, 0);
    
    /* See if the mapping range is allowed */
    if((Pos>>RME_PGTBL_NUMORD(Pgtbl_Parent->Size_Num_Order))!=0)
        return RME_ERR_PGT_ADDR;
    
    /* See if the child table falls within one slot of the parent table */
    Child_Size_Ord=RME_PGTBL_NUMORD(Pgtbl_Child->Size_Num_Order)+
                   RME_PGTBL_SIZEORD(Pgtbl_Child->Size_Num_Order);
    if(RME_PGTBL_SIZEORD(Pgtbl_Parent->Size_Num_Order)<Child_Size_Ord)
        return RME_ERR_PGT_ADDR;
    
#if(RME_VA_EQU_PA==RME_TRUE)
    /* Check if the virtual address mapping is correct */
    Parent_Map_Addr=(Pos<<RME_PGTBL_SIZEORD(Pgtbl_Parent->Size_Num_Order))+
                    RME_PGTBL_START(Pgtbl_Parent->Start_Addr);
    if(Pgtbl_Child->Start_Addr<Parent_Map_Addr)
        return RME_ERR_PGT_ADDR;
    if((Pgtbl_Child->Start_Addr+RME_POW2(Child_Size_Ord))>
       (Parent_Map_Addr+RME_POW2(RME_PGTBL_SIZEORD(Pgtbl_Parent->Size_Num_Order))))
        return RME_ERR_PGT_ADDR;
#endif

    /* Actually do the mapping - This work is passed down to the driver layer. 
     * Successful or not will be determined by the driver layer. */
    if(__RME_Pgtbl_Pgdir_Map(Pgtbl_Parent, Pos, Pgtbl_Child, Flags_Child)!=0)
        return RME_ERR_PGT_MAP;
    
    return 0;
}
/* End Function:_RME_Pgtbl_Boot_Con ******************************************/

/* Begin Function:_RME_Pgtbl_Boot_Add *****************************************
Description : This function is used to initialize the initial user memory mappings.
              This function is exclusively used to set up the Init process's memory
              mappings in the booting process. After the system boots, it is no longer
              possible to fabricate pages like this.
              Additionally, this function will set the cap to page table's property
              as unremovable. This means that it is not allowed to remove any pages
              in the directory. Additionally, it will set the reference count of the
              capability as 1, thus making the capability to the initial page table
              virtually undeletable.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              cid_t Cap_Pgtbl - The capability to the page table. 2-Level.
              ptr_t Paddr - The physical address to map from.
              ptr_t Pos - The virtual address position to map to. This position is
                          a index in the user memory.
              ptr_t Flags - The flags for the user page.
Output      : None.
Return      : ret_t - If the mapping is successful, it will return 0; else error code.
******************************************************************************/
ret_t _RME_Pgtbl_Boot_Add(struct RME_Cap_Captbl* Captbl, cid_t Cap_Pgtbl, 
                          ptr_t Paddr, ptr_t Pos, ptr_t Flags)
{
    struct RME_Cap_Pgtbl* Pgtbl_Op;
    
    /* Get the capability slot */
    RME_CAPTBL_GETCAP(Captbl,Cap_Pgtbl,RME_CAP_PGTBL,struct RME_Cap_Pgtbl*,Pgtbl_Op);    
    /* Check if the target captbl is not frozen, but don't check their properties */
    RME_CAP_CHECK(Pgtbl_Op,0);

#if(RME_VA_EQU_PA==RME_TRUE)
    /* Check if we force identical mapping */
    if(Paddr!=((Pos<<RME_PGTBL_SIZEORD(Pgtbl_Op->Size_Num_Order))+RME_PGTBL_START(Pgtbl_Op->Start_Addr)))
        return RME_ERR_PGT_ADDR; 
#endif

    /* See if the mapping range and the granularity is allowed */
    if(((Pos>>RME_PGTBL_NUMORD(Pgtbl_Op->Size_Num_Order))!=0)||
       ((Paddr&RME_MASK_END(RME_PGTBL_SIZEORD(Pgtbl_Op->Size_Num_Order)-1))!=0))
        return RME_ERR_PGT_ADDR;

    /* Actually do the mapping - This work is passed down to the driver layer. 
     * Successful or not will be determined by the driver layer. */
    if(__RME_Pgtbl_Page_Map(Pgtbl_Op, Paddr, Pos, Flags)!=0)
        return RME_ERR_PGT_MAP;
    
    return 0;
}
/* End Function:_RME_Pgtbl_Boot_Add ******************************************/

/* Begin Function:_RME_Pgtbl_Crt **********************************************
Description : Create a layer of page table, and put that capability into a designated
              capability table. The function will check if the memory region is large
              enough, and how it has been typed; if the function found out that it is
              impossible to create a page table due to some error, it will return an
              error.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              cid_t Cap_Captbl - The capability to the captbl that may contain the cap
                                 to new captbl. 2-Level.
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
ret_t _RME_Pgtbl_Crt(struct RME_Cap_Captbl* Captbl, cid_t Cap_Captbl,
                     cid_t Cap_Kmem, cid_t Cap_Pgtbl, ptr_t Vaddr,
                     ptr_t Start_Addr, ptr_t Top_Flag, ptr_t Size_Order, ptr_t Num_Order)
{
    struct RME_Cap_Captbl* Captbl_Op;
    struct RME_Cap_Kmem* Kmem_Op;
    struct RME_Cap_Pgtbl* Pgtbl_Crt;
    ptr_t Type_Ref;
    
    /* Check if the total representable memory exceeds our maximum possible
     * addressible memory under the machine word length */
    if((Size_Order+Num_Order)>RME_POW2(RME_WORD_ORDER))
        return RME_ERR_PGT_HW;
    
    /* Check if these parameters are feasible */
    if(__RME_Pgtbl_Check(Start_Addr, Top_Flag, Size_Order, Num_Order, Vaddr)!=0)
        return RME_ERR_PGT_HW;
    
    /* Get the cap location that we care about */
    RME_CAPTBL_GETCAP(Captbl,Cap_Captbl,RME_CAP_CAPTBL,struct RME_Cap_Captbl*,Captbl_Op);
    RME_CAPTBL_GETCAP(Captbl,Cap_Kmem,RME_CAP_KMEM,struct RME_Cap_Kmem*,Kmem_Op);
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Captbl_Op,RME_CAPTBL_FLAG_CRT);
    /* See if the creation is valid for this kmem range */
    if(Top_Flag!=0)
        RME_KMEM_CHECK(Kmem_Op,RME_KMEM_FLAG_PGTBL,Vaddr,RME_PGTBL_SIZE_TOP(Num_Order));
    else
        RME_KMEM_CHECK(Kmem_Op,RME_KMEM_FLAG_PGTBL,Vaddr,RME_PGTBL_SIZE_NOM(Num_Order));
    
    /* Get the cap slot */
    RME_CAPTBL_GETSLOT(Captbl_Op,Cap_Pgtbl,struct RME_Cap_Pgtbl*,Pgtbl_Crt);
    /* Take the slot if possible */
    RME_CAPTBL_OCCUPY(Pgtbl_Crt,Type_Ref);
    
    /* Try to populate the area - Are we creating the top level? */
    if(Top_Flag!=0)
    {  
        if(_RME_Kotbl_Mark(Vaddr, RME_PGTBL_SIZE_TOP(Num_Order))!=0)
        {
            Pgtbl_Crt->Head.Type_Ref=0;
            return RME_ERR_CAP_KOTBL;
        }
    }
    else
    {
        if(_RME_Kotbl_Mark(Vaddr, RME_PGTBL_SIZE_NOM(Num_Order))!=0)
        {
            Pgtbl_Crt->Head.Type_Ref=0;
            return RME_ERR_CAP_KOTBL;
        }
    }
    
    Pgtbl_Crt->Head.Parent=0;
    Pgtbl_Crt->Head.Object=Vaddr;
    Pgtbl_Crt->Head.Flags=RME_PGTBL_FLAG_FULL_RANGE|
                          RME_PGTBL_FLAG_ADD_SRC|RME_PGTBL_FLAG_ADD_DST|RME_PGTBL_FLAG_REM|
                          RME_PGTBL_FLAG_CON_CHILD|RME_PGTBL_FLAG_CON_PARENT|RME_PGTBL_FLAG_DES|
                          RME_PGTBL_FLAG_PROC_CRT|RME_PGTBL_FLAG_PROC_PGT;
    Pgtbl_Crt->Start_Addr=Start_Addr|Top_Flag;
    /* These two variables are directly placed here. Checks will be done by the driver */
    Pgtbl_Crt->Size_Num_Order=RME_PGTBL_ORDER(Size_Order,Num_Order);
    /* Done. We start initialization of the page table, and we also add all 
     * kernel pages to them. If unsuccessful, we revert operations. At here, 
     * all the information of the page table should be filled in, except for
     * its header */
    if(__RME_Pgtbl_Init(Pgtbl_Crt)!=0)
    {
        /* This must be successful */
        if(Top_Flag!=0)
            RME_ASSERT(_RME_Kotbl_Erase(Vaddr, RME_PGTBL_SIZE_TOP(Num_Order))==0);
        else
            RME_ASSERT(_RME_Kotbl_Erase(Vaddr, RME_PGTBL_SIZE_NOM(Num_Order))==0);
        
        /* Unsuccessful. Revert operations */
        Pgtbl_Crt->Head.Type_Ref=0;
        return RME_ERR_PGT_HW;
    }

    /* Creation complete */
    RME_WRITE_RELEASE();
    Pgtbl_Crt->Head.Type_Ref=RME_CAP_TYPEREF(RME_CAP_PGTBL,0);
    return 0;
}
/* End Function:_RME_Pgtbl_Crt ***********************************************/

/* Begin Function:_RME_Pgtbl_Del **********************************************
Description : Delete a layer of page table. We do not care if the childs are all
              deleted. For MPU based environments, it is required that all the 
              mapped child page tables are deconstructed from the master table
              before we can destroy the master table.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              cid_t Cap_Captbl - The capability to the captbl that may contain the cap
                                 to new captbl. 2-Level.
              cid_t Cap_Pgtbl - The capability slot that you want this newly created
                                page table capability to be in. 1-Level.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t _RME_Pgtbl_Del(struct RME_Cap_Captbl* Captbl, cid_t Cap_Captbl, cid_t Cap_Pgtbl)
{
    struct RME_Cap_Captbl* Captbl_Op;
    struct RME_Cap_Pgtbl* Pgtbl_Del;
    ptr_t Type_Ref;
    /* These are used for deletion */
    ptr_t Object;
    ptr_t Size;
    
    /* Get the cap location that we care about */
    RME_CAPTBL_GETCAP(Captbl,Cap_Captbl,RME_CAP_CAPTBL,struct RME_Cap_Captbl*,Captbl_Op);
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Captbl_Op,RME_CAPTBL_FLAG_DEL);
    
    /* Get the cap slot */
    RME_CAPTBL_GETSLOT(Captbl_Op,Cap_Pgtbl,struct RME_Cap_Pgtbl*,Pgtbl_Del);
    /* Delete check */
    RME_CAP_DEL_CHECK(Pgtbl_Del,Type_Ref,RME_CAP_PGTBL);
    
    /* Hardware related deletion check - can we delete this now? This work is passed
     * down to the hardware level. The consistency check should make sure that, when 
     * a page table can be directly deleted:
     * 1> It is not referenced by any higher-level page tables.
     * 2> It does not reference any lower-level page tables.
     * If the driver layer does not conform to this, the deletion of page table is
     * not guaranteed to main kernel consistency, and such consistency must be maintained
     * by the user-level. */
    if(__RME_Pgtbl_Del_Check(Pgtbl_Del)!=0)
    {
        RME_CAP_DEFROST(Pgtbl_Del,Type_Ref);
        return RME_ERR_PGT_HW;
    }
    
    /* Remember these two variables for deletion */
    Object=RME_CAP_GETOBJ(Pgtbl_Del,ptr_t);
    if(((Pgtbl_Del->Start_Addr)&RME_PGTBL_TOP)!=0)
        Size=RME_PGTBL_SIZE_TOP(RME_PGTBL_NUMORD(Pgtbl_Del->Size_Num_Order));
    else
        Size=RME_PGTBL_SIZE_NOM(RME_PGTBL_NUMORD(Pgtbl_Del->Size_Num_Order));
    
    /* Now we can safely delete the cap */
    RME_WRITE_RELEASE();
    RME_CAP_REMDEL(Pgtbl_Del,Type_Ref);
    /* Try to erase the area - This must be successful */
    RME_ASSERT(_RME_Kotbl_Erase(Object, Size));
    
    return 0;
}
/* End Function:_RME_Pgtbl_Del ***********************************************/

/* Begin Function:_RME_Pgtbl_Add **********************************************
Description : Delegate a page from one page table to another. This is the only way
              to add pages to new page tables after the system boots.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              cid_t Cap_Pgtbl_Dst - The capability to the destination page directory. 2-Level.
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
ret_t _RME_Pgtbl_Add(struct RME_Cap_Captbl* Captbl, 
                     cid_t Cap_Pgtbl_Dst, ptr_t Pos_Dst, ptr_t Flags_Dst,
                     cid_t Cap_Pgtbl_Src, ptr_t Pos_Src, ptr_t Index)
{
    struct RME_Cap_Pgtbl* Pgtbl_Src;
    struct RME_Cap_Pgtbl* Pgtbl_Dst;
    ptr_t Paddr_Dst;
    ptr_t Paddr_Src;
    ptr_t Flags_Src;
    
    /* Get the capability slots */
    RME_CAPTBL_GETCAP(Captbl,Cap_Pgtbl_Dst,RME_CAP_PGTBL,struct RME_Cap_Pgtbl*,Pgtbl_Dst);
    RME_CAPTBL_GETCAP(Captbl,Cap_Pgtbl_Src,RME_CAP_PGTBL,struct RME_Cap_Pgtbl*,Pgtbl_Src);
    /* Check if both page table caps are not frozen and allows such operations */
    RME_CAP_CHECK(Pgtbl_Dst, RME_PGTBL_FLAG_ADD_DST);
    RME_CAP_CHECK(Pgtbl_Src, RME_PGTBL_FLAG_ADD_SRC);
    /* Check the operation range - This is page table specific */
    if((Pos_Dst>RME_PGTBL_FLAG_HIGH(Pgtbl_Dst->Head.Flags))||
       (Pos_Dst<RME_PGTBL_FLAG_LOW(Pgtbl_Dst->Head.Flags))||
       (Pos_Src>RME_PGTBL_FLAG_HIGH(Pgtbl_Src->Head.Flags))||
       (Pos_Src<RME_PGTBL_FLAG_LOW(Pgtbl_Src->Head.Flags)))
        return RME_ERR_CAP_FLAG;
            
    /* See if the size order relationship is correct */
    if(RME_PGTBL_SIZEORD(Pgtbl_Dst->Size_Num_Order)>RME_PGTBL_SIZEORD(Pgtbl_Src->Size_Num_Order))
        return RME_ERR_PGT_ADDR;
    /* See if the position indices are out of range */
    if(((Pos_Dst>>RME_PGTBL_NUMORD(Pgtbl_Dst->Size_Num_Order))!=0)||
       ((Pos_Src>>RME_PGTBL_NUMORD(Pgtbl_Src->Size_Num_Order))!=0))
        return RME_ERR_PGT_ADDR;
    /* See if the source subposition index is out of range */
    if(RME_POW2(RME_PGTBL_SIZEORD(Pgtbl_Src->Size_Num_Order))<=
       (Index<<RME_PGTBL_SIZEORD(Pgtbl_Dst->Size_Num_Order)))
        return RME_ERR_PGT_ADDR;
    /* Get the physical address and RME standard flags of that source page */
    if(__RME_Pgtbl_Lookup(Pgtbl_Src, Pos_Src, &Paddr_Src, &Flags_Src)!=0)
        return RME_ERR_PGT_HW;
    
    /* Calculate the destination physical address */
    Paddr_Dst=Paddr_Src+(Index<<RME_PGTBL_SIZEORD(Pgtbl_Dst->Size_Num_Order));
#if(RME_VA_EQU_PA==RME_TRUE)
    /* Check if we force identical mapping. No need to check granularity here */
    if(Paddr_Dst!=((Pos_Dst<<RME_PGTBL_SIZEORD(Pgtbl_Dst->Size_Num_Order))+
                   RME_PGTBL_START(Pgtbl_Dst->Start_Addr)))
        return RME_ERR_PGT_ADDR;
#endif
    /* Analyze the flags - we do not allow expansion of access permissions */
    if(((Flags_Dst)&(~Flags_Src))!=0)
        return RME_ERR_PGT_PERM;
    
    /* Actually do the mapping - This work is passed down to the driver layer. 
     * Successful or not will be determined by the driver layer. Under a multi-core
     * environment, the driver layer need to determine whether two cores are modifying
     * a same page, and do corresponding CAS if such operations are to be avoided.
     */
    if(__RME_Pgtbl_Page_Map(Pgtbl_Dst, Paddr_Dst, Pos_Dst, Flags_Dst)!=0)
        return RME_ERR_PGT_MAP;
    
    return 0;
}
/* End Function:_RME_Pgtbl_Add ***********************************************/

/* Begin Function:_RME_Pgtbl_Rem **********************************************
Description : Remove a page from the page table. We are doing unmapping of a page.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              cid_t Cap_Pgtbl - The capability to the page table. 2-Level.
              ptr_t Pos - The virtual address position to unmap from.
Output      : None.
Return      : ret_t - If the unmapping is successful, it will return 0; else error code.
******************************************************************************/
ret_t _RME_Pgtbl_Rem(struct RME_Cap_Captbl* Captbl, cid_t Cap_Pgtbl, ptr_t Pos)
{
    struct RME_Cap_Pgtbl* Pgtbl_Rem;
    
    /* Get the cap location that we care about */
    RME_CAPTBL_GETCAP(Captbl,Cap_Pgtbl,RME_CAP_CAPTBL,struct RME_Cap_Pgtbl*,Pgtbl_Rem);
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Pgtbl_Rem,RME_PGTBL_FLAG_REM);
    /* Check the operation range - This is page table specific */
    if((Pos>RME_PGTBL_FLAG_HIGH(Pgtbl_Rem->Head.Flags))||
       (Pos<RME_PGTBL_FLAG_LOW(Pgtbl_Rem->Head.Flags)))
        return RME_ERR_CAP_FLAG;

    /* See if the unmapping range is allowed */
    if((Pos>>RME_PGTBL_NUMORD(Pgtbl_Rem->Size_Num_Order))!=0)
        return RME_ERR_PGT_ADDR;
    
    /* Actually do the unmapping - This work is passed down to the driver layer.
     * Successful or not will be determined by the driver layer. In the multi-core
     * environment, this should be taken care of by the driver to make sure hazard will
     * not happen by using the CAS. */
    if(__RME_Pgtbl_Page_Unmap(Pgtbl_Rem, Pos)!=0)
        return RME_ERR_PGT_MAP;
    
    return 0;
}
/* End Function:_RME_Pgtbl_Rem ***********************************************/

/* Begin Function:_RME_Pgtbl_Con **********************************************
Description : Map a child page table from the parent page table. Basically, we 
              are doing the construction of a page table.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              cid_t Cap_Pgtbl_Parent - The capability to the parent page table. 2-Level.
              ptr_t Pos - The virtual address to position map the child page table to.
              cid_t Cap_Pgtbl_Child - The capability to the child page table. 2-Level.
              ptr_t Flags_Child - The flags for the child page table mapping. This restricts
                                  the access permissions of all the memory under this mapping.
Output      : None.
Return      : ret_t - If the mapping is successful, it will return 0; else error code.
******************************************************************************/
ret_t _RME_Pgtbl_Con(struct RME_Cap_Captbl* Captbl,
                     cid_t Cap_Pgtbl_Parent, ptr_t Pos,
                     cid_t Cap_Pgtbl_Child, ptr_t Flags_Child)
{
    struct RME_Cap_Pgtbl* Pgtbl_Parent;
    struct RME_Cap_Pgtbl* Pgtbl_Child;
    /* The total size order of the child table */
    ptr_t Child_Size_Ord;
#if(RME_VA_EQU_PA==RME_TRUE)
    /* The start mapping address in the parent */
    ptr_t Parent_Map_Addr;
#endif
    
    /* Get the capability slots */
    RME_CAPTBL_GETCAP(Captbl,Cap_Pgtbl_Parent,RME_CAP_PGTBL,struct RME_Cap_Pgtbl*,Pgtbl_Parent);
    RME_CAPTBL_GETCAP(Captbl,Cap_Pgtbl_Child,RME_CAP_PGTBL,struct RME_Cap_Pgtbl*,Pgtbl_Child);
    /* Check if both page table caps are not frozen and allows such operations */
    RME_CAP_CHECK(Pgtbl_Parent, RME_PGTBL_FLAG_CON_PARENT);
    RME_CAP_CHECK(Pgtbl_Child, RME_PGTBL_FLAG_CON_CHILD);
    /* Check the operation range - This is page table specific */
    if((Pos>RME_PGTBL_FLAG_HIGH(Pgtbl_Parent->Head.Flags))||
       (Pos<RME_PGTBL_FLAG_LOW(Pgtbl_Parent->Head.Flags)))
        return RME_ERR_CAP_FLAG;
    
    /* See if the mapping range is allowed */
    if((Pos>>RME_PGTBL_NUMORD(Pgtbl_Parent->Size_Num_Order))!=0)
        return RME_ERR_PGT_ADDR;
    
    /* See if the child table falls within one slot of the parent table */
    Child_Size_Ord=RME_PGTBL_NUMORD(Pgtbl_Child->Size_Num_Order)+
                   RME_PGTBL_SIZEORD(Pgtbl_Child->Size_Num_Order);

#if(RME_VA_EQU_PA==RME_TRUE)
    /* Path-compression option available */
    if(RME_PGTBL_SIZEORD(Pgtbl_Parent->Size_Num_Order)<Child_Size_Ord)
        return RME_ERR_PGT_ADDR;
    /* Check if the virtual address mapping is correct */
    Parent_Map_Addr=(Pos<<RME_PGTBL_SIZEORD(Pgtbl_Parent->Size_Num_Order))+
                    RME_PGTBL_START(Pgtbl_Parent->Start_Addr);
    if(Pgtbl_Child->Start_Addr<Parent_Map_Addr)
        return RME_ERR_PGT_ADDR;
    if((Pgtbl_Child->Start_Addr+RME_POW2(Child_Size_Ord))>
       (Parent_Map_Addr+RME_POW2(RME_PGTBL_SIZEORD(Pgtbl_Parent->Size_Num_Order))))
        return RME_ERR_PGT_ADDR;
#else
    /* If this is the case, then we force no path compression */
    if(RME_PGTBL_SIZEORD(Pgtbl_Parent->Size_Num_Order)!=Child_Size_Ord)
        return RME_ERR_PGT_ADDR;
#endif
    /* Actually do the mapping - This work is passed down to the driver layer. 
     * Successful or not will be determined by the driver layer. */
    if(__RME_Pgtbl_Pgdir_Map(Pgtbl_Parent, Pos, Pgtbl_Child, Flags_Child)!=0)
        return RME_ERR_PGT_MAP;
    
    return 0;
}
/* End Function:_RME_Pgtbl_Con ***********************************************/

/* Begin Function:_RME_Pgtbl_Des **********************************************
Description : Unmap a child page table from the parent page table. Basically, we 
              are doing the destruction of a page table.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              cid_t Cap_Pgtbl - The capability to the page table. 2-Level.
              ptr_t Pos - The virtual address to position unmap the child page
                          table from.
Output      : None.
Return      : ret_t - If the mapping is successful, it will return 0; else error code.
******************************************************************************/
ret_t _RME_Pgtbl_Des(struct RME_Cap_Captbl* Captbl, cid_t Cap_Pgtbl, ptr_t Pos)
{
    struct RME_Cap_Pgtbl* Pgtbl_Des;
    
    /* Get the cap location that we care about */
    RME_CAPTBL_GETCAP(Captbl,Cap_Pgtbl,RME_CAP_CAPTBL,struct RME_Cap_Pgtbl*,Pgtbl_Des);
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Pgtbl_Des,RME_PGTBL_FLAG_DES);
    /* Check the operation range - This is page table specific */
    if((Pos>RME_PGTBL_FLAG_HIGH(Pgtbl_Des->Head.Flags))||
       (Pos<RME_PGTBL_FLAG_LOW(Pgtbl_Des->Head.Flags)))
        return RME_ERR_CAP_FLAG;

    /* See if the unmapping range is allowed */
    if((Pos>>RME_PGTBL_NUMORD(Pgtbl_Des->Size_Num_Order))!=0)
        return RME_ERR_PGT_ADDR;
    
    /* Actually do the unmapping - This work is passed down to the driver layer.
     * Successful or not will be determined by the driver layer. */
    if(__RME_Pgtbl_Pgdir_Unmap(Pgtbl_Des, Pos)!=0)
        return RME_ERR_PGT_MAP;
    
    return 0;
}
/* End Function:_RME_Pgtbl_Des ***********************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/

