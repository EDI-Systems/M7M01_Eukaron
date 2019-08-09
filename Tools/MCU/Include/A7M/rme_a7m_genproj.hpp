/******************************************************************************
Filename    : rme_a7m_genproj.hpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The header for the whole project generation class of ARMv7-M.
******************************************************************************/

/* Defines *******************************************************************/
namespace rme_mcu
{
#ifdef __HDR_DEFS__
#ifndef __RME_A7M_GENPROJ_HPP_DEFS__
#define __RME_A7M_GENPROJ_HPP_DEFS__
/*****************************************************************************/

/*****************************************************************************/
/* __RME_A7M_GENPROJ_HPP_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Classes *******************************************************************/
#ifdef __HDR_CLASSES__
#ifndef __RME_A7M_GENPROJ_HPP_CLASSES__
#define __RME_A7M_GENPROJ_HPP_CLASSES__
/*****************************************************************************/
class A7M_Proj_Gen:public Proj_Gen
{
    void File(void);
    void Proj(void);

public:
    virtual void Plat_Gen(void) final override;
};
/*****************************************************************************/
/* __RME_A7M_GENPROJ_HPP_CLASSES__ */
#endif
/* __HDR_CLASSES__ */
#endif
}
/* End Classes ***************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
