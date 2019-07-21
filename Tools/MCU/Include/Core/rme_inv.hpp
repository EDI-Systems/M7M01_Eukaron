/******************************************************************************
Filename    : rme_inv.hpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The header for the invocation class.
******************************************************************************/

/* Defines *******************************************************************/
namespace rme_mcu
{
#ifdef __HDR_DEFS__
#ifndef __RME_INV_HPP_DEFS__
#define __RME_INV_HPP_DEFS__
/*****************************************************************************/
    
/*****************************************************************************/
/* __RME_INV_HPP_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Classes *******************************************************************/
#ifdef __HDR_CLASSES__
#ifndef __RME_INV_HPP_CLASSES__
#define __RME_INV_HPP_CLASSES__
/*****************************************************************************/
/* Invocation memory map information */
class Inv_Memmap
{
public:
    /* Position of the entry address from the header */
    ptr_t Entry_Addr;
    /* The address of the stack */
    ptr_t Stack_Base;
    /* The size of the stack */
    ptr_t Stack_Size;
};

/* Invocation information */
class Inv:public Kobj
{
public:
    /* The entry of the thread */
	std::unique_ptr<std::string> Entry;
    /* The stack size of the thread */
	ptr_t Stack_Size;
    /* Memory map */
    std::unique_ptr<class Inv_Memmap> Map;
 
    Inv(xml_node_t* Node);
};
/*****************************************************************************/
/* __RME_INV_HPP_CLASSES__ */
#endif
/* __HDR_CLASSES__ */
#endif
}
/* End Classes ***************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
