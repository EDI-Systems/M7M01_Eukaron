/******************************************************************************
Filename    : rme_a7m_genproc.hpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The header for the process generation class of ARMv7-M.
******************************************************************************/

/* Defines *******************************************************************/
namespace rme_mcu
{
#ifdef __HDR_DEFS__
#ifndef __RME_A7M_GENPROC_HPP_DEFS__
#define __RME_A7M_GENPROC_HPP_DEFS__
/*****************************************************************************/

/*****************************************************************************/
/* __RME_A7M_GENPROC_HPP_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Classes *******************************************************************/
#ifdef __HDR_CLASSES__
#ifndef __RME_A7M_GENPROC_HPP_CLASSES__
#define __RME_A7M_GENPROC_HPP_CLASSES__
/*****************************************************************************/
class A7M_Proc_Gen:public Proc_Gen
{
    void Chip_Hdr(class Proc* Proc);
    void Asm(class Proc* Proc);
    void Lds(class Proc* Proc);
    void Proj(class Proc* Proc);

public:
    virtual void Plat_Gen(class Proc* Proc) final override;
};
/*****************************************************************************/
/* __RME_A7M_GENPROC_HPP_CLASSES__ */
#endif
/* __HDR_CLASSES__ */
#endif
}
/* End Classes ***************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
