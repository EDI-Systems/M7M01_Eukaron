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
#ifndef __RME_PORT_HPP_DEFS__
#define __RME_PORT_HPP_DEFS__
/*****************************************************************************/

/*****************************************************************************/
/* __RME_PORT_HPP_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Classes *******************************************************************/
#ifdef __HDR_CLASSES__
#ifndef __RME_PORT_HPP_CLASSES__
#define __RME_PORT_HPP_CLASSES__

/* Use defines in these headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__

/*****************************************************************************/
/* Port information */
class Port:public Kobj
{
public:
    std::unique_ptr<std::string> Proc_Name;

    Port(xml_node_t* Node);
    ~Port(void);
};
/*****************************************************************************/
/* __RME_PORT_HPP_CLASSES__ */
#endif
/* __HDR_CLASSES__ */
#endif
}
/* End C++ Classes ***********************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
