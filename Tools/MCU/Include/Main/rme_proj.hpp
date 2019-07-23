/******************************************************************************
Filename    : rme_proj.hpp
Author      : pry
Date        : 08/04/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The header for the project reader.
******************************************************************************/

/* Defines *******************************************************************/
namespace rme_mcu
{
#ifdef __HDR_DEFS__
#ifndef __RME_PROJ_HPP_DEFS__
#define __RME_PROJ_HPP_DEFS__
/*****************************************************************************/
#define RAW_CAPTBL_SIZE(BITS,NUM)               ((BITS)/8*8*(NUM))
#define RAW_PROC_SIZE(BITS)                     ((BITS)/8*3)
#define RAW_SIG_SIZE(BITS)                      ((BITS)/8*3)
/*****************************************************************************/
/* __RME_PROJ_HPP_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Classes *******************************************************************/
#ifdef __HDR_CLASSES__
#ifndef __RME_PROJ_HPP_CLASSES__
#define __RME_PROJ_HPP_CLASSES__
/*****************************************************************************/
class Plat
{
public:
    ptr_t Kmem_Order;
    ptr_t Word_Bits;
    ptr_t Capacity;
    ptr_t Init_Num_Ord;
    ptr_t Raw_Thd_Size;
    ptr_t Raw_Inv_Size;

    Plat(ptr_t Word_Bits, ptr_t Init_Num_Ord, ptr_t Thd_Size, ptr_t Inv_Size);

    /* Kernel object sizes */
    virtual ptr_t Captbl_Size(ptr_t Entry) final;
    virtual ptr_t Captbl_Num(ptr_t Entry) final;
    virtual ptr_t Captbl_Total(ptr_t Entry) final;
    virtual ptr_t Pgtbl_Size(ptr_t Num_Order, ptr_t Is_Top) final;
    virtual ptr_t Proc_Size(void) final;
    virtual ptr_t Thd_Size(void) final;
    virtual ptr_t Inv_Size(void) final;
    virtual ptr_t Sig_Size(void) final;
    
    /* Raw page table size */
    virtual ptr_t Raw_Pgtbl_Size(ptr_t Num_Order, ptr_t Is_Top)=0;
    /* Align memory */
    virtual void Align_Mem(std::unique_ptr<class Proj>& Proj)=0;
    /* Allocate page table */
    virtual void Alloc_Pgtbl(std::unique_ptr<class Proj>& Proj, std::unique_ptr<class Chip>& Chip)=0;
};

/* For parsing and storing project information */
class Proj
{
public:
    /* The name of the project */
	std::unique_ptr<std::string> Name;
    /* The platform used */
    std::unique_ptr<std::string> Plat_Name;
    /* The all-lower-case of the platform used */
    std::unique_ptr<std::string> Plat_Lower;
    /* The chip class used */
	std::unique_ptr<std::string> Chip_Class;
    /* The full name of the exact chip used */
    std::unique_ptr<std::string> Chip_Full;
    
    /* The RME kernel information */
	std::unique_ptr<class RME> RME;
    /* The RVM user-library information */
	std::unique_ptr<class RVM> RVM;
    /* The process information */
	std::vector<std::unique_ptr<class Proc>> Proc;

    Proj(xml_node_t* Node);
    
    void Kobj_Alloc(std::unique_ptr<class Plat>& Plat, ptr_t Init_Capsz);
};
/*****************************************************************************/
/* __RME_PROJ_HPP_CLASSES__ */
#endif
/* __HDR_CLASSES__ */
#endif
}
/* End Classes ***************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
