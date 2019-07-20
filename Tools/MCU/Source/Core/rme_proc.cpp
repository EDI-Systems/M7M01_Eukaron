/******************************************************************************
Filename    : rme_proc.cpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The send endpoint class.
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
#include "Core/rme_comp.hpp"
#include "Core/rme_mem.hpp"
#include "Core/rme_kobj.hpp"
#include "Core/rme_captbl.hpp"
#include "Core/rme_pgtbl.hpp"
#include "Core/rme_thd.hpp"
#include "Core/rme_inv.hpp"
#include "Core/rme_port.hpp"
#include "Core/rme_recv.hpp"
#include "Core/rme_send.hpp"
#include "Core/rme_vect.hpp"
#include "Core/rme_proc.hpp"
#undef __HDR_DEFS__

#define __HDR_CLASSES__
#include "Core/rme_comp.hpp"
#include "Core/rme_mem.hpp"
#include "Core/rme_kobj.hpp"
#include "Core/rme_captbl.hpp"
#include "Core/rme_pgtbl.hpp"
#include "Core/rme_thd.hpp"
#include "Core/rme_inv.hpp"
#include "Core/rme_port.hpp"
#include "Core/rme_recv.hpp"
#include "Core/rme_send.hpp"
#include "Core/rme_vect.hpp"
#include "Core/rme_proc.hpp"
#undef __HDR_CLASSES__
/* End Includes **************************************************************/
namespace rme_mcu
{
/* Begin Function:Proc::Proc **************************************************
Description : Constructor for Proc class.
Input       : xml_node_t* Node - The node containing the receive endpoint.
Output      : None.
Return      : None.
******************************************************************************/
/* void */ Proc::Proc(xml_node_t* Node)
{
    xml_node_t* Temp;
    xml_node_t* Trunk;
    xml_node_t* Mem_Type;
    std::unique_ptr<std::string> Str;

    try
    {
        /* Name */
        if((XML_Child(Node,"Name",&Temp)<0)||(Name==0))
            throw std::invalid_argument("Name section is missing.");
        if(Temp->XML_Val_Len==0)
            throw std::invalid_argument("Name section is empty.");
        this->Name=std::make_unique<std::string>(Temp->XML_Val,(int)Temp->XML_Val_Len);

        /* Extra_Captbl */
        if((XML_Child(Node,"Extra_Captbl",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Extra capability table size section is missing.");
        this->Captbl=std::make_unique<class Captbl>();
        if(XML_Get_Uint(Temp,&(this->Captbl->Extra))<0)
            throw std::invalid_argument("Extra capability table size is not a valid unsigned integer.");

        /* Compiler */
        if((XML_Child(Node,"Compiler",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Compiler section is missing.");
        this->Comp=std::make_unique<class Comp>(Temp);

        /* Memory */
        if((XML_Child(Node,"Memory",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Memory section is missing.");
        if(XML_Child(Temp,0,&Trunk)<0)
            throw std::invalid_argument("Memory section parsing internal error.");
        if(Trunk==0)
            throw std::invalid_argument("Memory section is empty.");
        while(Trunk!=0)
        {
            if((XML_Child(Trunk,"Type",&Mem_Type)<0)||(Mem_Type==0))
                throw std::invalid_argument("Memory type section is missing.");
            if(Mem_Type->XML_Val_Len==0)
                throw std::invalid_argument("Memory type section is empty.");
            
            Str=std::make_unique<std::string>(Mem_Type->XML_Val,(int)Mem_Type->XML_Val_Len);

            if(*Str=="Code")
                this->Code.push_back(std::make_unique<class Mem>(Trunk));
            else if(*Str=="Data")
                this->Data.push_back(std::make_unique<class Mem>(Trunk));
            else if(*Str=="Device")
                this->Device.push_back(std::make_unique<class Mem>(Trunk));
            else
                throw std::invalid_argument("Memory type is malformed.");

            if(XML_Child(Node,"",&Trunk)<0)
                throw std::invalid_argument("Memory section parsing internal error.");
        }

        /* Thread */
        if((XML_Child(Node,"Thread",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Thread section is missing.");
        if(XML_Child(Temp,0,&Trunk)<0)
            throw std::invalid_argument("Thread section parsing internal error.");
        if(Trunk==0)
            throw std::invalid_argument("Thread section is empty.");
        while(Trunk!=0)
        {
            this->Thd.push_back(std::make_unique<class Thd>(Trunk));

            if(XML_Child(Node,"",&Trunk)<0)
                throw std::invalid_argument("Thread section parsing internal error.");
        }

        /* Invocation */
        if((XML_Child(Node,"Invocation",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Invocation section is missing.");
        if(XML_Child(Temp,0,&Trunk)<0)
            throw std::invalid_argument("Invocation section parsing internal error.");
        while(Trunk!=0)
        {
            this->Inv.push_back(std::make_unique<class Inv>(Trunk));

            if(XML_Child(Temp,"",&Trunk)<0)
                throw std::invalid_argument("Invocation section parsing internal error.");
        }

        /* Port */
        if((XML_Child(Node,"Port",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Port section is missing.");
        if(XML_Child(Temp,0,&Trunk)<0)
            throw std::invalid_argument("Port section parsing internal error.");
        while(Trunk!=0)
        {
            this->Port.push_back(std::make_unique<class Port>(Trunk));

            if(XML_Child(Temp,"",&Trunk)<0)
                throw std::invalid_argument("Port section parsing internal error.");
        }

        /* Receive */
        if((XML_Child(Node,"Receive",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Receive section is missing.");
        if(XML_Child(Temp,0,&Trunk)<0)
            throw std::invalid_argument("Receive section parsing internal error.");
        while(Trunk!=0)
        {
            this->Recv.push_back(std::make_unique<class Recv>(Trunk));

            if(XML_Child(Temp,"",&Trunk)<0)
                throw std::invalid_argument("Receive section parsing internal error.");
        }

        /* Send */
        if((XML_Child(Node,"Send",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Send section is missing.");
        if(XML_Child(Temp,0,&Trunk)<0)
            throw std::invalid_argument("Send section parsing internal error.");
        while(Trunk!=0)
        {
            this->Send.push_back(std::make_unique<class Send>(Trunk));

            if(XML_Child(Temp,"",&Trunk)<0)
                throw std::invalid_argument("Send section parsing internal error.");
        }

        /* Vector */
        if((XML_Child(Node,"Vector",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Vector section missing.");
        if(XML_Child(Temp,0,&Trunk)<0)
            throw std::invalid_argument("Vector section parsing internal error.");
        while(Trunk!=0)
        {
            this->Vect.push_back(std::make_unique<class Vect>(Trunk));

            if(XML_Child(Temp,"",&Trunk)<0)
                throw std::invalid_argument("Vector section parsing internal error.");
        }
    }
    catch(std::exception& Exc)
    {
        if(this->Name!=nullptr)
            throw std::runtime_error(std::string("Process: ")+*(this->Name)+"\n"+Exc.what());
        else
            throw std::runtime_error(std::string("Process: ")+"Unknown"+"\n"+Exc.what());
    }
}
/* End Function:Proc::Proc ***************************************************/
}
/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
