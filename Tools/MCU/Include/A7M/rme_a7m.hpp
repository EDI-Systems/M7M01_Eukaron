/******************************************************************************
Filename    : rme_a7m.hpp
Author      : pry
Date        : 08/04/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The header of the mcu tool.
******************************************************************************/

/* Defines *******************************************************************/
namespace rme_mcu
{
#ifdef __HDR_DEFS__
#ifndef __RME_A7M_HPP_DEFS__
#define __RME_A7M_HPP_DEFS__
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

/* Alignment requirements for A7M */
#define A7M_MEM_ALIGN           (0x20)
/*****************************************************************************/
/* __RME_A7M_HPP_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Classes *******************************************************************/
#ifdef __HDR_CLASSES__
#ifndef __RME_A7M_HPP_CLASSES__
#define __RME_A7M_HPP_CLASSES__
/*****************************************************************************/
/* A7M-specific project information */
class A7M:public Plat
{
private:
    void Align_Block(std::unique_ptr<class Mem>& Mem);

    ptr_t Pgtbl_Tot_Ord(std::vector<std::unique_ptr<class Mem>>& List, ptr_t* Start_Addr);
    ptr_t Pgtbl_Num_Ord(std::vector<std::unique_ptr<class Mem>>& List, ptr_t Total_Order, ptr_t Start_Addr);
    void Map_Page(std::vector<std::unique_ptr<class Mem>>& List, std::unique_ptr<class Pgtbl>& Pgtbl);
    void Map_Pgdir(std::unique_ptr<class Proj>& Proj,
                   std::unique_ptr<class Proc>& Proc,
                   std::vector<std::unique_ptr<class Mem>>& List, 
                   std::unique_ptr<class Pgtbl>& Pgtbl);
    std::unique_ptr<class Pgtbl> Gen_Pgtbl(std::unique_ptr<class Proj>& Proj,
                                           std::unique_ptr<class Proc>& Proc,
                                           std::vector<std::unique_ptr<class Mem>>& List,
                                           ptr_t Total_Max);

public:
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

    A7M(std::unique_ptr<class Proj>& Proj, std::unique_ptr<class Chip>& Chip);
    ~A7M(void){};

    virtual ptr_t Pgtbl_Size(ptr_t Num_Order, ptr_t Is_Top) final override;
    virtual void Align_Mem(std::unique_ptr<class Proj>& Proj) final override;
    virtual void Alloc_Pgtbl(std::unique_ptr<class Proj>& Proj, std::unique_ptr<class Chip>& Chip) final override;
};
/*****************************************************************************/
/* __RME_A7M_HPP_CLASSES__ */
#endif
/* __HDR_CLASSES__ */
#endif
}
/* End Classes ***************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/