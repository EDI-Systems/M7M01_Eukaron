/******************************************************************************
Filename    : rme_proc.hpp
Author      : pry
Date        : 08/04/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The header for the process.
******************************************************************************/

/* Defines *******************************************************************/
namespace rme_mcu
{
#ifdef __HDR_DEFS__
#ifndef __RME_PROC_HPP_DEFS__
#define __RME_PROC_HPP_DEFS__
/*****************************************************************************/
/* Recovery options */
#define RECOVERY_THD        (0)
#define RECOVERY_PROC       (1)
#define RECOVERY_SYS        (2)
/*****************************************************************************/
/* __RME_PROC_HPP_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Classes *******************************************************************/
#ifdef __HDR_CLASSES__
#ifndef __RME_PROC_HPP_CLASSES__
#define __RME_PROC_HPP_CLASSES__
/*****************************************************************************/
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
    
    void Alloc_Kmem(ptr_t Kmem_Front, ptr_t Kmem_Order);
};

/* RVM's capability information, from the user processes */
class Cap
{
public:
    /* What process is this capability in? */
    class Proc* Proc;
    /* What's the content of the capability, exactly? */
    class Kobj* Kobj;

    Cap(class Proc* Proc, class Kobj* Kobj);
};

/* The memory map information for RVM */
class RVM_Memmap
{  
public:
    /* The ultimate capability table size */
    ptr_t Captbl_Size;
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

    void Alloc_Mem(ptr_t Code_Start, ptr_t Data_Start);
};

/* Process memory map */
 class Proc_Memmap
{
public:
    /* Code memory */
    ptr_t Code_Base;
    ptr_t Code_Size;
    /* Data memory */
    ptr_t Data_Base;
    ptr_t Data_Size;
    /* Code memory frontier for entries */
    ptr_t Entry_Code_Front;
};

/* For parsing and storing process information */
class Proc:public Kobj
{
private: 
    void Alloc_RVM_Pgtbl(std::unique_ptr<class RVM>& RVM,
                         std::unique_ptr<class Pgtbl>& Pgtbl);
    void Alloc_Macro_Pgtbl(std::unique_ptr<class Pgtbl>& Pgtbl);

public:
    /* Compiler information */
    std::unique_ptr<class Comp> Comp;

    /* Memory trunk information */
    std::vector<std::unique_ptr<class Mem>> Code;
    std::vector<std::unique_ptr<class Mem>> Data;
    std::vector<std::unique_ptr<class Mem>> Device;

    /* Kernel object information */
    std::unique_ptr<class Captbl> Captbl;
    std::unique_ptr<class Pgtbl> Pgtbl;
    std::vector<std::unique_ptr<class Thd>> Thd;
    std::vector<std::unique_ptr<class Inv>> Inv;
    std::vector<std::unique_ptr<class Port>> Port;
    std::vector<std::unique_ptr<class Recv>> Recv;
    std::vector<std::unique_ptr<class Send>> Send;
    std::vector<std::unique_ptr<class Vect>> Vect;

    /* Memory map for itself */
    std::unique_ptr<class Proc_Memmap> Map;

    Proc(xml_node_t* Node);
    
    void Check_Kobj(void);
    void Alloc_Loc(ptr_t Capacity);
    void Alloc_RVM(std::unique_ptr<class RVM>& RVM);
    void Alloc_Macro(void);
    void Alloc_Mem(ptr_t Word_Bits);
};
/*****************************************************************************/
/* __RME_PROC_HPP_CLASSES__ */
#endif
/* __HDR_CLASSES__ */
#endif
}
/* End Classes ***************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
