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
#include "algorithm"
#include "stdexcept"

extern "C"
{
#include "xml.h"
}

#define __HDR_DEFS__
#include "Core/rme_mcu.hpp"
#include "Core/rme_comp.hpp"
#include "Core/rme_raw.hpp"
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
#include "Core/rme_proj.hpp"
#undef __HDR_DEFS__

#define __HDR_CLASSES__
#include "Core/rme_comp.hpp"
#include "Core/rme_raw.hpp"
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
#include "Core/rme_proj.hpp"
#undef __HDR_CLASSES__
/* End Includes **************************************************************/
namespace rme_mcu
{
/* Begin Function:RME::RME ****************************************************
Description : Constructor for RME class.
Input       : xml_node_t* Node - The node containing the whole project.
Output      : None.
Return      : None.
******************************************************************************/
/* void */ RME::RME(xml_node_t* Node)
{
    xml_node_t* Temp;
    xml_node_t* Trunk;
    
    try
    {
        /* Code start address */
        if((XML_Child(Node,"Code_Start",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Code address section is missing.");
        if(XML_Get_Hex(Temp,&(this->Code_Start))<0)
            throw std::invalid_argument("Code address is not a valid hex integer.");

        /* Code size */
        if((XML_Child(Node,"Code_Size",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Code size section is missing.");
        if(XML_Get_Hex(Temp,&(this->Code_Size))<0)
            throw std::invalid_argument("Code size is not a valid hex integer.");

        /* Data start address */
        if((XML_Child(Node,"Data_Start",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Data address section is missing.");
        if(XML_Get_Hex(Temp,&(this->Data_Start))<0)
            throw std::invalid_argument("Data address is not a valid hex integer.");

        /* Data size */
        if((XML_Child(Node,"Data_Size",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Data size section is missing.");
        if(XML_Get_Hex(Temp,&(this->Data_Size))<0)
            throw std::invalid_argument("Data size is not a valid hex integer.");

        /* Stack size */
        if((XML_Child(Node,"Stack_Size",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Stack size section is missing.");
        if(XML_Get_Hex(Temp,&(this->Stack_Size))<0)
            throw std::invalid_argument("Stack size is not a valid hex integer.");

        /* Extra kernel memory */
        if((XML_Child(Node,"Extra_Kmem",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Extra kernel memory section is missing.");
        if(XML_Get_Hex(Temp,&(this->Extra_Kmem))<0)
            throw std::invalid_argument("Extra kernel memory is not a valid hex integer.");

        /* Kmem_Order */
        if((XML_Child(Node,"Kmem_Order",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Kernel memory order section is missing.");
        if(XML_Get_Uint(Temp,&(this->Kmem_Order))<0)
            throw std::invalid_argument("Kernel memory order is not a valid unsigned integer.");

        /* Priorities */
        if((XML_Child(Node,"Kern_Prios",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Priority number section is missing.");
        if(XML_Get_Uint(Temp,&(this->Kern_Prios))<0)
            throw std::invalid_argument("Priority number is not a valid unsigned integer.");

        /* Compiler */
        if((XML_Child(Node,"Compiler",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Complier option section is missing.");
        this->Comp=std::make_unique<class Comp>(Temp);

        /* Platform */
        if((XML_Child(Node,"Platform",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Platform option section is missing.");
        if(XML_Child(Temp,0,&Trunk)<0)
            throw std::invalid_argument("Platform option section parsing internal error.");
        while(Trunk!=0)
        {
            this->Plat.push_back(std::make_unique<class Raw>(Trunk));

            if(XML_Child(Temp,"",&Trunk)<0)
                throw std::invalid_argument("Platform option section parsing internal error.");
        }

        /* Chip */
        if((XML_Child(Node,"Chip",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Chip option section is missing.");
        if(XML_Child(Temp,0,&Trunk)<0)
            throw std::invalid_argument("Chip option section parsing internal error.");
        while(Trunk!=0)
        {
            this->Chip.push_back(std::make_unique<class Raw>(Trunk));

            if(XML_Child(Temp,"",&Trunk)<0)
                throw std::invalid_argument("Chip option section parsing internal error.");
        }
    }
    catch(std::exception& Exc)
    {
        throw std::runtime_error(std::string("RME:\n")+Exc.what());
    }
}
/* End Function:RME::RME *****************************************************/

/* Begin Function:RVM::RVM ****************************************************
Description : Constructor for RVM class.
Input       : xml_node_t* Node - The node containing the whole project.
Output      : None.
Return      : None.
******************************************************************************/
/* void */ RVM::RVM(xml_node_t* Node)
{
    xml_node_t* Temp;
    std::unique_ptr<std::string> Str;
    
    try
    {
        /* Code size */
        if((XML_Child(Node,"Code_Size",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Code size section is missing.");
        if(XML_Get_Hex(Temp,&(this->Code_Size))<0)
            throw std::invalid_argument("Code size is not a valid hex integer.");

        /* Data size */
        if((XML_Child(Node,"Data_Size",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Data size section is missing.");
        if(XML_Get_Hex(Temp,&(this->Data_Size))<0)
            throw std::invalid_argument("Data size is not a valid hex integer.");

        /* Stack size */
        if((XML_Child(Node,"Stack_Size",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Stack size section is missing.");
        if(XML_Get_Hex(Temp,&(this->Stack_Size))<0)
            throw std::invalid_argument("Stack size is not a valid hex integer.");

        /* Extra Captbl */
        if((XML_Child(Node,"Extra_Captbl",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Extra capability table size section is missing.");
        if(XML_Get_Uint(Temp,&(this->Extra_Captbl))<0)
            throw std::invalid_argument("Extra capability table size is not a valid unsigned integer.");

        /* Recovery */
        if((XML_Child(Node,"Recovery",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Recovery method section is missing.");
        if(Temp->XML_Val_Len==0)
            throw std::invalid_argument("Recovery method section is empty.");
        Str=std::make_unique<std::string>(Temp->XML_Val,(int)Temp->XML_Val_Len);

        if(*Str=="Thread")
            this->Recovery=RECOVERY_THD;
        else if(*Str=="Process")
            this->Recovery=RECOVERY_PROC;
        else if(*Str=="System")
            this->Recovery=RECOVERY_SYS;
        else
            throw std::invalid_argument("Recovery method is malformed.");

        /* Compiler */
        if((XML_Child(Node,"Compiler",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Complier section is missing.");
        this->Comp=std::make_unique<class Comp>(Temp);

        /* VMM section is not read currently */
    }
    catch(std::exception& Exc)
    {
        throw std::runtime_error(std::string("RVM:\n")+Exc.what());
    }
}
/* End Function:RVM::RVM *****************************************************/

/* Begin Function:Proj::Proj **************************************************
Description : Constructor for Proj class.
Input       : xml_node_t* Node - The node containing the whole project.
Output      : None.
Return      : None.
******************************************************************************/
/* void */ Proj::Proj(xml_node_t* Node)
{
    xml_node_t* Temp;
    xml_node_t* Trunk;

    try
    {
        /* Parse the XML content */
        if((Node->XML_Tag_Len!=7)||(strncmp(Node->XML_Tag,"Project",7)!=0))
            throw std::invalid_argument("Project XML is malformed.");

        /* Name */
        if((XML_Child(Node,"Name",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Name section is missing.");
        if(Temp->XML_Val_Len==0)
            throw std::invalid_argument("Name section is empty.");
        this->Name=std::make_unique<std::string>(Temp->XML_Val,(int)Temp->XML_Val_Len);

        /* Platform */
        if((XML_Child(Node,"Platform",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Platform section is missing.");
        if(Temp->XML_Val_Len==0)
            throw std::invalid_argument("Platform section is empty.");
        this->Plat_Name=std::make_unique<std::string>(Temp->XML_Val,(int)Temp->XML_Val_Len);
        this->Plat_Lower=std::make_unique<std::string>(Temp->XML_Val,(int)Temp->XML_Val_Len);
        std::transform(this->Plat_Lower->begin(), this->Plat_Lower->end(), this->Plat_Lower->begin(), std::tolower);

        /* Chip_Class */
        if((XML_Child(Node,"Chip_Class",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Chip class section is missing.");
        if(Temp->XML_Val_Len==0)
            throw std::invalid_argument("Chip class section is empty.");
        this->Chip_Class=std::make_unique<std::string>(Temp->XML_Val,(int)Temp->XML_Val_Len);

        /* Chip_Full */
        if((XML_Child(Node,"Chip_Full",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Chip fullname section is missing.");
        if(Temp->XML_Val_Len==0)
            throw std::invalid_argument("Chip fullname section is empty.");
        this->Chip_Full=std::make_unique<std::string>(Temp->XML_Val,(int)Temp->XML_Val_Len);

        /* RME */
        if((XML_Child(Node,"RME",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("RME section is missing.");
        this->RME=std::make_unique<class RME>(Temp);

        /* RVM */
        if((XML_Child(Node,"RVM",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("RVM section is missing.");
        this->RVM=std::make_unique<class RVM>(Temp);

        /* Process */
        if((XML_Child(Node,"Process",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Process section is missing.");
        if(XML_Child(Temp,0,&Trunk)<0)
            throw std::invalid_argument("Process section parsing internal error.");
        while(Trunk!=0)
        {
            this->Proc.push_back(std::make_unique<class Proc>(Trunk));
            
            if(XML_Child(Temp,"",&Trunk)<0)
                throw std::invalid_argument("Process section parsing internal error.");
        }
    }
    catch(std::exception& Exc)
    {
        if(this->Name!=nullptr)
            throw std::runtime_error(std::string("Project: ")+*(this->Name)+"\n"+Exc.what());
        else
            throw std::runtime_error(std::string("Project: ")+"Unknown"+"\n"+Exc.what());
    }
}
/* End Function:Proj::Proj ***************************************************/
}
/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
