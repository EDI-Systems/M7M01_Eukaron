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
#define PARA_DOC
#define PARA_HEADER
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
    std::unique_ptr<std::string> Output;

    virtual ~Fsys(void){};

    ret_t Dir_Present(std::unique_ptr<std::string>& Path);
    ret_t Dir_Empty(std::unique_ptr<std::string>& Path);
    void Make_Dir(std::unique_ptr<std::string>& Path);
    void Make_Dir(const s8_t* Path, ...);
    void Copy_File(s8_t* Path, ...);
    FILE* Open_File(std::unique_ptr<std::string>& File);
    FILE* Open_File(s8_t* Path, ...);

    virtual ptr_t File_Size(std::unique_ptr<std::string>& Path)=0;
    virtual void Copy_File(std::unique_ptr<std::string>& File)=0;
    virtual std::unique_ptr<std::string> Read_Proj(std::unique_ptr<std::string>& Path)=0;
    virtual std::unique_ptr<std::string> Read_Chip(std::unique_ptr<std::string>& Path)=0;
};

class Sysfs:public Fsys
{
public:
    std::unique_ptr<std::string> Root;

    Sysfs(std::unique_ptr<std::string>& Root, std::unique_ptr<std::string>& Output);

    virtual ptr_t File_Size(std::unique_ptr<std::string>& Path) final override;
    virtual void Copy_File(std::unique_ptr<std::string>& File) final override;
    virtual std::unique_ptr<std::string> Read_Proj(std::unique_ptr<std::string>& Path) final override;
    virtual std::unique_ptr<std::string> Read_Chip(std::unique_ptr<std::string>& Path) final override;
};

/* PBFS-based solution - currently not implemented */
class Pbfs:public Fsys
{
public:
    struct PBFS_Env PBFS;

    Pbfs(std::unique_ptr<std::string>& PBFS, std::unique_ptr<std::string>& Output);

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
