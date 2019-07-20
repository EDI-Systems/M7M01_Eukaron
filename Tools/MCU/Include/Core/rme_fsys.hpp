/******************************************************************************
Filename    : rme_fsys.hpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The header for the file system access class.
******************************************************************************/

/* Defines *******************************************************************/
namespace rme_mcu
{
#ifdef __HDR_DEFS__
#ifndef __RME_FSYS_HPP_DEFS__
#define __RME_FSYS_HPP_DEFS__
/*****************************************************************************/

/*****************************************************************************/
/* __RME_FSYS_HPP_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Classes *******************************************************************/
#ifdef __HDR_CLASSES__
#ifndef __RME_FSYS_HPP_CLASSES__
#define __RME_FSYS_HPP_CLASSES__
/*****************************************************************************/
/* Filesystem operations */
class Fsys
{
public:
    Fsys(void){};
    virtual ~Fsys(void){};

    ret_t Fsys::Dir_Present(std::unique_ptr<std::string>& Path);
    ret_t Fsys::Dir_Empty(std::unique_ptr<std::string>& Path);
    void Fsys::Make_Dir(std::unique_ptr<std::string>& Path);

    virtual ptr_t File_Size(std::unique_ptr<std::string>& Path)=0;
    virtual void Copy_File(std::unique_ptr<std::string>& File)=0;
    virtual std::unique_ptr<std::string> Read_Proj(std::unique_ptr<std::string>& Path)=0;
    virtual std::unique_ptr<std::string> Read_Chip(std::unique_ptr<std::string>& Path)=0;
};

class Sysfs:public Fsys
{
public:
    std::unique_ptr<std::string> Root;
    std::unique_ptr<std::string> Output;

    Sysfs(std::unique_ptr<std::string>& Root, std::unique_ptr<std::string>& Output);
    ~Sysfs(void){};

    virtual ptr_t File_Size(std::unique_ptr<std::string>& Path) final override;
    virtual void Copy_File(std::unique_ptr<std::string>& File) final override;
    virtual std::unique_ptr<std::string> Read_Proj(std::unique_ptr<std::string>& Path) final override;
    virtual std::unique_ptr<std::string> Read_Chip(std::unique_ptr<std::string>& Path) final override;
};

class Pbfs:public Fsys
{
public:
    struct PBFS_Env PBFS;
    std::unique_ptr<std::string> Output;

    Pbfs(std::unique_ptr<std::string>& PBFS, std::unique_ptr<std::string>& Output);
    ~Pbfs(void){};

    virtual ptr_t File_Size(std::unique_ptr<std::string>& Path) final override;
    virtual void Copy_File(std::unique_ptr<std::string>& File) final override;
    virtual std::unique_ptr<std::string> Read_Proj(std::unique_ptr<std::string>& Path) final override;
    virtual std::unique_ptr<std::string> Read_Chip(std::unique_ptr<std::string>& Path) final override;
};
/*****************************************************************************/
/* __RME_FSYS_HPP_CLASSES__ */
#endif
/* __HDR_CLASSES__ */
#endif
}
/* End Classes ***************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
