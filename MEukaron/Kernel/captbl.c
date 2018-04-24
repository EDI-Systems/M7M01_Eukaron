/******************************************************************************
Filename    : captbl.c
Author      : pry
Date        : 23/03/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The capability table operations for RME OS. 
              This section of code might be confusing if the operating principles
              of capability-based systems is not well understood. 
              1> Owning a capability means you have the power to use the function
                 represented by that capability.
              2> Capabilities have an field called flags, which denotes what operations
                 is allowed on that captbl.
              3> Owning a capability to a capability table means that you have the
                 power to modify the capability table's contents.
              4> Creation and deletion of kernel objects is an operation on capability
                 table, thus requiring the capability to the capability table.
              Remember we do not check our master table to see if it is frozen, or if it is
              captbl, or something, because if the master table's cap is not explicitly passed
              in, we do not operate on it at all; If it is explicitly passed in, it will be checked.
              Hazard Table:
Operation 1    Operation 2    Reason why it is safe(Operation 2 follows operation 1)
-------------------------------------------------------------------------------------------
Create         Create         Only one creation will be successful, because OCCUPY slot is done by CAS.
Create         Delete         OCCUPY only set the FROZEN. Delete will require a TYPE data, which will
                              only be set after the creation completes. If it is set, then the FROZEN
                              will be cleared, and the deletion CAS will fail.
Create         Freeze         OCCUPY only set the FROZEN. FROZEN will require that bit not set.
Create         Add-Src        Add-Src will require a TYPE data, which will only be set after the 
                              creation completes.
Create         Add-Dst        Only one creation will be successful, because OCCUPY slot is done by CAS.
Create         Remove         OCCUPY only set the FROZEN. Remove will require a TYPE data, which will
                              only be set after the creation completes.
Create         Use            OCCUPY only set the FROZEN. Use the cap will require a TYPE data, which
                              will only be set after the creation completes.
-------------------------------------------------------------------------------------------
Delete         Delete         Actual deletion done by CAS so only one deletion will be successful.
Delete         Freeze         If the deletion fails and clears the freeze flag, nothing will be done;
                              If it does not fail, then the cap will be erased, and the FREEZE CAS
                              will not succeed.
Delete         Remove         Only one will be successful because only the root can be DELETED.
Delete         Others         Banned by the FROZEN flag before deletion.
-------------------------------------------------------------------------------------------
Freeze         Create         Create will fail because something is still in the slot.
Freeze         Delete         Delete will fail if not FROZEN; Even if FROZEN, QUIESCENCE will ban it. 
Freeze         Remove         Remove will fail if not FROZEN; Even if FROZEN, QUIESCENCE will ban it. 
Freeze         Freeze         Freeze done by CAS, and only one will be successful.
Freeze         Others         Freeze will ban them if they do attempt after FROZEN set.
-------------------------------------------------------------------------------------------
Add-Src        Create         Impossible because something in that slot.
Add-Src        Freeze         Cannot freeze if already increased refcnt. If they increase REFCNT just
                              after FROZEN set, let it be. The cap cannot be removed or deleted because
                              they will check refcnt.
Add-Src        Delete         Impossible because cap not frozen.
Add-Src        Remove         Impossible because cap not frozen.
Add-Src        Others         These operations can be done in parallel, so no worry.
-------------------------------------------------------------------------------------------
Add-Dst         ...           Conclusion same as Create operation.
-------------------------------------------------------------------------------------------
Remove          ...           Conclusion same as Delete operation.
-------------------------------------------------------------------------------------------
Use            Create         Impossible because something in that slot.
Use            Delete         Impossible because not FROZEN.
Use            Freeze         It is OK.
Use            Add-Src        It is OK.
Use            Add-Dst        Impossible because something in that slot.
Use            Remove         Impossible because not FROZEN.
Use            Use            It is OK.
******************************************************************************/

/* Includes ******************************************************************/
#define __HDR_DEFS__
#include "Platform/RME_platform.h"
#include "Kernel/kernel.h"
#include "Kernel/captbl.h"
#include "Kernel/pgtbl.h"
#include "Kernel/kotbl.h"
#undef __HDR_DEFS__

#define __HDR_STRUCTS__
#include "Platform/RME_platform.h"
#include "Kernel/captbl.h"
#include "Kernel/kernel.h"
#include "Kernel/pgtbl.h"
#include "Kernel/kotbl.h"
#undef __HDR_STRUCTS__

/* Private include */
#include "Kernel/captbl.h"

#define __HDR_PUBLIC_MEMBERS__
#include "Platform/RME_platform.h"
#include "Kernel/kernel.h"
#include "Kernel/kotbl.h"
#undef __HDR_PUBLIC_MEMBERS__
/* End Includes **************************************************************/

/* Begin Function:_RME_Captbl_Boot_Init ***************************************
Description : Create the first boot-time capability table. This will be the first
              capability table created in the system. This function must be called
              at system startup first before setting up any other kernel objects.
              This function does not ask for kernel memory capability.
Input       : cid_t Cap_Captbl - The capability slot that you want this newly created
                                 capability table capability to be in. 1-Level.
              ptr_t Vaddr - The virtual address to store the capability table.
              ptr_t Entry_Num - The number of capabilities in the capability table.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t _RME_Captbl_Boot_Init(cid_t Cap_Captbl, ptr_t Vaddr, ptr_t Entry_Num)
{
    cnt_t Count;
    struct RME_Cap_Captbl* Captbl;
    
    /* See if the entry number is too big */
    if((Entry_Num==0)||(Entry_Num>RME_CAPID_2L))
            return RME_ERR_CAP_RANGE;
    
    /* Try to populate the area */
    if(_RME_Kotbl_Mark(Vaddr, RME_CAPTBL_SIZE(Entry_Num))!=0)
        return RME_ERR_CAP_KOTBL;
    
    /* Done. We start creation of the capability table. Clear header as well */
    for(Count=0;Count<Entry_Num;Count++)
        RME_CAP_CLEAR(&(((struct RME_Cap_Struct*)Vaddr)[Count]));
    
    Captbl=&(((struct RME_Cap_Captbl*)Vaddr)[Cap_Captbl]);
    /* Set the cap's parameters according to what we have just created */
    RME_CAP_CLEAR(Captbl);
    Captbl->Head.Type_Ref=RME_CAP_TYPEREF(RME_CAP_CAPTBL,0);
    Captbl->Head.Parent=0;
    Captbl->Head.Object=Vaddr;
    /* New cap allows all operations */
    Captbl->Head.Flags=RME_CAPTBL_FLAG_CRT|RME_CAPTBL_FLAG_DEL|RME_CAPTBL_FLAG_FRZ|
                       RME_CAPTBL_FLAG_ADD_SRC|RME_CAPTBL_FLAG_ADD_DST|RME_CAPTBL_FLAG_REM|
                       RME_CAPTBL_FLAG_PROC_CRT|RME_CAPTBL_FLAG_PROC_CPT;
    Captbl->Entry_Num=Entry_Num;
    
    return Cap_Captbl;
}
/* End Function:_RME_Captbl_Boot_Init ****************************************/

/* Begin Function:_RME_Captbl_Boot_Crt *********************************************
Description : Create a boot-time capability table at the memory segment designated.
              This function does not ask for kernel memory capability.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              cid_t Cap_Captbl_Crt - The capability to the captbl that may contain
                                     the cap to new captbl. 2-Level.
              cid_t Cap_Crt - The cap position to hold the new cap. 1-Level.
              ptr_t Vaddr - The virtual address to store the capability table.
              ptr_t Entry_Num - The number of capabilities in the capability table.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t _RME_Captbl_Boot_Crt(struct RME_Cap_Captbl* Captbl, cid_t Cap_Captbl_Crt,
                           cid_t Cap_Crt, ptr_t Vaddr, ptr_t Entry_Num)
{
    cnt_t Count;
    struct RME_Cap_Captbl* Captbl_Op;
    struct RME_Cap_Kmem* Kmem_Op;
    struct RME_Cap_Captbl* Captbl_Crt;
    ptr_t Type_Ref;
    
    /* See if the entry number is too big */
    if((Entry_Num==0)||(Entry_Num>RME_CAPID_2L))
        return RME_ERR_CAP_RANGE;
    
    /* Get the cap location that we care about */
    RME_CAPTBL_GETCAP(Captbl,Cap_Captbl_Crt,RME_CAP_CAPTBL,struct RME_Cap_Captbl*,Captbl_Op);
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Captbl_Op,RME_CAPTBL_FLAG_CRT);
    
    /* Get the cap slot */
    RME_CAPTBL_GETSLOT(Captbl_Op,Cap_Crt,struct RME_Cap_Captbl*,Captbl_Crt);
    /* Take the slot if possible */
    RME_CAPTBL_OCCUPY(Captbl_Crt,Type_Ref);
    /* Try to mark this area as populated */
    if(_RME_Kotbl_Mark(Vaddr, RME_CAPTBL_SIZE(Entry_Num))!=0)
    {
        /* Failure. Set the Type_Ref back to 0 and abort the creation process */
        Captbl->Head.Type_Ref=0;
        return RME_ERR_CAP_KOTBL;
    }
    
    /* Done. We start creation of the capability table. Clear header as well */
    for(Count=0;Count<Entry_Num;Count++)
        RME_CAP_CLEAR(&(((struct RME_Cap_Struct*)Vaddr)[Count]));
    
    /* Set the cap's parameters according to what we have just created */
    Captbl_Crt->Head.Parent=0;
    Captbl_Crt->Head.Object=Vaddr;
    Captbl_Crt->Head.Flags=RME_CAPTBL_FLAG_CRT|RME_CAPTBL_FLAG_DEL|RME_CAPTBL_FLAG_FRZ|
                           RME_CAPTBL_FLAG_ADD_SRC|RME_CAPTBL_FLAG_ADD_DST|RME_CAPTBL_FLAG_REM|
                           RME_CAPTBL_FLAG_PROC_CRT|RME_CAPTBL_FLAG_PROC_CPT;
    Captbl_Crt->Entry_Num=Entry_Num;
    /* At last, write into slot the correct information, and clear the frozen bit */
    Captbl_Crt->Head.Type_Ref=RME_CAP_TYPEREF(RME_CAP_CAPTBL,0);
    
    return 0;
}
/* End Function:_RME_Captbl_Boot_Crt *****************************************/

/* Begin Function:_RME_Captbl_Crt *********************************************
Description : Create a capability table at the memory segment designated. The table
              must be located in a memory segment that is designated as kernel memory.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              cid_t Cap_Captbl_Crt - The capability to the captbl that may contain
                                     the cap to new captbl. 2-Level.
              cid_t Cap_Kmem - The kernel memory capability. 2-Level.
              cid_t Cap_Crt - The cap position to hold the new cap. 1-Level.
              ptr_t Vaddr - The virtual address to store the capability table.
              ptr_t Entry_Num - The number of capabilities in the capability table.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t _RME_Captbl_Crt(struct RME_Cap_Captbl* Captbl, cid_t Cap_Captbl_Crt,
                      cid_t Cap_Kmem, cid_t Cap_Crt, ptr_t Vaddr, ptr_t Entry_Num)
{
    cnt_t Count;
    struct RME_Cap_Captbl* Captbl_Op;
    struct RME_Cap_Kmem* Kmem_Op;
    struct RME_Cap_Captbl* Captbl_Crt;
    ptr_t Type_Ref;

    /* See if the entry number is too big */
    if((Entry_Num==0)||(Entry_Num>RME_CAPID_2L))
        return RME_ERR_CAP_RANGE;

    /* Get the cap location that we care about */
    RME_CAPTBL_GETCAP(Captbl,Cap_Captbl_Crt,RME_CAP_CAPTBL,struct RME_Cap_Captbl*,Captbl_Op);
    RME_CAPTBL_GETCAP(Captbl,Cap_Kmem,RME_CAP_KMEM,struct RME_Cap_Kmem*,Kmem_Op);
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Captbl_Op,RME_CAPTBL_FLAG_CRT);
    /* See if the creation is valid for this kmem range */
    RME_KMEM_CHECK(Kmem_Op,RME_KMEM_FLAG_CAPTBL,Vaddr,RME_CAPTBL_SIZE(Entry_Num));

    /* Get the cap slot */
    RME_CAPTBL_GETSLOT(Captbl_Op,Cap_Crt,struct RME_Cap_Captbl*,Captbl_Crt);
    /* Take the slot if possible */
    RME_CAPTBL_OCCUPY(Captbl_Crt,Type_Ref);
    /* Try to mark this area as populated */
    if(_RME_Kotbl_Mark(Vaddr, RME_CAPTBL_SIZE(Entry_Num))!=0)
    {
        /* Failure. Set the Type_Ref back to 0 and abort the creation process */
        Captbl->Head.Type_Ref=0;
        return RME_ERR_CAP_KOTBL;
    }

    /* Done. We start creation of the capability table. Clear header as well */
    for(Count=0;Count<Entry_Num;Count++)
        RME_CAP_CLEAR(&(((struct RME_Cap_Struct*)Vaddr)[Count]));

    /* Set the cap's parameters according to what we have just created */
    Captbl_Crt->Head.Parent=0;
    Captbl_Crt->Head.Object=Vaddr;
    Captbl_Crt->Head.Flags=RME_CAPTBL_FLAG_CRT|RME_CAPTBL_FLAG_DEL|RME_CAPTBL_FLAG_FRZ|
                           RME_CAPTBL_FLAG_ADD_SRC|RME_CAPTBL_FLAG_ADD_DST|RME_CAPTBL_FLAG_REM|
                           RME_CAPTBL_FLAG_PROC_CRT|RME_CAPTBL_FLAG_PROC_CPT;
    Captbl_Crt->Entry_Num=Entry_Num;
    /* At last, write into slot the correct information, and clear the frozen bit */
    Captbl_Crt->Head.Type_Ref=RME_CAP_TYPEREF(RME_CAP_CAPTBL,0);

    return 0;
}
/* End Function:_RME_Captbl_Crt **********************************************/

/* Begin Function:_RME_Captbl_Del *********************************************
Description : Delete a layer of capability table.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              cid_t Cap_Captbl_Del  - The capability table containing the cap to
                                      captbl for deletion. 2-Level.
              cid_t Cap_Del - The capability to the captbl being deleted. 1-Level.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t _RME_Captbl_Del(struct RME_Cap_Captbl* Captbl, cid_t Cap_Captbl_Del, cid_t Cap_Del)
{
    cnt_t Count;
    struct RME_Cap_Captbl* Captbl_Op;
    struct RME_Cap_Captbl* Captbl_Del;
    ptr_t Type_Ref;
    /* These are used for deletion */
    ptr_t Object;
    ptr_t Size;
    
    /* Get the capability slot */
    RME_CAPTBL_GETCAP(Captbl,Cap_Captbl_Del,RME_CAP_CAPTBL,struct RME_Cap_Captbl*,Captbl_Op);    
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Captbl_Op,RME_CAPTBL_FLAG_DEL);
    
    /* Get the cap slot */
    RME_CAPTBL_GETSLOT(Captbl_Op,Cap_Del,struct RME_Cap_Captbl*,Captbl_Del);
    /* Delete check */
    RME_CAP_DEL_CHECK(Captbl_Del,Type_Ref,RME_CAP_CAPTBL);
    
    /* Is there any capability in this capability table? If yes, we cannot destroy it.
     * We will check every slot to make sure nothing is there. This is surely,
     * predictable but not so perfect. So, if the time of such operations is to be 
     * bounded, the user must control the number of entries in the table */
    for(Count=0;Count<Captbl_Del->Entry_Num;Count++)
    {
        if(RME_CAP_TYPE(RME_CAP_GETOBJ(Captbl_Del,struct RME_Cap_Struct*)[Count].Head.Type_Ref)!=RME_CAP_NOP)
        {
            RME_CAP_DEFROST(Captbl_Del,Type_Ref);
            return RME_ERR_CAP_EXIST;
        }
    }
    
    /* Remember these two variables for deletion */
    Object=RME_CAP_GETOBJ(Captbl_Del,ptr_t);
    Size=RME_CAPTBL_SIZE(Captbl_Del->Entry_Num);
    
    /* Now we can safely delete the cap */
    RME_CAP_REMDEL(Captbl_Del,Type_Ref);
    /* Try to depopulate the area - this must be successful */
    RME_ASSERT(_RME_Kotbl_Erase(Object,Size)!=0);
    
    return 0;
}
/* End Function:_RME_Captbl_Del **********************************************/

/* Begin Function:_RME_Captbl_Frz *********************************************
Description : Freeze a capability in the capability table.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              cid_t Cap_Captbl_Frz  - The capability table containing the cap to
                                      captbl for this operation. 2-Level.
              cid_t Cap_Frz - The cap to the kernel object being freezed. 1-Level.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t _RME_Captbl_Frz(struct RME_Cap_Captbl* Captbl, cid_t Cap_Captbl_Frz, cid_t Cap_Frz)
{
    struct RME_Cap_Captbl* Captbl_Op;
    struct RME_Cap_Struct* Captbl_Frz;
    ptr_t Type_Ref;
    
    /* Get the capability slot */
    RME_CAPTBL_GETCAP(Captbl,Cap_Captbl_Frz,RME_CAP_CAPTBL,struct RME_Cap_Captbl*,Captbl_Op);    
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Captbl_Op,RME_CAPTBL_FLAG_FRZ);
    
    /* Get the cap slot */
    RME_CAPTBL_GETSLOT(Captbl_Op,Cap_Frz,struct RME_Cap_Struct*,Captbl_Frz);
    /* Check if anything is there. If nothing there, the Type_Ref must be 0 */
    Type_Ref=Captbl_Frz->Head.Type_Ref;
    /* See if there is a cap */
    if(RME_CAP_TYPE(Type_Ref)==RME_CAP_NOP)
        return RME_ERR_CAP_NULL;
    /* The reference count does not allow freezing */
    if(RME_CAP_REF(Type_Ref)!=0)
        return RME_ERR_CAP_REFCNT;
    /* The capability is already frozen - why do it again? */
    if((Type_Ref&RME_CAP_FROZEN)!=0)
        return RME_ERR_CAP_FROZEN;
    
    /* Update the timestamp */
    Captbl_Frz->Head.Timestamp=RME_Timestamp;
    
    /* Finally, freeze it */
    if(__RME_Comp_Swap(&(Captbl_Frz->Head.Type_Ref),&Type_Ref,Type_Ref|RME_CAPTBL_FLAG_FRZ)==0)
        return RME_ERR_CAP_EXIST;
    
    return 0;
}
/* End Function:_RME_Captbl_Frz **********************************************/

/* Begin Function:_RME_Captbl_Add *********************************************
Description : Add one capability into the capability table. This is doing capability
              delegation.
Input       : struct RME_Cap_Captbl* Captbl - The capability to the master capability table.
              cid_t Cap_Captbl_Dst - The capability to the destination capability table. 2-Level.
              cid_t Cap_Dst - The capability slot you want to add to. 1-Level.
              cid_t Cap_Captbl_Src - The capability to the source capability table. 2-Level.
              cid_t Cap_Src - The capability in the source capability table to delegate. 1-Level.
              ptr_t Flags - The flags to delegate. The flags can restrict which operations
                            are possible on the cap. If the cap delegated is a page table, we also
                            pass the range information in this field.
              ptr_t Ext_Flags - The extended flags, only effective for kernel memory capability.
Output      : None.
Return      : ret_t - If the mapping is successful, it will return 0; else error code.
******************************************************************************/
ret_t _RME_Captbl_Add(struct RME_Cap_Captbl* Captbl,
                      cid_t Cap_Captbl_Dst, cid_t Cap_Dst, 
                      cid_t Cap_Captbl_Src, cid_t Cap_Src,
                      ptr_t Flags, ptr_t Ext_Flags)
{
    struct RME_Cap_Captbl* Captbl_Dst;
    struct RME_Cap_Captbl* Captbl_Src;
    struct RME_Cap_Struct* Cap_Dst_Struct;
    struct RME_Cap_Struct* Cap_Src_Struct;
    ptr_t Type_Ref;
    
    /* These two variables are only used for kernel memory checks */
    ptr_t Kmem_End;
    ptr_t Kmem_Start;
    ptr_t Kmem_Flags;

    /* Get the capability slots */
    RME_CAPTBL_GETCAP(Captbl,Cap_Captbl_Dst,RME_CAP_CAPTBL,struct RME_Cap_Captbl*,Captbl_Dst);
    RME_CAPTBL_GETCAP(Captbl,Cap_Captbl_Src,RME_CAP_CAPTBL,struct RME_Cap_Captbl*,Captbl_Src);
    /* Check if both captbls are not frozen and allows such operations */
    RME_CAP_CHECK(Captbl_Dst,RME_CAPTBL_FLAG_ADD_DST);
    RME_CAP_CHECK(Captbl_Src,RME_CAPTBL_FLAG_ADD_SRC);
    
    /* Get the cap slots */
    RME_CAPTBL_GETSLOT(Captbl_Dst,Cap_Dst,struct RME_Cap_Struct*,Cap_Dst_Struct);
    RME_CAPTBL_GETSLOT(Captbl_Src,Cap_Src,struct RME_Cap_Struct*,Cap_Src_Struct);
    
    /* Does the source cap exist, and not freezed? */
    if(Cap_Src_Struct->Head.Type_Ref==0)
        return RME_ERR_CAP_NULL;
    if(((Cap_Src_Struct->Head.Type_Ref)&RME_CAP_FROZEN)!=0)
        return RME_ERR_CAP_FROZEN;
    
    /* Is there a flag conflict? - For page tables, we have different checking mechanisms */
    if(RME_CAP_TYPE(Cap_Src_Struct->Head.Type_Ref)==RME_CAP_PGTBL)
    {
        /* Check the delegation range */
        if(RME_PGTBL_FLAG_HIGH(Flags)>RME_PGTBL_FLAG_HIGH(Cap_Src_Struct->Head.Flags))
            return RME_ERR_CAP_FLAG;
        if(RME_PGTBL_FLAG_LOW(Flags)<RME_PGTBL_FLAG_LOW(Cap_Src_Struct->Head.Flags))
            return RME_ERR_CAP_FLAG;
        if(RME_PGTBL_FLAG_HIGH(Flags)<RME_PGTBL_FLAG_LOW(Flags))
            return RME_ERR_CAP_FLAG;
        /* Check the flags - if there are extra ones, or all zero */
        if(RME_PGTBL_FLAG_FLAGS(Flags)==0)
            return RME_ERR_CAP_FLAG;
        if((RME_PGTBL_FLAG_FLAGS(Flags)&(~RME_PGTBL_FLAG_FLAGS(Cap_Src_Struct->Head.Flags)))!=0)
            return RME_ERR_CAP_FLAG;
    }
    else if(RME_CAP_TYPE(Cap_Src_Struct->Head.Type_Ref)==RME_CAP_KERN)
    {
        /* Kernel capabilities only have ranges, no flags - check the delegation range */
        /* Check the delegation range */
        if(RME_KERN_FLAG_HIGH(Flags)>RME_KERN_FLAG_HIGH(Cap_Src_Struct->Head.Flags))
            return RME_ERR_CAP_FLAG;
        if(RME_KERN_FLAG_LOW(Flags)<RME_KERN_FLAG_LOW(Cap_Src_Struct->Head.Flags))
            return RME_ERR_CAP_FLAG;
        if(RME_KERN_FLAG_HIGH(Flags)<RME_KERN_FLAG_LOW(Flags))
            return RME_ERR_CAP_FLAG;
    }
    else if(RME_CAP_TYPE(Cap_Src_Struct->Head.Type_Ref)==RME_CAP_KMEM)
    {
        Kmem_End=RME_KMEM_FLAG_HIGH(Flags,Ext_Flags);
        Kmem_Start=RME_KMEM_FLAG_LOW(Flags,Ext_Flags);
        Kmem_Flags=RME_KMEM_FLAG_FLAGS(Ext_Flags);
        
        /* Round start and end to the slot boundary, if we are using slots bigger than 64 bytes */
#if(RME_KMEM_SLOT_ORDER>6)
        Kmem_End=RME_ROUND_DOWN(Kmem_End,RME_KMEM_SLOT_ORDER);
        Kmem_Start=RME_ROUND_UP(Kmem_Start,RME_KMEM_SLOT_ORDER);
#endif
        if(Kmem_End==Kmem_Start)
            return RME_ERR_CAP_FLAG;
        /* Kernel memory capabilities have ranges and flags - check both, and we use extended flags */
        if(((struct RME_Cap_Kmem*)Cap_Src_Struct)->Start>Kmem_Start)
            return RME_ERR_CAP_FLAG;
        if(((struct RME_Cap_Kmem*)Cap_Src_Struct)->End<(Kmem_End-1))
            return RME_ERR_CAP_FLAG;
        /* Check the flags - if there are extra ones, or all zero */
        if(Kmem_Flags==0)
            return RME_ERR_CAP_FLAG;
        if((Kmem_Flags&(~(Cap_Src_Struct->Head.Flags)))!=0)
            return RME_ERR_CAP_FLAG;
    }
    else
    {
        /* Check the flags - if there are extra ones, or all zero */
        if(Flags==0)
            return RME_ERR_CAP_FLAG;
        if((Flags&(~(Cap_Src_Struct->Head.Flags)))!=0)
            return RME_ERR_CAP_FLAG;
    }
    /* Is the destination slot unoccupied, and is quiescent? */
    if(Cap_Dst_Struct->Head.Type_Ref!=0)
        return RME_ERR_CAP_EXIST;
    if(RME_CAP_QUIE(Cap_Dst_Struct->Head.Timestamp)==0)
        return RME_ERR_CAP_QUIE;
    
    /* Try to take the empty slot */
    RME_CAPTBL_OCCUPY(Cap_Dst_Struct,Type_Ref);
    
    /* All done, we replicate the cap with flags */
    if(RME_CAP_TYPE(Cap_Src_Struct->Head.Type_Ref)==RME_CAP_KMEM)
    {
        RME_CAP_COPY(Cap_Dst_Struct,Cap_Src_Struct,Kmem_Flags);
        /* If this is a kernel memory cap, we need to write the range information as well */
        ((struct RME_Cap_Kmem*)Cap_Dst_Struct)->Start=Kmem_Start;
        ((struct RME_Cap_Kmem*)Cap_Dst_Struct)->End=Kmem_End-1;
    }
    else
        RME_CAP_COPY(Cap_Dst_Struct,Cap_Src_Struct,Flags);
    
    /* Set the parent */
    Cap_Dst_Struct->Head.Parent=(ptr_t)Cap_Src_Struct;
    /* Set the parent's reference count */
    Type_Ref=__RME_Fetch_Add(&(Cap_Src_Struct->Head.Type_Ref), 1);
    /* Is it overflowed? */
    if(RME_CAP_REF(Type_Ref)>=RME_CAP_MAXREF)
    {
        /* Refcnt overflowed(very unlikely to happen) */
        __RME_Fetch_Add(&(Cap_Src_Struct->Head.Type_Ref), -1);
        /* Clear the taken slot as well */
        Cap_Dst_Struct->Head.Type_Ref=0;
        return RME_ERR_CAP_REFCNT;
    }
    /* Write in the correct information */
    Cap_Dst_Struct->Head.Type_Ref=RME_CAP_TYPEREF(RME_CAP_TYPE(Cap_Src_Struct->Head.Type_Ref),0);
    
    return 0;
}
/* End Function:_RME_Captbl_Add **********************************************/

/* Begin Function:_RME_Captbl_Rem *********************************************
Description : Remove one capability from the capability table. This function reverts
              the delegation.
Input       : struct RME_Cap_Captbl* Captbl - The master capability table.
              cid_t Cap_Captbl_Rem - The capability to the capability table to 
                                     remove from. 2-Level.
              cid_t Cap_Rem - The capability slot you want to remove. 1-Level.
Output      : None.
Return      : ret_t - If the mapping is successful, it will return 0; else error code.
******************************************************************************/
ret_t _RME_Captbl_Rem(struct RME_Cap_Captbl* Captbl, cid_t Cap_Captbl_Rem, cid_t Cap_Rem)
{
    struct RME_Cap_Captbl* Captbl_Op;
    struct RME_Cap_Struct* Captbl_Rem;
    ptr_t Type_Ref;
    /* This is used for removal */
    struct RME_Cap_Struct* Parent;
    
    /* Get the capability slot */
    RME_CAPTBL_GETCAP(Captbl,Cap_Captbl_Rem,RME_CAP_CAPTBL,struct RME_Cap_Captbl*,Captbl_Op);    
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Captbl_Op,RME_CAPTBL_FLAG_REM);
    
    /* Get the cap slot */
    RME_CAPTBL_GETSLOT(Captbl_Op,Cap_Rem,struct RME_Cap_Struct*,Captbl_Rem);
    /* Removal check */
    RME_CAP_REM_CHECK(Captbl_Rem,Type_Ref);
    /* Remember this for refcnt operations */
    Parent=(struct RME_Cap_Struct*)(Captbl_Rem->Head.Parent);
    /* Remove the cap */
    RME_CAP_REMDEL(Captbl_Rem,Type_Ref);
    
    /* Check done, decrease its parent's refcnt */
    __RME_Fetch_Add(&(Parent->Head.Type_Ref), -1);
    
    return 0;
}
/* End Function:_RME_Captbl_Rem **********************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
