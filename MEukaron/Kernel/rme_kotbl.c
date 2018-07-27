/******************************************************************************
Filename    : rme_kotbl.c
Author      : pry
Date        : 01/04/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The kernel object table for the RME RTOS.
******************************************************************************/

/* Includes ******************************************************************/
#define __HDR_DEFS__
#include "Platform/rme_platform.h"
#include "Kernel/rme_kernel.h"
#include "Kernel/rme_kotbl.h"
#undef __HDR_DEFS__

#define __HDR_STRUCTS__
#include "Platform/rme_platform.h"
#include "Kernel/rme_captbl.h"
#include "Kernel/rme_pgtbl.h"
#include "Kernel/rme_kotbl.h"
#undef __HDR_STRUCTS__

/* Private include */
#include "Kernel/rme_kotbl.h"

#define __HDR_PUBLIC_MEMBERS__
#include "Platform/rme_platform.h"
#include "Kernel/rme_kernel.h"
#undef __HDR_PUBLIC_MEMBERS__
/* End Includes **************************************************************/

/* Begin Function:_RME_Kotbl_Init *********************************************
Description : Initialize the kernel object table according to the size of the table.
Input       : rme_ptr_t Words - the number of words in the table.
Output      : None.
Return      : rme_ret_t - Always 0.
******************************************************************************/
rme_ret_t _RME_Kotbl_Init(rme_ptr_t Words)
{
    rme_ptr_t Count;
    
    if(Words<RME_KOTBL_WORD_NUM)
        return -1;
    
    /* Avoid compiler warning about unused variable */
    RME_Kotbl[0]=0;

    /* Zero out the whole table */
    for(Count=0;Count<Words;Count++)
    	RME_KOTBL[Count]=0;
    
    return 0;
}
/* End Function:_RME_Kotbl_Init **********************************************/

/* Begin Function:_RME_Kotbl_Mark *********************************************
Description : Populate the kernel object bitmap contiguously.
Input       : rme_ptr_t Kaddr - The kernel virtual address.
              rme_ptr_t Size - The size of the memory to populate.
Output      : None.
Return      : rme_ret_t - If the operation is successful, it will return 0; else error code.
******************************************************************************/
rme_ret_t _RME_Kotbl_Mark(rme_ptr_t Kaddr, rme_ptr_t Size)
{
    /* The old value */
    rme_ptr_t Old_Val;
    /* Whether we need to undo our operations because of CAS failure */
    rme_ptr_t Undo;
    /* The actual word to start the marking */
    rme_ptr_t Start;
    /* The actual word to end the marking */
    rme_ptr_t End;
    /* The mask at the start word */
    rme_ptr_t Start_Mask;
    /* The mask at the end word */
    rme_ptr_t End_Mask;
    rme_ptr_t Count;

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
        if(RME_COMP_SWAP(&RME_KOTBL[Start],Old_Val,Old_Val|(Start_Mask&End_Mask))==0)
            return RME_ERR_KOT_BMP;
    }
    else
    {
        Undo=0;
        /* Check&Mark the start */
        Old_Val=RME_KOTBL[Start];
        if((Old_Val&Start_Mask)!=0)
            return RME_ERR_KOT_BMP;
        if(RME_COMP_SWAP(&RME_KOTBL[Start],Old_Val,Old_Val|Start_Mask)==0)
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
                if(RME_COMP_SWAP(&RME_KOTBL[Count],Old_Val,RME_ALLBITS)==0)
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
                if(RME_COMP_SWAP(&RME_KOTBL[End],Old_Val,Old_Val|End_Mask)==0)
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
            RME_FETCH_AND(&(RME_KOTBL[Start]),~Start_Mask);
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
Input       : rme_ptr_t Kaddr - The kernel virtual address.
              rme_ptr_t Size - The size of the memory to depopulate.
Output      : None.
Return      : rme_ret_t - If the operation is successful, it will return 0; else error code.
******************************************************************************/
rme_ret_t _RME_Kotbl_Erase(rme_ptr_t Kaddr, rme_ptr_t Size)
{
    /* The actual word to start the marking */
    rme_ptr_t Start;
    /* The actual word to end the marking */
    rme_ptr_t End;
    /* The mask at the start word */
    rme_ptr_t Start_Mask;
    /* The mask at the end word */
    rme_ptr_t End_Mask;
    rme_ptr_t Count;

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
        /* Check done, do the unmarking - need atomic operations */
        RME_FETCH_AND(&(RME_KOTBL[Start]),~(Start_Mask&End_Mask));
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
        RME_FETCH_AND(&(RME_KOTBL[Start]),~Start_Mask);
        /* Erase the middle - do not need atomics here */
        for(Count=Start+1;Count<End-1;Count++)
            RME_KOTBL[Count]=0;
        /* Erase the end - make it atomic */
        RME_FETCH_AND(&(RME_KOTBL[End]),~End_Mask);
    }
    
    return 0;
}
/* End Function:_RME_Kotbl_Erase *********************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
