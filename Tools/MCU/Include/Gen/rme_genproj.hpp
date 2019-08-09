/******************************************************************************
Filename    : rme_genproj.hpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The header for the whole project generation class.
******************************************************************************/

/* Defines *******************************************************************/
namespace rme_mcu
{
#ifdef __HDR_DEFS__
#ifndef __RME_GENPROJ_HPP_DEFS__
#define __RME_GENPROJ_HPP_DEFS__
/*****************************************************************************/

/*****************************************************************************/
/* __RME_GENPROJ_HPP_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Classes *******************************************************************/
#ifdef __HDR_CLASSES__
#ifndef __RME_GENPROJ_HPP_CLASSES__
#define __RME_GENPROJ_HPP_CLASSES__
/*****************************************************************************/
/* Process user changeable file */
class Proj_Gen
{
public:
    class Main* Main;

    virtual ~Proj_Gen(void){};
    
    virtual void Plat_Gen(void)=0;
};
/*****************************************************************************/
/* __RME_GENPROJ_HPP_CLASSES__ */
#endif
/* __HDR_CLASSES__ */
#endif
}
/* End Classes ***************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
