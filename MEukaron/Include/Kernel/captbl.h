/******************************************************************************
Filename    : captbl.h
Author      : pry
Date        : 01/04/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The header of type table. All the branches in these macros are
              pretty much optimized with RME_LIKELY and (mainly )RME_UNLIKELY.
              Due to the huge penalty of a wrong hint, only error handling in
              headers, synchronous invocation activation and synchronous invocation
              returning are heavily optimized.
******************************************************************************/

/* Defines *******************************************************************/
#ifdef __HDR_DEFS__
#ifndef __CAPTBL_H_DEFS__
#define __CAPTBL_H_DEFS__
/*****************************************************************************/
/* This capability is empty and is basically nothing */
#define RME_CAP_NOP                 0
/* Kernel function */
#define RME_CAP_KERN                1
/* Kernel memory */
#define RME_CAP_KMEM                2
/* Capability table */
#define RME_CAP_CAPTBL              3
/* Page table */
#define RME_CAP_PGTBL               4
/* Process */
#define RME_CAP_PROC                5
/* Thread */
#define RME_CAP_THD                 6
/* Synchronous invocation */
#define RME_CAP_INV                 7
/* Asynchronous signal */
#define RME_CAP_SIG                 8

/* This capability is currently freezed, and new operations cannot be initiated on it */
#define RME_CAP_FROZEN              (((ptr_t)1)<<((sizeof(ptr_t)*6)-1))

/* Capability size macro */
#define RME_CAP_SIZE                (8*sizeof(ptr_t))
/* Capability table size calculation macro */
#define RME_CAPTBL_SIZE(NUM)        (sizeof(struct RME_Cap_Struct)*(NUM))
/* The operation inline macros on the capabilities */
/* Refcnt_Type:example for 32-bit and 64-bit systems
 * 32-bit system:
 * [31    Type   24][23 Frozen][22                 Refcnt                    0]
 * 64-bit system:
 * [63    Type   48][47 Frozen][46                 Refcnt                    0]
 * Refcnt is used to track delegation.
 * Frozen is a fine-grained lock to lock the entries involved so that no parallel
 *        create/destroy/alteration operations on them can be done. If one of the locks failed, we
 *        will give up all the locks, and retry.
 * Type is a field denoting what is it.
 */
#define RME_CAP_REF_MASK           (RME_MASK_END((sizeof(ptr_t)*6)-2))
#define RME_CAP_MAXREF             (RME_CAP_REF_MASK>>1)
#define RME_CAP_TYPEREF(TYPE,REF)  ((((ptr_t)(TYPE))<<(sizeof(ptr_t)*6))|(REF))
#define RME_CAP_TYPE(X)            ((X)>>(sizeof(ptr_t)*6))
#define RME_CAP_REF(X)             ((X)&RME_CAP_REF_MASK)
/* Is this cap quiescent? Yes-1, No-0 */
#if(RME_QUIE_TIME!=0)
#if(RME_WORD_ORDER==5)
/* If this is a 32-bit system, need to consider overflows */
#define RME_CAP_QUIE(X)            (((RME_Timestamp-(X))>(X)-RME_Timestamp)? \
                                    (((X)-RME_Timestamp)>RME_QUIE_TIME): \
                                    ((RME_Timestamp-(X))>RME_QUIE_TIME))
#else
#define RME_CAP_QUIE(X)            ((RME_Timestamp-(X))>RME_QUIE_TIME)
#endif
#else
#define RME_CAP_QUIE(X)            (1)
#endif
/* Get the object */
#define RME_CAP_GETOBJ(X,TYPE)     ((TYPE)((X)->Head.Object))
/* 1-layer capid addressing:
 * 32-bit systems: Capid range 0x00 - 0x7F
 * [15             Reserved             8][7  2L(0)][6    Table(Master)   0]
 * 64-bit systems: Capid range 0x0000 - 0x7FFF
 * [31             Reserved            16][15 2L(0)][14   Table(Master)   0]
 * 2-layer capid addressing:
 * 32-bit systems: Capid range 0x00 - 0x7F
 * [15 Reserved][14 High Table(Master)  8][7  2L(1)][6   Low Table(Child) 0]
 * 64-bit systems: Capid range 0x0000 - 0x7FFF
 * [31 Reserved][30 High Table(Master) 16][15 2L(1)][14  Low Table(Child) 0] */
#define RME_CAPID_NULL             (((cid_t)1)<<(sizeof(ptr_t)*4-1))
/* See if the capid is a 2-level representation */
#define RME_CAPID_2L               (((cid_t)1)<<(sizeof(ptr_t)*2-1))
/* High-level capability table capability position */
#define RME_CAP_H(X)               ((X)>>(sizeof(ptr_t)*2))
/* Low-level capability table capability position */
#define RME_CAP_L(X)               ((X)&RME_MASK_END(sizeof(ptr_t)*2-2))

/* When we are clearing capabilities */
#define RME_CAP_CLEAR(X) \
do \
{ \
    /* Do this at last lest that some overlapping operations may happen */ \
    (X)->Head.Type_Ref=0; \
} \
while(0)

/* Replicate the capability. The Type_Ref field is filled in in the last step 
 * of capability creation; the Parent field is also populated in the creation
 * process appropriately. Thus, they are not filled in here.
 * DST - The pointer to the destination slot.
 * SRC - The pointer to the source slot.
 * FLAGS - The new operation flags of this capability. */
#define RME_CAP_COPY(DST,SRC,FLAGS) \
do \
{ \
    /* The suboperation capability flags */ \
    (DST)->Head.Flags=(FLAGS); \
    /* The object address */ \
    (DST)->Head.Object=(SRC)->Head.Object; \
    (DST)->Info[0]=(SRC)->Info[0]; \
    (DST)->Info[1]=(SRC)->Info[1]; \
    (DST)->Info[2]=(SRC)->Info[2]; \
} \
while(0)

/* Check if the capability is ready for some operations.
 * CAP - The pointer to the capability slot to check.
 * FLAG - The operation flags to check against the slot. */
#define RME_CAP_CHECK(CAP,FLAG) \
do \
{ \
    /* See if this capability allows such operations */ \
    if(RME_UNLIKELY(((CAP)->Head.Flags&(FLAG))!=(FLAG))) \
        return RME_ERR_CAP_FLAG; \
} \
while(0)

/* Check if the kernel memory capability range is valid.
 * CAP - The kernel memory capability to check.
 * FLAG - The flags to check against the kernel memory capability.
 * START - The start address of the kernel memory.
 * SIZE - The size of the kernel memory trunk. */
#define RME_KMEM_CHECK(CAP,FLAG,START,SIZE) \
do \
{ \
    /* See if the creation of such capability is allowed */ \
    if(RME_UNLIKELY(((CAP)->Head.Flags&(FLAG))!=(FLAG))) \
        return RME_ERR_CAP_FLAG; \
    /* The end is always aligned to 256 bytes in the kernel, and does not include the ending byte */ \
    if(RME_UNLIKELY(((CAP)->Start>(START))||((CAP)->End<((START)+(SIZE))))) \
        return RME_ERR_CAP_FLAG; \
} \
while(0)

/* Defrost a frozen cap - we do not check failure because if we fail, someone must 
 * have done it for us. The reason why we need to defrost a capability after we decide
 * that it is unfit for deletion or removal is that the capability may later be used in
 * kernel object deallocation operations. If we keep it freezed, we can't deallocate
 * other resources that may depend on it.
 * CAP - The pointer to the capability slot to defreeze.
 * TEMP - A temporary variable, for compare-and-swap. */
#define RME_CAP_DEFROST(CAP,TEMP) \
do \
{ \
    __RME_Comp_Swap(&((CAP)->Head.Type_Ref),&(TEMP),(TEMP)&(~((ptr_t)RME_CAP_FROZEN))); \
} \
while(0)

/* Checks to be done before deleting - the barrier is for preventing stale timestamp
 * before the FROZEN bit is set under read reordering situations. Different from a removal
 * check, the type check is also performed against the slot.
 * CAP - The pointer to the capability slot to check for deletion.
 * TEMP - A temporary variable, for compare-and-swap. 
 * TYPE - What type should we anticipate when we check against the slot? */
#define RME_CAP_DEL_CHECK(CAP,TEMP,TYPE) \
do \
{ \
    /* Atomic read - Need a read acquire barrier here to avoid stale reads below */ \
    (TEMP)=RME_READ_ACQUIRE(&((CAP)->Head.Type_Ref)); \
    /* See if the slot is frozen, and its cap must be non-zero */ \
    if(RME_UNLIKELY(((TEMP)&RME_CAP_FROZEN)!=0)) \
        return RME_ERR_CAP_FROZEN; \
    /* See if we are in the creation/delegation process - This frozen flag is set by the creator */ \
    if(RME_UNLIKELY(RME_CAP_TYPE(TEMP)==RME_CAP_NOP)) \
        return RME_ERR_CAP_NULL; \
    /* See if the cap type is correct. Only deletion checks type, while removing does not */ \
    if(RME_UNLIKELY(RME_CAP_TYPE(TEMP)!=(TYPE))) \
        return RME_ERR_CAP_TYPE; \
    /* See if the slot is quiescent */ \
    if(RME_UNLIKELY(RME_CAP_QUIE((CAP)->Head.Timestamp)==0)) \
        return RME_ERR_CAP_QUIE; \
    /* To use deletion, we must be an unreferenced root */ \
    if(RME_UNLIKELY((RME_CAP_REF(TEMP)!=0)||(((CAP)->Head.Parent)!=0))) \
    { \
        /* We defrost the cap and return. Need cas, in case two competing deletions happen */ \
        RME_CAP_DEFROST(CAP,TEMP); \
        return RME_ERR_CAP_REFCNT; \
    } \
} \
while(0)
    
/* Checks to be done before removal - the barrier is for preventing stale timestamp
 * before the FROZEN bit is set under read reordering situations. Different from a 
 * deletion check, the type of the slot will not be checked.
 * CAP - The pointer to the capability slot to check for removal.
 * TEMP - A temporary variable, for compare-and-swap. */
#define RME_CAP_REM_CHECK(CAP,TEMP) \
do \
{ \
    /* Atomic read - Need a read acquire barrier here to avoid stale reads below */ \
    (TEMP)=RME_READ_ACQUIRE(&((CAP)->Head.Type_Ref)); \
    /* See if the slot is frozen, and its cap must be non-zero */ \
    if(RME_UNLIKELY(((TEMP)&RME_CAP_FROZEN)!=0)) \
        return RME_ERR_CAP_FROZEN; \
    /* See if we are in the creation/delegation process - This frozen flag is set by the creator */ \
    if(RME_UNLIKELY(RME_CAP_TYPE(TEMP)==RME_CAP_NOP)) \
        return RME_ERR_CAP_NULL; \
    /* See if the slot is quiescent */ \
    if(RME_UNLIKELY(RME_CAP_QUIE((CAP)->Head.Timestamp)==0)) \
        return RME_ERR_CAP_QUIE; \
    /* To use removal, we must be an unreferenced child */ \
    if(RME_UNLIKELY((RME_CAP_REF(TEMP)!=0)||(((CAP)->Head.Parent)==0))) \
    { \
        /* We defrost the cap and return. Need cas, in case two competing removals happen */ \
        RME_CAP_DEFROST(CAP,TEMP); \
        return RME_ERR_CAP_REFCNT; \
    } \
} \
while(0)

/* Actually remove/delete the cap.
 * CAP - The pointer to the capability slot to delete or remove.
 * TEMP - A temporary variable, for compare-and-swap. */
#define RME_CAP_REMDEL(CAP,TEMP) \
do \
{ \
    /* If this fails, then it means that somebody have deleted/removed it first */ \
    if(RME_UNLIKELY(__RME_Comp_Swap(&((CAP)->Head.Type_Ref),&(TEMP),0)==0)) \
        return RME_ERR_CAP_NULL; \
} \
while(0)

/* Check if we can take the slot, if we can, just take it. This also updates the timestamp,
 * so that we can enforce creation-freezing quiescence.
 * CAP - The pointer to the capability slot to occupy.
 * TEMP - A temporary variable, for compare-and-swap. */
#define RME_CAPTBL_OCCUPY(CAP,TEMP) \
do \
{ \
    /* Check if anything is there. If there is nothing there, the Type_Ref must be 0 */ \
    (TEMP)=RME_CAP_TYPEREF(RME_CAP_NOP,0); \
    if(RME_UNLIKELY(__RME_Comp_Swap(&((CAP)->Head.Type_Ref),&(TEMP),RME_CAP_FROZEN)==0)) \
        return RME_ERR_CAP_EXIST; \
    /* We have taken the slot. Now log the quiescence counter in. No barrier needed as our atomics are serializing */ \
    (CAP)->Head.Timestamp=RME_Timestamp; \
} \
while(0)

/* Get the capability slot from the master table according to a 1-level encoding.
 * This will not check the validity of the slot; nor will it try to resolve 2-level
 * encodings.
 * CAPTBL - The master capability table.
 * CAP_NUM - The capability number. Only allows 1-level encoding.
 * TYPE -  When assigning the pointer to the PARAM, what struct pointer type should I cast it into?
 * PARAM - The parameter to receive the pointer to the found capability slot. */
#define RME_CAPTBL_GETSLOT(CAPTBL,CAP_NUM,TYPE,PARAM) \
do \
{ \
    /* Check if the captbl is over range */ \
    if(RME_UNLIKELY((CAP_NUM)>=((CAPTBL)->Entry_Num))) \
        return RME_ERR_CAP_RANGE; \
    /* Get the slot position */ \
    (PARAM)=&(RME_CAP_GETOBJ((CAPTBL),TYPE)[(CAP_NUM)]); \
} \
while(0)

/* Get the capability from the master table according to capability number encoding.
 * The read acquire is for handling the case where the other CPU is creating
 * the capability. Suppose we are now operating on an empty slot, and the other CPU
 * have not started the creation yet. If we allow the other operation that follow the
 * TYPE check to reorder with it, then we are checking something that is not even
 * created at all.(and by the time type check is taking place, the creation finishes
 * so it passes). This also will cause a race condition.
 * CAPTBL - The current master capability table.
 * CAP_NUM - The capability number. Allows 1- and 2-level encodings.
 * CAP_TYPE - The type of the capability, i.e. CAP_CAPTBL or CAP_PGTBL or CAP_THD, etc?
 * TYPE - When assigning the pointer to the PARAM, what struct pointer type should I cast it into?
 * PARAM - The parameter to receive the pointer to the found capability slot.
 * TEMP - A temporary variable, used to atomically keep a snapshot of Type_Ref when checking.
 */
#define RME_CAPTBL_GETCAP(CAPTBL,CAP_NUM,CAP_TYPE,TYPE,PARAM,TEMP) \
do \
{ \
    /* See if this is a 2-level cap */ \
    if(((CAP_NUM)&RME_CAPID_2L)==0) \
    { \
        /* Check if the captbl is over range */ \
        if(RME_UNLIKELY((CAP_NUM)>=((CAPTBL)->Entry_Num))) \
            return RME_ERR_CAP_RANGE; \
        /* Get the cap slot and check the type */ \
        (PARAM)=(TYPE)(&RME_CAP_GETOBJ(CAPTBL,struct RME_Cap_Struct*)[(CAP_NUM)]); \
        /* Atomic read - Need a read acquire barrier here to avoid stale reads below */ \
        (TEMP)=RME_READ_ACQUIRE(&((PARAM)->Head.Type_Ref)); \
        /* See if the capability is frozen */ \
        if(RME_UNLIKELY(((TEMP)&RME_CAP_FROZEN)!=0)) \
            return RME_ERR_CAP_FROZEN; \
        if(RME_UNLIKELY(RME_CAP_TYPE(TEMP)!=(CAP_TYPE))) \
            return RME_ERR_CAP_TYPE; \
    } \
    /* Yes, this is a 2-level cap */ \
    else \
    { \
        /* Check if the cap to potential captbl is over range */ \
        if(RME_UNLIKELY(RME_CAP_H(CAP_NUM)>=((CAPTBL)->Entry_Num))) \
            return RME_ERR_CAP_RANGE; \
        /* Get the cap slot */ \
        (PARAM)=(TYPE)(&RME_CAP_GETOBJ(CAPTBL,struct RME_Cap_Captbl*)[RME_CAP_H(CAP_NUM)]); \
        /* Atomic read - Need a read acquire barrier here to avoid stale reads below */ \
        (TEMP)=RME_READ_ACQUIRE(&((PARAM)->Head.Type_Ref)); \
        /* See if the captbl is frozen for deletion or removal */ \
        if(RME_UNLIKELY(((TEMP)&RME_CAP_FROZEN)!=0)) \
            return RME_ERR_CAP_FROZEN; \
        /* See if this is a captbl */ \
        if(RME_UNLIKELY(RME_CAP_TYPE(TEMP)!=RME_CAP_CAPTBL)) \
            return RME_ERR_CAP_TYPE; \
        /* Check if the 2nd-layer captbl is over range */ \
        if(RME_UNLIKELY(RME_CAP_L(CAP_NUM)>=(((struct RME_Cap_Captbl*)(PARAM))->Entry_Num))) \
            return RME_ERR_CAP_RANGE; \
        /* Get the cap slot and check the type */ \
        (PARAM)=(TYPE)(&RME_CAP_GETOBJ(PARAM,struct RME_Cap_Struct*)[RME_CAP_L(CAP_NUM)]); \
        /* Atomic read - Need a read acquire barrier here to avoid stale reads below */ \
        (TEMP)=RME_READ_ACQUIRE(&((PARAM)->Head.Type_Ref)); \
        /* See if the capability is frozen */ \
        if(RME_UNLIKELY(((TEMP)&RME_CAP_FROZEN)!=0)) \
            return RME_ERR_CAP_FROZEN; \
        if(RME_UNLIKELY(RME_CAP_TYPE(TEMP)!=(CAP_TYPE))) \
            return RME_ERR_CAP_TYPE; \
    } \
} \
while(0)
/*****************************************************************************/
/* __CAPTBL_H_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Structs *******************************************************************/
#ifdef __HDR_STRUCTS__
#ifndef __CAPTBL_H_STRUCTS__
#define __CAPTBL_H_STRUCTS__
/* We used structs in the header */

/* Use defines in these headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__
/*****************************************************************************/
/* The capability header */
struct RME_Cap_Head
{
    /* The type, freeze and the reference count. The reference count
     * is used to manage delegation only. */
    ptr_t Type_Ref;
    /* The parent capability(we delegated which one to here?) */
    ptr_t Parent;
    /* The suboperation capability flags */
    ptr_t Flags;
    /* The object address */
    ptr_t Object;
    /* The freeze timestamp */
    ptr_t Timestamp;
};
/* The capability information structure */
struct RME_Cap_Struct
{
    struct RME_Cap_Head Head;
    
    ptr_t Info[3];
};

/* The capability to the capability table */
struct RME_Cap_Captbl
{
    struct RME_Cap_Head Head;
    /* The number of entries in this captbl */
    ptr_t Entry_Num;
    
    ptr_t Info[2];
};
/*****************************************************************************/
/* __CAPTBL_H_STRUCTS__ */
#endif
/* __HDR_STRUCTS__ */
#endif
/* End Structs ***************************************************************/

/* Private Global Variables **************************************************/
#if(!(defined __HDR_DEFS__||defined __HDR_STRUCTS__))
#ifndef __CAPTBL_MEMBERS__
#define __CAPTBL_MEMBERS__

/* In this way we can use the data structures and definitions in the headers */
#define __HDR_DEFS__

#undef __HDR_DEFS__

#define __HDR_STRUCTS__

#undef __HDR_STRUCTS__

/* If the header is not used in the public mode */
#ifndef __HDR_PUBLIC_MEMBERS__
/*****************************************************************************/

/*****************************************************************************/
/* End Private Global Variables **********************************************/

/* Private C Function Prototypes *********************************************/ 
/*****************************************************************************/

/*****************************************************************************/
#define __EXTERN__
/* End Private C Function Prototypes *****************************************/

/* Public Global Variables ***************************************************/
/* __HDR_PUBLIC_MEMBERS__ */
#else
#define __EXTERN__ EXTERN 
/* __HDR_PUBLIC_MEMBERS__ */
#endif

/*****************************************************************************/

/*****************************************************************************/

/* End Public Global Variables ***********************************************/

/* Public C Function Prototypes **********************************************/
/*****************************************************************************/
__EXTERN__ ret_t _RME_Captbl_Boot_Init(cid_t Cap_Captbl, ptr_t Vaddr, ptr_t Entry_Num);
__EXTERN__ ret_t _RME_Captbl_Boot_Crt(struct RME_Cap_Captbl* Captbl, cid_t Cap_Captbl_Crt,
                                      cid_t Cap_Crt, ptr_t Vaddr, ptr_t Entry_Num);
__EXTERN__ ret_t _RME_Captbl_Crt(struct RME_Cap_Captbl* Captbl, cid_t Cap_Captbl_Crt, 
                                 cid_t Cap_Kmem, cid_t Cap_Crt, ptr_t Vaddr, ptr_t Entry_Num);
__EXTERN__ ret_t _RME_Captbl_Del(struct RME_Cap_Captbl* Captbl, cid_t Cap_Captbl_Del, cid_t Cap_Del);
__EXTERN__ ret_t _RME_Captbl_Frz(struct RME_Cap_Captbl* Captbl, cid_t Cap_Captbl_Frz, cid_t Cap_Frz);
__EXTERN__ ret_t _RME_Captbl_Add(struct RME_Cap_Captbl* Captbl,
                                 cid_t Cap_Captbl_Dst, cid_t Cap_Dst, 
                                 cid_t Cap_Captbl_Src, cid_t Cap_Src,
                                 ptr_t Flags, ptr_t Ext_Flags);
__EXTERN__ ret_t _RME_Captbl_Rem(struct RME_Cap_Captbl* Captbl, cid_t Cap_Captbl_Rem, cid_t Cap_Rem);
/*****************************************************************************/
/* Undefine "__EXTERN__" to avoid redefinition */
#undef __EXTERN__
/* __CAPTBL_MEMBERS__ */
#endif
/* !(defined __HDR_DEFS__||defined __HDR_STRUCTS__) */
#endif
/* End Public C Function Prototypes ******************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
