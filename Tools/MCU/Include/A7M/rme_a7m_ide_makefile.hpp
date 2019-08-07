/******************************************************************************
Filename    : rme_a7m_ide_makefile.hpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The header for the ARMv7-M port's Keil IDE port.
******************************************************************************/

/* Defines *******************************************************************/
namespace rme_mcu
{
#ifdef __HDR_DEFS__
#ifndef __RME_A7M_IDE_MAKEFILE_HPP_DEFS__
#define __RME_A7M_IDE_MAKEFILE_HPP_DEFS__
/*****************************************************************************/

/*****************************************************************************/
/* __RME_A7M_IDE_MAKEFILE_HPP_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Classes *******************************************************************/
#ifdef __HDR_CLASSES__
#ifndef __RME_A7M_IDE_MAKEFILE_HPP_CLASSES__
#define __RME_A7M_IDE_MAKEFILE_HPP_CLASSES__
/*****************************************************************************/
/* RVM user changeable file */
class A7M_IDE_Makefile
{
public:
    static void RME_Proj(class Main* Main);
    static void RVM_Proj(class Main* Main);
    static void Proc_Proj(class Main* Main, class Proc* Proc);
};
/*****************************************************************************/
/* __RME_A7M_IDE_MAKEFILE_HPP_CLASSES__ */
#endif
/* __HDR_CLASSES__ */
#endif
}
/* End Classes ***************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
