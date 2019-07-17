/******************************************************************************
Filename    : rme_comp.cpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The compiler class.
******************************************************************************/

/* Includes ******************************************************************/
#include "string"
#include "memory"
#include "stdexcept"

extern "C"
{
#include "xml.h"
}

#define __HDR_DEFS__
#include "Core/rme_mcu.hpp"
#include "Core/rme_comp.hpp"
#undef __HDR_DEFS__

#define __HDR_CLASSES__
#include "Core/rme_comp.hpp"
#undef __HDR_CLASSES__
/* End Includes **************************************************************/
namespace rme_mcu
{
/* Begin Function:Comp::Comp **************************************************
Description : Constructor for Send class.
Input       : xml_node_t* Node - The node containing the receive endpoint.
Output      : None.
Return      : None.
******************************************************************************/
/* void */ Comp::Comp(xml_node_t* Node)
{
    xml_node_t* Temp;
    std::unique_ptr<std::string> Str;

    try
    {
        /* Optimization level */
        if((XML_Child(Node,"Optimization",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Optimization level section is missing.");
        if(Temp->XML_Val_Len==0)
            throw std::invalid_argument("Optimization level section is empty.");

        Str=std::make_unique<std::string>(Temp->XML_Val,(int)Temp->XML_Val_Len);

        if(*Str=="O0")
            this->Opt=OPT_O0;
        else if(*Str=="O1")
            this->Opt=OPT_O1;
        else if(*Str=="O2")
            this->Opt=OPT_O2;
        else if(*Str=="O3")
            this->Opt=OPT_O3;
        else
            throw std::invalid_argument("Optimization level is malformed.");

        /* Time or size optimization */
        if((XML_Child(Node,"Prioritization",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Optimization prioritization section is missing.");
        if(Temp->XML_Val_Len==0)
            throw std::invalid_argument("Optimization prioritization section is empty.");

        Str=std::make_unique<std::string>(Temp->XML_Val,(int)Temp->XML_Val_Len);

        if(*Str=="Time")
            this->Prio=OPT_TIME;
        else if(*Str=="Size")
            this->Prio=OPT_SIZE;
        else
            throw std::invalid_argument("Optimization prioritization is malformed.");
    }
    catch(std::exception* Exc)
    {
        throw std::runtime_error(std::string("Compiler options: ")+"\n"+Exc->what());
    }
}
/* End Function:Comp::Comp ***************************************************/
}
/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
