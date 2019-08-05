/******************************************************************************
Filename    : rme_genrme.cpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The rme folder generation class.
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
    std::unique_ptr<std::list<std::unique_ptr<std::string>>> File;
    std::unique_ptr<class Doc> Doc;
    class Para* Para;

    /* Read the header file */
    File=this->Srcfs->Read_File("M7M1_MuEukaron/MEukaron/Include/Platform/%s/Chips/%s/rme_platform_%s.h",
                                this->Proj->Plat_Name->c_str(),this->Chip->Chip_Class->c_str(),this->Chip->Chip_Class->c_str());
    Doc=std::make_unique<class Doc>(File,DOCTYPE_CHDR);
    Para=Doc->Para.front().get();

    /* Set the macros in this header file - what cdefs are there? */
    Para->Cdef();

    /* The virtual memory start address for the kernel objects */
#define RME_KMEM_VA_START                       0x20003000
/* The size of the kernel object virtual memory */
#define RME_KMEM_SIZE                           0xD000
/* The virtual memory start address for the virtual machines - If no virtual machines is used, set to 0 */
#define RME_HYP_VA_START                        0x20020000
/* The size of the hypervisor reserved virtual memory */
#define RME_HYP_SIZE                            0x60000
/* Kernel stack address - we have 4kB stack */
#define RME_KMEM_STACK_ADDR                     0x20000FF0
/* The maximum number of preemption priority levels in the system.
 * This parameter must be divisible by the word length - 32 is usually sufficient */
#define RME_MAX_PREEMPT_PRIO                    32

/* Shared interrupt flag region address - always use 256*4 = 1kB memory */
#define RME_A7M_INT_FLAG_ADDR                   0x20010000
/* Initial kernel object frontier limit */
#define RME_A7M_KMEM_BOOT_FRONTIER              0x20003400
/* Init process's first thread's entry point address */
#define RME_A7M_INIT_ENTRY                      0x08010001
/* Init process's first thread's stack address */
#define RME_A7M_INIT_STACK                      0x2001FFF0
/* What is the NVIC priority grouping? */
#define RME_A7M_NVIC_GROUPING                   RME_A7M_NVIC_GROUPING_P2S6
/* What is the Systick value? - 10ms per tick*/
#define RME_A7M_SYSTICK_VAL                     2160000

/* Fixed *********************************************************************/
/* The granularity of kernel memory allocation, in bytes */
#define RME_KMEM_SLOT_ORDER                     4
/* Number of MPU regions available */
#define RME_A7M_MPU_REGIONS                     8
/* What is the FPU type? */
#define RME_A7M_FPU_TYPE                        RME_A7M_FPV5_DP
    
    /* After we finish all these, we go back and populate the re-read file functionality */

}
/* End Function:A7M_RME_Gen::Chip_Hdr ****************************************/

/* Begin Function:A7M_RME_Gen::Ld_Script **************************************
Description : Generate the linker script for RME. This is toolchain-dependent.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void A7M_RME_Gen::Ld_Script(void)
{
    /*
    Prepare general ld generation...
    what IDE are we using? call that IDE's ld generation..

    if gcc, generate gcc's ld; else we generate keil's ld.
    */
}
/* End Function:A7M_RME_Gen::Ld_Script ****************************************/

/* Begin Function:A7M_RME_Gen::IDE_Proj ****************************************
Description : Generate the chip header for RME. This is toolchain-dependent.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void A7M_RME_Gen::IDE_Proj(void)
{
    /*
    Prepare general project generation...
    */
}
/* End Function:A7M_RME_Gen::IDE_Proj ****************************************/

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
    Ld_Script();
    IDE_Proj();
}
/* End Function:A7M_RME_Gen::Plat_Gen ****************************************/
}
/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
