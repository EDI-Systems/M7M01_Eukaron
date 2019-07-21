/******************************************************************************
Filename    : rme_thd.hpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The header for the thread class.
******************************************************************************/

/* Defines *******************************************************************/
namespace rme_mcu
{
#ifdef __HDR_DEFS__
#ifndef __RME_THD_HPP_DEFS__
#define __RME_THD_HPP_DEFS__
/*****************************************************************************/

/*****************************************************************************/
/* __RME_THD_HPP_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Classes *******************************************************************/
#ifdef __HDR_CLASSES__
#ifndef __RME_THD_HPP_CLASSES__
#define __RME_THD_HPP_CLASSES__
/*****************************************************************************/
/* Thread memory map information */
class Thd_Memmap
{
public:
    /* Position of the entry address from the header */
    ptr_t Entry_Addr;
    /* Value of the parameter at creation time */
    ptr_t Param_Value;
    /* The address of the stack */
    ptr_t Stack_Base;
    /* The size of the stack */
    ptr_t Stack_Size;

    Thd_Memmap(void){};
    ~Thd_Memmap(void){};
};

/* Kernel object information */
class Thd:public Kobj
{
public:
    /* The entry of the thread */
	std::unique_ptr<std::string> Entry;
    /* The stack size of the thread */
	ptr_t Stack_Size;
    /* The parameter passed to the thread */
	ptr_t Param;
    /* The priority of the thread */
	ptr_t Prio;
    /* Memory map */
    std::unique_ptr<class Thd_Memmap> Map;
 
    Thd(xml_node_t* Node);
};
/*****************************************************************************/
/* __RME_THD_HPP_CLASSES__ */
#endif
/* __HDR_CLASSES__ */
#endif
}
/* End Classes ***************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
