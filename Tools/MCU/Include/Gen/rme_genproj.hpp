/******************************************************************************
Filename    : rme_genproj.hpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The header for the project user-modifiable file class.
******************************************************************************/

/* Defines *******************************************************************/
namespace rme_mcu
{
#ifdef __HDR_DEFS__
#ifndef __RME_GENPROC_HPP_DEFS__
#define __RME_GENPROC_HPP_DEFS__
/*****************************************************************************/

/*****************************************************************************/
/* __RME_GENPROC_HPP_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Classes *******************************************************************/
#ifdef __HDR_CLASSES__
#ifndef __RME_GENPROC_HPP_CLASSES__
#define __RME_GENPROC_HPP_CLASSES__
/*****************************************************************************/
/* Process user changeable file */
class Proc_User:public Doc
{
public:
    void Read(FILE* File);
};
/*****************************************************************************/
/* __RME_GENPROC_HPP_CLASSES__ */
#endif
/* __HDR_CLASSES__ */
#endif
}
/* End Classes ***************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
