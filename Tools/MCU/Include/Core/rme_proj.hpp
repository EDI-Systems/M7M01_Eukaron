/******************************************************************************
Filename    : rme_proj.hpp
Author      : pry
Date        : 08/04/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The header for the project reader.
******************************************************************************/

/* Defines *******************************************************************/
namespace rme_mcu
{
#ifdef __HDR_DEFS__
#ifndef __RME_PROJ_HPP_DEFS__
#define __RME_PROJ_HPP_DEFS__
/*****************************************************************************/
/* Recovery options */
#define RECOVERY_THD        (0)
#define RECOVERY_PROC       (1)
#define RECOVERY_SYS        (2)
/*****************************************************************************/
/* __RME_PROJ_HPP_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Classes *******************************************************************/
#ifdef __HDR_CLASSES__
#ifndef __RME_PROJ_HPP_CLASSES__
#define __RME_PROJ_HPP_CLASSES__
/*****************************************************************************/
class Plat
{
public:
    ptr_t Word_Bits;
    ptr_t Capacity;
    ptr_t Init_Num_Ord;
    ptr_t Thd_Size;
    ptr_t Inv_Size;

    Plat(ptr_t Word_Bits, ptr_t Init_Num_Ord, ptr_t Thd_Size, ptr_t Inv_Size);

    /* Page table size */
    virtual ptr_t Pgtbl_Size(ptr_t Num_Order, ptr_t Is_Top)=0;
    /* Align memory */
    virtual void Align_Mem(std::unique_ptr<class Proj>& Proj)=0;
    /* Allocate page table */
    virtual void Alloc_Pgtbl(std::unique_ptr<class Proj>& Proj, std::unique_ptr<class Chip>& Chip)=0;
};

/* The memory map information for RME */
class RME_Memmap
{
public:
    /* Kernel code section */
    ptr_t Code_Base;
    ptr_t Code_Size;
    /* Kernel data section */
    ptr_t Data_Base;
    ptr_t Data_Size;
    /* Kernel memory */
    ptr_t Kmem_Base;
    ptr_t Kmem_Size;
    /* Kernel stack */
    ptr_t Stack_Base;
    ptr_t Stack_Size;
    /* Interrupt flag section */
    ptr_t Intf_Base;
    ptr_t Intf_Size;

    /* Initial state for vector creation */
    ptr_t Vect_Cap_Front;
    ptr_t Vect_Kmem_Front;
};

/* RME kernel information */
class RME
{
public:
    /* RME code section start address */
	ptr_t Code_Start;
    /* RME code section size */
	ptr_t Code_Size;
    /* RME data section start address */
	ptr_t Data_Start;
    /* RME data section size */
	ptr_t Data_Size;
    /* RME kernel stack size */
	ptr_t Stack_Size;
    /* Extra amount of kernel memory */
	ptr_t Extra_Kmem;
    /* Slot order of kernel memory */
	ptr_t Kmem_Order;
    /* Priorities supported */
	ptr_t Kern_Prios;

    /* Compiler information */
    std::unique_ptr<class Comp> Comp;

    /* Raw information about platform, to be deal with by the platform-specific generator */
	std::vector<std::unique_ptr<class Raw>> Plat;
    /* Raw information about chip, to be deal with by the platform-specific generator */
	std::vector<std::unique_ptr<class Raw>> Chip;

    /* Final memory map information */
    std::unique_ptr<class RME_Memmap> Map;

    RME(xml_node_t* Node);
};

/* RVM's capability information, from the user processes */
class Cap
{
public:
    /* What process is this capability in? */
    class Proc* Proc;
    /* What's the content of the capability, exactly? */
    class Kobj* Kobj;

    Cap(std::unique_ptr<class Proc>& Proc, std::unique_ptr<class Kobj>& Kobj);
};

/* The memory map information for RVM */
class RVM_Memmap
{  
public:
    /* Kernel code section */
    ptr_t Code_Base;
    ptr_t Code_Size;
    /* Kernel data section */
    ptr_t Data_Base;
    ptr_t Data_Size;
    /* Guard daemon stack */
    ptr_t Guard_Stack_Base;
    ptr_t Guard_Stack_Size;
    /* VMM daemon stack - currently unused */
    ptr_t VMM_Stack_Base;
    ptr_t VMM_Stack_Size;
    /* Interrupt daemon stack - currently unused */
    ptr_t Intd_Stack_Base;
    ptr_t Intd_Stack_Size;

    /* Initial state for RVM setup */ 
    ptr_t Before_Cap_Front;
    ptr_t Before_Kmem_Front;
    /* When we begin creating capability tables */
    ptr_t Captbl_Cap_Front;
    ptr_t Captbl_Kmem_Front;
    /* When we begin creating page tables */
    ptr_t Pgtbl_Cap_Front;
    ptr_t Pgtbl_Kmem_Front;
    /* When we begin creating processes */
    ptr_t Proc_Cap_Front;
    ptr_t Proc_Kmem_Front;
    /* When we begin creating threads */
    ptr_t Thd_Cap_Front;
    ptr_t Thd_Kmem_Front;
    /* When we begin creating invocations */
    ptr_t Inv_Cap_Front;
    ptr_t Inv_Kmem_Front;
    /* When we begin creating receive endpoints */
    ptr_t Recv_Cap_Front;
    ptr_t Recv_Kmem_Front;
    /* After the booting all finishes */ 
    ptr_t After_Cap_Front;
    ptr_t After_Kmem_Front;
};

/* RVM user-level library information. */
class RVM
{
public:
    /* Size of the code section. This always immediately follow the code section of RME. */
    ptr_t Code_Size;
    /* Size of the data section. This always immediately follow the data section of RME. */
    ptr_t Data_Size;
    /* RVM service threads stack size */
	ptr_t Stack_Size;
    /* The extra amount in the main capability table */
	ptr_t Extra_Captbl;
    /* The recovery mode - by thread, process or the whole system? */
	ptr_t Recovery;
    
    /* Compiler information */
    std::unique_ptr<class Comp> Comp;

    /* Global captbl containing all sorts of kernel objects */
    std::vector<std::unique_ptr<class Cap>> Captbl;
    std::vector<std::unique_ptr<class Cap>> Pgtbl;
    std::vector<std::unique_ptr<class Cap>> Proc;
    std::vector<std::unique_ptr<class Cap>> Thd;
    std::vector<std::unique_ptr<class Cap>> Inv;
    std::vector<std::unique_ptr<class Cap>> Recv;
    std::vector<std::unique_ptr<class Cap>> Vect;

    /* Final memory map information */
    std::unique_ptr<class RVM_Memmap> Map;

    RVM(xml_node_t* Node);
};

/* For parsing and storing project information */
class Proj
{
public:
    /* The name of the project */
	std::unique_ptr<std::string> Name;
    /* The platform used */
    std::unique_ptr<std::string> Plat_Name;
    /* The all-lower-case of the platform used */
    std::unique_ptr<std::string> Plat_Lower;
    /* The chip class used */
	std::unique_ptr<std::string> Chip_Class;
    /* The full name of the exact chip used */
    std::unique_ptr<std::string> Chip_Full;
    
    /* The RME kernel information */
	std::unique_ptr<class RME> RME;
    /* The RVM user-library information */
	std::unique_ptr<class RVM> RVM;
    /* The process information */
	std::vector<std::unique_ptr<class Proc>> Proc;

    Proj(xml_node_t* Node);

    static void To_Upper(std::unique_ptr<std::string>& Str);
    static void To_Lower(std::unique_ptr<std::string>& Str);
};
/*****************************************************************************/
/* __RME_PROJ_HPP_CLASSES__ */
#endif
/* __HDR_CLASSES__ */
#endif
}
/* End Classes ***************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
