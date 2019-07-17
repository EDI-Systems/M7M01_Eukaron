/******************************************************************************
Filename    : rme_mcu.h
Author      : pry
Date        : 08/04/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The header of the mcu tool.
******************************************************************************/

/* Defines *******************************************************************/
#ifdef __HDR_DEFS__
#ifndef __RME_MCU_H_DEFS__
#define __RME_MCU_H_DEFS__
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
/* Optimization levels */
#define OPT_O0              (0)
#define OPT_O1              (1)
#define OPT_O2              (2)
#define OPT_O3              (3)
#define OPT_OS              (4)
/* Time or size optimization choice */
#define PRIO_SIZE           (0)
#define PRIO_TIME           (1)
/* Capability ID placement */
#define AUTO                ((ptr_t)(-1LL))
#define INVALID             ((ptr_t)(-2LL))
/* Recovery options */
#define RECOVERY_THD        (0)
#define RECOVERY_PROC       (1)
#define RECOVERY_SYS        (2)
/* Memory access permissions */
#define MEM_READ            POW2(0)
#define MEM_WRITE           POW2(1)
#define MEM_EXECUTE         POW2(2)
#define MEM_BUFFERABLE      POW2(3)
#define MEM_CACHEABLE       POW2(4)
#define MEM_STATIC          POW2(5)
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
/* __RME_MCU_H_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Structs *******************************************************************/
#ifdef __HDR_STRUCTS__
#ifndef __RME_MCU_H_STRUCTS__
#define __RME_MCU_H_STRUCTS__

/* Use defines in these headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__

/*****************************************************************************/
/* List head structure */
struct List
{
	struct List* Prev;
	struct List* Next;
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

/* Memory segment information */
struct Mem_Info
{
    struct List Head;
    /* The start address */
	ptr_t Start;
    /* The size */
	ptr_t Size;
    /* The attributes - read, write, execute, cacheable, bufferable, static */
	ptr_t Attr;
    /* The alignment granularity */
    ptr_t Align;
};

/* Capability information - not all fields used for every capability */
struct Cap_Info
{
    /* The local capid of the port */
    ptr_t Loc_Capid;
    /* The global linear capid of the endpoint */
    ptr_t RVM_Capid;
    /* The macro denoting the global capid */
    s8_t* Loc_Macro;
    /* The macro denoting the global capid */
    s8_t* RVM_Macro;
    /* The macro denoting the global capid - for RME */
    s8_t* RME_Macro;
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

/* Thread memory map information */
struct Thd_Memmap_Info
{
    /* Position of the entry address from the header */
    ptr_t Entry_Addr;
    /* Value of the parameter at creation time */
    ptr_t Param_Value;
    /* The address of the stack */
    ptr_t Stack_Base;
    /* The size of the stack */
    ptr_t Stack_Size;
};

/* Thread information */
struct Thd_Info
{
    struct List Head;
    /* Name of the thread, unique in a process */
	s8_t* Name;
    /* The entry of the thread */
	s8_t* Entry;
    /* The stack size of the thread */
	ptr_t Stack_Size;
    /* The parameter passed to the thread */
	s8_t* Parameter;
    /* The priority of the thread */
	ptr_t Priority;

    /* Capability related information */
    struct Cap_Info Cap;

    /* Memory map information */
    struct Thd_Memmap_Info Map;
};

/* Invocation memory map information */
struct Inv_Memmap_Info
{
    /* Address of the entry */
    ptr_t Entry_Addr;
    /* Address of the stack */
    ptr_t Stack_Base;
    /* The size of the stack */
    ptr_t Stack_Size;
};

/* Invocation information */
struct Inv_Info
{
    struct List Head;
    /* The name of the invocation, unique in a process */
	s8_t* Name;
    /* The entry address of the invocation */
	s8_t* Entry;
    /* The stack size of the invocation */
	ptr_t Stack_Size;

    /* Capability related information */
    struct Cap_Info Cap;

    /* Memory map information */
    struct Inv_Memmap_Info Map;
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

/* Receive endpoint information */
struct Recv_Info
{
    struct List Head;
    /* The name of the receive endpoint, unique in a process */
	s8_t* Name;

    /* Capability related information */
    struct Cap_Info Cap;
};

/* Send endpoint information */
struct Send_Info
{
    struct List Head;
    /* The name of the send endpoint, unique in a process, and must 
     * have a corresponding receive endpoint in the process designated. */
	s8_t* Name;
    /* The process's name, only useful for send endpoints */
    s8_t* Proc_Name;

    /* Capability related information */
    struct Cap_Info Cap;
};

/* Vector endpoint information */
struct Vect_Info
{
    struct List Head;
    /* Globally unique vector name */
	s8_t* Name;
    /* Vector number */
    ptr_t Num;

    /* Capability related information */
    struct Cap_Info Cap;
};

struct Proc_Memmap_Info
{
    /* Code memory */
    ptr_t Code_Base;
    ptr_t Code_Size;
    /* Data memory */
    ptr_t Data_Base;
    ptr_t Data_Size;
    /* Code memory frontier for entries */
    ptr_t Entry_Code_Front;
};

/* Process information */
struct Proc_Info
{
    struct List Head;
    /* Name of the process */
    s8_t* Name;
    /* Extra first level captbl capacity required */
	ptr_t Extra_Captbl;
    /* Current local capability table frontier */ 
    ptr_t Captbl_Front;
    /* Page table information */
    struct Pgtbl_Info* Pgtbl;
    /* Compiler information */
	struct Comp_Info Comp;
    /* Memory trunk information */
	struct List Code;
	struct List Data;
	struct List Device;
    /* Kernel object information */
	struct List Thd;
	struct List Inv;
	struct List Port;
	struct List Recv;
	struct List Send;
	struct List Vect;

    /* Capability information for itself */
    struct Cap_Info Captbl_Cap;
    struct Cap_Info Proc_Cap;

    struct Proc_Memmap_Info Map;
};

/* Whole project information */
struct Proj_Info
{
    /* The name of the project */
	s8_t* Name;
    /* The platform used */
    s8_t* Plat_Name;
    /* The all-lower-case of the platform used */
    s8_t* Lower_Plat;
    /* The chip class used */
	s8_t* Chip_Class;
    /* The full name of the exact chip used */
    s8_t* Chip_Full;
    /* The platform information */
    struct Plat_Info Plat;
    /* The RME kernel information */
	struct RME_Info RME;
    /* The RVM user-library information */
	struct RVM_Info RVM;
    /* The process information */
	struct List Proc;
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
/* __RME_MCU_H_STRUCTS__ */
#endif
/* __HDR_STRUCTS__ */
#endif
/* End Structs ***************************************************************/

/* Private Global Variables **************************************************/
#if(!(defined __HDR_DEFS__||defined __HDR_STRUCTS__))
#ifndef __RME_MCU_MEMBERS__
#define __RME_MCU_MEMBERS__

/* In this way we can use the data structures and definitions in the headers */
#define __HDR_DEFS__

#undef __HDR_DEFS__

#define __HDR_STRUCTS__

#undef __HDR_STRUCTS__

/* If the header is not used in the public mode */
#ifndef __HDR_PUBLIC_MEMBERS__
/*****************************************************************************/
/* The list containing all memory allocated */
static struct List Mem_List;
/*****************************************************************************/
/* End Private Global Variables **********************************************/

/* Private C Function Prototypes *********************************************/
static void Cmdline_Proc(int argc, char* argv[], s8_t** Input_File, s8_t** Output_Path,
                         s8_t** RME_Path, s8_t** RVM_Path, s8_t** Format);

static s8_t* Read_XML(s8_t* Path);

static void Parse_Compiler(struct Comp_Info* Comp, xml_node_t* Node);
static void Parse_Proj_RME(struct RME_Info* RME, xml_node_t* Node);
static void Parse_Proj_RVM(struct RVM_Info* RVM, xml_node_t* Node);
static void Parse_Proc_Mem(struct Proc_Info* Proc, xml_node_t* Node);
static void Parse_Proc_Thd(struct Proc_Info* Proc, xml_node_t* Node);
static void Parse_Proc_Inv(struct Proc_Info* Proc, xml_node_t* Node);
static void Parse_Proc_Port(struct Proc_Info* Proc, xml_node_t* Node);
static void Parse_Proc_Recv(struct Proc_Info* Proc, xml_node_t* Node);
static void Parse_Proc_Send(struct Proc_Info* Proc, xml_node_t* Node);
static void Parse_Proc_Vect(struct Proc_Info* Proc, xml_node_t* Node);
static void Parse_Proj_Proc(struct Proj_Info* Proj, xml_node_t* Node);
static struct Proj_Info* Parse_Proj(s8_t* Proj_File);

static void Parse_Chip_Mem(struct Chip_Info* Chip, xml_node_t* Node);
static void Parse_Chip_Option(struct Chip_Info* Chip, xml_node_t* Node);
static void Parse_Chip_Vect(struct Chip_Info* Chip, xml_node_t* Node);
static struct Chip_Info* Parse_Chip(s8_t* Chip_File);

static ret_t Try_Bitmap(s8_t* Bitmap, ptr_t Start, ptr_t Size);
static void Mark_Bitmap(s8_t* Bitmap, ptr_t Start, ptr_t Size);
static ret_t Fit_Static_Mem(struct Mem_Map* Map, ptr_t Start, ptr_t Size);
static ret_t Fit_Auto_Mem(struct Mem_Map* Map, struct Mem_Info* Mem);
static ret_t Compare_Addr(struct List* First, struct List* Second);
static ret_t Compare_Size(struct List* First, struct List* Second);

static void Alloc_Code(struct Proj_Info* Proj, struct Chip_Info* Chip);
static void Check_Code(struct Proj_Info* Proj, struct Chip_Info* Chip);
static void Alloc_Data(struct Proj_Info* Proj, struct Chip_Info* Chip);

static void Check_Device(struct Proj_Info* Proj, struct Chip_Info* Chip);
static void Check_Input(struct Proj_Info* Proj, struct Chip_Info* Chip);
static void Check_Name(s8_t* Name);
static void Check_Vect(struct Proj_Info* Proj);
static void Check_Conflict(struct Proj_Info* Proj);

static void Alloc_Local_Capid(struct Proj_Info* Proj);
static void Alloc_Global_Capid(struct Proj_Info* Proj);
static void Alloc_Capid_Macros(struct Proj_Info* Proj);
static void Backprop_Global_Capid(struct Proj_Info* Proj, struct Chip_Info* Chip);
static void Alloc_Captbl(struct Proj_Info* Proj,  struct Chip_Info* Chip);

static void Get_Kmem_Size(struct Proj_Info* Proj, ptr_t Capacity, ptr_t Init_Captbl_Size);
static void Alloc_RME_Kmem(struct Proj_Info* Proj);
static void Alloc_RVM_Mem(struct Proj_Info* Proj);
static void Alloc_Proc_Mem(struct Proj_Info* Proj);
static void Alloc_Mem(struct Proj_Info* Proj);

static void Setup_RME_Folder(struct Proj_Info* Proj, struct Chip_Info* Chip, s8_t* RME_Path, s8_t* Output_Path);
static void Setup_RVM_Folder(struct Proj_Info* Proj, struct Chip_Info* Chip, s8_t* RVM_Path, s8_t* Output_Path);

static void Setup_RME_Conf(struct Proj_Info* Proj, struct Chip_Info* Chip, s8_t* RME_Path, s8_t* Output_Path);
static void Setup_RVM_Conf(struct Proj_Info* Proj, struct Chip_Info* Chip, s8_t* RVM_Path, s8_t* Output_Path);

static void Print_RME_Inc(FILE* File, struct Proj_Info* Proj);
static void Gen_RME_Boot(struct Proj_Info* Proj, struct Chip_Info* Chip, s8_t* RME_Path, s8_t* Output_Path);
static void Gen_RME_User(struct Proj_Info* Proj, struct Chip_Info* Chip, s8_t* RME_Path, s8_t* Output_Path);
static void Print_RVM_Inc(FILE* File, struct Proj_Info* Proj);
static void Cons_RVM_Pgtbl(FILE* File, struct Pgtbl_Info* Pgtbl);
static void Map_RVM_Pgtbl(FILE* File, struct Pgtbl_Info* Pgtbl, ptr_t Init_Size_Ord);
static void Init_RVM_Pgtbl(FILE* File, struct Proj_Info* Proj);
static void Gen_RVM_Boot(struct Proj_Info* Proj, struct Chip_Info* Chip, s8_t* RVM_Path, s8_t* Output_Path);
static void Gen_RVM_User(struct Proj_Info* Proj, struct Chip_Info* Chip, s8_t* RVM_Path, s8_t* Output_Path);
/*****************************************************************************/
#define __EXTERN__
/* End Private C Function Prototypes *****************************************/

/* Public Global Variables ***************************************************/
/* __HDR_PUBLIC_MEMBERS__ */
#else
#define __EXTERN__ EXTERN 
/* __HDR_PUBLIC_MEMBERS__ */
#endif

/*****************************************************************************/

/*****************************************************************************/

/* End Public Global Variables ***********************************************/

/* Public C Function Prototypes **********************************************/
__EXTERN__ void* Memcpy(void* Dst, void* Src, xml_ptr_t Num);
__EXTERN__ ret_t Strncmp(s8_t* Str1, s8_t* Str2, ptr_t Num);
__EXTERN__ ptr_t Strlen(s8_t* Str);

__EXTERN__ void List_Crt(volatile struct List* Head);
__EXTERN__ void List_Del(volatile struct List* Prev,volatile struct List* Next);
__EXTERN__ void List_Ins(volatile struct List* New,
                         volatile struct List* Prev,
                         volatile struct List* Next);

__EXTERN__ void Free_All(void);
__EXTERN__ void* Malloc(ptr_t Size);
__EXTERN__ void Free(void* Addr);

__EXTERN__ ret_t Dir_Present(s8_t* Path);
__EXTERN__ ret_t Dir_Empty(s8_t* Path);
__EXTERN__ ptr_t Get_Size(s8_t* Path);
__EXTERN__ void Make_Dir(s8_t* Path);
__EXTERN__ void Copy_File(s8_t* Dst, s8_t* Src);
__EXTERN__ u8_t* Read_File(s8_t* Path);
__EXTERN__ void Lower_Case(s8_t* Str);

__EXTERN__ void Merge_Sort(struct List* List, ret_t (*Compare)(struct List*, struct List*));

__EXTERN__ ret_t Strcicmp(s8_t* Str1, s8_t* Str2);
__EXTERN__ s8_t* Make_Macro(s8_t* Str1, s8_t* Str2, s8_t* Str3, s8_t* Str4);
__EXTERN__ s8_t* Make_Str(s8_t* Str1, s8_t* Str2);
__EXTERN__ s8_t* Raw_Match(struct List* Raw, s8_t* Tag);

__EXTERN__ void Write_Src_Desc(FILE* File, s8_t* Filename, s8_t* Description);
__EXTERN__ void Write_Src_Footer(FILE* File);
__EXTERN__ void Write_Func_Desc(FILE* File, s8_t* Funcname);
__EXTERN__ void Write_Func_None(FILE* File);
__EXTERN__ void Write_Func_Footer(FILE* File, s8_t* Funcname);
__EXTERN__ void Make_Define_Str(FILE* File, s8_t* Macro, s8_t* Value, ptr_t Align);
__EXTERN__ void Make_Define_Int(FILE* File, s8_t* Macro, ptr_t Value, ptr_t Align);
__EXTERN__ void Make_Define_Hex(FILE* File, s8_t* Macro, ptr_t Value, ptr_t Align);
/*****************************************************************************/
/* Undefine "__EXTERN__" to avoid redefinition */
#undef __EXTERN__
/* __RME_MCU_MEMBERS__ */
#endif
/* !(defined __HDR_DEFS__||defined __HDR_STRUCTS__) */
#endif
/* End Public C Function Prototypes ******************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
