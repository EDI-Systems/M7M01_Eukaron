/******************************************************************************
Filename    : rme_mem.cpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The memory block class.
******************************************************************************/

/* Includes ******************************************************************/
#include "string"
#include "memory"
#include "vector"
#include "bitset"
#include "stdexcept"

extern "C"
{
#include "xml.h"
}

#define __HDR_DEFS__
#include "Core/rme_mcu.hpp"
#include "Core/rme_kobj.hpp"
#include "Core/rme_mem.hpp"
#undef __HDR_DEFS__

#define __HDR_CLASSES__
#include "Core/rme_kobj.hpp"
#include "Core/rme_mem.hpp"
#undef __HDR_CLASSES__
/* End Includes **************************************************************/
namespace rme_mcu
{
/* Begin Function:Mem::Mem ****************************************************
Description : Constructor for Mem class.
Input       : xml_node_t* Node - The node containing the memory block information.
Output      : None.
Return      : None.
******************************************************************************/
/* void */ Mem::Mem(xml_node_t* Node)
{
    xml_node_t* Temp;
    std::unique_ptr<std::string> Str;

    try
    {
        /* Start */
        if((XML_Child(Node,"Start",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Start section is missing.");
        if((Temp->XML_Val_Len==4)&&(std::string(Temp->XML_Val,4)=="Auto"))
            this->Start=MEM_AUTO;
        else if(XML_Get_Hex(Temp,&(this->Start))<0)
            throw std::invalid_argument("Start is not a valid hex integer.");

        /* Size */
        if((XML_Child(Node,"Size",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Size section is missing.");
        if(XML_Get_Hex(Temp,&(this->Size))<0)
            throw std::invalid_argument("Size is not a valid hex integer.");
        if(this->Size==0)
            throw std::invalid_argument("Size cannot be zero.");
        if(this->Start!=MEM_AUTO)
        {
            if((this->Start+this->Size)>0x100000000ULL)
                throw std::invalid_argument("Size is out of bound.");
        }
        else
        {
            if(this->Size>0x100000000ULL)
                throw std::invalid_argument("Size is out of bound.");
        }

        /* Type */
        if((XML_Child(Node,"Type",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Type section is missing.");
        if((Temp->XML_Val_Len==6)&&(std::string(Temp->XML_Val,6)=="Device"))
        {
            if(this->Start==MEM_AUTO)
                throw std::invalid_argument("Device typed memory cannot be automatically allocated.");
        }

        /* Attribute */
        if((XML_Child(Node,"Attribute",&Temp)<0)||(Temp==0))
            throw std::invalid_argument("Attribute section is missing.");
        if(Temp->XML_Val_Len==0)
            throw std::invalid_argument("Attribute section is empty.");
        Str=std::make_unique<std::string>(Temp->XML_Val,(int)Temp->XML_Val_Len);
        this->Attr=0;

        if(Str->rfind('R')!=std::string::npos)
            this->Attr|=MEM_READ;
        if(Str->rfind('W')!=std::string::npos)
            this->Attr|=MEM_WRITE;
        if(Str->rfind('X')!=std::string::npos)
            this->Attr|=MEM_EXECUTE;

        if(this->Attr==0)
            throw std::invalid_argument("Attribute does not allow any access and is malformed.");

        if(Str->rfind('B')!=std::string::npos)
            this->Attr|=MEM_BUFFERABLE;
        if(Str->rfind('C')!=std::string::npos)
            this->Attr|=MEM_CACHEABLE;
        if(Str->rfind('S')!=std::string::npos)
            this->Attr|=MEM_STATIC;
    }
    catch(std::exception& Exc)
    {
        throw std::runtime_error(std::string("Memory:\n")+Exc.what());
    }
}
/* End Function:Mem::Mem *****************************************************/

/* Begin Function:Mem::Mem ****************************************************
Description : Constructor for Mem class.
Input       : ptr_t Start - The start address.
              ptr_t Size - The memory trunk size.
              ptr_t Attr - The attributes of this memory block.
              ptr_t Align - The alignment size.
Output      : None.
Return      : None.
******************************************************************************/
/* void */ Mem::Mem(ptr_t Start, ptr_t Size, ptr_t Attr, ptr_t Align)
{
    this->Start=Start;
    this->Size=Size;
    this->Attr=Attr;
    this->Align=Align;
}
/* End Function:Mem::Mem *****************************************************/

/* Begin Function:Memmap::Memmap **********************************************
Description : Constructor for Memmap class.
Input       : std::unique_ptr<class Mem>& Mem - The memory trunk.
Output      : None.
Return      : None.
******************************************************************************/
/* void */ Memmap::Memmap(std::unique_ptr<class Mem>& Mem)
{
    /* This pointer does not assume ownership */
    this->Mem=Mem.get();
    this->Map.resize((unsigned int)(Mem->Size/MAP_ALIGN+1));
    std::fill(this->Map.begin(), this->Map.end(), false);
}
/* End Function:Memmap::Memmap ***********************************************/

/* Begin Function:Memmap::Try *************************************************
Description : See if this bitmap segment is already covered.
Input       : std::unique_ptr<class Memmap>& Map - The bitmap.
              ptr_t Start - The starting address.
              ptr_t Size - The size to allocate.
Output      : None.
Return      : ret_t - If can be marked, 0; else -1.
******************************************************************************/
ret_t Memmap::Try(std::unique_ptr<class Memmap>& Map, ptr_t Start, ptr_t Size)
{
    ptr_t Count;
    ptr_t Bit_Start;
    ptr_t Bit_Size;

    /* See if we can fit there */
    if((Start<Map->Mem->Start)&&(Start>=(Map->Mem->Start+Map->Mem->Size)))
        return -1;
    if((Map->Mem->Start+Map->Mem->Size)<(Start+Size))
        return -1;

    Bit_Start=(Start-Map->Mem->Start)/MAP_ALIGN;
    Bit_Size=Size/MAP_ALIGN;
    
    for(Count=Bit_Start;Count<Bit_Start+Bit_Size;Count++)
    {
        if(Map->Map[(unsigned int)Count]!=false)
            return -1;
    }
    return 0;
}
/* End Function:Memmap::Try **************************************************/

/* Begin Function:Memmap::Mark ************************************************
Description : Actually mark this bitmap segment. Each bit is always 4 bytes.
Input       : std::unique_ptr<class Memmap>& Map - The bitmap.
              ptr_t Start - The starting address.
              ptr_t Size - The size to allocate.
Output      : std::unique_ptr<class Memmap>& Map - The updated bitmap.
Return      : None.
******************************************************************************/
ret_t Memmap::Mark(std::unique_ptr<class Memmap>& Map, ptr_t Start, ptr_t Size)
{
    ptr_t Count;
    ptr_t Bit_Start;
    ptr_t Bit_Size;

    /* See if we can fit there */
    if((Start<Map->Mem->Start)&&(Start>=(Map->Mem->Start+Map->Mem->Size)))
        return -1;
    if((Map->Mem->Start+Map->Mem->Size)<(Start+Size))
        return -1;

    Bit_Start=(Start-Map->Mem->Start)/MAP_ALIGN;
    Bit_Size=Size/MAP_ALIGN;

    for(Count=Bit_Start;Count<Bit_Start+Bit_Size;Count++)
        Map->Map[(unsigned int)Count]=true;

    return 0;
}
/* End Function:Memmap::Mark *************************************************/

/* Begin Function:Memmap::Fit_Static ******************************************
Description : Populate the memory data structure with this memory segment.
              This operation will be conducted with no respect to whether this
              portion have been populated with someone else.
Input       : std::vector<std::unique_ptr<class Memmap>>& Map - The memory map.
              ptr_t Start - The start address of the memory.
              ptr_t Size - The size of the memory.
              ptr_t Attr - The attributes of the memory.
Output      : std::vector<std::unique_ptr<class Memmap>>& Map - The updated memory map.
Return      : ret_t - If successful, 0; else -1.
******************************************************************************/
ret_t Memmap::Fit_Static(std::vector<std::unique_ptr<class Memmap>>& Map,
                         ptr_t Start, ptr_t Size, ptr_t Attr)
{
    class Mem* Mem;

    /* See if we can even find a segment that accomodates this */
    for(std::unique_ptr<class Memmap>& Info:Map)
    {
        Mem=Info->Mem;
        if((Start>=Mem->Start)&&(Start<(Mem->Start+Mem->Size)))
        {
            /* See if we can fit there */
            if((Mem->Start+Mem->Size)<(Start+Size))
                return -1;

            /* Is the attributes correct? */
            if((Mem->Attr&Attr)!=Attr)
                return -1;

            /* It is clear that we can fit now. Mark all the bits. We do not check it it
             * is already marked, because we allow overlapping. */
            return Mark(Info, Start, Size);
        }
    }

    return -1;
}
/* End Function:Memmap::Fit_Static *******************************************/

/* Begin Function:Memmap::Fit_Auto ********************************************
Description : Fit the auto-placed memory segments to a fixed location.
Input       : std::vector<std::unique_ptr<class Memmap>>& Map - The memory map.
              ptr_t Start - The start address of the memory.
              ptr_t Size - The size of the memory.
              ptr_t Attr - The attributes of the memory.
              ptr_t Align - The alignment granularity of the memory.
Output      : std::vector<std::unique_ptr<class Memmap>>& Map - The updated memory map.
              ptr_t* Start - The start address of the memory.
Return      : ret_t - If successful, 0; else -1.
******************************************************************************/
ret_t Memmap::Fit_Auto(std::vector<std::unique_ptr<class Memmap>>& Map,
                       ptr_t* Start, ptr_t Size, ptr_t Align, ptr_t Attr)
{
    ptr_t Fit_Start;
    ptr_t Fit_End;
    ptr_t Fit_Try;
    class Mem* Fit;

    /* Find somewhere to fit this memory trunk, and if found, we will populate it */
    for(std::unique_ptr<class Memmap>& Info:Map)
    {
        Fit=Info->Mem;

        /* Is the size possibly sufficient? */
        if(Size>Fit->Size)
            continue;

        /* Is the attribute a superset of what we require? */
        if((Fit->Attr&Attr)!=Attr)
            continue;

        /* Round start address up, round end address down, to alignment */
        Fit_Start=((Fit->Start+Align-1)/Align)*Align;
        Fit_End=((Fit->Start+Fit->Size)/Align)*Align;

        if(Size>(Fit_End-Fit_Start))
            continue;

        for(Fit_Try=Fit_Start;Fit_Try<Fit_End;Fit_Try+=Align)
        {
            if(Try(Info,Fit_Try,Size)==0)
            {
                /* Found a fit */
                Mark(Info,Fit_Try,Size);
                *Start=Fit_Try;
                return 0;
            }
        }
    }

    /* Can't find any fit */
    return -1;
}
/* End Function:Memmap::Fit_Auto *********************************************/
}
/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
