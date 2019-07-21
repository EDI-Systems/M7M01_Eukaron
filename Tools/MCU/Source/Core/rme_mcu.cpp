/******************************************************************************
Filename    : rme_mcu.cpp
Author      : pry
Date        : 20/04/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The configuration generator for the MCU ports. This does not
              apply to the desktop or mainframe port; it uses its own generator.
			  This generator includes 12 big steps, and is considerably complex.
               1. Process the command line arguments and figure out where the source
                  are located at.
               2. Read the project-level configuration XMLs and device-level 
                  configuration XMLs into its internal data structures. Should
                  we find any parsing errors, we report and error out.
               3. Align memory. For program memory and data memory, rounding their
                  size is allowed; for specifically pointed out memory, rounding
                  their size is not allowed. 
               4. Generate memory map. This places all the memory segments into
                  the memory map, and fixes their specific size, etc. 
               5. Check if the generated memory map is valid. Each process should have
                  at least one code section and one data section, and they shall all
                  be STATIC.
               6. Allocate local and global linear capability IDs for all kernel objects.
                  The global linear capability ID assumes that all capability in the
                  same class are in the same capability table, and in 32-bit systems
                  this may not be the case.
               7. Set up the folder structure of the project so that the port-specific
                  generators can directly use them.
               8. Call the port-level generator to generate the project and port-specific
                  files for the project.
                  1. Detect any errors in the configuration structure. If any is found,
                     error out.
                  2. Allocate the page table contents and allocate capid/macros for them.
                  3. Call the tool-level project generator to generate project files.
                     Should the tool have any project group or workspace creation capability,
                     create the project group or workspace.
                     Memory map and linker file is also generated in this phase. 
                     1. The generator should generate separate projects for the RME.
                     2. Then generates project for RVM. 
                     3. And generates project for all other processes.
               9. Generate the vector creation scripts for RME.
              10. Generate the kernel object creation and delegation scripts for RVM.
                  1. Generate the capability tables.
                  2. Generate the page tables, calls the port-specific generator callback.
                  3. Generate the processes.
                  4. Generate the threads.
                  5. Generate the invocations.
                  6. Generate the receive endpoints.
                  7. Generate the delegation scripts.
              11. Generate stubs for all processes.
              12. Report to the user that the project generation is complete.
******************************************************************************/

/* Includes ******************************************************************/
extern "C"
{
#include "xml.h"
#include "pbfs.h"
}

#include "string"
#include "memory"
#include "vector"
#include "iterator"
#include "stdexcept"
#include "algorithm"

#define __HDR_DEFS__
#include "Core/rme_mcu.hpp"
#include "Core/rme_fsys.hpp"
#include "Core/rme_chip.hpp"
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

#include "A7M/rme_a7m.hpp"
#undef __HDR_DEFS__

#define __HDR_CLASSES__
#include "Core/rme_fsys.hpp"
#include "Core/rme_chip.hpp"
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
#include "Core/rme_mcu.hpp"

#include "A7M/rme_a7m.hpp"
#undef __HDR_CLASSES__
/* End Includes **************************************************************/
namespace rme_mcu
{
/* Begin Function:Main::Main *************************************************
Description : Preprocess the input parameters, and generate a preprocessed
              instruction listing with all the comments stripped.
Input       : int argc - The number of arguments.
              char* argv[] - The arguments.
Output      : s8_t** Input_File - The input project file path.
              s8_t** Output_File - The output folder path, must be empty.
			  s8_t** Format - The output format.
Return      : None.
******************************************************************************/
/* void */ Main::Main(int argc, char* argv[])
{
    ptr_t Count;

    try
    {
        if(argc!=7)
        throw std::runtime_error("Too many or too few input parameters.\n"
                                 "Usage: -i input.xml -o output_path -f format.\n"
                                 "       -i: Project description file.\n"
                                 "       -o: Output folder, must be empty.\n"
                                 "       -f: Output file format.\n"
                                 "           keil: Keil uVision IDE.\n"
                                 "           eclipse: Eclipse IDE.\n"
                                 "           makefile: Makefile project.\n");

	    this->Input=nullptr;
	    this->Output=nullptr;
	    this->Format=nullptr;

        Count=1;
        /* Read the command line one by one */
        while(Count<(ptr_t)argc)
        {
            /* We need to open some input file */
            if(strcmp(argv[Count],"-i")==0)
            {
                if(this->Input!=nullptr)
                    throw std::invalid_argument("More than one input file.");

                this->Input=std::make_unique<std::string>(argv[Count+1]);
                Count+=2;
            }
            /* We need to check some output path. We no longer care if it is empty */
            else if(strcmp(argv[Count],"-o")==0)
            {
                if(this->Output!=nullptr)
                    throw std::invalid_argument("More than one output path.");

                this->Output=std::make_unique<std::string>(argv[Count+1]);
                Count+=2;
            }
            /* We need to set the format of the output project */
            else if(strcmp(argv[Count],"-f")==0)
            {
                if(this->Format!=nullptr)
                    throw std::invalid_argument("More than one output format.");
            
                this->Format=std::make_unique<std::string>(argv[Count+1]);
                Count+=2;
            }
            else
                throw std::invalid_argument("Unrecognized command linev argument.");
        }

        if(this->Input==nullptr)
            throw std::invalid_argument("No input file specified.");
        if(this->Output==nullptr)
            throw std::invalid_argument("No output path specified.");
        if(this->Format==nullptr)
            throw std::invalid_argument("No output project format specified.");
    }
    catch(std::exception& Exc)
    {
        throw std::runtime_error(std::string("Command line:\n")+Exc.what());
    }
}
/* End Function:Main::Main ***************************************************/

/* Begin Function:Main::Parse *************************************************
Description : Parse the files.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void Main::Parse(void)
{
    std::unique_ptr<std::string> Str;
    xml_node_t* Node;

    try
    {
        this->Fsys=std::make_unique<class Sysfs>(std::make_unique<std::string>("../../../"),
                                                 std::make_unique<std::string>(this->Output->c_str()));
        /* Read project */
        Str=this->Fsys->Read_Proj(this->Input);
        if(XML_Parse(&Node,(xml_s8_t*)(Str->c_str()))<0)
            throw std::runtime_error("Project XML parsing failed.");
        this->Proj=std::make_unique<class Proj>(Node);
        if(XML_Del(Node)<0)
            throw std::runtime_error("Project XML parsing failed.");

        /* Read chip */
        Str=std::make_unique<std::string>("M7M1_MuEukaron/MEukaron/Include/Platform/");
        *Str+=*(this->Proj->Plat_Name)+"/Chips/"+*(this->Proj->Chip_Class)+"/rme_platform_"+*(this->Proj->Chip_Class)+".xml";
        Str=this->Fsys->Read_Chip(Str);
        if(XML_Parse(&Node,(xml_s8_t*)(Str->c_str()))<0)
            throw std::runtime_error("Chip XML parsing failed.");
        this->Chip=std::make_unique<class Chip>(Node);
        if(XML_Del(Node)<0)
            throw std::runtime_error("Chip XML parsing failed.");

        /* Select platform */
        if(*(this->Chip->Plat)!=*(this->Proj->Plat_Name))
            throw std::runtime_error("Project XML and chip XML platform does not match.");
        if(*(this->Chip->Chip_Class)!=*(this->Proj->Chip_Class))
            throw std::runtime_error("Project XML and chip XML chip class does not match.");
        if((*(this->Chip->Chip_Compat)).find(*(this->Proj->Chip_Full),0)==std::string::npos)
            throw std::runtime_error("The specific chip designated in project XML not found in chip XML.");

        if(*(this->Chip->Plat)=="A7M")
            this->Plat=std::make_unique<class A7M>(this->Proj,this->Chip);
    }
    catch(std::exception& Exc)
    {
        throw std::runtime_error(std::string("Parse:\n")+Exc.what());
    }
}
/* End Function:Main::Parse **************************************************/

/* Begin Function:Main::Alloc_Code ********************************************
Description : Allocate the code section of all processes.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void Main::Alloc_Code(void)
{
    std::vector<std::unique_ptr<class Memmap>> Map;
    std::vector<class Mem*> Auto;

    for(std::unique_ptr<class Mem>& Mem:this->Chip->Code)
        Map.push_back(std::make_unique<class Memmap>(Mem));

    std::sort(Map.begin(),Map.end(),
    [](std::unique_ptr<class Memmap> const& Left, std::unique_ptr<class Memmap> const& Right)
    {
            return Left->Mem->Start<Right->Mem->Start;
    });

    /* Now populate the RME & RVM sections - must be continuous */
    if(Memmap::Fit_Static(Map,this->Proj->RME->Code_Start,this->Proj->RME->Code_Size)!=0)
        throw std::runtime_error("RME code section is invalid.");
    if(Memmap::Fit_Static(Map,this->Proj->RME->Code_Start+this->Proj->RME->Code_Size,this->Proj->RVM->Code_Size)!=0)
        throw std::runtime_error("RVM code section is invalid.");

    /* Sort all processes's memory in according to their size */
    for(std::unique_ptr<class Proc>& Proc:this->Proj->Proc)
    {
        try
        {
            for(std::unique_ptr<class Mem>& Mem:Proc->Code)
            {
                /* If this memory is not auto memory, we wait to deal with it */
                if(Mem->Start!=MEM_AUTO)
                {
                    if(Memmap::Fit_Static(Map, Mem->Start, Mem->Size)!=0)
                        throw std::runtime_error("Code section is invalid.");
                }
                else
                    Auto.push_back(Mem.get());
            }
        }
        catch(std::exception& Exc)
        {
            throw std::runtime_error(std::string("Process: ")+*(Proc->Name)+"\n"+Exc.what());
        }
    }
    
    std::sort(Auto.begin(),Auto.end(),
    [](class Mem* const& Left, class Mem* const& Right)
    {
            return Left->Size<Right->Size;
    });

    /* Fit whatever that does not have a fixed address */
    for(class Mem*& Mem:Auto)
    {
        if(Memmap::Fit_Auto(Map, &(Mem->Start), Mem->Size, Mem->Align)!=0)
            throw std::runtime_error("Code memory fitter failed.");
    }
}
/* End Function:Main::Alloc_Code *********************************************/

/* Begin Function:Main::Check_Code ********************************************
Description : Check if the code layout is valid.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void Main::Check_Code(void)
{
    std::vector<class Mem*> Prim;
    std::vector<class Mem*>::iterator Iter;
    std::unique_ptr<class Mem> Sys;
    class Mem* Prev;
    class Mem* Next;

    /* Create and insert system memory into that list */
    Sys=std::make_unique<class Mem>(this->Proj->RME->Code_Start,this->Proj->RME->Code_Size+this->Proj->RVM->Code_Size,0,0);
    Prim.push_back(Sys.get());

    /* Insert all primary code sections into that list */
    for(std::unique_ptr<class Proc>& Proc:this->Proj->Proc)
        Prim.push_back(Proc->Code[0].get());

    std::sort(Prim.begin(),Prim.end(),
    [](const class Mem* Left, const class Mem* Right)
    {
            return Left->Start<Right->Start;
    });

    /* Check if adjacent memories will overlap */
    for(Iter=Prim.begin();Iter!=Prim.end();Iter++)
    {
        /* If we still have something after us, check for overlap */
        Prev=*Iter;
        if(std::next(Iter)!=Prim.end())
        {
            Next=*std::next(Iter);
            if((Prev->Start+Prev->Size)>Next->Start)
                throw std::runtime_error("Process primary code section overlapped.");
        }
    }
}
/* End Function:Main::Check_Code *********************************************/

/* Begin Function:Main::Alloc_Data ********************************************
Description : Allocate the data section of all processes.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void Main::Alloc_Data(void)
{
    std::vector<std::unique_ptr<class Memmap>> Map;
    std::vector<class Mem*> Auto;

    for(std::unique_ptr<class Mem>& Mem:this->Chip->Data)
        Map.push_back(std::make_unique<class Memmap>(Mem));

    std::sort(Map.begin(),Map.end(),
    [](std::unique_ptr<class Memmap> const& Left, std::unique_ptr<class Memmap> const& Right)
    {
            return Left->Mem->Start<Right->Mem->Start;
    });

    /* Now populate the RME & RVM sections - must be continuous */
    if(Memmap::Fit_Static(Map,this->Proj->RME->Data_Start,this->Proj->RME->Data_Size)!=0)
        throw std::runtime_error("RME data section is invalid.");
    if(Memmap::Fit_Static(Map,this->Proj->RME->Data_Start+this->Proj->RME->Data_Size,this->Proj->RVM->Data_Size)!=0)
        throw std::runtime_error("RVM data section is invalid.");

    /* Sort all processes's memory in according to their size */
    for(std::unique_ptr<class Proc>& Proc:Proj->Proc)
    {
        try
        {
            for(std::unique_ptr<class Mem>& Mem:Proc->Data)
            {
                /* If this memory is not auto memory, we wait to deal with it */
                if(Mem->Start!=MEM_AUTO)
                {
                    if(Memmap::Fit_Static(Map, Mem->Start, Mem->Size)!=0)
                        throw std::runtime_error("Data section is invalid.");
                }
                else
                    Auto.push_back(Mem.get());
            }
        }
        catch(std::exception& Exc)
        {
            throw std::runtime_error(std::string("Process: ")+*(Proc->Name)+"\n"+Exc.what());
        }
    }
    
    std::sort(Auto.begin(),Auto.end(),
    [](const class Mem* const& Left, const class Mem* const& Right)
    {
        return Left->Size<Right->Size;
    });

    /* Fit whatever that does not have a fixed address */
    for(class Mem*& Mem:Auto)
    {
        if(Memmap::Fit_Auto(Map, &(Mem->Start), Mem->Size, Mem->Align)!=0)
            throw std::runtime_error("Data memory fitter failed.");
    }
}
/* End Function:Main::Alloc_Data *********************************************/

/* Begin Function:Main::Check_Device ******************************************
Description : Check the device memory to make sure that all of them falls into range.
              There are faster algorithms; but we don't do such optimization now.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void Main::Check_Device(void)
{
    ptr_t Found;

    /* Is it true that the device memory is in device memory range? */
    for(std::unique_ptr<class Proc>& Proc:this->Proj->Proc)
    {
        for(std::unique_ptr<class Mem>& Proc_Mem:Proc->Device)
        {
            Found=0;

            for(std::unique_ptr<class Mem>& Chip_Mem:this->Chip->Device)
            {
                if(Proc_Mem->Start>=Chip_Mem->Start)
                {
                    if((Proc_Mem->Start+Proc_Mem->Size)<=(Chip_Mem->Start+Chip_Mem->Size))
                    {
                        Found=1;
                        break;
                    }
                }
            }

            if(Found==0)
                throw std::runtime_error(std::string("Process: ")+*(Proc->Name)+"\nDevice memory segment is out of bound.");
        }
    }
}
/* End Function:Check_Device *************************************************/

/* Begin Function:Main::Alloc_Mem *********************************************
Description : Allocate memoro trunks to their respective positions.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void Main::Alloc_Mem(void)
{
    try
    {
        Alloc_Code();
        Check_Code();
        Alloc_Data();
        Check_Device();
    }
    catch(std::exception& Exc)
    {
        throw std::runtime_error(std::string("Memory allocator:\n")+Exc.what());
    }
}
/* End Function:Main::Alloc_Mem **********************************************/

/* Begin Function:Main::Check_Kobj ********************************************
Description : Detect namespace conflicts in the system. It also checks if the
              names are at least regular C identifiers.
Input       : struct Proj_Info* Proj - The project information struct.
Output      : None.
Return      : None.
******************************************************************************/
void Main::Check_Kobj(void)
{
    std::string* Errmsg;

    /* Check processes */
    Errmsg=Kobj::Check_Kobj<class Proc>(this->Proj->Proc);
    if(Errmsg!=0)
        throw std::invalid_argument(std::string("Process: ")+*Errmsg+"\nName is duplicate or invalid.");
        
    /* Check other kernel objects */
    for(std::unique_ptr<class Proc>& Proc:this->Proj->Proc)
    {
        try
        {
            /* Check for duplicate threads */
            Errmsg=Kobj::Check_Kobj<class Thd>(Proc->Thd);
            if(Errmsg!=0)
                throw std::invalid_argument(std::string("Thread: ")+*Errmsg+"\nName is duplicate or invalid.");

            /* Check for duplicate invocations */
            Errmsg=Kobj::Check_Kobj<class Inv>(Proc->Inv);
            if(Errmsg!=0)
                throw std::invalid_argument(std::string("Invocation: ")+*Errmsg+"\nName is duplicate or invalid.");

            /* Check for duplicate ports */
            Errmsg=Kobj::Check_Kobj_Proc_Name<class Port>(Proc->Port);
            if(Errmsg!=0)
                throw std::invalid_argument(std::string("Port: ")+*Errmsg+"\nName/process name is duplicate or invalid.");

            /* Check for duplicate receive endpoints */
            Errmsg=Kobj::Check_Kobj<class Recv>(Proc->Recv);
            if(Errmsg!=0)
                throw std::invalid_argument(std::string("Receive endpoint: ")+*Errmsg+"\nName is duplicate or invalid.");

            /* Check for duplicate send endpoints */
            Errmsg=Kobj::Check_Kobj_Proc_Name<class Send>(Proc->Send);
            if(Errmsg!=0)
                throw std::invalid_argument(std::string("Send endpoint: ")+*Errmsg+"\nName/process name is duplicate or invalid.");
        }
        catch(std::exception& Exc)
        {
            throw std::runtime_error(std::string("Process: ")+*(Proc->Name)+"\n"+Exc.what());
        }
    }
    
    /* Check for duplicate vectors */
    Errmsg=Vect::Check_Vect(this->Proj);
    if(Errmsg!=0)
        throw std::invalid_argument(std::string("Vector endpoint: ")+*Errmsg+"\nName/name is duplicate or invalid.");
}
/* End Function:Main::Check_Kobj *********************************************/

/* Begin Function:Main::Alloc_Loc *********************************************
Description : Allocate local capability IDs for all kernel objects. 
              Only ports, receive endpoints, send endpoints and vector endpoints
              have such ID.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void Main::Alloc_Loc(void)
{
    ptr_t Capid;

    for(std::unique_ptr<class Proc>& Proc:this->Proj->Proc)
    {
        Capid=0;

        for(std::unique_ptr<class Port>& Port:Proc->Port)
            Port->Loc_Capid=Capid++;

        for(std::unique_ptr<class Recv>& Recv:Proc->Recv)
            Recv->Loc_Capid=Capid++;

        for(std::unique_ptr<class Send>& Send:Proc->Send)
            Send->Loc_Capid=Capid++;

        for(std::unique_ptr<class Vect>& Vect:Proc->Vect)
            Vect->Loc_Capid=Capid++;

        Proc->Captbl->Front=Capid;
        Proc->Captbl->Size=Proc->Captbl->Front+Proc->Captbl->Extra;

        if(Proc->Captbl->Size>this->Plat->Capacity)
            throw std::runtime_error("Process: "+*(Proc->Name)+"\n"+"Capability too large.");
    }
}
/* End Function:Main::Alloc_Loc **********************************************/

/* Begin Function:Main::Alloc_RVM_Pgtbl ***************************************
Description : Recursively allocate page tables.
Input       : std::unique_ptr<class Proc>& Proc, std::unique_ptr<class Pgtbl>& Pgtbl.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : None.
******************************************************************************/
void Main::Alloc_RVM_Pgtbl(std::unique_ptr<class Proc>& Proc,
                           std::unique_ptr<class Pgtbl>& Pgtbl)
{
    ptr_t Count;

    Pgtbl->RVM_Capid=this->Proj->RVM->Pgtbl.size();
    this->Proj->RVM->Pgtbl.push_back(std::make_unique<class Cap>(Pgtbl,Proc));

    /* Recursively do allocation */
    for(Count=0;Count<Pgtbl->Pgdir.size();Count++)
    {
        if(Pgtbl->Pgdir[Count]!=nullptr)
            Alloc_RVM_Pgtbl(Proc, Pgtbl->Pgdir[Count]);
    }
}
/* End Function:Main::Alloc_RVM_Pgtbl ****************************************/

/* Begin Function:Main::Alloc_RVM *********************************************
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
Input       : struct Proj_Info* Proj - The project structure.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : None.
******************************************************************************/
void Main::Alloc_RVM(void)
{
    for(std::unique_ptr<class Proc>& Proc:this->Proj->Proc)
    {
        Proc->Captbl->RVM_Capid=this->Proj->RVM->Captbl.size();
        this->Proj->RVM->Captbl.push_back(std::make_unique<class Cap>(Proc->Captbl,Proc));
        Alloc_RVM_Pgtbl(Proc, Proc->Pgtbl);
        Proc->RVM_Capid=this->Proj->RVM->Proc.size();
        this->Proj->RVM->Proc.push_back(std::make_unique<class Cap>(Proc,Proc));

        for(std::unique_ptr<class Thd>& Thd:Proc->Thd)
        {
            Thd->RVM_Capid=this->Proj->RVM->Thd.size();
            this->Proj->RVM->Thd.push_back(std::make_unique<class Cap>(Thd,Proc));
        }

        for(std::unique_ptr<class Inv>& Inv:Proc->Inv)
        {
            Inv->RVM_Capid=this->Proj->RVM->Inv.size();
            this->Proj->RVM->Inv.push_back(std::make_unique<class Cap>(Inv,Proc));
        }

        for(std::unique_ptr<class Recv>& Recv:Proc->Recv)
        {
            Recv->RVM_Capid=this->Proj->RVM->Recv.size();
            this->Proj->RVM->Recv.push_back(std::make_unique<class Cap>(Recv,Proc));
        }

        for(std::unique_ptr<class Vect>& Vect:Proc->Vect)
        {
            Vect->RVM_Capid=this->Proj->RVM->Vect.size();
            this->Proj->RVM->Vect.push_back(std::make_unique<class Cap>(Vect,Proc));
        }
    }
}
/* End Function:Main::Alloc_RVM **********************************************/

/* Begin Function:Main::Alloc_Macro_Pgtbl *************************************
Description : Recursively allocate page tables.
Input       : std::unique_ptr<class Proc>& Proc, std::unique_ptr<class Pgtbl>& Pgtbl.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : None.
******************************************************************************/
void Main::Alloc_Macro_Pgtbl(std::unique_ptr<class Proc>& Proc,
                             std::unique_ptr<class Pgtbl>& Pgtbl)
{
    static ptr_t Serial;
    ptr_t Count;

    if(Pgtbl->Is_Top!=0)
        Serial=0;
    
    Pgtbl->RVM_Macro=std::make_unique<std::string>(std::string("RVM_PGTBL_")+*(Proc->Name)+
                                                   "_N"+std::to_string(Serial));
    Proj::To_Upper(Pgtbl->RVM_Macro);

    /* Recursively do allocation */
    for(Count=0;Count<Pgtbl->Pgdir.size();Count++)
    {
        if(Pgtbl->Pgdir[Count]!=nullptr)
            Alloc_Macro_Pgtbl(Proc, Pgtbl->Pgdir[Count]);
    }
}
/* End Function:Main::Alloc_Macro_Pgtbl **************************************/

/* Begin Function:Main::Alloc_Macro *******************************************
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
void Main::Alloc_Macro(void)
{
    for(std::unique_ptr<class Proc>& Proc:this->Proj->Proc)
    {
        Proc->RVM_Macro=std::make_unique<std::string>(std::string("RVM_PROC_")+*(Proc->Name));
        Proj::To_Upper(Proc->RVM_Macro);
        Alloc_Macro_Pgtbl(Proc,Proc->Pgtbl);
        Proc->Captbl->RVM_Macro=std::make_unique<std::string>(std::string("RVM_CAPTBL_")+*(Proc->Name));

        for(std::unique_ptr<class Thd>& Thd:Proc->Thd)
            Thd->RVM_Macro=std::make_unique<std::string>(std::string("RVM_PROC_")+*(Proc->Name)+"_THD_",*(Thd->Name));

        /* Invocations - RVM only */
        for(std::unique_ptr<class Inv>& Inv:Proc->Inv)
            Inv->Cap.RVM_Macro=Make_Macro("RVM_PROC_",Proc->Name,"_INV_",Inv->Name);

        /* Ports - Local only */
        for(EACH(struct Port_Info*,Port,Proc->Port))
            Port->Cap.Loc_Macro=Make_Macro("PORT_",Port->Name,"","");

        /* Receive endpoints - RVM and local */
        for(EACH(struct Recv_Info*,Recv,Proc->Recv))
        {
            Recv->Cap.Loc_Macro=Make_Macro("RECV_",Recv->Name,"","");
            Recv->Cap.RVM_Macro=Make_Macro("RVM_PROC_",Proc->Name,"_RECV_",Recv->Name);
        }

        /* Send endpoints - Local only */
        for(EACH(struct Send_Info*,Send,Proc->Send))
            Send->Cap.Loc_Macro=Make_Macro("SEND_",Send->Name,"","");

        /* Vector endpoints - RVM, RME and local */
        for(EACH(struct Vect_Info*,Vect,Proc->Vect))
        {
            Vect->Cap.Loc_Macro=Make_Macro("VECT_",Vect->Name,"","");
            Vect->Cap.RVM_Macro=Make_Macro("RVM_BOOT_VECT_",Vect->Name,"","");
            Vect->Cap.RME_Macro=Make_Macro("RME_BOOT_VECT_",Vect->Name,"","");
        }
    }
}
/* End Function:Main::Alloc_Macro ********************************************/

/* Begin Function:Backprop_Global_Capid ***************************************
Description : Back propagate the global ID to all the ports and send endpoints,
              which are derived from kernel objects. Also detects if all the port
              and send endpoint names in the system are valid. If any of them includes
              dangling references to invocations and receive endpoints, abort.
              These comparisons use strcmp because we require that the process name
              cases match.
Input       : struct Proj_Info* Proj - The project information structure.
              struct Chip_Info* Chip - The chip information structure.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : None.
******************************************************************************/
void Backprop_Global_Capid(struct Proj_Info* Proj, struct Chip_Info* Chip)
{
    struct Proc_Info* Proc;
    struct Proc_Info* Proc_Temp;
    struct Port_Info* Port;
    struct Inv_Info* Inv;
    struct Send_Info* Send;
    struct Recv_Info* Recv;
    struct Vect_Info* Vect;
    struct Chip_Vect_Info* Chip_Vect;

    for(EACH(struct Proc_Info*,Proc,Proj->Proc))
    {
        /* For every port, there must be a invocation somewhere */
        for(EACH(struct Port_Info*,Port,Proc->Port))
        {
            for(EACH(struct Proc_Info*,Proc_Temp,Proj->Proc))
            {
                if(strcmp(Proc_Temp->Name, Port->Proc_Name)==0)
                    break;
            }
            if(IS_HEAD(Proc_Temp,Proj->Proc))
                EXIT_FAIL("Invalid Process for Port.");

            for(EACH(struct Inv_Info*,Inv,Proc_Temp->Inv))
            {
                if(strcmp(Inv->Name, Port->Name)==0)
                {
                    Port->Cap.RVM_Capid=Inv->Cap.RVM_Capid;
                    Port->Cap.RVM_Macro=Inv->Cap.RVM_Macro;
                    break;
                }
            }
            if(IS_HEAD(Inv,Proc->Inv))
                EXIT_FAIL("Invalid Invocation for Port.");
        }

        /* For every send endpoint, there must be a receive endpoint somewhere */
        for(EACH(struct Send_Info*,Send,Proc->Send))
        {
            for(EACH(struct Proc_Info*,Proc_Temp,Proj->Proc))
            {
                if(strcmp(Proc_Temp->Name, Send->Proc_Name)==0)
                    break;
            }
            if(IS_HEAD(Proc_Temp,Proj->Proc))
                EXIT_FAIL("Invalid Process for Send endpoint.");

            for(EACH(struct Recv_Info*,Recv,Proc_Temp->Recv))
            {
                if(strcmp(Recv->Name, Send->Name)==0)
                {
                    Send->Cap.RVM_Capid=Recv->Cap.RVM_Capid;
                    Send->Cap.RVM_Macro=Recv->Cap.RVM_Macro;
                    break;
                }
            }
            if(IS_HEAD(Recv,Proc->Recv))
                EXIT_FAIL("Invalid Receive endpoint for Send endpoint.");
        }

        /* For every vector, there must be a corresponding chip interrupt vector somewhere */
        for(EACH(struct Vect_Info*,Vect,Proc->Vect))
        {
            for(EACH(struct Chip_Vect_Info*,Chip_Vect,Chip->Vect))
            {
                if(strcmp(Chip_Vect->Name, Vect->Name)==0)
                {
                    Vect->Num=Chip_Vect->Num;
                    break;
                }
            }
            if(IS_HEAD(Chip_Vect,Chip->Vect))
                EXIT_FAIL("Invalid Chip vector for Vector endpoint.");
        }
    }
}
/* End Function:Backprop_RVM *************************************************/


void Main::Alloc_Captbl(void)
{
    /* First, check whether there are conflicts - this is not case insensitive */
    Check_Kobj();
    /* Allocate local project IDs for all entries */
    Alloc_Loc();
    /* Allocate global project IDs for kernel object entries */
    Alloc_RVM();
    /* Allocate the global and local macros to them */
    Alloc_Macro();
    /* Back propagate global entrie number to the ports and send endpoints */
    Backprop_RVM();
}
}
/* Begin Function:main ********************************************************
Description : The entry of the tool.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
using namespace rme_mcu;
int main(int argc, char* argv[])
{
    std::unique_ptr<class Main> Main;
/* Phase 1: Process command line and do parsing ******************************/
    /* Process the command line first */
    Main=std::make_unique<class Main>(argc, argv);
    Main->Parse();

/* Phase 2: Allocate page tables *********************************************/
    Main->Plat->Align_Mem(Main->Proj);
    Main->Alloc_Mem();
    Main->Plat->Alloc_Pgtbl(Main->Proj,Main->Chip);

/* Phase 3: Allocate kernel object ID & macros *******************************/
    Main->Alloc_Captbl();

/* Phase 4: Allocate memory **************************************************/
    //Main->Alloc_Kmem();

/* Phase 5: Produce output ***************************************************/
    //Main->Output();

    return 0;
}
/* End Function:main *********************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
