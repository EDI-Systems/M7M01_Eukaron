/******************************************************************************
Filename    : rme_pgtbl.cpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The page table class.
******************************************************************************/

/* Includes ******************************************************************/
#include "string"
#include "memory"
#include "vector"
#include "stdexcept"

#define __HDR_DEFS__
#include "Core/rme_mcu.hpp"
#include "Core/rme_kobj.hpp"
#include "Core/rme_pgtbl.hpp"
#undef __HDR_DEFS__

#define __HDR_CLASSES__
#include "Core/rme_kobj.hpp"
#include "Core/rme_pgtbl.hpp"
#undef __HDR_CLASSES__
/* End Includes **************************************************************/
namespace rme_mcu
{
/* Begin Function:Pgtbl::Pgtbl ************************************************
Description : Constructor for Pgtbl class.
Input       : ptr_t Start_Addr - The start address.
              ptr_t Size_Order - The size order.
              ptr_t Num_Order - The number order.
              ptr_t Attr - The attributes on the page directory itself.
Output      : None.
Return      : None.
******************************************************************************/
/* void */ Pgtbl::Pgtbl(ptr_t Start_Addr, ptr_t Size_Order, ptr_t Num_Order, ptr_t Attr)
{
    try
    {
        this->Is_Top=0;
        this->Start_Addr=Start_Addr;
        this->Size_Order=Size_Order;
        this->Num_Order=Num_Order;
        this->Attr=Attr;

        this->Page.resize(POW2(Num_Order));
        std::fill(this->Page.begin(), this->Page.end(), 0);

        /* Automatically filled with nullptr */
        this->Pgdir.resize(POW2(Num_Order));
    }
    catch(std::exception& Exc)
    {
        throw std::runtime_error(std::string("Page table creation:\n")+Exc.what());
    }
}
/* End Function:Pgtbl::Pgtbl *************************************************/
}
/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
