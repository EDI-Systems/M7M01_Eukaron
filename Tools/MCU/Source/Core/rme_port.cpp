/******************************************************************************
Filename    : rme_port.cpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The port class.
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
#include "Core/rme_mcu.hpp"
#include "Core/rme_kobj.hpp"
#include "Core/rme_port.hpp"
#undef __HDR_DEFS__

#define __HDR_CLASSES__
#include "Core/rme_kobj.hpp"
#include "Core/rme_port.hpp"
#undef __HDR_CLASSES__
/* End Includes **************************************************************/
namespace rme_mcu
{
/* Begin Function:Port::Port **************************************************
Description : Constructor for Port class.
Input       : xml_node_t* Node - The node containing the port.
Output      : None.
Return      : None.
******************************************************************************/
/* void */ Port::Port(xml_node_t* Node)
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

        /* Proc_Name */
        if((XML_Child(Node,"Process",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Process name section is missing.");
        if(Temp->XML_Val_Len==0)
            throw std::invalid_argument("Process name section is empty.");
        this->Proc_Name=std::make_unique<std::string>(Temp->XML_Val,(int)Temp->XML_Val_Len);
    }
    catch(std::exception& Exc)
    {
        if(this->Name!=nullptr)
            throw std::runtime_error(std::string("Port: ")+*(this->Name)+"\n"+Exc.what());
        else
            throw std::runtime_error(std::string("Port: ")+"Unknown"+"\n"+Exc.what());
    }
}
/* End Function:Port::Port ***************************************************/
}
/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
