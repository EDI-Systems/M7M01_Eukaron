/******************************************************************************
Filename    : rme_proc.hpp
Author      : pry
Date        : 08/04/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The header for the process.
******************************************************************************/

/* Defines *******************************************************************/
namespace rme_mcu
{
#ifdef __HDR_DEFS__
#ifndef __RME_PROC_HPP_DEFS__
#define __RME_PROC_HPP_DEFS__
/*****************************************************************************/

/*****************************************************************************/
/* __RME_MCU_HPP_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Classes *******************************************************************/
#ifdef __HDR_CLASSES__
#ifndef __RME_PROC_HPP_CLASSES__
#define __RME_PROC_HPP_CLASSES__

/* Use defines in these headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__

/*****************************************************************************/
 /* Process memory map */
 class Proc_Memmap
{
public:
    /* Code memory */
    ptr_t Code_Base;
    ptr_t Code_Size;
    /* Data memory */
    ptr_t Data_Base;
    ptr_t Data_Size;
    /* Code memory frontier for entries */
    ptr_t Entry_Code_Front;

    Proc_Memmap(void){};
    ~Proc_Memmap(void){};
};

/* For parsing and storing process information */
class Proc:public Kobj
{
public:
    /* Extra first level captbl capacity required */
	ptr_t Extra_Captbl;
    /* Current local capability table frontier */ 
    ptr_t Captbl_Front;

    /* Compiler information */
    std::unique_ptr<class Comp> Comp;

    /* Memory trunk information */
    std::vector<std::unique_ptr<class Mem>> Code;
    std::vector<std::unique_ptr<class Mem>> Data;
    std::vector<std::unique_ptr<class Mem>> Device;

    /* Kernel object information */
    std::unique_ptr<class Captbl> Captbl;
    std::shared_ptr<class Pgtbl> Pgtbl;
    std::vector<std::unique_ptr<class Thd>> Thd;
    std::vector<std::unique_ptr<class Inv>> Inv;
    std::vector<std::unique_ptr<class Port>> Port;
    std::vector<std::unique_ptr<class Recv>> Recv;
    std::vector<std::unique_ptr<class Send>> Send;
    std::vector<std::unique_ptr<class Vect>> Vect;

    /* Memory map for itself */
    std::unique_ptr<class Proc_Memmap> Map;

    Proc(xml_node_t* Node);
    ~Proc(void){};
};
/*****************************************************************************/
/* __RME_PROC_HPP_CLASSES__ */
#endif
/* __HDR_CLASSES__ */
#endif
}
/* End C++ Classes ***********************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
