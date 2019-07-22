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
#include "Core/rme_raw.hpp"
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
#include "Core/rme_raw.hpp"
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

        this->Map=std::make_unique<class RME_Memmap>();
    }
    catch(std::exception& Exc)
    {
        throw std::runtime_error(std::string("RME:\n")+Exc.what());
    }
}
/* End Function:RME::RME *****************************************************/

/* Begin Function:RME::Alloc_Kmem *********************************************
Description : Allocate the kernel objects and memory for RME itself.
Input       : ptr_t Kmem_Front - The current kernel memory frontier.
              ptr_t Kmem_Order - The kernel memory order.
Output      : None.
Return      : None.
******************************************************************************/
void RME::Alloc_Kmem(ptr_t Kmem_Front, ptr_t Kmem_Order)
{
    /* Code section */
    this->Map->Code_Base=this->Code_Start;
    this->Map->Code_Size=this->Code_Size;

    /* Data section */
    this->Map->Data_Base=this->Data_Start;
    this->Map->Data_Size=this->Data_Size;

    /* Interrupt flag section - cut out from the data section */
    this->Map->Intf_Base=this->Map->Data_Base+this->Map->Data_Size-KERNEL_INTF_SIZE;
    this->Map->Intf_Size=KERNEL_INTF_SIZE;
    if(this->Map->Intf_Base<=this->Map->Data_Base)
        throw std::runtime_error("RME data section is not big enough, unable to allocate interrupt flags.");
    this->Map->Data_Size=this->Map->Intf_Base-this->Map->Data_Base;

    /* Stack section - cut out from the data section */
    this->Map->Stack_Base=this->Map->Data_Base+this->Map->Data_Size-this->Map->Stack_Size;
    this->Map->Stack_Size=this->Map->Stack_Size;
    if(this->Map->Stack_Base<=this->Map->Data_Base)
        throw std::runtime_error("RME data section is not big enough, unable to allocate kernel stacks.");
    this->Map->Data_Size=this->Map->Stack_Base-this->Map->Data_Base;

    /* Kernel memory section - cut out from the data section */
    this->Map->Kmem_Base=this->Map->Data_Base+this->Map->Data_Size-Kmem_Front-this->Extra_Kmem;
    this->Map->Kmem_Base=ROUND_DOWN(this->Map->Kmem_Base,Kmem_Order);
    this->Map->Kmem_Size=Kmem_Front+this->Extra_Kmem;
    if(this->Map->Kmem_Base<=this->Map->Data_Base)
        throw std::runtime_error("RME data section is not big enough, unable to allocate kernel object memory.");
    this->Map->Data_Size=this->Map->Kmem_Base-this->Map->Data_Base;
}
/* End Function:RME::Alloc_Kmem **********************************************/

/* Begin Function:Cap::Cap ****************************************************
Description : Constructor for Cap class.
Input       : class Proc* Proc - The pointer to the process.
              class Kobj* Kobj - The pointer to the kernel object.
Output      : None.
Return      : None.
******************************************************************************/
/* void */ Cap::Cap(class Proc* Proc, class Kobj* Kobj)
{
    this->Proc=Proc;
    this->Kobj=Kobj;
}
/* End Function:Cap::Cap *****************************************************/

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
        
        this->Map=std::make_unique<class RVM_Memmap>();
    }
    catch(std::exception& Exc)
    {
        throw std::runtime_error(std::string("RVM:\n")+Exc.what());
    }
}
/* End Function:RVM::RVM *****************************************************/

/* Begin Function:RVM::Alloc_Mem **********************************************
Description : Allocate the kernel objects and memory for RVM user-level library.
Input       : ptr_t Code_Start - The code start position for RVM.
              ptr_t Data_Start - The data start position for RVM.
Output      : None.
Return      : None.
******************************************************************************/
void RVM::Alloc_Mem(ptr_t Code_Start, ptr_t Data_Start)
{
    /* Code section */
    this->Map->Code_Base=Code_Start;
    this->Map->Code_Size=this->Code_Size;

    /* Data section */
    this->Map->Data_Base=Data_Start;
    this->Map->Data_Size=this->Data_Size;

    /* Guard stack section - cut out from the data section */
    this->Map->Guard_Stack_Base=this->Map->Data_Base+this->Map->Data_Size-this->Stack_Size;
    this->Map->Guard_Stack_Size=this->Stack_Size;
    if(this->Map->Guard_Stack_Base<=this->Map->Data_Base)
        throw std::runtime_error("RVM data section is not big enough, unable to allocate guard thread stack.");
    this->Map->Data_Size=this->Map->Guard_Stack_Base-this->Map->Data_Base;

    /* VMM stack section - cut out from the data section */
    this->Map->VMM_Stack_Base=this->Map->Data_Base+this->Map->Data_Size-this->Stack_Size;
    this->Map->VMM_Stack_Size=this->Stack_Size;
    if(this->Map->VMM_Stack_Base<=this->Map->Data_Base)
        throw std::runtime_error("RVM data section is not big enough, unable to allocate monitor thread stack.");
    this->Map->Data_Size=this->Map->VMM_Stack_Base-this->Map->Data_Base;
    
    /* Interrupt stack section - cut out from the data section */
    this->Map->Intd_Stack_Base=this->Map->Data_Base+this->Map->Data_Size-this->Stack_Size;
    this->Map->Intd_Stack_Size=this->Stack_Size;
    if(this->Map->Intd_Stack_Base<=this->Map->Data_Base)
        throw std::runtime_error("RVM data section is not big enough, unable to allocate interrupt handling thread stack.");
    this->Map->Data_Size=this->Map->Intd_Stack_Base-this->Map->Data_Base;
}
/* End Function:RVM::Alloc_Mem ***********************************************/

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

        /* Every process must have at least one code and data segment, and they must be static. 
         * The primary code segment allow RXS, the primary data segment must allow RWS */
        if(this->Code.size()==0)
            throw std::invalid_argument("No code section exists.");
        if(this->Data.size()==0)
            throw std::invalid_argument("No data section exists.");

        if(((this->Code[0]->Attr)&(MEM_READ|MEM_EXECUTE|MEM_STATIC))!=(MEM_READ|MEM_EXECUTE|MEM_STATIC))
            throw std::invalid_argument("Primary code section does not have RXS attribute.");
        
        if(((this->Data[0]->Attr)&(MEM_READ|MEM_WRITE|MEM_STATIC))!=(MEM_READ|MEM_WRITE|MEM_STATIC))
            throw std::invalid_argument("Primary data section does not have RWS attribute.");

        /* All processes shall have at least one thread */
        if(this->Thd.size()==0)
            throw std::invalid_argument("No thread exists.");
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

/* Begin Function:Proc::Check_Kobj ********************************************
Description : Check kernel objects within a process.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void Proc::Check_Kobj(void)
{
    std::string* Errmsg;
    try
    {
        /* Check for duplicate threads */
        Errmsg=Kobj::Check_Kobj<class Thd>(this->Thd);
        if(Errmsg!=0)
            throw std::invalid_argument(std::string("Thread: ")+*Errmsg+"\nName is duplicate or invalid.");

        /* Check for duplicate invocations */
        Errmsg=Kobj::Check_Kobj<class Inv>(this->Inv);
        if(Errmsg!=0)
            throw std::invalid_argument(std::string("Invocation: ")+*Errmsg+"\nName is duplicate or invalid.");

        /* Check for duplicate ports */
        Errmsg=Kobj::Check_Kobj_Proc_Name<class Port>(this->Port);
        if(Errmsg!=0)
            throw std::invalid_argument(std::string("Port: ")+*Errmsg+"\nName/process name is duplicate or invalid.");

        /* Check for duplicate receive endpoints */
        Errmsg=Kobj::Check_Kobj<class Recv>(this->Recv);
        if(Errmsg!=0)
            throw std::invalid_argument(std::string("Receive endpoint: ")+*Errmsg+"\nName is duplicate or invalid.");

        /* Check for duplicate send endpoints */
        Errmsg=Kobj::Check_Kobj_Proc_Name<class Send>(this->Send);
        if(Errmsg!=0)
            throw std::invalid_argument(std::string("Send endpoint: ")+*Errmsg+"\nName/process name is duplicate or invalid.");
    }
    catch(std::exception& Exc)
    {
        throw std::runtime_error(std::string("Process: ")+*(this->Name)+"\n"+Exc.what());
    }
}
/* End Function:Proc::Check_Kobj *********************************************/

/* Begin Function:Proc::Alloc_Loc *********************************************
Description : Allocate local capability table.
Input       : ptr_t Capacity - The capacity of the capability table.
Output      : None.
Return      : None.
******************************************************************************/
void Proc::Alloc_Loc(ptr_t Capacity)
{
    ptr_t Capid=0;

    for(std::unique_ptr<class Port>& Port:this->Port)
        Port->Loc_Capid=Capid++;

    for(std::unique_ptr<class Recv>& Recv:this->Recv)
        Recv->Loc_Capid=Capid++;

    for(std::unique_ptr<class Send>& Send:this->Send)
        Send->Loc_Capid=Capid++;

    for(std::unique_ptr<class Vect>& Vect:this->Vect)
        Vect->Loc_Capid=Capid++;

    this->Captbl->Front=Capid;
    this->Captbl->Size=this->Captbl->Front+this->Captbl->Extra;

    if(this->Captbl->Size>Capacity)
        throw std::runtime_error("Process: "+*(this->Name)+"\n"+"Capability too large.");
}
/* End Function:Proc::Alloc_Loc **********************************************/

/* Begin Function:Proc::Alloc_RVM_Pgtbl ***************************************
Description : Recursively allocate page tables.
Input       : std::unique_ptr<class Proc>& Proc, std::unique_ptr<class Pgtbl>& Pgtbl.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : None.
******************************************************************************/
void Proc::Alloc_RVM_Pgtbl(std::unique_ptr<class RVM>& RVM,
                           std::unique_ptr<class Pgtbl>& Pgtbl)
{
    ptr_t Count;

    Pgtbl->RVM_Capid=RVM->Pgtbl.size();
    RVM->Pgtbl.push_back(std::make_unique<class Cap>(this,Pgtbl.get()));

    /* Recursively do allocation */
    for(Count=0;Count<Pgtbl->Pgdir.size();Count++)
    {
        if(Pgtbl->Pgdir[(unsigned int)Count]!=nullptr)
            Alloc_RVM_Pgtbl(RVM, Pgtbl->Pgdir[(unsigned int)Count]);
    }
}
/* End Function:Main::Alloc_RVM_Pgtbl ****************************************/

/* Begin Function:Proc::Alloc_Loc *********************************************
Description : Allocate (relative) global capability IDs for all kernel objects. 
              Each global object will reside in its own capability table. 
              This facilitates management, and circumvents the capability size
              limit that may present on 32-bit systems.
              How many distinct kernel objects are there? We just need to add up
              the following: All captbls (each process have one), all processes,
              all threads, all invocations, all receive endpoints. The ports and
              send endpoints do not have a distinct kernel object; the vector 
              endpoints are created by the kernel at boot-time, while the pgtbls
              are decided by architecture-specific code.
Input       : std::unique_ptr<class RVM>& RVM - The RVM struct.
Output      : None.
Return      : None.
******************************************************************************/
void Proc::Alloc_RVM(std::unique_ptr<class RVM>& RVM)
{
    this->Captbl->RVM_Capid=RVM->Captbl.size();
    RVM->Captbl.push_back(std::make_unique<class Cap>(this,this->Captbl.get()));
    Alloc_RVM_Pgtbl(RVM, this->Pgtbl);
    this->RVM_Capid=RVM->Proc.size();
    RVM->Proc.push_back(std::make_unique<class Cap>(this,this));

    for(std::unique_ptr<class Thd>& Thd:this->Thd)
    {
        Thd->RVM_Capid=RVM->Thd.size();
        RVM->Thd.push_back(std::make_unique<class Cap>(this,Thd.get()));
    }

    for(std::unique_ptr<class Inv>& Inv:this->Inv)
    {
        Inv->RVM_Capid=RVM->Inv.size();
        RVM->Inv.push_back(std::make_unique<class Cap>(this,Inv.get()));
    }

    for(std::unique_ptr<class Recv>& Recv:this->Recv)
    {
        Recv->RVM_Capid=RVM->Recv.size();
        RVM->Recv.push_back(std::make_unique<class Cap>(this,Recv.get()));
    }

    for(std::unique_ptr<class Vect>& Vect:this->Vect)
    {
        Vect->RVM_Capid=RVM->Vect.size();
        RVM->Vect.push_back(std::make_unique<class Cap>(this,Vect.get()));
    }
}
/* End Function:Proc::Alloc_Loc **********************************************/

/* Begin Function:Proc::Alloc_Macro_Pgtbl *************************************
Description : Recursively allocate page tables.
Input       : std::unique_ptr<class Proc>& Proc, std::unique_ptr<class Pgtbl>& Pgtbl.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : None.
******************************************************************************/
void Proc::Alloc_Macro_Pgtbl(std::unique_ptr<class Pgtbl>& Pgtbl)
{
    static ptr_t Serial;
    ptr_t Count;

    if(Pgtbl->Is_Top!=0)
        Serial=0;
    
    Pgtbl->RVM_Macro=std::make_unique<std::string>(std::string("RVM_PGTBL_")+*(this->Name)+
                                                   "_N"+std::to_string(Serial));
    Kobj::To_Upper(Pgtbl->RVM_Macro);

    /* Recursively do allocation */
    for(Count=0;Count<Pgtbl->Pgdir.size();Count++)
    {
        if(Pgtbl->Pgdir[(unsigned int)Count]!=nullptr)
            Alloc_Macro_Pgtbl(Pgtbl->Pgdir[(unsigned int)Count]);
    }
}
/* End Function:Proc::Alloc_Macro_Pgtbl **************************************/

/* Begin Function:Proc::Alloc_Macro *******************************************
Description : Allocate the capability ID macros. Both the local one and the global
              one will be allocated.
              The allocation table is shown below:
-------------------------------------------------------------------------------
Type            Local                           Global
-------------------------------------------------------------------------------
Process         -                               RVM_PROC_<PROCNAME>
-------------------------------------------------------------------------------
Pgtbl           -                               RVM_PGTBL_<PROCNAME>_N#num
-------------------------------------------------------------------------------
Captbl          -                               RVM_CAPTBL_<PROCNAME>
-------------------------------------------------------------------------------
Thread          -                               RVM_PROC_<PROCNAME>_THD_<THDNAME>
-------------------------------------------------------------------------------
Invocation      -                               RVM_PROC_<PROCNAME>_INV_<INVNAME>
-------------------------------------------------------------------------------
Port            PROC_<PROCNAME>_PORT_<PORTNAME> (Inherit invocation name)
-------------------------------------------------------------------------------
Receive         RECV_<ENDPNAME>                 RVM_PROC_<PROCNAME>_RECV_<RECVNAME>
-------------------------------------------------------------------------------
Send            PROC_<PROCNAME>_SEND_<ENDPNAME> (Inherit receive endpoint name)
-------------------------------------------------------------------------------
Vector          VECT_<VECTNAME>                 RVM_BOOT_VECT_<VECTNAME> (RVM)
                                                RME_BOOT_VECT_<VECTNAME> (RME)
-------------------------------------------------------------------------------
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void Proc::Alloc_Macro(void)
{
    this->RVM_Macro=std::make_unique<std::string>(std::string("RVM_PROC_")+*(this->Name));
    Kobj::To_Upper(this->RVM_Macro);
    Alloc_Macro_Pgtbl(this->Pgtbl);
    this->Captbl->RVM_Macro=std::make_unique<std::string>(std::string("RVM_CAPTBL_")+*(this->Name));
    Kobj::To_Upper(this->Captbl->RVM_Macro);

    for(std::unique_ptr<class Thd>& Thd:this->Thd)
    {
        Thd->RVM_Macro=std::make_unique<std::string>(std::string("RVM_PROC_")+*(this->Name)+"_THD_"+*(Thd->Name));
        Kobj::To_Upper(Thd->RVM_Macro);
    }

    for(std::unique_ptr<class Inv>& Inv:this->Inv)
    {
        Inv->RVM_Macro=std::make_unique<std::string>(std::string("RVM_PROC_")+*(this->Name)+"_INV_"+*(Inv->Name));
        Kobj::To_Upper(Inv->RVM_Macro);
    }

    for(std::unique_ptr<class Port>& Port:this->Port)
    {
        Port->Loc_Macro=std::make_unique<std::string>(std::string("PORT_")+*(Port->Name));
        Kobj::To_Upper(Port->Loc_Macro);
    }

    for(std::unique_ptr<class Recv>& Recv:this->Recv)
    {
        Recv->Loc_Macro=std::make_unique<std::string>(std::string("RECV_")+*(Recv->Name));
        Kobj::To_Upper(Recv->Loc_Macro);
        Recv->RVM_Macro=std::make_unique<std::string>(std::string("RVM_PROC_")+*(this->Name)+"_RECV_"+*(Recv->Name));
        Kobj::To_Upper(Recv->RVM_Macro);
    }

    for(std::unique_ptr<class Send>& Send:this->Send)
    {
        Send->Loc_Macro=std::make_unique<std::string>(std::string("SEND_")+*(Send->Name));
        Kobj::To_Upper(Send->Loc_Macro);
    }

    for(std::unique_ptr<class Vect>& Vect:this->Vect)
    {
        Vect->Loc_Macro=std::make_unique<std::string>(std::string("VECT_")+*(Vect->Name));
        Kobj::To_Upper(Vect->Loc_Macro);
        Vect->RVM_Macro=std::make_unique<std::string>(std::string("RVM_BOOT_VECT_")+*(Vect->Name));
        Kobj::To_Upper(Vect->RVM_Macro);
        Vect->RME_Macro=std::make_unique<std::string>(std::string("RME_BOOT_VECT_")+*(Vect->Name));
        Kobj::To_Upper(Vect->RME_Macro);
    }
}
/* End Function:Proc::Alloc_Macro ********************************************/

/* Begin Function:Proc::Alloc_Mem *********************************************
Description : Allocate process memory.
Input       : ptr_t Word_Bits - The number of bits in a word.
Output      : None.
Return      : None.
******************************************************************************/
void Proc::Alloc_Mem(ptr_t Word_Bits)
{
    class Mem* Mem;

    /* Deal with primary code and data sections */
    Mem=this->Code[0].get();
    this->Map->Code_Base=Mem->Start;
    this->Map->Code_Size=Mem->Size;
    this->Map->Entry_Code_Front=this->Map->Code_Base;
    
    Mem=this->Data[0].get();
    this->Map->Data_Base=Mem->Start;
    this->Map->Data_Size=Mem->Size;

    /* Threads come first */
    for(std::unique_ptr<class Thd>& Thd:this->Thd)
    {
        /* Allocate stack from the main data memory */
        Thd->Map->Stack_Base=this->Map->Data_Base+this->Map->Data_Size-Thd->Stack_Size;
        Thd->Map->Stack_Size=Thd->Stack_Size;
        if(Thd->Map->Stack_Base<=this->Map->Data_Base)
            throw std::runtime_error("Data section size is not big enough:\nUnable to allocate stack for thread: "+*(Thd->Name)+".");
        this->Map->Data_Size=Thd->Map->Stack_Base-this->Map->Data_Base;

        /* Allocate entry from code memory */
        Thd->Map->Entry_Addr=this->Map->Entry_Code_Front;
        this->Map->Entry_Code_Front+=Word_Bits/8*ENTRY_SLOT_SIZE;

        /* The parameter is always the param, turned into an unsigned integer */
        Thd->Map->Param_Value=Thd->Param;
    }

    /* Then invocations */
    for(std::unique_ptr<class Inv>& Inv:this->Inv)
    {
        /* Allocate stack from the main data memory */
        Inv->Map->Stack_Base=this->Map->Data_Base+this->Map->Data_Size-Inv->Stack_Size;
        Inv->Map->Stack_Size=Inv->Stack_Size;
        if(Inv->Map->Stack_Base<=this->Map->Data_Base)
            throw std::runtime_error("Data section size is not big enough:\nUnable to allocate stack for invocation: "+*(Inv->Name)+".");
        this->Map->Data_Size=Inv->Map->Stack_Base-this->Map->Data_Base;

        /* Allocate entry from code memory */
        Inv->Map->Entry_Addr=this->Map->Entry_Code_Front;
        this->Map->Entry_Code_Front+=Word_Bits/8*ENTRY_SLOT_SIZE;
    }
}
/* End Function:Proc::Alloc_Mem **********************************************/
}
/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
