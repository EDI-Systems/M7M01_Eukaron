/******************************************************************************
Filename    : rme_kobj.hpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The header for the kernel object class.
******************************************************************************/

/* Defines *******************************************************************/
namespace rme_mcu
{
#ifdef __HDR_DEFS__
#ifndef __RME_KOBJ_HPP_DEFS__
#define __RME_KOBJ_HPP_DEFS__
/*****************************************************************************/

/*****************************************************************************/
/* __RME_KOBJ_HPP_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Classes *******************************************************************/
#ifdef __HDR_CLASSES__
#ifndef __RME_KOBJ_HPP_CLASSES__
#define __RME_KOBJ_HPP_CLASSES__
/*****************************************************************************/
/* Kernel object information */
class Kobj
{
public:
    /* Name of the kernel object */
    std::unique_ptr<std::string> Name;
    /* The local capid of the port */
    ptr_t Loc_Capid;
    /* The global linear capid of the endpoint */
    ptr_t RVM_Capid;
    /* The macro denoting the global capid */
    std::unique_ptr<std::string> Loc_Macro;
    /* The macro denoting the global capid */
    std::unique_ptr<std::string> RVM_Macro;
    /* The macro denoting the global capid - for RME */
    std::unique_ptr<std::string> RME_Macro;

    virtual ~Kobj(void)=0;

    static ret_t Strcicmp(const std::string& Str1, const std::string& Str2);
    static ret_t Check_Name(const std::string& Name);

    template<class T>
    static std::string* Check_Kobj(std::vector<std::unique_ptr<T>>& List);
    template<class T>
    static std::string* Check_Kobj_Proc_Name(std::vector<std::unique_ptr<T>>& List);
};
/*****************************************************************************/
/* __RME_KOBJ_HPP_CLASSES__ */
#endif
/* __HDR_CLASSES__ */
#endif
}
/* End Classes ***************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
