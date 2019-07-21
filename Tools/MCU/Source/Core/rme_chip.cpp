/******************************************************************************
Filename    : rme_chip.cpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The chip class.
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
#include "Core/rme_mem.hpp"
#include "Core/rme_raw.hpp"
#include "Core/rme_kobj.hpp"
#include "Core/rme_vect.hpp"
#include "Core/rme_chip.hpp"
#undef __HDR_DEFS__

#define __HDR_CLASSES__
#include "Core/rme_mem.hpp"
#include "Core/rme_raw.hpp"
#include "Core/rme_kobj.hpp"
#include "Core/rme_vect.hpp"
#include "Core/rme_chip.hpp"
#undef __HDR_CLASSES__
/* End Includes **************************************************************/
namespace rme_mcu
{
/* Begin Function:Option::Option **********************************************
Description : Parse the option section of a particular chip.
Input       : xml_node_t* Node - The option section's XML node.
Output      : None.
Return      : None.
******************************************************************************/
/* void */ Option::Option(xml_node_t* Node)
{
    xml_node_t* Temp;
    ptr_t Start;
    ptr_t End;
    std::unique_ptr<std::string> Str;

    try
    {
        /* Name */
        if((XML_Child(Node,"Name",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Name section is missing.");
        if(Temp->XML_Val_Len==0)
            throw std::invalid_argument("Name section is empty.");
        this->Name=std::make_unique<std::string>(Temp->XML_Val,(int)Temp->XML_Val_Len);

        /* Type */
        if((XML_Child(Node,"Type",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Type section is missing.");
        if(Temp->XML_Val_Len==0)
            throw std::invalid_argument("Type section is empty.");
        Str=std::make_unique<std::string>(Temp->XML_Val,(int)Temp->XML_Val_Len);

        if(*Str=="Range")
            this->Type=OPTION_RANGE;
        else if(*Str=="Select")
            this->Type=OPTION_SELECT;
        else
            throw std::invalid_argument("Type is malformed.");

        /* Macro */
        if((XML_Child(Node,"Macro",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Macro section is missing.");
        if(Temp->XML_Val_Len==0)
            throw std::invalid_argument("Macro section is empty.");
        this->Macro=std::make_unique<std::string>(Temp->XML_Val,(int)Temp->XML_Val_Len);

        /* Range */
        if((XML_Child(Node,"Range",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Range section is missing.");
        if(Temp->XML_Val_Len==0)
            throw std::invalid_argument("Range section is empty.");
        Start=0;
        while(Start<Temp->XML_Val_Len)
        {
            End=Start;
            while(End<Temp->XML_Val_Len)
            {
                if(Temp->XML_Val[End]==',')
                    break;
                End++;
            }
            if(End==Start)
                throw std::invalid_argument("Range section have an empty value.");
            this->Range.push_back(std::make_unique<std::string>(&Temp->XML_Val[Start],(int)(End-Start)));
            Start=End+1;
        }
        if(this->Type==OPTION_RANGE)
        {
            if(this->Range.size()!=2)
                throw std::invalid_argument("Range typed option cannot have more or less than two ends specified.");
        }
    }
    catch(std::exception& Exc)
    {
        if(this->Name!=nullptr)
            throw std::runtime_error(std::string("Option: ")+*(this->Name)+"\n"+Exc.what());
        else
            throw std::runtime_error(std::string("Option: ")+"Unknown"+"\n"+Exc.what());
    }
}
/* End Function:Option::Option ***********************************************/

/* Begin Function:Chip::Chip **************************************************
Description : Constructor for Chip class.
Input       : xml_node_t* Node - The node containing the chip information.
Output      : None.
Return      : None.
******************************************************************************/
/* void */ Chip::Chip(xml_node_t* Node)
{
    xml_node_t* Temp;
    xml_node_t* Trunk;
    xml_node_t* Mem_Type;
    std::unique_ptr<std::string> Str;

    try
    {
        /* Class */
        if((XML_Child(Node,"Class",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Class section is missing.");
        if(Temp->XML_Val_Len==0)
            throw std::invalid_argument("Class section is empty.");
        this->Chip_Class=std::make_unique<std::string>(Temp->XML_Val,(int)Temp->XML_Val_Len);

        /* Compatible */
        if((XML_Child(Node,"Platform",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Compatible variant section is missing.");
        if(Temp->XML_Val_Len==0)
            throw std::invalid_argument("Compatible variant section is empty.");
        this->Chip_Compat=std::make_unique<std::string>(Temp->XML_Val,(int)Temp->XML_Val_Len);

        /* Vendor */
        if((XML_Child(Node,"Vendor",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Vendor section is missing.");
        if(Temp->XML_Val_Len==0)
            throw std::invalid_argument("Vendor section is empty.");
        this->Vendor=std::make_unique<std::string>(Temp->XML_Val,(int)Temp->XML_Val_Len);

        /* Platform */
        if((XML_Child(Node,"Platform",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Platform section is missing.");
        if(Temp->XML_Val_Len==0)
            throw std::invalid_argument("Platform section is empty.");
        this->Plat=std::make_unique<std::string>(Temp->XML_Val,(int)Temp->XML_Val_Len);

        /* Cores */
        if((XML_Child(Node,"Cores",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Cores section is missing.");
        if(XML_Get_Uint(Temp,&(this->Cores))<0)
            throw std::invalid_argument("Cores is not an unsigned integer.");

        /* Regions */
        if((XML_Child(Node,"Regions",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Regions section is missing.");
        if(XML_Get_Uint(Temp,&(this->Regions))<0)
            throw std::invalid_argument("Regions is not an unsigned integer.");
    
        /* Attribute */
        if((XML_Child(Node,"Attribute",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Attribute section missing.");

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

        if(this->Code.size()==0)
            throw std::invalid_argument("No code section exists.");
        if(this->Data.size()==0)
            throw std::invalid_argument("No data section exists.");
        if(this->Device.size()==0)
            throw std::invalid_argument("No device section exists.");

        /* Option */
        if((XML_Child(Node,"Option",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Option section is missing.");
        if(XML_Child(Temp,0,&Trunk)<0)
            throw std::invalid_argument("Option section parsing internal error.");
        while(Trunk!=0)
        {
            this->Option.push_back(std::make_unique<class Option>(Trunk));

            if(XML_Child(Node,"",&Trunk)<0)
                throw std::invalid_argument("Option section parsing internal error.");
        }

        /* Vector */
        if((XML_Child(Node,"Vector",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Vector section missing.");
        if(XML_Child(Temp,0,&Trunk)<0)
            throw std::invalid_argument("Vector section parsing internal error.");
        while(Trunk!=0)
        {
            this->Vect.push_back(std::make_unique<class Vect>(Trunk));
            
            if(XML_Child(Node,"",&Trunk)<0)
                throw std::invalid_argument("Vector section parsing internal error.");
        }
    }
    catch(std::exception& Exc)
    {
        throw std::runtime_error(std::string("Chip: ")+"\n"+Exc.what());
    }
}
/* End Function:Chip::Chip ***************************************************/
}
/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
