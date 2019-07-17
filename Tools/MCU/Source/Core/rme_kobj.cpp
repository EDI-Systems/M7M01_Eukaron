/******************************************************************************
Filename    : rme_kobj.cpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The kernel object class.
******************************************************************************/

/* Includes ******************************************************************/
#include "string"
#include "memory"

#define __HDR_DEFS__
#include "Core/rme_mcu.hpp"
#include "Core/rme_kobj.hpp"
#undef __HDR_DEFS__

#define __HDR_CLASSES__
#include "Core/rme_kobj.hpp"
#undef __HDR_CLASSES__
/* End Includes **************************************************************/
namespace rme_mcu
{
/* Begin Function:Kobj::Kobj **************************************************
Description : Constructor for Kobj class.
Input       : std::unique_ptr<std::string> Name - The name of the kernel object.
Output      : None.
Return      : None.
******************************************************************************/
/* void */ Kobj::Kobj(std::unique_ptr<std::string> Name)
{
    this->Name=std::move(Name);
}
/* End Function:Kobj::Kobj ***************************************************/
}
/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
