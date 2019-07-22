/******************************************************************************
Filename    : rme_mem.hpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The header for the memory class.
******************************************************************************/

/* Defines *******************************************************************/
namespace rme_mcu
{
#ifdef __HDR_DEFS__
#ifndef __RME_MEM_HPP_DEFS__
#define __RME_MEM_HPP_DEFS__
/*****************************************************************************/
/* Memory access permissions */
#define MEM_READ            POW2(0)
#define MEM_WRITE           POW2(1)
#define MEM_EXECUTE         POW2(2)
#define MEM_BUFFERABLE      POW2(3)
#define MEM_CACHEABLE       POW2(4)
#define MEM_STATIC          POW2(5)
/* Memory placement */
#define MEM_AUTO            ((ptr_t)(-1LL))
/* Memmap granularity */
#define MAP_ALIGN           (32)
/*****************************************************************************/
/* __RME_MEM_HPP_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Classes *******************************************************************/
#ifdef __HDR_CLASSES__
#ifndef __RME_MEM_HPP_CLASSES__
#define __RME_MEM_HPP_CLASSES__
/*****************************************************************************/
/* Send endpoint information */
class Mem
{
public:
    /* The start address */
	ptr_t Start;
    /* The size */
	ptr_t Size;
    /* The attributes - read, write, execute, cacheable, bufferable, static */
	ptr_t Attr;
    /* The alignment granularity */
    ptr_t Align;

    Mem(xml_node_t* Node);
    Mem(ptr_t Start, ptr_t Size, ptr_t Attr, ptr_t Align);
};

class Memmap
{
public:
    class Mem* Mem;
    std::vector<bool> Map;

    Memmap(std::unique_ptr<class Mem>& Mem);

    static ret_t Try(std::unique_ptr<class Memmap>& Map, ptr_t Start, ptr_t Size);
    static ret_t Mark(std::unique_ptr<class Memmap>& Map, ptr_t Start, ptr_t Size);
    static ret_t Fit_Static(std::vector<std::unique_ptr<class Memmap>>& Map,
                            ptr_t Start, ptr_t Size, ptr_t Attr);
    static ret_t Fit_Auto(std::vector<std::unique_ptr<class Memmap>>& Map,
                          ptr_t* Start, ptr_t Size, ptr_t Align, ptr_t Attr);
};
/*****************************************************************************/
/* __RME_MEM_HPP_CLASSES__ */
#endif
/* __HDR_CLASSES__ */
#endif
}
/* End Classes ***************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
