/******************************************************************************
Filename    : rme_genproc.hpp
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
class Proc_Gen
{
public:
    class Main* Main;

    virtual ~Proc_Gen(void){};

    void Folder(class Proc* Proc);
    void Proc_Hdr(class Proc* Proc);
    void Proc_Src(class Proc* Proc);
    
    virtual void Plat_Gen(class Proc* Proc)=0;
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
