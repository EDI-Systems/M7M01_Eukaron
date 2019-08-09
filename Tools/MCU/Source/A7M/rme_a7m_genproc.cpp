/******************************************************************************
Filename    : rme_genproc.cpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The process folder generation class for ARMv7-M. This will be called
              once for every single project folder.
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
#include "Gen/rme_genproc.hpp"

#include "A7M/rme_a7m_tc_gcc.hpp"
#include "A7M/rme_a7m_tc_armc5.hpp"
#include "A7M/rme_a7m_ide_keil.hpp"
#include "A7M/rme_a7m_ide_eclipse.hpp"
#include "A7M/rme_a7m_ide_makefile.hpp"
#include "A7M/rme_a7m_genproc.hpp"
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
#include "Gen/rme_genproc.hpp"

#include "A7M/rme_a7m_tc_gcc.hpp"
#include "A7M/rme_a7m_tc_armc5.hpp"
#include "A7M/rme_a7m_ide_keil.hpp"
#include "A7M/rme_a7m_ide_eclipse.hpp"
#include "A7M/rme_a7m_ide_makefile.hpp"
#include "A7M/rme_a7m_genproc.hpp"
#include "A7M/rme_a7m.hpp"
#undef __HDR_CLASSES__
/* End Includes **************************************************************/
namespace rme_mcu
{
/* Begin Function:A7M_Proc_Gen::Chip_Hdr **************************************
Description : Generate the chip header for processes. This is toolchain agnostic.
Input       : class Proc* Proc - The process to generate for.
Output      : None.
Return      : None.
******************************************************************************/
void A7M_Proc_Gen::Chip_Hdr(class Proc* Proc)
{
    /* Currently we do not need this file at all */
}
/* End Function:A7M_Proc_Gen::Chip_Hdr ***************************************/

/* Begin Function:A7M_Proc_Gen::Asm *******************************************
Description : Generate the assembly file for RME. This is toolchain-dependent.
Input       : class Proc* Proc - The process to generate for.
Output      : None.
Return      : None.
******************************************************************************/
void A7M_Proc_Gen::Asm(class Proc* Proc)
{
    if(*(this->Main->Format)=="keil")
        A7M_TC_Armc5::Proc_Asm(this->Main, Proc);
    else if(*(this->Main->Format)=="eclipse")
        A7M_TC_Gcc::Proc_Asm(this->Main, Proc);
    else if(*(this->Main->Format)=="makefile")
        A7M_TC_Gcc::Proc_Asm(this->Main, Proc);
    else
        throw std::runtime_error("A7M:\nThis output format is not supported.");
}
/* End Function:A7M_Proc_Gen::Asm ********************************************/

/* Begin Function:A7M_Proc_Gen::Lds *******************************************
Description : Generate the linker script for RME. This is toolchain-dependent.
Input       : class Proc* Proc - The process to generate for.
Output      : None.
Return      : None.
******************************************************************************/
void A7M_Proc_Gen::Lds(class Proc* Proc)
{
    if(*(this->Main->Format)=="keil")
        A7M_TC_Armc5::Proc_Lds(this->Main, Proc);
    else if(*(this->Main->Format)=="eclipse")
        A7M_TC_Gcc::Proc_Lds(this->Main, Proc);
    else if(*(this->Main->Format)=="makefile")
        A7M_TC_Gcc::Proc_Lds(this->Main, Proc);
    else
        throw std::runtime_error("A7M:\nThis output format is not supported.");
}
/* End Function:A7M_Proc_Gen::Lds ********************************************/

/* Begin Function:A7M_Proc_Gen::Proj ******************************************
Description : Generate the chip header for RME. This is toolchain-dependent.
Input       : class Proc* Proc - The process to generate for.
Output      : None.
Return      : None.
******************************************************************************/
void A7M_Proc_Gen::Proj(class Proc* Proc)
{
    if(*(this->Main->Format)=="keil")
        A7M_IDE_Keil::Proc_Proj(this->Main, Proc);
    else if(*(this->Main->Format)=="eclipse")
        A7M_IDE_Eclipse::Proc_Proj(this->Main, Proc);
    else if(*(this->Main->Format)=="makefile")
        A7M_IDE_Makefile::Proc_Proj(this->Main, Proc);
    else
        throw std::runtime_error("A7M:\nThis output format is not supported.");
}
/* End Function:A7M_Proc_Gen::Proj *******************************************/

/* Begin Function:A7M_Proc_Gen::Plat_Gen **************************************
Description : Generate platform-related portion of the process's project.
              This includes the chip's header itself, the linker script,
              and the organization of the project.
Input       : class Proc* Proc - The process to generate for.
Output      : None.
Return      : None.
******************************************************************************/
void A7M_Proc_Gen::Plat_Gen(class Proc* Proc)
{
    Chip_Hdr(Proc);
    Asm(Proc);
    Lds(Proc);
    Proj(Proc);
}
/* End Function:A7M_Proc_Gen::Plat_Gen ***************************************/
}
/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
