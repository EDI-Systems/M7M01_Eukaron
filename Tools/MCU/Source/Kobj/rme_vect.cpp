/******************************************************************************
Filename    : rme_vect.cpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The vector endpoint class.
******************************************************************************/

/* Includes ******************************************************************/
#include "list"
#include "string"
#include "memory"
#include "vector"
#include "stdexcept"

extern "C"
{
#include "xml.h"
#include "pbfs.h"
}

#define __HDR_DEFS__
#include "Main/rme_mcu.hpp"

#include "Kobj/rme_kobj.hpp"
#include "Kobj/rme_vect.hpp"
#undef __HDR_DEFS__

#define __HDR_CLASSES__
#include "Main/rme_fsys.hpp"
#include "Main/rme_chip.hpp"
#include "Main/rme_comp.hpp"
#include "Main/rme_raw.hpp"
#include "Main/rme_mem.hpp"

#include "Kobj/rme_kobj.hpp"
#include "Kobj/rme_captbl.hpp"
#include "Kobj/rme_pgtbl.hpp"
#include "Kobj/rme_thd.hpp"
#include "Kobj/rme_inv.hpp"
#include "Kobj/rme_port.hpp"
#include "Kobj/rme_recv.hpp"
#include "Kobj/rme_send.hpp"
#include "Kobj/rme_vect.hpp"
#include "Kobj/rme_proc.hpp"

#include "Main/rme_proj.hpp"
#undef __HDR_CLASSES__
/* End Includes **************************************************************/
namespace rme_mcu
{
/* Begin Function:Vect::Vect **************************************************
Description : Constructor for Vect class.
Input       : xml_node_t* Node - The node containing the receive endpoint.
Output      : None.
Return      : None.
******************************************************************************/
/* void */ Vect::Vect(xml_node_t* Node)
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

        /* Number */
        if((XML_Child(Node,"Number",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Number section is missing.");
        if(Temp->XML_Val_Len==0)
            throw std::invalid_argument("Number section is empty.");
        if(XML_Get_Uint(Temp,&(this->Num))<0)
            throw std::invalid_argument("Number is not a valid unsigned integer.");
    }
    catch(std::exception& Exc)
    {
        if(this->Name!=nullptr)
            throw std::runtime_error(std::string("Vector: ")+*(this->Name)+"\n"+Exc.what());
        else
            throw std::runtime_error(std::string("Vector: ")+"Unknown"+"\n"+Exc.what());
    }
}
/* End Function:Vect::Vect ***************************************************/

/* Begin Function:Vect::Check_Vect ********************************************
Description : Detect vector conflicts in the system. Every vector name is globally
              unique - it cannot overlap with any other vector in any other process.
              If one of the processes have a vector endpoint, then no other process
              can have the same one.
Input       : std::unique_ptr<class Proj>& Proj - The project information struct.
Output      : None.
Return      : std::string* - The name that caused the error.
******************************************************************************/
std::string* Vect::Check_Vect(std::unique_ptr<class Proj>& Proj)
{
    for(std::unique_ptr<class Proc>& Proc:Proj->Proc)
    {
        for(std::unique_ptr<class Vect>& Vect:Proc->Vect)
        {
            for(std::unique_ptr<class Proc>& Proc_Temp:Proj->Proc)
            {
                for(std::unique_ptr<class Vect>& Vect_Temp:Proc_Temp->Vect)
                {
                    if(&Vect_Temp==&Vect)
                        continue;
            
                    if(Strcicmp(*(Vect->Name), *(Vect_Temp->Name))==0)
                        return Vect->Name.get();
                }
            }
        }
    }

    return nullptr;
}
/* End Function:Vect::Check_Vect *********************************************/
}
/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
