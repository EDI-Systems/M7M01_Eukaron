/******************************************************************************
Filename    : rme_recv.cpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The receive endpoint class.
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
#include "Kobj/rme_recv.hpp"
#undef __HDR_DEFS__

#define __HDR_CLASSES__
#include "Kobj/rme_kobj.hpp"
#include "Kobj/rme_recv.hpp"
#undef __HDR_CLASSES__
/* End Includes **************************************************************/
namespace rme_mcu
{
/* Begin Function:Recv::Recv **************************************************
Description : Constructor for Recv class.
Input       : xml_node_t* Node - The node containing the receive endpoint.
Output      : None.
Return      : None.
******************************************************************************/
/* void */ Recv::Recv(xml_node_t* Node)
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
    }
    catch(std::exception& Exc)
    {
        if(this->Name!=nullptr)
            throw std::runtime_error(std::string("Receive endpoint: ")+*(this->Name)+"\n"+Exc.what());
        else
            throw std::runtime_error(std::string("Receive endpoint: ")+"Unknown"+"\n"+Exc.what());
    }
}
/* End Function:Recv::Recv ***************************************************/
}
/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
