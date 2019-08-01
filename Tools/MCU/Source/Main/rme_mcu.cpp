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
#include "iostream"
#include "iterator"
#include "stdexcept"
#include "algorithm"

#define __HDR_DEFS__
#include "Main/rme_mcu.hpp"
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

#include "Gen/rme_doc.hpp"
#include "Gen/rme_genrme.hpp"
#include "Gen/rme_genrvm.hpp"
#include "Gen/rme_genproc.hpp"
#include "Gen/rme_genproj.hpp"
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
#include "Main/rme_mcu.hpp"

#include "Gen/rme_doc.hpp"
#include "Gen/rme_genrme.hpp"
#include "Gen/rme_genrvm.hpp"
#include "Gen/rme_genproc.hpp"
#include "Gen/rme_genproj.hpp"
#undef __HDR_CLASSES__

#include "A7M/rme_a7m_mcu.hpp"
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
        this->Fsys=std::make_unique<class Sysfs>(std::make_unique<std::string>("../../../../../"),
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
        {
            this->Plat=std::make_unique<class A7M>(this->Proj,this->Chip);
            this->RME_Gen=std::make_unique<class A7M_RME_Gen>();
            this->RVM_Gen=std::make_unique<class A7M_RVM_Gen>();
            this->Proc_Gen=std::make_unique<class A7M_Proc_Gen>();
            this->Proj_Gen=std::make_unique<class A7M_Proj_Gen>();
        }
        else
            throw std::runtime_error("The specific platform is currently not supported.");

        this->Plat->Kmem_Order=this->Proj->RME->Kmem_Order;

        /* Generator objects */
        this->RME_Gen->Fsys=this->Fsys.get();
        this->RME_Gen->Plat=this->Plat.get();
        this->RME_Gen->Proj=this->Proj.get();
        this->RME_Gen->Chip=this->Chip.get();

        this->RVM_Gen->Fsys=this->Fsys.get();
        this->RVM_Gen->Plat=this->Plat.get();
        this->RVM_Gen->Proj=this->Proj.get();
        this->RVM_Gen->Chip=this->Chip.get();

        this->Proc_Gen->Fsys=this->Fsys.get();
        this->Proc_Gen->Plat=this->Plat.get();
        this->Proc_Gen->Proj=this->Proj.get();
        this->Proc_Gen->Chip=this->Chip.get();

        this->Proj_Gen->Fsys=this->Fsys.get();
        this->Proj_Gen->Plat=this->Plat.get();
        this->Proj_Gen->Proj=this->Proj.get();
        this->Proj_Gen->Chip=this->Chip.get();
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
    ptr_t Attr;

    for(std::unique_ptr<class Mem>& Mem:this->Chip->Code)
        Map.push_back(std::make_unique<class Memmap>(Mem));

    std::sort(Map.begin(),Map.end(),
    [](std::unique_ptr<class Memmap> const& Left, std::unique_ptr<class Memmap> const& Right)
    {
            return Left->Mem->Start<Right->Mem->Start;
    });

    /* Now populate the RME & RVM sections - must be continuous */
    Attr=MEM_READ|MEM_EXECUTE|MEM_STATIC;
    if(Memmap::Fit_Static(Map,this->Proj->RME->Code_Start,this->Proj->RME->Code_Size,Attr)!=0)
        throw std::runtime_error("RME code section is invalid, either wrong range or wrong attribute.");
    if(Memmap::Fit_Static(Map,this->Proj->RME->Code_Start+this->Proj->RME->Code_Size,this->Proj->RVM->Code_Size,Attr)!=0)
        throw std::runtime_error("RVM code section is invalid, either wrong range or wrong attribute.");

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
                    if(Memmap::Fit_Static(Map, Mem->Start, Mem->Size, Mem->Attr)!=0)
                        throw std::runtime_error("Code section is invalid, either wrong range or wrong attribute.");
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
        if(Memmap::Fit_Auto(Map, &(Mem->Start), Mem->Size, Mem->Align, Mem->Attr)!=0)
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
    ptr_t Attr;

    for(std::unique_ptr<class Mem>& Mem:this->Chip->Data)
        Map.push_back(std::make_unique<class Memmap>(Mem));

    std::sort(Map.begin(),Map.end(),
    [](std::unique_ptr<class Memmap> const& Left, std::unique_ptr<class Memmap> const& Right)
    {
            return Left->Mem->Start<Right->Mem->Start;
    });

    /* Now populate the RME & RVM sections - must be continuous */
    Attr=MEM_READ|MEM_WRITE|MEM_STATIC;
    if(Memmap::Fit_Static(Map,this->Proj->RME->Data_Start,this->Proj->RME->Data_Size,Attr)!=0)
        throw std::runtime_error("RME data section is invalid, either wrong range or wrong attribute.");
    if(Memmap::Fit_Static(Map,this->Proj->RME->Data_Start+this->Proj->RME->Data_Size,this->Proj->RVM->Data_Size,Attr)!=0)
        throw std::runtime_error("RVM data section is invalid, either wrong range or wrong attribute.");

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
                    if(Memmap::Fit_Static(Map, Mem->Start, Mem->Size, Mem->Attr)!=0)
                        throw std::runtime_error("Data section is invalid, either wrong range or wrong attribute.");
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
        if(Memmap::Fit_Auto(Map, &(Mem->Start), Mem->Size, Mem->Align, Mem->Attr)!=0)
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
                        if((Chip_Mem->Attr&Proc_Mem->Attr)!=Proc_Mem->Attr)
                            throw std::runtime_error(std::string("Process: ")+*(Proc->Name)+"\nDevice memory have wrong attributes.");
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
/* End Function:Main::Check_Device *******************************************/

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

/* Begin Function:Main::Link_Cap **********************************************
Description : Back propagate the global ID to all the ports and send endpoints,
              which are derived from kernel objects. Also detects if all the port
              and send endpoint names in the system are valid. If any of them includes
              dangling references to invocations and receive endpoints, abort.
              These comparisons use strcmp because we require that the process name
              cases match.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void Main::Link_Cap(void)
{
    class Proc* Proc_Dst;
    class Inv* Inv_Dst;
    class Recv* Recv_Dst;
    class Vect* Vect_Dst;

    for(std::unique_ptr<class Proc>& Proc:this->Proj->Proc)
    {
        /* For every port, there must be a invocation somewhere */
        for(std::unique_ptr<class Port>& Port:Proc->Port)
        {
            Proc_Dst=0;
            for(std::unique_ptr<class Proc>& Proc_Temp:this->Proj->Proc)
            {
                if(*(Proc_Temp->Name)==*(Port->Proc_Name))
                {
                    Proc_Dst=Proc_Temp.get();
                    break;
                }
            }
            if(Proc_Dst==0)
                throw std::runtime_error(std::string("Port:")+*(Port->Name)+"\nInvalid process name.");

            Inv_Dst=0;
            for(std::unique_ptr<class Inv>& Inv:Proc_Dst->Inv)
            {
                if(*(Inv->Name)==*(Port->Name))
                {
                    Inv_Dst=Inv.get();
                    break;
                }
            }
            if(Inv_Dst==0)
                throw std::runtime_error(std::string("Port:")+*(Port->Name)+"\nInvalid invocation name.");

            Port->RVM_Capid=Inv_Dst->RVM_Capid;
            Port->RVM_Macro=std::make_unique<std::string>(*(Inv_Dst->RVM_Macro));
        }

        /* For every send endpoint, there must be a receive endpoint somewhere */
        for(std::unique_ptr<class Send>& Send:Proc->Send)
        {
            Proc_Dst=0;
            for(std::unique_ptr<class Proc>& Proc_Temp:this->Proj->Proc)
            {
                if(*(Proc_Temp->Name)==*(Send->Proc_Name))
                {
                    Proc_Dst=Proc_Temp.get();
                    break;
                }
            }
            if(Proc_Dst==0)
                throw std::runtime_error(std::string("Send endpoint:")+*(Send->Name)+"\nInvalid process name.");

            Recv_Dst=0;
            for(std::unique_ptr<class Recv>& Recv:Proc_Dst->Recv)
            {
                if(*(Recv->Name)==*(Send->Name))
                {
                    Recv_Dst=Recv.get();
                    break;
                }
            }
            if(Recv_Dst==0)
                throw std::runtime_error(std::string("Send endpoint:")+*(Send->Name)+"\nInvalid receive endpoint name.");

            Send->RVM_Capid=Recv_Dst->RVM_Capid;
            Send->RVM_Macro=std::make_unique<std::string>(*(Recv_Dst->RVM_Macro));
        }

        /* For every vector, there must be a corresponding chip interrupt vector somewhere */
        for(std::unique_ptr<class Vect>& Vect:Proc->Vect)
        {
            Vect_Dst=0;
            for(std::unique_ptr<class Vect>& Vect_Chip:Chip->Vect)
            {
                if(*(Vect_Chip->Name)==*(Vect->Name))
                {
                    if(Vect_Chip->Num!=Vect->Num)
                        throw std::runtime_error(std::string("Vector endpoint:")+*(Vect->Name)+"\nInvalid vector number.");

                    Vect_Dst=Vect_Chip.get();
                    break;
                }
            }
            if(Vect_Dst==0)
                throw std::runtime_error(std::string("Vector endpoint:")+*(Vect->Name)+"\nInvalid vector name.");

            Vect->Num=Vect_Dst->Num;
        }
    }
}
/* End Function:Main::Link_Cap ***********************************************/

/* Begin Function:Main::Alloc_Cap *********************************************
Description : Allocate the capability ID and macros. Both the local one and
              the global one will be allocated.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void Main::Alloc_Cap(void)
{    
    try
    {
        std::string* Errmsg;

        /* Check processes */
        Errmsg=Kobj::Check_Kobj<class Proc>(this->Proj->Proc);
        if(Errmsg!=0)
            throw std::invalid_argument(std::string("Process: ")+*Errmsg+"\nName is duplicate or invalid.");

        /* Check other kernel objects */
        for(std::unique_ptr<class Proc>& Proc:this->Proj->Proc)
            Proc->Check_Kobj();
    
        /* Check for duplicate vectors */
        Errmsg=Vect::Check_Vect(this->Proj);
        if(Errmsg!=0)
            throw std::invalid_argument(std::string("Vector endpoint: ")+*Errmsg+"\nName/name is duplicate or invalid.");

        for(std::unique_ptr<class Proc>& Proc:this->Proj->Proc)
            Proc->Alloc_Loc(this->Plat->Capacity);

        for(std::unique_ptr<class Proc>& Proc:this->Proj->Proc)
            Proc->Alloc_RVM(this->Proj->RVM);

        for(std::unique_ptr<class Proc>& Proc:this->Proj->Proc)
            Proc->Alloc_Macro();

        Link_Cap();
    }
    catch(std::exception& Exc)
    {
        throw std::runtime_error(std::string("Capability allocator:\n")+Exc.what());
    }
}
/* End Function:Main::Alloc_Cap ********************************************/

/* Begin Function:Main::Alloc_Obj *********************************************
Description : Allocate the kernel objects and memory.
Input       : struct Proj_Info* Proj - The project structure.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : None.
******************************************************************************/
void Main::Alloc_Obj(void)
{
    try
    {
        /* Compute the kernel memory needed, disregarding the initial
         * capability table size because we don't know its size yet */
        this->Proj->Kobj_Alloc(this->Plat,0);
        /* Now recompute to get the real usage */
        this->Proj->Kobj_Alloc(this->Plat,this->Proj->RVM->Map->Captbl_Size);

        /* Populate RME information */
        this->Proj->RME->Alloc_Kmem(this->Proj->RVM->Map->After_Kmem_Front, this->Plat->Kmem_Order);

        /* Populate RVM information */
        this->Proj->RVM->Alloc_Mem(this->Proj->RME->Code_Start+this->Proj->RME->Code_Size,
                                   this->Proj->RME->Data_Start+this->Proj->RME->Data_Size);
    
        /* Populate Process information */
        for(std::unique_ptr<class Proc>& Proc:this->Proj->Proc)
            Proc->Alloc_Mem(this->Plat->Word_Bits);
    }
    catch(std::exception& Exc)
    {
        throw std::runtime_error(std::string("Object allocator:\n")+Exc.what());
    }
}
/* End Function:Main::Alloc_Obj **********************************************/
void Main::Gen_RME(void)
{
    this->RME_Gen->
}

void Main::Gen_RVM(void)
{

}

void Main::Gen_Proc(void)
{

}

void Main::Gen_Proj(void)
{

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
    try
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
        Main->Alloc_Cap();

/* Phase 4: Allocate object memory ********************************************/
        Main->Alloc_Obj();

/* Phase 5: Produce output ***************************************************/
        Main->Gen_RME();
        Main->Gen_RVM();
        Main->Gen_Proc();
        Main->Gen_Proj();
        /*
        use this top-down stripping structure. also, be sure to support RISC-V with full power.
        choose partners with care. try to find if there are any risc-v manufacturers.
        Main->Copy_Files();
        Main->Gen_Files();

        so far so good. this looks much better now.
        the job of tomorrow is to redesign the generator.
        the generator have the following components:

        1. generic file copyer - simply does copying - this is always done. 
        2. generic file generator - simply generates stuff - this needs dom model.
        3. port specific file copyer - simply copies files in that port.
        4. port specific file generator - simply generates files for that port.
        5. toolchain-specific file copyer - simply copies files in that port.
        6. toolchain-specific file generator - simply generates stuff for that toolchain.

        file copyer can be devided into three categories:
        RME copyer - copies RME related stuff.
        RVM copyer - copies RVM related stuff.
        Process copyer - copies process related stuff.

        for generators, we also have the three sets.

        we also need to process cases where the original file already exists. This is an major
            issue: what if the user modifies the file?

        ps. multi-core support and VMM support/posix support still pending.
        need to allocate time to that.

        if we detect the difference is just the memory map, simple - replace the linker scripts.
        at least make the linker scripts there. 
        what if we need to add a new thread?
        maybe we should use a DOM model to process them.
        that will be too hard for now, maybe. 
        when the user adds or removes kernel objects, things will change. 
        and also there will be a lot of mess if this is not dealt with carefully.
        we may need to do this now - must be done.
        use a dom model right out of the stuff - but may require us to read the projects!
        anyway, if we detect projects, we neevr regenerate them.
        the dom model should be at user desc. - or not?

        projects - do not touch
        linker scripts - always regenerate
        */
    }
    catch(std::exception& Exc)
    {
        std::cout<<(std::string("Error:\n")+Exc.what());
    }

    return 0;
}
/* End Function:main *********************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
