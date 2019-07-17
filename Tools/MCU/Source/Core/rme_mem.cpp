/******************************************************************************
Filename    : rme_send.cpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The send endpoint class.
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
#include "Core/rme_kobj.hpp"
#include "Core/rme_mem.hpp"
#undef __HDR_DEFS__

#define __HDR_CLASSES__
#include "Core/rme_kobj.hpp"
#include "Core/rme_mem.hpp"
#undef __HDR_CLASSES__
/* End Includes **************************************************************/
namespace rme_mcu
{
/* Begin Function:Mem::Mem ****************************************************
Description : Constructor for Send class.
Input       : xml_node_t* Node - The node containing the receive endpoint.
Output      : None.
Return      : None.
******************************************************************************/
/* void */ Mem::Mem(xml_node_t* Node)
{
    xml_node_t* Temp;
    std::unique_ptr<std::string> Str;

    try
    {
        /* Start */
        if((XML_Child(Node,"Start",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Start section is missing.");
        if((Temp->XML_Val_Len==4)&&(std::string(Temp->XML_Val,4)=="Auto"))
            this->Start=AUTO;
        else if(XML_Get_Hex(Temp,&(this->Start))<0)
            throw std::invalid_argument("Start is not a valid hex integer.");

        /* Size */
        if((XML_Child(Node,"Size",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Size section is missing.");
        if(XML_Get_Hex(Temp,&(this->Size))<0)
            throw std::invalid_argument("Size is not a valid hex integer.");
        if(this->Size==0)
            throw std::invalid_argument("Size cannot be zero.");
        if(this->Start!=AUTO)
        {
            if((this->Start+this->Size)>0x100000000ULL)
                throw std::invalid_argument("Size is out of bound.");
        }
        else
        {
            if(this->Size>0x100000000ULL)
                throw std::invalid_argument("Size is out of bound.");
        }

        /* Type */
        if((XML_Child(Node,"Type",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Type section is missing.");
        if((Temp->XML_Val_Len==6)&&(std::string(Temp->XML_Val,6)=="Device"))
        {
            if(this->Start==AUTO)
                throw std::invalid_argument("Device typed memory cannot be automatically allocated.");
        }

        /* Attribute */
        if((XML_Child(Node,"Attribute",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Attribute section is missing.");
        if(Temp->XML_Val_Len==0)
            throw std::invalid_argument("Attribute section is empty.");
        Str=std::make_unique<std::string>(Temp->XML_Tag,(int)Temp->XML_Tag_Len);
        this->Attr=0;

        if(Str->rfind('R')!=std::string::npos)
            this->Attr|=MEM_READ;
        if(Str->rfind('W')!=std::string::npos)
            this->Attr|=MEM_WRITE;
        if(Str->rfind('X')!=std::string::npos)
            this->Attr|=MEM_EXECUTE;

        if(this->Attr==0)
            throw std::invalid_argument("Attribute does not allow any access and is malformed.");

        if(Str->rfind('B')!=std::string::npos)
            this->Attr|=MEM_BUFFERABLE;
        if(Str->rfind('C')!=std::string::npos)
            this->Attr|=MEM_CACHEABLE;
        if(Str->rfind('S')!=std::string::npos)
            this->Attr|=MEM_STATIC;
    }
    catch(std::exception& Exc)
    {
        throw std::runtime_error(std::string("Memory:\n")+Exc.what());
    }
}
/* End Function:Mem::Mem *****************************************************/
}
/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
