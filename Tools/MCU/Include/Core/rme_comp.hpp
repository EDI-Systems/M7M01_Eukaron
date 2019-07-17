/******************************************************************************
Filename    : rme_comp.hpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The header for the compiler option class.
******************************************************************************/

/* Defines *******************************************************************/
namespace rme_mcu
{
#ifdef __HDR_DEFS__
#ifndef __RME_COMP_HPP_DEFS__
#define __RME_COMP_HPP_DEFS__
/*****************************************************************************/
/* Optimization levels */
#define OPT_O0              (0)
#define OPT_O1              (1)
#define OPT_O2              (2)
#define OPT_O3              (3)
/* Time or size optimization choice */
#define OPT_SIZE            (0)
#define OPT_TIME            (1)
/*****************************************************************************/
/* __RME_COMP_HPP_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Classes *******************************************************************/
#ifdef __HDR_CLASSES__
#ifndef __RME_COMP_HPP_CLASSES__
#define __RME_COMP_HPP_CLASSES__

/* Use defines in these headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__

/*****************************************************************************/
/* Send endpoint information */
class Comp
{
public:
    /* Optimization level */
    ptr_t Opt;
    /* Prioritization */
    ptr_t Prio;

    Comp(xml_node_t* Node);
    ~Comp(void);
};
/*****************************************************************************/
/* __RME_COMP_HPP_CLASSES__ */
#endif
/* __HDR_CLASSES__ */
#endif
}
/* End C++ Classes ***********************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
