/******************************************************************************
Filename    : rme_rmeuser.hpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The header for the RME user-modifiable file class.
******************************************************************************/

/* Defines *******************************************************************/
namespace rme_mcu
{
#ifdef __HDR_DEFS__
#ifndef __RME_GENRME_HPP_DEFS__
#define __RME_GENRME_HPP_DEFS__
/*****************************************************************************/

/*****************************************************************************/
/* __RME_GENRME_HPP_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Classes *******************************************************************/
#ifdef __HDR_CLASSES__
#ifndef __RME_GENRME_HPP_CLASSES__
#define __RME_GENRME_HPP_CLASSES__
/*****************************************************************************/
/* RME user changeable file */
class RME_User:public Doc
{
public:
    virtual void Read(FILE* File) final override;
};
/*****************************************************************************/
/* __RME_GENRME_HPP_CLASSES__ */
#endif
/* __HDR_CLASSES__ */
#endif
}
/* End Classes ***************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
