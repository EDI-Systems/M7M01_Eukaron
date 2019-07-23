/******************************************************************************
Filename    : rme_plat.hpp
Author      : pry
Date        : 08/04/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The header for the platform abstract class.
******************************************************************************/

/* Defines *******************************************************************/
namespace rme_mcu
{
#ifdef __HDR_DEFS__
#ifndef __RME_PLAT_HPP_DEFS__
#define __RME_PLAT_HPP_DEFS__
/*****************************************************************************/
/*****************************************************************************/
/* __RME_PLAT_HPP_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Classes *******************************************************************/
#ifdef __HDR_CLASSES__
#ifndef __RME_PLAT_HPP_CLASSES__
#define __RME_PLAT_HPP_CLASSES__
/*****************************************************************************/
class Plat
{
public:
    ptr_t Word_Bits;
    ptr_t Capacity;
    ptr_t Init_Num_Ord;
    ptr_t Thd_Size;
    ptr_t Inv_Size;

    /* Page table size */
    virtual ptr_t Pgtbl_Size(ptr_t Num_Order, ptr_t Is_Top)=0;
    /* Check input validity */
    virtual void Check_Input(std::unique_ptr<class Proj>& Proj, std::unique_ptr<class Chip>& Chip)=0;
    /* Parse platform-related options */
    virtual void Parse_Options(std::unique_ptr<class Proj>& Proj, std::unique_ptr<class Chip>& Chip)=0;
    /* Align memory */
    virtual void Align_Mem(std::unique_ptr<class Proj>& Proj)=0;
    /* Allocate page table */
    virtual void Alloc_Pgtbl(std::unique_ptr<class Proj>& Proj, std::unique_ptr<class Chip>& Chip)=0;

    Plat(ptr_t Word_Bits, ptr_t Init_Num_Ord, ptr_t Thd_Size, ptr_t Inv_Size);
    ~Plat(void){};
};
/*****************************************************************************/
/* __RME_PLAT_HPP_CLASSES__ */
#endif
/* __HDR_CLASSES__ */
#endif
}
/* End Classes ***************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
