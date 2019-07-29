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
    
    Para(std::unique_ptr<std::string> Name);
    Para(const s8_t* Name, ...);

    void Add(std::unique_ptr<std::string> Line);
    void Add(const s8_t* Line, ...);
    void Write(FILE* File);
    
    void Cfunc_Desc(std::unique_ptr<std::string>& Name,
                    std::unique_ptr<std::string>& Desc,
                    std::vector<std::unique_ptr<std::string>>& Input,
                    std::vector<std::unique_ptr<std::string>>& Output,
                    std::unique_ptr<std::string>& Return);
    void Cfunc_Desc(const s8_t* Name, const s8_t* Desc,
                     std::vector<std::unique_ptr<std::string>>& Input,
                    std::vector<std::unique_ptr<std::string>>& Output,
                    const s8_t* Return);
    void Cfunc_Foot(std::unique_ptr<std::string>& Name);
    void Cfunc_Foot(const s8_t* Name);
    
    void Cdef(std::unique_ptr<std::string>& Macro, std::unique_ptr<std::string>& Value);
    void Cdef(std::unique_ptr<std::string>& Macro, ret_t Value);
    void Cdef(std::unique_ptr<std::string>& Macro, ptr_t Value);
};

/* Document */
class Doc
{
public:
    std::list<std::unique_ptr<class Para>> Para;

    void Add(std::unique_ptr<class Para> Para);
    std::list<std::unique_ptr<class Para>>::iterator Doc::Find(std::unique_ptr<std::string>& Name);
    void Write(FILE* File);

    void Csrc_Desc(std::unique_ptr<std::string>& Name, std::unique_ptr<std::string>& Desc);
    void Csrc_Desc(s8_t* Name, s8_t* Desc);
    void Csrc_Foot(void);
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
