/******************************************************************************
Filename    : rme_proj.hpp
Author      : pry
Date        : 08/04/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The header for the project reader.
******************************************************************************/

/* Defines *******************************************************************/
namespace rme_mcu
{
#ifdef __HDR_DEFS__
#ifndef __RME_PROJ_HPP_DEFS__
#define __RME_PROJ_HPP_DEFS__
/*****************************************************************************/

/*****************************************************************************/
/* __RME_PROJ_HPP_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Classes *******************************************************************/
#ifdef __HDR_CLASSES__
#ifndef __RME_PROJ_HPP_CLASSES__
#define __RME_PROJ_HPP_CLASSES__

/* Use defines in these headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__

/*****************************************************************************/
/* For parsing and storing project information */
class Proj_Cls
{
public:
    /* The name of the project */
	std::string Name;
    /* The platform used */
    std::string Plat_Name;
    /* The all-lower-case of the platform used */
    std::string Lower_Plat;
    /* The chip class used */
	std::string Chip_Class;
    /* The full name of the exact chip used */
    std::string Chip_Full;
    
    /* The platform information */
    std::unique_ptr<Plat_Cls> Plat;
    /* The RME kernel information */
	std::unique_ptr<RME_Cls> RME;
    /* The RVM user-library information */
	std::unique_ptr<RVM_Cls> RVM;
    /* The process information */
	std::vector<std::unique_ptr<Proc_Cls>> Proc;

    Proj(std::string* Buf);
    ~Proj(void);
};
/*****************************************************************************/
/* __RME_PROJ_HPP_CLASSES__ */
#endif
/* __HDR_CLASSES__ */
#endif
}
/* End C++ Classes ***********************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
