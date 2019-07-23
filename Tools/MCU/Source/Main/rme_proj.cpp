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
#include "Main/rme_mcu.hpp"
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
#undef __HDR_DEFS__

#define __HDR_CLASSES__
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
/* Begin Function:Plat::Plat **************************************************
Description : Constructor for Plat class.
Input       : ptr_t Word_Bits - The number of bits in a processor word.
              ptr_t Init_Num_Ord - The initial number order of the page table.
              ptr_t Thd_Size - The raw thread size.
              ptr_t Inv_Size - The raw invocation size.
Output      : None.
Return      : None.
******************************************************************************/
/* void */ Plat::Plat(ptr_t Word_Bits, ptr_t Init_Num_Ord, ptr_t Thd_Size, ptr_t Inv_Size)
{
    this->Word_Bits=Word_Bits;
    this->Capacity=POW2(Word_Bits/4-1);
    this->Init_Num_Ord=Init_Num_Ord;
    this->Raw_Thd_Size=Thd_Size;
    this->Raw_Inv_Size=Inv_Size;
}
/* End Function:Plat::Plat ***************************************************/

/* Begin Function:Plat::Captbl_Size *******************************************
Description : Calculate the platform's capability table size.
Input       : ptr_t Entry - The number of entries for the capability table.
Output      : None.
Return      : ptr_t - The size of the capability table, in bytes.
******************************************************************************/
ptr_t Plat::Captbl_Size(ptr_t Entry)
{
    return ROUND_UP(RAW_CAPTBL_SIZE(this->Word_Bits,Entry), this->Kmem_Order);
}
/* End Function:Plat::Captbl_Size ********************************************/

/* Begin Function:Plat::Captbl_Num ********************************************
Description : Calculate the number of capability tables needed given the number
              of capabilities.
Input       : ptr_t Entry - The total number of entries for the capability tables.
Output      : None.
Return      : ptr_t - The number of capability tables needed.
******************************************************************************/
ptr_t Plat::Captbl_Num(ptr_t Entry)
{
    return (Entry+this->Capacity-1)/this->Capacity;
}
/* End Function:Plat::Captbl_Num *********************************************/

/* Begin Function:Plat::Captbl_Total ******************************************
Description : Calculate the total size of capability tables needed given the number
              of capabilities.
Input       : ptr_t Entry - The number of entries for the capability tables.
Output      : None.
Return      : ptr_t - The total size size of the capability tables, in bytes.
******************************************************************************/
ptr_t Plat::Captbl_Total(ptr_t Entry)
{
    ptr_t Size;

    Size=ROUND_UP(RAW_CAPTBL_SIZE(this->Word_Bits,this->Capacity), this->Kmem_Order);
    Size*=Entry/this->Capacity;
    Size+=ROUND_UP(RAW_CAPTBL_SIZE(this->Word_Bits,Entry%this->Capacity), this->Kmem_Order);

    return Size;
}
/* End Function:Plat::Captbl_Total *******************************************/

/* Begin Function:Plat::Pgtbl_Size ********************************************
Description : Calculate the platform's page table size.
Input       : ptr_t Num_Order - The number order.
              ptr_t Is_Top - Whether this is a top-level.
Output      : None.
Return      : ptr_t - The size of the page table, in bytes.
******************************************************************************/
ptr_t Plat::Pgtbl_Size(ptr_t Num_Order, ptr_t Is_Top)
{
    return ROUND_UP(this->Raw_Pgtbl_Size(Num_Order, Is_Top), this->Kmem_Order);
}
/* End Function:Plat::Pgtbl_Size *********************************************/

/* Begin Function:Plat::Proc_Size *********************************************
Description : Calculate the platform's process size.
Input       : None.
Output      : None.
Return      : ptr_t - The size of the process, in bytes.
******************************************************************************/
ptr_t Plat::Proc_Size(void)
{
    return ROUND_UP(RAW_PROC_SIZE(this->Word_Bits), this->Kmem_Order);
}
/* End Function:Plat::Proc_Size **********************************************/

/* Begin Function:Plat::Thd_Size **********************************************
Description : Calculate the platform's thread size.
Input       : None.
Output      : None.
Return      : ptr_t - The size of the thread, in bytes.
******************************************************************************/
ptr_t Plat::Thd_Size(void)
{
    return ROUND_UP(this->Raw_Thd_Size, this->Kmem_Order);
}
/* End Function:Plat::Thd_Size ***********************************************/

/* Begin Function:Plat::Inv_Size **********************************************
Description : Calculate the platform's invocation size.
Input       : None.
Output      : None.
Return      : ptr_t - The size of the thread, in bytes.
******************************************************************************/
ptr_t Plat::Inv_Size(void)
{
    return ROUND_UP(this->Raw_Inv_Size, this->Kmem_Order);
}
/* End Function:Plat::Inv_Size ***********************************************/

/* Begin Function:Plat::Sig_Size **********************************************
Description : Calculate the platform's signal endpoint size.
Input       : None.
Output      : None.
Return      : ptr_t - The size of the thread, in bytes.
******************************************************************************/
ptr_t Plat::Sig_Size(void)
{
    return ROUND_UP(RAW_SIG_SIZE(this->Word_Bits), this->Kmem_Order);
}
/* End Function:Plat::Sig_Size ***********************************************/

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
        Kobj::To_Lower(this->Plat_Lower);

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

/* Begin Function:Proj::Kobj_Alloc ********************************************
Description : Get the size of the kernel memory, and generate the initial states
              for kernel object creation.
Input       : std::unique_ptr<class Plat>& Plat - The platform structure.
              ptr_t Init_Captbl_Size - The initial capability table's size;
Output      : None.
Return      : None.
******************************************************************************/
void Proj::Kobj_Alloc(std::unique_ptr<class Plat>& Plat, ptr_t Init_Capsz)
{
    ptr_t Cap_Front;
    ptr_t Kmem_Front;

    /* Compute initial state when creating the vectors */
    Cap_Front=0;
    Kmem_Front=0;
    /* Initial capability table */
    Cap_Front++;
    Kmem_Front+=Plat->Captbl_Size(Init_Capsz);
    /* Initial page table */
    Cap_Front++;
    Kmem_Front+=Plat->Pgtbl_Size(Plat->Init_Num_Ord,1);
    /* Initial RVM process */
    Cap_Front++;
    Kmem_Front+=Plat->Proc_Size();
    /* Initial kcap and kmem */
    Cap_Front+=2;
    /* Initial tick timer/interrupt endpoint */
    Cap_Front+=2;
    Kmem_Front+=2*Plat->Sig_Size();
    /* Initial thread */
    Cap_Front++;
    Kmem_Front+=Plat->Thd_Size();

    /* Create vectors */
    this->RME->Map->Vect_Cap_Front=Cap_Front;
    this->RME->Map->Vect_Kmem_Front=Kmem_Front;
    /* Capability tables for containing vector endpoints */
    Cap_Front+=Plat->Captbl_Num(this->RVM->Vect.size());
    Kmem_Front+=Plat->Captbl_Total(this->RVM->Vect.size());
    /* Vector endpoint themselves */
    Kmem_Front+=this->RVM->Vect.size()*Plat->Sig_Size();

    /* Create RVM */
    this->RVM->Map->Before_Cap_Front=Cap_Front;
    this->RVM->Map->Before_Kmem_Front=Kmem_Front;
    /* Three threads for RVM - now only one will be started */
    Cap_Front+=3;
    Kmem_Front+=3*Plat->Thd_Size();

    /* Create capability tables */
    this->RVM->Map->Captbl_Cap_Front=Cap_Front;
    this->RVM->Map->Captbl_Kmem_Front=Kmem_Front;
    /* Capability tables for containing capability tables */
    Cap_Front+=Plat->Captbl_Num(this->RVM->Captbl.size());
    Kmem_Front+=Plat->Captbl_Total(this->RVM->Captbl.size());
    /* Capability tables themselves */
    for(std::unique_ptr<class Cap> const& Info:this->RVM->Captbl)
        Kmem_Front+=Plat->Captbl_Size(static_cast<class Captbl*>(Info->Kobj)->Size);

    /* Create page tables */
    this->RVM->Map->Pgtbl_Cap_Front=Cap_Front;
    this->RVM->Map->Pgtbl_Kmem_Front=Kmem_Front;
    /* Capability tables for containing page tables */
    Cap_Front+=Plat->Captbl_Num(this->RVM->Pgtbl.size());
    Kmem_Front+=Plat->Captbl_Total(this->RVM->Pgtbl.size());
    /* Page table themselves */
    for(std::unique_ptr<class Cap> const& Info:this->RVM->Pgtbl)
    {
        Kmem_Front+=Plat->Pgtbl_Size(static_cast<class Pgtbl*>(Info->Kobj)->Num_Order,
                                     static_cast<class Pgtbl*>(Info->Kobj)->Is_Top);
    }

    /* Create processes */
    this->RVM->Map->Proc_Cap_Front=Cap_Front;
    this->RVM->Map->Proc_Kmem_Front=Kmem_Front;
    /* Capability tables for containing processes */
    Cap_Front+=Plat->Captbl_Num(this->RVM->Proc.size());
    Kmem_Front+=Plat->Captbl_Total(this->RVM->Proc.size());
    /* Processes themselves */
    Kmem_Front+=this->RVM->Proc.size()*Plat->Proc_Size();

    /* Create threads */
    this->RVM->Map->Thd_Cap_Front=Cap_Front;
    this->RVM->Map->Thd_Kmem_Front=Kmem_Front;
    /* Capability tables for containing threads */
    Cap_Front+=Plat->Captbl_Num(this->RVM->Thd.size());
    Kmem_Front+=Plat->Captbl_Total(this->RVM->Thd.size());
    /* Threads themselves */
    Kmem_Front+=this->RVM->Thd.size()*Plat->Thd_Size();


    /* Create invocations */
    this->RVM->Map->Inv_Cap_Front=Cap_Front;
    this->RVM->Map->Inv_Kmem_Front=Kmem_Front;
    /* Capability tables for containing invocations */
    Cap_Front+=Plat->Captbl_Num(this->RVM->Inv.size());
    Kmem_Front+=Plat->Captbl_Total(this->RVM->Inv.size());
    /* Invocations themselves */
    Kmem_Front+=this->RVM->Inv.size()*Plat->Inv_Size();


    /* Create receive endpoints */
    this->RVM->Map->Recv_Cap_Front=Cap_Front;
    this->RVM->Map->Recv_Kmem_Front=Kmem_Front;
    /* Capability tables for containing receive endpoints */
    Cap_Front+=Plat->Captbl_Num(this->RVM->Recv.size());
    Kmem_Front+=Plat->Captbl_Total(this->RVM->Recv.size());
    /* Receive endpoints themselves */
    Kmem_Front+=this->RVM->Recv.size()*Plat->Sig_Size();

    /* Final pointer positions */
    this->RVM->Map->After_Cap_Front=Cap_Front;
    this->RVM->Map->After_Kmem_Front=Kmem_Front;

    /* Check if we are out of table space */
    this->RVM->Map->Captbl_Size=this->RVM->Extra_Captbl+this->RVM->Map->After_Cap_Front;
    if(this->RVM->Map->After_Cap_Front>Plat->Capacity)
        throw std::runtime_error("RVM capability size too big.");
}
/* End Function:Get_Kmem_Size ************************************************/
}
/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
