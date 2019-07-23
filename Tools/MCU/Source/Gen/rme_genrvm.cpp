/******************************************************************************
Filename    : rme_genrvm.cpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The rvm folder generation class.
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
#include "Gen/rme_genrvm.hpp"
#undef __HDR_DEFS__

#define __HDR_CLASSES__
#include "Gen/rme_doc.hpp"
#include "Gen/rme_genrvm.hpp"
#undef __HDR_CLASSES__
/* End Includes **************************************************************/
namespace rme_mcu
{
/* Begin Function:RVM_User::Read **********************************************
Description : Read the rvm_user.c file, which contains all the user modifiable functions.
Input       : FILE* File - The file to read from.
Output      : None.
Return      : None.
******************************************************************************/
void RVM_User::Read(FILE* File)
{
    /* Currently left empty - should construct the rvm_user.c document tree */
}
/* End Function:RVM_User::Read ***********************************************/
}
/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
