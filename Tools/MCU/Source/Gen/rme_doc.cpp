/******************************************************************************
Filename    : rme_doc.cpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The document model class.
******************************************************************************/

/* Includes ******************************************************************/
#include "list"
#include "string"
#include "memory"
#include "vector"
#include "stdexcept"

#define __HDR_DEFS__
#include "Main/rme_mcu.hpp"

#include "Gen/rme_doc.hpp"
#undef __HDR_DEFS__

#define __HDR_CLASSES__
#include "Gen/rme_doc.hpp"
#undef __HDR_CLASSES__
/* End Includes **************************************************************/
namespace rme_mcu
{
/* Begin Function:Para::Add ***************************************************
Description : Add a single line to a paragraph.
Input       : std::unique_ptr<std::string>& Line - The single line to add.
Output      : None.
Return      : None.
******************************************************************************/
void Para::Add(std::unique_ptr<std::string>& Line)
{
    this->Line.push_back(std::move(Line));
}
/* End Function:Para::Add ****************************************************/

/* Begin Function:Para::Write *************************************************
Description : Write out the whole paragraph to document.
Input       : FILE* File - The file to write to.
Output      : None.
Return      : None.
******************************************************************************/
void Para::Write(FILE* File)
{
    for(std::unique_ptr<std::string>& Line:this->Line)
        fprintf(File,"%s\n",Line->c_str());
}
/* End Function:Para::Write **************************************************/

/* Begin Function:Doc::Add ****************************************************
Description : Add a single paragraph to a document.
Input       : std::unique_ptr<class Para>& Para - The paragraph to add.
Output      : None.
Return      : None.
******************************************************************************/
void Doc::Add(std::unique_ptr<class Para>& Para)
{
    this->Para.push_back(std::move(Para));
}
/* End Function:Doc::Add *****************************************************/

/* Begin Function:Doc::Write **************************************************
Description : Write out the whole file to document.
Input       : FILE* File - The file to write to.
Output      : None.
Return      : None.
******************************************************************************/
void Doc::Write(FILE* File)
{
    for(std::unique_ptr<class Para>& Para:this->Para)
    {
        Para->Write(File);
        fprintf(File,"\n");
    }
}
/* End Function:Doc::Write ***************************************************/

/* Begin Function:Doc::Find ***************************************************
Description : Write out the whole file to document.
Input       : FILE* File - The file to write to.
Output      : None.
Return      : std::list<std::unique_ptr<class Para>>::iterator - The position of that paragraph.
******************************************************************************/
std::list<std::unique_ptr<class Para>>::iterator Doc::Find(std::unique_ptr<std::string>& Name)
{
    std::list<std::unique_ptr<class Para>>::iterator Iter;
    
    for(Iter=this->Para.begin();Iter!=this->Para.end();Iter++)
    {
        if(*((*Iter)->Name)==*Name)
            return Iter;
    }

    return this->Para.end();
}
/* End Function:Doc::Find ****************************************************/
}
/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
