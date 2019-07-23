/******************************************************************************
Filename    : rme_doc.hpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The header for the document class.
******************************************************************************/

/* Defines *******************************************************************/
namespace rme_mcu
{
#ifdef __HDR_DEFS__
#ifndef __RME_DOC_HPP_DEFS__
#define __RME_DOC_HPP_DEFS__
/*****************************************************************************/

/*****************************************************************************/
/* __RME_DOC_HPP_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Classes *******************************************************************/
#ifdef __HDR_CLASSES__
#ifndef __RME_DOC_HPP_CLASSES__
#define __RME_DOC_HPP_CLASSES__
/*****************************************************************************/
/* Paragraph */
class Para
{
public:
    std::unique_ptr<std::string> Name;
    std::list<std::unique_ptr<std::string>> Line;

    void Add(std::unique_ptr<std::string>& Line);
    void Write(FILE* File);
};

/* Document */
class Doc
{
public:
    std::list<std::unique_ptr<class Para>> Para;

    virtual void Read(FILE* File)=0;

    void Add(std::unique_ptr<class Para>& Para);
    std::list<std::unique_ptr<class Para>>::iterator Doc::Find(std::unique_ptr<std::string>& Name);
    void Write(FILE* File);
};
/*****************************************************************************/
/* __RME_DOC_HPP_CLASSES__ */
#endif
/* __HDR_CLASSES__ */
#endif
}
/* End Classes ***************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
