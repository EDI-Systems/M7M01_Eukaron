/******************************************************************************
Filename    : rme_mcu.c
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
#include "stdexcept"

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
                                                 std::make_unique<std::string>(this->Output));
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
    }
    catch(std::exception& Exc)
    {
        throw std::runtime_error(std::string("Parse:\n")+Exc.what());
    }
}
/* End Function:Main::Parse **************************************************/

/* Begin Function:main ********************************************************
Description : The entry of the tool.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
int main(int argc, char* argv[])
{
    std::unique_ptr<class Main> Main;
/* Phase 1: Process command line and do parsing ******************************/
    /* Process the command line first */
    Main=std::make_unique<class Main>(argc, argv);
    Main->Parse();

    return 0;
}
/* End Function:main *********************************************************/
}
/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
