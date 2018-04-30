/******************************************************************************
Filename    : kotbl.c
Author      : pry
Date        : 01/04/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The kernel object table for the RME RTOS.
******************************************************************************/

/* Includes ******************************************************************/
#define __HDR_DEFS__
#include "Platform/RME_platform.h"
#include "Kernel/kernel.h"
#include "Kernel/kotbl.h"
#undef __HDR_DEFS__

#define __HDR_STRUCTS__
#include "Platform/RME_platform.h"
#include "Kernel/captbl.h"
#include "Kernel/pgtbl.h"
#include "Kernel/kotbl.h"
#undef __HDR_STRUCTS__

/* Private include */
#include "Kernel/kotbl.h"

#define __HDR_PUBLIC_MEMBERS__
#include "Platform/RME_platform.h"
#include "Kernel/kernel.h"
#undef __HDR_PUBLIC_MEMBERS__
/* End Includes **************************************************************/

/* Begin Function:_RME_Kotbl_Init *********************************************
Description : Initialize the kernel object table according to the size of the table.
Input       : ptr_t Words - the number of words in the table.
Output      : None.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t _RME_Kotbl_Init(ptr_t Words)
{
    ptr_t Count;
    
    if(Words<RME_KOTBL_WORD_NUM)
        return -1;
    
    /* Zero out the whole table */
    for(Count=0;Count<Words;Count++)
        RME_KOTBL[Count]=0;
    
    return 0;
}
/* End Function:_RME_Kotbl_Init **********************************************/

/* Begin Function:_RME_Kotbl_Mark *********************************************
Description : Populate the kernel object bitmap contiguously.
Input       : ptr_t Kaddr - The kernel virtual address.
              ptr_t Size - The size of the memory to populate.
Output      : None.
Return      : ret_t - If the operation is successful, it will return 0; else error code.
******************************************************************************/
ret_t _RME_Kotbl_Mark(ptr_t Kaddr, ptr_t Size)
{
    /* The old value */
    ptr_t Old_Val;
    /* Whether we need to undo our operations because of CAS failure */
    ptr_t Undo;
    /* The actual word to start the marking */
    ptr_t Start;
    /* The actual word to end the marking */
    ptr_t End;
    /* The mask at the start word */
    ptr_t Start_Mask;
    /* The mask at the end word */
    ptr_t End_Mask;
    ptr_t Count;

    /* Check if the marking is well aligned */
    if((Kaddr&RME_MASK_END(RME_KMEM_SLOT_ORDER-1))!=0)
        return RME_ERR_KOT_BMP;
    /* Check if the marking is within range - unnecessary due to the kmem cap range limits */
    /* if((Kaddr<RME_KMEM_VA_START)||((Kaddr+Size)>(RME_KMEM_VA_START+RME_KMEM_SIZE)))
        return RME_ERR_KOT_BMP; */
    
    /* Round the marking to RME_KMEM_SLOT_ORDER boundary, and rely on compiler for optimization */
    Start=(Kaddr-RME_KMEM_VA_START)>>RME_KMEM_SLOT_ORDER;
    Start_Mask=RME_MASK_START(Start&RME_MASK_END(RME_WORD_ORDER-1));
    Start=Start>>RME_WORD_ORDER;
    
    End=(Kaddr+Size-1-RME_KMEM_VA_START)>>RME_KMEM_SLOT_ORDER;
    End_Mask=RME_MASK_END(End&RME_MASK_END(RME_WORD_ORDER-1));
    End=End>>RME_WORD_ORDER;

    /* See if the start and end are in the same word */
    if(Start==End)
    {
        /* Someone already populated something here */
        Old_Val=RME_KOTBL[Start];
        if((Old_Val&(Start_Mask&End_Mask))!=0)
            return RME_ERR_KOT_BMP;
        /* Check done, do the marking with CAS */
        if(__RME_Comp_Swap(&RME_KOTBL[Start],&Old_Val,Old_Val|(Start_Mask&End_Mask))==0)
            return RME_ERR_KOT_BMP;
    }
    else
    {
        Undo=0;
        /* Check&Mark the start */
        Old_Val=RME_KOTBL[Start];
        if((Old_Val&Start_Mask)!=0)
            return RME_ERR_KOT_BMP;
        if(__RME_Comp_Swap(&RME_KOTBL[Start],&Old_Val,Old_Val|Start_Mask)==0)
            return RME_ERR_KOT_BMP;
        /* Check&Mark the middle */
        for(Count=Start+1;Count<End;Count++)
        {
            Old_Val=RME_KOTBL[Count];
            if(Old_Val!=0)
            {
                Undo=1;
                break;
            }
            else
            {
                if(__RME_Comp_Swap(&RME_KOTBL[Count],&Old_Val,RME_ALLBITS)==0)
                {
                    Undo=1;
                    break;
                }
            }
        }
        /* See if the middle part failed. If yes, we skip the end marking */
        if(Undo==0)
        {
            /* Check&Mark the end */
            Old_Val=RME_KOTBL[End];
            if((Old_Val&End_Mask)!=0)
                Undo=1;
            else
            {
                if(__RME_Comp_Swap(&RME_KOTBL[End],&Old_Val,Old_Val|End_Mask)==0)
                    Undo=1;
            }
        }
        
        /* See if we need to undo. If yes, proceed to unroll and return error */
        if(Undo!=0)
        {
            /* Undo the middle part - we do not need CAS here, because write back is always atomic */
            for(Count--;Count>Start;Count--)
                RME_KOTBL[Count]=0;
            /* Undo the first word - need atomic instructions */
            __RME_Fetch_And(&(RME_KOTBL[Start]),~Start_Mask);
            /* Return failure */
            return RME_ERR_KOT_BMP;
        }
    }

    return 0;
}
/* End Function:_RME_Kotbl_Mark **********************************************/

/* Begin Function:_RME_Kotbl_Erase ********************************************
Description : Depopulate the kernel object bitmap contiguously. We do not need 
              CAS on erasure operations.
Input       : ptr_t Kaddr - The kernel virtual address.
              ptr_t Size - The size of the memory to depopulate.
Output      : None.
Return      : ret_t - If the operation is successful, it will return 0; else error code.
******************************************************************************/
ret_t _RME_Kotbl_Erase(ptr_t Kaddr, ptr_t Size)
{
    /* The actual word to start the marking */
    ptr_t Start;
    /* The actual word to end the marking */
    ptr_t End;
    /* The mask at the start word */
    ptr_t Start_Mask;
    /* The mask at the end word */
    ptr_t End_Mask;
    ptr_t Count;

    /* Check if the marking is well aligned */
    if((Kaddr&RME_MASK_END(RME_KMEM_SLOT_ORDER-1))!=0)
        return RME_ERR_KOT_BMP;
    
    /* Check if the marking is within range - unnecessary due to the kmem cap range limits */
    /* if((Kaddr<RME_KMEM_VA_START)||((Kaddr+Size)>(RME_KMEM_VA_START+RME_KMEM_SIZE)))
        return RME_ERR_KOT_BMP; */
    
    /* Round the marking to RME_KMEM_SLOT_ORDER boundary, and rely on compiler for optimization */
    Start=(Kaddr-RME_KMEM_VA_START)>>RME_KMEM_SLOT_ORDER;
    Start_Mask=RME_MASK_START(Start&RME_MASK_END(RME_WORD_ORDER-1));
    Start=Start>>RME_WORD_ORDER;
    
    End=(Kaddr+Size-1-RME_KMEM_VA_START)>>RME_KMEM_SLOT_ORDER;
    End_Mask=RME_MASK_END(End&RME_MASK_END(RME_WORD_ORDER-1));
    End=End>>RME_WORD_ORDER;
    
    /* See if the start and end are in the same word */
    if(Start==End)
    {
        /* This address range is not fully populated */
        if((RME_KOTBL[Start]&(Start_Mask&End_Mask))!=(Start_Mask&End_Mask))
            return RME_ERR_KOT_BMP;
        /* Check done, do the marking - need atomic operations */
        __RME_Fetch_And(&(RME_KOTBL[Start]),~(Start_Mask&End_Mask));
    }
    else
    {
        /* Check the start */
        if((RME_KOTBL[Start]&Start_Mask)!=Start_Mask)
            return RME_ERR_KOT_BMP;
        /* Check the middle */
        for(Count=Start+1;Count<End-1;Count++)
        {
            if(RME_KOTBL[Count]!=RME_ALLBITS)
                return RME_ERR_KOT_BMP;
        }
        /* Check the end */
        if((RME_KOTBL[End]&End_Mask)!=End_Mask)
            return RME_ERR_KOT_BMP;
        
        /* Erase the start - make it atomic */
        __RME_Fetch_And(&(RME_KOTBL[Start]),~Start_Mask);
        /* Erase the middle - do not need atomics here */
        for(Count=Start+1;Count<End-1;Count++)
            RME_KOTBL[Count]=0;
        /* Erase the end - make it atomic */
        __RME_Fetch_And(&(RME_KOTBL[End]),~End_Mask);
    }
    
    return 0;
}
/* End Function:_RME_Kotbl_Erase *********************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
