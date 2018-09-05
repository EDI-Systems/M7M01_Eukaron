/******************************************************************************
Filename    : rme_pgtbl.h
Author      : pry
Date        : 08/04/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The header of page table.
******************************************************************************/

/* Defines *******************************************************************/
#ifdef __HDR_DEFS__
#ifndef __RME_PGTBL_H_DEFS__
#define __RME_PGTBL_H_DEFS__
/*****************************************************************************/
/* Driver layer error reporting macro */
#define RME_ERR_PGT_OPFAIL              ((rme_ptr_t)(-1))

/* Page table flag arrangement
* 32-bit systems: Maximum page table size 2^12 = 4096
* [31    High Limit    20] [19    Low Limit    8][7    Flags    0]
* 64-bit systems: Maximum page table size 2^28 = 268435456
* [63    High Limit    36] [35    Low Limit    8][7    Flags    0] */
/* Maximum number of entries in a page table */
#define RME_PGTBL_MAX_ENTRY             RME_POW2(sizeof(rme_ptr_t)*4-4)
/* Range high limit */ 
#define RME_PGTBL_FLAG_HIGH(X)          ((X)>>(sizeof(rme_ptr_t)*4+4))
/* Range low limit */
#define RME_PGTBL_FLAG_LOW(X)           (((X)>>8)&RME_MASK_END(sizeof(rme_ptr_t)*4-5))
/* Permission flags */
#define RME_PGTBL_FLAG_FLAGS(X)         ((X)&RME_MASK_END(7))
/* The initial flag of boot-time page table - allows all range delegation access only */
#define RME_PGTBL_FLAG_FULL_RANGE       (((rme_ptr_t)(-1))&RME_MASK_START(sizeof(rme_ptr_t)*4+4))
    
/* The page table creation extra parameter packed in the svc number */
#define RME_PARAM_PC(SVC)               ((SVC)>>((sizeof(rme_ptr_t)<<1)))

/* Page table start address/top-level attributes */
#define RME_PGTBL_START(X)              ((X)&(~((rme_ptr_t)1)))
#define RME_PGTBL_TOP                   (1)
#define RME_PGTBL_NOM                   (0)

/* Size order and number order */
#define RME_PGTBL_SIZEORD(X)            ((X)>>(sizeof(rme_ptr_t)*4))
#define RME_PGTBL_NUMORD(X)             ((X)&RME_MASK_END(sizeof(rme_ptr_t)*4-1))
#define RME_PGTBL_ORDER(SIZE,NUM)       (((SIZE)<<(sizeof(rme_ptr_t)*4))|(NUM))
/*****************************************************************************/
/* __RME_PGTBL_H_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Structs *******************************************************************/
#ifdef __HDR_STRUCTS__
#ifndef __RME_PGTBL_H_STRUCTS__
#define __RME_PGTBL_H_STRUCTS__

/* Use defines in these headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__

struct RME_Cap_Pgtbl
{
    struct RME_Cap_Head Head;
    /* The entry size/number order */
    rme_ptr_t Size_Num_Order;
    /* The start address of this page table */
    rme_ptr_t Start_Addr;
    /* We will not place the page table parent/child counter and extra information
     * like ASID here, because we consider that as a inherent part of page table.
     * Because page tables are required to be aligned to some address, thus we 
     * usually an't simply append data to it. We leave these counters to the HAL. */
    rme_ptr_t Info[1];
};
/*****************************************************************************/

/*****************************************************************************/
/* __RME_PGTBL_H_STRUCTS__ */
#endif
/* __HDR_STRUCTS__ */
#endif
/* End Structs ***************************************************************/

/* Private Global Variables **************************************************/
#if(!(defined __HDR_DEFS__||defined __HDR_STRUCTS__))
#ifndef __RME_PGTBL_MEMBERS__
#define __RME_PGTBL_MEMBERS__

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
__EXTERN__ rme_ret_t _RME_Pgtbl_Boot_Crt(struct RME_Cap_Captbl* Captbl, rme_cid_t Cap_Captbl,
                                         rme_cid_t Cap_Pgtbl, rme_ptr_t Vaddr, rme_ptr_t Start_Addr,
                                         rme_ptr_t Top_Flag, rme_ptr_t Size_Order, rme_ptr_t Num_Order);
__EXTERN__ rme_ret_t _RME_Pgtbl_Boot_Con(struct RME_Cap_Captbl* Captbl,
                                         rme_cid_t Cap_Pgtbl_Parent, rme_ptr_t Pos,
                                         rme_cid_t Cap_Pgtbl_Child, rme_ptr_t Flags_Child);
__EXTERN__ rme_ret_t _RME_Pgtbl_Boot_Add(struct RME_Cap_Captbl* Captbl, rme_cid_t Cap_Pgtbl, 
                                         rme_ptr_t Paddr, rme_ptr_t Pos, rme_ptr_t Flags);
__EXTERN__ rme_ret_t _RME_Pgtbl_Crt(struct RME_Cap_Captbl* Captbl, rme_cid_t Cap_Captbl,
                                    rme_cid_t Cap_Kmem, rme_cid_t Cap_Pgtbl, rme_ptr_t Raddr,
                                    rme_ptr_t Start_Addr, rme_ptr_t Top_Flag, rme_ptr_t Size_Order, rme_ptr_t Num_Order);
__EXTERN__ rme_ret_t _RME_Pgtbl_Del(struct RME_Cap_Captbl* Captbl,  rme_cid_t Cap_Captbl, rme_cid_t Cap_Pgtbl);
__EXTERN__ rme_ret_t _RME_Pgtbl_Add(struct RME_Cap_Captbl* Captbl, 
                                    rme_cid_t Cap_Pgtbl_Dst, rme_ptr_t Pos_Dst, rme_ptr_t Flags_Dst,
                                    rme_cid_t Cap_Pgtbl_Src, rme_ptr_t Pos_Src, rme_ptr_t Index);
__EXTERN__ rme_ret_t _RME_Pgtbl_Rem(struct RME_Cap_Captbl* Captbl, rme_cid_t Cap_Pgtbl, rme_ptr_t Pos);
__EXTERN__ rme_ret_t _RME_Pgtbl_Con(struct RME_Cap_Captbl* Captbl,
                                    rme_cid_t Cap_Pgtbl_Parent, rme_ptr_t Pos,
                                    rme_cid_t Cap_Pgtbl_Child, rme_ptr_t Flags_Child);
__EXTERN__ rme_ret_t _RME_Pgtbl_Des(struct RME_Cap_Captbl* Captbl, rme_cid_t Cap_Pgtbl, rme_ptr_t Pos);
/*****************************************************************************/
/* Undefine "__EXTERN__" to avoid redefinition */
#undef __EXTERN__
/* __RME_PGTBL_MEMBERS__ */
#endif
/* !(defined __HDR_DEFS__||defined __HDR_STRUCTS__) */
#endif
/* End Public C Function Prototypes ******************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
