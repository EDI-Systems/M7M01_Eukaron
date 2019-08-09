/******************************************************************************
Filename    : rme_a7m_tc_armc5.cpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The ARM toolchain v5 port.
******************************************************************************/

/* Includes ******************************************************************/
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

extern "C"
{
#include "time.h"
#include "xml.h"
#include "pbfs.h"
}

#include "list"
#include "string"
#include "memory"
#include "vector"
#include "stdexcept"

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

#include "A7M/rme_a7m_ide_keil.hpp"
#include "A7M/rme_a7m.hpp"
#undef __HDR_DEFS__

#define __HDR_CLASSES__
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

#include "A7M/rme_a7m_ide_keil.hpp"
#include "A7M/rme_a7m.hpp"
#undef __HDR_CLASSES__
/* End Includes **************************************************************/
namespace rme_mcu
{
/* Begin Function:A7M_IDE_Keil::Proj ******************************************
Description : Generate the keil project for ARMv7-M.
Input       : class Main* Main - The main structure.
              std::unique_ptr<std::string>& Filename - The project filename, path included.
              ptr_t Opt - The optimization level chosen.
              ptr_t Timeopt - The time optimization level chosen.
              std::unique_ptr<std::string>& Target - The target name.
              std::unique_ptr<std::string>& Sct - The scatter file path.
              std::unique_ptr<std::vector<std::unique_ptr<std::string>>>& Inc - The include paths.
              std::unique_ptr<std::vector<std::unique_ptr<std::string>>>& Src - The source file paths.
Output      : None.
Return      : None.
******************************************************************************/
void A7M_IDE_Keil::Proj(class Main* Main,
                        std::unique_ptr<std::string>& Filename,
                        ptr_t Opt, ptr_t Timeopt,
                        std::unique_ptr<std::string>& Target,
                        std::unique_ptr<std::string>& Sct,
                        std::unique_ptr<std::vector<std::unique_ptr<std::string>>>& Inc,
                        std::unique_ptr<std::vector<std::unique_ptr<std::string>>>& Src)
{
    FILE* File;
    class A7M* A7M;
    std::string CPU_Type;
    std::string FPU_Type;
    std::string Endianness;
    std::string Dlloption;
    std::string Device;
    std::string Include_Path;
    
    std::unique_ptr<class Doc> Doc;
    std::unique_ptr<class Para> Para;

    A7M=static_cast<class A7M*>(Main->Plat.get());
    
    /* Decide CPU type, FPU type and endianness */
    switch(A7M->CPU_Type)
    {
        case A7M_CPU_CM0P:CPU_Type="Cortex-M0+";Dlloption="-pCM0+";break;
        case A7M_CPU_CM3:CPU_Type="Cortex-M3";Dlloption="-pCM3";break;
        case A7M_CPU_CM4:CPU_Type="Cortex-M4";Dlloption="-pCM4";break;
        case A7M_CPU_CM7:CPU_Type="Cortex-M7";Dlloption="-pCM7";break;
        default:throw std::runtime_error("A7M:\nInternal processor type error.");
    }
    switch(A7M->FPU_Type)
    {
        case A7M_FPU_NONE:FPU_Type="";break;
        case A7M_FPU_FPV4:FPU_Type="FPU2";break;
        case A7M_FPU_FPV5_SP:FPU_Type="FPU3(SFPU)";break;
        case A7M_FPU_FPV5_DP:FPU_Type="FPU3(DFPU)";break;
        default:throw std::runtime_error("A7M:\nInternal FPU type error.");break;
    }
    if(A7M->Endianness==A7M_END_LITTLE)
        Endianness="ELITTLE";
    else if(A7M->Endianness==A7M_END_BIG)
        Endianness="EBIG";
    else
        throw std::runtime_error("A7M:\nInternal endianness error.");

    /* Decide device */
    if(Main->Proj->Chip_Class->find("STM32")!=std::string::npos)
    {
        if(Main->Proj->Chip_Class->find("STM32F1")!=std::string::npos)
            Device=*(Main->Proj->Chip_Class);
        else
        {
            Device=*(Main->Proj->Chip_Full);
            /* Except for STM32F1, all chips end with suffix, and the last number is substituted with 'x' */
            Device.back()='x';
        }
    }
    else
        Device=*(Main->Proj->Chip_Full);

    /* Construct include paths */
    for(std::unique_ptr<std::string>& Include:*Inc)
        Include_Path+=*Include+";";
    Include_Path+=".";

    Doc=std::make_unique<class Doc>();
    Para=std::make_unique<class Para>("Keil");

    Para->Add("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>");
    Para->Add("<Project xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"project_projx.xsd\">");
    Para->Add("  <SchemaVersion>2.1</SchemaVersion>");
    Para->Add("  <Header>### uVision Project, (C) Keil Software</Header>");
    Para->Add("  <Targets>");
    Para->Add("    <Target>");
    Para->Add("      <TargetName>%s</TargetName>", Target->c_str());
    Para->Add("      <ToolsetNumber>0x4</ToolsetNumber>");
    Para->Add("      <ToolsetName>ARM-ADS</ToolsetName>");
    Para->Add("      <pCCUsed>ARMCC</pCCUsed>");
    Para->Add("      <uAC6>0</uAC6>");
    Para->Add("      <TargetOption>");
    Para->Add("        <TargetCommonOption>");
    Para->Add("          <Device>%s</Device>", Device.c_str());
    Para->Add("          <Vendor>%s</Vendor>", Main->Chip->Vendor->c_str());
    Para->Add("          <Cpu>IRAM(0x08000000,0x10000) IROM(0x20000000,0x10000) CPUTYPE(\"%s\") %s CLOCK(12000000) %s</Cpu>",
              CPU_Type.c_str(), FPU_Type.c_str(), Endianness.c_str());
    Para->Add("          <OutputDirectory>.\\Objects\\</OutputDirectory>");
    Para->Add("          <OutputName>%s</OutputName>", Target->c_str());
    Para->Add("          <CreateExecutable>1</CreateExecutable>");
    Para->Add("          <CreateHexFile>1</CreateHexFile>");
    Para->Add("          <DebugInformation>1</DebugInformation>");
    Para->Add("          <BrowseInformation>1</BrowseInformation>");
    Para->Add("          <ListingPath>.\\Listings\\</ListingPath>");
    Para->Add("          <HexFormatSelection>1</HexFormatSelection>");
    Para->Add("          <AfterMake>");
    Para->Add("            <RunUserProg1>0</RunUserProg1>");
    Para->Add("            <RunUserProg2>0</RunUserProg2>");
    Para->Add("            <UserProg1Name></UserProg1Name>");
    Para->Add("            <UserProg2Name></UserProg2Name>");
    Para->Add("            <UserProg1Dos16Mode>0</UserProg1Dos16Mode>");
    Para->Add("            <UserProg2Dos16Mode>0</UserProg2Dos16Mode>");
    Para->Add("            <nStopA1X>0</nStopA1X>");
    Para->Add("            <nStopA2X>0</nStopA2X>");
    Para->Add("          </AfterMake>");
    Para->Add("        </TargetCommonOption>");
    Para->Add("        <CommonProperty>");
    Para->Add("          <UseCPPCompiler>0</UseCPPCompiler>");
    Para->Add("          <RVCTCodeConst>0</RVCTCodeConst>");
    Para->Add("          <RVCTZI>0</RVCTZI>");
    Para->Add("          <RVCTOtherData>0</RVCTOtherData>");
    Para->Add("          <ModuleSelection>0</ModuleSelection>");
    Para->Add("          <IncludeInBuild>1</IncludeInBuild>");
    Para->Add("          <AlwaysBuild>0</AlwaysBuild>");
    Para->Add("          <GenerateAssemblyFile>0</GenerateAssemblyFile>");
    Para->Add("          <AssembleAssemblyFile>0</AssembleAssemblyFile>");
    Para->Add("          <PublicsOnly>0</PublicsOnly>");
    Para->Add("          <StopOnExitCode>3</StopOnExitCode>");
    Para->Add("          <CustomArgument></CustomArgument>");
    Para->Add("          <IncludeLibraryModules></IncludeLibraryModules>");
    Para->Add("          <ComprImg>1</ComprImg>");
    Para->Add("        </CommonProperty>");
    Para->Add("        <DllOption>");
    Para->Add("          <SimDllName>SARMCM3.DLL</SimDllName>");
    Para->Add("          <SimDllArguments> -REMAP -MPU</SimDllArguments>");
    Para->Add("          <SimDlgDll>DCM.DLL</SimDlgDll>");
    Para->Add("          <SimDlgDllArguments>%s</SimDlgDllArguments>", Dlloption.c_str());
    Para->Add("          <TargetDllName>SARMCM3.DLL</TargetDllName>");
    Para->Add("          <TargetDllArguments> -MPU</TargetDllArguments>");
    Para->Add("          <TargetDlgDll>TCM.DLL</TargetDlgDll>");
    Para->Add("          <TargetDlgDllArguments>%s</TargetDlgDllArguments>", Dlloption.c_str());
    Para->Add("        </DllOption>");
    Para->Add("        <TargetArmAds>");
    Para->Add("          <ArmAdsMisc>");
    Para->Add("            <useUlib>1</useUlib>");
    Para->Add("            <OptFeed>0</OptFeed>");
    Para->Add("          </ArmAdsMisc>");
    Para->Add("          <Cads>");
    Para->Add("            <interw>1</interw>");
    Para->Add("            <Optim>%lld</Optim>", Opt+1);
    Para->Add("            <oTime>%d</oTime>", (Timeopt!=OPT_SIZE));
    Para->Add("            <SplitLS>0</SplitLS>");
    Para->Add("            <OneElfS>1</OneElfS>");
    Para->Add("            <Strict>0</Strict>");
    Para->Add("            <EnumInt>1</EnumInt>");
    Para->Add("            <PlainCh>1</PlainCh>");
    Para->Add("            <Ropi>0</Ropi>");
    Para->Add("            <Rwpi>0</Rwpi>");
    Para->Add("            <wLevel>2</wLevel>");
    Para->Add("            <uThumb>0</uThumb>");
    Para->Add("            <uSurpInc>0</uSurpInc>");
    Para->Add("            <uC99>1</uC99>");
    Para->Add("            <uGnu>1</uGnu>");
    Para->Add("            <useXO>0</useXO>");
    Para->Add("            <v6Lang>1</v6Lang>");
    Para->Add("            <v6LangP>1</v6LangP>");
    Para->Add("            <vShortEn>1</vShortEn>");
    Para->Add("            <vShortWch>1</vShortWch>");
    Para->Add("            <v6Lto>0</v6Lto>");
    Para->Add("            <v6WtE>0</v6WtE>");
    Para->Add("            <v6Rtti>0</v6Rtti>");
    Para->Add("            <VariousControls>");
    Para->Add("              <MiscControls></MiscControls>");
    Para->Add("              <Define></Define>");
    Para->Add("              <Undefine></Undefine>");
    Para->Add("              <IncludePath>%s</IncludePath>",Include_Path.c_str());
    Para->Add("            </VariousControls>");
    Para->Add("          </Cads>");
    Para->Add("          <Aads>");
    Para->Add("            <interw>1</interw>");
    Para->Add("            <Ropi>0</Ropi>");
    Para->Add("            <Rwpi>0</Rwpi>");
    Para->Add("            <thumb>0</thumb>");
    Para->Add("            <SplitLS>0</SplitLS>");
    Para->Add("            <SwStkChk>0</SwStkChk>");
    Para->Add("            <NoWarn>0</NoWarn>");
    Para->Add("            <uSurpInc>0</uSurpInc>");
    Para->Add("            <useXO>0</useXO>");
    Para->Add("            <uClangAs>0</uClangAs>");
    Para->Add("            <VariousControls>");
    Para->Add("              <MiscControls></MiscControls>");
    Para->Add("              <Define></Define>");
    Para->Add("              <Undefine></Undefine>");
    Para->Add("              <IncludePath>%s</IncludePath>",Include_Path.c_str());
    Para->Add("            </VariousControls>");
    Para->Add("          </Aads>");
    Para->Add("          <LDads>");
    Para->Add("            <umfTarg>0</umfTarg>");
    Para->Add("            <Ropi>0</Ropi>");
    Para->Add("            <Rwpi>0</Rwpi>");
    Para->Add("            <noStLib>0</noStLib>");
    Para->Add("            <RepFail>1</RepFail>");
    Para->Add("            <useFile>0</useFile>");
    Para->Add("            <TextAddressRange>0x08000000</TextAddressRange>");
    Para->Add("            <DataAddressRange>0x20000000</DataAddressRange>");
    Para->Add("            <pXoBase></pXoBase>");
    Para->Add("            <ScatterFile>%s</ScatterFile>", Sct->c_str());
    Para->Add("            <IncludeLibs></IncludeLibs>");
    Para->Add("            <IncludeLibsPath></IncludeLibsPath>");
    Para->Add("            <Misc></Misc>");
    Para->Add("            <LinkerInputFile></LinkerInputFile>");
    Para->Add("            <DisabledWarnings></DisabledWarnings>");
    Para->Add("          </LDads>");
    Para->Add("        </TargetArmAds>");
    Para->Add("      </TargetOption>");
    Para->Add("      <Groups>");
    /* Print all files. We only have two groups in all cases. */
    Para->Add("        <Group>");
    Para->Add("          <GroupName>%s</GroupName>", Target->c_str());
    Para->Add("          <Files>");
    for(std::unique_ptr<std::string>& Source:*Src)
    {
        Para->Add("            <File>");

        if(Source->find_last_of("/")==std::string::npos)
            Para->Add("              <FileName>%s</FileName>", Source->c_str());
        else
            Para->Add("              <FileName>%s</FileName>", Source->substr(Source->find_last_of("/")+1).c_str());

        if(Source->back()=='c')
            Para->Add("              <FileType>1</FileType>");
        else
            Para->Add("              <FileType>2</FileType>");

        Para->Add("              <FilePath>%s</FilePath>", Source->c_str());
        Para->Add("            </File>");
    }
    Para->Add("          </Files>");
    Para->Add("        </Group>");
    Para->Add("        <Group>");
    Para->Add("          <GroupName>User</GroupName>");
    Para->Add("        </Group>");
    Para->Add("      </Groups>");
    Para->Add("    </Target>");
    Para->Add("  </Targets>");
    Para->Add("</Project>");

    Doc->Add(std::move(Para));

    /* After we finish all these, we go back and populate the re-read file functionality */
    File=Main->Dstfs->Open_File(Filename);
    Doc->Write(File);
    fclose(File);
}
/* End Function:A7M_IDE_Keil::Proj *******************************************/

/* Begin Function:A7M_IDE_Keil::RME_Proj **************************************
Description : Generate the RME keil project for ARMv7-M.
Input       : class Main* Main - The main structure.
Output      : None.
Return      : None.
******************************************************************************/
void A7M_IDE_Keil::RME_Proj(class Main* Main)
{
    class A7M* A7M;
    std::unique_ptr<std::string> Filename;
    std::unique_ptr<std::string> Target;
    std::unique_ptr<std::string> Sct;
    std::unique_ptr<std::vector<std::unique_ptr<std::string>>> Inc;
    std::unique_ptr<std::vector<std::unique_ptr<std::string>>> Src;

    /* Filename */
    Filename=std::make_unique<std::string>("M7M1_MuEukaron/Project/");
    *Filename+=*(Main->Proj->Name)+"_RME.uvprojx";

    /* Target name */
    Target=std::make_unique<std::string>("RME");
    
    /* Scatter file */
    Sct=std::make_unique<std::string>("../MEukaron/Include/Platform/A7M/Chips/");
    *Sct+=*(Main->Proj->Chip_Class)+"/rme_platform_"+*(Main->Proj->Chip_Class)+".sct";

    /* Allocate the include list */
    Inc=std::make_unique<std::vector<std::unique_ptr<std::string>>>();
    Inc->push_back(std::make_unique<std::string>("./Source"));
    Inc->push_back(std::make_unique<std::string>("./Include"));
    Inc->push_back(std::make_unique<std::string>("../MEukaron/Include"));

    /* Allocate the file list */
    Src=std::make_unique<std::vector<std::unique_ptr<std::string>>>();
    Src->push_back(std::make_unique<std::string>("./Source/rme_boot.c"));
    Src->push_back(std::make_unique<std::string>("./Source/rme_user.c"));
    Src->push_back(std::make_unique<std::string>("../MEukaron/Kernel/rme_kernel.c"));
    Src->push_back(std::make_unique<std::string>("../MEukaron/Platform/A7M/rme_platform_a7m.c"));
    
    A7M=static_cast<class A7M*>(Main->Plat.get());
    if(A7M->CPU_Type==A7M_CPU_CM0P)
        Src->push_back(std::make_unique<std::string>("../MEukaron/Platform/A7M/rme_platform_a7m0p_asm.s"));
    else
        Src->push_back(std::make_unique<std::string>("../MEukaron/Platform/A7M/rme_platform_a7m_asm.s"));
   
    Proj(Main,
         Filename, Main->Proj->RME->Comp->Opt,  Main->Proj->RME->Comp->Prio,
         Target, Sct, Inc, Src);
}
/* End Function:A7M_IDE_Keil::RME_Proj ***************************************/

/* Begin Function:A7M_IDE_Keil::RVM_Proj **************************************
Description : Generate the RVM keil project for ARMv7-M.
Input       : class Main* Main - The main structure.
Output      : None.
Return      : None.
******************************************************************************/
void A7M_IDE_Keil::RVM_Proj(class Main* Main)
{
    class A7M* A7M;
    std::unique_ptr<std::string> Filename;
    std::unique_ptr<std::string> Target;
    std::unique_ptr<std::string> Sct;
    std::unique_ptr<std::vector<std::unique_ptr<std::string>>> Inc;
    std::unique_ptr<std::vector<std::unique_ptr<std::string>>> Src;

    /* Filename */
    Filename=std::make_unique<std::string>("M7M2_MuEukaron/Project/");
    *Filename+=*(Main->Proj->Name)+"_RVM.uvprojx";

    /* Target name */
    Target=std::make_unique<std::string>("RVM");
    
    /* Scatter file */
    Sct=std::make_unique<std::string>("../MAmmonite/Include/Platform/A7M/Chips/");
    *Sct+=*(Main->Proj->Chip_Class)+"/rvm_platform_"+*(Main->Proj->Chip_Class)+".sct";

    /* Allocate the include list */
    Inc=std::make_unique<std::vector<std::unique_ptr<std::string>>>();
    Inc->push_back(std::make_unique<std::string>("./Source"));
    Inc->push_back(std::make_unique<std::string>("./Include"));
    Inc->push_back(std::make_unique<std::string>("../MAmmonite/Include"));

    /* Allocate the file list */
    Src=std::make_unique<std::vector<std::unique_ptr<std::string>>>();
    Src->push_back(std::make_unique<std::string>("./Source/rvm_boot.c"));
    Src->push_back(std::make_unique<std::string>("./Source/rvm_user.c"));
    Src->push_back(std::make_unique<std::string>("../MAmmonite/Init/rvm_init.c"));
    Src->push_back(std::make_unique<std::string>("../MAmmonite/Platform/A7M/rvm_platform_a7m.c"));
    
    A7M=static_cast<class A7M*>(Main->Plat.get());
    if(A7M->CPU_Type==A7M_CPU_CM0P)
        Src->push_back(std::make_unique<std::string>("../MAmmonite/Platform/A7M/rvm_platform_a7m0p_asm.s"));
    else
        Src->push_back(std::make_unique<std::string>("../MAmmonite/Platform/A7M/rvm_platform_a7m_asm.s"));
   
    Proj(Main,
         Filename, Main->Proj->RME->Comp->Opt,  Main->Proj->RME->Comp->Prio,
         Target, Sct, Inc, Src);
}
/* End Function:A7M_IDE_Keil::RVM_Proj ***************************************/
}
/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
