/******************************************************************************
Filename    : rme_kobj.cpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The kernel object class.
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

#define __HDR_DEFS__
#include "Core/rme_mcu.hpp"
#include "Core/rme_kobj.hpp"
#undef __HDR_DEFS__

#define __HDR_CLASSES__
#include "Core/rme_kobj.hpp"
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
#undef __HDR_CLASSES__
/* End Includes **************************************************************/
namespace rme_mcu
{
/* Begin Function:Kobj::~Kobj *************************************************
Description : Pure virtual destructor for Kobj class (cannot be omitted).
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
/* void */ Kobj::~Kobj(void)
{
    return;
}
/* End Function:Kobj::~Kobj **************************************************/

/* Begin Function:Kobj::Strcicmp **********************************************
Description : Compare two strings in a case insensitive way.
Input       : const std::string& Str1 - The first string.
              const std::string& Str2 - The second string.
Output      : None.
Return      : ret_t - If two strings are equal, then 0; if not, -1.
******************************************************************************/
ret_t Kobj::Strcicmp(const std::string& Str1, const std::string& Str2)
{
    bool Equal;

    Equal=std::equal(Str1.begin(), Str1.end(), Str2.begin(), Str2.end(),
            [](s8_t Char1, s8_t Char2)
            {
                return std::tolower(Char1)==std::tolower(Char2);
            });

    if(Equal)
        return 0;

    return -1;
}
/* End Function:Kobj::Strcicmp ***********************************************/

/* Begin Function:Kobj::Check_Name ********************************************
Description : See if the names are valid C identifiers.
Input       : const std::string& Name - The names.
Output      : None.
Return      : ret_t - If a valid identifier, 0; else -1.
******************************************************************************/
ret_t Kobj::Check_Name(const std::string& Name)
{
    ptr_t Count;
    const s8_t* Str;

    Str=Name.c_str();
    /* Should not begin with number */
    if((Str[0]>='0')&&(Str[0]<='9'))
        return -1;

    Count=0;
    while(1)
    {
        Count++;
        if(Str[Count]=='\0')
            return 0;
        if((Str[Count]>='a')&&(Str[Count]<='z'))
            continue;
        if((Str[Count]>='A')&&(Str[Count]<='Z'))
            continue;
        if((Str[Count]>='0')&&(Str[Count]<='9'))
            continue;
        if(Str[Count]=='_')
            continue;
        break;
    }
    
    return -1;
}
/* End Function:Kobj::Check_Name *********************************************/

/* Begin Template:Kobj::Check_Kobj ********************************************
Description : Check for kernel object uniqueness within a process.
Type        : class T - The actual type of the kernel object.
Input       : std::vector<std::unique_ptr<T>>& List - List of kernel objects.
Output      : None.
Return      : std::string* - The name that caused the error.
******************************************************************************/
template<class T>
std::string* Kobj::Check_Kobj(std::vector<std::unique_ptr<T>>& List)
{
    for(std::unique_ptr<T>& Obj:List)
    {
        if(Check_Name(*(Obj->Name))!=0)
            return Obj->Name.get();

        for(std::unique_ptr<T>& Temp:List)
        {
            if(&Obj==&Temp)
                continue;
            if(Strcicmp(*(Obj->Name),*(Temp->Name))==0)
                return Obj->Name.get();
        }
    }

    return nullptr;
}
template std::string* Kobj::Check_Kobj<class Proc>(std::vector<std::unique_ptr<class Proc>>& List);
template std::string* Kobj::Check_Kobj<class Thd>(std::vector<std::unique_ptr<class Thd>>& List);
template std::string* Kobj::Check_Kobj<class Inv>(std::vector<std::unique_ptr<class Inv>>& List);
template std::string* Kobj::Check_Kobj<class Recv>(std::vector<std::unique_ptr<class Recv>>& List);
/* End Template:Kobj::Check_Kobj *********************************************/

/* Begin Template:Kobj::Check_Kobj_Proc_Name **********************************
Description : Check for kernel object uniqueness within a process, also considering
              the process name specified by it. Two objects that have the same name
              will be considered different if their process name is different.
Type        : class T - The actual type of the kernel object.
Input       : std::vector<std::unique_ptr<T>>& List - List of kernel objects.
Output      : None.
Return      : std::string* - The name that caused the error.
******************************************************************************/
template<class T>
std::string* Kobj::Check_Kobj_Proc_Name(std::vector<std::unique_ptr<T>>& List)
{
    for(std::unique_ptr<T>& Obj:List)
    {
        if(Check_Name(*(Obj->Name))!=0)
            return Obj->Name.get();

        for(std::unique_ptr<T>& Temp:List)
        {
            if(&Obj==&Temp)
                continue;
            if((Strcicmp(*(Obj->Name),*(Temp->Name))==0)&&
               (Strcicmp(*(Obj->Proc_Name),*(Temp->Proc_Name))==0))
                return Obj->Name.get();
        }
    }

    return nullptr;
}
template std::string* Kobj::Check_Kobj_Proc_Name<class Port>(std::vector<std::unique_ptr<class Port>>& List);
template std::string* Kobj::Check_Kobj_Proc_Name<class Send>(std::vector<std::unique_ptr<class Send>>& List);
/* End Template:Kobj::Check_Kobj_Proc_Name ***********************************/
}
/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
