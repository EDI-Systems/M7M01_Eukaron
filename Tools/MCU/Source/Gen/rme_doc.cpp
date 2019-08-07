/******************************************************************************
Filename    : rme_doc.cpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The document model class.
******************************************************************************/

/* Includes ******************************************************************/
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "time.h"
#include "stdarg.h"

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
/* Begin Function:Para::Para **************************************************
Description : Constructor for the Para class.
Input       : std::unique_ptr<std::string> Name - The name of the paragraph.
Output      : None.
Return      : None.
******************************************************************************/
/* void */ Para::Para(std::unique_ptr<std::string> Name)
{
    this->Name=std::move(Name);
}
/* End Function:Para::Para ***************************************************/

/* Begin Function:Para::Para **************************************************
Description : Add a single line to a paragraph.
Input       : const s8_t* Name - The single line to add.
Output      : None.
Return      : None.
******************************************************************************/
/* void */ Para::Para(const s8_t* Name, ...)
{
    s8_t Buf[1024];
    va_list Args;

    va_start(Args, Name);
    vsprintf(Buf, Name, Args);
    va_end(Args);

    this->Name=std::make_unique<std::string>(Buf);
}
/* End Function:Para::Para ***************************************************/

/* Begin Function:Para::Add ***************************************************
Description : Add a single line to a paragraph.
Input       : std::unique_ptr<std::string> Line - The single line to add.
Output      : None.
Return      : None.
******************************************************************************/
void Para::Add(std::unique_ptr<std::string> Line)
{
    this->Line.push_back(std::move(Line));
}
/* End Function:Para::Add ****************************************************/

/* Begin Function:Para::Add ***************************************************
Description : Add a single line to a paragraph.
Input       : const s8_t* Line - The single line to add.
Output      : None.
Return      : None.
******************************************************************************/
void Para::Add(const s8_t* Line, ...)
{
    s8_t Buf[1024];
    va_list Args;

    va_start(Args, Line);
    vsprintf(Buf, Line, Args);
    va_end(Args);

    this->Line.push_back(std::make_unique<std::string>(Buf));
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

/* Begin Function:Para::Cfunc_Desc ********************************************
Description : Write the description part of a C function.
Input       : std::unique_ptr<std::string>& Name - Name of the function.
              std::unique_ptr<std::string>& Desc - One-line description.
              std::vector<std::unique_ptr<std::string>>& Input - The inputs.
              std::vector<std::unique_ptr<std::string>>& Output - The outputs.
              std::unique_ptr<std::string>& Return - The return value.
Output      : None.
Return      : None.
******************************************************************************/
void Para::Cfunc_Desc(std::unique_ptr<std::string>& Name,
                      std::unique_ptr<std::string>& Desc,
                      std::vector<std::unique_ptr<std::string>>& Input,
                      std::vector<std::unique_ptr<std::string>>& Output,
                      std::unique_ptr<std::string>& Return)
{
    ptr_t Len;
    s8_t Buf[256];
    std::unique_ptr<std::string> Line;

    for(Len=sprintf(Buf, "/* Begin Function:%s ", Name->c_str());Len<79;Len++)
        Buf[Len]='*';
    Buf[Len]='\0';
    Line=std::make_unique<std::string>(Buf);
    this->Add(std::move(Line));

    /* Print all inputs */
    if(Input.size()==0)
    {
        Line=std::make_unique<std::string>("Input       : None.");
        this->Add(std::move(Line));
    }
    else
    {
        Line=std::make_unique<std::string>("Input       : ");
        *Line+=*Input[0];
        this->Add(std::move(Line));
        for(std::unique_ptr<std::string>& Entry:Input)
        {
            if(Entry==Input[0])
                continue;
            Line=std::make_unique<std::string>("           : ");
            *Line+=*Entry;
            this->Add(std::move(Line));
        }
    }

    /* Print all outputs */
    if(Output.size()==0)
    {
        Line=std::make_unique<std::string>("Output      : None.");
        this->Add(std::move(Line));
    }
    else
    {
        Line=std::make_unique<std::string>("Output      : ");
        *Line+=*Output[0];
        this->Add(std::move(Line));
        for(std::unique_ptr<std::string>& Entry:Output)
        {
            if(Entry==Output[0])
                continue;
            Line=std::make_unique<std::string>("           : ");
            *Line+=*Entry;
            this->Add(std::move(Line));
        }
    }

    /* Print return */
    Line=std::make_unique<std::string>("Return      : ");
    *Line+=*Return;
    this->Add(std::move(Line));
    
    this->Add("******************************************************************************/");
}
/* End Function:Para::Cfunc_Desc *********************************************/

/* Begin Function:Para::Cfunc_Desc ********************************************
Description : Write the description part of a C function.
Input       : const s8_t* Name - Name of the function.
              const s8_t* Desc - One-line description.
              std::vector<std::unique_ptr<std::string>>& Input - The inputs.
              std::vector<std::unique_ptr<std::string>>& Output - The outputs.
              const s8_t* Return - The return value.
Output      : None.
Return      : None.
******************************************************************************/
void Para::Cfunc_Desc(const s8_t* Name, const s8_t* Desc,
                      std::vector<std::unique_ptr<std::string>>& Input,
                      std::vector<std::unique_ptr<std::string>>& Output,
                      const s8_t* Return)
{
    Cfunc_Desc(std::make_unique<std::string>(Name),
               std::make_unique<std::string>(Desc),
               Input, Output, std::make_unique<std::string>(Return));
}
/* End Function:Para::Cfunc_Desc *********************************************/

/* Begin Function:Para::Cfunc_Foot ********************************************
Description : Write the footer part of a C function.
Input       : std::unique_ptr<std::string>& Name - Name of the function.
Output      : None.
Return      : None.
******************************************************************************/
void Para::Cfunc_Foot(std::unique_ptr<std::string>& Name)
{
    ptr_t Len;
    s8_t Buf[256];

    for(Len=sprintf(Buf, "/* End Function:%s ", Name->c_str());Len<78;Len++)
        Buf[Len]='*';

    Buf[Len]='/';
    Buf[Len+1]='\0';
    
    this->Add(std::make_unique<std::string>(Buf));
}
/* End Function:Para::Cfunc_Foot *********************************************/

/* Begin Function:Para::Cfunc_Foot ********************************************
Description : Write the footer part of a C function.
Input       : const s8_t* Name - Name of the function.
Output      : None.
Return      : None.
******************************************************************************/
void Para::Cfunc_Foot(const s8_t* Name)
{
    Cfunc_Foot(std::make_unique<std::string>(Name));
}
/* End Function:Para::Cfunc_Foot *********************************************/

/* Begin Function:Para::Cdef_Find *********************************************
Description : Search for a define macro in a file. This only searches for
              canonical macros, which are in the #define XXX form. The space
              between the XXX and "#define" must be one.
Input       : s8_t* Macro - The macro.
Output      : None.
Return      : None.
******************************************************************************/
std::string* Para::Cdef_Find(s8_t* Macro)
{
    std::unique_ptr<std::string> Compare;


    Compare=std::make_unique<std::string>(std::string("#define ")+*Macro);

    for(std::unique_ptr<std::string>& Str:this->Line)
    {
        if(Str->find(*Compare)!=std::string::npos)
            return Str.get();
    }

    return nullptr;
}
/* End Function:Para::Cdef_Find **********************************************/

/* Begin Function:Para::Cdef **************************************************
Description : Make a define statement in the file. The define statement can have
              three parts, which will be converted to uppercase and concatenated
              together. The value here is a string.
Input       : s8_t* Macro - The macro.
              s8_t* Value - The value of the macro.
Output      : None.
Return      : None.
******************************************************************************/
void Para::Cdef(s8_t* Macro, s8_t* Value)
{
    s8_t Format[32];
    s8_t Buf[256];
    std::string* Line;

    /* Print to file */
    sprintf(Format, "#define %%-%ds    (%%s)\n", MACRO_ALIGN-4-8);
    sprintf(Buf, Format, Macro, Value);

    /* See if this already exists */
    Line=Cdef_Find(Macro);
    if(Line!=nullptr)
        *Line=Buf;
    else
        this->Add(std::make_unique<std::string>(Buf));
}
/* End Function:Para::Cdef ***************************************************/

/* Begin Function:Para::Cdef **************************************************
Description : Make a define statement in the file. The define statement can have
              three parts, which will be converted to uppercase and concatenated
              together. The value here is a string.
Input       : std::unique_ptr<std::string>& Macro - The macro.
              std::unique_ptr<std::string>& Value - The value of the macro.
Output      : None.
Return      : None.
******************************************************************************/
void Para::Cdef(std::unique_ptr<std::string>& Macro, std::unique_ptr<std::string>& Value)
{
    Cdef((s8_t*)(Macro->c_str()),(s8_t*)(Value->c_str()));
}
/* End Function:Para::Cdef ***************************************************/

/* Begin Function:Para::Cdef **************************************************
Description : Make a define statement in the file. The define statement can have
              three parts, which will be converted to uppercase and concatenated
              together.
              The value here is a integer.
Input       : s8_t* Macro - The macro.
              ret_t Value - The value of the macro.
Output      : None.
Return      : None.
******************************************************************************/
void Para::Cdef(s8_t* Macro, ret_t Value)
{
    s8_t Format[32];
    s8_t Buf[256];
    std::string* Line;

    /* Print to file */
    sprintf(Format, "#define %%-%lds    (%%lld)\n", MACRO_ALIGN-4-8);
    sprintf(Buf, Format, Macro, Value);

    /* See if this already exists */
    Line=Cdef_Find(Macro);
    if(Line!=nullptr)
        *Line=Buf;
    else
        this->Add(std::make_unique<std::string>(Buf));
}
/* End Function:Para::Cdef ***************************************************/

/* Begin Function:Para::Cdef **************************************************
Description : Make a define statement in the file. The define statement can have
              three parts, which will be converted to uppercase and concatenated
              together.
              The value here is a integer.
Input       : std::unique_ptr<std::string>& Macro - The macro.
              ret_t Value - The value of the macro.
Output      : None.
Return      : None.
******************************************************************************/
void Para::Cdef(std::unique_ptr<std::string>& Macro, ret_t Value)
{
    Cdef((s8_t*)(Macro->c_str()),Value);
}
/* End Function:Para::Cdef ***************************************************/

/* Begin Function:Para::Cdef **************************************************
Description : Make a define statement in the file. The define statement can have
              three parts, which will be converted to uppercase and concatenated
              together.
              This function will see if the define macro is there at all. If it is
              already there, then that line will be modified.
              The value here is a hex integer.
Input       : s8_t* Macro - The macro.
              ptr_t Value - The value of the macro.
Output      : None.
Return      : None.
******************************************************************************/
void Para::Cdef(s8_t* Macro, ptr_t Value)
{
    s8_t Format[32];
    s8_t Buf[256];
    std::string* Line;

    /* Print to file */
    sprintf(Format, "#define %%-%lds    (0x%%llX)\n", MACRO_ALIGN-4-8);
    sprintf(Buf, Format, Macro, Value);

    /* See if this already exists */
    Line=Cdef_Find(Macro);
    if(Line!=nullptr)
        *Line=Buf;
    else
        this->Add(std::make_unique<std::string>(Buf));
}
/* End Function:Para::Cdef ***************************************************/

/* Begin Function:Para::Cdef **************************************************
Description : Make a define statement in the file. The define statement can have
              three parts, which will be converted to uppercase and concatenated
              together.
              This function will see if the define macro is there at all. If it is
              already there, then that line will be modified.
              The value here is a hex integer.
Input       : std::unique_ptr<std::string>& Macro - The macro.
              ptr_t Value - The value of the macro.
Output      : None.
Return      : None.
******************************************************************************/
void Para::Cdef(std::unique_ptr<std::string>& Macro, ptr_t Value)
{
    Cdef((s8_t*)(Macro->c_str()),Value);
}
/* End Function:Para::Cdef ***************************************************/

/* Begin Function:Doc::Doc ****************************************************
Description : Constructor for the Doc class. This parser only parse the functions
              from C source.
Input       : std::unique_ptr<std::list<std::unique_ptr<std::string>>> File - The whole file.
              ptr_t Type - The type of the file, either C source, C header or something else.
Output      : None.
Return      : None.
******************************************************************************/
/* void */ Doc::Doc(std::unique_ptr<std::list<std::unique_ptr<std::string>>> File, ptr_t Type)
{
    std::unique_ptr<std::string> Str;
    std::unique_ptr<class Para> Para;
    ptr_t Start;
    ptr_t End;

    Para=nullptr;
    if(Type==DOCTYPE_CSRC)
    {
        /* Parse C source file */
        while(File->size()!=0)
        {
            /* Extract lines from the file */
            Str=std::move(File->front());
            File->pop_front();
            
            if(Para==nullptr)
            {
                if(Str->find("Begin Function:")!=std::string::npos)
                {
                    Start=Str->find(":");
                    End=Str->find(" ",(unsigned int)Start);

                    if(End==std::string::npos)
                        throw std::runtime_error("C source parsing:\nFile malformed.");

                    Para=std::make_unique<class Para>(Str->substr((unsigned int)Start,(unsigned int)(End-Start)).c_str());
                    Para->Add(std::move(Str));
                }
            }
            else
            {
                Para->Add(std::move(Str));

                if(Str->find("End Function:")!=std::string::npos)
                {
                    Start=Str->find(":");
                    End=Str->find(" ",(unsigned int)Start);

                    if(End==std::string::npos)
                        throw std::runtime_error("C source parsing:\nFile malformed.");

                    if(Str->substr((unsigned int)Start,(unsigned int)(End-Start))!=*(Para->Name))
                        throw std::runtime_error("C source parsing:\nInvalid function comment block.");

                    this->Add(std::move(Para));
                    Para=nullptr;
                }
            }
        }

        if(Para!=nullptr)
            throw std::runtime_error("C source parsing:\nFile malformed.");
    }
    else
    {
        /* Fill the whole thing into a single paragraph */
        Para=std::make_unique<class Para>("Content");

        while(File->size()!=0)
        {
            /* Extract lines from the file */
            Str=std::move(File->front());
            File->pop_front();
            Para->Add(std::move(Str));
        }
        
        this->Add(std::move(Para));
    }
}
/* End Function:Doc::Doc *****************************************************/

/* Begin Function:Doc::Add ****************************************************
Description : Add a single paragraph to a document.
Input       : std::unique_ptr<class Para> Para - The paragraph to add.
Output      : None.
Return      : None.
******************************************************************************/
void Doc::Add(std::unique_ptr<class Para> Para)
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

/* Begin Function:Doc::Csrc_Desc **********************************************
Description : Output the header that is sticked to every C file.
Input       : std::unique_ptr<std::string> Name& - The name of the file.
              std::unique_ptr<std::string> Desc& - The description of the file.
Output      : None.
Return      : None.
******************************************************************************/
void Doc::Csrc_Desc(std::unique_ptr<std::string>& Name, std::unique_ptr<std::string>& Desc)
{
    s8_t Date[64];
    time_t Time;
    struct tm* Time_Struct;
    std::unique_ptr<class Para> Para;
    std::unique_ptr<std::string> Line;

    Para=std::make_unique<class Para>((s8_t*)"Doc:%s",Name->c_str());

    time(&Time);
    Time_Struct=localtime(&Time);
    sprintf(Date,"%02d/%02d/%d",Time_Struct->tm_mday,Time_Struct->tm_mon+1,Time_Struct->tm_year+1900);

    Para->Add("/******************************************************************************");
    Para->Add("Filename    : %s",Name->c_str());
    Para->Add("Author      : " CODE_AUTHOR);
    Para->Add("Date        : %s",Date);
    Para->Add("License     : " CODE_LICENSE);
    Para->Add("Description : %s",Desc->c_str());
    Para->Add("******************************************************************************/");

    this->Add(std::move(Para));
}
/* End Function:Doc::Csrc_Desc ***********************************************/

/* Begin Function:Doc::Csrc_Desc **********************************************
Description : Output the header that is sticked to every C file.
Input       : s8_t* Name - The name of the file.
              s8_t* Desc - The description of the file.
Output      : None.
Return      : None.
******************************************************************************/
void Doc::Csrc_Desc(s8_t* Name, s8_t* Desc)
{
    s8_t Date[64];
    time_t Time;
    struct tm* Time_Struct;
    std::unique_ptr<class Para> Para;
    std::unique_ptr<std::string> Line;

    Para=std::make_unique<class Para>("Doc:%s",Name);

    time(&Time);
    Time_Struct=localtime(&Time);
    sprintf(Date,"%02d/%02d/%d",Time_Struct->tm_mday,Time_Struct->tm_mon+1,Time_Struct->tm_year+1900);

    Para->Add("/******************************************************************************");
    Para->Add("Filename    : %s",Name);
    Para->Add("Author      : " CODE_AUTHOR);
    Para->Add("Date        : %s",Date);
    Para->Add("License     : " CODE_LICENSE);
    Para->Add("Description : %s",Desc);
    Para->Add("******************************************************************************/");

    this->Add(std::move(Para));
}
/* End Function:Doc::Csrc_Desc ***********************************************/

/* Begin Function:Doc::Csrc_Foot **********************************************
Description : Output the footer that is appended to every C file.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void Doc::Csrc_Foot(void)
{
    std::unique_ptr<class Para> Para;
    std::unique_ptr<std::string> Line;

    Para=std::make_unique<class Para>(std::make_unique<std::string>("Doc:Footer"));

    Line=std::make_unique<std::string>("/* End Of File ***************************************************************/");
    Para->Add(std::move(Line));

    Line=std::make_unique<std::string>("");
    Para->Add(std::move(Line));

    Line=std::make_unique<std::string>("/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/");
    Para->Add(std::move(Line));

    this->Add(std::move(Para));
}
/* End Function:Doc::Csrc_Foot ***********************************************/
}
/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
