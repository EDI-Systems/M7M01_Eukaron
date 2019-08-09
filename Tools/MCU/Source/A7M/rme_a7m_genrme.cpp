/******************************************************************************
Filename    : rme_genrme.cpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The rme folder generation class for ARMv7-M.
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
#include "Gen/rme_genrme.hpp"

#include "A7M/rme_a7m_tc_gcc.hpp"
#include "A7M/rme_a7m_tc_armc5.hpp"
#include "A7M/rme_a7m_ide_keil.hpp"
#include "A7M/rme_a7m_ide_eclipse.hpp"
#include "A7M/rme_a7m_ide_makefile.hpp"
#include "A7M/rme_a7m_genrme.hpp"
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

#include "A7M/rme_a7m_tc_gcc.hpp"
#include "A7M/rme_a7m_tc_armc5.hpp"
#include "A7M/rme_a7m_ide_keil.hpp"
#include "A7M/rme_a7m_ide_eclipse.hpp"
#include "A7M/rme_a7m_ide_makefile.hpp"
#include "A7M/rme_a7m_genrme.hpp"
#include "A7M/rme_a7m.hpp"
#undef __HDR_CLASSES__
/* End Includes **************************************************************/
namespace rme_mcu
{
/* Begin Function:A7M_RME_Gen::Chip_Hdr ***************************************
Description : Generate the chip header for RME. This is toolchain agnostic.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void A7M_RME_Gen::Chip_Hdr(void)
{
    FILE* File;
    class A7M* A7M;
    class Para* Para;
    std::unique_ptr<class Doc> Doc;
    std::unique_ptr<std::list<std::unique_ptr<std::string>>> Content;

    /* Read the header file */
    Content=this->Main->Srcfs->Read_File("M7M1_MuEukaron/MEukaron/Include/Platform/%s/Chips/%s/rme_platform_%s.h",
                                         this->Main->Proj->Plat_Name->c_str(),
                                         this->Main->Chip->Chip_Class->c_str(),
                                         this->Main->Chip->Chip_Class->c_str());
    Doc=std::make_unique<class Doc>(std::move(Content),DOCTYPE_CHDR);
    Para=Doc->Para.front().get();

    /* General settings */
    /* The virtual memory start address for the kernel objects */
    Para->Cdef("RME_KMEM_VA_START",this->Main->Proj->RME->Map->Kmem_Base);
    /* The size of the kernel object virtual memory */
    Para->Cdef("RME_KMEM_SIZE",this->Main->Proj->RME->Map->Kmem_Size);
    /* The virtual memory start address for the virtual machines - If no virtual machines is used, set to 0 */
    Para->Cdef("RME_HYP_VA_START",0ULL);
    /* The size of the hypervisor reserved virtual memory */
    Para->Cdef("RME_HYP_SIZE",0ULL);
    /* Kernel stack address */
    Para->Cdef("RME_KMEM_STACK_ADDR",this->Main->Proj->RME->Map->Stack_Base+this->Main->Proj->RME->Map->Stack_Size-16);
    /* The maximum number of preemption priority levels in the system.
     * This parameter must be divisible by the word length - 32 is usually sufficient */
    Para->Cdef("RME_MAX_PREEMPT_PRIO",(ret_t)(this->Main->Proj->RME->Kern_Prios));
    /* The granularity of kernel memory allocation, in bytes */
    Para->Cdef("RME_KMEM_SLOT_ORDER",(ret_t)(this->Main->Proj->RME->Kmem_Order));

    /* Cortex-M related settings */
    /* Shared interrupt flag region address */
    Para->Cdef("RME_A7M_INT_FLAG_ADDR",this->Main->Proj->RME->Map->Intf_Base);
    /* Initial kernel object frontier limit */
    Para->Cdef("RME_A7M_KMEM_BOOT_FRONTIER",this->Main->Proj->RVM->Map->Before_Kmem_Front);
    /* Init process's first thread's entry point address */
    Para->Cdef("RME_A7M_INIT_ENTRY",this->Main->Proj->RVM->Map->Code_Base|0x01);
    /* Init process's first thread's stack address */
    Para->Cdef("RME_A7M_INIT_STACK",
               (this->Main->Proj->RVM->Map->Guard_Stack_Base+this->Main->Proj->RVM->Map->Guard_Stack_Size-16)&0xFFFFFFF0ULL);
    /* What is the NVIC priority grouping? */
    A7M=static_cast<class A7M*>(this->Main->Plat.get());
    switch(A7M->NVIC_Grouping)
    {
        case A7M_NVIC_P0S8:Para->Cdef("RME_A7M_NVIC_GROUPING","RME_A7M_NVIC_GROUPING_P0S8");break;
        case A7M_NVIC_P1S7:Para->Cdef("RME_A7M_NVIC_GROUPING","RME_A7M_NVIC_GROUPING_P1S7");break;
        case A7M_NVIC_P2S6:Para->Cdef("RME_A7M_NVIC_GROUPING","RME_A7M_NVIC_GROUPING_P2S6");break;
        case A7M_NVIC_P3S5:Para->Cdef("RME_A7M_NVIC_GROUPING","RME_A7M_NVIC_GROUPING_P3S5");break;
        case A7M_NVIC_P4S4:Para->Cdef("RME_A7M_NVIC_GROUPING","RME_A7M_NVIC_GROUPING_P4S4");break;
        case A7M_NVIC_P5S3:Para->Cdef("RME_A7M_NVIC_GROUPING","RME_A7M_NVIC_GROUPING_P5S3");break;
        case A7M_NVIC_P6S2:Para->Cdef("RME_A7M_NVIC_GROUPING","RME_A7M_NVIC_GROUPING_P6S2");break;
        case A7M_NVIC_P7S1:Para->Cdef("RME_A7M_NVIC_GROUPING","RME_A7M_NVIC_GROUPING_P7S1");break;
        default:throw std::runtime_error("A7M:\nInternal NVIC grouping error.");break;
    }
    /* What is the Systick value? - 10ms per tick*/
    Para->Cdef("RME_A7M_SYSTICK_VAL",(ret_t)(A7M->Systick_Val));

    /* Fixed settings - we will refill these with database values */
    /* Number of MPU regions available */
    Para->Cdef("RME_A7M_MPU_REGIONS",(ret_t)(this->Main->Chip->Regions));
    /* What is the FPU type? */
    switch(A7M->FPU_Type)
    {
        case A7M_FPU_NONE:Para->Cdef("RME_A7M_FPU_TYPE","RME_A7M_FPU_NONE");break;
        case A7M_FPU_FPV4:Para->Cdef("RME_A7M_FPU_TYPE","RME_A7M_FPU_FPV4");break;
        case A7M_FPU_FPV5_SP:Para->Cdef("RME_A7M_FPU_TYPE","RME_A7M_FPU_FPV5_SP");break;
        case A7M_FPU_FPV5_DP:Para->Cdef("RME_A7M_FPU_TYPE","RME_A7M_FPU_FPV5_DP");break;
        default:throw std::runtime_error("A7M:\nInternal FPU type error.");break;
    }
    
    /* After we finish all these, we go back and populate the re-read file functionality */
    File=this->Main->Dstfs->Open_File("M7M1_MuEukaron/MEukaron/Include/Platform/%s/Chips/%s/rme_platform_%s.h",
                                      this->Main->Proj->Plat_Name->c_str(),
                                      this->Main->Chip->Chip_Class->c_str(),
                                      this->Main->Chip->Chip_Class->c_str());
    Doc->Write(File);
    fclose(File);
}
/* End Function:A7M_RME_Gen::Chip_Hdr ****************************************/

/* Begin Function:A7M_RME_Gen::Asm ********************************************
Description : Generate the assembly file for RME. This is toolchain-dependent.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void A7M_RME_Gen::Asm(void)
{
    if(*(this->Main->Format)=="keil")
        A7M_TC_Armc5::RME_Asm(this->Main);
    else if(*(this->Main->Format)=="eclipse")
        A7M_TC_Gcc::RME_Asm(this->Main);
    else if(*(this->Main->Format)=="makefile")
        A7M_TC_Gcc::RME_Asm(this->Main);
    else
        throw std::runtime_error("A7M:\nThis output format is not supported.");
}
/* End Function:A7M_RME_Gen::Asm *********************************************/

/* Begin Function:A7M_RME_Gen::Lds ********************************************
Description : Generate the linker script for RME. This is toolchain-dependent.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void A7M_RME_Gen::Lds(void)
{
    if(*(this->Main->Format)=="keil")
        A7M_TC_Armc5::RME_Lds(this->Main);
    else if(*(this->Main->Format)=="eclipse")
        A7M_TC_Gcc::RME_Lds(this->Main);
    else if(*(this->Main->Format)=="makefile")
        A7M_TC_Gcc::RME_Lds(this->Main);
    else
        throw std::runtime_error("A7M:\nThis output format is not supported.");
}
/* End Function:A7M_RME_Gen::Lds *********************************************/

/* Begin Function:A7M_RME_Gen::Proj *******************************************
Description : Generate the chip header for RME. This is toolchain-dependent.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void A7M_RME_Gen::Proj(void)
{
    if(*(this->Main->Format)=="keil")
        A7M_IDE_Keil::RME_Proj(this->Main);
    else if(*(this->Main->Format)=="eclipse")
        A7M_IDE_Eclipse::RME_Proj(this->Main);
    else if(*(this->Main->Format)=="makefile")
        A7M_IDE_Makefile::RME_Proj(this->Main);
    else
        throw std::runtime_error("A7M:\nThis output format is not supported.");
}
/* End Function:A7M_RME_Gen::Proj ********************************************/

/* Begin Function:A7M_RME_Gen::Plat_Gen ***************************************
Description : Generate platform-related portion of the RME project.
              This includes the chip's header itself, the linker script,
              and the organization of the project.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void A7M_RME_Gen::Plat_Gen(void)
{
    Chip_Hdr();
    Asm();
    Lds();
    Proj();
}
/* End Function:A7M_RME_Gen::Plat_Gen ****************************************/
}
/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
