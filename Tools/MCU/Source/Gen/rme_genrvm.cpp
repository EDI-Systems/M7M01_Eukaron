/******************************************************************************
Filename    : rme_genrvm.cpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The rvm folder generation class.
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
#undef __HDR_CLASSES__
/* End Includes **************************************************************/
namespace rme_mcu
{
/* Begin Function:RVM_User::Read **********************************************
Description : Read the rvm_user.c file, which contains all the user modifiable functions.
Input       : FILE* File - The file to read from.
Output      : None.
Return      : None.
******************************************************************************/
void RVM_User::Read(FILE* File)
{
    /* Currently left empty - should construct the rvm_user.c document tree */
}
/* End Function:RVM_User::Read ***********************************************/

/* Begin Function:RVM_Gen::Include ********************************************
Description : Generate the RVM-related include section.
Input       : std::unique_ptr<class Para>& Para - The paragraph to add these to.
Output      : None.
Return      : None.
******************************************************************************/
void RVM_Gen::Include(std::unique_ptr<class Para>& Para)
{
    /* Print includes */
    Para->Add("#define __HDR_DEFS__");
    Para->Add("#include \"Platform/%s/rvm_platform_%s.h\"",
              this->Proj->Plat_Name->c_str(), this->Proj->Plat_Lower->c_str());
    Para->Add("#include \"Init/rvm_syssvc.h\"");
    Para->Add("#include \"Init/rvm_init.h\"");
    Para->Add("#undef __HDR_DEFS__");
    Para->Add("");

    Para->Add("#define __HDR_STRUCTS__");
    Para->Add("#include \"Platform/%s/rme_platform_%s.h\"",
              this->Proj->Plat_Name->c_str(), this->Proj->Plat_Lower->c_str());
    Para->Add("#include \"Init/rvm_syssvc.h\"");
    Para->Add("#include \"Init/rvm_init.h\"");
    Para->Add("#undef __HDR_STRUCTS__");
    Para->Add("");

    Para->Add("#define __HDR_PUBLIC_MEMBERS__");
    Para->Add("#include \"Platform/%s/rme_platform_%s.h\"",
              this->Proj->Plat_Name->c_str(), this->Proj->Plat_Lower->c_str());
    Para->Add("#include \"Init/rvm_syssvc.h\"");
    Para->Add("#include \"Init/rvm_init.h\"");
    Para->Add("#undef __HDR_PUBLIC_MEMBERS__");
    Para->Add("");
}
/* End Function:RVM_Gen::Include *********************************************/

/* Begin Function:RVM_Gen::Folder *********************************************
Description : Setup the folder contents for RVM.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void RVM_Gen::Folder(void)
{
    /* RME directory */
    this->Fsys->Make_Dir("M7M2_MuAmmonite");
    this->Fsys->Make_Dir("M7M2_MuAmmonite/Documents");
    this->Fsys->Make_Dir("M7M2_MuAmmonite/MAmmonite");
    this->Fsys->Make_Dir("M7M2_MuAmmonite/MAmmonite/Include");
    this->Fsys->Make_Dir("M7M2_MuAmmonite/MAmmonite/Include/Init");
    this->Fsys->Make_Dir("M7M2_MuAmmonite/MAmmonite/Include/Platform");
    this->Fsys->Make_Dir("M7M2_MuAmmonite/MAmmonite/Include/Platform/%s",this->Proj->Plat_Name->c_str());
    this->Fsys->Make_Dir("M7M2_MuAmmonite/MAmmonite/Include/Platform/%s/Chips",this->Proj->Plat_Name->c_str());
    this->Fsys->Make_Dir("M7M2_MuAmmonite/MAmmonite/Include/Platform/%s/Chips/%s",
                         this->Proj->Plat_Name->c_str(),this->Chip->Chip_Class->c_str());
    this->Fsys->Make_Dir("M7M2_MuAmmonite/MAmmonite/Init");
    this->Fsys->Make_Dir("M7M2_MuAmmonite/MAmmonite/Platform");
    this->Fsys->Make_Dir("M7M2_MuAmmonite/MAmmonite/Platform/%s",this->Proj->Plat_Name->c_str());
    this->Fsys->Make_Dir("M7M2_MuAmmonite/Project");
    this->Fsys->Make_Dir("M7M2_MuAmmonite/Project/Source");
    this->Fsys->Make_Dir("M7M2_MuAmmonite/Project/Include");

    /* Copy kernel file, kernel header, platform file, platform header, and chip headers */
    this->Fsys->Copy_File("M7M2_MuAmmonite/Documents/EN_M7M2_RT-Runtime-User-Manual.pdf");
    this->Fsys->Copy_File("M7M2_MuAmmonite/Documents/CN_M7M2_RT-Runtime-User-Manual.pdf");
    /* Currently the VMM and Posix is disabled, thus only the init is copied. */
    this->Fsys->Copy_File("M7M2_MuAmmonite/MAmmonite/Init/rvm_init.c");
    this->Fsys->Copy_File("M7M2_MuAmmonite/MAmmonite/Include/rvm.h");
    /* The toolchain specific one will be created when we are playing with toolchains */
    this->Fsys->Copy_File("M7M2_MuAmmonite/MAmmonite/Platform/%s/rvm_platform_%s.c",
                          this->Proj->Plat_Name->c_str(),this->Proj->Plat_Lower->c_str());
    this->Fsys->Copy_File("M7M2_MuAmmonite/MAmmonite/Include/Init/rvm_init.h");
    this->Fsys->Copy_File("M7M2_MuAmmonite/MAmmonite/Include/Platform/%s/rvm_platform_%s.h",
                          this->Proj->Plat_Name->c_str(),this->Proj->Plat_Lower->c_str());
    this->Fsys->Copy_File("M7M2_MuAmmonite/MAmmonite/Include/Platform/%s/Chips/%s/rvm_platform_%s.h",
                          this->Proj->Plat_Name->c_str(),this->Chip->Chip_Class->c_str(),this->Chip->Chip_Class->c_str());
}
/* End Function:RVM_Gen::Folder **********************************************/

/* Begin Function:RVM_Gen::Macro_Vect *****************************************
Description : Generate the macros for vectors.
Input       : std::unique_ptr<class Para>& Para - The paragraph to add to.
Output      : None.
Return      : None.
******************************************************************************/
void RVM_Gen::Macro_Vect(std::unique_ptr<class Para>& Para)
{
    s8_t Buf[512];
    ptr_t Obj_Cnt;
    ret_t Cap_Front;
    ptr_t Capacity;
    class Vect* Vect;

    Capacity=this->Plat->Capacity;

    /* Vector capability tables & Vectors */
    Para->Add("/* Vector endpoint capability tables */");
    Cap_Front=this->Proj->RME->Map->Vect_Cap_Front;
    Capacity=this->Plat->Capacity;
    Para->Add("/* Vector capability table capability tables */");
    for(Obj_Cnt=0;Obj_Cnt<this->Proj->RVM->Vect.size();Obj_Cnt+=Capacity)
    {
        sprintf(Buf, "RVM_BOOT_CTVECT%lld",Obj_Cnt/Capacity);
        Para->Cdef(std::make_unique<std::string>(Buf), Cap_Front++);
    }
    Para->Add("");
    Para->Add("/* Vectors */");
    for(std::unique_ptr<class Cap>& Info:this->Proj->RVM->Vect)
    {
        Vect=static_cast<class Vect*>(Info->Kobj);
        sprintf(Buf, "RVM_CAPID(RVM_BOOT_CTVECT%lld,%lld)", 
                     Vect->RVM_Capid/Capacity, Vect->RVM_Capid%Capacity);
        Para->Cdef(Vect->RVM_Macro, std::make_unique<std::string>(Buf));
    }
    Para->Add("");
    if(Cap_Front!=this->Proj->RVM->Map->Before_Cap_Front)
        throw std::runtime_error("Vector:\nInternal capability table computation failure.");
}
/* End Function:RVM_Gen::Macro_Vect ******************************************/

/* Begin Function:RVM_Gen::Macro_Captbl ***************************************
Description : Generate the macros for capability tables.
Input       : std::unique_ptr<class Para>& Para - The paragraph to add to.
Output      : None.
Return      : None.
******************************************************************************/
void RVM_Gen::Macro_Captbl(std::unique_ptr<class Para>& Para)
{
    s8_t Buf[512];
    ptr_t Obj_Cnt;
    ret_t Cap_Front;
    ptr_t Capacity;
    class Captbl* Captbl;

    Capacity=this->Plat->Capacity;

    /* Captbl capability tables & Captbls */
    Cap_Front=this->Proj->RVM->Map->Captbl_Cap_Front;
    Para->Add("/* Process capability table capability tables */");
    for(Obj_Cnt=0;Obj_Cnt<Proj->RVM->Captbl.size();Obj_Cnt+=Capacity)
    {
        sprintf(Buf, "RVM_BOOT_CTCAPTBL%lld",Obj_Cnt/Capacity);
        Para->Cdef(std::make_unique<std::string>(Buf), Cap_Front++);
    }
    Para->Add("");
    Para->Add("/* Process capability tables */");
    for(std::unique_ptr<class Cap>& Info:this->Proj->RVM->Captbl)
    {
        Captbl=static_cast<class Captbl*>(Info->Kobj);
        sprintf(Buf, "RVM_CAPID(RVM_BOOT_CTCAPTBL%lld,%lld)", 
                     Captbl->RVM_Capid/Capacity, Captbl->RVM_Capid%Capacity);
        Para->Cdef(Captbl->RVM_Macro, std::make_unique<std::string>(Buf));
    }
    Para->Add("");
    if(Cap_Front!=this->Proj->RVM->Map->Pgtbl_Cap_Front)
        throw std::runtime_error("Capability table:\nInternal capability table computation failure.");
}
/* End Function:RVM_Gen::Macro_Captbl ****************************************/

/* Begin Function:RVM_Gen::Macro_Pgtbl ****************************************
Description : Generate the macros for page tables.
Input       : std::unique_ptr<class Para>& Para - The paragraph to add to.
Output      : None.
Return      : None.
******************************************************************************/
void RVM_Gen::Macro_Pgtbl(std::unique_ptr<class Para>& Para)
{
    s8_t Buf[512];
    ptr_t Obj_Cnt;
    ret_t Cap_Front;
    ptr_t Capacity;
    class Pgtbl* Pgtbl;

    Capacity=this->Plat->Capacity;

    /* Pgtbl capability tables & Pgtbls */
    Cap_Front=this->Proj->RVM->Map->Pgtbl_Cap_Front;
    Para->Add("/* Process page table capability tables */");
    for(Obj_Cnt=0;Obj_Cnt<Proj->RVM->Pgtbl.size();Obj_Cnt+=Capacity)
    {
        sprintf(Buf, "RVM_BOOT_CTPGTBL%lld",Obj_Cnt/Capacity);
        Para->Cdef(std::make_unique<std::string>(Buf), Cap_Front++);
    }
    Para->Add("");
    Para->Add("/* Process page tables */");
    for(std::unique_ptr<class Cap>& Info:this->Proj->RVM->Pgtbl)
    {
        Pgtbl=static_cast<class Pgtbl*>(Info->Kobj);
        sprintf(Buf, "RVM_CAPID(RVM_BOOT_CTPGTBL%lld,%lld)", 
                     Pgtbl->RVM_Capid/Capacity, Pgtbl->RVM_Capid%Capacity);
        Para->Cdef(Pgtbl->RVM_Macro, std::make_unique<std::string>(Buf));
    }
    Para->Add("");
    if(Cap_Front!=this->Proj->RVM->Map->Proc_Cap_Front)
        throw std::runtime_error("Page table:\nInternal capability table computation failure.");
}
/* End Function:RVM_Gen::Macro_Pgtbl *****************************************/

/* Begin Function:RVM_Gen::Macro_Proc ****************************************
Description : Generate the macros for processes.
Input       : std::unique_ptr<class Para>& Para - The paragraph to add to.
Output      : None.
Return      : None.
******************************************************************************/
void RVM_Gen::Macro_Proc(std::unique_ptr<class Para>& Para)
{
    s8_t Buf[512];
    ptr_t Obj_Cnt;
    ret_t Cap_Front;
    ptr_t Capacity;
    class Proc* Proc;

    Capacity=this->Plat->Capacity;

    /* Process capability tables & Processes */
    Cap_Front=this->Proj->RVM->Map->Proc_Cap_Front;
    Para->Add("/* Process capability tables */");
    for(Obj_Cnt=0;Obj_Cnt<this->Proj->RVM->Proc.size();Obj_Cnt+=Capacity)
    {
        sprintf(Buf, "RVM_BOOT_CTPROC%lld",Obj_Cnt/Capacity);
        Para->Cdef(std::make_unique<std::string>(Buf), Cap_Front++);
    }
    Para->Add("");
    Para->Add("/* Processes */");
    for(std::unique_ptr<class Cap>& Info:this->Proj->RVM->Proc)
    {
        Proc=static_cast<class Proc*>(Info->Kobj);
        sprintf(Buf, "RVM_CAPID(RVM_BOOT_CTPROC%lld,%lld)",
                     Proc->RVM_Capid/Capacity, Proc->RVM_Capid%Capacity);
        Para->Cdef(Proc->RVM_Macro, std::make_unique<std::string>(Buf));
    }
    Para->Add("");
    if(Cap_Front!=this->Proj->RVM->Map->Thd_Cap_Front)
        throw std::runtime_error("Process:\nInternal capability table computation failure.");
}
/* End Function:RVM_Gen::Macro_Proc ******************************************/

/* Begin Function:RVM_Gen::Macro_Thd ****************************************
Description : Generate the macros for threads.
Input       : std::unique_ptr<class Para>& Para - The paragraph to add to.
Output      : None.
Return      : None.
******************************************************************************/
void RVM_Gen::Macro_Thd(std::unique_ptr<class Para>& Para)
{
    s8_t Buf[512];
    ptr_t Obj_Cnt;
    ret_t Cap_Front;
    ptr_t Capacity;
    class Thd* Thd;

    Capacity=this->Plat->Capacity;

    /* Thread capability tables & Threads */
    Cap_Front=this->Proj->RVM->Map->Thd_Cap_Front;
    Para->Add("/* Thread capability tables */");
    for(Obj_Cnt=0;Obj_Cnt<this->Proj->RVM->Thd.size();Obj_Cnt+=Capacity)
    {
        sprintf(Buf, "RVM_BOOT_CTTHD%lld",Obj_Cnt/Capacity);
        Para->Cdef(std::make_unique<std::string>(Buf), Cap_Front++);
    }
    Para->Add("");
    Para->Add("/* Threads */");
    for(std::unique_ptr<class Cap>& Info:this->Proj->RVM->Thd)
    {
        Thd=static_cast<class Thd*>(Info->Kobj);
        sprintf(Buf, "RVM_CAPID(RVM_BOOT_CTTHD%lld,%lld)",
                     Thd->RVM_Capid/Capacity, Thd->RVM_Capid%Capacity);
        Para->Cdef(Thd->RVM_Macro, std::make_unique<std::string>(Buf));
    }
    Para->Add("");
    if(Cap_Front!=Proj->RVM->Map->Inv_Cap_Front)
        throw std::runtime_error("Thread:\nInternal capability table computation failure.");
}
/* End Function:RVM_Gen::Macro_Thd *******************************************/

/* Begin Function:RVM_Gen::Macro_Inv ****************************************
Description : Generate the macros for invocations.
Input       : std::unique_ptr<class Para>& Para - The paragraph to add to.
Output      : None.
Return      : None.
******************************************************************************/
void RVM_Gen::Macro_Inv(std::unique_ptr<class Para>& Para)
{
    s8_t Buf[512];
    ptr_t Obj_Cnt;
    ret_t Cap_Front;
    ptr_t Capacity;
    class Inv* Inv;

    Capacity=this->Plat->Capacity;

    /* Invocation capability tables & Invocations */
    Cap_Front=this->Proj->RVM->Map->Inv_Cap_Front;
    Para->Add("/* Invocation capability tables */");
    for(Obj_Cnt=0;Obj_Cnt<Proj->RVM->Inv.size();Obj_Cnt+=Capacity)
    {
        sprintf(Buf, "RVM_BOOT_CTINV%lld",Obj_Cnt/Capacity);
        Para->Cdef(std::make_unique<std::string>(Buf), Cap_Front++);
    }
    Para->Add("");
    Para->Add("/* Invocations */");
    for(std::unique_ptr<class Cap>& Info:this->Proj->RVM->Inv)
    {
        Inv=static_cast<class Inv*>(Info->Kobj);
        sprintf(Buf, "RVM_CAPID(RVM_BOOT_CTINV%lld,%lld)",
                     Inv->RVM_Capid/Capacity, Inv->RVM_Capid%Capacity);
        Para->Cdef(Inv->RVM_Macro, std::make_unique<std::string>(Buf));
    }
    Para->Add("");
    if(Cap_Front!=Proj->RVM->Map->Recv_Cap_Front)
        throw std::runtime_error("Invocation:\nInternal capability table computation failure.");
}
/* End Function:RVM_Gen::Macro_Inv *******************************************/

/* Begin Function:RVM_Gen::Macro_Recv ***************************************
Description : Generate the macros for receive endpoints.
Input       : std::unique_ptr<class Para>& Para - The paragraph to add to.
Output      : None.
Return      : None.
******************************************************************************/
void RVM_Gen::Macro_Recv(std::unique_ptr<class Para>& Para)
{
    s8_t Buf[512];
    ptr_t Obj_Cnt;
    ret_t Cap_Front;
    ptr_t Capacity;
    class Recv* Recv;

    Capacity=this->Plat->Capacity;

    /* Receive endpoint capability tables & Receive endpoints */
    Cap_Front=this->Proj->RVM->Map->Recv_Cap_Front;
    Para->Add("/* Receive endpoint capability tables */");
    for(Obj_Cnt=0;Obj_Cnt<Proj->RVM->Recv.size();Obj_Cnt+=Capacity)
    {
        sprintf(Buf, "RVM_BOOT_CTRECV%lld",Obj_Cnt/Capacity);
        Para->Cdef(std::make_unique<std::string>(Buf), Cap_Front++);
    }
    Para->Add("");
    Para->Add("/* Receive endpoints */");
    for(std::unique_ptr<class Cap>& Info:this->Proj->RVM->Recv)
    {
        Recv=static_cast<class Recv*>(Info->Kobj);
        sprintf(Buf, "RVM_CAPID(RVM_BOOT_CTRECV%lld,%lld)",
                     Recv->RVM_Capid/Capacity, Recv->RVM_Capid%Capacity);
        Para->Cdef(Recv->RVM_Macro, std::make_unique<std::string>(Buf));
    }
    Para->Add("");
    if(Cap_Front!=Proj->RVM->Map->After_Cap_Front)
        throw std::runtime_error("Receive endpoint:\nInternal capability table computation failure.");
}
/* End Function:RVM_Gen::Macro_Recv ******************************************/

/* Begin Function:RVM_Gen::Boot_Hdr *******************************************
Description : Generate the rvm_boot.h.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void RVM_Gen::Boot_Hdr(void)
{
    FILE* File;
    std::unique_ptr<class Doc> Doc;
    std::unique_ptr<class Para> Para;

    Doc=std::make_unique<class Doc>();
    Doc->Csrc_Desc("rvm_boot.h", "The boot-time initialization file header.");

    Para=std::make_unique<class Para>("Doc:Defines");
    Para->Add("/* Defines *******************************************************************/");
    Macro_Vect(Para);
    /* RVM internal resource creation goes here - not created by the script */
    Macro_Captbl(Para);
    Macro_Pgtbl(Para);
    Macro_Proc(Para);
    Macro_Thd(Para);
    Macro_Inv(Para);
    Macro_Recv(Para);
    
    /* Extra capability table frontier */
    Para->Add("/* Capability table frontier */");
    Para->Cdef(std::make_unique<std::string>("RVM_BOOT_CAP_FRONTIER"),
               (ret_t)(this->Proj->RVM->Map->After_Cap_Front));
    /* Extra kernel memory frontier */
    Para->Add("/* Kernel memory frontier */");
    Para->Cdef(std::make_unique<std::string>("RVM_BOOT_KMEM_FRONTIER"),
               this->Proj->RVM->Map->After_Kmem_Front);

    /* Finish file generation */
    Para->Add("/* End Defines ***************************************************************/");
    Doc->Add(std::move(Para));
    Doc->Csrc_Foot();

    /* Generate rme_boot.h */
    File=this->Fsys->Open_File("M7M2_MuAmmonite/Project/Include/rvm_boot.h");
    Doc->Write(File);
    fclose(File);
}
/* End Function:RVM_Gen::Boot_Hdr ********************************************/

/* Begin Function:RVM_Gen::Cons_Pgtbl *****************************************
Description : Construct the page table for RVM. This will produce the desired final
              page table tree, and is recursive.
Input       : std::unique_ptr<class Para>& Para - The paragraph to add these to.
              class Pgtbl* Pgtbl - The page table structure.
Output      : None.
Return      : None.
******************************************************************************/
void RVM_Gen::Cons_Pgtbl(std::unique_ptr<class Para>& Para, class Pgtbl* Pgtbl)
{
    ptr_t Count;
    class Pgtbl* Child;
    class Pgtbl_Info* Child;

    /* Construct whatever page table to this page table */
    for(Count=0;Count<POW2(Pgtbl->Num_Order);Count++)
    {
        Child=Pgtbl->Pgdir[Count].get();
        if(Child==nullptr)
            continue;
        
        Para->Add("    RVM_ASSERT(RVM_Pgtbl_Cons(%s, 0x%llX, %s, %s)==0);",
                  Pgtbl->RVM_Macro->c_str(), Count, Child->RVM_Macro->c_str(), "RVM_PGTBL_ALL_PERM");

        /* Recursively call this for all the page tables */
        Cons_Pgtbl(Para, Child);
    }
}
/* End Function:RVM_Gen::Cons_Pgtbl ******************************************/

/* Begin Function:RVM_Gen::Map_Pgtbl ******************************************
Description : Map pages into a page table. This is not recursive.
Input       : std::unique_ptr<class Para>& Para - The paragraph to add these to.
              class Pgtbl* Pgtbl - The page table structure.
              ptr_t Init_Size_Ord - The initial page table's number order.
Output      : None.
Return      : None.
******************************************************************************/
void RVM_Gen::Map_Pgtbl(std::unique_ptr<class Para>& Para, class Pgtbl* Pgtbl, ptr_t Init_Size_Ord)
{
    ptr_t Count;
    ptr_t Attr;
    ptr_t Pos_Src;
    ptr_t Index;
    ptr_t Page_Start;
    ptr_t Page_Size;
    ptr_t Page_Num;
    ptr_t Init_Size;
    std::unique_ptr<std::string> Flags;

    Page_Size=POW2(Pgtbl->Size_Order);
    Page_Num=POW2(Pgtbl->Num_Order);
    Init_Size=POW2(Init_Size_Ord);

    /* Map whatever pages into this page table */
    for(Count=0;Count<Page_Num;Count++)
    {
        Attr=Pgtbl->Page[Count];
        if(Attr==0)
            continue;

        /* Compute flags */
        *Flags="";

        if((Attr&MEM_READ)!=0)
            *Flags+="RVM_PGTBL_READ|";
        if((Attr&MEM_WRITE)!=0)
            *Flags+="RVM_PGTBL_WRITE|";
        if((Attr&MEM_EXECUTE)!=0)
            *Flags+="RVM_PGTBL_EXECUTE|";
        if((Attr&MEM_CACHEABLE)!=0)
            *Flags+="RVM_PGTBL_CACHEABLE|";
        if((Attr&MEM_BUFFERABLE)!=0)
            *Flags+="RVM_PGTBL_BUFFERABLE|";
        if((Attr&MEM_STATIC)!=0)
            *Flags+="RVM_PGTBL_STATIC|";

        Flags->pop_back();

        /* Compute Pos_Src and Index */
        Page_Start=Pgtbl->Start_Addr+Count*Page_Size;
        Pos_Src=Page_Start/Init_Size;
        Index=(Page_Start-Pos_Src*Init_Size)/Page_Size;

        Para->Add("    RVM_ASSERT(RVM_Pgtbl_Add(%s, 0x%llX, \\\n"
                  "                             %s, \\\n"
                  "                             %s, 0x%llX, 0x%llX)==0);",
                  Pgtbl->RVM_Macro->c_str(), Count, Flags->c_str(), "RVM_BOOT_PGTBL", Pos_Src, Index);
    }
}
/* End Function:RVM_Gen::Map_Pgtbl *******************************************/

/* Begin Function:RVM_Gen::Init_Pgtbl *****************************************
Description : Initialize page tables.
Input       : std::unique_ptr<class Doc>& Doc - The document to add this to.
Output      : None.
Return      : None.
******************************************************************************/
void RVM_Gen::Init_Pgtbl(std::unique_ptr<class Doc>& Doc)
{
    class Pgtbl* Pgtbl;
    std::unique_ptr<class Para> Para;
    std::unique_ptr<std::string> Line;
    std::vector<std::unique_ptr<std::string>> Input;
    std::vector<std::unique_ptr<std::string>> Output;

    Para=std::make_unique<class Para>("Func:RVM_Boot_Pgtbl_Init");

    /* Page table initialization */
    Para->Cfunc_Desc("RVM_Boot_Pgtbl_Init",
                     "Initialize the page tables of all processes.", Input, Output, "None.");
    Para->Add("void RVM_Boot_Pgtbl_Init(void)");
    Para->Add("{");

    /* Do page table construction first */
    for(std::unique_ptr<class Proc>& Proc:this->Proj->Proc)
    {
        Para->Add("    /* Constructing page tables for process: %s */",Proc->Name->c_str());
        Cons_Pgtbl(Para,Proc->Pgtbl.get());
        Para->Add("");
    }
    
    /* Then do the mapping for all page tables */
    Para->Add("    /* Mapping pages into page tables */");
    for(std::unique_ptr<class Cap>& Info:this->Proj->RVM->Pgtbl)
    {
        Pgtbl=static_cast<class Pgtbl*>(Info->Kobj);
        Map_Pgtbl(Para, Pgtbl, this->Plat->Word_Bits-this->Plat->Init_Num_Ord);
        Para->Add("");
    }

    Para->Add("}");
    Para->Cfunc_Foot("RVM_Boot_Pgtbl_Init");
    Doc->Add(std::move(Para));
}
/* End Function:RVM_Gen::Init_Pgtbl ******************************************/

/* Begin Function:RVM_Gen::Boot_Src *******************************************
Description : Generate the rvm_boot.c.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void RVM_Gen::Boot_Src(void)
{
    s8_t* Buf;
    FILE* File;
    ptr_t Obj_Cnt;
    struct RVM_Cap_Info* Info;
    struct Pgtbl_Info* Pgtbl;
    struct Proc_Info* Proc;
    struct Thd_Info* Thd;
    struct Inv_Info* Inv;
    struct Port_Info* Port;
    struct Recv_Info* Recv;
    struct Send_Info* Send;
    struct Vect_Info* Vect;
    ptr_t Cap_Front;
    ptr_t Capacity;
    ptr_t Captbl_Size;

    Buf=Malloc(4096);

    /* Generate rvm_boot.c */
    sprintf(Buf, "%s/M7M2_MuAmmonite/Project/Source/rvm_boot.c", Output_Path);
    File=fopen(Buf, "wb");
    if(File==0)
        EXIT_FAIL("rvm_boot.c open failed.");
    Write_Src_Desc(File, "rvm_boot.c", "The boot-time initialization file.");

    /* Print all header includes */
    fprintf(File, "/* Includes ******************************************************************/\n");
    Print_RVM_Inc(File, Proj);
    fprintf(File, "#include \"rvm_boot.h\"\n");
    fprintf(File, "/* End Includes **************************************************************/\n\n");

    /* Print all global variables and prototypes */
    fprintf(File, "/* Private C Function Prototypes *********************************************/\n");
    fprintf(File, "/* Kernel object creation */\n");
    fprintf(File, "static void RVM_Boot_Captbl_Crt(void);\n");
    fprintf(File, "static void RVM_Boot_Pgtbl_Crt(void);\n");
    fprintf(File, "static void RVM_Boot_Proc_Crt(void);\n");
    fprintf(File, "static void RVM_Boot_Inv_Crt(void);\n");
    fprintf(File, "static void RVM_Boot_Recv_Crt(void);\n\n");
    fprintf(File, "/* Kernel object initialization */\n");
    fprintf(File, "static void RVM_Boot_Captbl_Init(void);\n");
    fprintf(File, "static void RVM_Boot_Pgtbl_Init(void);\n");
    fprintf(File, "static void RVM_Boot_Proc_Init(void);\n");
    fprintf(File, "static void RVM_Boot_Inv_Init(void);\n");
    fprintf(File, "static void RVM_Boot_Recv_Init(void);\n");
    fprintf(File, "/* End Private C Function Prototypes *****************************************/\n\n");
    fprintf(File, "/* Public C Function Prototypes **********************************************/\n");
    fprintf(File, "void RVM_Boot_Kobj_Crt(void);\n");
    fprintf(File, "void RVM_Boot_Kobj_Init(void);\n");
    fprintf(File, "/* End Public C Function Prototypes ******************************************/\n\n");

    /* Capability table creation */
    Write_Func_Desc(File, "RVM_Boot_Captbl_Crt");
    fprintf(File, "Description : Create all capability tables at boot-time.\n");
    Write_Func_None(File);
    fprintf(File, "void RVM_Boot_Captbl_Crt(void)\n");
    fprintf(File, "{\n");
    fprintf(File, "    rme_ptr_t Cur_Addr;\n\n");
    fprintf(File, "    Cur_Addr==0x%llX;\n\n", Proj->RVM.Map.Captbl_Kmem_Front);
    fprintf(File, "    /* Create all the capability table capability tables first */\n");
    for(Obj_Cnt=0;Obj_Cnt<Proj->RVM.Captbl_Front;Obj_Cnt+=Capacity)
    {
        if(Proj->RVM.Captbl_Front>=(Obj_Cnt+1)*Capacity)
            Captbl_Size=Capacity;
        else
            Captbl_Size=Proj->RVM.Captbl_Front%Capacity;

        fprintf(File, "    RVM_ASSERT(RVM_Captbl_Crt(RVM_BOOT_CAPTBL, RVM_BOOT_INIT_KMEM, RVM_BOOT_CTCAPTBL%lld, Cur_Addr, %lld)==0);\n", 
                Obj_Cnt/Capacity,Captbl_Size);
        fprintf(File, "    Cur_Addr+=RVM_KOTBL_ROUND(RVM_CAPTBL_SIZE(%lld));\n",Captbl_Size);
    }
    fprintf(File, "\n    /* Then the capability tables themselves */\n");
    for(EACH(struct RVM_Cap_Info*,Info,Proj->RVM.Captbl))
    {
        Proc=(struct Proc_Info*)(Info->Cap);

        fprintf(File, "    RVM_ASSERT(RVM_Captbl_Crt(RVM_BOOT_CTCAPTBL%lld, RVM_BOOT_INIT_KMEM, %lld, Cur_Addr, %lld)==0);\n",
                Proc->Captbl_Cap.RVM_Capid/Capacity, Proc->Captbl_Cap.RVM_Capid%Capacity, Proc->Captbl_Front+Proc->Extra_Captbl);
        fprintf(File, "    Cur_Addr+=RME_KOTBL_ROUND(RVM_CAPTBL_SIZE(%lld));\n", Proc->Captbl_Front+Proc->Extra_Captbl);
    }

    fprintf(File, "\n    RME_ASSERT(Cur_Addr==0x%llX);\n", Proj->RVM.Map.Pgtbl_Kmem_Front);
    fprintf(File, "}\n");
    Write_Func_Footer(File, "RVM_Boot_Captbl_Crt");

    /* Page table creation */
    Write_Func_Desc(File, "RVM_Boot_Pgtbl_Crt");
    fprintf(File, "Description : Create all page tables at boot-time.\n");
    Write_Func_None(File);
    fprintf(File, "void RVM_Boot_Pgtbl_Crt(void)\n");
    fprintf(File, "{\n");
    fprintf(File, "    rme_ptr_t Cur_Addr;\n\n");
    fprintf(File, "    Cur_Addr==0x%llX;\n\n", Proj->RVM.Map.Pgtbl_Kmem_Front);
    fprintf(File, "    /* Create all the page tables capability tables first */\n");
    for(Obj_Cnt=0;Obj_Cnt<Proj->RVM.Pgtbl_Front;Obj_Cnt+=Capacity)
    {
        if(Proj->RVM.Pgtbl_Front>=(Obj_Cnt+1)*Capacity)
            Captbl_Size=Capacity;
        else
            Captbl_Size=Proj->RVM.Pgtbl_Front%Capacity;

        fprintf(File, "    RVM_ASSERT(RVM_Captbl_Crt(RVM_BOOT_CAPTBL, RVM_BOOT_INIT_KMEM, RVM_BOOT_CTPGTBL%lld, Cur_Addr, %lld)==0);\n", 
                Obj_Cnt/Capacity,Captbl_Size);
        fprintf(File, "    Cur_Addr+=RVM_KOTBL_ROUND(RVM_CAPTBL_SIZE(%lld));\n",Captbl_Size);
    }
    fprintf(File, "\n    /* Then the page tables themselves */\n");
    for(EACH(struct RVM_Cap_Info*,Info,Proj->RVM.Pgtbl))
    {
        Pgtbl=Info->Cap;

        fprintf(File, "    RVM_ASSERT(RVM_Pgtbl_Crt(RVM_BOOT_CTPGTBL%lld, RVM_BOOT_INIT_KMEM, %lld, Cur_Addr, 0x%llX, %lld, %lld, %lld)==0);\n",
                Proc->Captbl_Cap.RVM_Capid/Capacity, Proc->Captbl_Cap.RVM_Capid%Capacity,
                Pgtbl->Start_Addr,(ptr_t)(Pgtbl->Is_Top!=0),Pgtbl->Size_Order, Pgtbl->Num_Order);

        if(Pgtbl->Is_Top!=0)
            fprintf(File, "    Cur_Addr+=RME_KOTBL_ROUND(RVM_PGTBL_SIZE_TOP(%lld));\n", Pgtbl->Num_Order);
        else
            fprintf(File, "    Cur_Addr+=RME_KOTBL_ROUND(RVM_PGTBL_SIZE_NOM(%lld));\n", Pgtbl->Num_Order);
    }

    fprintf(File, "\n    RME_ASSERT(Cur_Addr==0x%llX);\n", Proj->RVM.Map.Proc_Kmem_Front);
    fprintf(File, "}\n");
    Write_Func_Footer(File, "RVM_Boot_Pgtbl_Crt");

    /* Process creation */
    Write_Func_Desc(File, "RVM_Boot_Proc_Crt");
    fprintf(File, "Description : Create all processes at boot-time.\n");
    Write_Func_None(File);
    fprintf(File, "void RVM_Boot_Proc_Crt(void)\n");
    fprintf(File, "{\n");
    fprintf(File, "    rme_ptr_t Cur_Addr;\n\n");
    fprintf(File, "    Cur_Addr=0x%llX;\n\n", Proj->RVM.Map.Proc_Kmem_Front);
    fprintf(File, "    /* Create all the process capability tables first */\n");
    for(Obj_Cnt=0;Obj_Cnt<Proj->RVM.Proc_Front;Obj_Cnt+=Capacity)
    {
        if(Proj->RVM.Proc_Front>=(Obj_Cnt+1)*Capacity)
            Captbl_Size=Capacity;
        else
            Captbl_Size=Proj->RVM.Proc_Front%Capacity;

        fprintf(File, "    RVM_ASSERT(RVM_Captbl_Crt(RVM_BOOT_CAPTBL, RVM_BOOT_INIT_KMEM, RVM_BOOT_CTPROC%lld, Cur_Addr, %lld)==0);\n", 
                Obj_Cnt/Capacity,Captbl_Size);
        fprintf(File, "    Cur_Addr+=RVM_KOTBL_ROUND(RVM_CAPTBL_SIZE(%lld));\n",Captbl_Size);
    }
    fprintf(File, "\n    /* Then the processes themselves */\n");
    for(EACH(struct RVM_Cap_Info*,Info,Proj->RVM.Proc))
    {
        Proc=(struct Proc_Info*)(Info->Cap);
        fprintf(File, "    RVM_ASSERT(RVM_Proc_Crt(RVM_BOOT_CTPROC%lld, RVM_BOOT_INIT_KMEM, %lld, %s, %s, Cur_Addr)==0);\n",
                Proc->Proc_Cap.RVM_Capid/Capacity, Proc->Proc_Cap.RVM_Capid%Capacity, Proc->Proc_Cap.RVM_Macro, Proc->Pgtbl->Cap.RVM_Macro);
        fprintf(File, "    Cur_Addr+=RME_KOTBL_ROUND(RVM_PROC_SIZE);\n");
    }
    fprintf(File, "\n    RME_ASSERT(Cur_Addr==0x%llX);\n", Proj->RVM.Map.Thd_Kmem_Front);
    fprintf(File, "}\n");
    Write_Func_Footer(File, "RVM_Boot_Proc_Crt");

    /* Thread creation */
    Write_Func_Desc(File, "RVM_Boot_Thd_Crt");
    fprintf(File, "Description : Create all threads at boot-time.\n");
    Write_Func_None(File);
    fprintf(File, "void RVM_Boot_Thd_Init(void)\n");
    fprintf(File, "{\n");
    fprintf(File, "    rme_ptr_t Cur_Addr;\n\n");
    fprintf(File, "    Cur_Addr=0x%llX;\n\n", Proj->RVM.Map.Thd_Kmem_Front);
    fprintf(File, "    /* Create all the thread capability tables first */\n");
    for(Obj_Cnt=0;Obj_Cnt<Proj->RVM.Thd_Front;Obj_Cnt+=Capacity)
    {
        if(Proj->RVM.Thd_Front>=(Obj_Cnt+1)*Capacity)
            Captbl_Size=Capacity;
        else
            Captbl_Size=Proj->RVM.Thd_Front%Capacity;

        fprintf(File, "    RVM_ASSERT(RVM_Captbl_Crt(RVM_BOOT_CAPTBL, RVM_BOOT_INIT_KMEM, RVM_BOOT_CTTHD%lld, Cur_Addr, %lld)==0);\n", 
                Obj_Cnt/Capacity,Captbl_Size);
        fprintf(File, "    Cur_Addr+=RVM_KOTBL_ROUND(RVM_CAPTBL_SIZE(%lld));\n",Captbl_Size);
    }
    fprintf(File, "\n    /* Then the threads themselves */\n");
    for(EACH(struct RVM_Cap_Info*,Info,Proj->RVM.Thd))
    {
        Thd=(struct Thd_Info*)(Info->Cap);
        Proc=Info->Proc;
        fprintf(File, "    RVM_ASSERT(RVM_Thd_Crt(RVM_BOOT_CTTHD%lld, RVM_BOOT_INIT_KMEM, %lld, %s, %lld, Cur_Addr)==0);\n",
                Thd->Cap.RVM_Capid/Capacity, Thd->Cap.RVM_Capid%Capacity, Proc->Proc_Cap.RVM_Macro, Thd->Priority);
        fprintf(File, "    Cur_Addr+=RME_KOTBL_ROUND(RVM_THD_SIZE);\n");
    }
    fprintf(File, "\n    RME_ASSERT(Cur_Addr==0x%llX);\n", Proj->RVM.Map.Inv_Kmem_Front);
    fprintf(File, "}\n");
    Write_Func_Footer(File, "RVM_Boot_Thd_Crt");

    /* Invocation creation */
    Write_Func_Desc(File, "RVM_Boot_Inv_Crt");
    fprintf(File, "Description : Create all invocations at boot-time.\n");
    Write_Func_None(File);
    fprintf(File, "void RVM_Boot_Inv_Crt(void)\n");
    fprintf(File, "{\n");
    fprintf(File, "    rme_ptr_t Cur_Addr;\n\n");
    fprintf(File, "    Cur_Addr=0x%llX;\n\n", Proj->RVM.Map.Inv_Kmem_Front);
    fprintf(File, "    /* Create all the invocation capability tables first */\n");
    for(Obj_Cnt=0;Obj_Cnt<Proj->RVM.Inv_Front;Obj_Cnt+=Capacity)
    {
        if(Proj->RVM.Inv_Front>=(Obj_Cnt+1)*Capacity)
            Captbl_Size=Capacity;
        else
            Captbl_Size=Proj->RVM.Inv_Front%Capacity;

        fprintf(File, "    RVM_ASSERT(RVM_Captbl_Crt(RVM_BOOT_CAPTBL, RVM_BOOT_INIT_KMEM, RVM_BOOT_CTINV%lld, Cur_Addr, %lld)==0);\n", 
                Obj_Cnt/Capacity,Captbl_Size);
        fprintf(File, "    Cur_Addr+=RVM_KOTBL_ROUND(RVM_CAPTBL_SIZE(%lld));\n",Captbl_Size);
    }
    fprintf(File, "\n    /* Then the invocations themselves */\n");
    for(EACH(struct RVM_Cap_Info*,Info,Proj->RVM.Inv))
    {
        Inv=(struct Inv_Info*)(Info->Cap);
        Proc=Info->Proc;
        fprintf(File, "    RVM_ASSERT(RVM_Inv_Crt(RVM_BOOT_CTINV%lld, RVM_BOOT_INIT_KMEM, %lld, %s, Cur_Addr)==0);\n",
                Inv->Cap.RVM_Capid/Capacity, Inv->Cap.RVM_Capid%Capacity, Proc->Proc_Cap.RVM_Macro);
        fprintf(File, "    Cur_Addr+=RME_KOTBL_ROUND(RVM_INV_SIZE);\n");
    }
    fprintf(File, "\n    RME_ASSERT(Cur_Addr==0x%llX);\n", Proj->RVM.Map.Recv_Kmem_Front);
    fprintf(File, "}\n");
    Write_Func_Footer(File, "RVM_Boot_Inv_Crt");

    /* Receive endpoint creation */
    Write_Func_Desc(File, "RVM_Boot_Recv_Crt");
    fprintf(File, "Description : Create all receive endpoints at boot-time.\n");
    Write_Func_None(File);
    fprintf(File, "void RVM_Boot_Recv_Crt(void)\n");
    fprintf(File, "{\n");
    fprintf(File, "    rme_ptr_t Cur_Addr;\n\n");
    fprintf(File, "    Cur_Addr=0x%llX;\n\n", Proj->RVM.Map.Recv_Kmem_Front);
    fprintf(File, "    /* Create all the receive endpoint capability tables first */\n");
    for(Obj_Cnt=0;Obj_Cnt<Proj->RVM.Recv_Front;Obj_Cnt+=Capacity)
    {
        if(Proj->RVM.Recv_Front>=(Obj_Cnt+1)*Capacity)
            Captbl_Size=Capacity;
        else
            Captbl_Size=Proj->RVM.Recv_Front%Capacity;

        fprintf(File, "    RVM_ASSERT(RVM_Captbl_Crt(RVM_BOOT_CAPTBL, RVM_BOOT_INIT_KMEM, RVM_BOOT_CTRECV%lld, Cur_Addr, %lld)==0);\n", 
                Obj_Cnt/Capacity,Captbl_Size);
        fprintf(File, "    Cur_Addr+=RVM_KOTBL_ROUND(RVM_CAPTBL_SIZE(%lld));\n",Captbl_Size);
    }
    fprintf(File, "\n    /* Then the receive endpoints themselves */\n");
    for(EACH(struct RVM_Cap_Info*,Info,Proj->RVM.Recv))
    {
        Recv=(struct Recv_Info*)(Info->Cap);
        fprintf(File, "    RVM_ASSERT(RVM_Sig_Crt(RVM_BOOT_CTRECV%lld, RVM_BOOT_INIT_KMEM, %lld, Cur_Addr)==0);\n",
                Recv->Cap.RVM_Capid/Capacity, Recv->Cap.RVM_Capid%Capacity);
        fprintf(File, "    Cur_Addr+=RME_KOTBL_ROUND(RVM_SIG_SIZE);\n");
    }
    fprintf(File, "\n    RME_ASSERT(Cur_Addr==0x%llX);\n", Proj->RVM.Map.After_Kmem_Front);
    fprintf(File, "}\n");
    Write_Func_Footer(File, "RVM_Boot_Recv_Crt");

    /* Main creation function */
    Write_Func_Desc(File, "RVM_Boot_Kobj_Crt");
    fprintf(File, "Description : Create all kernel objects at boot-time.\n");
    Write_Func_None(File);
    fprintf(File, "void RVM_Boot_Kobj_Crt(void)\n");
    fprintf(File, "{\n");
    fprintf(File, "    RVM_Boot_Captbl_Crt();\n");
    fprintf(File, "    RVM_Boot_Pgtbl_Crt();\n");
    fprintf(File, "    RVM_Boot_Proc_Crt();\n");
    fprintf(File, "    RVM_Boot_Thd_Crt();\n");
    fprintf(File, "    RVM_Boot_Inv_Crt();\n");
    fprintf(File, "    RVM_Boot_Recv_Crt();\n");
    fprintf(File, "}\n");
    Write_Func_Footer(File, "RVM_Boot_Kobj_Crt");
 
    /* Capability table initialization */
    Write_Func_Desc(File, "RVM_Boot_Captbl_Init");
    fprintf(File, "Description : Initialize the capability tables of all processes.\n");
    Write_Func_None(File);
    fprintf(File, "void RVM_Boot_Captbl_Init(void)\n");
    fprintf(File, "{");
    for(EACH(struct Proc_Info*,Proc,Proj->Proc))
    {
        fprintf(File, "\n    /* Initializing captbl for process: %s */\n", Proc->Name);

        /* Ports */
        fprintf(File, "    /* Ports */\n");
        for(EACH(struct Port_Info*,Port,Proc->Port))
        {
            fprintf(File, "    RVM_ASSERT(RVM_Captbl_Add(%s, %lld, RVM_CTINV%lld, %lld, %s)==0);\n",
                    Proc->Captbl_Cap.RVM_Macro, Port->Cap.Loc_Capid, Port->Cap.RVM_Capid/Capacity, Port->Cap.RVM_Capid%Capacity,
                    "RME_INV_FLAG_ACT");
        }

        /* Receive endpoints */
        fprintf(File, "    /* Receive endpoints */\n");
        for(EACH(struct Recv_Info*,Recv,Proc->Recv))
        {
            fprintf(File, "    RVM_ASSERT(RVM_Captbl_Add(%s, %lld, RVM_CTRECV%lld, %lld, %s)==0);\n",
                    Proc->Captbl_Cap.RVM_Macro, Recv->Cap.Loc_Capid, Recv->Cap.RVM_Capid/Capacity, Recv->Cap.RVM_Capid%Capacity,
                    "RME_SIG_FLAG_SND|RME_SIG_FLAG_RCV");
        }

        /* Send endpoints */
        fprintf(File, "    /* Send endpoints */\n");
        for(EACH(struct Send_Info*,Send,Proc->Send))
        {
            fprintf(File, "    RVM_ASSERT(RVM_Captbl_Add(%s, %lld, RVM_CTRECV%lld, %lld, %s)==0);\n",
                    Proc->Captbl_Cap.RVM_Macro, Send->Cap.Loc_Capid, Send->Cap.RVM_Capid/Capacity, Send->Cap.RVM_Capid%Capacity,
                    "RME_SIG_FLAG_SND");
        }

        /* Vector endpoints */
        fprintf(File, "    /* Vector endpoints */\n");
        for(EACH(struct Vect_Info*,Vect,Proc->Vect))
        {
            fprintf(File, "    RVM_ASSERT(RVM_Captbl_Add(%s, %lld, RVM_CTVECT%lld, %lld, %s)==0);\n",
                    Proc->Captbl_Cap.RVM_Macro, Vect->Cap.Loc_Capid, Vect->Cap.RVM_Capid/Capacity, Vect->Cap.RVM_Capid%Capacity,
                    "RME_SIG_FLAG_RCV");
        }
    }
    fprintf(File, "}\n");
    Write_Func_Footer(File, "RVM_Boot_Captbl_Init");

    /* Thread initialization */
    Write_Func_Desc(File, "RVM_Boot_Thd_Init");
    fprintf(File, "Description : Initialize the all threads.\n");
    Write_Func_None(File);
    fprintf(File, "void RVM_Boot_Thd_Init(void)\n");
    fprintf(File, "{\n");
    fprintf(File, "    rvm_ptr_t Init_Stack_Addr;\n");
    for(EACH(struct Proc_Info*,Proc,Proj->Proc))
    {
        fprintf(File, "    \n    /* Initializing thread for process: %s */\n", Proc->Name);
        
        for(EACH(struct Thd_Info*,Thd,Proc->Thd))
        {
            fprintf(File, "    RVM_ASSERT(RVM_Thd_Sched_Bind(%s, RVM_INIT_GUARD_THD, RVM_INIT_GUARD_SIG, %s, %lld)==0);\n",
                    Thd->Cap.RVM_Macro, Thd->Cap.RVM_Macro, Thd->Priority);
            fprintf(File, "    Init_Stack_Addr=RVM_Stack_Init(0x%llX, 0x%llX, 0x%llX, 0x%llX);\n",
                    Thd->Map.Stack_Base, Thd->Map.Stack_Size, Thd->Map.Entry_Addr, Proc->Map.Entry_Code_Front);
            fprintf(File, "    RVM_ASSERT(RVM_Thd_Exec_Set(%s, 0x%llX, Init_Stack_Addr, 0x%llX)==0);\n",
                    Thd->Cap.RVM_Macro, Thd->Map.Entry_Addr, Thd->Map.Param_Value);
        }
    }
    fprintf(File, "}\n");
    Write_Func_Footer(File, "RVM_Boot_Thd_Init");

    /* Invocation initialization */
    Write_Func_Desc(File, "RVM_Boot_Inv_Init");
    fprintf(File, "Description : Initialize the all invocations.\n");
    Write_Func_None(File);
    fprintf(File, "void RVM_Boot_Inv_Init(void)\n");
    fprintf(File, "{\n");
    fprintf(File, "    rvm_ptr_t Init_Stack_Addr;\n");
    for(EACH(struct Proc_Info*,Proc,Proj->Proc))
    {
        fprintf(File, "\n    /* Initializing invocation for process: %s */\n", Proc->Name);
        
        for(EACH(struct Inv_Info*,Inv,Proc->Inv))
        {
            fprintf(File, "    Init_Stack_Addr=RVM_Stack_Init(0x%llX, 0x%llX, 0x%llX, 0x%llX);\n",
                    Inv->Map.Stack_Base, Inv->Map.Stack_Size, Inv->Map.Entry_Addr, Proc->Map.Entry_Code_Front);
            /* We always return directly on fault for MCUs, because RVM does not do fault handling there */
            fprintf(File, "    RVM_ASSERT(RVM_Inv_Set(%s, 0x%llX, Init_Stack_Addr, 1)==0);\n",
                    Inv->Cap.RVM_Macro, Inv->Map.Entry_Addr);
        }
    }
    fprintf(File, "}\n");
    Write_Func_Footer(File, "RVM_Boot_Inv_Init");

    /* Receive endpoint initialization - no need at all */

    /* Main initialization function */
    Write_Func_Desc(File, "RVM_Boot_Kobj_Init");
    fprintf(File, "Description : Initialize all kernel objects at boot-time.\n");
    Write_Func_None(File);
    fprintf(File, "void RVM_Boot_Kobj_Init(void)\n");
    fprintf(File, "{\n");
    fprintf(File, "    RVM_Boot_Captbl_Init();\n");
    fprintf(File, "    RVM_Boot_Pgtbl_Init();\n");
    fprintf(File, "    RVM_Boot_Thd_Init();\n");
    fprintf(File, "    RVM_Boot_Inv_Init();\n");
    fprintf(File, "    RVM_Boot_Recv_Init();\n");
    fprintf(File, "}\n");
    Write_Func_Footer(File, "RVM_Boot_Kobj_Init");

    /* Close the file */
    Write_Src_Footer(File);
    fclose(File);
    Free(Buf);
}
/* End Function:RVM_Gen::Boot_Src ********************************************/

/* Begin Function:RVM_Gen::User_Src *******************************************
Description : Generate the rvm_user.c. This file is mainly responsible for user-
              supplied hooks. If the user needs to add functionality, consider
              modifying this file.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void RVM_Gen::User_Src(void)
{
    s8_t* Buf;
    FILE* File;

    /* Create user stubs - pre initialization and post initialization */
    /* Generate rvm_user.c */
    sprintf(Buf, "%s/M7M2_MuAmmonite/Project/Source/rvm_user.c", Output_Path);
    File=fopen(Buf, "wb");
    if(File==0)
        EXIT_FAIL("rvm_user.c open failed.");
    Write_Src_Desc(File, "rvm_user.c", "The user hook file.");

    /* Print all header includes */
    fprintf(File, "/* Includes ******************************************************************/\n");
    Print_RVM_Inc(File, Proj);
    fprintf(File, "#include \"rvm_boot.h\"\n");
    fprintf(File, "/* End Includes **************************************************************/\n\n");

    /* Print all global prototypes */
    fprintf(File, "/* Public C Function Prototypes **********************************************/\n");
    fprintf(File, "void RVM_Boot_Pre_Init(void);\n");
    fprintf(File, "void RVM_Boot_Post_Init(void);\n");
    fprintf(File, "/* End Public C Function Prototypes ******************************************/\n\n");

    /* Preinitialization of hardware */
    Write_Func_Desc(File, "RVM_Boot_Pre_Init");
    fprintf(File, "Description : Initialize critical hardware before any kernel object creation takes place.\n");
    Write_Func_None(File);
    fprintf(File, "void RVM_Boot_Pre_Init(void)\n");
    fprintf(File, "{\n");
    fprintf(File, "    /* Add code here */\n");
    fprintf(File, "}\n");
    Write_Func_Footer(File, "RVM_Boot_Pre_Init");

    /* Postinitialization of hardware */
    Write_Func_Desc(File, "RVM_Boot_Post_Init");
    fprintf(File, "Description : Initialize hardware after all kernel object creation took place.\n");
    Write_Func_None(File);
    fprintf(File, "void RVM_Boot_Post_Init(void)\n");
    fprintf(File, "{\n");
    fprintf(File, "    /* Add code here */\n");
    fprintf(File, "}\n");
    Write_Func_Footer(File, "RVM_Boot_Post_Init");

    /* Close the file */
    Write_Src_Footer(File);
    fclose(File);
    Free(Buf);
}
/* End Function:RVM_Gen::User_Src ********************************************/
}
/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
