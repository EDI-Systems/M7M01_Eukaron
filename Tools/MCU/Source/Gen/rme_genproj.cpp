/******************************************************************************
Filename    : rme_genproj.cpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The project folder generation class.
******************************************************************************/

/* Includes ******************************************************************/
#include "list"
#include "string"
#include "memory"
#include "vector"
#include "stdexcept"

#define __HDR_DEFS__
#include "Main/rme_mcu.hpp"

#include "Gen/rme_doc.hpp"
#include "Gen/rme_genproc.hpp"
#undef __HDR_DEFS__

#define __HDR_CLASSES__
#include "Gen/rme_doc.hpp"
#include "Gen/rme_genproc.hpp"
#undef __HDR_CLASSES__
/* End Includes **************************************************************/
namespace rme_mcu
{
/* Begin Function:Proc_User::Read ********************************************
Description : Read the proc_user.c file, which contains all the user modifiable functions.
Input       : FILE* File - The file to read from.
Output      : None.
Return      : None.
******************************************************************************/
void Proc_User::Read(FILE* File)
{
    /* Currently left empty - should construct the proc_user.c document tree */
}
/* End Function:Proc_User::Read **********************************************/
}
/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
