/******************************************************************************
Filename    : rme_vect.hpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The header for the vector class.
******************************************************************************/

/* Defines *******************************************************************/
namespace rme_mcu
{
#ifdef __HDR_DEFS__
#ifndef __RME_VECT_HPP_DEFS__
#define __RME_VECT_HPP_DEFS__
/*****************************************************************************/
    
/*****************************************************************************/
/* __RME_VECT_HPP_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Classes *******************************************************************/
#ifdef __HDR_CLASSES__
#ifndef __RME_VECT_HPP_CLASSES__
#define __RME_VECT_HPP_CLASSES__
/*****************************************************************************/
/* Vector endpoint information */
class Vect:public Kobj
{
public:
    ptr_t Num;

    Vect(xml_node_t* Node);

    static std::string* Vect::Check_Vect(std::unique_ptr<class Proj>& Proj);
};
/*****************************************************************************/
/* __RME_VECT_HPP_CLASSES__ */
#endif
/* __HDR_CLASSES__ */
#endif
}
/* End Classes ***************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
