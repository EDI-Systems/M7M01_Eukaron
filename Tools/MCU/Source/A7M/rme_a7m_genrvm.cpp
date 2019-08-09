/******************************************************************************
Filename    : rme_genrvm.cpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The rvm folder generation class for ARMv7-M.
******************************************************************************/

/* Includes ******************************************************************/
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

extern "C"
{
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
#include "Gen/rme_genrvm.hpp"

#include "A7M/rme_a7m_tc_gcc.hpp"
#include "A7M/rme_a7m_tc_armc5.hpp"
#include "A7M/rme_a7m_ide_keil.hpp"
#include "A7M/rme_a7m_ide_eclipse.hpp"
#include "A7M/rme_a7m_ide_makefile.hpp"
#include "A7M/rme_a7m_genrvm.hpp"
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
#include "Gen/rme_genrvm.hpp"

#include "A7M/rme_a7m_tc_gcc.hpp"
#include "A7M/rme_a7m_tc_armc5.hpp"
#include "A7M/rme_a7m_ide_keil.hpp"
#include "A7M/rme_a7m_ide_eclipse.hpp"
#include "A7M/rme_a7m_ide_makefile.hpp"
#include "A7M/rme_a7m_genrvm.hpp"
#include "A7M/rme_a7m.hpp"
#undef __HDR_CLASSES__
/* End Includes **************************************************************/
namespace rme_mcu
{
/* Begin Function:A7M_RVM_Gen::Chip_Hdr ***************************************
Description : Generate the chip header for RVM. This is toolchain agnostic.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void A7M_RVM_Gen::Chip_Hdr(void)
{
    FILE* File;
    class A7M* A7M;
    class Para* Para;
    std::unique_ptr<class Doc> Doc;
    std::unique_ptr<std::list<std::unique_ptr<std::string>>> Content;

    /* Read the header file */
    Content=this->Main->Srcfs->Read_File("M7M2_MuAmmonite/MAmmonite/Include/Platform/%s/Chips/%s/rvm_platform_%s.h",
                                         this->Main->Proj->Plat_Name->c_str(),
                                         this->Main->Chip->Chip_Class->c_str(),
                                         this->Main->Chip->Chip_Class->c_str());
    Doc=std::make_unique<class Doc>(std::move(Content),DOCTYPE_CHDR);
    Para=Doc->Para.front().get();

    /* General settings */
    /* The virtual memory start address for the kernel objects */
    Para->Cdef("RVM_KMEM_VA_START",this->Main->Proj->RME->Map->Kmem_Base);
    /* The size of the kernel object virtual memory */
    Para->Cdef("RVM_KMEM_SIZE",this->Main->Proj->RME->Map->Kmem_Size);
    /* The maximum number of preemption priority levels in the system.
     * This parameter must be divisible by the word length - 32 is usually sufficient */
    Para->Cdef("RVM_MAX_PREEMPT_PRIO",(ret_t)(this->Main->Proj->RME->Kern_Prios));
    /* The granularity of kernel memory allocation, in bytes */
    Para->Cdef("RVM_KMEM_SLOT_ORDER",(ret_t)(this->Main->Proj->RME->Kmem_Order));

    /* Cortex-M related settings */
    /* Shared interrupt flag region address */
    Para->Cdef("RVM_A7M_INT_FLAG_ADDR",this->Main->Proj->RME->Map->Intf_Base);
    /* Initial kernel object frontier limit */
    Para->Cdef("RVM_A7M_KMEM_BOOT_FRONTIER",this->Main->Proj->RVM->Map->Before_Kmem_Front);
    /* Init process's first thread's entry point address */
    Para->Cdef("RVM_A7M_INIT_ENTRY",this->Main->Proj->RVM->Map->Code_Base|0x01);
    /* Init process's first thread's stack address */
    Para->Cdef("RVM_A7M_INIT_STACK",
               (this->Main->Proj->RVM->Map->Guard_Stack_Base+this->Main->Proj->RVM->Map->Guard_Stack_Size-16)&0xFFFFFFF0ULL);

    /* Fixed settings - we will refill these with database values */
    /* Number of MPU regions available */
    Para->Cdef("RVM_A7M_MPU_REGIONS",(ret_t)(this->Main->Chip->Regions));
    /* What is the FPU type? */
    A7M=static_cast<class A7M*>(this->Main->Plat.get());
    switch(A7M->FPU_Type)
    {
        case A7M_FPU_NONE:Para->Cdef("RVM_A7M_FPU_TYPE","RVM_A7M_FPU_NONE");break;
        case A7M_FPU_FPV4:Para->Cdef("RVM_A7M_FPU_TYPE","RVM_A7M_FPU_FPV4");break;
        case A7M_FPU_FPV5_SP:Para->Cdef("RVM_A7M_FPU_TYPE","RVM_A7M_FPU_FPV5_SP");break;
        case A7M_FPU_FPV5_DP:Para->Cdef("RVM_A7M_FPU_TYPE","RVM_A7M_FPU_FPV5_DP");break;
        default:throw std::runtime_error("A7M:\nInternal FPU type error.");break;
    }
    
    /* After we finish all these, we go back and populate the re-read file functionality */
    File=this->Main->Dstfs->Open_File("M7M2_MuAmmonite/MAmmonite/Include/Platform/%s/Chips/%s/rvm_platform_%s.h",
                                      this->Main->Proj->Plat_Name->c_str(),
                                      this->Main->Chip->Chip_Class->c_str(),
                                      this->Main->Chip->Chip_Class->c_str());
    Doc->Write(File);
    fclose(File);
}
/* End Function:A7M_RVM_Gen::Chip_Hdr ****************************************/

/* Begin Function:A7M_RVM_Gen::Asm ********************************************
Description : Generate the assembly file for RME. This is toolchain-dependent.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void A7M_RVM_Gen::Asm(void)
{
    if(*(this->Main->Format)=="keil")
        A7M_TC_Armc5::RVM_Asm(this->Main);
    else if(*(this->Main->Format)=="eclipse")
        A7M_TC_Gcc::RVM_Asm(this->Main);
    else if(*(this->Main->Format)=="makefile")
        A7M_TC_Gcc::RVM_Asm(this->Main);
    else
        throw std::runtime_error("A7M:\nThis output format is not supported.");
}
/* End Function:A7M_RVM_Gen::Asm *********************************************/

/* Begin Function:A7M_RVM_Gen::Lds ********************************************
Description : Generate the linker script for RME. This is toolchain-dependent.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void A7M_RVM_Gen::Lds(void)
{
    if(*(this->Main->Format)=="keil")
        A7M_TC_Armc5::RVM_Lds(this->Main);
    else if(*(this->Main->Format)=="eclipse")
        A7M_TC_Gcc::RVM_Lds(this->Main);
    else if(*(this->Main->Format)=="makefile")
        A7M_TC_Gcc::RVM_Lds(this->Main);
    else
        throw std::runtime_error("A7M:\nThis output format is not supported.");
}
/* End Function:A7M_RVM_Gen::Lds *********************************************/

/* Begin Function:A7M_RME_Gen::Proj *******************************************
Description : Generate the chip header for RME. This is toolchain-dependent.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void A7M_RVM_Gen::Proj(void)
{
    if(*(this->Main->Format)=="keil")
        A7M_IDE_Keil::RVM_Proj(this->Main);
    else if(*(this->Main->Format)=="eclipse")
        A7M_IDE_Eclipse::RVM_Proj(this->Main);
    else if(*(this->Main->Format)=="makefile")
        A7M_IDE_Makefile::RVM_Proj(this->Main);
    else
        throw std::runtime_error("A7M:\nThis output format is not supported.");
}
/* End Function:A7M_RVM_Gen::Proj ********************************************/

/* Begin Function:A7M_RVM_Gen::Plat_Gen ***************************************
Description : Generate platform-related portion of the RME project.
              This includes the chip's header itself, the linker script,
              and the organization of the project.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void A7M_RVM_Gen::Plat_Gen(void)
{
    Chip_Hdr();
    Asm();
    Lds();
    Proj();
}
/* End Function:A7M_RVM_Gen::Plat_Gen ****************************************/
}
/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
