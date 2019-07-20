/******************************************************************************
Filename    : rme_pgtbl.hpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The header for the page table class.
******************************************************************************/

/* Defines *******************************************************************/
namespace rme_mcu
{
#ifdef __HDR_DEFS__
#ifndef __RME_PGTBL_HPP_DEFS__
#define __RME_PGTBL_HPP_DEFS__
/*****************************************************************************/
    
/*****************************************************************************/
/* __RME_PGTBL_HPP_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Classes *******************************************************************/
#ifdef __HDR_CLASSES__
#ifndef __RME_PGTBL_HPP_CLASSES__
#define __RME_PGTBL_HPP_CLASSES__
/*****************************************************************************/
/* Page table information */
class Pgtbl:public Kobj
{
public:
    /* Is this a top-level? */
    ptr_t Is_Top;
    /* The start address of the page table */
    ptr_t Start_Addr;
    /* The size order */
    ptr_t Size_Order;
    /* The number order */
    ptr_t Num_Order;
    /* The attribute (on this table) */
    ptr_t Attr;
    /* Page directories mapped in */
    std::vector<std::unique_ptr<class Pgtbl>> Pgdir;
    /* Pages mapped in - if not 0, then attr is directly here */
    std::vector<ptr_t> Page;

    Pgtbl(void){};
    ~Pgtbl(void){};
};
/*****************************************************************************/
/* __RME_PGTBL_HPP_CLASSES__ */
#endif
/* __HDR_CLASSES__ */
#endif
}
/* End Classes ***************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
