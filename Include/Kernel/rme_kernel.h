/******************************************************************************
Filename    : rme_kernel.h
Author      : pry
Date        : 08/04/2017
Licence     : The Unlicense; see LICENSE for details.
Description : The header of the kernel. Whitebox testing of all branches
              encapsulated in macros are inspected manually carefully.
              Note that the kernel global variables usually don't need to be 
              'volatile'-qualified (unlike the RMP library operating system)
              because these kernel calls are over the protection boundary. That
              way, there's no chance that the compiler can cache values in 
              registers since all kernel execution are short-lived in nature.
              It is indeed possible that, when the compilers are clever enough 
              (whole-program optimizations of GCC, etc.), optimize out the final
              stages of writes to memory, because this will not alter the 
              behavior of the abstract machine w.r.t. the current kernel trap.
              In this case, the 'volatile' is needed. Note that this cannot be
              replaced by the compiler barriers: they only tell the compiler
              that instruction shouldn't be reordered across the barrier, and
              don't mean that writes (that are provably useless in this
              particular kernel trap execution) cannot be optimized away.
******************************************************************************/

/* Define ********************************************************************/
#ifdef __HDR_DEF__
#ifndef __RME_KERNEL_DEF__
#define __RME_KERNEL_DEF__
/*****************************************************************************/
/* Generic *******************************************************************/
#define RME_NULL                                    (0U)
#define RME_EXIST                                   (1U)
#define RME_EMPTY                                   (0U)
#define RME_CASFAIL                                 (0U)

/* Dewarn compiler */
#define RME_USE(X)                                  ((void)(X))

/* Power of 2 */
#define RME_FIELD(VAL,POW)                          (((rme_ptr_t)(VAL))<<(POW))
#define RME_POW2(POW)                               RME_FIELD(1U,POW)
#define RME_IS_POW2(X)                              ((((rme_ptr_t)(X))&(((rme_ptr_t)(X))-1U))==0U)
/* Word size */
#define RME_WORD_BIT                                RME_POW2(RME_WORD_ORDER)
#define RME_WORD_BYTE                               (RME_WORD_BIT>>3)
#define RME_WORD_BIT_O1                             (RME_WORD_BYTE)
#define RME_WORD_BIT_O2                             (RME_WORD_BYTE*2U)
#define RME_WORD_BIT_O3                             (RME_WORD_BYTE*3U)
#define RME_WORD_BIT_O4                             (RME_WORD_BYTE*4U)
#define RME_WORD_BIT_O5                             (RME_WORD_BYTE*5U)
#define RME_WORD_BIT_O6                             (RME_WORD_BYTE*6U)
#define RME_WORD_BIT_O7                             (RME_WORD_BYTE*7U)
#define RME_WORD_BIT_Q1                             RME_WORD_BIT_O2
#define RME_WORD_BIT_Q2                             RME_WORD_BIT_O4
#define RME_WORD_BIT_Q3                             RME_WORD_BIT_O6
#define RME_WORD_BIT_D1                             RME_WORD_BIT_Q2
/* Bit mask */
#define RME_MASK_FULL                               (~((rme_ptr_t)0U))
#define RME_MASK_WORD                               (~(RME_MASK_FULL<<RME_WORD_ORDER))
#define RME_MASK_WORD_O                             RME_MASK_END(RME_WORD_BIT_O1-1U)
#define RME_MASK_WORD_Q                             RME_MASK_END(RME_WORD_BIT_Q1-1U)
#define RME_MASK_WORD_D                             RME_MASK_END(RME_WORD_BIT_D1-1U)
/* Mask to keep BEGIN to MSB bits */
#define RME_MASK_BEGIN(BEGIN)                       ((RME_MASK_FULL)<<(BEGIN))
/* Mask to keep LSB to END bits */
#define RME_MASK_END(END)                           ((RME_MASK_FULL)>>(RME_WORD_BIT-1U-(END)))
/* Mask to keep BEGIN to END bits, BEGIN < END */
#define RME_MASK(BEGIN,END)                         ((RME_MASK_BEGIN(BEGIN))&(RME_MASK_END(END)))
/* Round the number down & up to a power of 2 */
#define RME_ROUND_DOWN(NUM,POW)                     ((NUM)&(RME_MASK_BEGIN(POW)))
#define RME_ROUND_UP(NUM,POW)                       RME_ROUND_DOWN((NUM)+RME_MASK_END(POW-1U),POW)
/* Check if address is aligned on word boundary */
#define RME_IS_ALIGNED(ADDR)                        (((ADDR)&RME_MASK_END(RME_WORD_ORDER-4U))==0U)
/* Bitmap */
#define RME_BITMAP_SET(BMP,POS)                     ((BMP)[(POS)>>RME_WORD_ORDER]|=RME_POW2((POS)&RME_MASK_WORD))
#define RME_BITMAP_CLR(BMP,POS)                     ((BMP)[(POS)>>RME_WORD_ORDER]&=~RME_POW2((POS)&RME_MASK_WORD))
#define RME_BITMAP_IS_SET(BMP,POS)                  (((BMP)[(POS)>>RME_WORD_ORDER]&RME_POW2((POS)&RME_MASK_WORD))!=0U)

/* Maximum logging length */
#define RME_DBGLOG_MAX                              (255U)

/* Debug logging macros */
#if(RME_DBGLOG_ENABLE!=0U)
#define RME_DBG_I(INT)                              RME_Int_Print((rme_cnt_t)(INT))
#define RME_DBG_H(HEX)                              RME_Hex_Print((rme_ptr_t)(HEX))
#define RME_DBG_S(STR)                              RME_Str_Print((const rme_s8_t*)(STR))
#else
#define RME_DBG_I(INT)
#define RME_DBG_H(HEX)
#define RME_DBG_S(STR)
#endif

/* Logging macro */
#ifndef RME_LOG
#define RME_LOG_OP(F,L,D,T)                         RME_Log(F,L,D,T)
#else
#define RME_LOG_OP(F,L,D,T)                         RME_LOG(F,L,D,T)
#endif

/* Failure macro */
#ifndef RME_ASSERT_FAIL
#define RME_ASSERT_FAIL_OP(F,L,D,T)
#else
#define RME_ASSERT_FAIL_OP(F,L,D,T)                 RME_ASSERT_FAIL(F,L,D,T)
#endif

/* Assert macro - used only in internal development */
#if(RME_ASSERT_ENABLE!=0U)
#define RME_ASSERT(X) \
do \
{ \
    if(!(X)) \
    { \
        RME_LOG_OP(__FILE__,__LINE__,__DATE__,__TIME__); \
        RME_ASSERT_FAIL_OP(__FILE__,__LINE__,__DATE__,__TIME__); \
        while(1); \
    } \
} \
while(0)
#else
#define RME_ASSERT(X) \
do \
{ \
    RME_USE(X); \
} \
while(0)
#endif

/* Coverage marker enabling */
#ifdef RME_COV_LINE_NUM
#define RME_COV_WORD_NUM                            (RME_ROUND_UP(RME_COV_LINE_NUM,RME_WORD_ORDER)>>RME_WORD_ORDER)
#define RME_COV_MARKER()                            RME_BITMAP_SET(RME_Cov,__LINE__)
#else
#define RME_COV_MARKER()
#endif

/* Bit field extraction macros for easy extraction of parameters
[MSB                                 PARAMS                                 LSB]
[                  D1                  ][                  D0                  ]
[        Q3        ][        Q2        ][        Q1        ][        Q0        ]
[   O7   ][   O6   ][   O5   ][   O4   ][   O3   ][   O2   ][   O1   ][   O0   ] */
/* Cut in half */
#define RME_PARAM_D1(X)                             ((X)>>RME_WORD_BIT_D1)
#define RME_PARAM_D0(X)                             ((X)&RME_MASK_WORD_D)
/* Cut into 4 parts */
#define RME_PARAM_Q3(X)                             ((X)>>RME_WORD_BIT_Q3)
#define RME_PARAM_Q2(X)                             (((X)>>RME_WORD_BIT_Q2)&RME_MASK_WORD_Q)
#define RME_PARAM_Q1(X)                             (((X)>>RME_WORD_BIT_Q1)&RME_MASK_WORD_Q)
#define RME_PARAM_Q0(X)                             ((X)&RME_MASK_WORD_Q)
/* Cut into 8 parts */
#define RME_PARAM_O7(X)                             ((X)>>RME_WORD_BIT_O7)
#define RME_PARAM_O6(X)                             (((X)>>RME_WORD_BIT_O6)&RME_MASK_WORD_O)
#define RME_PARAM_O5(X)                             (((X)>>RME_WORD_BIT_O5)&RME_MASK_WORD_O)
#define RME_PARAM_O4(X)                             (((X)>>RME_WORD_BIT_O4)&RME_MASK_WORD_O)
#define RME_PARAM_O3(X)                             (((X)>>RME_WORD_BIT_O3)&RME_MASK_WORD_O)
#define RME_PARAM_O2(X)                             (((X)>>RME_WORD_BIT_O2)&RME_MASK_WORD_O)
#define RME_PARAM_O1(X)                             (((X)>>RME_WORD_BIT_O1)&RME_MASK_WORD_O)
#define RME_PARAM_O0(X)                             ((X)&RME_MASK_WORD_O)

/* This is the special one used for delegation, and used for kernel memory
 * capability only because it is very complicated. Other capabilities will not use this */
#define RME_PARAM_KM(SVC,CID)                       (RME_FIELD(SVC,RME_WORD_BIT_D1)|(CID))
/* This is the special one used for page table top-level flags */
#define RME_PARAM_PT(X)                             ((X)&0x01U)
/* The page table creation extra parameter packed in the svc number */
#define RME_PARAM_PC(SVC)                           ((SVC)>>RME_WORD_BIT_Q1)

/* The return procedure of a possible context switch - If successful, the function itself
 * is responsible for setting the parameters; If failed, we set the parameters for it.
 * Possible categories of context switch includes synchronous invocation and thread switch. */
#define RME_SWITCH_RETURN(REG,RETVAL) \
do \
{ \
    if(RME_UNLIKELY((RETVAL)<0)) \
        __RME_Svc_Retval_Set((REG),(RETVAL)); \
    \
    return; \
} \
while(0)

/* The system call numbers are included in both user level and kernel level */
#include "rme.h"

/* Kernel Object Table *******************************************************/
/* Bitmap reference error */
#define RME_ERR_KOT_BMP                             (-1)

/* Number of slots, and size of each slot */
#define RME_KOT_SLOT_NUM                            (RME_KOM_VA_SIZE>>RME_KOM_SLOT_ORDER)
#define RME_KOT_SLOT_SIZE                           RME_POW2(RME_KOM_SLOT_ORDER)
#define RME_KOT_WORD_NUM                            (RME_ROUND_UP(RME_KOT_SLOT_NUM,RME_WORD_ORDER)>>RME_WORD_ORDER)

/* Round the kernel object size to the entry slot size */
#define RME_KOM_ROUND(X)                            RME_ROUND_UP(X,RME_KOM_SLOT_ORDER)

/* Capability Table **********************************************************/
/* Capability size macro - always 8 full machine words */
#define RME_CAP_SIZE                                (RME_WORD_BYTE*8U)
/* Capability table size calculation macro */
#define RME_CPT_SIZE(NUM)                           (sizeof(struct RME_Cap_Struct)*(NUM))
/* The operation inline macros on the capabilities */
/* Type_Stat: example for 32-bit and 64-bit systems
 * 32-bit system:
 * [31    Type   24] [23  Status  16] [15              Attribute                0]
 * 64-bit system:
 * [63    Type   48] [47  Status  32] [31              Attribute                0]
 * Refcnt is used to track delegation, and also in the case of process creation,
 * used to track if the capability table or the page table is referenced.
 * Frozen is a fine-grained lock to lock the entries involved so that no parallel
 *        create/destroy/alteration operations on them can be done. If one of the
 *        locks failed, we will give up all the locks, and retry.
 * Type is a field denoting what is it. */
#define RME_CAP_TYPE_STAT(TYPE,STAT,ATTR)           (RME_FIELD(TYPE,RME_WORD_BIT_Q3)| \
                                                     RME_FIELD(STAT,RME_WORD_BIT_Q2)|(ATTR))

/* Capability types */
#define RME_CAP_TYPE(X)                             ((X)>>RME_WORD_BIT_Q3)
/* Empty */
#define RME_CAP_TYPE_NOP                            (0U)
/* Kernel function */
#define RME_CAP_TYPE_KFN                            (1U)
/* Kernel memory */
#define RME_CAP_TYPE_KOM                            (2U)
/* Capability table */
#define RME_CAP_TYPE_CPT                            (3U)
/* Page table */
#define RME_CAP_TYPE_PGT                            (4U)
/* Process */
#define RME_CAP_TYPE_PRC                            (5U)
/* Thread - there are two types */
#define RME_CAP_TYPE_THD                            (6U)
/* Synchronous invocation */
#define RME_CAP_TYPE_INV                            (7U)
/* Asynchronous signal endpoint */
#define RME_CAP_TYPE_SIG                            (8U)

/* Capability status */
#define RME_CAP_STAT(X)                             (((X)>>RME_WORD_BIT_Q2)&RME_MASK_WORD_Q)
/* Valid capability */
#define RME_CAP_STAT_VALID                          (0U)
/* Capability under creation */
#define RME_CAP_STAT_CREATING                       (1U)
/* Frozen capability */
#define RME_CAP_STAT_FROZEN                         (2U)

/* Capability attributes - currently only ROOT and LEAF are present */
#define RME_CAP_ATTR(X)                             ((X)&RME_MASK_WORD_D)
/* Root capability */
#define RME_CAP_ATTR_ROOT                           (0U)
/* Leaf capability */
#define RME_CAP_ATTR_LEAF                           (1U)

/* Is this cap quiescent? Yes-1, No-0 */
#if(RME_QUIE_TIME!=0U)
#if(RME_WORD_ORDER==5U)
/* If this is a 32-bit system, need to consider overflows */
#define RME_CAP_QUIE(X)                             (_RME_Diff((RME_TIMESTAMP),(X))>RME_QUIE_TIME)
#else
#define RME_CAP_QUIE(X)                             (((RME_TIMESTAMP)-(X))>RME_QUIE_TIME)
#endif
#else
#define RME_CAP_QUIE(X)                             (1U)
#endif

/* Convert to root */
#define RME_CAP_IS_ROOT(X)                          (RME_CAP_ATTR((X)->Head.Type_Stat)==RME_CAP_ATTR_ROOT)
#define RME_CAP_CONV_ROOT(X,TYPE)                   (RME_CAP_IS_ROOT(X)? \
                                                     ((TYPE)(X)):((TYPE)((X)->Head.Root_Ref)))
/* Get the object */
#define RME_CAP_GETOBJ(X,TYPE)                      ((TYPE)((X)->Head.Object))
/* 1-layer capid addressing: 
 * 32-bit systems: Capid range 0x00 - 0x7F
 * [15             Reserved             8] [7  2L(0)] [6    Table(Master)   0]
 * 64-bit systems: Capid range 0x0000 - 0x7FFF
 * [31             Reserved            16] [15 2L(0)] [14   Table(Master)   0]
 * 2-layer capid addressing:
 * 32-bit systems: Capid range 0x00 - 0x7F
 * [15 Reserved][14 High Table(Master)  8] [7  2L(1)] [6   Low Table(Child) 0]
 * 64-bit systems: Capid range 0x0000 - 0x7FFF
 * [31 Reserved][30 High Table(Master) 16] [15 2L(1)] [14  Low Table(Child) 0] */
#define RME_CID_NULL                                ((rme_cid_t)RME_POW2(RME_WORD_BIT_D1-1U))
/* See if the capid is a 2-level representation */
#define RME_CID_2L                                  RME_POW2(RME_WORD_BIT_Q1-1U)
/* Make 2-level capability */
#define RME_CID(X,Y)                                ((rme_cid_t)(RME_FIELD(X,RME_WORD_BIT_Q1)|((rme_ptr_t)(Y))|RME_CID_2L))
/* High-level capability table capability position */
#define RME_CAP_H(X)                                (((rme_ptr_t)(X))>>RME_WORD_BIT_Q1)
/* Low-level capability table capability position */
#define RME_CAP_L(X)                                (((rme_ptr_t)(X))&RME_MASK_END(RME_WORD_BIT_Q1-2U))

/* When we are clearing capabilities */
#define RME_CAP_CLEAR(X) \
do \
{ \
    /* Do this at last lest that some overlapping operations may happen */ \
    (X)->Head.Type_Stat=0U; \
} \
while(0)

/* Replicate the capability. The Type_Ref field is filled in in the last step 
 * of capability creation; the Parent field is also populated in the creation
 * process appropriately. Thus, they are not filled in here.
 * DST - The pointer to the destination slot.
 * SRC - The pointer to the source slot.
 * FLAGS - The new operation flags of this capability. */
#define RME_CAP_COPY(DST,SRC,FLAG) \
do \
{ \
    /* The suboperation capability flags */ \
    (DST)->Head.Flag=(FLAG); \
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
    if(RME_UNLIKELY(((CAP)->Head.Flag&(FLAG))!=(FLAG))) \
        return RME_ERR_CPT_FLAG; \
} \
while(0)

/* Check if the kernel memory capability range is valid.
 * CAP - The kernel memory capability to check against.
 * FLAG - The flags to check against the kernel memory capability.
 * RADDR - The relative begin address of the kernel object in kernel memory.
 * VADDR - The true begin address of the kernel object in kernel memory, output.
 * SIZE - The size of the kernel memory trunk. */
#define RME_KOM_CHECK(CAP,FLAG,RADDR,VADDR,SIZE) \
do \
{ \
    /* See if the creation of such capability is allowed */ \
    if(RME_UNLIKELY(((CAP)->Head.Flag&(FLAG))!=(FLAG))) \
        return RME_ERR_CPT_FLAG; \
    /* Convert relative address to virtual address */ \
    (VADDR)=(RADDR)+(CAP)->Begin; \
    /* Check begin boundary and its possible wraparound */ \
    if(RME_UNLIKELY((VADDR)<(RADDR))) \
        return RME_ERR_CPT_FLAG; \
    if(RME_UNLIKELY(((CAP)->Begin>(VADDR)))) \
        return RME_ERR_CPT_FLAG; \
    /* Check end boundary and its possible wraparound */ \
    if(RME_UNLIKELY((((VADDR)+(SIZE))<(VADDR)))) \
        return RME_ERR_CPT_FLAG; \
    if(RME_UNLIKELY((CAP)->End<((VADDR)+(SIZE)-1U))) \
        return RME_ERR_CPT_FLAG; \
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
    RME_COMP_SWAP(&((CAP)->Head.Type_Stat),(TEMP), \
                  RME_CAP_TYPE_STAT(RME_CAP_TYPE(TEMP),RME_CAP_STAT_VALID,RME_CAP_ATTR(TEMP))); \
} \
while(0)

/* Checks to be done before deleting - the barrier is for preventing stale timestamp
 * before the FROZEN status is set under read reordering situations. Different from a
 * removal check, the type check is also performed against the slot.
 * CAP - The pointer to the capability slot to check for deletion.
 * TEMP - A temporary variable, for compare-and-swap. 
 * TYPE - What type should we anticipate when we check against the slot? */
#define RME_CAP_DEL_CHECK(CAP,TEMP,TYPE) \
do \
{ \
    /* Atomic read - Need a read acquire barrier here to avoid stale reads below */ \
    (TEMP)=RME_READ_ACQUIRE(&((CAP)->Head.Type_Stat)); \
    /* See if the slot is frozen */ \
    if(RME_UNLIKELY(RME_CAP_STAT(TEMP)!=RME_CAP_STAT_FROZEN)) \
        return RME_ERR_CPT_FROZEN; \
    /* See if the cap type is correct. Only deletion checks type, while removing does not */ \
    if(RME_UNLIKELY(RME_CAP_TYPE(TEMP)!=(TYPE))) \
        return RME_ERR_CPT_TYPE; \
    /* See if the slot is quiescent */ \
    if(RME_UNLIKELY(RME_CAP_QUIE((CAP)->Head.Timestamp)==0U)) \
        return RME_ERR_CPT_QUIE; \
    /* To use deletion, we must be an unreferenced root */ \
    if(RME_UNLIKELY(((CAP)->Head.Root_Ref)!=0U)) \
    { \
        /* Defrost the cap if it is a root (likely), and return */ \
        if(RME_LIKELY(RME_CAP_ATTR(TEMP)==RME_CAP_ATTR_ROOT)) \
            RME_CAP_DEFROST(CAP,TEMP); \
        return RME_ERR_CPT_REFCNT; \
    } \
    /* The only case where the Root_Ref is 0 is that this is a unreferenced root cap */ \
    RME_ASSERT(RME_CAP_ATTR(TEMP)==RME_CAP_ATTR_ROOT); \
} \
while(0)

/* Checks to be done before removal - the barrier is for preventing stale timestamp
 * before the FROZEN bit is set under read reordering situations. Different from a 
 * deletion check, the type of the slot will not be checked.
 * CAP - The pointer to the capability slot to check for removal.
 * TEMP - A temporary variable, for compare-and-swap. */
#define RME_CAP_REM_CHECK(CAP, TEMP) \
do \
{ \
    /* Atomic read - Need a read acquire barrier here to avoid stale reads below */ \
    (TEMP)=RME_READ_ACQUIRE(&((CAP)->Head.Type_Stat)); \
    /* See if the slot is frozen */ \
    if(RME_UNLIKELY(RME_CAP_STAT(TEMP)!=RME_CAP_STAT_FROZEN)) \
        return RME_ERR_CPT_FROZEN; \
    /* See if the slot is quiescent */ \
    if(RME_UNLIKELY(RME_CAP_QUIE((CAP)->Head.Timestamp)==0U)) \
        return RME_ERR_CPT_QUIE; \
    /* To use removal, we must be a leaf */ \
    if(RME_UNLIKELY(RME_CAP_ATTR(TEMP)==RME_CAP_ATTR_ROOT)) \
        return RME_ERR_CPT_ROOT; \
} \
while(0)

/* Actually delete the cap.
 * CAP - The pointer to the capability slot to delete or remove.
 * TEMP - A temporary variable, for compare-and-swap's old value. */
#define RME_CAP_DELETE(CAP,TEMP) \
do \
{ \
    /* If this fails, then it means that somebody have deleted/removed it first */ \
    if(RME_UNLIKELY(RME_COMP_SWAP(&((CAP)->Head.Type_Stat),(TEMP),0U)==RME_CASFAIL)) \
        return RME_ERR_CPT_NULL; \
} \
while(0)

/* Check if we can take the slot, if we can, just take it. This also updates the timestamp,
 * so that we can enforce creation-freezing quiescence. We must update the counter after we
 * freeze the slot to ensure that we obtain exclusive access to it, and we must ensure that
 * other cores also see it that way.
 * CAP - The pointer to the capability slot to occupy.
 * TEMP - A temporary variable, for compare-and-swap. */
#define RME_CPT_OCCUPY(CAP) \
do \
{ \
    /* Check if anything is there. If there is nothing there, the Type_Ref must be 0 */ \
    if(RME_UNLIKELY(RME_COMP_SWAP(&((CAP)->Head.Type_Stat),0U, \
                                  RME_CAP_TYPE_STAT(RME_CAP_TYPE_NOP,RME_CAP_STAT_CREATING,RME_CAP_ATTR_ROOT))==RME_CASFAIL)) \
        return RME_ERR_CPT_EXIST; \
    /* We have taken the slot. Now log the quiescence counter in. No barrier needed as our atomics are serializing */ \
    (CAP)->Head.Timestamp=RME_TIMESTAMP; \
} \
while(0)

/* Get the capability slot from the master table according to a 1-level encoding.
 * This will not check the validity of the slot; nor will it try to resolve 2-level
 * encodings.
 * CPT - The master capability table.
 * CAP_NUM - The capability number. Only allows 1-level encoding.
 * TYPE -  When assigning the pointer to the PARAM, what struct pointer type should I cast it into?
 * PARAM - The parameter to receive the pointer to the found capability slot. */
#define RME_CPT_GETSLOT(CPT,CAP_NUM,TYPE,PARAM) \
do \
{ \
    /* Check if the captbl is over range */ \
    if(RME_UNLIKELY(((rme_ptr_t)(CAP_NUM))>=((CPT)->Entry_Num))) \
        return RME_ERR_CPT_RANGE; \
    /* Get the slot position */ \
    (PARAM)=&(RME_CAP_GETOBJ((CPT),TYPE)[(CAP_NUM)]); \
} \
while(0)

/* Get the capability from the master table according to capability number encoding.
 * The read acquire is for handling the case where the other CPU is creating
 * the capability. Suppose we are now operating on an empty slot, and the other CPU
 * have not started the creation yet. If we allow the other operation that follow the
 * TYPE check to reorder with it, then we are checking something that is not even
 * created at all (and by the time type check is taking place, the creation finishes
 * so it passes). This also will cause a race condition.
 * CPT - The current master capability table.
 * CAP_NUM - The capability number. Allows 1- and 2-level encodings.
 * CAP_TYPE - The type of the capability, i.e. CAP_CPT or CAP_PGT or CAP_THD, etc?
 * TYPE - When assigning the pointer to the PARAM, what struct pointer type should I cast it into?
 * PARAM - The parameter to receive the pointer to the found capability slot.
 * TEMP - A temporary variable, used to atomically keep a snapshot of Type_Ref when checking. */
#define RME_CPT_GETCAP(CPT,CAP_NUM,CAP_TYPE,TYPE,PARAM,TEMP) \
do \
{ \
    /* See if this is a 2-level cap */ \
    if((((rme_ptr_t)(CAP_NUM))&RME_CID_2L)==0U) \
    { \
        /* Check if the captbl is over range */ \
        if(RME_UNLIKELY(((rme_ptr_t)(CAP_NUM))>=((CPT)->Entry_Num))) \
            return RME_ERR_CPT_RANGE; \
        /* Get the cap slot and check the type */ \
        (PARAM)=(TYPE)(&RME_CAP_GETOBJ(CPT,struct RME_Cap_Struct*)[(CAP_NUM)]); \
        /* Atomic read - Need a read acquire barrier here to avoid stale reads below */ \
        (TEMP)=RME_READ_ACQUIRE(&((PARAM)->Head.Type_Stat)); \
        /* See if the capability is frozen */ \
        if(RME_UNLIKELY(RME_CAP_STAT(TEMP)==RME_CAP_STAT_FROZEN)) \
            return RME_ERR_CPT_FROZEN; \
        /* See if the type is correct */ \
        if(RME_UNLIKELY(RME_CAP_TYPE(TEMP)!=(CAP_TYPE))) \
            return RME_ERR_CPT_TYPE; \
    } \
    /* Yes, this is a 2-level cap */ \
    else \
    { \
        /* Check if the cap to potential captbl is over range */ \
        if(RME_UNLIKELY(RME_CAP_H(CAP_NUM)>=((CPT)->Entry_Num))) \
            return RME_ERR_CPT_RANGE; \
        /* Get the cap slot */ \
        (PARAM)=(TYPE)(&RME_CAP_GETOBJ(CPT,struct RME_Cap_Cpt*)[RME_CAP_H(CAP_NUM)]); \
        /* Atomic read - Need a read acquire barrier here to avoid stale reads below */ \
        (TEMP)=RME_READ_ACQUIRE(&((PARAM)->Head.Type_Stat)); \
        /* See if the capability table is frozen for deletion or removal */ \
        if(RME_UNLIKELY(RME_CAP_STAT(TEMP)==RME_CAP_STAT_FROZEN)) \
            return RME_ERR_CPT_FROZEN; \
        /* See if this is a captbl */ \
        if(RME_UNLIKELY(RME_CAP_TYPE(TEMP)!=RME_CAP_TYPE_CPT)) \
            return RME_ERR_CPT_TYPE; \
        /* Check if the 2nd-layer captbl is over range */ \
        if(RME_UNLIKELY(RME_CAP_L(CAP_NUM)>=(((volatile struct RME_Cap_Cpt*)(PARAM))->Entry_Num))) \
            return RME_ERR_CPT_RANGE; \
        /* Get the cap slot and check the type */ \
        (PARAM)=(TYPE)(&RME_CAP_GETOBJ(PARAM,struct RME_Cap_Struct*)[RME_CAP_L(CAP_NUM)]); \
        /* Atomic read - Need a read acquire barrier here to avoid stale reads below */ \
        (TEMP)=RME_READ_ACQUIRE(&((PARAM)->Head.Type_Stat)); \
        /* See if the capability is frozen */ \
        if(RME_UNLIKELY(RME_CAP_STAT(TEMP)==RME_CAP_STAT_FROZEN)) \
            return RME_ERR_CPT_FROZEN; \
        /* See if the type is correct */ \
        if(RME_UNLIKELY(RME_CAP_TYPE(TEMP)!=(CAP_TYPE))) \
            return RME_ERR_CPT_TYPE; \
    } \
} \
while(0)

/* Page Table ****************************************************************/
/* Default driver layer error - return anything smaller than 0 is ok */
#define RME_ERR_HAL_FAIL                            (-1)

/* Page table flag arrangement - limits are inclusive
* 32-bit systems: Maximum page table size 2^12 = 4096
* [31    High Limit    20] [19    Low Limit    8] [7    Flag    0]
* 64-bit systems: Maximum page table size 2^28 = 268435456
* [63    High Limit    36] [35    Low Limit    8] [7    Flag    0] */
/* Maximum number of entries in a page table */
#define RME_PGT_MAX_ENTRY                           RME_POW2(RME_WORD_BIT_D1-4U)
/* Range high limit */ 
#define RME_PGT_FLAG_HIGH(X)                        ((X)>>(RME_WORD_BIT_D1+4U))
/* Range low limit */
#define RME_PGT_FLAG_LOW(X)                         (((X)>>8)&RME_MASK_END(RME_WORD_BIT_D1-5U))
/* Permission flags */
#define RME_PGT_FLAG_FLAG(X)                        ((X)&RME_MASK_END(7U))
/* The initial flag of boot-time page table - allows all range delegation access only */
#define RME_PGT_FLAG_FULL_RANGE                     RME_MASK_BEGIN(RME_WORD_BIT_D1+4U)

/* Page table base address/top-level attributes */
#define RME_PGT_BASE(X)                             ((X)&RME_MASK_BEGIN(1U))
#define RME_PGT_TOP                                 (1U)
#define RME_PGT_NOM                                 (0U)

/* Size order and number order */
#define RME_PGT_SZORD(X)                            ((X)>>RME_WORD_BIT_D1)
#define RME_PGT_NMORD(X)                            ((X)&RME_MASK_WORD_D)
#define RME_PGT_ORDER(SIZE,NUM)                     (RME_FIELD(SIZE,RME_WORD_BIT_D1)|(NUM))
    
/* Kernel Memory *************************************************************/
/* Kernel memory function capability flag arrangement - extended flags used, 
 * granularity always 64 bytes min, because the reserved bits (for svc number)
 * is always 6 bits, and the flags field in the Ext_Flag is always 6 bits too.
 * 32-bit systems:
 * [31          High Limit[31:16]         16] [15       Low Limit[31:16]       0]  Flags
 * [31 High Limit[15: 6] 22] [21 Reserved 16] [15 Low Limit[15: 6] 6] [5 Flags 0]  Ext_Flag
 * 64-bit systems:
 * [63          High Limit[64:32]         32] [31       Low Limit[64:32]       0]  Flags
 * [63 High Limit[31: 6] 38] [37 Reserved 32] [31 Low Limit[31: 6] 6] [5 Flags 0]  Ext_Flag */
#define RME_KOM_FLAG_HIGH_F(FLAG)                   ((FLAG)&RME_MASK_BEGIN(RME_WORD_BIT_D1))
#define RME_KOM_FLAG_HIGH_E(EFLAG)                  (((EFLAG)>>RME_WORD_BIT_D1)&RME_MASK_BEGIN(6U))
#define RME_KOM_FLAG_HIGH(FLAG, EFLAG)              (RME_KOM_FLAG_HIGH_F(FLAG)|RME_KOM_FLAG_HIGH_E(EFLAG))
#define RME_KOM_FLAG_LOW_F(FLAG)                    RME_FIELD(FLAG,RME_WORD_BIT_D1)
#define RME_KOM_FLAG_LOW_E(EFLAG)                   ((EFLAG)&RME_MASK(RME_WORD_BIT_D1-1U,6U))
#define RME_KOM_FLAG_LOW(FLAG, EFLAG)               (RME_KOM_FLAG_LOW_F(FLAG)|RME_KOM_FLAG_LOW_E(EFLAG))
#define RME_KOM_FLAG_KOM(EFLAG)                     ((EFLAG)&RME_MASK(5U,0U))

/* Process and Thread ********************************************************/
/* Thread state flag arrangement - just plain state */
/* Thread is currently ready for scheduling */
#define RME_THD_READY                               (0U)
/* Thread is currently blocked on a asynchronous send endpoint */
#define RME_THD_BLOCKED                             (1U)
/* Thread just ran out of time */
#define RME_THD_TIMEOUT                             (2U)
/* Thread is stopped due to an unhandled exception */
#define RME_THD_EXCPEND                             (3U)

/* Priority level bitmap */
#define RME_PRIO_WORD_NUM                           (RME_ROUND_UP(RME_PREEMPT_PRIO_NUM,RME_WORD_ORDER)>>RME_WORD_ORDER)

/* Thread binding state */
#define RME_THD_FREE                                ((struct RME_CPU_Local*)RME_MASK_FULL)
/* Thread sched rcv timeout state */
#define RME_THD_TIMEOUT_FLAG                        RME_POW2(RME_WORD_BIT-3U)
/* Thread sched rcv faulty state */
#define RME_THD_EXCPEND_FLAG                        RME_POW2(RME_WORD_BIT-2U)
/* Init thread infinite time marker */
#define RME_THD_INIT_TIME                           (RME_MASK_FULL>>1)
/* Other thread infinite time marker */
#define RME_THD_INF_TIME                            (RME_THD_INIT_TIME-1U)

/* Thread attribute flag arrangement - svc number used to carry Is_Hyp and Attr
 * 32-bit systems:
 * [16               Attr              7] [6 Is_Hyp] [5        Reserved        0]  Svc
 * 64-bit systems:
 * [32               Attr              7] [6 Is_Hyp] [5        Reserved        0]  Svc
 * Hyp_Attr:
 * [63/31 Is_Hyp] [62/30                       Attr                             ]
 */
#define RME_THD_HYP_FLAG                            RME_POW2(RME_WORD_BIT-1U)
#define RME_THD_IS_HYP(X)                           (((X)&RME_THD_HYP_FLAG)!=0U)
#define RME_THD_ATTR(X)                             ((X)&(RME_MASK_FULL>>1))

#define RME_HYP_SIZE                                sizeof(struct RME_Thd_Struct)
#if(RME_COP_NUM!=0U)
#define RME_REG_SIZE(X)                             (sizeof(struct RME_Thd_Reg)-RME_WORD_BYTE+__RME_Thd_Cop_Size(X))
#else
#define RME_REG_SIZE(X)                             (sizeof(struct RME_Thd_Reg)-RME_WORD_BYTE)
#endif
#define RME_THD_SIZE(X)                             (RME_HYP_SIZE+RME_REG_SIZE(X))
    
/* Time checking macro */
#define RME_TIME_CHECK(DST, AMOUNT) \
do \
{ \
    /* Check if exceeded maximum time or overflowed */ \
    if(RME_UNLIKELY((((DST)+(AMOUNT))>=RME_THD_INF_TIME)||(((DST)+(AMOUNT))<(DST)))) \
        return RME_ERR_PTH_OVERFLOW; \
} \
while(0)

/* Signal and Invocation *****************************************************/
/* The maximum number of signals on an endpoint */
#define RME_MAX_SIG_NUM                             (RME_MASK_FULL>>1)

/* The kernel object sizes */
#define RME_INV_SIZE                                sizeof(struct RME_Inv_Struct)

/* Get the top of invocation stack - no volatile needed here because this is single-threaded */
#define RME_INVSTK_TOP_ADDR(THD)                    ((((THD)->Ctx.Invstk.Next)==&((THD)->Ctx.Invstk))? \
                                                     (RME_NULL): \
                                                     ((THD)->Ctx.Invstk.Next))
#define RME_INVSTK_TOP(THD)                         ((struct RME_Inv_Struct*)RME_INVSTK_TOP_ADDR(THD))

/* Kernel Function ***********************************************************/
/* Driver layer error reporting macro */
#define RME_ERR_KFN_FAIL                            (-1)
/* Kernel function capability flag arrangement
* 32-bit systems: Maximum kernel function number 2^16
* [31        High Limit        16] [15        Low Limit        0]
* 64-bit systems: Maximum kernel function number 2^32
* [63        High Limit        32] [31        Low Limit        0] */
#define RME_KFN_FLAG_HIGH(X)                        ((X)>>RME_WORD_BIT_D1)
#define RME_KFN_FLAG_LOW(X)                         ((X)&RME_MASK_WORD_D)
#define RME_KFN_FLAG_FULL_RANGE                     RME_MASK_BEGIN(RME_WORD_BYTE*4U)

/* __RME_KERNEL_DEF__ */
#endif
/* __HDR_DEF__ */
#endif
/* End Define ****************************************************************/

/* Struct ********************************************************************/
#ifdef __HDR_STRUCT__
#ifndef __RME_KERNEL_STRUCT__
#define __RME_KERNEL_STRUCT__

/* Use defines in these headers */
#define __HDR_DEF__
#undef __HDR_DEF__

/*****************************************************************************/
/* Generic *******************************************************************/
/* Capability header structure */
struct RME_Cap_Head
{
    /* The type, status */
    rme_ptr_t Type_Stat;
    /* The root capability (non-root caps), or the reference count (root caps) */
    rme_ptr_t Root_Ref;
    /* The suboperation capability flags */
    rme_ptr_t Flag;
    /* The object address */
    rme_ptr_t Object;
    /* The freeze timestamp */
    rme_ptr_t Timestamp;
};

/* Generic capability structure */
struct RME_Cap_Struct
{
    struct RME_Cap_Head Head;
    rme_ptr_t Info[3];
};

/* Capability Table **********************************************************/
/* Capability table capability structure */
struct RME_Cap_Cpt
{
    struct RME_Cap_Head Head;
    /* The number of entries in this captbl */
    rme_ptr_t Entry_Num;
    
    rme_ptr_t Info[2];
};

/* Page Table ****************************************************************/
/* Page table capability structure */
#if(RME_PGT_RAW_ENABLE==0U)
struct RME_Cap_Pgt
{
    struct RME_Cap_Head Head;
    /* The entry size/number order */
    rme_ptr_t Order;
    /* The base virtual address of this page table, only used when path compression
     * is enabled. Also, when this layer is a top-level, the last bit will be set. */
    rme_ptr_t Base;
    /* Address space ID, if applicable - need to convert to root cap to r/w. This
     * is only operated on by kernel function extensions, and is otherwise unused
     * when the corresponding kernel function is not implemented. Of course, this
     * could also act as a pointer to other structures that extends address space. */
    rme_ptr_t ASID;
};
#endif

/* Kernel Memory *************************************************************/
/* Kernel memory capability structure */
struct RME_Cap_Kom
{
    struct RME_Cap_Head Head;
    /* The begin address of the allowed kernel memory */
    rme_ptr_t Begin;
    /* The end address of the allowed kernel memory - internally inclusive */
    rme_ptr_t End;
    rme_ptr_t Info[1];
};

/* Process and Thread ********************************************************/
/* List head structure */
struct RME_List
{
    struct RME_List* Next;
    struct RME_List* Prev;
};

/* Per-CPU run queue structure */
struct RME_Run_Struct
{
    /* The bitmap marking to show if there are active threads at a run level */
    rme_ptr_t Bitmap[RME_PRIO_WORD_NUM];
    /* The actual RME running list */
    struct RME_List List[RME_PREEMPT_PRIO_NUM];
};

/* Process capability structure - does not have an object */
struct RME_Cap_Prc
{
    struct RME_Cap_Head Head;
    /* The capability table struct - need to convert to root cap to r/w */
    struct RME_Cap_Cpt* Cpt;
#if(RME_PGT_RAW_ENABLE==0U)
    /* The page table struct - need to convert to root cap to r/w */
    struct RME_Cap_Pgt* Pgt;
#else
    /* A pointer to page table in trusted memory */
    rme_ptr_t Pgt;
#endif
    rme_ptr_t Info[1];
};

/* Thread scheduling state structure */
struct RME_Thd_Sched
{
    /* The runnable thread header - This will be inserted into the per-core
     * run queue */
    struct RME_List Run;
    /* The list head for notifications - This will be inserted into scheduler
     * threads' event list */
    struct RME_List Notif; 
    /* TID of the thread - internally unsigned */
    rme_ptr_t TID;
    /* What is the CPU-local data structure that this thread is on? If this is
     * 0xFF....FF, then this is not bound to any core. "struct RME_CPU_Local" is
     * not yet defined here but compilation will still pass - it is a pointer */
    struct RME_CPU_Local* Local;
    /* How much time slices is left for this thread? */
    rme_ptr_t Slice;
    /* What is the current state of the thread? */
    rme_ptr_t State;
    /* What's the priority of the thread? */
    rme_ptr_t Prio;
    /* What's the maximum priority allowed for this thread? */
    rme_ptr_t Prio_Max;
    /* What signal endpoint does this thread block on? */
    struct RME_Cap_Sig* Signal;
    /* Which process is it created in? */
    struct RME_Cap_Prc* Prc;
    /* Am I referenced by someone as a scheduler? */
    rme_ptr_t Sched_Ref;
    /* What is its scheduler thread? */
    struct RME_Thd_Struct* Sched_Thd;
    /* What is the signal endpoint to send to if we have scheduler notifications? (optional) */
    struct RME_Cap_Sig* Sched_Sig;
    /* The event list for the thread */
    struct RME_List Event;
};

/* Thread register set structure - not instantiated at all */
struct RME_Thd_Reg
{
    /* Register set - architecture specific */
    struct RME_Reg_Struct Reg;
    /* Error information - architecture specific */
    struct RME_Exc_Struct Exc;
    /* Coprocessor/FPU context - architecture specific */
    rme_ptr_t Cop[1];
};

/* Thread context structure */
struct RME_Thd_Ctx
{
    /* Hypervisor flag and context attribute */
    rme_ptr_t Hyp_Attr;
    /* Pointer to current register set */
    struct RME_Thd_Reg* Reg;
    /* Synchronous invocation stack */
    struct RME_List Invstk;
    /* Synchronous invocation stack depth */
    rme_ptr_t Invstk_Depth;
};

/* Thread object structure */
struct RME_Thd_Struct
{
    /* The thread scheduling structure */
    struct RME_Thd_Sched Sched;
    /* The thread context */
    struct RME_Thd_Ctx Ctx;
};

/* Thread capability structure */
struct RME_Cap_Thd
{
    struct RME_Cap_Head Head;
    rme_ptr_t Info[3];
};

/* Signal and Invocation *****************************************************/
/* Signal endpoint capability structure - does not have an object */
struct RME_Cap_Sig
{
    struct RME_Cap_Head Head;
    /* The number of signals sent to here - need to convert to root cap to r/w */
    rme_ptr_t Sig_Num;
    /* What thread blocked on this one - need to convert to root cap to r/w */
    struct RME_Thd_Struct* Thd;
    rme_ptr_t Info[1];
};

/* Invocation object structure */
struct RME_Inv_Struct
{
    /* This will be inserted into a thread structure */
    struct RME_List Head;
    /* The process pointer */
    struct RME_Cap_Prc* Prc;
    /* Thread currently occupying the invocation port */
    struct RME_Thd_Struct* Thd_Act;
    /* The entry and stack of the invocation */
    rme_ptr_t Entry;
    rme_ptr_t Stack;
    /* Do we return immediately on fault? */
    rme_ptr_t Is_Exc_Ret;
    /* The registers to be saved in the invocation */
    struct RME_Iret_Struct Ret;
};

/* Invocation capability structure */
struct RME_Cap_Inv
{
    struct RME_Cap_Head Head;
    rme_ptr_t Info[3];
};

/* CPU-local data structure */
struct RME_CPU_Local
{
    /* The CPUID of the CPU */
    rme_ptr_t CPUID;
    /* The current thread on the CPU */
    struct RME_Thd_Struct* Thd_Cur;
    /* The tick timer signal endpoint */
    struct RME_Cap_Sig* Sig_Tim;
    /* The vector signal endpoint */
    struct RME_Cap_Sig* Sig_Vct;
    /* The runqueue and bitmap */
    struct RME_Run_Struct Run;
};

/* Kernel Function ***********************************************************/
/* Kernel function capability structure */
struct RME_Cap_Kfn
{
    struct RME_Cap_Head Head;
    rme_ptr_t Info[3];
};

/*****************************************************************************/
/* __RME_KERNEL_STRUCT__ */
#endif
/* __HDR_STRUCT__ */
#endif
/* End Struct ****************************************************************/

/* Private Variable **********************************************************/
#if(!(defined __HDR_DEF__||defined __HDR_STRUCT__))
#ifndef __RME_KERNEL_MEMBER__
#define __RME_KERNEL_MEMBER__

/* In this way we can use the data structures and definitions in the headers */
#define __HDR_DEF__

#undef __HDR_DEF__

#define __HDR_STRUCT__

#undef __HDR_STRUCT__

/* If the header is not used in the public mode */
#ifndef __HDR_PUBLIC__
/*****************************************************************************/
#ifdef RME_COV_LINE_NUM
/* For kernel coverage use only */
static volatile rme_ptr_t RME_Cov[RME_COV_WORD_NUM];
#endif
/*****************************************************************************/
/* End Private Variable ******************************************************/

/* Private Function **********************************************************/
/* Generic *******************************************************************/
static rme_ret_t _RME_Lowlvl_Check(void);

/* Capability Table **********************************************************/
/* Capability system calls */
static rme_ret_t _RME_Cpt_Crt(struct RME_Cap_Cpt* Cpt,
                              rme_cid_t Cap_Cpt_Crt, 
                              rme_cid_t Cap_Kom,
                              rme_cid_t Cap_Crt,
                              rme_ptr_t Raddr,
                              rme_ptr_t Entry_Num);
static rme_ret_t _RME_Cpt_Del(struct RME_Cap_Cpt* Cpt,
                              rme_cid_t Cap_Cpt_Del,
                              rme_cid_t Cap_Del);
static rme_ret_t _RME_Cpt_Frz(struct RME_Cap_Cpt* Cpt,
                              rme_cid_t Cap_Cpt_Frz,
                              rme_cid_t Cap_Frz);
static rme_ret_t _RME_Cpt_Add(struct RME_Cap_Cpt* Cpt,
                              rme_cid_t Cap_Cpt_Dst,
                              rme_cid_t Cap_Dst, 
                              rme_cid_t Cap_Cpt_Src,
                              rme_cid_t Cap_Src,
                              rme_ptr_t Flag,
                              rme_ptr_t Ext_Flag);
static rme_ret_t _RME_Cpt_Rem(struct RME_Cap_Cpt* Cpt,
                              rme_cid_t Cap_Cpt_Rem,
                              rme_cid_t Cap_Rem);

/* Page Table ****************************************************************/
/* Page table system calls */
#if(RME_PGT_RAW_ENABLE==0U)
static rme_ret_t _RME_Pgt_Crt(struct RME_Cap_Cpt* Cpt,
                              rme_cid_t Cap_Cpt,
                              rme_cid_t Cap_Kom,
                              rme_cid_t Cap_Pgt,
                              rme_ptr_t Raddr,
                              rme_ptr_t Base_Addr,
                              rme_ptr_t Is_Top,
                              rme_ptr_t Size_Order,
                              rme_ptr_t Num_Order);
static rme_ret_t _RME_Pgt_Del(struct RME_Cap_Cpt* Cpt,
                              rme_cid_t Cap_Cpt,
                              rme_cid_t Cap_Pgt);
static rme_ret_t _RME_Pgt_Add(struct RME_Cap_Cpt* Cpt, 
                              rme_cid_t Cap_Pgt_Dst,
                              rme_ptr_t Pos_Dst,
                              rme_ptr_t Flag_Dst,
                              rme_cid_t Cap_Pgt_Src,
                              rme_ptr_t Pos_Src,
                              rme_ptr_t Index);
static rme_ret_t _RME_Pgt_Rem(struct RME_Cap_Cpt* Cpt,
                              rme_cid_t Cap_Pgt,
                              rme_ptr_t Pos);
static rme_ret_t _RME_Pgt_Con(struct RME_Cap_Cpt* Cpt,
                              rme_cid_t Cap_Pgt_Parent,
                              rme_ptr_t Pos,
                              rme_cid_t Cap_Pgt_Child,
                              rme_ptr_t Flag_Child);
static rme_ret_t _RME_Pgt_Des(struct RME_Cap_Cpt* Cpt, 
                              rme_cid_t Cap_Pgt_Parent,
                              rme_ptr_t Pos,
                              rme_cid_t Cap_Pgt_Child);
#endif
/* Process and Thread ********************************************************/
/* In-kernel ready-queue primitives */
static void _RME_Run_Ins(struct RME_Thd_Struct* Thd);
static void _RME_Run_Del(struct RME_Thd_Struct* Thd);
static struct RME_Thd_Struct* _RME_Run_High(struct RME_CPU_Local* Local);
static void _RME_Run_Notif(struct RME_Thd_Struct* Thd);
static rme_ret_t _RME_Run_Swt(struct RME_Reg_Struct* Reg,
                              struct RME_Thd_Struct* Thd_Cur, 
                              struct RME_Thd_Struct* Thd_New);
/* Process system calls */
#if(RME_PGT_RAW_ENABLE==0U)
static rme_ret_t _RME_Prc_Crt(struct RME_Cap_Cpt* Cpt,
                              rme_cid_t Cap_Cpt_Crt,
                              rme_cid_t Cap_Prc,
                              rme_cid_t Cap_Cpt,
                              rme_cid_t Cap_Pgt);
#else
static rme_ret_t _RME_Prc_Crt(struct RME_Cap_Cpt* Cpt,
                              rme_cid_t Cap_Cpt_Crt,
                              rme_cid_t Cap_Prc,
                              rme_cid_t Cap_Cpt,
                              rme_ptr_t Raw_Pgt);
#endif
static rme_ret_t _RME_Prc_Del(struct RME_Cap_Cpt* Cpt,
                              rme_cid_t Cap_Cpt,
                              rme_cid_t Cap_Prc);
static rme_ret_t _RME_Prc_Cpt(struct RME_Cap_Cpt* Cpt,
                              rme_cid_t Cap_Prc,
                              rme_cid_t Cap_Cpt);
#if(RME_PGT_RAW_ENABLE==0U)
static rme_ret_t _RME_Prc_Pgt(struct RME_Cap_Cpt* Cpt,
                              rme_cid_t Cap_Prc,
                              rme_cid_t Cap_Pgt);
#else
static rme_ret_t _RME_Prc_Pgt(struct RME_Cap_Cpt* Cpt,
                              rme_cid_t Cap_Prc,
                              rme_ptr_t Raw_Pgt);
#endif
/* Thread system calls */
static rme_ret_t _RME_Thd_Crt(struct RME_Cap_Cpt* Cpt,
                              rme_cid_t Cap_Cpt,
                              rme_cid_t Cap_Kom,
                              rme_cid_t Cap_Thd,
                              rme_cid_t Cap_Prc,
                              rme_ptr_t Prio_Max,
                              rme_ptr_t Raddr,
                              rme_ptr_t Attr,
                              rme_ptr_t Is_Hyp);
static rme_ret_t _RME_Thd_Del(struct RME_Cap_Cpt* Cpt,
                              rme_cid_t Cap_Cpt,
                              rme_cid_t Cap_Thd);
static rme_ret_t _RME_Thd_Sched_Bind(struct RME_Cap_Cpt* Cpt,
                                     rme_cid_t Cap_Thd,
                                     rme_cid_t Cap_Thd_Sched,
                                     rme_cid_t Cap_Sig,
                                     rme_tid_t TID,
                                     rme_ptr_t Prio,
                                     rme_ptr_t Haddr);
static rme_ret_t _RME_Thd_Sched_Free(struct RME_Cap_Cpt* Cpt,
                                     struct RME_Reg_Struct* Reg,
                                     rme_cid_t Cap_Thd);
static rme_ret_t _RME_Thd_Exec_Set(struct RME_Cap_Cpt* Cpt,
                                   struct RME_Reg_Struct* Reg,
                                   rme_cid_t Cap_Thd,
                                   rme_ptr_t Entry,
                                   rme_ptr_t Stack,
                                   rme_ptr_t Param);
static rme_ret_t _RME_Thd_Sched_Prio(struct RME_Cap_Cpt* Cpt,
                                     struct RME_Reg_Struct* Reg,
                                     rme_ptr_t Number,
                                     rme_cid_t Cap_Thd0,
                                     rme_ptr_t Prio0,
                                     rme_cid_t Cap_Thd1,
                                     rme_ptr_t Prio1,
                                     rme_cid_t Cap_Thd2,
                                     rme_ptr_t Prio2);
static rme_ret_t _RME_Thd_Sched_Rcv(struct RME_Cap_Cpt* Cpt,
                                    rme_cid_t Cap_Thd);
static rme_ret_t _RME_Thd_Time_Xfer(struct RME_Cap_Cpt* Cpt,
                                    struct RME_Reg_Struct* Reg,
                                    rme_cid_t Cap_Thd_Dst,
                                    rme_cid_t Cap_Thd_Src,
                                    rme_ptr_t Time);
static rme_ret_t _RME_Thd_Swt(struct RME_Cap_Cpt* Cpt,
                              struct RME_Reg_Struct* Reg,
                              rme_cid_t Cap_Thd,
                              rme_ptr_t Is_Yield);
                              
/* Signal and Invocation *****************************************************/
/* Signal system calls */
static rme_ret_t _RME_Sig_Crt(struct RME_Cap_Cpt* Cpt,
                              rme_cid_t Cap_Cpt,
                              rme_cid_t Cap_Sig);
static rme_ret_t _RME_Sig_Del(struct RME_Cap_Cpt* Cpt,
                              rme_cid_t Cap_Cpt,
                              rme_cid_t Cap_Sig);
static rme_ret_t _RME_Sig_Snd(struct RME_Cap_Cpt* Cpt,
                              struct RME_Reg_Struct* Reg,
                              rme_cid_t Cap_Sig);
static rme_ret_t _RME_Sig_Rcv(struct RME_Cap_Cpt* Cpt,
                              struct RME_Reg_Struct* Reg,
                              rme_cid_t Cap_Sig,
                              rme_ptr_t Option);
/* Invocation system calls */
static rme_ret_t _RME_Inv_Crt(struct RME_Cap_Cpt* Cpt,
                              rme_cid_t Cap_Cpt,
                              rme_cid_t Cap_Kom,
                              rme_cid_t Cap_Inv,
                              rme_cid_t Cap_Prc,
                              rme_ptr_t Raddr);
static rme_ret_t _RME_Inv_Del(struct RME_Cap_Cpt* Cpt,
                              rme_cid_t Cap_Cpt,
                              rme_cid_t Cap_Inv);
static rme_ret_t _RME_Inv_Set(struct RME_Cap_Cpt* Cpt,
                              rme_cid_t Cap_Inv,
                              rme_ptr_t Entry,
                              rme_ptr_t Stack,
                              rme_ptr_t Is_Exc_Ret);
static rme_ret_t _RME_Inv_Act(struct RME_Cap_Cpt* Cpt, 
                              struct RME_Reg_Struct* Reg,
                              rme_cid_t Cap_Inv,
                              rme_ptr_t Param);
static rme_ret_t _RME_Inv_Ret(struct RME_Reg_Struct* Reg,
                              rme_ptr_t Retval,
                              rme_ptr_t Is_Exc);

/* Kernel Function ***********************************************************/
static rme_ret_t _RME_Kfn_Act(struct RME_Cap_Cpt* Cpt,
                              struct RME_Reg_Struct* Reg,
                              rme_cid_t Cap_Kern,
                              rme_ptr_t Func_ID,
                              rme_ptr_t Sub_ID,
                              rme_ptr_t Param1,
                              rme_ptr_t Param2);

/*****************************************************************************/
#define __RME_EXTERN__
/* End Private Function ******************************************************/

/* Public Variable ***********************************************************/
/* __HDR_PUBLIC__ */
#else
#define __RME_EXTERN__ RME_EXTERN 
/* __HDR_PUBLIC__ */
#endif

/*****************************************************************************/

/*****************************************************************************/

/* End Public Variable *******************************************************/

/* Public Function ***********************************************************/
/* Generic *******************************************************************/
/* Debugging printing */
#if(RME_DBGLOG_ENABLE!=0U)
__RME_EXTERN__ rme_cnt_t RME_Hex_Print(rme_ptr_t Uint);
__RME_EXTERN__ rme_cnt_t RME_Int_Print(rme_cnt_t Int);
__RME_EXTERN__ rme_cnt_t RME_Str_Print(const rme_s8_t* String);
#endif

/* Default logging */
#ifndef RME_LOG
__RME_EXTERN__ void RME_Log(const char* File,
                            long Line,
                            const char* Date,
                            const char* Time);                        
#endif
                            
/* Coverage test - internal use */
#ifdef RME_COV_LINE_NUM
__RME_EXTERN__ void RME_Cov_Print(void);
#endif

/* Misc helper */
__RME_EXTERN__ void _RME_Clear(void* Addr,
                               rme_ptr_t Size);
__RME_EXTERN__ rme_ret_t _RME_Memcmp(const void* Ptr1,
                                     const void* Ptr2,
                                     rme_ptr_t Num);
__RME_EXTERN__ void _RME_Memcpy(void* Dst,
                                void* Src,
                                rme_ptr_t Num);
__RME_EXTERN__ rme_ptr_t _RME_Diff(rme_ptr_t Num1,
                                   rme_ptr_t Num2);

/* Bit manipulation */
__RME_EXTERN__ rme_ptr_t _RME_MSB_Generic(rme_ptr_t Value);
__RME_EXTERN__ rme_ptr_t _RME_LSB_Generic(rme_ptr_t Value);
__RME_EXTERN__ rme_ptr_t _RME_RBT_Generic(rme_ptr_t Value);

/* Single-core atomic */
__RME_EXTERN__ rme_ptr_t _RME_Comp_Swap_Single(volatile rme_ptr_t* Ptr,
                                               rme_ptr_t Old,
                                               rme_ptr_t New);
__RME_EXTERN__ rme_ptr_t _RME_Fetch_Add_Single(volatile rme_ptr_t* Ptr,
                                               rme_cnt_t Addend);
__RME_EXTERN__ rme_ptr_t _RME_Fetch_And_Single(volatile rme_ptr_t* Ptr,
                                               rme_ptr_t Operand);

/* Linked list operation */
__RME_EXTERN__ void _RME_List_Crt(struct RME_List* Head);
__RME_EXTERN__ void _RME_List_Del(struct RME_List* Prev,
                                  struct RME_List* Next);
__RME_EXTERN__ void _RME_List_Ins(struct RME_List* New,
                                  struct RME_List* Prev,
                                  struct RME_List* Next);
                                  
/* Kernel entry */
__RME_EXTERN__ rme_ret_t RME_Kmain(void);
/* System call handler */
__RME_EXTERN__ void _RME_Svc_Handler(struct RME_Reg_Struct* Reg);
/* Timer interrupt handler */
__RME_EXTERN__ void _RME_Tim_Handler(struct RME_Reg_Struct* Reg, rme_ptr_t Slice);
__RME_EXTERN__ void _RME_Tim_Elapse(rme_ptr_t Slice);
__RME_EXTERN__ rme_ptr_t _RME_Tim_Future(void);

/* Capability Table **********************************************************/
/* Boot-time calls */
__RME_EXTERN__ rme_ret_t _RME_Cpt_Boot_Init(rme_cid_t Cap_Cpt,
                                            rme_ptr_t Vaddr,
                                            rme_ptr_t Entry_Num);
__RME_EXTERN__ rme_ret_t _RME_Cpt_Boot_Crt(struct RME_Cap_Cpt* Cpt,
                                           rme_cid_t Cap_Cpt_Crt,
                                           rme_cid_t Cap_Crt,
                                           rme_ptr_t Vaddr,
                                           rme_ptr_t Entry_Num);

/* Page Table ****************************************************************/
/* Boot-time calls */
#if(RME_PGT_RAW_ENABLE==0U)
__RME_EXTERN__ rme_ret_t _RME_Pgt_Boot_Crt(struct RME_Cap_Cpt* Cpt,
                                           rme_cid_t Cap_Cpt,
                                           rme_cid_t Cap_Pgt,
                                           rme_ptr_t Vaddr,
                                           rme_ptr_t Base_Addr,
                                           rme_ptr_t Is_Top,
                                           rme_ptr_t Size_Order,
                                           rme_ptr_t Num_Order);
__RME_EXTERN__ rme_ret_t _RME_Pgt_Boot_Con(struct RME_Cap_Cpt* Cpt,
                                           rme_cid_t Cap_Pgt_Parent,
                                           rme_ptr_t Pos,
                                           rme_cid_t Cap_Pgt_Child,
                                           rme_ptr_t Flag_Child);
__RME_EXTERN__ rme_ret_t _RME_Pgt_Boot_Add(struct RME_Cap_Cpt* Cpt,
                                           rme_cid_t Cap_Pgt, 
                                           rme_ptr_t Paddr,
                                           rme_ptr_t Pos,
                                           rme_ptr_t Flag);
#endif
/* Kernel Memory *************************************************************/
__RME_EXTERN__ rme_ret_t _RME_Kot_Init(rme_ptr_t Word);
/* Kernel memory operations (in case HAL needs to allocate kernel memory) */
__RME_EXTERN__ rme_ret_t _RME_Kot_Mark(rme_ptr_t Kaddr,
                                       rme_ptr_t Size);
__RME_EXTERN__ rme_ret_t _RME_Kot_Erase(rme_ptr_t Kaddr,
                                        rme_ptr_t Size);
/* Boot-time calls */
__RME_EXTERN__ rme_ret_t _RME_Kom_Boot_Crt(struct RME_Cap_Cpt* Cpt,
                                           rme_cid_t Cap_Cpt,
                                           rme_cid_t Cap_Kom,
                                           rme_ptr_t Begin,
                                           rme_ptr_t End,
                                           rme_ptr_t Flag);

/* Process and Thread ********************************************************/
/* Initialize per-CPU data structures */
__RME_EXTERN__ void _RME_CPU_Local_Init(struct RME_CPU_Local* Local,
                                        rme_ptr_t CPUID);
/* Thread page table consult */
#if(RME_PGT_RAW_ENABLE==0U)
__RME_EXTERN__ struct RME_Cap_Pgt* _RME_Thd_Pgt(struct RME_Thd_Struct* Thd);
#else
__RME_EXTERN__ rme_ptr_t _RME_Thd_Pgt(struct RME_Thd_Struct* Thd);
#endif
/* Thread fatal killer */
__RME_EXTERN__ void _RME_Thd_Fatal(struct RME_Reg_Struct* Reg);                              
/* Boot-time calls */
#if(RME_PGT_RAW_ENABLE==0U)
__RME_EXTERN__ rme_ret_t _RME_Prc_Boot_Crt(struct RME_Cap_Cpt* Cpt,
                                           rme_cid_t Cap_Cpt_Crt,
                                           rme_cid_t Cap_Prc,
                                           rme_cid_t Cap_Cpt,
                                           rme_cid_t Cap_Pgt);
#else
__RME_EXTERN__ rme_ret_t _RME_Prc_Boot_Crt(struct RME_Cap_Cpt* Cpt,
                                           rme_cid_t Cap_Cpt_Crt,
                                           rme_cid_t Cap_Prc,
                                           rme_cid_t Cap_Cpt,
                                           rme_ptr_t Raw_Pgt);
#endif
__RME_EXTERN__ rme_ret_t _RME_Thd_Boot_Crt(struct RME_Cap_Cpt* Cpt,
                                           rme_cid_t Cap_Cpt,
                                           rme_cid_t Cap_Thd,
                                           rme_cid_t Cap_Prc,
                                           rme_ptr_t Vaddr,
                                           rme_ptr_t Prio,
                                           struct RME_CPU_Local* Local);

/* Signal and Invocation *****************************************************/
/* Kernel send facilities */
__RME_EXTERN__ rme_ret_t _RME_Kern_Snd(struct RME_Cap_Sig* Sig);
__RME_EXTERN__ void _RME_Kern_High(struct RME_Reg_Struct* Reg,
                                   struct RME_CPU_Local* Local);
/* Boot-time calls */
__RME_EXTERN__ rme_ret_t _RME_Sig_Boot_Crt(struct RME_Cap_Cpt* Cpt,
                                           rme_cid_t Cap_Cpt,
                                           rme_cid_t Cap_Sig);

/* Kernel Function ***********************************************************/
__RME_EXTERN__ rme_ret_t _RME_Kfn_Boot_Crt(struct RME_Cap_Cpt* Cpt,
                                           rme_cid_t Cap_Cpt,
                                           rme_cid_t Cap_Kern);

/*****************************************************************************/
/* Undefine "__RME_EXTERN__" to avoid redefinition */
#undef __RME_EXTERN__
/* __RME_KERNEL_MEMBER__ */
#endif
/* !(defined __HDR_DEF__||defined __HDR_STRUCT__) */
#endif
/* End Public Function *******************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
