/******************************************************************************
Filename    : rme_inv.cpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The invocation class.
******************************************************************************/

/* Includes ******************************************************************/
#include "string"
#include "memory"
#include "vector"
#include "stdexcept"

extern "C"
{
#include "xml.h"
}

#define __HDR_DEFS__
#include "Main/rme_mcu.hpp"

#include "Kobj/rme_kobj.hpp"
#include "Kobj/rme_inv.hpp"
#undef __HDR_DEFS__

#define __HDR_CLASSES__
#include "Kobj/rme_kobj.hpp"
#include "Kobj/rme_inv.hpp"
#undef __HDR_CLASSES__
/* End Includes **************************************************************/
namespace rme_mcu
{
/* Begin Function:Inv::Inv ****************************************************
Description : Constructor for Inv class.
Input       : xml_node_t* Node - The node containing the invocation.
Output      : None.
Return      : None.
******************************************************************************/
/* void */ Inv::Inv(xml_node_t* Node)
{
    xml_node_t* Temp;

    try
    {
        /* Name */
        if((XML_Child(Node,"Name",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Name section is missing.");
        if(Temp->XML_Val_Len==0)
            throw std::invalid_argument("Name section is empty.");
        this->Name=std::make_unique<std::string>(Temp->XML_Val,(int)Temp->XML_Val_Len);

        /* Entry */
        if((XML_Child(Node,"Entry",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Entry section is missing.");
        if(Temp->XML_Val_Len==0)
            throw std::invalid_argument("Entry section is empty.");
        this->Entry=std::make_unique<std::string>(Temp->XML_Val,(int)Temp->XML_Val_Len);

        /* Stack Size */
        if((XML_Child(Node,"Stack_Size",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Stack size section is missing.");
        if(XML_Get_Hex(Temp,&(this->Stack_Size))<0)
            throw std::invalid_argument("Stack size is not a valid hex integer.");
        
        this->Map=std::make_unique<class Inv_Memmap>();

    }
    catch(std::exception& Exc)
    {
        if(this->Name!=nullptr)
            throw std::runtime_error(std::string("Invocation: ")+*(this->Name)+"\n"+Exc.what());
        else
            throw std::runtime_error(std::string("Invocation: ")+"Unknown"+"\n"+Exc.what());
    }
}
/* End Function:Inv::Inv *****************************************************/
}
/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
