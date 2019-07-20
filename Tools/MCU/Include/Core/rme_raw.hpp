/******************************************************************************
Filename    : rme_raw.hpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The header for the raw information class.
******************************************************************************/

/* Defines *******************************************************************/
namespace rme_mcu
{
#ifdef __HDR_DEFS__
#ifndef __RME_RAW_HPP_DEFS__
#define __RME_RAW_HPP_DEFS__
/*****************************************************************************/
    
/*****************************************************************************/
/* __RME_RAW_HPP_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Classes *******************************************************************/
#ifdef __HDR_CLASSES__
#ifndef __RME_RAW_HPP_CLASSES__
#define __RME_RAW_HPP_CLASSES__
/*****************************************************************************/
/* Raw information */
class Raw
{
public:
    std::unique_ptr<std::string> Tag;
    std::unique_ptr<std::string> Val;

    Raw(xml_node_t* Node);
    ~Raw(void){};
};
/*****************************************************************************/
/* __RME_RAW_HPP_CLASSES__ */
#endif
/* __HDR_CLASSES__ */
#endif
}
/* End Classes ***************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
