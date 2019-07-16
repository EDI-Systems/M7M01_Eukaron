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

/* Initial page table order */
#define A7M_INIT_PGTBL_NUM_ORD  (3)
/* Thread size and invocation size */
#define A7M_THD_SIZE            (64)
#define A7M_INV_SIZE            (32)
/* Page table size */
#define A7M_PGTBL_SIZE_TOP(NUM_ORDER)   (POW2(NUM_ORDER)*4)
#define A7M_PGTBL_SIZE_NOM(NUM_ORDER)   (POW2(NUM_ORDER)*4)
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
/* A7M-specific project information */
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
static ptr_t A7M_Pgtbl_Size(ptr_t Num_Order, ptr_t Is_Top);

static void A7M_Align(struct Mem_Info* Mem);

static ptr_t A7M_Total_Order(struct List* Mem_List, ptr_t* Start_Addr);
static ptr_t A7M_Num_Order(struct List* Mem_List, ptr_t Total_Order, ptr_t Start_Addr);
static void A7M_Map_Page(struct List* Mem_List, struct Pgtbl_Info* Pgtbl);
static void A7M_Map_Pgdir(struct Proj_Info* Proj, struct Proc_Info* Proc,
                          struct List* Mem_List, struct Pgtbl_Info* Pgtbl);
static struct Pgtbl_Info* A7M_Gen_Pgtbl(struct Proj_Info* Proj, struct Proc_Info* Proc,
                                        struct List* Mem_List, ptr_t Total_Max);

static void A7M_Gen_Keil_RME(struct Proj_Info* Proj, struct Chip_Info* Chip, s8_t* RME_Path, s8_t* Output_Path);
static void A7M_Gen_Keil_RVM(struct Proj_Info* Proj, struct Chip_Info* Chip, s8_t* RVM_Path, s8_t* Output_Path);
static void A7M_Gen_Keil_Proc(struct Proj_Info* Proj, struct Chip_Info* Chip, s8_t* RME_Path, s8_t* RVM_Path, s8_t* Output_Path);
static void A7M_Gen_Keil_Proj(FILE* Keil,
                              s8_t* Target, s8_t* Device, s8_t* Vendor, 
                              s8_t* CPU_Type, s8_t* FPU_Type, s8_t* Endianness,
                              ptr_t Timeopt, ptr_t Opt,
                              s8_t** Includes, ptr_t Include_Num,
                              s8_t** Paths, s8_t** Files, ptr_t File_Num);
static void A7M_Gen_Keil(struct Proj_Info* Proj, struct Chip_Info* Chip, s8_t* RME_Path, s8_t* RVM_Path, s8_t* Output_Path);

static void A7M_Gen_Makefile(struct Proj_Info* Proj, struct Chip_Info* Chip,
                             ptr_t Output_Type, s8_t* Output_Path, s8_t* RME_Path, s8_t* RVM_Path);

static void A7M_Parse_Options(struct Proj_Info* Proj, struct Chip_Info* Chip);
static void A7M_Check_Input(struct Proj_Info* Proj, struct Chip_Info* Chip);
static void A7M_Align_Mem(struct Proj_Info* Proj);
static void A7M_Alloc_Pgtbl(struct Proj_Info* Proj, struct Chip_Info* Chip);
static void A7M_Gen_Proj(struct Proj_Info* Proj, struct Chip_Info* Chip,
                         s8_t* RME_Path, s8_t* RVM_Path, s8_t* Output_Path, s8_t* Format);
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
static struct A7M_Info* A7M;
/*****************************************************************************/

/* End Public Global Variables ***********************************************/

/* Public C Function Prototypes **********************************************/
__EXTERN__ void A7M_Plat_Select(struct Proj_Info* Proj);
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
