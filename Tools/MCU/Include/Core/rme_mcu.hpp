/******************************************************************************
Filename    : rme_mcu.hpp
Author      : pry
Date        : 08/04/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The header of the mcu tool.
******************************************************************************/

/* Defines *******************************************************************/
namespace rme_mcu
{
#ifdef __HDR_DEFS__
#ifndef __RME_MCU_HPP_DEFS__
#define __RME_MCU_HPP_DEFS__
/*****************************************************************************/
typedef char s8_t;
typedef short s16_t;
typedef int s32_t;
typedef long long s64_t;
typedef unsigned char u8_t;
typedef unsigned short u16_t;
typedef unsigned int u32_t;
typedef unsigned long long u64_t;
/* Make things compatible in 32-bit or 64-bit environments */
typedef s64_t ret_t;
typedef u64_t ptr_t;

/* EXTERN definition */
#define EXTERN              extern

/* Power of 2 macros */
#define ALIGN_POW(X,POW)    (((X)>>(POW))<<(POW))
#define POW2(POW)           (((ptr_t)1)<<(POW))

/* Capability ID placement */
#define AUTO                ((ptr_t)(-1LL))
#define INVALID             ((ptr_t)(-2LL))
/* Recovery options */
#define RECOVERY_THD        (0)
#define RECOVERY_PROC       (1)
#define RECOVERY_SYS        (2)
/* Option types */
#define OPTION_RANGE        (0)
#define OPTION_SELECT       (1)
/* Failure reporting macros */
#define EXIT_FAIL(Reason) \
do \
{ \
    printf(Reason); \
    printf("\n\n"); \
    Free_All(); \
    exit(-1); \
} \
while(0)
/* Make a directory */
#define MAKE_DIR(BUF,...) \
do \
{ \
    sprintf(BUF,__VA_ARGS__); \
    Make_Dir(BUF); \
} \
while(0);

/* Iteration macros */
#define EACH(Type,Trav,Head) \
(Trav)=(Type)((Head).Next);((struct List*)(Trav))!=&(Head);(Trav)=((Type)(((struct List*)(Trav))->Next))
#define NEXT(Type,Trav)     ((Type)(((struct List*)(Trav))->Next))
#define IS_HEAD(Trav,Head)  (((struct List*)(Trav))==&(Head))

/* The alignment value used when printing macros */
#define MACRO_ALIGNMENT     (56)
/* The code generator author name */
#define CODE_AUTHOR         ("The A7M project generator.")

/* Generic kernel object sizes */
#define CAPTBL_SIZE(NUM,BITS)        ((BITS)/8*8*(NUM))
#define PROC_SIZE(BITS)              ((BITS)/8*8)
#define SIG_SIZE(BITS)               ((BITS)/8*4)

/* Interrupt flag area size (in bytes) */
#define KERNEL_INTF_SIZE             (1024)
/* Entry point slot size (in words) */
#define ENTRY_SLOT_SIZE              (8)

/* Kerneo object size rounding macro */
#define KOTBL_ROUND(SIZE)            (SIZE)
/* Compute the total capability table size when given the macros */
#define CAPTBL_TOTAL(NUM,CAPACITY,BITS)     (((NUM)/(CAPACITY))*KOTBL_ROUND(CAPTBL_SIZE(CAPACITY,(BITS)))+ \
                                             KOTBL_ROUND(CAPTBL_SIZE((NUM)%(CAPACITY),(BITS))))
/*****************************************************************************/
/* __RME_MCU_HPP_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Classes *******************************************************************/
#ifdef __HDR_CLASSES__
#ifndef __RME_MCU_HPP_CLASSES__
#define __RME_MCU_HPP_CLASSES__

/* Use defines in these headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__

/*****************************************************************************/

/* The application instance class */
class Main
{
public:
    std::unique_ptr<Proj> Proj;
    std::unique_ptr<Chip> Chip;
    
    Main(s8_t* Proj, s8_t* Output, s8_t* Format);
    ~Main(void);
};

/* Platform agnostic structures */
/* Compiler information */
struct Comp_Info
{
    /* Optimization level */
	ptr_t Opt;
    /* Priority */
	ptr_t Prio;
};

/* Raw information to be fed to the platform-specific parser */
struct Raw_Info
{
	struct List Head;
    /* Tags */
	s8_t* Tag;
    /* Value of tags */
	s8_t* Val;
};

/* Platform information */
struct Plat_Info
{
    ptr_t Word_Bits;
    ptr_t Captbl_Capacity;
    ptr_t Init_Pgtbl_Num_Ord;
    ptr_t Thd_Size;
    ptr_t Inv_Size;

    void (*Parse_Options)(struct Proj_Info* Proj, struct Chip_Info* Chip);
    void (*Check_Input)(struct Proj_Info* Proj, struct Chip_Info* Chip);
    void (*Align_Mem)(struct Proj_Info* Proj);
    void (*Alloc_Pgtbl)(struct Proj_Info* Proj, struct Chip_Info* Chip);
    ptr_t (*Pgtbl_Size)(ptr_t Num_Order, ptr_t Is_Top);
    void (*Gen_Proj)();

    /* Extra information used by the platform itself */
    void* Extra;
};

/* The memory map information for RME */
struct RME_Memmap_Info
{
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
struct RME_Info
{
    /* Compiler information */
	struct Comp_Info Comp;
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

    /* Raw information about platform, to be deal with by the platform-specific generator */
	struct List Plat;
    /* Raw information about chip, to be deal with by the platform-specific generator */
	struct List Chip;

    /* Final memory map information */
    struct RME_Memmap_Info Map;
};

/* RVM's capability information, from the user processes */
struct RVM_Cap_Info
{
    struct List Head;
    /* What process is this capability in? */
    struct Proc_Info* Proc;
    /* What's the content of the capability, exactly? */
    void* Cap;
};

/* The memory map information for RVM */
struct RVM_Memmap_Info
{   
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
struct RVM_Info
{
    /* Compiler information */
	struct Comp_Info Comp;
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

    /* Global captbl containing captbls */
    ptr_t Captbl_Front;
    struct List Captbl;
    /* Global captbl containing page tables */
    ptr_t Pgtbl_Front;
    struct List Pgtbl;
    /* Global captbl containing processes */
    ptr_t Proc_Front;
    struct List Proc;
    /* Global captbl containing threads */
    ptr_t Thd_Front;
    struct List Thd;
    /* Global captbl containing invocations */
    ptr_t Inv_Front;
    struct List Inv;
    /* Global captbl containing receive endpoints */
    ptr_t Recv_Front;
    struct List Recv;
    /* Global captbl containing kernel endpoints - actually created by kernel itself */
    ptr_t Vect_Front;
    struct List Vect;

    /* Final memory map information */
    struct RVM_Memmap_Info Map;
};

struct Pgtbl_Info
{
    struct List Head;
    /* Is this a top-level? */
    ptr_t Is_Top;
    /* The start address of the page table */
    ptr_t Start_Addr;
    /* The size order */
    ptr_t Size_Order;
    /* The number order */
    ptr_t Num_Order;
    /* The attribute (on this table) */
    ptr_t Attr;
    /* Page directories mapped in */
    struct Pgtbl_Info** Child_Pgdir;
    /* Pages mapped in - if not 0, then attr is directly here */
    ptr_t* Child_Page;
    /* Capability information */
    struct Cap_Info Cap;
};

/* Port information */
struct Port_Info
{
    struct List Head;
    /* The name of the port, unique in a process, and must have
     * a corresponding invocation in the process designated. */
	s8_t* Name;
    /* The process's name */
    s8_t* Proc_Name;

    /* Capability related information */
    struct Cap_Info Cap;
};

/* The option information */
struct Chip_Option_Info
{
    struct List Head;
    /* Name*/
    s8_t* Name;
    /* Type of the option, either range or select */
    ptr_t Type;
    /* Macro of the option */
    s8_t* Macro;
    /* Range of the option */
    s8_t* Range;
};

/* Vector informations */
struct Chip_Vect_Info
{
    struct List Head;
    /* The name of the vector */
	s8_t* Name;
    /* The vector number */
	ptr_t Num;
};

/* Chip information - this is platform independent as well */
struct Chip_Info
{
    /* The name of the chip class */
	s8_t* Class;
    /* Compatible chip list */
	s8_t* Compat;
    /* The vendor */
    s8_t* Vendor;
    /* The platform */
	s8_t* Plat;
    /* The number of CPU cores */
	ptr_t Cores;
    /* The number of MPU regions */
    ptr_t Regions;
    /* The platform-specific attributes to be passed to the platform-specific generator */
    struct List Attr;
    /* Memory information */
	struct List Code;
	struct List Data;
	struct List Device;
    /* Raw option information */
	struct List Option;
    /* Interrupt vector information */
	struct List Vect;
};

/* Memory map */
struct Mem_Map_Info
{
    struct List Head;
    /* The memory information itself */
    struct Mem_Info* Mem;
    /* The bitmap of the memory trunk, aligned on 32-byte boundary */
    u8_t* Bitmap;
};

/* Memory map information - min granularity 4B */
struct Mem_Map
{
    struct List Chip_Mem;
    /* The exact list of these unallocated requirements */
    struct List Proc_Mem;
};
/*****************************************************************************/
/* __RME_MCU_HPP_CLASSES__ */
#endif
/* __HDR_CLASSES__ */
#endif
}
/* End C++ Classes ***********************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
