/******************************************************************************
Filename    : rme_kobj.hpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The header for the kernel object class.
******************************************************************************/

/* Defines *******************************************************************/
namespace rme_mcu
{
#ifdef __HDR_DEFS__
#ifndef __RME_KOBJ_HPP_DEFS__
#define __RME_KOBJ_HPP_DEFS__
/*****************************************************************************/

/*****************************************************************************/
/* __RME_MCU_HPP_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Classes *******************************************************************/
#ifdef __HDR_CLASSES__
#ifndef __RME_KOBJ_HPP_CLASSES__
#define __RME_KOBJ_HPP_CLASSES__

/* Use defines in these headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__

/*****************************************************************************/
/* Kernel object information */
class Kobj
{
public:
    /* Name of the kernel object */
    std::unique_ptr<std::string> Name;
    /* The local capid of the port */
    ptr_t Loc_Capid;
    /* The global linear capid of the endpoint */
    ptr_t RVM_Capid;
    /* The macro denoting the global capid */
    std::unique_ptr<std::string> Loc_Macro;
    /* The macro denoting the global capid */
    std::unique_ptr<std::string> RVM_Macro;
    /* The macro denoting the global capid - for RME */
    std::unique_ptr<std::string> RME_Macro;
 
    Kobj(void){};
    ~Kobj(void){};
};
/*****************************************************************************/
/* __RME_KOBJ_HPP_CLASSES__ */
#endif
/* __HDR_CLASSES__ */
#endif
}
/* End C++ Classes ***********************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
