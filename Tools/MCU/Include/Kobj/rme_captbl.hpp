/******************************************************************************
Filename    : rme_captbl.hpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The header for the capability table class.
******************************************************************************/

/* Defines *******************************************************************/
namespace rme_mcu
{
#ifdef __HDR_DEFS__
#ifndef __RME_CAPTBL_HPP_DEFS__
#define __RME_CAPTBL_HPP_DEFS__
/*****************************************************************************/

/*****************************************************************************/
/* __RME_CAPTBL_HPP_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Classes *******************************************************************/
#ifdef __HDR_CLASSES__
#ifndef __RME_CAPTBL_HPP_CLASSES__
#define __RME_CAPTBL_HPP_CLASSES__
/*****************************************************************************/
/* Capability table information */
class Captbl:public Kobj
{
public:
    /* The frontier */
    ptr_t Front;
    /* Extra size */
    ptr_t Extra;
    /* The ultimate size */
    ptr_t Size;
};
/*****************************************************************************/
/* __RME_CAPTBL_HPP_CLASSES__ */
#endif
/* __HDR_CLASSES__ */
#endif
}
/* End Classes ***************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
