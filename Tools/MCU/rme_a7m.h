/******************************************************************************
Filename    : rme_a7m.h
Author      : pry
Date        : 08/04/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The header of the mcu tool.
******************************************************************************/

/* Defines *******************************************************************/
#ifdef __HDR_DEFS__
#ifndef __RME_A7M_H_DEFS__
#define __RME_A7M_H_DEFS__
/*****************************************************************************/
/* NVIC grouping */
#define A7M_NVIC_P0S8           (7)    
#define A7M_NVIC_P1S7           (6)
#define A7M_NVIC_P2S6           (5)
#define A7M_NVIC_P3S5           (4)
#define A7M_NVIC_P4S4           (3)
#define A7M_NVIC_P5S3           (2)
#define A7M_NVIC_P6S2           (1)
#define A7M_NVIC_P7S1           (0)
/* CPU type */
#define A7M_CPU_CM0P            (0)
#define A7M_CPU_CM3             (1)
#define A7M_CPU_CM4             (2)
#define A7M_CPU_CM7             (3)
/* FPU type */
#define A7M_FPU_NONE            (0)
#define A7M_FPU_FPV4            (1)
#define A7M_FPU_FPV5_SP         (2)
#define A7M_FPU_FPV5_DP         (3)
/* Endianness */
#define A7M_END_LITTLE          (0)
#define A7M_END_BIG             (1)

/* Page table */
#define A7M_PGT_NOTHING         (0)
#define A7M_PGT_MAPPED          ((struct A7M_Pgtbl*)(-1))

/* RVM page table capability table size */
#define A7M_PGTBL_CAPTBL_SIZE   (4096)
/* The process capability table size limit */
#define A7M_PROC_CAPTBL_LIMIT   (128)
/* The A7M boot-time capability table starting slot */
#define A7M_CAPTBL_START        (8)
/*****************************************************************************/
/* __RME_A7M_H_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Structs *******************************************************************/
#ifdef __HDR_STRUCTS__
#ifndef __RME_A7M_H_STRUCTS__
#define __RME_A7M_H_STRUCTS__

/* Use defines in these headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__

/*****************************************************************************/
struct A7M_Pgtbl
{
    /* The start address of the page table */
    ptr_t Start_Addr;
    /* The size order */
    ptr_t Size_Order;
    /* The number order */
    ptr_t Num_Order;
    /* The attribute */
    ptr_t Attr;
    /* The global linear capability ID */
    ptr_t RVM_Capid;
    /* The macro corresponding to the global capid */
    s8_t* RVM_Capid_Macro;
    /* Whether we have the 8 subregions mapped: 0 - not mapped 1 - mapped other - pointer to the next */
    struct A7M_Pgtbl* Mapping[8];
};

struct A7M_Info
{
    /* The NVIC grouping */
	ptr_t NVIC_Grouping;
    /* The systick value */
	ptr_t Systick_Val;
    /* The CPU type */
    ptr_t CPU_Type;
    /* The FPU type */
    ptr_t FPU_Type;
    /* Endianness - big or little */
    ptr_t Endianness;
    /* The page tables for all processes */
    struct A7M_Pgtbl** Pgtbl;
    /* Global captbl containing pgtbls */
    ptr_t Pgtbl_Captbl_Front;
    struct RVM_Cap_Info* Pgtbl_Captbl;
};
/*****************************************************************************/
/* __RME_A7M_H_STRUCTS__ */
#endif
/* __HDR_STRUCTS__ */
#endif
/* End Structs ***************************************************************/

/* Private Global Variables **************************************************/
#if(!(defined __HDR_DEFS__||defined __HDR_STRUCTS__))
#ifndef __RME_A7M_MEMBERS__
#define __RME_A7M_MEMBERS__

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
/* Generic *******************************************************************/

/*****************************************************************************/
/* Undefine "__EXTERN__" to avoid redefinition */
#undef __EXTERN__
/* __RME_A7M_MEMBERS__ */
#endif
/* !(defined __HDR_DEFS__||defined __HDR_STRUCTS__) */
#endif
/* End Public C Function Prototypes ******************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
