/******************************************************************************
Filename    : rme_mcu.c
Author      : pry
Date        : 20/04/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The configuration generator for the MCU ports. This does not
              apply to the desktop or mainframe port; it uses its own generator.
			  This generator includes 12 big steps, and is considerably complex.
               1. Process the command line arguments and figure out where the source
                  are located at.
               2. Read the project-level configuration XMLs and device-level 
                  configuration XMLs into its internal data structures. Should
                  we find any parsing errors, we report and error out.
               3. Align memory. For program memory and data memory, rounding their
                  size is allowed; for specifically pointed out memory, rounding
                  their size is not allowed. 
               4. Generate memory map. This places all the memory segments into
                  the memory map, and fixes their specific size, etc. 
               5. Check if the generated memory map is valid. Each process should have
                  at least one code section and one data section, and they shall all
                  be STATIC.
               6. Allocate local and global linear capability IDs for all kernel objects.
                  The global linear capability ID assumes that all capability in the
                  same class are in the same capability table, and in 32-bit systems
                  this may not be the case.
               7. Set up the folder structure of the project so that the port-specific
                  generators can directly use them.
               8. Call the port-level generator to generate the project and port-specific
                  files for the project.
                  1. Detect any errors in the configuration structure. If any is found,
                     error out.
                  2. Allocate the page table contents and allocate capid/macros for them.
                  3. Call the tool-level project generator to generate project files.
                     Should the tool have any project group or workspace creation capability,
                     create the project group or workspace.
                     Memory map and linker file is also generated in this phase. 
                     1. The generator should generate separate projects for the RME.
                     2. Then generates project for RVM. 
                     3. And generates project for all other processes.
               9. Generate the vector creation scripts for RME.
              10. Generate the kernel object creation and delegation scripts for RVM.
                  1. Generate the capability tables.
                  2. Generate the page tables, calls the port-specific generator callback.
                  3. Generate the processes.
                  4. Generate the threads.
                  5. Generate the invocations.
                  6. Generate the receive endpoints.
                  7. Generate the delegation scripts.
              11. Generate stubs for all processes.
              12. Report to the user that the project generation is complete.
******************************************************************************/

/* Includes ******************************************************************/
/* Kill CRT warnings for MS. This also relies on Shlwapi.lib, remember to add it */
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "stdio.h"
#include "memory.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"

#if(defined _WIN32)
#include "Windows.h"
#include "shlwapi.h"
#elif(defined linux)
#include <dirent.h>
#include <errno.h>
#else
#error "The target platform is not supported. Please compile on Windows or Linux."
#endif
/* End Includes **************************************************************/

/* Defines *******************************************************************/
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
/* Memory types */
#define MEM_CODE            (0)
#define MEM_DATA            (1)
#define MEM_DEVICE          (2)
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
/* Extract the content of the next label */
#define GET_NEXT_LABEL(START, END, LABEL_START, LABEL_END, VAL_START, VAL_END, NAME) \
do \
{ \
    if(XML_Get_Next((START),(END),&(LABEL_START),&(LABEL_END),&(VAL_START),&(VAL_END))!=0) \
	{ \
        printf("%s", (NAME)); \
        EXIT_FAIL(" label is malformed."); \
    } \
    if(Compare_Label((LABEL_START), (LABEL_END), (NAME))!=0) \
	{ \
        printf("%s", (NAME)); \
        EXIT_FAIL(" label not found."); \
    } \
    Start=Val_End; \
} \
while(0)

/* The alignment value used when printing macros */
#define MACRO_ALIGNMENT     (56)
/* The code generator author name */
#define CODE_AUTHOR         ("The A7M project generator.")
/* End Defines ***************************************************************/

/* Typedefs ******************************************************************/
typedef char s8;
typedef short s16;
typedef int s32;
typedef long long s64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
/* Make things compatible in 32-bit or 64-bit environments */
typedef s64 ret_t;
typedef u64 ptr_t;
/* End Typedefs **************************************************************/

/* Structs *******************************************************************/
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
    /* Number of tags */
	ptr_t Num;
    /* Tags */
	s8** Tag;
    /* Value of tags */
	s8** Value;
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
    /* Extra amount of kernel memory */
	ptr_t Extra_Kmem;
    /* Slot order of kernel memory */
	ptr_t Kmem_Order;
    /* Priorities supported */
	ptr_t Kern_Prios;
    /* Raw information about platform, to be deal with by the platform-specific generator */
	struct Raw_Info Plat_Raw;
    /* Raw information about chip, to be deal with by the platform-specific generator */
	struct Raw_Info Chip_Raw;
};
/* RVM's capability information, from the user processes */
struct RVM_Cap_Info
{
    /* What process is this capability in? */
    struct Proc_Info* Proc;
    /* What's the content of the capability, exactly? */
    void* Cap;
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
    /* The extra amount in the main capability table */
	ptr_t Extra_Captbl;
    /* The recovery mode - by thread, process or the whole system? */
	ptr_t Recovery;
    /* Global captbl containing captbls */
    ptr_t Captbl_Captbl_Front;
    struct RVM_Cap_Info* Captbl_Captbl;
    /* Global captbl containing processes */
    ptr_t Proc_Captbl_Front;
    struct RVM_Cap_Info* Proc_Captbl;
    /* Global captbl containing threads */
    ptr_t Thd_Captbl_Front;
    struct RVM_Cap_Info* Thd_Captbl;
    /* Global captbl containing invocations */
    ptr_t Inv_Captbl_Front;
    struct RVM_Cap_Info* Inv_Captbl;
    /* Global captbl containing receive endpoints */
    ptr_t Recv_Captbl_Front;
    struct RVM_Cap_Info* Recv_Captbl;
    /* Global captbl containing kernel endpoints - actually created by kernel itself */
    ptr_t Vector_Captbl_Front;
    struct RVM_Cap_Info* Vector_Captbl;
};
/* Memory segment information */
struct Mem_Info
{
    /* The start address */
	ptr_t Start;
    /* The size */
	ptr_t Size;
    /* The type - code, data or device */
	ptr_t Type;
    /* The attributes - read, write, execute, cacheable, bufferable, static */
	ptr_t Attr;
    /* The alignment granularity */
    ptr_t Align;
};
/* Thread information */
struct Thd_Info
{
    /* Name of the thread, unique in a process */
	s8* Name;
    /* The local capability ID */
    ptr_t Capid;
    /* The macro denoting the local capability ID */
    s8* Capid_Macro;
    /* The global linear capability ID */
    ptr_t RVM_Capid;
    /* The macro denoting the global capability ID */
    s8* RVM_Capid_Macro;
    /* The entry of the thread */
	s8* Entry;
    /* The stack address of the thread */
	ptr_t Stack_Addr;
    /* The stack size of the thread */
	ptr_t Stack_Size;
    /* The parameter passed to the thread */
	s8* Parameter;
    /* The priority of the thread */
	ptr_t Priority;
};
/* Invocation information */
struct Inv_Info
{
    /* The name of the invocation, unique in a process */
	s8* Name;
    /* The local capid */
    ptr_t Capid;
    /* The macro denoting the local capid */
    s8* Capid_Macro;
    /* The global linear capid of the invocation */
    ptr_t RVM_Capid;
    /* The macro denoting the global capid */
    s8* RVM_Capid_Macro;
    /* The entry address of the invocation */
	s8* Entry;
    /* The stack address of the invocation */
	ptr_t Stack_Addr;
    /* The stack size of the invocation */
	ptr_t Stack_Size;
};
/* Port information */
struct Port_Info
{
    /* The name of the port, unique in a process, and must have
     * a corresponding invocation in the process designated. */
	s8* Name;
    /* The local capid of the port */
    ptr_t Capid;
    /* The macro denoting the global capid */
    s8* Capid_Macro;
    /* The global linear capid of the port */
    ptr_t RVM_Capid;
    /* The macro denoting the global capid */
    s8* RVM_Capid_Macro;
    /* The process's name */
    s8* Process;
};
/* Receive endpoint information */
struct Recv_Info
{
    /* The name of the receive endpoint, unique in a process */
	s8* Name;
    /* The local capid of the port */
    ptr_t Capid;
    /* The macro denoting the global capid */
    s8* Capid_Macro;
    /* The global linear capid of the endpoint */
    ptr_t RVM_Capid;
    /* The macro denoting the global capid */
    s8* RVM_Capid_Macro;
};
/* Send endpoint information */
struct Send_Info
{
    /* The name of the send endpoint, unique in a process, and must 
     * have a corresponding receive endpoint in the process designated. */
	s8* Name;
    /* The local capid of the port */
    ptr_t Capid;
    /* The macro denoting the global capid */
    s8* Capid_Macro;
    /* The global linear capid of the endpoint */
    ptr_t RVM_Capid;
    /* The macro denoting the global capid */
    s8* RVM_Capid_Macro;
    /* The process's name, only useful for send endpoints */
    s8* Process;
};
/* Vector endpoint information */
struct Vect_Info
{
    /* The name of the vector endpoint, globally unique, and must have
     * a corresponding interrupt vector designated. */
	s8* Name;
    /* The local capid of the port */
    ptr_t Capid;
    /* The macro denoting the global capid */
    s8* Capid_Macro;
    /* The global linear capid of the endpoint */
    ptr_t RVM_Capid;
    /* The macro denoting the global capid */
    s8* RVM_Capid_Macro;
    /* The macro denoting the global capid - for RME */
    s8* RME_Capid_Macro;
    /* The vector number */
    ptr_t Number;
};
/* Process information */
struct Proc_Info
{
    /* The name of the process */
    s8* Name;
    /* The global linear capid of the process */
    ptr_t RVM_Proc_Capid;
    /* The macro denoting the linear capid */
    s8* RVM_Proc_Capid_Macro;
    /* The global linear capid of the process's captbl */
    ptr_t RVM_Captbl_Capid;
    /* The macro denoting the linear capid */
    s8* RVM_Captbl_Capid_Macro;
    /* The extra first level captbl capacity required */
	ptr_t Extra_Captbl;
    /* The current local capability table frontier */ 
    ptr_t Captbl_Front;
    /* The compiler information */
	struct Comp_Info Comp;
    /* The memory trunk information */
	ptr_t Mem_Num;
	struct Mem_Info* Mem;
    /* The thread information */
	ptr_t Thd_Num;
	struct Thd_Info* Thd;
    /* The invocation information */
	ptr_t Inv_Num;
	struct Inv_Info* Inv;
    /* The port information */
	ptr_t Port_Num;
	struct Port_Info* Port;
    /* The receive endpoint information */
	ptr_t Recv_Num;
	struct Recv_Info* Recv;
    /* The receive endpoint information */
	ptr_t Send_Num;
	struct Send_Info* Send;
    /* The vector endpoint information */
	ptr_t Vect_Num;
	struct Vect_Info* Vect;
};
/* Whole project information */
struct Proj_Info
{
    /* The name of the project */
	s8* Name;
    /* The platform used */
    s8* Platform;
    /* The chip class used */
	s8* Chip;
    /* The full name of the exact chip used */
    s8* Fullname;
    /* The RME kernel information */
	struct RME_Info RME;
    /* The RVM user-library information */
	struct RVM_Info RVM;
    /* The process information */
	ptr_t Proc_Num;
	struct Proc_Info* Proc;
};
/* Chip option macro informations */
struct Option_Info
{
    /* The name of this option */
	s8* Name;
    /* The type of the option - selection or range */
	ptr_t Type;
    /* The corresponding macro */
	s8* Macro;
    /* Only one of these will take effect */
    /* Minimum and maximum range */
    ptr_t Range_Min;
    ptr_t Range_Max;
    /* Select options */
    ptr_t Select_Num;
	s8** Select_Opt;
};
/* Vector informations */
struct Chip_Vect_Info
{
    /* The name of the vector */
	s8* Name;
    /* The vector number */
	ptr_t Number;
};
/* Chip information - this is platform independent as well */
struct Chip_Info
{
    /* The name of the chip class */
	s8* Name;
    /* The vendor */
    s8* Vendor;
    /* The platform */
	s8* Platform;
    /* The number of CPU cores */
	ptr_t Cores;
    /* The number of MPU regions */
    ptr_t Regions;
    /* The platform-specific attributes to be passed to the platform-specific generator */
    struct Raw_Info Attr_Raw;
    /* Memory information */
	ptr_t Mem_Num;
	struct Mem_Info* Mem;
    /* Option information */
	ptr_t Option_Num;
	struct Option_Info* Option;
    /* Interrupt vector information */
	ptr_t Vect_Num;
	struct Chip_Vect_Info* Vect;
};

/* Memory map information - min granularity 4B */
struct Mem_Map
{
    /* Number of memory trunks */
    ptr_t Mem_Num;
    /* Bitmaps of these memory trunks */
    s8** Mem_Bitmap;
    /* The memory trunk list */
    struct Mem_Info** Mem_Array;
    /* The unallocated memory requirements in all processes */
    ptr_t Proc_Mem_Num;
    /* The exact list of these unallocated requirements */
    struct Mem_Info** Proc_Mem_Array;
};

/* The capability and kernel memory information supplied by the port-specific generator */
struct Cap_Alloc_Info
{
    /* Processor bits */
    ptr_t Processor_Bits;
    /* Kernel memory base */
    ptr_t Kmem_Abs_Base;
    /* When we go into creating the kernel endpoints */
    ptr_t Cap_Vect_Front;
    ptr_t Kmem_Vect_Front;
    /* When we go into creating capability tables */
    ptr_t Cap_Captbl_Front;
    ptr_t Kmem_Captbl_Front;
    /* When we go into creating page tables */
    ptr_t Cap_Pgtbl_Front;
    ptr_t Kmem_Pgtbl_Front;
    /* When we go into creating processes */
    ptr_t Cap_Proc_Front;
    ptr_t Kmem_Proc_Front;
    /* When we go into creating threads */
    ptr_t Cap_Thd_Front;
    ptr_t Kmem_Thd_Front;
    /* When we go into creating invocations */
    ptr_t Cap_Inv_Front;
    ptr_t Kmem_Inv_Front;
    /* When we go into creating receive endpoints */
    ptr_t Cap_Recv_Front;
    ptr_t Kmem_Recv_Front;
    /* After the booting all finishes */ 
    ptr_t Cap_Boot_Front;
    ptr_t Kmem_Boot_Front;
    /* The header file name, for RME and RVM */
    s8* RME_Plat_Header;
    s8* RVM_Plat_Header;
};
/* End Structs ***************************************************************/

/* Global Variables **********************************************************/
/* The list containing all memory allocated */
struct List Mem_List;
/* End Global Variables ******************************************************/

/* Begin Function:List_Crt ****************************************************
Description : Create a doubly linkled list.
Input       : volatile struct List* Head - The pointer to the list head.
Output      : None.
Return      : None.
******************************************************************************/
void List_Crt(volatile struct List* Head)
{
    Head->Prev=(struct List*)Head;
    Head->Next=(struct List*)Head;
}
/* End Function:List_Crt *****************************************************/

/* Begin Function:List_Del ****************************************************
Description : Delete a node from the doubly-linked list.
Input       : volatile struct RMP_List* Prev - The prevoius node of the target node.
              volatile struct RMP_List* Next - The next node of the target node.
Output      : None.
Return      : None.
******************************************************************************/
void List_Del(volatile struct List* Prev,volatile struct List* Next)
{
    Next->Prev=(struct List*)Prev;
    Prev->Next=(struct List*)Next;
}
/* End Function:List_Del *****************************************************/

/* Begin Function:List_Ins ****************************************************
Description : Insert a node to the doubly-linked list.
Input       : volatile struct List* New - The new node to insert.
              volatile struct List* Prev - The previous node.
              volatile struct List* Next - The next node.
Output      : None.
Return      : None.
******************************************************************************/
void List_Ins(volatile struct List* New,
              volatile struct List* Prev,
              volatile struct List* Next)
{
    Next->Prev=(struct List*)New;
    New->Next=(struct List*)Next;
    New->Prev=(struct List*)Prev;
    Prev->Next=(struct List*)New;
}
/* End Function:List_Ins *****************************************************/

/* Begin Function:Malloc ******************************************************
Description : Allocate some memory and register it with the system.
Input       : ptr_t Size - The size to allocate, in bytes.
Output      : None.
Return      : void* - If successful, the address; else 0.
******************************************************************************/
void* Malloc(ptr_t Size)
{
	struct List* Addr;

	/* See if the allocation is successful */
	Addr=malloc((size_t)(Size+sizeof(struct List)));
	if(Addr==0)
		return 0;

	/* Insert into the queue */
	List_Ins(Addr,&Mem_List,Mem_List.Next);

	return &Addr[1];
}
/* End Function:Malloc *******************************************************/

/* Begin Function:Free ********************************************************
Description : Deallocate the memory and deregister it.
Input       : void* Addr - The address to free.
Output      : None.
Return      : None.
******************************************************************************/
void Free(void* Addr)
{
	struct List* List;
	
	/* Get the memory block and deregister it in the queue */
	List=(struct List*)Addr;
	List=List-1;
	List_Del(List->Prev,List->Next);
	free(List);
}
/* End Function:Free *********************************************************/

/* Begin Function:Free_All ****************************************************
Description : We encountered a failure somewhere and need to free all memory allocated
              so far.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void Free_All(void)
{
	struct List* Ptr;

	while(Mem_List.Next!=&Mem_List)
	{
		Ptr=Mem_List.Next;
		List_Del(Ptr->Prev,Ptr->Next);
		free(Ptr);
	}
}
/* End Function:Free_All *****************************************************/

/* Begin Function:Dir_Present *************************************************
Description : Figure out whether the directoy is present.
Input       : s8* Path - The path to the directory.
Output      : None.
Return      : ret_t - 0 for present, -1 for non-present.
******************************************************************************/
ret_t Dir_Present(s8* Path)
{
#ifdef _WIN32
    u32 Attr;
    Attr=PathIsDirectory(Path);
    if(Attr!=0)
        return 0;
    else
        return -1;
#else
    DIR* Dir;
    Dir=opendir(Path);
    if(Dir!=0)
    {
        closedir(Dir);
        return 0;
    }
    else
        return -1;
#endif
}
/* End Function:Dir_Present **************************************************/

/* Begin Function:Dir_Empty ***************************************************
Description : Figure out whether the directoy is empty. When using this function,
              the directory must be present.
Input       : s8* Path - The path to the directory.
Output      : None.
Return      : ret_t - 0 for empty, -1 for non-empty.
******************************************************************************/
ret_t Dir_Empty(s8* Path)
{
#ifdef _WIN32
    u32 Attr;
    Attr=PathIsDirectoryEmpty(Path);
    if(Attr!=0)
        return 0;
    else
        return -1;
#else
    DIR* Dir;
    dirent* File;
    Dir=opendir(Path);
    if(Dir==0)
        return -1;

    while((File=readdir(Dir))!=NULL)
    {
        n++;
        if(n>2)
        {
            closedir(Dir);
            return -1;
        }
    }

    closedir(dir);
    return 0;
#endif
}
/* End Function:Dir_Empty ****************************************************/

/* Begin Function:Make_Dir ****************************************************
Description : Create a directory if it does not exist.
Input       : s8* Path - The path to the directory.
Output      : None.
Return      : ret_t - 0 for successful, -1 for failure.
******************************************************************************/
ret_t Make_Dir(s8* Path)
{
    if(Dir_Present(Path)==0)
        return 0;

#ifdef _WIN32
    if(CreateDirectory(Path, NULL)!=0)
        return 0;
#else
    if(mkdir(Path, S_IRWXU)==0)
        return 0;
#endif

    return -1;
}
/* End Function:Make_Dir *****************************************************/

/* Begin Function:Copy_File ***************************************************
Description : Copy a file from some position to another position. If the file
              exists, we need to overwrite it with the new files.
Input       : s8* Dst - The destination path.
              s8* Src - The source path.
Output      : None.
Return      : ret_t - 0 for successful, -1 for failure.
******************************************************************************/
ret_t Copy_File(s8* Dst, s8* Src)
{
    FILE* Dst_File;
    FILE* Src_File;
    s8 Buf[128];
    ptr_t Size;

    Src_File=fopen(Src, "rb");
    if(Src_File==0)
        return -1;
    /* This will wipe the contents of the file */
    Dst_File=fopen(Dst, "wb");
    if(Dst_File==0)
    {
        fclose(Src_File);
        return -1;
    }

    Size=fread(Buf, 1, 128, Src_File);
    while (Size!=0) {
        fwrite(Buf, 1, (size_t)Size, Dst_File);
        Size=fread(Buf, 1, 128, Src_File);
    }

    fclose(Src_File);
    fclose(Dst_File);

    return 0;
}
/* End Function:Copy_File ****************************************************/

/* Begin Function:Cmdline_Proc ************************************************
Description : Preprocess the input parameters, and generate a preprocessed
              instruction listing with all the comments stripped.
Input       : int argc - The number of arguments.
              char* argv[] - The arguments.
Output      : s8** Input_File - The input project file path.
              s8** Output_File - The output folder path, must be empty.
			  s8** RME_Path - The RME root folder path, must contain RME files.
			  s8** RVM_Path - The RME root folder path, must contain RME files.
			  s8** Format - The output format.
Return      : None.
******************************************************************************/
void Cmdline_Proc(int argc,char* argv[], s8** Input_File, s8** Output_Path,
                  s8** RME_Path, s8** RVM_Path, s8** Format)
{
    ptr_t Count;

    if(argc!=11)
        EXIT_FAIL("Too many or too few input parameters.\n"
                  "Usage: -i input.xml -o output_path -k rme_root -u rvm_root -f format.\n"
                  "       -i: Project description file name and path, with extension.\n"
                  "       -o: Output path, must be empty.\n"
                  "       -k: RME root path, must contain all necessary files.\n"
                  "       -u: RVM root path, must contain all necessary files.\n"
                  "       -f: Output file format.\n"
                  "           keil: Keil uVision IDE.\n"
                  "           eclipse: Eclipse IDE.\n"
                  "           makefile: Makefile project.\n");

	*Input_File=0;
	*Output_Path=0;
	*RME_Path=0;
	*RVM_Path=0;
	*Format=0;

    Count=1;
    /* Read the command line one by one */
    while(Count<(ptr_t)argc)
    {
        /* We need to open some input file */
        if(strcmp(argv[Count],"-i")==0)
        {
            if(*Input_File!=0)
                EXIT_FAIL("More than one input file.");

            *Input_File=argv[Count+1];

            Count+=2;
        }
        /* We need to check some output path. */
        else if(strcmp(argv[Count],"-o")==0)
        {
            if(*Output_Path!=0)
                EXIT_FAIL("More than one output path.");

            *Output_Path=argv[Count+1];
            if(Dir_Present(*Output_Path)!=0)
                EXIT_FAIL("Output path is not present.");
            if(Dir_Empty(*Output_Path)!=0)
                EXIT_FAIL("Output path is not empty.");

            Count+=2;
        }
        /* We need to check the RME root folder. */
        else if(strcmp(argv[Count],"-k")==0)
        {
            if(*RME_Path!=0)
                EXIT_FAIL("More than one RME root folder.");

            *RME_Path=argv[Count+1];
            if(Dir_Present(*RME_Path)!=0)
                EXIT_FAIL("RME root path is not present.");
            if(Dir_Empty(*RME_Path)==0)
                EXIT_FAIL("RME root path is empty, wrong path selected.");

            Count+=2;
        }
        /* We need to check the RVM root folder. */
        else if(strcmp(argv[Count],"-u")==0)
        {
            if(*RVM_Path!=0)
                EXIT_FAIL("More than one RVM root folder.");

            *RVM_Path=argv[Count+1];
            if(Dir_Present(*RVM_Path)!=0)
                EXIT_FAIL("RVM root path is not present.");
            if(Dir_Empty(*RVM_Path)==0)
                EXIT_FAIL("RVM root path is empty, wrong path selected.");

            Count+=2;
        }
        /* We need to set the format of the output project */
        else if(strcmp(argv[Count],"-f")==0)
        {
            if(*Format!=0)
                EXIT_FAIL("Conflicting output project format designated.");
            
            *Format=argv[Count+1];

            Count+=2;
        }
        else
            EXIT_FAIL("Unrecognized argument designated.");
    }

    if(*Input_File==0)
        EXIT_FAIL("No input file specified.");
    if(*Output_Path==0)
        EXIT_FAIL("No output path specified.");
    if(*RME_Path==0)
        EXIT_FAIL("No RME root path specified.");
    if(*RVM_Path==0)
        EXIT_FAIL("No RVM root path specified.");
    if(*Format==0)
        EXIT_FAIL("No output project type specified.");
}
/* End Function:Cmdline_Proc *************************************************/

/* Begin Function:Read_File ***************************************************
Description : Read the content of the whole file into the buffer.
Input       : s8* Path - The path to the file.
Output      : None.
Return      : s8* - The buffer containing the file contents.
******************************************************************************/
s8* Read_File(s8* Path)
{
	ptr_t Size;
	FILE* Handle;
	s8* Buf;

	Handle=fopen(Path,"r");
	if(Handle==0)
		EXIT_FAIL("Input file open failed.");

	fseek(Handle, 0, SEEK_END);
	Size=ftell(Handle);
	fseek(Handle, 0, SEEK_SET);

	Buf=Malloc(Size+1);
	if(Buf==0)
		EXIT_FAIL("File buffer allocation failed.");
	fread(Buf, 1, (size_t)Size, Handle);

	fclose(Handle);
	Buf[Size]='\0';

	return Buf;
}
/* End Function:Read_File ****************************************************/

/* Begin Function:XML_Strcmp **************************************************
Description : Compare if the contents of the two labels are the same.
Input       : s8* Start1 - The start position of the first string.
              s8* End1 - The end position of the first string.
			  s8* Start2 - The start position of the second string.
			  s8* End2 - The end position of the second string.
Output      : None.
Return      : If 0, match; if -1, mismatch.
******************************************************************************/
ret_t XML_Strcmp(s8* Start1, s8* End1, s8* Start2, s8* End2)
{
	ptr_t Count;

	if((Start1==0)||(End1==0)||(Start2==0)||(End2==0))
		return -1;

	if((End1<Start1)||(End2<Start2))
		return -1;

	if((End1-Start1)!=(End2-Start2))
		return -1;

	for(Count=0;Start1+Count<=End1;Count++)
	{
		if(Start1[Count]!=Start2[Count])
			return -1;
	}

	return 0;
}
/* End Function:XML_Strcmp ***************************************************/

/* Begin Function:XML_Get_Next ************************************************
Description : Get the next XML element.
Input       : s8* Start - The start position of the string.
              s8* End - The end position of the string.
Output      : s8** Label_Start - The start position of the label.
		      s8** Label_End - The end position of the label.
			  s8** Val_Start - The start position of the label's content.
			  s8** Val_End - The end position of the label's content.
Return      : ret_t - If 0, successful; if -1, failure.
******************************************************************************/
ret_t XML_Get_Next(s8* Start, s8* End,
	               s8** Label_Start, s8** Label_End,
	               s8** Val_Start, s8** Val_End)
{
	s8* Slide_Ptr;
	ptr_t Num_Elems;
	s8* Close_Label_Start;
	s8* Close_Label_End;

	if(Start>=End)
		return -1;

	/* Find the first "<" whose next character is not "/". This is the start of the label */
	for(Slide_Ptr=Start;Slide_Ptr<End;Slide_Ptr++)
	{
		if((Slide_Ptr[0]=='<')&&(Slide_Ptr[1]!='/'))
			break;
	}
	if(Slide_Ptr>=End)
		return -1;
	*Label_Start=Slide_Ptr+1;

	/* Find the closing position */
	for(Slide_Ptr++;Slide_Ptr<End;Slide_Ptr++)
	{
		if(Slide_Ptr[0]=='>')
			break;
	}
	if(Slide_Ptr>=End)
		return -1;
	*Label_End=Slide_Ptr-1;

	/* Find the value starting position */
	Slide_Ptr++;
	*Val_Start=Slide_Ptr;

	/* Find where this (could be) huge tag finally closes */
	for(Num_Elems=1;(Slide_Ptr<End)&&(Num_Elems!=0);Slide_Ptr++)
	{
		if((Slide_Ptr[0]=='<')&&(Slide_Ptr[1]!='/'))
			Num_Elems++;

		if((Slide_Ptr[0]=='<')&&(Slide_Ptr[1]=='/'))
			Num_Elems--;
	}
	if(Slide_Ptr>=End)
		return -1;
	*Val_End=Slide_Ptr-2;
	Close_Label_Start=Slide_Ptr+1;

	/* See if the tag is the same as what we captured above */
	for(Slide_Ptr+=2;Slide_Ptr<=End;Slide_Ptr++)
	{
		if(Slide_Ptr[0]=='>')
			break;
	}
	if(Slide_Ptr>End)
		return -1;
	if(Slide_Ptr[0]!='>')
		return -1;
	Close_Label_End=Slide_Ptr-1;

	if(XML_Strcmp(Close_Label_Start,Close_Label_End,*Label_Start,*Label_End)!=0)
		return -1;

	return 0;
}
/* End Function:XML_Get_Next *************************************************/

/* Begin Function:XML_Num***** ************************************************
Description : Get the number of XML elements in a given section.
Input       : s8* Start - The start position of the string.
              s8* End - The end position of the string.
Output      : None.
Return      : ptr_t - The number of elements found.
******************************************************************************/
ptr_t XML_Num(s8* Start, s8* End)
{
    ptr_t Num;
    s8* Start_Ptr;
    s8* Val_Start;
    s8* Val_End;
    s8* Label_Start;
    s8* Label_End;

    Start_Ptr=Start;
    Num=0;
    while(XML_Get_Next(Start_Ptr, End, &Label_Start, &Label_End, &Val_Start, &Val_End)==0)
    {
        Start_Ptr=Val_End;
        Num++;
    }
    
    return Num;
}
/* End Function:XML_Num ******************************************************/

/* Begin Function:Get_String **************************************************
Description : Extract the string value from the XML to another buffer.
Input       : s8* Start - The start position of the string.
              s8* End - The end position of the string.
Output      : None.
Return      : s8* - The extracted string with newly allocated memory.
******************************************************************************/
s8* Get_String(s8* Start, s8* End)
{
    s8* Buf;
    
    if((End<Start)||(Start==0)||(End==0))
        return 0;

    Buf=Malloc(End-Start+2);
    if(Buf==0)
        return 0;

    memcpy(Buf,Start,End-Start+1);
    Buf[End-Start+1]='\0';
    return Buf;
}
/* Begin Function:Get_String *************************************************/

/* Begin Function:Get_Hex *****************************************************
Description : Extract the hex value from the XML.
Input       : s8* Start - The start position of the string.
              s8* End - The end position of the string.
Output      : None.
Return      : ptr_t - The extracted hex number.
******************************************************************************/
ptr_t Get_Hex(s8* Start, s8* End)
{
    ptr_t Val;
    s8* Start_Ptr;

    if((End<Start)||(Start==0)||(End==0))
        return 0;

    Val=0;
    if((Start[0]!='0')||((Start[1]!='x')&&(Start[1]!='X')))
    {
        if(strncmp(Start,"Auto",4)==0)
            return AUTO;
        else
            return INVALID;
    }

    for(Start_Ptr=Start+2;Start_Ptr<=End;Start_Ptr++)
    {
        Val*=16;
        if((Start_Ptr[0]>='0')&&(Start_Ptr[0]<='9'))
            Val+=Start_Ptr[0]-'0';
        else if((Start_Ptr[0]>='A')&&(Start_Ptr[0]<='F'))
            Val+=Start_Ptr[0]-'A'+10;
        else if((Start_Ptr[0]>='a')&&(Start_Ptr[0]<='f'))
            Val+=Start_Ptr[0]-'a'+10;
        else
            return INVALID;
    }

    return Val;
}
/* End Function:Get_Hex ******************************************************/

/* Begin Function:Get_Uint ****************************************************
Description : Extract the unsigned integer value from the XML.
Input       : s8* Start - The start position of the string.
              s8* End - The end position of the string.
Output      : None.
Return      : ptr_t - The extracted unsigned integer number.
******************************************************************************/
ptr_t Get_Uint(s8* Start, s8* End)
{
    ptr_t Val;
    s8* Start_Ptr;

    if((End<Start)||(Start==0)||(End==0))
        return 0;

    Val=0;
    if(strncmp(Start,"Auto",4)==0)
        return AUTO;

    for(Start_Ptr=Start;Start_Ptr<=End;Start_Ptr++)
    {
        Val*=10;
        if((Start_Ptr[0]>='0')&&(Start_Ptr[0]<='9'))
            Val+=Start_Ptr[0]-'0';
        else
            return INVALID;
    }

    return Val;
}
/* End Function:Get_Uint *****************************************************/

/* Begin Function:Compare_Label ***********************************************
Description : See if the two labels are the same.
Input       : s8* Start - The start position of the string.
              s8* End - The end position of the string.
              s8* Label - The label to compare with.
Output      : None.
Return      : ret_t - If same, 0; else -1.
******************************************************************************/
ret_t Compare_Label(s8* Start, s8* End, s8* Label)
{
    ptr_t Len;

    Len=End-Start+1;
    if(Len!=strlen(Label))
        return -1;

    if(strncmp(Start,Label,(size_t)Len)!=0)
        return -1;
    
    return 0;
}
/* End Function:Compare_Label ************************************************/

/* Begin Function:Parse_RME ***************************************************
Description : Parse the RME section of the user-supplied project configuration file.
Input       : struct Proj_Info* Proj - The project structure.
              s8* Str_Start - The start position of the string.
              s8* Str_End - The end position of the string.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t Parse_RME(struct Proj_Info* Proj, s8* Str_Start, s8* Str_End)
{
    s8* Start;
    s8* End;
    s8* Label_Start;
    s8* Label_End;
    s8* Val_Start;
    s8* Val_End;
    s8* Compiler_Start;
    s8* Compiler_End;
    s8* General_Start;
    s8* General_End;
    s8* Platform_Start;
    s8* Platform_End;
    s8* Chip_Start;
    s8* Chip_End;
    ptr_t Count;

    Start=Str_Start;
    End=Str_End;

    /* Compiler */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Compiler");
    Compiler_Start=Val_Start;
    Compiler_End=Val_End;
    /* General */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "General");
    General_Start=Val_Start;
    General_End=Val_End;
    /* Platform-Name */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, Proj->Platform);
    Platform_Start=Val_Start;
    Platform_End=Val_End;
    /* Chip-Name */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, Proj->Chip);
    Chip_Start=Val_Start;
    Chip_End=Val_End;

    /* Now read the contents of the Compiler section */
    Start=Compiler_Start;
    End=Compiler_End;
    /* Optimization level */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Optimization");
    if(strncmp(Val_Start,"O0",2)==0)
        Proj->RME.Comp.Opt=OPT_O0;
    else if(strncmp(Val_Start,"O1",2)==0)
        Proj->RME.Comp.Opt=OPT_O1;
    else if(strncmp(Val_Start,"O2",2)==0)
        Proj->RME.Comp.Opt=OPT_O2;
    else if(strncmp(Val_Start,"O3",2)==0)
        Proj->RME.Comp.Opt=OPT_O3;
    else if(strncmp(Val_Start,"OS",2)==0)
        Proj->RME.Comp.Opt=OPT_OS;
    else
        EXIT_FAIL("The optimization option is malformed.");
    /* Time or size optimization */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Prioritization");
    if(strncmp(Val_Start,"Time",4)==0)
        Proj->RME.Comp.Prio=PRIO_TIME;
    else if(strncmp(Val_Start,"Size",4)==0)
        Proj->RME.Comp.Prio=PRIO_SIZE;
    else
        EXIT_FAIL("The prioritization option is malformed.");

    /* Now read the contents of the General section */
    Start=General_Start;
    End=General_End;
    /* Code start address */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Code_Start");
    Proj->RME.Code_Start=Get_Hex(Val_Start,Val_End);
    if(Proj->RME.Code_Start>=INVALID)
        EXIT_FAIL("Code section start address is malformed. This cannot be Auto.");
    /* Code size */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Code_Size");
    Proj->RME.Code_Size=Get_Hex(Val_Start,Val_End);
    if(Proj->RME.Code_Size>=INVALID)
        EXIT_FAIL("Code section size is malformed. This cannot be Auto.");
    /* Data start address */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Data_Start");
    Proj->RME.Data_Start=Get_Hex(Val_Start,Val_End);
    if(Proj->RME.Data_Start>=INVALID)
        EXIT_FAIL("Data section start address is malformed. This cannot be Auto.");
    /* Data size */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Data_Size");
    Proj->RME.Data_Size=Get_Hex(Val_Start,Val_End);
    if(Proj->RME.Data_Size>=INVALID)
        EXIT_FAIL("Data section size is malformed. This cannot be Auto.");
    /* Extra Kmem */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Extra_Kmem");
    Proj->RME.Extra_Kmem=Get_Hex(Val_Start,Val_End);
    if(Proj->RME.Extra_Kmem>=INVALID)
        EXIT_FAIL("Extra kernel memory size is malformed. This cannot be Auto.");
    /* Kmem_Order */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Kmem_Order");
    Proj->RME.Kmem_Order=Get_Uint(Val_Start,Val_End);
    if(Proj->RME.Kmem_Order>=INVALID)
        EXIT_FAIL("Kernel memory slot order is malformed. This cannot be Auto.");
    /* Priorities */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Kern_Prios");
    Proj->RME.Kern_Prios=Get_Uint(Val_Start,Val_End);
    if(Proj->RME.Kern_Prios>=INVALID)
        EXIT_FAIL("Priority number is malformed. This cannot be Auto.");

    /* Now read the platform section */
    Start=Platform_Start;
    End=Platform_End;
    Proj->RME.Plat_Raw.Num=XML_Num(Start,End);
    if(Proj->RME.Plat_Raw.Num!=0)
    {
        Proj->RME.Plat_Raw.Tag=Malloc(sizeof(s8*)*Proj->RME.Plat_Raw.Num);
        if(Proj->RME.Plat_Raw.Tag==0)
            EXIT_FAIL("Platform raw tag buffer allocation failed.");
        Proj->RME.Plat_Raw.Value=Malloc(sizeof(s8*)*Proj->RME.Plat_Raw.Num);
        if(Proj->RME.Plat_Raw.Value==0)
            EXIT_FAIL("Platform raw value buffer allocation failed.");
        for(Count=0;Count<Proj->RME.Plat_Raw.Num;Count++)
        {
            if(XML_Get_Next(Start, End, &Label_Start, &Label_End, &Val_Start, &Val_End)!=0)
                EXIT_FAIL("Unexpected error when parsing platform section.");
            Start=Val_End;
            Proj->RME.Plat_Raw.Tag[Count]=Get_String(Label_Start, Label_End);
            if(Proj->RME.Plat_Raw.Tag[Count]==0)
                EXIT_FAIL("Platform section tag read failed.");
            Proj->RME.Plat_Raw.Value[Count]=Get_String(Val_Start, Val_End);
            if(Proj->RME.Plat_Raw.Value[Count]==0)
                EXIT_FAIL("Platform section value read failed.");
        }
    }

    /* Now read the chip section */
    Start=Chip_Start;
    End=Chip_End;
    Proj->RME.Chip_Raw.Num=XML_Num(Start,End);
    if(Proj->RME.Chip_Raw.Num!=0)
    {
        Proj->RME.Chip_Raw.Tag=Malloc(sizeof(s8*)*Proj->RME.Chip_Raw.Num);
        if(Proj->RME.Chip_Raw.Tag==0)
            EXIT_FAIL("Chip raw tag buffer allocation failed.");
        Proj->RME.Chip_Raw.Value=Malloc(sizeof(s8*)*Proj->RME.Chip_Raw.Num);
        if(Proj->RME.Chip_Raw.Value==0)
            EXIT_FAIL("Chip raw value buffer allocation failed.");
        for(Count=0;Count<Proj->RME.Chip_Raw.Num;Count++)
        {
            if(XML_Get_Next(Start, End, &Label_Start, &Label_End, &Val_Start, &Val_End)!=0)
                EXIT_FAIL("Unexpected error when parsing chip section.");
            Start=Val_End;
            Proj->RME.Chip_Raw.Tag[Count]=Get_String(Label_Start, Label_End);
            if(Proj->RME.Chip_Raw.Tag[Count]==0)
                EXIT_FAIL("Chip section tag read failed.");
            Proj->RME.Chip_Raw.Value[Count]=Get_String(Val_Start, Val_End);
            if(Proj->RME.Chip_Raw.Value[Count]==0)
                EXIT_FAIL("Chip section value read failed.");
        }
    }

    return 0;
}
/* End Function:Parse_RME ****************************************************/

/* Begin Function:Parse_RVM ***************************************************
Description : Parse the RVM section of the user-supplied project configuration file.
Input       : struct Proj_Info* Proj - The project structure.
              s8* Str_Start - The start position of the string.
              s8* Str_End - The end position of the string.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t Parse_RVM(struct Proj_Info* Proj, s8* Str_Start, s8* Str_End)
{
    /* We don't parse RVM now as the functionality is not supported */
    s8* Start;
    s8* End;
    s8* Label_Start;
    s8* Label_End;
    s8* Val_Start;
    s8* Val_End;
    s8* Compiler_Start;
    s8* Compiler_End;
    s8* General_Start;
    s8* General_End;
    s8* VMM_Start;
    s8* VMM_End;

    Start=Str_Start;
    End=Str_End;

    /* Compiler */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Compiler");
    Compiler_Start=Val_Start;
    Compiler_End=Val_End;
    /* General */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "General");
    General_Start=Val_Start;
    General_End=Val_End;
    /* Platform-Name */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "VMM");
    VMM_Start=Val_Start;
    VMM_End=Val_End;

    /* Now read the contents of the Compiler section */
    Start=Compiler_Start;
    End=Compiler_End;
    /* Optimization level */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Optimization");
    if(strncmp(Val_Start,"O0",2)==0)
        Proj->RVM.Comp.Opt=OPT_O0;
    else if(strncmp(Val_Start,"O1",2)==0)
        Proj->RVM.Comp.Opt=OPT_O1;
    else if(strncmp(Val_Start,"O2",2)==0)
        Proj->RVM.Comp.Opt=OPT_O2;
    else if(strncmp(Val_Start,"O3",2)==0)
        Proj->RVM.Comp.Opt=OPT_O3;
    else if(strncmp(Val_Start,"OS",2)==0)
        Proj->RVM.Comp.Opt=OPT_OS;
    else
        EXIT_FAIL("The optimization option is malformed.");
    /* Time or size optimization */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Prioritization");
    if(strncmp(Val_Start,"Time",4)==0)
        Proj->RVM.Comp.Prio=PRIO_TIME;
    else if(strncmp(Val_Start,"Size",4)==0)
        Proj->RVM.Comp.Prio=PRIO_SIZE;
    else
        EXIT_FAIL("The prioritization option is malformed.");

    /* Now read the contents of the General section */
    Start=General_Start;
    End=General_End;
    /* Code size */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Code_Size");
    Proj->RVM.Code_Size=Get_Hex(Val_Start,Val_End);
    if(Proj->RVM.Code_Size>=INVALID)
        EXIT_FAIL("Code section size is malformed. This cannot be Auto.");
    /* Data size */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Data_Size");
    Proj->RVM.Data_Size=Get_Hex(Val_Start,Val_End);
    if(Proj->RVM.Data_Size>=INVALID)
        EXIT_FAIL("Data section size is malformed. This cannot be Auto.");
    /* Extra Kmem */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Extra_Captbl");
    Proj->RVM.Extra_Captbl=Get_Uint(Val_Start,Val_End);
    if(Proj->RVM.Extra_Captbl>=INVALID)
        EXIT_FAIL("Extra kernel memory size is malformed. This cannot be Auto.");
    /* Recovery */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Recovery");
    if(strncmp(Val_Start,"Thread",6)==0)
        Proj->RVM.Recovery=RECOVERY_THD;
    else if(strncmp(Val_Start,"Process",7)==0)
        Proj->RVM.Recovery=RECOVERY_PROC;
    else if(strncmp(Val_Start,"System",6)==0)
        Proj->RVM.Recovery=RECOVERY_SYS;
    else
        EXIT_FAIL("The recovery option is malformed.");

    /* The VMM section is currently unused. We don't care about this now */
    
    return 0;
}
/* End Function:Parse_RVM ****************************************************/

/* Begin Function:Parse_Process_Memory ****************************************
Description : Parse the memory section of a particular process.
Input       : struct Proj_Info* Proj - The project structure.
              ptr_t Proc_Num - The process number.
              ptr_t Mem_Num - The memory number.
              s8* Str_Start - The start position of the string.
              s8* Str_End - The end position of the string.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t Parse_Process_Memory(struct Proj_Info* Proj, ptr_t Proc_Num, ptr_t Mem_Num, s8* Str_Start, s8* Str_End)
{
    s8* Start;
    s8* End;
    s8* Label_Start;
    s8* Label_End;
    s8* Val_Start;
    s8* Val_End;
    s8* Attr_Temp;

    Start=Str_Start;
    End=Str_End;

    /* Start */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Start");
    Proj->Proc[Proc_Num].Mem[Mem_Num].Start=Get_Hex(Val_Start,Val_End);
    if(Proj->Proc[Proc_Num].Mem[Mem_Num].Start==INVALID)
        EXIT_FAIL("Memory start address read failed.");
    /* Size */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Size");
    Proj->Proc[Proc_Num].Mem[Mem_Num].Size=Get_Hex(Val_Start,Val_End);
    if(Proj->Proc[Proc_Num].Mem[Mem_Num].Size>=INVALID)
        EXIT_FAIL("Memory size read failed.");
    if(Proj->Proc[Proc_Num].Mem[Mem_Num].Size==0)
        EXIT_FAIL("Memory size cannot be zero.");
    if(Proj->Proc[Proc_Num].Mem[Mem_Num].Start!=AUTO)
    {
        if((Proj->Proc[Proc_Num].Mem[Mem_Num].Start+
            (Proj->Proc[Proc_Num].Mem[Mem_Num].Size-1))<Proj->Proc[Proc_Num].Mem[Mem_Num].Start)
            EXIT_FAIL("Memory trunk out of bound, illegal.");
    }
    /* Type */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Type");
    if(strncmp(Val_Start,"Code",4)==0)
        Proj->Proc[Proc_Num].Mem[Mem_Num].Type=MEM_CODE;
    else if(strncmp(Val_Start,"Data",4)==0)
        Proj->Proc[Proc_Num].Mem[Mem_Num].Type=MEM_DATA;
    else if(strncmp(Val_Start,"Device",6)==0)
        Proj->Proc[Proc_Num].Mem[Mem_Num].Type=MEM_DEVICE;
    else
        EXIT_FAIL("The memory type is malformed.");
    /* Attribute */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Attribute");
    Proj->Proc[Proc_Num].Mem[Mem_Num].Attr=0;
    Attr_Temp=Get_String(Val_Start, Val_End);
    if(strchr(Attr_Temp,'R')!=0)
        Proj->Proc[Proc_Num].Mem[Mem_Num].Attr|=MEM_READ;
    if(strchr(Attr_Temp,'W')!=0)
        Proj->Proc[Proc_Num].Mem[Mem_Num].Attr|=MEM_WRITE;
    if(strchr(Attr_Temp,'X')!=0)
        Proj->Proc[Proc_Num].Mem[Mem_Num].Attr|=MEM_EXECUTE;

    if(Proj->Proc[Proc_Num].Mem[Mem_Num].Attr==0)
        EXIT_FAIL("No access to the memory is allowed.");

    if(strchr(Attr_Temp,'B')!=0)
        Proj->Proc[Proc_Num].Mem[Mem_Num].Attr|=MEM_BUFFERABLE;
    if(strchr(Attr_Temp,'C')!=0)
        Proj->Proc[Proc_Num].Mem[Mem_Num].Attr|=MEM_CACHEABLE;
    if(strchr(Attr_Temp,'S')!=0)
        Proj->Proc[Proc_Num].Mem[Mem_Num].Attr|=MEM_STATIC;

    Free(Attr_Temp);
    return 0;
}
/* End Function:Parse_Process_Memory *****************************************/

/* Begin Function:Parse_Process_Thread ****************************************
Description : Parse the thread section of a particular process.
Input       : struct Proj_Info* Proj - The project structure.
              ptr_t Proc_Num - The process number.
              ptr_t Thd_Num - The thread number.
              s8* Str_Start - The start position of the string.
              s8* Str_End - The end position of the string.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t Parse_Process_Thread(struct Proj_Info* Proj, ptr_t Proc_Num, ptr_t Thd_Num, s8* Str_Start, s8* Str_End)
{
    s8* Start;
    s8* End;
    s8* Label_Start;
    s8* Label_End;
    s8* Val_Start;
    s8* Val_End;

    Start=Str_Start;
    End=Str_End;

    /* Name */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Name");
    Proj->Proc[Proc_Num].Thd[Thd_Num].Name=Get_String(Val_Start,Val_End);
    if(Proj->Proc[Proc_Num].Thd[Thd_Num].Name==0)
        EXIT_FAIL("Thread name value read failed.");
    /* Entry */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Entry");
    Proj->Proc[Proc_Num].Thd[Thd_Num].Entry=Get_String(Val_Start,Val_End);
    if(Proj->Proc[Proc_Num].Thd[Thd_Num].Entry==0)
        EXIT_FAIL("Thread entry value read failed.");
    /* Stack address */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Stack_Addr");
    Proj->Proc[Proc_Num].Thd[Thd_Num].Stack_Addr=Get_Hex(Val_Start,Val_End);
    if(Proj->Proc[Proc_Num].Thd[Thd_Num].Stack_Addr==INVALID)
        EXIT_FAIL("Thread stack address read failed.");
    /* Stack size */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Stack_Size");
    Proj->Proc[Proc_Num].Thd[Thd_Num].Stack_Size=Get_Hex(Val_Start,Val_End);
    if(Proj->Proc[Proc_Num].Thd[Thd_Num].Stack_Size>=INVALID)
        EXIT_FAIL("Thread stack size read failed.");
    /* Parameter */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Parameter");
    Proj->Proc[Proc_Num].Thd[Thd_Num].Parameter=Get_String(Val_Start,Val_End);
    if(Proj->Proc[Proc_Num].Thd[Thd_Num].Parameter==0)
        EXIT_FAIL("Thread parameter value read failed.");
    /* Priority level */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Priority");
    Proj->Proc[Proc_Num].Thd[Thd_Num].Priority=Get_Uint(Val_Start,Val_End);
    if(Proj->Proc[Proc_Num].Thd[Thd_Num].Priority>=INVALID)
        EXIT_FAIL("Thread priority read failed.");

    return 0;
}
/* End Function:Parse_Process_Thread *****************************************/

/* Begin Function:Parse_Process_Invocation ************************************
Description : Parse the invocation section of a particular process.
Input       : struct Proj_Info* Proj - The project structure.
              ptr_t Proc_Num - The process number.
              ptr_t Inv_Num - The invocation number.
              s8* Str_Start - The start position of the string.
              s8* Str_End - The end position of the string.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t Parse_Process_Invocation(struct Proj_Info* Proj, ptr_t Proc_Num, ptr_t Inv_Num, s8* Str_Start, s8* Str_End)
{
    s8* Start;
    s8* End;
    s8* Label_Start;
    s8* Label_End;
    s8* Val_Start;
    s8* Val_End;

    Start=Str_Start;
    End=Str_End;

    /* Name */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Name");
    Proj->Proc[Proc_Num].Inv[Inv_Num].Name=Get_String(Val_Start,Val_End);
    if(Proj->Proc[Proc_Num].Inv[Inv_Num].Name==0)
        EXIT_FAIL("Invocation name value read failed.");
    /* Entry */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Entry");
    Proj->Proc[Proc_Num].Inv[Inv_Num].Entry=Get_String(Val_Start,Val_End);
    if(Proj->Proc[Proc_Num].Inv[Inv_Num].Entry==0)
        EXIT_FAIL("Invocation entry value read failed.");
    /* Stack address */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Stack_Addr");
    Proj->Proc[Proc_Num].Inv[Inv_Num].Stack_Addr=Get_Hex(Val_Start,Val_End);
    if(Proj->Proc[Proc_Num].Inv[Inv_Num].Stack_Addr==INVALID)
        EXIT_FAIL("Invocation stack address read failed.");
    /* Stack size */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Stack_Size");
    Proj->Proc[Proc_Num].Inv[Inv_Num].Stack_Size=Get_Hex(Val_Start,Val_End);
    if(Proj->Proc[Proc_Num].Inv[Inv_Num].Stack_Size>=INVALID)
        EXIT_FAIL("Invocation stack size read failed.");
    
    return 0;
}
/* End Function:Parse_Process_Invocation *************************************/

/* Begin Function:Parse_Process_Port ******************************************
Description : Parse the port section of a particular process.
Input       : struct Proj_Info* Proj - The project structure.
              ptr_t Proc_Num - The process number.
              ptr_t Port_Num - The port number.
              s8* Str_Start - The start position of the string.
              s8* Str_End - The end position of the string.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t Parse_Process_Port(struct Proj_Info* Proj, ptr_t Proc_Num, ptr_t Port_Num, s8* Str_Start, s8* Str_End)
{
    s8* Start;
    s8* End;
    s8* Label_Start;
    s8* Label_End;
    s8* Val_Start;
    s8* Val_End;

    Start=Str_Start;
    End=Str_End;

    /* Name */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Name");
    Proj->Proc[Proc_Num].Port[Port_Num].Name=Get_String(Val_Start,Val_End);
    if(Proj->Proc[Proc_Num].Port[Port_Num].Name==0)
        EXIT_FAIL("Port name value read failed.");
    /* Process */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Process");
    Proj->Proc[Proc_Num].Port[Port_Num].Process=Get_String(Val_Start,Val_End);
    if(Proj->Proc[Proc_Num].Port[Port_Num].Process==0)
        EXIT_FAIL("Port process value read failed.");
    
    return 0;
}
/* End Function:Parse_Process_Port *******************************************/

/* Begin Function:Parse_Process_Receive ***************************************
Description : Parse the receive endpoint section of a particular process.
Input       : struct Proj_Info* Proj - The project structure.
              ptr_t Proc_Num - The process number.
              ptr_t Recv_Num - The receive endpoint number.
              s8* Str_Start - The start position of the string.
              s8* Str_End - The end position of the string.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t Parse_Process_Receive(struct Proj_Info* Proj, ptr_t Proc_Num, ptr_t Recv_Num, s8* Str_Start, s8* Str_End)
{
    s8* Start;
    s8* End;
    s8* Label_Start;
    s8* Label_End;
    s8* Val_Start;
    s8* Val_End;

    Start=Str_Start;
    End=Str_End;

    /* Name */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Name");
    Proj->Proc[Proc_Num].Recv[Recv_Num].Name=Get_String(Val_Start,Val_End);
    if(Proj->Proc[Proc_Num].Recv[Recv_Num].Name==0)
        EXIT_FAIL("Receive endpoint name value read failed.");

    return 0;
}
/* End Function:Parse_Process_Receive ***************************************/

/* Begin Function:Parse_Process_Send *****************************************
Description : Parse the send endpoint section of a particular process.
Input       : struct Proj_Info* Proj - The project structure.
              ptr_t Proc_Num - The process number.
              ptr_t Send_Num - The send endpoint number.
              s8* Str_Start - The start position of the string.
              s8* Str_End - The end position of the string.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t Parse_Process_Send(struct Proj_Info* Proj, ptr_t Proc_Num, ptr_t Send_Num, s8* Str_Start, s8* Str_End)
{
    s8* Start;
    s8* End;
    s8* Label_Start;
    s8* Label_End;
    s8* Val_Start;
    s8* Val_End;

    Start=Str_Start;
    End=Str_End;

    /* Name */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Name");
    Proj->Proc[Proc_Num].Send[Send_Num].Name=Get_String(Val_Start,Val_End);
    if(Proj->Proc[Proc_Num].Send[Send_Num].Name==0)
        EXIT_FAIL("Send endpoint name value read failed.");
    /* Process */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Process");
    Proj->Proc[Proc_Num].Send[Send_Num].Process=Get_String(Val_Start,Val_End);
    if(Proj->Proc[Proc_Num].Send[Send_Num].Process==0)
        EXIT_FAIL("Send endpoint process value read failed.");

    return 0;
}
/* End Function:Parse_Process_Send *******************************************/

/* Begin Function:Parse_Process_Vector ****************************************
Description : Parse the vector endpoint section of a particular process.
Input       : struct Proj_Info* Proj - The project structure.
              ptr_t Proc_Num - The process number.
              ptr_t Vect_Num - The vector endpoint number.
              s8* Str_Start - The start position of the string.
              s8* Str_End - The end position of the string.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t Parse_Process_Vector(struct Proj_Info* Proj, ptr_t Proc_Num, ptr_t Vect_Num, s8* Str_Start, s8* Str_End)
{
    s8* Start;
    s8* End;
    s8* Label_Start;
    s8* Label_End;
    s8* Val_Start;
    s8* Val_End;

    Start=Str_Start;
    End=Str_End;

    /* Name */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Name");
    Proj->Proc[Proc_Num].Vect[Vect_Num].Name=Get_String(Val_Start,Val_End);
    if(Proj->Proc[Proc_Num].Vect[Vect_Num].Name==0)
        EXIT_FAIL("Vector endpoint name value read failed.");

    return 0;
}
/* End Function:Parse_Process_Vector *****************************************/

/* Begin Function:Parse_Project_Process ***************************************
Description : Parse a particular process.
Input       : struct Proj_Info* Proj - The project structure.
              ptr_t Num - The process number.
              s8* Str_Start - The start position of the string.
              s8* Str_End - The end position of the string.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t Parse_Project_Process(struct Proj_Info* Proj, ptr_t Num, s8* Str_Start, s8* Str_End)
{
    s8* Start;
    s8* End;
    s8* Label_Start;
    s8* Label_End;
    s8* Val_Start;
    s8* Val_End;
    ptr_t Count;
    s8* General_Start;
    s8* General_End;
    s8* Compiler_Start;
    s8* Compiler_End;
    s8* Memory_Start;
    s8* Memory_End;
    s8* Thread_Start;
    s8* Thread_End;
    s8* Invocation_Start;
    s8* Invocation_End;
    s8* Port_Start;
    s8* Port_End;
    s8* Receive_Start;
    s8* Receive_End;
    s8* Send_Start;
    s8* Send_End;
    s8* Vector_Start;
    s8* Vector_End;

    Start=Str_Start;
    End=Str_End;

    /* General section */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "General");
    General_Start=Val_Start;
    General_End=Val_End;
    /* Compiler section */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Compiler");
    Compiler_Start=Val_Start;
    Compiler_End=Val_End;
    /* Memories section */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Memory");
    Memory_Start=Val_Start;
    Memory_End=Val_End;
    /* Threads section */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Thread");
    Thread_Start=Val_Start;
    Thread_End=Val_End;
    /* Invocations section */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Invocation");
    Invocation_Start=Val_Start;
    Invocation_End=Val_End;
    /* Ports section */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Port");
    Port_Start=Val_Start;
    Port_End=Val_End;
    /* Receive endpoints section */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Receive");
    Receive_Start=Val_Start;
    Receive_End=Val_End;
    /* Send endpoints section */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Send");
    Send_Start=Val_Start;
    Send_End=Val_End;
    /* Vector endpoints section */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Vector");
    Vector_Start=Val_Start;
    Vector_End=Val_End;

    /* Parse general section */
    Start=General_Start;
    End=General_End;
    /* Process name */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Name");
    Proj->Proc[Num].Name=Get_String(Val_Start,Val_End);
    if(Proj->Proc[Num].Name==0)
        EXIT_FAIL("Name value read failed.");
    /* Capability extra size */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Extra_Captbl");
    Proj->Proc[Num].Extra_Captbl=Get_Uint(Val_Start,Val_End);
    if(Proj->Proc[Num].Extra_Captbl>=INVALID)
        EXIT_FAIL("Extra capability table size value read failed.");
    
    /* Parse compiler section */
    Start=Compiler_Start;
    End=Compiler_End;
    /* Optimization level */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Optimization");
    if(strncmp(Val_Start,"O0",2)==0)
        Proj->Proc[Num].Comp.Opt=OPT_O0;
    else if(strncmp(Val_Start,"O1",2)==0)
        Proj->Proc[Num].Comp.Opt=OPT_O1;
    else if(strncmp(Val_Start,"O2",2)==0)
        Proj->Proc[Num].Comp.Opt=OPT_O2;
    else if(strncmp(Val_Start,"O3",2)==0)
        Proj->Proc[Num].Comp.Opt=OPT_O3;
    else if(strncmp(Val_Start,"OS",2)==0)
        Proj->Proc[Num].Comp.Opt=OPT_OS;
    else
        EXIT_FAIL("The optimization option is malformed.");
    /* Time or size optimization */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Prioritization");
    if(strncmp(Val_Start,"Time",4)==0)
        Proj->Proc[Num].Comp.Prio=PRIO_TIME;
    else if(strncmp(Val_Start,"Size",4)==0)
        Proj->Proc[Num].Comp.Prio=PRIO_SIZE;
    else
        EXIT_FAIL("The prioritization option is malformed.");

    /* Parse memory section */
    Start=Memory_Start;
    End=Memory_End;
    Proj->Proc[Num].Mem_Num=XML_Num(Start, End);
    if(Proj->Proc[Num].Mem_Num==0)
        EXIT_FAIL("The memories section is malformed.");
    Proj->Proc[Num].Mem=Malloc(sizeof(struct Mem_Info)*Proj->Proc[Num].Mem_Num);
    if(Proj->Proc[Num].Mem==0)
        EXIT_FAIL("The memory structure allocation failed.");
    for(Count=0;Count<Proj->Proc[Num].Mem_Num;Count++)
    {
        if(XML_Get_Next(Start, End, &Label_Start, &Label_End, &Val_Start, &Val_End)!=0)
            EXIT_FAIL("Unexpected error when parsing memories section.");
        Start=Val_End;
        Parse_Process_Memory(Proj, Num, Count, Val_Start, Val_End);
    }

    /* Parse threads section */
    Start=Thread_Start;
    End=Thread_End;
    Proj->Proc[Num].Thd_Num=XML_Num(Start, End);
    if(Proj->Proc[Num].Thd_Num==0)
        EXIT_FAIL("The process is malformed, doesn't contain any threads or invocations.");
    Proj->Proc[Num].Thd=Malloc(sizeof(struct Thd_Info)*Proj->Proc[Num].Thd_Num);
    if(Proj->Proc[Num].Thd==0)
        EXIT_FAIL("The thread structure allocation failed.");
    for(Count=0;Count<Proj->Proc[Num].Thd_Num;Count++)
    {
        if(XML_Get_Next(Start, End, &Label_Start, &Label_End, &Val_Start, &Val_End)!=0)
            EXIT_FAIL("Unexpected error when parsing thread section.");
        Start=Val_End;
        Parse_Process_Thread(Proj, Num, Count, Val_Start, Val_End);
    }

    /* Parse invocations section */
    Start=Invocation_Start;
    End=Invocation_End;
    Proj->Proc[Num].Inv_Num=XML_Num(Start, End);
    if(Proj->Proc[Num].Inv_Num!=0)
    {
        Proj->Proc[Num].Inv=Malloc(sizeof(struct Inv_Info)*Proj->Proc[Num].Inv_Num);
        if(Proj->Proc[Num].Inv==0)
            EXIT_FAIL("The invocation structure allocation failed.");
        for(Count=0;Count<Proj->Proc[Num].Inv_Num;Count++)
        {
            if(XML_Get_Next(Start, End, &Label_Start, &Label_End, &Val_Start, &Val_End)!=0)
                EXIT_FAIL("Unexpected error when parsing invocation section.");
            Start=Val_End;
            Parse_Process_Invocation(Proj, Num, Count, Val_Start, Val_End);
        }
    }
    
    /* Parse ports section */
    Start=Port_Start;
    End=Port_End;
    Proj->Proc[Num].Port_Num=XML_Num(Start, End);
    if(Proj->Proc[Num].Port_Num!=0)
    {
        Proj->Proc[Num].Port=Malloc(sizeof(struct Port_Info)*Proj->Proc[Num].Port_Num);
        if(Proj->Proc[Num].Port==0)
            EXIT_FAIL("The port structure allocation failed.");
        for(Count=0;Count<Proj->Proc[Num].Port_Num;Count++)
        {
            if(XML_Get_Next(Start, End, &Label_Start, &Label_End, &Val_Start, &Val_End)!=0)
                EXIT_FAIL("Unexpected error when parsing port section.");
            Start=Val_End;
            Parse_Process_Port(Proj, Num, Count, Val_Start, Val_End);
        }
    }

    /* Parse receive endpoints section */
    Start=Receive_Start;
    End=Receive_End;
    Proj->Proc[Num].Recv_Num=XML_Num(Start, End);
    if(Proj->Proc[Num].Recv_Num!=0)
    {
        Proj->Proc[Num].Recv=Malloc(sizeof(struct Recv_Info)*Proj->Proc[Num].Recv_Num);
        if(Proj->Proc[Num].Recv==0)
            EXIT_FAIL("The receive endpoint structure allocation failed.");
        for(Count=0;Count<Proj->Proc[Num].Recv_Num;Count++)
        {
            if(XML_Get_Next(Start, End, &Label_Start, &Label_End, &Val_Start, &Val_End)!=0)
                EXIT_FAIL("Unexpected error when parsing receive endpoint section.");
            Start=Val_End;
            Parse_Process_Receive(Proj, Num, Count, Val_Start, Val_End);
        }
    }

    /* Parse send endpoints section */
    Start=Send_Start;
    End=Send_End;
    Proj->Proc[Num].Send_Num=XML_Num(Start, End);
    if(Proj->Proc[Num].Send_Num!=0)
    {
        Proj->Proc[Num].Send=Malloc(sizeof(struct Send_Info)*Proj->Proc[Num].Send_Num);
        if(Proj->Proc[Num].Send==0)
            EXIT_FAIL("The send endpoint structure allocation failed.");
        for(Count=0;Count<Proj->Proc[Num].Send_Num;Count++)
        {
            if(XML_Get_Next(Start, End, &Label_Start, &Label_End, &Val_Start, &Val_End)!=0)
                EXIT_FAIL("Unexpected error when parsing send endpoint section.");
            Start=Val_End;
            Parse_Process_Send(Proj, Num, Count, Val_Start, Val_End);
        }
    }

    /* Parse vector endpoints section */
    Start=Vector_Start;
    End=Vector_End;
    Proj->Proc[Num].Vect_Num=XML_Num(Start, End);
    if(Proj->Proc[Num].Vect_Num!=0)
    {
        Proj->Proc[Num].Vect=Malloc(sizeof(struct Vect_Info)*Proj->Proc[Num].Vect_Num);
        if(Proj->Proc[Num].Vect==0)
            EXIT_FAIL("The vector endpoint structure allocation failed.");
        for(Count=0;Count<Proj->Proc[Num].Vect_Num;Count++)
        {
            if(XML_Get_Next(Start, End, &Label_Start, &Label_End, &Val_Start, &Val_End)!=0)
                EXIT_FAIL("Unexpected error when parsing vector endpoint section.");
            Start=Val_End;
            Parse_Process_Vector(Proj, Num, Count, Val_Start, Val_End);
        }
    }

    return 0;
}
/* End Function:Parse_Project_Process ****************************************/

/* Begin Function:Parse_Project ***********************************************
Description : Parse the project description file, and fill in the struct.
Input       : s8* Proj_File - The buffer containing the project file contents.
Output      : None.
Return      : struct Proj_Info* - The struct containing the project information.
******************************************************************************/
struct Proj_Info* Parse_Project(s8* Proj_File)
{
	s8* Start;
	s8* End;
    ptr_t Count;
    s8* Label_Start;
    s8* Label_End;
    s8* Val_Start;
    s8* Val_End;
    struct Proj_Info* Proj;

    /* Allocate the project information structure */
    Proj=Malloc(sizeof(struct Proj_Info));
    if(Proj==0)
        EXIT_FAIL("Project structure allocation failed.");
    
    /* How long is the file? */
    Count=strlen(Proj_File);
    Start=Proj_File;
    End=&Proj_File[Count-1];

    /* Skip the xml header */
    Start++;
    while(Start[0]!='\0')
    {
        if(Start[0]=='<')
            break;
        Start++;
    }
    if(Start[0]=='\0')
        EXIT_FAIL("Project XML header is malformed.");

    /* Read basics of the project */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Project");
    Start=Val_Start;
    End=Val_End;

    /* Name */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Name");
    Proj->Name=Get_String(Val_Start,Val_End);
    if(Proj->Name==0)
        EXIT_FAIL("Name value read failed.");
    /* Platform */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Platform");
    Proj->Platform=Get_String(Val_Start,Val_End);
    if(Proj->Platform==0)
        EXIT_FAIL("Platform value read failed.");
    /* Chip */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Chip");
    Proj->Chip=Get_String(Val_Start,Val_End);
    if(Proj->Chip==0)
        EXIT_FAIL("Chip value read failed.");
    /* Fullname */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Fullname");
    Proj->Fullname=Get_String(Val_Start,Val_End);
    if(Proj->Fullname==0)
        EXIT_FAIL("Chip fullname value read failed.");
    /* RME */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "RME");
    if(Parse_RME(Proj, Val_Start, Val_End)!=0)
        EXIT_FAIL("RME section parsing failed.");
    /* RVM */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "RVM");
    if(Parse_RVM(Proj, Val_Start, Val_End)!=0)
        EXIT_FAIL("RVM section parsing failed.");
    /* Process */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Process");
    Start=Val_Start;
    End=Val_End;
    Proj->Proc_Num=XML_Num(Start, End);
    if(Proj->Proc_Num==0)
        EXIT_FAIL("The project section is malformed.");
    Proj->Proc=Malloc(sizeof(struct Proc_Info)*Proj->Proc_Num);
    if(Proj->Proc==0)
        EXIT_FAIL("The process structure allocation failed.");
    for(Count=0;Count<Proj->Proc_Num;Count++)
    {
        if(XML_Get_Next(Start, End, &Label_Start, &Label_End, &Val_Start, &Val_End)!=0)
            EXIT_FAIL("Unexpected error when parsing process section.");
        Start=Val_End;
        Parse_Project_Process(Proj, Count, Val_Start, Val_End);
    }

    return Proj;
}
/* End Function:Parse_Project ************************************************/

/* Begin Function:Parse_Chip_Memory *******************************************
Description : Parse the memory section of a particular chip.
Input       : struct Chip_Info* Chip - The project structure.
              ptr_t Num - The memory number.
              s8* Str_Start - The start position of the string.
              s8* Str_End - The end position of the string.
Output      : struct Chip_Info* Chip - The updated chip structure.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t Parse_Chip_Memory(struct Chip_Info* Chip, ptr_t Num, s8* Str_Start, s8* Str_End)
{
    s8* Start;
    s8* End;
    s8* Label_Start;
    s8* Label_End;
    s8* Val_Start;
    s8* Val_End;

    Start=Str_Start;
    End=Str_End;

    /* Start */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Start");
    Chip->Mem[Num].Start=Get_Hex(Val_Start,Val_End);
    if(Chip->Mem[Num].Start>=INVALID)
        EXIT_FAIL("Memory start address read failed.");
    /* Size */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Size");
    Chip->Mem[Num].Size=Get_Hex(Val_Start,Val_End);
    if(Chip->Mem[Num].Size>=INVALID)
        EXIT_FAIL("Memory size read failed.");
    if(Chip->Mem[Num].Size==0)
        EXIT_FAIL("Memory size cannot be zero.");
    if((Chip->Mem[Num].Start+(Chip->Mem[Num].Size-1))<Chip->Mem[Num].Start)
        EXIT_FAIL("Memory trunk out of bound, illegal.");
    /* Type */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Type");
    if(strncmp(Val_Start,"Code",4)==0)
        Chip->Mem[Num].Type=MEM_CODE;
    else if(strncmp(Val_Start,"Data",4)==0)
        Chip->Mem[Num].Type=MEM_DATA;
    else if(strncmp(Val_Start,"Device",6)==0)
        Chip->Mem[Num].Type=MEM_DEVICE;
    else
        EXIT_FAIL("The memory type is malformed.");

    return 0;
}
/* End Function:Parse_Chip_Memory ********************************************/

/* Begin Function:Parse_Chip_Option *******************************************
Description : Parse the option section of a particular chip.
Input       : struct Chip_Info* Chip - The project structure.
              ptr_t Num - The option number.
              s8* Str_Start - The start position of the string.
              s8* Str_End - The end position of the string.
Output      : struct Chip_Info* Chip - The updated chip structure.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t Parse_Chip_Option(struct Chip_Info* Chip, ptr_t Num, s8* Str_Start, s8* Str_End)
{
    s8* Start;
    s8* End;
    s8* Label_Start;
    s8* Label_End;
    s8* Val_Start;
    s8* Val_End;
    s8* Value_Temp;
    ptr_t Count;

    Start=Str_Start;
    End=Str_End;

    /* Name */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Name");
    Chip->Option[Num].Name=Get_String(Val_Start,Val_End);
    if(Chip->Option[Num].Name==0)
        EXIT_FAIL("Option name read failed.");
    /* Type */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Type");
    if(strncmp(Val_Start,"Range",5)==0)
        Chip->Option[Num].Type=OPTION_RANGE;
    else if(strncmp(Val_Start,"Select",6)==0)
        Chip->Option[Num].Type=OPTION_SELECT;
    else
        EXIT_FAIL("The option type is malformed.");
    /* Macro */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Macro");
    Chip->Option[Num].Macro=Get_String(Val_Start,Val_End);
    if(Chip->Option[Num].Macro==0)
        EXIT_FAIL("Option macro read failed.");
    /* Value */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Value");
    Value_Temp=Get_String(Val_Start,Val_End);
    if(Value_Temp==0)
        EXIT_FAIL("Option macro read failed.");
    if(strstr(Value_Temp,",,")!=0)
        EXIT_FAIL("Option macro read failed.");
    if(Chip->Option[Num].Type==OPTION_RANGE)
    {
        /* Find the start and end of this */
        for(Start=Val_Start;Start<=Val_End;Start++)
        {
            if(Start[0]==',')
                break;
        }
        if(Start>=Val_End)
            EXIT_FAIL("Incorrect range.");
        Chip->Option[Num].Range_Min=Get_Uint(Val_Start,Start-1);
        Chip->Option[Num].Range_Max=Get_Uint(Start+1,Val_End);
        if((Chip->Option[Num].Range_Min>=INVALID)||(Chip->Option[Num].Range_Max>=INVALID))
            EXIT_FAIL("Incorrect range.");
        if(Chip->Option[Num].Range_Min>=Chip->Option[Num].Range_Max)
            EXIT_FAIL("Incorrect range.");
    }
    else
    {
        /* See how many options exist */
        Count=0;
        for(Start=Val_Start;Start<=Val_End;Start++)
        {
            if(Start[0]==',')
                Count++;
        }
        if(Count==0)
            EXIT_FAIL("Incorrect options.");
        Chip->Option[Num].Select_Num=Count+1;
        Chip->Option[Num].Select_Opt=Malloc(sizeof(s8*)*Chip->Option[Num].Select_Num);
        Start=Val_Start;
        End=Val_Start;
        for(Count=0;Count<Chip->Option[Num].Select_Num;Count++)
        {
            while((End[0]!=',')&&(End<=Val_End))
                End++;

            Chip->Option[Num].Select_Opt[Count]=Get_String(Start,End-1);
            Start=End+1;
            End=Start;
            if(Chip->Option[Num].Select_Opt[Count]==0)
                EXIT_FAIL("Chip select option memory allocation failed.");
        }
    }

    Free(Value_Temp);
    return 0;
}
/* End Function:Parse_Chip_Option ********************************************/

/* Begin Function:Parse_Chip_Vector *******************************************
Description : Parse the vector section of a particular chip.
Input       : struct Chip_Info* Chip - The project structure.
              ptr_t Num - The option number.
              s8* Str_Start - The start position of the string.
              s8* Str_End - The end position of the string.
Output      : struct Chip_Info* Chip - The updated chip structure.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t Parse_Chip_Vector(struct Chip_Info* Chip, ptr_t Num, s8* Str_Start, s8* Str_End)
{
    s8* Start;
    s8* End;
    s8* Label_Start;
    s8* Label_End;
    s8* Val_Start;
    s8* Val_End;

    Start=Str_Start;
    End=Str_End;

    /* Name */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Name");
    Chip->Vect[Num].Name=Get_String(Val_Start,Val_End);
    if(Chip->Vect[Num].Name==0)
        EXIT_FAIL("Vector name read failed.");

    /* Number */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Number");
    Chip->Vect[Num].Number=Get_Uint(Val_Start,Val_End);
    if(Chip->Vect[Num].Number>=INVALID)
        EXIT_FAIL("Vector number read failed.");

    return 0;
}
/* Begin Function:Parse_Chip_Vector *******************************************

/* Begin Function:Parse_Chip **************************************************
Description : Parse the chip description file, and fill in the struct.
Input       : s8* Chip_File - The buffer containing the chip file contents.
Output      : None.
Return      : struct Chip_Info* - The struct containing the chip information.
******************************************************************************/
struct Chip_Info* Parse_Chip(s8* Chip_File)
{
	s8* Start;
	s8* End;
    ptr_t Count;
    s8* Label_Start;
    s8* Label_End;
    s8* Val_Start;
    s8* Val_End;
    struct Chip_Info* Chip;
    s8* Attribute_Start;
    s8* Attribute_End;
    s8* Memory_Start;
    s8* Memory_End;
    s8* Option_Start;
    s8* Option_End;
    s8* Vector_Start;
    s8* Vector_End;

    /* Allocate the project information structure */
    Chip=Malloc(sizeof(struct Chip_Info));
    if(Chip==0)
        EXIT_FAIL("Chip structure allocation failed.");
    
    /* How long is the file? */
    Count=strlen(Chip_File);
    Start=Chip_File;
    End=&Chip_File[Count-1];

    /* Skip the xml header */
    Start++;
    while(Start[0]!='\0')
    {
        if(Start[0]=='<')
            break;
        Start++;
    }
    if(Start[0]=='\0')
        EXIT_FAIL("Chip XML header is malformed.");

    /* Read basics of the chip */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Chip");
    Start=Val_Start;
    End=Val_End;

    /* Name */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Name");
    Chip->Name=Get_String(Val_Start,Val_End);
    if(Chip->Name==0)
        EXIT_FAIL("Name value read failed.");
    /* Vendor */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Vendor");
    Chip->Vendor=Get_String(Val_Start,Val_End);
    if(Chip->Vendor==0)
        EXIT_FAIL("Vendor value read failed.");
    /* Platform */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Platform");
    Chip->Platform=Get_String(Val_Start,Val_End);
    if(Chip->Platform==0)
        EXIT_FAIL("Platform value read failed.");
    /* Cores */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Cores");
    Chip->Cores=Get_Uint(Val_Start,Val_End);
    if((Chip->Cores==0)||(Chip->Cores>=INVALID))
        EXIT_FAIL("Chip cores read failed.");
    /* Regions */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Regions");
    Chip->Regions=Get_Uint(Val_Start,Val_End);
    if((Chip->Regions<=2)||(Chip->Regions>=INVALID))
        EXIT_FAIL("Chip regions read failed.");
    /* Attribute */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Attribute");
    Attribute_Start=Val_Start;
    Attribute_End=Val_End;
    /* Memory */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Memory");
    Memory_Start=Val_Start;
    Memory_End=Val_End;
    /* Option */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Option");
    Option_Start=Val_Start;
    Option_End=Val_End;
    /* Vector */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Vector");
    Vector_Start=Val_Start;
    Vector_End=Val_End;

    /* Attribute */
    Start=Attribute_Start;
    End=Attribute_End;
    Chip->Attr_Raw.Num=XML_Num(Start, End);
    if(Chip->Attr_Raw.Num!=0)
    {
        Chip->Attr_Raw.Tag=Malloc(sizeof(struct Mem_Info)*Chip->Attr_Raw.Num);
        if(Chip->Attr_Raw.Tag==0)
            EXIT_FAIL("The attribute structure allocation failed.");
        Chip->Attr_Raw.Value=Malloc(sizeof(struct Mem_Info)*Chip->Attr_Raw.Num);
        if(Chip->Attr_Raw.Value==0)
            EXIT_FAIL("The attribute structure allocation failed.");
        for(Count=0;Count<Chip->Attr_Raw.Num;Count++)
        {
            if(XML_Get_Next(Start, End, &Label_Start, &Label_End, &Val_Start, &Val_End)!=0)
                EXIT_FAIL("Unexpected error when parsing chip attribute section.");
            Start=Val_End;
            Chip->Attr_Raw.Tag[Count]=Get_String(Label_Start, Label_End);
            if(Chip->Attr_Raw.Tag[Count]==0)
                EXIT_FAIL("Chip attribute section tag read failed.");
            Chip->Attr_Raw.Value[Count]=Get_String(Val_Start, Val_End);
            if(Chip->Attr_Raw.Value[Count]==0)
                EXIT_FAIL("Chip attribute section value read failed.");
        }
    }

    /* Memory */
    Start=Memory_Start;
    End=Memory_End;
    Chip->Mem_Num=XML_Num(Start, End);
    if(Chip->Mem_Num==0)
        EXIT_FAIL("The memory section is malformed.");
    Chip->Mem=Malloc(sizeof(struct Mem_Info)*Chip->Mem_Num);
    if(Chip->Mem==0)
        EXIT_FAIL("The memory structure allocation failed.");
    for(Count=0;Count<Chip->Mem_Num;Count++)
    {
        if(XML_Get_Next(Start, End, &Label_Start, &Label_End, &Val_Start, &Val_End)!=0)
            EXIT_FAIL("Unexpected error when parsing memory section.");
        Start=Val_End;
        Parse_Chip_Memory(Chip, Count, Val_Start, Val_End);
    }

    /* Option */
    Start=Option_Start;
    End=Option_End;
    Chip->Option_Num=XML_Num(Start, End);
    if(Chip->Option_Num==0)
        EXIT_FAIL("The option section is malformed.");
    Chip->Option=Malloc(sizeof(struct Option_Info)*Chip->Option_Num);
    if(Chip->Option==0)
        EXIT_FAIL("The option structure allocation failed.");
    for(Count=0;Count<Chip->Option_Num;Count++)
    {
        if(XML_Get_Next(Start, End, &Label_Start, &Label_End, &Val_Start, &Val_End)!=0)
            EXIT_FAIL("Unexpected error when parsing option section.");
        Start=Val_End;
        Parse_Chip_Option(Chip, Count, Val_Start, Val_End);
    }

    /* Vector */
    Start=Vector_Start;
    End=Vector_End;
    Chip->Vect_Num=XML_Num(Start, End);
    if(Chip->Vect_Num==0)
        EXIT_FAIL("The option section is malformed.");
    Chip->Vect=Malloc(sizeof(struct Option_Info)*Chip->Vect_Num);
    if(Chip->Vect==0)
        EXIT_FAIL("The option structure allocation failed.");
    for(Count=0;Count<Chip->Vect_Num;Count++)
    {
        if(XML_Get_Next(Start, End, &Label_Start, &Label_End, &Val_Start, &Val_End)!=0)
            EXIT_FAIL("Unexpected error when parsing option section.");
        Start=Val_End;
        Parse_Chip_Vector(Chip, Count, Val_Start, Val_End);
    }
    
    return Chip;
}
/* End Function:Parse_Chip ***************************************************/

/* Begin Function:Align_Mem ***************************************************
Description : Align the memory according to the platform's alignment functions.
              We will only align the memory of the processes.
Input       : struct Proj_Info* Proj - The struct containing the project information.
              ret_t (*Align)(struct Mem_Info*) - The platform's alignment function pointer.
Output      : struct Proj_Info* Proj - The struct containing the project information,
                                       with all memory size aligned.
Return      : None.
******************************************************************************/
void Align_Mem(struct Proj_Info* Proj, ret_t (*Align)(struct Mem_Info*))
{
    ptr_t Proc_Cnt;
    ptr_t Mem_Cnt;

    for(Proc_Cnt=0;Proc_Cnt<Proj->Proc_Num;Proc_Cnt++)
    {
        for(Mem_Cnt=0;Mem_Cnt<Proj->Proc[Proc_Cnt].Mem_Num;Mem_Cnt++)
        {
            if(Align(&(Proj->Proc[Proc_Cnt].Mem[Mem_Cnt]))!=0)
                EXIT_FAIL("Memory aligning failed.");
        }
    }
}
/* End Function:Align_Mem ****************************************************/

/* Begin Function:Insert_Mem **************************************************
Description : Insert memory blocks into a queue with increasing start address/size.
Input       : struct Mem_Info** Array - The array containing all the memory blocks to sort.
              ptr_t Len - The maximum length of the array.
              struct Mem_Info* Mem - The memory block to insert.
              ptr_t Category - The insert option, 0 for start address, 1 for size.
Output      : struct Mem_Info** Array - The updated array.
Return      : ret_t - If successful, 0; else -1.
******************************************************************************/
ret_t Insert_Mem(struct Mem_Info** Array, ptr_t Len, struct Mem_Info* Mem, ptr_t Category)
{
    ptr_t Pos;
    ptr_t End;

    for(Pos=0;Pos<Len;Pos++)
    {
        if(Array[Pos]==0)
            break;
        if(Category==0)
        {
            if(Array[Pos]->Start>Mem->Start)
                break;
        }
        else
        {
            if(Array[Pos]->Size>Mem->Size)
                break;
        }
    }
    if(Pos>=Len)
        return -1;
    for(End=Pos;End<Len;End++)
    {
        if(Array[End]==0)
            break;
    }
    if(End>0)
    {
        for(End--;End>=Pos;End--)
            Array[End+1]=Array[End];
    }

    Array[Pos]=Mem;
    return 0;
}
/* End Function:Insert_Mem ***************************************************/

/* Begin Function:Try_Bitmap **************************************************
Description : See if this bitmap segment is already covered.
Input       : s8* Bitmap - The bitmap.
              ptr_t Start - The starting bit location.
              ptr_t Size - The number of bits.
Output      : None.
Return      : ret_t - If can be marked, 0; else -1.
******************************************************************************/
ret_t Try_Bitmap(s8* Bitmap, ptr_t Start, ptr_t Size)
{
    ptr_t Count;

    for(Count=0;Count<Size;Count++)
    {
        if((Bitmap[(Start+Count)/8]&POW2((Start+Count)%8))!=0)
            return -1;
    }
    return 0;
}
/* End Function:Try_Bitmap ***************************************************/

/* Begin Function:Mark_Bitmap *************************************************
Description : Actually mark this bitmap segment.
Input       : s8* Bitmap - The bitmap.
              ptr_t Start - The starting bit location.
              ptr_t Size - The number of bits.
Output      : s8* Bitmap - The updated bitmap.
Return      : None.
******************************************************************************/
void Mark_Bitmap(s8* Bitmap, ptr_t Start, ptr_t Size)
{
    ptr_t Count;

    for(Count=0;Count<Size;Count++)
        Bitmap[(Start+Count)/8]|=POW2((Start+Count)%8);
}
/* End Function:Mark_Bitmap **************************************************/

/* Begin Function:Populate_Mem ************************************************
Description : Populate the memory data structure with this memory segment.
              This operation will be conducted with no respect to whether this
              portion have been populated with someone else.
Input       : struct Mem_Map* Map - The memory map.
              ptr_t Start - The start address of the memory.
              ptr_t Size - The size of the memory.
Output      : struct Mem_Map* Map - The updated memory map.
Return      : ret_t - If successful, 0; else -1.
******************************************************************************/
ret_t Populate_Mem(struct Mem_Map* Map, ptr_t Start, ptr_t Size)
{
    ptr_t Mem_Cnt;
    ptr_t Rel_Start;

    for(Mem_Cnt=0;Mem_Cnt<Map->Mem_Num;Mem_Cnt++)
    {
        if((Start>=Map->Mem_Array[Mem_Cnt]->Start)&&
           (Start<=Map->Mem_Array[Mem_Cnt]->Start+(Map->Mem_Array[Mem_Cnt]->Size-1)))
            break;
    }

    /* Must be in this segment. See if we can fit there */
    if(Mem_Cnt==Map->Mem_Num)
        return -1;
    if((Map->Mem_Array[Mem_Cnt]->Start+(Map->Mem_Array[Mem_Cnt]->Size-1))<(Start+(Size-1)))
        return -1;
    
    /* It is clear that we can fit now. Mark all the bits */
    Rel_Start=Start-Map->Mem_Array[Mem_Cnt]->Start;
    Mark_Bitmap(Map->Mem_Bitmap[Mem_Cnt],Rel_Start/4,Size/4);
    return 0;
}
/* End Function:Populate_Mem *************************************************/

/* Begin Function:Fit_Mem *****************************************************
Description : Fit the auto-placed memory segments to a fixed location.
Input       : struct Mem_Map* Map - The memory map.
              ptr_t Mem_Num - The memory info number in the process memory array.
Output      : struct Mem_Map* Map - The updated memory map.
Return      : ret_t - If successful, 0; else -1.
******************************************************************************/
ret_t Fit_Mem(struct Mem_Map* Map, ptr_t Mem_Num)
{
    ptr_t Fit_Cnt;
    ptr_t Start_Addr;
    ptr_t End_Addr;
    ptr_t Try_Addr;
    ptr_t Bitmap_Start;
    ptr_t Bitmap_End;
    struct Mem_Info* Mem;
    struct Mem_Info* Fit;

    Mem=Map->Proc_Mem_Array[Mem_Num];
    /* Find somewhere to fit this memory trunk, and if found, we will populate it */
    for(Fit_Cnt=0;Fit_Cnt<Map->Mem_Num;Fit_Cnt++)
    {
        Fit=Map->Mem_Array[Fit_Cnt];
        if(Mem->Size>Fit->Size)
            continue;
        /* Round start address up, round end address down, to alignment */
        Start_Addr=((Fit->Start+Mem->Align-1)/Mem->Align)*Mem->Align;
        End_Addr=((Fit->Start+Fit->Size)/Mem->Align)*Mem->Align;
        if(Mem->Size>(End_Addr-Start_Addr))
            continue;
        End_Addr-=Mem->Size;
        for(Try_Addr=Start_Addr;Try_Addr<End_Addr;Try_Addr+=Mem->Align)
        {
            Bitmap_Start=(Try_Addr-Fit->Start)/4;
            Bitmap_End=Mem->Size/4;
            if(Try_Bitmap(Map->Mem_Bitmap[Fit_Cnt], Bitmap_Start,Bitmap_End)==0)
            {
                Mark_Bitmap(Map->Mem_Bitmap[Fit_Cnt], Bitmap_Start,Bitmap_End);
                Mem->Start=Try_Addr;
                /* Found a fit */
                return 0;
            }
        }
    }
    /* Can't find any fit */
    return -1;
}
/* End Function:Fit_Mem ******************************************************/

/* Begin Function:Alloc_Mem ***************************************************
Description : Actually allocate all the code memories that are automatically
              placed. After this, all memories will have fixed address ranges.
Input       : struct Proj_Info* Proj - The struct containing the project information.
              struct Chip_Info* Chip - The struct containing the chip information.
              ptr_t Type - The type of the memory, either MEM_CODE or MEM_DATA.
Output      : struct Proj_Info* Proj - The struct containing the project information,
                                       with all memory location allocated.
Return      : None.
******************************************************************************/
void Alloc_Mem(struct Proj_Info* Proj, struct Chip_Info* Chip, ptr_t Type)
{
    ptr_t Count;
    ptr_t Proc_Cnt;
    ptr_t Mem_Cnt;
    struct Mem_Map* Map;

    if((Type!=MEM_CODE)&&(Type!=MEM_DATA))
        EXIT_FAIL("Wrong fitting type.");

    Map=Malloc(sizeof(struct Mem_Map));
    if(Map==0)
        EXIT_FAIL("Memory map allocation failed.");

    /* Find all memory sections with a particular type */
    Count=0;
    for(Mem_Cnt=0;Mem_Cnt<Chip->Mem_Num;Mem_Cnt++)
    {
        if(Chip->Mem[Mem_Cnt].Type==Type)
            Count++;
    }

    Map->Mem_Num=Count;
    Map->Mem_Array=Malloc(sizeof(struct Mem_Info*)*Map->Mem_Num);
    if(Map->Mem_Array==0)
        EXIT_FAIL("Memory map allocation failed.");
    memset(Map->Mem_Array,0,(size_t)(sizeof(struct Mem_Info*)*Map->Mem_Num));
    Map->Mem_Bitmap=Malloc(sizeof(s8*)*Map->Mem_Num);
    if(Map->Mem_Bitmap==0)
        EXIT_FAIL("Memory map allocation failed.");
    
    /* Insert sort according to the start address */
    for(Mem_Cnt=0;Mem_Cnt<Chip->Mem_Num;Mem_Cnt++)
    {
        if(Chip->Mem[Mem_Cnt].Type==Type)
        {
            if(Insert_Mem(Map->Mem_Array,Map->Mem_Num,&Chip->Mem[Mem_Cnt],0)!=0)
                EXIT_FAIL("Code memory insertion sort failed.");
        }
    }

    /* Now allocate the bitmap array according to their size */
    for(Mem_Cnt=0;Mem_Cnt<Map->Mem_Num;Mem_Cnt++)
    {
        /* We insist that one bit represents 4 bytes in the bitmap */
        Map->Mem_Bitmap[Mem_Cnt]=Malloc((Map->Mem_Array[Mem_Cnt]->Size/4)+1);
        if(Map->Mem_Bitmap[Mem_Cnt]==0)
            EXIT_FAIL("Code bitmap allocation failed");
        memset(Map->Mem_Bitmap[Mem_Cnt],0,(size_t)((Map->Mem_Array[Mem_Cnt]->Size/4)+1));
    }

    /* Now populate the RME & RVM sections */
    if(Type==MEM_CODE)
    {
        if(Populate_Mem(Map, Proj->RME.Code_Start,Proj->RME.Code_Size)!=0)
            EXIT_FAIL("Invalid address designated.");
        if(Populate_Mem(Map, Proj->RME.Code_Start+Proj->RME.Code_Size,Proj->RVM.Code_Size)!=0)
            EXIT_FAIL("Invalid address designated.");
    }
    else
    {
        if(Populate_Mem(Map, Proj->RME.Data_Start,Proj->RME.Data_Size)!=0)
            EXIT_FAIL("Invalid address designated.");
        if(Populate_Mem(Map, Proj->RME.Data_Start+Proj->RME.Data_Size,Proj->RVM.Data_Size)!=0)
            EXIT_FAIL("Invalid address designated.");
    }

    /* Find all project code memory sections */
    Count=0;
    for(Proc_Cnt=0;Proc_Cnt<Proj->Proc_Num;Proc_Cnt++)
    {
        for(Mem_Cnt=0;Mem_Cnt<Proj->Proc[Proc_Cnt].Mem_Num;Mem_Cnt++)
        {
            if(Proj->Proc[Proc_Cnt].Mem[Mem_Cnt].Type==Type)
            {
                if(Proj->Proc[Proc_Cnt].Mem[Mem_Cnt].Start==AUTO)
                    Count++;
                else
                {
                    if(Populate_Mem(Map, Proj->Proc[Proc_Cnt].Mem[Mem_Cnt].Start, Proj->Proc[Proc_Cnt].Mem[Mem_Cnt].Size)!=0)
                        EXIT_FAIL("Invalid address designated.");
                }
            }
        }
    }

    if(Count!=0)
    {
        Map->Proc_Mem_Num=Count;
        Map->Proc_Mem_Array=Malloc(sizeof(struct Mem_Info*)*Map->Proc_Mem_Num);
        if(Map->Proc_Mem_Array==0)
            EXIT_FAIL("Memory map allocation failed.");
        memset(Map->Proc_Mem_Array,0,(size_t)(sizeof(struct Mem_Info*)*Map->Proc_Mem_Num));

        /* Insert sort according to size */
        for(Proc_Cnt=0;Proc_Cnt<Proj->Proc_Num;Proc_Cnt++)
        {
            for(Mem_Cnt=0;Mem_Cnt<Proj->Proc[Proc_Cnt].Mem_Num;Mem_Cnt++)
            {
                if(Proj->Proc[Proc_Cnt].Mem[Mem_Cnt].Type==Type)
                {
                    if(Proj->Proc[Proc_Cnt].Mem[Mem_Cnt].Start==AUTO)
                    {
                        if(Insert_Mem(Map->Proc_Mem_Array,Map->Proc_Mem_Num,&(Proj->Proc[Proc_Cnt].Mem[Mem_Cnt]),1)!=0)
                            EXIT_FAIL("Code memory insertion sort failed.");
                    }
                }
            }
        }

        /* Fit whatever that does not have a fixed address */
        for(Mem_Cnt=0;Mem_Cnt<Map->Proc_Mem_Num;Mem_Cnt++)
        {
            if(Fit_Mem(Map,Mem_Cnt)!=0)
                EXIT_FAIL("Memory fitter failed.");
        }

        Free(Map->Proc_Mem_Array);
    }

    /* Clean up before returning */
    for(Mem_Cnt=0;Mem_Cnt<Map->Mem_Num;Mem_Cnt++)
        Free(Map->Mem_Bitmap[Mem_Cnt]);
    
    Free(Map->Mem_Array);
    Free(Map->Mem_Bitmap);
    Free(Map);
}
/* End Function:Alloc_Mem ****************************************************/

/* Begin Function:Check_Mem ***************************************************
Description : Check the memory layout to make sure that they don't overlap.
              Also check if the device memory of all processes are in the device
              memory range, and if all processes have at least a data segment and
              a code segment. If not, we need to abort immediately.
              These algorithms are far from efficient; there are O(nlogn) variants,
              which we leave as a possible future optimization.
Input       : struct Proj_Info* Proj - The struct containing the project information.
              struct Chip_Info* Chip - The struct containing the chip information.
Output      : None.
Return      : None.
******************************************************************************/
void Check_Mem(struct Proj_Info* Proj, struct Chip_Info* Chip)
{
    ptr_t Proc_Cnt;
    ptr_t Mem_Cnt;
    ptr_t Proc_Temp_Cnt;
    ptr_t Mem_Temp_Cnt;
    ptr_t Chip_Mem_Cnt;
    struct Mem_Info* Mem1;
    struct Mem_Info* Mem2;


    /* Is it true that each process have a code segment and a data segment? */
    for(Proc_Cnt=0;Proc_Cnt<Proj->Proc_Num;Proc_Cnt++)
    {
        for(Mem_Cnt=0;Mem_Cnt<Proj->Proc[Proc_Cnt].Mem_Num;Mem_Cnt++)
        {
            if(Proj->Proc[Proc_Cnt].Mem[Mem_Cnt].Type==MEM_CODE)
                break;
        }
        if(Mem_Cnt==Proj->Proc[Proc_Cnt].Mem_Num)
            EXIT_FAIL("At least one process does not have a single code segment.");

        for(Mem_Cnt=0;Mem_Cnt<Proj->Proc[Proc_Cnt].Mem_Num;Mem_Cnt++)
        {
            if(Proj->Proc[Proc_Cnt].Mem[Mem_Cnt].Type==MEM_DATA)
                break;
        }
        if(Mem_Cnt==Proj->Proc[Proc_Cnt].Mem_Num)
            EXIT_FAIL("At least one process does not have a single data segment.");
    }
    
    /* Is it true that the device memory is in device memory range？
     * Also, device memory cannot have AUTO placement, position must be designated. */
    for(Proc_Cnt=0;Proc_Cnt<Proj->Proc_Num;Proc_Cnt++)
    {
        for(Mem_Cnt=0;Mem_Cnt<Proj->Proc[Proc_Cnt].Mem_Num;Mem_Cnt++)
        {
            if(Proj->Proc[Proc_Cnt].Mem[Mem_Cnt].Type==MEM_DEVICE)
            {
                if(Proj->Proc[Proc_Cnt].Mem[Mem_Cnt].Start>=INVALID)
                    EXIT_FAIL("Device memory cannot have auto placement.");

                for(Chip_Mem_Cnt=0;Chip_Mem_Cnt<Chip->Mem_Num;Chip_Mem_Cnt++)
                {
                    if(Chip->Mem[Chip_Mem_Cnt].Type==MEM_DEVICE)
                    {
                        Mem1=&(Chip->Mem[Chip_Mem_Cnt]);
                        Mem2=&(Proj->Proc[Proc_Cnt].Mem[Mem_Cnt]);
                        if((Mem1->Start<=Mem2->Start)&&((Mem1->Start+(Mem1->Size-1))>=(Mem2->Start+(Mem2->Size-1))))
                            break;
                    }
                }
                if(Chip_Mem_Cnt==Chip->Mem_Num)
                    EXIT_FAIL("At least one device memory segment is out of bound.");
            }
        }
    }

    /* Is it true that the primary code memory does not overlap with each other? */
    for(Proc_Cnt=0;Proc_Cnt<Proj->Proc_Num;Proc_Cnt++)
    {
        for(Mem_Cnt=0;Mem_Cnt<Proj->Proc[Proc_Cnt].Mem_Num;Mem_Cnt++)
        {
            if(Proj->Proc[Proc_Cnt].Mem[Mem_Cnt].Type==MEM_CODE)
                break;
        }
        for(Proc_Temp_Cnt=0;Proc_Temp_Cnt<Proj->Proc_Num;Proc_Temp_Cnt++)
        {
            if(Proc_Temp_Cnt==Proc_Cnt)
                continue;
            for(Mem_Temp_Cnt=0;Mem_Cnt<Proj->Proc[Proc_Temp_Cnt].Mem_Num;Mem_Cnt++)
            {
                if(Proj->Proc[Proc_Temp_Cnt].Mem[Mem_Temp_Cnt].Type==MEM_CODE)
                    break;
            }
            
            Mem1=&(Proj->Proc[Proc_Cnt].Mem[Mem_Cnt]);
            Mem2=&(Proj->Proc[Proc_Temp_Cnt].Mem[Mem_Temp_Cnt]);

            if(((Mem1->Start+(Mem1->Size-1))<Mem2->Start)||((Mem2->Start+(Mem2->Size-1))<Mem1->Start))
                continue;
            else
                EXIT_FAIL("Two process's main code sections overlapped.");
        }
    }
}
/* End Function:Check_Mem ****************************************************/

/* Begin Function:Strcicmp ****************************************************
Description : Compare two strings in a case insensitive way.
Input       : s8* Str1 - The first string.
              s8* Str2 - The second string.
Output      : None.
Return      : ret_t - If two strings are equal, then 0; if the first is bigger, 
                      then positive; else negative.
******************************************************************************/
ret_t Strcicmp(s8* Str1, s8* Str2)
{
    ptr_t Count;
    ret_t Result;

    Count=0;
    while(1)
    {
        Result=tolower(Str1[Count])-tolower(Str2[Count]);
        if(Result!=0)
            return Result;
        
        if(Str1[Count]=='\0')
            break;

        Count++;
    }

    return Result;
}
/* End Function:Strcicmp *****************************************************/

/* Begin Function:Validate_Name ***********************************************
Description : See if the names are validate C identifiers.
Input       : struct Proj_Info* Proj - The project information struct.
Output      : None.
Return      : ret_t - If no conflict, 0; else -1.
******************************************************************************/
ret_t Validate_Name(s8* Name)
{
    ptr_t Count;
    /* Should not begin with number */
    if((Name[0]>='0')&&(Name[0]<='9'))
        return -1;
    Count=0;
    while(1)
    {
        Count++;
        if(Name[Count]=='\0')
            return 0;
        if((Name[Count]>='a')&&(Name[Count]<='z'))
            continue;
        if((Name[Count]>='A')&&(Name[Count]<='Z'))
            continue;
        if((Name[Count]>='0')&&(Name[Count]<='9'))
            continue;
        if(Name[Count]=='_')
            continue;
        break;
    }
    return -1;
}
/* End Function:Validate_Name ************************************************/

/* Begin Function:Detect_Vector ***********************************************
Description : Detect vector conflicts in the system.
Input       : struct Proj_Info* Proj - The project information struct.
Output      : None.
Return      : None.
******************************************************************************/
void Detect_Vector(struct Proj_Info* Proj)
{
    ptr_t Proc_Cnt;
    ptr_t Proc_Tmp_Cnt;
    ptr_t Obj_Cnt;
    ptr_t Obj_Tmp_Cnt;
    struct Proc_Info* Proc;
    struct Vect_Info* Vect;

    for(Proc_Cnt=0;Proc_Cnt<Proj->Proc_Num;Proc_Cnt++)
    {
        Proc=&(Proj->Proc[Proc_Cnt]);
        /* Check that every vector name is globally unique - it cannot overlap with any other
         * vector in any other process. If one of the processes have a vector endpoint, then
         * no other process can have the same one. */
        for(Obj_Cnt=0;Obj_Cnt<Proc->Vect_Num;Obj_Cnt++)
        {
            Vect=&(Proc->Vect[Obj_Cnt]);
            for(Proc_Tmp_Cnt=0;Proc_Tmp_Cnt<Proj->Proc_Num;Proc_Tmp_Cnt++)
            {
                for(Obj_Tmp_Cnt=0;Obj_Tmp_Cnt<Proj->Proc[Proc_Tmp_Cnt].Vect_Num;Obj_Tmp_Cnt++)
                {
                    if((Proc_Cnt==Proc_Tmp_Cnt)&&(Obj_Cnt==Obj_Tmp_Cnt))
                        continue;
                    if(Strcicmp(Proj->Proc[Proc_Tmp_Cnt].Vect[Obj_Tmp_Cnt].Name, Vect->Name)==0)
                        EXIT_FAIL("Duplicate vectors endpoints is now allowed.");
                }
            }
        }
    }       
}
/* End Function:Detect_Vector ************************************************/

/* Begin Function:Detect_Conflict *********************************************
Description : Detect namespace conflicts in the system. It also checks if the
              names are at least regular C identifiers.
Input       : struct Proj_Info* Proj - The project information struct.
Output      : None.
Return      : None.
******************************************************************************/
void Detect_Conflict(struct Proj_Info* Proj)
{
    ptr_t Proc_Cnt;
    ptr_t Obj_Cnt;
    ptr_t Count;
    struct Proc_Info* Proc;

    /* Are there two processes with the same name? */
    for(Proc_Cnt=0;Proc_Cnt<Proj->Proc_Num;Proc_Cnt++)
    {
        Proc=&(Proj->Proc[Proc_Cnt]);
        /* Check for duplicate processes */
        if(Validate_Name(Proc->Name)!=0)
            EXIT_FAIL("Invalid process name.");
        for(Count=0;Count<Proj->Proc_Num;Count++)
        {
            if((Count!=Proc_Cnt)&&(Strcicmp(Proc->Name,Proj->Proc[Count].Name)==0))
                EXIT_FAIL("Duplicate process name.");
        }
        /* Check for duplicate threads */
        for(Obj_Cnt=0;Obj_Cnt<Proc->Thd_Num;Obj_Cnt++)
        {
            if(Validate_Name(Proc->Thd[Obj_Cnt].Name)!=0)
                EXIT_FAIL("Invalid thread name.");
            for(Count=0;Count<Proc->Thd_Num;Count++)
            {
                if((Count!=Obj_Cnt)&&(Strcicmp(Proc->Thd[Count].Name,Proc->Thd[Obj_Cnt].Name)==0))
                    EXIT_FAIL("Duplicate thread name.");
            }
        }
        /* Check for duplicate invocations */
        for(Obj_Cnt=0;Obj_Cnt<Proc->Inv_Num;Obj_Cnt++)
        {
            if(Validate_Name(Proc->Inv[Obj_Cnt].Name)!=0)
                EXIT_FAIL("Invalid invocation name.");
            for(Count=0;Count<Proc->Inv_Num;Count++)
            {
                if((Count!=Obj_Cnt)&&(Strcicmp(Proc->Inv[Count].Name,Proc->Inv[Obj_Cnt].Name)==0))
                    EXIT_FAIL("Duplicate invocation name");
            }
        }
        /* Check for duplicate ports */
        for(Obj_Cnt=0;Obj_Cnt<Proc->Port_Num;Obj_Cnt++)
        {
            if(Validate_Name(Proc->Port[Obj_Cnt].Name)!=0)
                EXIT_FAIL("Invalid port name.");
            if(Validate_Name(Proc->Port[Obj_Cnt].Process)!=0)
                EXIT_FAIL("Invalid port process name.");
            if(Strcicmp(Proc->Port[Obj_Cnt].Process,Proc->Name)==0)
                EXIT_FAIL("Port cannot target within the same process.");
            for(Count=0;Count<Proc->Port_Num;Count++)
            {
                if((Count!=Obj_Cnt)&&
                   (Strcicmp(Proc->Port[Count].Name,Proc->Port[Obj_Cnt].Name)==0)&&
                   (Strcicmp(Proc->Port[Count].Process,Proc->Port[Obj_Cnt].Process)==0))
                    EXIT_FAIL("Duplicate port name");
            }
        }
        /* Check for duplicate receive endpoints */
        for(Obj_Cnt=0;Obj_Cnt<Proc->Recv_Num;Obj_Cnt++)
        {
            if(Validate_Name(Proc->Recv[Obj_Cnt].Name)!=0)
                EXIT_FAIL("Invalid receive endpoint name.");
            for(Count=0;Count<Proc->Recv_Num;Count++)
            {
                if(Count!=Obj_Cnt)
                {
                    if(Strcicmp(Proc->Recv[Count].Name,Proc->Recv[Obj_Cnt].Name)==0)
                        EXIT_FAIL("Duplicate receive endpoint name");
                }
            }
        }
        /* Check for duplicate send endpoints */
        for(Obj_Cnt=0;Obj_Cnt<Proc->Send_Num;Obj_Cnt++)
        {
            if(Validate_Name(Proc->Send[Obj_Cnt].Name)!=0)
                EXIT_FAIL("Invalid send endpoint name.");
            if(Validate_Name(Proc->Send[Obj_Cnt].Process)!=0)
                EXIT_FAIL("Invalid endpoint process name.");
            for(Count=0;Count<Proc->Send_Num;Count++)
            {
                if(Count!=Obj_Cnt)
                {
                    if((Strcicmp(Proc->Send[Count].Name,Proc->Send[Obj_Cnt].Name)==0)&&
                       (Strcicmp(Proc->Send[Count].Process,Proc->Send[Obj_Cnt].Process)==0))
                        EXIT_FAIL("Duplicate send endpoint name");
                }
            }
        }
    }

    /* Check for duplicate vector endpoints- they are globally unique */
    Detect_Vector(Proj);
}
/* End Function:Detect_Conflict **********************************************/

/* Begin Function:Alloc_Local_Capid *******************************************
Description : Allocate local capability IDs for all kernel objects. 
              We always allocate threads first, then invocations, then ports,
              then endpoints.
Input       : struct Proj_Info* Proj - The project structure.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : None.
******************************************************************************/
void Alloc_Local_Capid(struct Proj_Info* Proj)
{
    ptr_t Proc_Cnt;
    ptr_t Obj_Cnt;
    ptr_t Capid;
    struct Proc_Info* Proc;

    for(Proc_Cnt=0;Proc_Cnt<Proj->Proc_Num;Proc_Cnt++)
    {
        Capid=0;
        Proc=&(Proj->Proc[Proc_Cnt]);
        for(Obj_Cnt=0;Obj_Cnt<Proc->Thd_Num;Obj_Cnt++)
        {
            Proc->Thd[Obj_Cnt].Capid=Capid;
            Capid++;
        }
        for(Obj_Cnt=0;Obj_Cnt<Proc->Inv_Num;Obj_Cnt++)
        {
            Proc->Inv[Obj_Cnt].Capid=Capid;
            Capid++;
        }
        for(Obj_Cnt=0;Obj_Cnt<Proc->Port_Num;Obj_Cnt++)
        {
            Proc->Port[Obj_Cnt].Capid=Capid;
            Capid++;
        }
        for(Obj_Cnt=0;Obj_Cnt<Proc->Recv_Num;Obj_Cnt++)
        {
            Proc->Recv[Obj_Cnt].Capid=Capid;
            Capid++;
        }
        for(Obj_Cnt=0;Obj_Cnt<Proc->Send_Num;Obj_Cnt++)
        {
            Proc->Send[Obj_Cnt].Capid=Capid;
            Capid++;
        }
        for(Obj_Cnt=0;Obj_Cnt<Proc->Vect_Num;Obj_Cnt++)
        {
            Proc->Vect[Obj_Cnt].Capid=Capid;
            Capid++;
        }
        Proc->Captbl_Front=Capid;
    }
}
/* End Function:Alloc_Local_Capid ********************************************/

/* Begin Function:Alloc_Global_Capid ******************************************
Description : Allocate (relative) global capability IDs for all kernel objects. 
              Each global object will reside in its onw capability table. 
              This facilitates management, and circumvents the capability size
              limit that may present on 32-bit systems.
Input       : struct Proj_Info* Proj - The project structure.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : None.
******************************************************************************/
void Alloc_Global_Capid(struct Proj_Info* Proj)
{
    /* How many distinct kernel objects are there? We just need to add up the
     * following: All captbls (each process have one), all processes, all threads,
     * all invocations, all receive endpoints. The ports and send endpoints do not
     * have a distinct kernel object; the vector endpoints are created by the kernel
     * at boot-time, while the pgtbls are decided by architecture-specific code. */
    ptr_t Proc_Cnt;
    ptr_t Obj_Cnt;
    ptr_t Capid;
    struct Proc_Info* Proc;

    /* Fill in all captbls */
    Proj->RVM.Captbl_Captbl=Malloc(sizeof(struct RVM_Cap_Info)*Proj->Proc_Num);
    if(Proj->RVM.Captbl_Captbl==0)
        EXIT_FAIL("Global capability table for capability tables allocation failed.");
    Proj->RVM.Captbl_Captbl_Front=Proj->Proc_Num;
    Capid=0;
    for(Proc_Cnt=0;Proc_Cnt<Proj->Proc_Num;Proc_Cnt++)
    {
        Proc=&(Proj->Proc[Proc_Cnt]);
        Proj->RVM.Captbl_Captbl[Capid].Proc=Proc;
        Proj->RVM.Captbl_Captbl[Capid].Cap=Proc;
        Proc->RVM_Captbl_Capid=Capid;
        Capid++;
    }
    /* Fill in all processes */
    Proj->RVM.Proc_Captbl=Malloc(sizeof(struct RVM_Cap_Info)*Proj->Proc_Num);
    if(Proj->RVM.Proc_Captbl==0)
        EXIT_FAIL("Global capability table for processes allocation failed.");
    Proj->RVM.Proc_Captbl_Front=Proj->Proc_Num;
    Capid=0;
    for(Proc_Cnt=0;Proc_Cnt<Proj->Proc_Num;Proc_Cnt++)
    {
        Proc=&(Proj->Proc[Proc_Cnt]);
        Proj->RVM.Proc_Captbl[Capid].Proc=Proc;
        Proj->RVM.Proc_Captbl[Capid].Cap=Proc;
        Proc->RVM_Proc_Capid=Capid;
        Capid++;
    }
    /* Fill in all threads */
    Capid=0;
    for(Proc_Cnt=0;Proc_Cnt<Proj->Proc_Num;Proc_Cnt++)
        Capid+=Proj->Proc[Proc_Cnt].Thd_Num;
    Proj->RVM.Thd_Captbl_Front=Capid;
    Proj->RVM.Thd_Captbl=Malloc(sizeof(struct RVM_Cap_Info)*Proj->RVM.Thd_Captbl_Front);
    if(Proj->RVM.Thd_Captbl==0)
        EXIT_FAIL("Global capability table for threads failed.");
    Capid=0;
    for(Proc_Cnt=0;Proc_Cnt<Proj->Proc_Num;Proc_Cnt++)
    {
        Proc=&(Proj->Proc[Proc_Cnt]);
        for(Obj_Cnt=0;Obj_Cnt<Proc->Thd_Num;Obj_Cnt++)
        {
            Proj->RVM.Thd_Captbl[Capid].Proc=Proc;
            Proj->RVM.Thd_Captbl[Capid].Cap=&(Proc->Thd[Obj_Cnt]);
            Proc->Thd[Obj_Cnt].RVM_Capid=Capid;
            Capid++;
        }
    }
    /* Fill in all invocations */
    Capid=0;
    for(Proc_Cnt=0;Proc_Cnt<Proj->Proc_Num;Proc_Cnt++)
        Capid+=Proj->Proc[Proc_Cnt].Inv_Num;
    Proj->RVM.Inv_Captbl_Front=Capid;
    Proj->RVM.Inv_Captbl=Malloc(sizeof(struct RVM_Cap_Info)*Proj->RVM.Inv_Captbl_Front);
    if(Proj->RVM.Inv_Captbl==0)
        EXIT_FAIL("Global capability table for invocations failed.");
    Capid=0;
    for(Proc_Cnt=0;Proc_Cnt<Proj->Proc_Num;Proc_Cnt++)
    {
        Proc=&(Proj->Proc[Proc_Cnt]);
        for(Obj_Cnt=0;Obj_Cnt<Proc->Inv_Num;Obj_Cnt++)
        {
            Proj->RVM.Inv_Captbl[Capid].Proc=Proc;
            Proj->RVM.Inv_Captbl[Capid].Cap=&(Proc->Inv[Obj_Cnt]);
            Proc->Inv[Obj_Cnt].RVM_Capid=Capid;
            Capid++;
        }
    }
    /* Fill in all receive endpoints */
    Capid=0;
    for(Proc_Cnt=0;Proc_Cnt<Proj->Proc_Num;Proc_Cnt++)
        Capid+=Proj->Proc[Proc_Cnt].Recv_Num;
    Proj->RVM.Recv_Captbl_Front=Capid;
    if(Capid!=0)
    {
        Proj->RVM.Recv_Captbl=Malloc(sizeof(struct RVM_Cap_Info)*Proj->RVM.Recv_Captbl_Front);
        if(Proj->RVM.Recv_Captbl==0)
            EXIT_FAIL("Global capability table for receive endpoints failed.");
        Capid=0;
        for(Proc_Cnt=0;Proc_Cnt<Proj->Proc_Num;Proc_Cnt++)
        {
            Proc=&(Proj->Proc[Proc_Cnt]);
            for(Obj_Cnt=0;Obj_Cnt<Proc->Recv_Num;Obj_Cnt++)
            {
                Proj->RVM.Recv_Captbl[Capid].Proc=Proc;
                Proj->RVM.Recv_Captbl[Capid].Cap=&(Proc->Recv[Obj_Cnt]);
                Proc->Recv[Obj_Cnt].RVM_Capid=Capid;
                Capid++;
            }
        }
    }
    /* Fill in all vector endpoints */
    Capid=0;
    for(Proc_Cnt=0;Proc_Cnt<Proj->Proc_Num;Proc_Cnt++)
        Capid+=Proj->Proc[Proc_Cnt].Vect_Num;
    Proj->RVM.Vector_Captbl_Front=Capid;
    if(Capid!=0)
    {
        Proj->RVM.Vector_Captbl=Malloc(sizeof(struct RVM_Cap_Info)*Proj->RVM.Vector_Captbl_Front);
        if(Proj->RVM.Vector_Captbl==0)
            EXIT_FAIL("Global capability table for vector endpoints failed.");
        Capid=0;
        for(Proc_Cnt=0;Proc_Cnt<Proj->Proc_Num;Proc_Cnt++)
        {
            Proc=&(Proj->Proc[Proc_Cnt]);
            for(Obj_Cnt=0;Obj_Cnt<Proc->Vect_Num;Obj_Cnt++)
            {
                Proj->RVM.Vector_Captbl[Capid].Proc=Proc;
                Proj->RVM.Vector_Captbl[Capid].Cap=&(Proc->Vect[Obj_Cnt]);
                Proc->Vect[Obj_Cnt].RVM_Capid=Capid;
                Capid++;
            }
        }
    }
}
/* End Function:Alloc_Global_Capid *******************************************/

/* Begin Function:Make_Macro **************************************************
Description : Concatenate at most 4 parts into a macro, and turn everything uppercase.
Input       : s8* Str1 - The first part.
              s8* Str2 - The second part.
              s8* Str3 - The third part.
              s8* Str4 - The fourth part.
Output      : None.
Return      : s8* - The macro returned. This is allocated memory.
******************************************************************************/
s8* Make_Macro(s8* Str1, s8* Str2, s8* Str3, s8* Str4)
{
    ptr_t Count;
    ptr_t Len;
    s8* Ret;

    /* Print to buffer */
    Len=snprintf(NULL,0,"%s%s%s%s",Str1,Str2,Str3,Str4);
    Ret=Malloc(Len+1);
    if(Ret==0)
        EXIT_FAIL("Macro buffer memory allocation failed.");
    snprintf(Ret,(size_t)(Len+1),"%s%s%s%s",Str1,Str2,Str3,Str4);
    /* Turn everything to uppercase */
    for(Count=0;Count<Len;Count++)
        Ret[Count]=toupper(Ret[Count]);

    return Ret;
}
/* End Function:Make_Macro ***************************************************/

/* Begin Function:Alloc_Capid_Macros ******************************************
Description : Allocate the capability ID macros. Both the local one and the global
              one will be allocated. It might be better if we separate send, receive and handler...
              just separate them into different sections. This will make things much easier to write,
              and doesn't complicate the stuff really as much.
              The allocation table is shown below:
-------------------------------------------------------------------------------
Type            Local                           Global
-------------------------------------------------------------------------------
Process         -                               RVM_PROC_<PROCNAME>
-------------------------------------------------------------------------------
Captbl          -                               RVM_CAPTBL_<PROCNAME>
-------------------------------------------------------------------------------
Thread          THD_<THDNAME>                   RVM_PROC_<PROCNAME>_THD_<THDNAME>
-------------------------------------------------------------------------------
Invocation      INV_<INVNAME>                   RVM_PROC_<PROCNAME>_INV_<INVNAME>
-------------------------------------------------------------------------------
Port            PROC_<PROCNAME>_PORT_<PORTNAME> (Inherit invocation name)
-------------------------------------------------------------------------------
Receive         RECV_<ENDPNAME>                 RVM_PROC_<PROCNAME>_RECV_<RECVNAME>
-------------------------------------------------------------------------------
Send            PROC_<PROCNAME>_SEND_<ENDPNAME> (Inherit receive endpoint name)
-------------------------------------------------------------------------------
Vector          VECT_<VECTNAME>                 RVM_BOOT_VECT_<VECTNAME> (RVM)
                                                RME_BOOT_VECT_<VECTNAME> (RME)
-------------------------------------------------------------------------------
Input       : struct Proj_Info* Proj - The project structure.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : None.
******************************************************************************/
void Alloc_Capid_Macros(struct Proj_Info* Proj)
{
    ptr_t Proc_Cnt;
    ptr_t Obj_Cnt;

    for(Proc_Cnt=0;Proc_Cnt<Proj->Proc_Num;Proc_Cnt++)
    {
        /* Processes and their capability tables */
        Proj->Proc[Proc_Cnt].RVM_Proc_Capid_Macro=Make_Macro("RVM_PROC_",Proj->Proc[Proc_Cnt].Name,"","");
        Proj->Proc[Proc_Cnt].RVM_Captbl_Capid_Macro=Make_Macro("RVM_CAPTBL_",Proj->Proc[Proc_Cnt].Name,"","");
        /* Threads */
        for(Obj_Cnt=0;Obj_Cnt<Proj->Proc[Proc_Cnt].Thd_Num;Obj_Cnt++)
        {
            Proj->Proc[Proc_Cnt].Thd[Obj_Cnt].Capid_Macro=Make_Macro("THD_",Proj->Proc[Proc_Cnt].Thd[Obj_Cnt].Name,"","");
            Proj->Proc[Proc_Cnt].Thd[Obj_Cnt].RVM_Capid_Macro=Make_Macro("RVM_PROC_",Proj->Proc[Proc_Cnt].Name,
                                                                         "_THD_",Proj->Proc[Proc_Cnt].Thd[Obj_Cnt].Name);
        }
        /* Invocations */
        for(Obj_Cnt=0;Obj_Cnt<Proj->Proc[Proc_Cnt].Inv_Num;Obj_Cnt++)
        {
            Proj->Proc[Proc_Cnt].Inv[Obj_Cnt].Capid_Macro=Make_Macro("INV_",Proj->Proc[Proc_Cnt].Inv[Obj_Cnt].Name,"","");
            Proj->Proc[Proc_Cnt].Inv[Obj_Cnt].RVM_Capid_Macro=Make_Macro("RVM_PROC_",Proj->Proc[Proc_Cnt].Name,
                                                                         "_INV_",Proj->Proc[Proc_Cnt].Inv[Obj_Cnt].Name);
        }
        /* Ports */
        for(Obj_Cnt=0;Obj_Cnt<Proj->Proc[Proc_Cnt].Port_Num;Obj_Cnt++)
            Proj->Proc[Proc_Cnt].Port[Obj_Cnt].Capid_Macro=Make_Macro("PORT_",Proj->Proc[Proc_Cnt].Port[Obj_Cnt].Name,"","");
        /* Receive endpoints */
        for(Obj_Cnt=0;Obj_Cnt<Proj->Proc[Proc_Cnt].Recv_Num;Obj_Cnt++)
        {
            Proj->Proc[Proc_Cnt].Recv[Obj_Cnt].Capid_Macro=Make_Macro("RECV_",Proj->Proc[Proc_Cnt].Recv[Obj_Cnt].Name,"","");
            Proj->Proc[Proc_Cnt].Recv[Obj_Cnt].RVM_Capid_Macro=Make_Macro("RVM_PROC_",Proj->Proc[Proc_Cnt].Name,
                                                                          "_RECV_",Proj->Proc[Proc_Cnt].Recv[Obj_Cnt].Name);
        }
        /* Send endpoints */
        for(Obj_Cnt=0;Obj_Cnt<Proj->Proc[Proc_Cnt].Send_Num;Obj_Cnt++)
            Proj->Proc[Proc_Cnt].Send[Obj_Cnt].Capid_Macro=Make_Macro("SEND_",Proj->Proc[Proc_Cnt].Send[Obj_Cnt].Name,"","");
        /* Vector endpoints */
        for(Obj_Cnt=0;Obj_Cnt<Proj->Proc[Proc_Cnt].Vect_Num;Obj_Cnt++)
        {
            Proj->Proc[Proc_Cnt].Vect[Obj_Cnt].Capid_Macro=Make_Macro("VECT_",Proj->Proc[Proc_Cnt].Vect[Obj_Cnt].Name,"","");
            Proj->Proc[Proc_Cnt].Vect[Obj_Cnt].RVM_Capid_Macro=Make_Macro("RVM_BOOT_VECT_",Proj->Proc[Proc_Cnt].Vect[Obj_Cnt].Name,"","");
            Proj->Proc[Proc_Cnt].Vect[Obj_Cnt].RME_Capid_Macro=Make_Macro("RME_BOOT_VECT_",Proj->Proc[Proc_Cnt].Vect[Obj_Cnt].Name,"","");
        }
    }
}
/* End Function:Alloc_Capid_Macros *******************************************/

/* Begin Function:Backprop_Global_Capid ***************************************
Description : Back propagate the global ID to all the ports and send endpoints,
              which are derived from kernel objects. Also detects if all the port
              and send endpoint names in the system are valid. If any of them includes
              dangling references to invocations and receive endpoints, abort.
              These comparisons use strcmp because we require that the process name
              cases match.
Input       : struct Proj_Info* Proj - The project information structure.
              struct Chip_Info* Chip - The chip information structure.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : None.
******************************************************************************/
void Backprop_Global_Capid(struct Proj_Info* Proj, struct Chip_Info* Chip)
{
    ptr_t Proc_Cnt;
    ptr_t Proc_Tmp_Cnt;
    ptr_t Obj_Cnt;
    ptr_t Obj_Tmp_Cnt;
    struct Proc_Info* Proc;
    struct Port_Info* Port;
    struct Send_Info* Send;
    struct Vect_Info* Vect;

    for(Proc_Cnt=0;Proc_Cnt<Proj->Proc_Num;Proc_Cnt++)
    {
        Proc=&(Proj->Proc[Proc_Cnt]);
        /* For every port, there must be a invocation somewhere */
        for(Obj_Cnt=0;Obj_Cnt<Proc->Port_Num;Obj_Cnt++)
        {
            Port=&(Proc->Port[Obj_Cnt]);
            for(Proc_Tmp_Cnt=0;Proc_Tmp_Cnt<Proj->Proc_Num;Proc_Tmp_Cnt++)
            {
                if(strcmp(Proj->Proc[Proc_Tmp_Cnt].Name, Port->Process)==0)
                    break;
            }
            if(Proc_Tmp_Cnt==Proj->Proc_Num)
                EXIT_FAIL("Invalid process for port.");
            for(Obj_Tmp_Cnt=0;Obj_Tmp_Cnt<Proj->Proc[Proc_Tmp_Cnt].Inv_Num;Obj_Tmp_Cnt++)
            {
                if(strcmp(Proj->Proc[Proc_Tmp_Cnt].Inv[Obj_Tmp_Cnt].Name, Port->Name)==0)
                {
                    Port->RVM_Capid=Proj->Proc[Proc_Tmp_Cnt].Inv[Obj_Tmp_Cnt].RVM_Capid;
                    Port->RVM_Capid_Macro=Proj->Proc[Proc_Tmp_Cnt].Inv[Obj_Tmp_Cnt].RVM_Capid_Macro;
                    break;
                }
            }
            if(Obj_Tmp_Cnt==Proj->Proc[Proc_Tmp_Cnt].Inv_Num)
                EXIT_FAIL("One of the ports does not have a corresponding invocation.");
        }
        /* For every send endpoint, there must be a receive endpoint somewhere */
        for(Obj_Cnt=0;Obj_Cnt<Proc->Send_Num;Obj_Cnt++)
        {
            Send=&(Proc->Send[Obj_Cnt]);
            for(Proc_Tmp_Cnt=0;Proc_Tmp_Cnt<Proj->Proc_Num;Proc_Tmp_Cnt++)
            {
                if(strcmp(Proj->Proc[Proc_Tmp_Cnt].Name, Send->Process)==0)
                    break;
            }
            if(Proc_Tmp_Cnt==Proj->Proc_Num)
                EXIT_FAIL("Invalid process for endpoint.");
            for(Obj_Tmp_Cnt=0;Obj_Tmp_Cnt<Proj->Proc[Proc_Tmp_Cnt].Recv_Num;Obj_Tmp_Cnt++)
            {
                if(strcmp(Proj->Proc[Proc_Tmp_Cnt].Recv[Obj_Tmp_Cnt].Name, Send->Name)==0)
                {
                    Send->RVM_Capid=Proj->Proc[Proc_Tmp_Cnt].Recv[Obj_Tmp_Cnt].RVM_Capid;
                    Send->RVM_Capid_Macro=Proj->Proc[Proc_Tmp_Cnt].Recv[Obj_Tmp_Cnt].RVM_Capid_Macro;
                    break;
                }
            }
            if(Obj_Tmp_Cnt==Proj->Proc[Proc_Tmp_Cnt].Recv_Num)
                EXIT_FAIL("One of the send endpoints does not have a corresponding receive endpoint.");
        }
        /* For every vector, there must be a corresponding chip interrupt vector somewhere */
        for(Obj_Cnt=0;Obj_Cnt<Proc->Vect_Num;Obj_Cnt++)
        {
            Vect=&(Proc->Vect[Obj_Cnt]);
            for(Obj_Tmp_Cnt=0;Obj_Tmp_Cnt<Chip->Vect_Num;Obj_Tmp_Cnt++)
            {
                if(strcmp(Chip->Vect[Obj_Tmp_Cnt].Name, Vect->Name)==0)
                {
                    Vect->Number=Chip->Vect[Obj_Tmp_Cnt].Number;
                    break;
                }
            }
            if(Obj_Tmp_Cnt==Chip->Vect_Num)
                EXIT_FAIL("One of the vector endpoints does not have a corresponding chip vector.");
        }
    }
}
/* End Function:Backprop_Global_Capid ****************************************/

/* Begin Function:Alloc_Captbl ************************************************
Description : Allocate the capability table entries for the processes, then for
              the RVM.
Input       : struct Proj_Info* Proj - The project structure.
              struct Chip_Info* Chip - The chip structure.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : None.
******************************************************************************/
void Alloc_Captbl(struct Proj_Info* Proj,  struct Chip_Info* Chip)
{
    /* First, check whether there are conflicts - this is not case insensitive */
    Detect_Conflict(Proj);
    /* Allocate local project IDs for all entries */
    Alloc_Local_Capid(Proj);
    /* Allocate global project IDs for kernel object entries */
    Alloc_Global_Capid(Proj);
    /* Allocate the global and local macros to them */
    Alloc_Capid_Macros(Proj);
    /* Back propagate global entrie number to the ports and send endpoints */
    Backprop_Global_Capid(Proj, Chip);
}
/* End Function:Alloc_Captbl *************************************************/

/* Begin Function:Make_Str ****************************************************
Description : Concatenate two strings and return the result.
Input       : s8* Str1 - The first string.
              s8* Str2 - The second string.
Output      : None.
Return      : s8* - The final result.
******************************************************************************/
s8* Make_Str(s8* Str1, s8* Str2)
{
    s8* Ret;

    Ret=Malloc(strlen(Str1)+strlen(Str2)+1);
    if(Ret==0)
        return 0;

    strcpy(Ret, Str1);
    strcat(Ret, Str2);

    return Ret;
}
/* End Function:Make_Str *****************************************************/

/* Begin Function:Raw_Match ***************************************************
Description : Match the raw value tag and provide the pointer to the raw value string.
Input       : struct Raw_Info* Info - The raw data information structure.
              s8* Tag - The tag for the information.
Output      : None.
Return      : s8* - The value.
******************************************************************************/
s8* Raw_Match(struct Raw_Info* Info, s8* Tag)
{
    ptr_t Count;

    for(Count=0;Count<Info->Num;Count++)
    {
        if(strcmp(Info->Tag[Count],Tag)==0)
            return Info->Value[Count];
    }

    return 0;
}
/* End Function:Raw_Match ****************************************************/

/* Begin Function:Write_Src_Desc **********************************************
Description : Output the header that is sticked to every C file.
Input       : FILE* File - The pointer to the file.
              s8* Filename - The name of the file.
              s8* Author - The author of the file.
              s8* Date - The date of the file.
              s8* License - The license of the file.
              s8* Description - The description of the file.
Output      : FILE* File - The pointer to the updated file.
Return      : None.
******************************************************************************/
void Write_Src_Desc(FILE* File, s8* Filename, s8* Author, s8* Date, s8* License, s8* Description)
{
    fprintf(File, "/******************************************************************************\n");
    fprintf(File, "Filename    : %s\n", Filename);
    fprintf(File, "Author      : %s\n", Author);
    fprintf(File, "Date        : %s\n", Date);
    fprintf(File, "License     : %s\n", License);
    fprintf(File, "Description : %s\n", Description);
    fprintf(File, "******************************************************************************/\n\n");
}
/* End Function:Write_Src_Desc ***********************************************/

/* Begin Function:Write_Src_Footer ********************************************
Description : Output the footer that is appended to every C file.
Input       : FILE* File - The pointer to the file.
Output      : FILE* File - The pointer to the updated file.
Return      : None.
******************************************************************************/
void Write_Src_Footer(FILE* File)
{
    fprintf(File, "/* End Of File ***************************************************************/\n\n");
    fprintf(File, "/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/\n");
}
/* End Function:Write_Src_Footer *********************************************/

/* Begin Function:Write_Func_Desc *********************************************
Description : Output the header that is sticked to every C function.
Input       : FILE* File - The pointer to the file.
              s8* Funcname - The name of the function.
Output      : FILE* File - The pointer to the updated file.
Return      : None.
******************************************************************************/
void Write_Func_Desc(FILE* File, s8* Funcname)
{
    ptr_t Len;
    s8 Buf[256];

    for(Len=sprintf(Buf, "/* Begin Function:%s ", Funcname);Len<80;Len++)
        Buf[Len]='*';
    Buf[Len]='\0';
    fprintf(File, "%s\n",Buf);
}
/* End Function:Write_Func_Desc **********************************************/

/* Begin Function:Write_Func_Footer *******************************************
Description : Output the footer that is appended to every C function.
Input       : FILE* File - The pointer to the file.
              s8* Funcname - The name of the function.
Output      : FILE* File - The pointer to the updated file.
Return      : None.
******************************************************************************/
void Write_Func_Footer(FILE* File, s8* Funcname)
{
    ptr_t Len;
    s8 Buf[256];

    for(Len=sprintf(Buf, "/* End Function:%s ", Funcname);Len<79;Len++)
        Buf[Len]='*';
    Buf[Len]='/';
    Buf[Len+1]='\0';
    fprintf(File, "%s\n\n",Buf);
}
/* End Function:Write_Func_Footer ********************************************/

/* Begin Function:Make_Define_Str *********************************************
Description : Make a define statement in the file. The define statement can have
              three parts, which will be converted to uppercase and concatenated
              together.
              The value here is a string.
Input       : FILE* File - The file structure.
              s8* Macro - The macro.
              s8* Value - The value of the macro.
              s8* Align - The alignment, must be bigger than 12.
Output      : None.
Return      : None.
******************************************************************************/
void Make_Define_Str(FILE* File, s8* Macro, s8* Value, ptr_t Align)
{
    s8 Buf[32];

    /* Print to file */
    sprintf(Buf, "#define %%-%llds    (%%s)\n", Align-4-8);
    fprintf(File, Buf, Macro, Value);
}
/* End Function:Make_Define_Str **********************************************/

/* Begin Function:Make_Define_Int *********************************************
Description : Make a define statement in the file. The define statement can have
              three parts, which will be converted to uppercase and concatenated
              together.
              The value here is a integer.
Input       : FILE* File - The file structure.
              s8* Macro - The macro.
              ptr_t Value - The value of the macro.
              s8* Align - The alignment, must be bigger than 12.
Output      : None.
Return      : None.
******************************************************************************/
void Make_Define_Int(FILE* File, s8* Macro, ptr_t Value, ptr_t Align)
{
    s8 Buf[32];

    /* Print to file */
    sprintf(Buf, "#define %%-%llds    (%%lld)\n", Align-4-8);
    fprintf(File, Buf, Macro, Value);
}
/* End Function:Make_Define_Int **********************************************/

/* Begin Function:Make_Define_Hex *********************************************
Description : Make a define statement in the file. The define statement can have
              three parts, which will be converted to uppercase and concatenated
              together.
              The value here is a hex number.
Input       : FILE* File - The file structure.
              s8* Macro - The macro.
              ptr_t Value - The value of the macro.
              s8* Align - The alignment, must be bigger than 12.
Output      : None.
Return      : None.
******************************************************************************/
void Make_Define_Hex(FILE* File, s8* Macro, ptr_t Value, ptr_t Align)
{
    s8 Buf[32];

    /* Print to file */
    sprintf(Buf, "#define %%-%llds    (0x%%llx)\n", Align-4-8);
    fprintf(File, Buf, Macro, Value);
}
/* End Function:Make_Define_Hex **********************************************/

/* Begin Function:Lower_Case **************************************************
Description : Copy the string from the source to destination anjd convert that
              to lower case.
Input       : s8* Src - The source string.
Output      : s8* Dst - The destination string.
Return      : None.
******************************************************************************/
void Lower_Case(s8* Dst, s8* Src)
{
    ptr_t Count;

    strcpy(Dst, Src);
    Count=0;
    while(Dst[Count]!='\0')
    {
        Dst[Count]=tolower(Src[Count]);
        Count++;
    }
}
/* End Function:Lower_Case ***************************************************/

/* Begin Function:Gen_RME_Boot ************************************************
Description : Generate the rme_boot.h and rme_boot.c. These file are mainly
              responsible for setting up interrupt endpoints.
Input       : struct Proj_Info* Proj - The project structure.
              struct Chip_Info* Chip - The chip structure.
              s8* RME_Path - The RME root folder path.
              s8* Output_Path - The output folder path.
Output      : None.
Return      : None.
******************************************************************************/
void Gen_RME_Boot(struct Proj_Info* Proj, struct Chip_Info* Chip,
                  struct Cap_Alloc_Info* Alloc, s8* RME_Path, s8* Output_Path)
{
    /* Create the file and the file header */
    s8* Buf;
    FILE* Boot;
    time_t Time;
    struct tm* Time_Struct;
    ptr_t Vect_Cnt;
    struct Vect_Info* Vect;
    ptr_t Count;
    ptr_t Cap_Front;
    ptr_t Capacity;
    s8 Lower_Plat[16];

    /* Convert platform name to lower case */
    Lower_Case(Lower_Plat, Proj->Platform);

    Buf=Malloc(4096);
    if(Buf==0)
        EXIT_FAIL("Buffer allocation failed.");

    Cap_Front=Alloc->Cap_Vect_Front;
    Capacity=POW2((Alloc->Processor_Bits/4)-1);

    /* Generate rme_boot.h */
    sprintf(Buf, "%s/M7M1_MuEukaron/Project/Include/rme_boot.h", Output_Path);
    Boot=fopen(Buf, "wb");
    if(Boot==0)
        EXIT_FAIL("rme_boot.h open failed.");
    time(&Time);
    Time_Struct=localtime(&Time);
    sprintf(Buf,"%02d/%02d/%d",Time_Struct->tm_mday,Time_Struct->tm_mon+1,Time_Struct->tm_year+1900);
    Write_Src_Desc(Boot, "rme_boot.h", CODE_AUTHOR, Buf, 
                   "LGPL v3+; see COPYING for details.", "The boot-time initialization file header.");
    fprintf(Boot, "/* Defines *******************************************************************/\n");
    fprintf(Boot, "/* Vector endpoint capability tables */\n");
    /* Vector capability table */
    for(Count=0;Count<Proj->RVM.Vector_Captbl_Front;Count+=Capacity)
    {
        sprintf(Buf, "RME_BOOT_CTVECT%lld",Count/Capacity);
        Make_Define_Int(Boot, Buf, Cap_Front++, MACRO_ALIGNMENT);
    }
    /* Vector endpoints */
    fprintf(Boot, "/* Vector endpoints */\n");
    for(Vect_Cnt=0;Vect_Cnt<Proj->RVM.Vector_Captbl_Front;Vect_Cnt++)
    {
        Vect=(struct Vect_Info*)Proj->RVM.Vector_Captbl[Vect_Cnt].Cap;
        sprintf(Buf, "RME_CAPID_2L(RME_BOOT_CTVECT%lld,%lld)", Vect_Cnt/Capacity, Vect_Cnt%Capacity);
        Make_Define_Str(Boot, Vect->RME_Capid_Macro, Buf, MACRO_ALIGNMENT);
    }
    fprintf(Boot, "/* End Defines ***************************************************************/\n\n");
    Write_Src_Footer(Boot);
    fclose(Boot);

    /* Generate rme_boot.c */
    sprintf(Buf, "%s/M7M1_MuEukaron/Project/Source/rme_boot.c", Output_Path);
    Boot=fopen(Buf, "wb");
    if(Boot==0)
        EXIT_FAIL("rme_boot.c open failed.");
    time(&Time);
    Time_Struct=localtime(&Time);
    sprintf(Buf,"%02d/%02d/%d",Time_Struct->tm_mday,Time_Struct->tm_mon+1,Time_Struct->tm_year+1900);
    Write_Src_Desc(Boot, "rme_boot.c", CODE_AUTHOR, Buf, 
                   "LGPL v3+; see COPYING for details.", "The boot-time initialization file.");
    /* Print all header includes */
    fprintf(Boot, "/* Includes ******************************************************************/\n");
    fprintf(Boot, "#define __HDR_DEFS__\n");
    fprintf(Boot, "#include \"Kernel/rme_kernel.h\"\n");
    fprintf(Boot, "#include \"Platform/%s/rme_platform_%s.h\"\n", Proj->Platform, Lower_Plat);
    fprintf(Boot, "#undef __HDR_DEFS__\n\n");
    fprintf(Boot, "#define __HDR_STRUCTS__\n");
    fprintf(Boot, "#include \"Kernel/rme_kernel.h\"\n");
    fprintf(Boot, "#include \"Platform/%s/rme_platform_%s.h\"\n", Proj->Platform, Lower_Plat);
    fprintf(Boot, "#undef __HDR_STRUCTS__\n\n");
    fprintf(Boot, "#define __HDR_PUBLIC_MEMBERS__\n");
    fprintf(Boot, "#include \"Kernel/rme_kernel.h\"\n");
    fprintf(Boot, "#include \"Platform/%s/rme_platform_%s.h\"\n", Proj->Platform, Lower_Plat);
    fprintf(Boot, "#undef __HDR_PUBLIC_MEMBERS__\n\n");
    fprintf(Boot, "#include \"rme_boot.h\"\n");
    fprintf(Boot, "/* End Includes **************************************************************/\n\n");
    /* Print all global variables and prototypes */
    fprintf(Boot, "/* Private Global Variables **************************************************/\n");
    for(Count=0;Count<Proj->RVM.Vector_Captbl_Front;Count++)
    {
        Vect=(struct Vect_Info*)Proj->RVM.Vector_Captbl[Count].Cap;
        fprintf(Boot, "static struct RME_Sig_Struct* %s_Vect_Sig;\n", Vect->Name);
    }
    fprintf(Boot, "/* End Private Global Variables **********************************************/\n\n");
    fprintf(Boot, "/* Private C Function Prototypes *********************************************/\n");
    for(Count=0;Count<Proj->RVM.Vector_Captbl_Front;Count++)
    {
        Vect=(struct Vect_Info*)Proj->RVM.Vector_Captbl[Count].Cap;
        fprintf(Boot, "static rme_ptr_t RME_Vect_%s_User(rme_ptr_t Int_Num);\n", Vect->Name);
    }
    fprintf(Boot, "/* End Private C Function Prototypes *****************************************/\n\n");
    fprintf(Boot, "/* Public C Function Prototypes **********************************************/\n");
    fprintf(Boot, "void RME_Boot_Vect_Init(struct RME_Cap_Captbl* Captbl, rme_ptr_t Cap_Front, rme_ptr_t Kmem_Front);\n");
    fprintf(Boot, "rme_ptr_t RME_Boot_Int_Handler(rme_ptr_t Int_Num);\n");
    fprintf(Boot, "/* End Public C Function Prototypes ******************************************/\n\n");
    /* Boot-time setup routine for the interrupt endpoints */
    Write_Func_Desc(Boot, "RME_Boot_Vect_Init");
    fprintf(Boot, "Description : Initialize all the vector endpoints at boot-time.\n");
    fprintf(Boot, "Input       : rme_ptr_t Cap_Front - The current capability table frontier.\n");
    fprintf(Boot, "Input       : rme_ptr_t Kmem_Front - The current kernel absolute memory frontier.\n");
    fprintf(Boot, "Output      : None.\n");
    fprintf(Boot, "Return      : None.\n");
    fprintf(Boot, "******************************************************************************/\n");
    fprintf(Boot, "void RME_Boot_Vect_Init(struct RME_Cap_Captbl* Captbl, rme_ptr_t Cap_Front, rme_ptr_t Kmem_Front)\n");
    fprintf(Boot, "{\n");
    fprintf(Boot, "    rme_ptr_t Cur_Addr;\n\n");
    fprintf(Boot, "    /* The address here shall match what is in the generator */\n");
    fprintf(Boot, "    RME_ASSERT(Cap_Front==%lld);\n", Alloc->Cap_Vect_Front);
    fprintf(Boot, "    RME_ASSERT(Kmem_Front==0x%llx);\n\n", Alloc->Kmem_Vect_Front+Alloc->Kmem_Abs_Base);
    fprintf(Boot, "    Cur_Addr=Kmem_Front;\n");
    fprintf(Boot, "    /* Create all the vector capability tables first */\n");
    for(Count=0;Count<Proj->RVM.Vector_Captbl_Front;Count+=Capacity)
    {
        fprintf(Boot, "    RME_ASSERT(_RME_Captbl_Boot_Crt(Captbl, RME_BOOT_CAPTBL, RME_BOOT_CTVECT%lld, Cur_Addr, %lld))==0);\n", 
                Count/Capacity,(Proj->RVM.Vector_Captbl_Front>(Count+1)*Capacity)?(Capacity):(Proj->RVM.Vector_Captbl_Front%Capacity));
        fprintf(Boot, "    Cur_Addr+=RME_KOTBL_ROUND(RME_CAPTBL_SIZE(%lld));\n",
                (Proj->RVM.Vector_Captbl_Front>(Count+1)*Capacity)?(Capacity):(Proj->RVM.Vector_Captbl_Front%Capacity));
    }
    for(Count=0;Count<Proj->RVM.Vector_Captbl_Front;Count++)
    {
        Vect=(struct Vect_Info*)Proj->RVM.Vector_Captbl[Count].Cap;
        fprintf(Boot, "    %s_Vect_Sig=(struct RME_Sig_Struct*)Cur_Addr;\n", Vect->Name);
        fprintf(Boot, "    RME_ASSERT(_RME_Sig_Boot_Crt(Captbl, RME_BOOT_CTVECT%lld, %s, Cur_Addr)==0);\n", Count/Capacity, Vect->RME_Capid_Macro);
        fprintf(Boot, "    Cur_Addr+=RME_KOTBL_ROUND(RME_SIG_SIZE);\n");
    }
    fprintf(Boot, "}\n");
    Write_Func_Footer(Boot, "RME_Boot_Int_Init");
    /* Print the interrupt relaying function */
    Write_Func_Desc(Boot, "RME_Boot_Int_Handler");
    fprintf(Boot, "Description : The interrupt handler entry for all the vectors.\n");
    fprintf(Boot, "Input       : rme_ptr_t Int_Num - The interrupt number.\n");
    fprintf(Boot, "Output      : None.\n");
    fprintf(Boot, "Return      : rme_ptr_t - The number of signals to send to the generic vector endpoint.\n");
    fprintf(Boot, "******************************************************************************/\n");
    fprintf(Boot, "rme_ptr_t RME_Boot_Int_Handler(rme_ptr_t Int_Num)\n");
    fprintf(Boot, "{\n");
    fprintf(Boot, "    rme_ptr_t Send_Num;\n\n");
    fprintf(Boot, "    switch(Int_Num)\n");
    fprintf(Boot, "    {\n");
    for(Count=0;Count<Proj->RVM.Vector_Captbl_Front;Count++)
    {
        Vect=(struct Vect_Info*)Proj->RVM.Vector_Captbl[Count].Cap;
        fprintf(Boot, "        /* %s */\n", Vect->Name);
        fprintf(Boot, "        case %lld:\n", Vect->Number);
        fprintf(Boot, "        {\n");
        fprintf(Boot, "            Send_Num=RME_Vect_%s_User(Int_Num);\n", Vect->Name);
        fprintf(Boot, "            RME_Kern_Send(%s_Vect_Sig);\n", Vect->Name);
        fprintf(Boot, "            return Send_Num;\n");
        fprintf(Boot, "        }\n");
    }
    fprintf(Boot, "        default: break;\n");
    fprintf(Boot, "    }\n");
    fprintf(Boot, "    return 1;\n");
    fprintf(Boot, "}\n");
    Write_Func_Footer(Boot, "RME_Boot_Int_Handler");
    /* The rest are interrupt endpoint user preprocessing functions */
    for(Count=0;Count<Proj->RVM.Vector_Captbl_Front;Count++)
    {
        Vect=(struct Vect_Info*)Proj->RVM.Vector_Captbl[Count].Cap;
        sprintf(Buf, "RME_Vect_%s_User", Vect->Name);
        Write_Func_Desc(Boot, Buf);
        fprintf(Boot, "Description : The user top-half interrupt handler for %s.\n", Vect->Name);
        fprintf(Boot, "Input       : rme_ptr_t Int_Num - The interrupt number.\n");
        fprintf(Boot, "Output      : None.\n");
        fprintf(Boot, "Return      : rme_ptr_t - The number of signals to send to the generic vector endpoint.\n");
        fprintf(Boot, "******************************************************************************/\n");
        fprintf(Boot, "rme_ptr_t RME_Vect_%s_User(rme_ptr_t Int_Num)\n", Vect->Name);
        fprintf(Boot, "{\n");
        fprintf(Boot, "    /* Add code here */\n\n");
        fprintf(Boot, "    return 0;\n");
        fprintf(Boot, "}\n");
        Write_Func_Footer(Boot, Buf);
    }
    /* Close the file */
    Write_Src_Footer(Boot);
    fclose(Boot);
}
/* End Function:Gen_RME_Boot *************************************************/

/* Begin Function:Gen_RME_User ************************************************
Description : Generate the rme_user.c. This file is mainly responsible for user-
              supplied hooks. If the user needs to add functionality, consider
              modifying this file.
Input       : struct Proj_Info* Proj - The project structure.
              struct Chip_Info* Chip - The chip structure.
              struct A7M_Info* A7M - The platform-specific project structure.
              s8* RME_Path - The RME root folder path.
              s8* Output_Path - The output folder path.
Output      : None.
Return      : None.
******************************************************************************/
void Gen_RME_User(struct Proj_Info* Proj, struct Chip_Info* Chip, s8* RME_Path, s8* Output_Path)
{
    /* Create user stubs */
}
/* End Function:Gen_RME_User *************************************************/

/* Begin Function:Gen_RVM_Boot ************************************************
Description : Generate the rvm_boot.h and rvm_boot.c. They are mainly responsible
              for setting up all the kernel objects. If RVM or Posix functionality
              is enabled, these will also be handled by such file.
Input       : struct Proj_Info* Proj - The project structure.
              struct Chip_Info* Chip - The chip structure.
              s8* RVM_Path - The RVM root folder path.
              s8* Output_Path - The output folder path.
Output      : None.
Return      : None.
******************************************************************************/
void Gen_RVM_Boot(struct Proj_Info* Proj, struct Chip_Info* Chip, s8* RVM_Path, s8* Output_Path)
{
    /* Create all the capability table address */

    /* Create all the page tables first *//*
    Gen_Pgtbl_Header(&Captbl_Front);
    Gen_Pgtbl_Setup(&Captbl_Front, &Kmem_Front);*/

    /* Then capability tables for all processes */

    /* Then all processes */

    /* All threads */

    /* All invocations */

    /* All ports */

    /* All receive endpoints */

    /* All send endpoints */

    /* All vector endpoints */
}
/* End Function:Gen_RVM_Boot *************************************************/

/* Begin Function:Gen_RVM_User ************************************************
Description : Generate the rvm_user.c. This file is mainly responsible for user-
              supplied hooks. If the user needs to add functionality, consider
              modifying this file.
Input       : struct Proj_Info* Proj - The project structure.
              struct Chip_Info* Chip - The chip structure.
              s8* RVM_Path - The RVM root folder path.
              s8* Output_Path - The output folder path.
Output      : None.
Return      : None.
******************************************************************************/
void Gen_RVM_User(struct Proj_Info* Proj, struct Chip_Info* Chip, s8* RVM_Path, s8* Output_Path)
{
    /* User stubs */
}
/* End Function:Gen_RVM_User *************************************************/

/* Begin Function:Setup_Folder ************************************************
Description : Setup the generic folder contents.
Input       : struct Proj_Info* Proj - The project structure.
              struct Chip_Info* Chip - The chip structure.
              s8* RME_Path - The RME root folder path.
              s8* RVM_Path - The RVM root folder path.
              s8* Output_Path - The output folder path.
Output      : None.
Return      : None.
******************************************************************************/
void Setup_Folder(struct Proj_Info* Proj, struct Chip_Info* Chip, 
                  s8* RME_Path, s8* RVM_Path, s8* Output_Path)
{
    s8* Buf1;
    s8* Buf2;
    s8 Lower_Plat[16];

    /* Allocate the buffer */
    Buf1=Malloc(sizeof(s8)*4096);
    if(Buf1==0)
        EXIT_FAIL("Buffer allocation failed");
    Buf2=Malloc(sizeof(s8)*4096);
    if(Buf2==0)
        EXIT_FAIL("Buffer allocation failed");

    Lower_Case(Lower_Plat, Proj->Platform);

    /* RME directory */
    sprintf(Buf1,"%s/M7M1_MuEukaron",Output_Path);
    if(Make_Dir(Buf1)!=0)
        EXIT_FAIL("RME folder creation failed.");
    sprintf(Buf1,"%s/M7M1_MuEukaron/Documents",Output_Path);
    if(Make_Dir(Buf1)!=0)
        EXIT_FAIL("RME folder creation failed.");
    sprintf(Buf1,"%s/M7M1_MuEukaron/MEukaron",Output_Path);
    if(Make_Dir(Buf1)!=0)
        EXIT_FAIL("RME folder creation failed.");
    sprintf(Buf1,"%s/M7M1_MuEukaron/MEukaron/Include",Output_Path);
    if(Make_Dir(Buf1)!=0)
        EXIT_FAIL("RME folder creation failed.");
    sprintf(Buf1,"%s/M7M1_MuEukaron/MEukaron/Include/Kernel",Output_Path);
    if(Make_Dir(Buf1)!=0)
        EXIT_FAIL("RME folder creation failed.");
    sprintf(Buf1,"%s/M7M1_MuEukaron/MEukaron/Include/Platform",Output_Path);
    if(Make_Dir(Buf1)!=0)
        EXIT_FAIL("RME folder creation failed.");
    sprintf(Buf1,"%s/M7M1_MuEukaron/MEukaron/Include/Platform/%s",Output_Path,Proj->Platform);
    if(Make_Dir(Buf1)!=0)
        EXIT_FAIL("RME folder creation failed.");
    sprintf(Buf1,"%s/M7M1_MuEukaron/MEukaron/Include/Platform/%s/Chips",Output_Path,Proj->Platform);
    if(Make_Dir(Buf1)!=0)
        EXIT_FAIL("RME folder creation failed.");
    sprintf(Buf1,"%s/M7M1_MuEukaron/MEukaron/Include/Platform/%s/Chips/%s",Output_Path,Proj->Platform,Chip->Name);
    if(Make_Dir(Buf1)!=0)
        EXIT_FAIL("RME folder creation failed.");
    sprintf(Buf1,"%s/M7M1_MuEukaron/MEukaron/Kernel",Output_Path);
    if(Make_Dir(Buf1)!=0)
        EXIT_FAIL("RME folder creation failed.");
    sprintf(Buf1,"%s/M7M1_MuEukaron/MEukaron/Platform",Output_Path);
    if(Make_Dir(Buf1)!=0)
        EXIT_FAIL("RME folder creation failed.");
    sprintf(Buf1,"%s/M7M1_MuEukaron/MEukaron/Platform/%s",Output_Path,Proj->Platform);
    if(Make_Dir(Buf1)!=0)
        EXIT_FAIL("RME folder creation failed.");
    sprintf(Buf1,"%s/M7M1_MuEukaron/Project",Output_Path);
    if(Make_Dir(Buf1)!=0)
        EXIT_FAIL("RME folder creation failed.");
    sprintf(Buf1,"%s/M7M1_MuEukaron/Project/Source",Output_Path);
    if(Make_Dir(Buf1)!=0)
        EXIT_FAIL("RME folder creation failed.");
    sprintf(Buf1,"%s/M7M1_MuEukaron/Project/Include",Output_Path);
    if(Make_Dir(Buf1)!=0)
        EXIT_FAIL("RME folder creation failed.");

    /* Copy kernel file, kernel header, platform file, platform header, and chip headers */
    sprintf(Buf1,"%s/M7M1_MuEukaron/Documents/EN_M7M1_Microkernel-RTOS-User-Manual.pdf",Output_Path);
    sprintf(Buf2,"%s/Documents/EN_M7M1_Microkernel-RTOS-User-Manual.pdf",RME_Path);
    if(Copy_File(Buf1, Buf2)!=0)
        EXIT_FAIL("File copying failed.");
    sprintf(Buf1,"%s/M7M1_MuEukaron/Documents/CN_M7M1_Microkernel-RTOS-User-Manual.pdf",Output_Path);
    sprintf(Buf2,"%s/Documents/CN_M7M1_Microkernel-RTOS-User-Manual.pdf",RME_Path);
    if(Copy_File(Buf1, Buf2)!=0)
        EXIT_FAIL("File copying failed.");
    sprintf(Buf1,"%s/M7M1_MuEukaron/MEukaron/Kernel/rme_kernel.c",Output_Path);
    sprintf(Buf2,"%s/MEukaron/Kernel/rme_kernel.c",RME_Path);
    if(Copy_File(Buf1, Buf2)!=0)
        EXIT_FAIL("File copying failed.");
    /* The toolchain specific one will be created when we are playing with toolchains */
    sprintf(Buf1,"%s/M7M1_MuEukaron/MEukaron/Platform/%s/rme_platform_%s.c",Output_Path,Proj->Platform,Lower_Plat);
    sprintf(Buf2,"%s/MEukaron/Platform/%s/rme_platform_%s.c",RME_Path,Proj->Platform,Lower_Plat);
    if(Copy_File(Buf1, Buf2)!=0)
        EXIT_FAIL("File copying failed.");
    sprintf(Buf1,"%s/M7M1_MuEukaron/MEukaron/Include/rme.h",Output_Path);
    sprintf(Buf2,"%s/MEukaron/Include/rme.h",RME_Path);
    if(Copy_File(Buf1, Buf2)!=0)
        EXIT_FAIL("File copying failed.");
    sprintf(Buf1,"%s/M7M1_MuEukaron/MEukaron/Include/Kernel/rme_kernel.h",Output_Path);
    sprintf(Buf2,"%s/MEukaron/Include/Kernel/rme_kernel.h",RME_Path);
    if(Copy_File(Buf1, Buf2)!=0)
        EXIT_FAIL("File copying failed.");
    sprintf(Buf1,"%s/M7M1_MuEukaron/MEukaron/Include/Platform/%s/rme_platform_%s.h",Output_Path,Proj->Platform,Lower_Plat);
    sprintf(Buf2,"%s/MEukaron/Include/Platform/%s/rme_platform_%s.h",RME_Path,Proj->Platform,Lower_Plat);
    if(Copy_File(Buf1, Buf2)!=0)
        EXIT_FAIL("File copying failed.");

    /* Crank the selector headers */

    /* RVM directory */

    /* All other process directories */

    Free(Buf1);
    Free(Buf2);
}
/* End Function:Setup_Folder *************************************************/

/* A7M Toolset ***************************************************************/
ret_t A7M_Align(struct Mem_Info* Mem);
void A7M_Gen_Proj(struct Proj_Info* Proj, struct Chip_Info* Chip,
                  s8* RME_Path, s8* RVM_Path, s8* Output_Path, s8* Format);

/* Begin Function:main ********************************************************
Description : The entry of the tool.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
int main(int argc, char* argv[])
{
	/* The command line arguments */
	s8* Input_Path;
	s8* Output_Path;
	s8* RME_Path;
	s8* RVM_Path;
	s8* Format;
	/* The input buffer */
	s8* Input_Buf;
    /* The path synthesis buffer */
    s8* Path_Buf;
	/* The file handle */
	/* The project and chip pointers */
	struct Proj_Info* Proj;
	struct Chip_Info* Chip;

	/* Initialize memory pool */
	List_Crt(&Mem_List);

    /* Process the command line first */
    Cmdline_Proc(argc,argv, &Input_Path, &Output_Path, &RME_Path, &RVM_Path, &Format);

	/* Read the project contents */
	Input_Buf=Read_File(Input_Path);
	Proj=Parse_Project(Input_Buf);
	Free(Input_Buf);

	/* Parse the chip in a platform-agnostic way - we need to know where the chip file is. Now, we just give a fixed path */
    Path_Buf=Malloc(4096);
    if(Path_Buf==0)
        EXIT_FAIL("Platform path synthesis buffer allocation failed.");
    sprintf(Path_Buf, "%s/MEukaron/Include/Platform/%s/Chips/%s/rme_platform_%s.xml", RME_Path, Proj->Platform, Proj->Chip, Proj->Chip);
	Input_Buf=Read_File(Path_Buf);
	Chip=Parse_Chip(Input_Buf);
	Free(Input_Buf);
    Free(Path_Buf);

    /* Check if the platform is the same */
    if(strcmp(Proj->Platform, Chip->Platform)!=0)
        EXIT_FAIL("The chip description file platform conflicted with the project file.");

	/* Align memory to what it should be */
	if(strcmp(Proj->Platform,"A7M")==0)
	    Align_Mem(Proj, A7M_Align);
    else
		EXIT_FAIL("Other platforms not currently supported.");

	/* Actually allocate the auto memory segments by fixing their start addresses.
     * We don't deal with device memory at all; this is mapped in anyway at user request. */
	Alloc_Mem(Proj, Chip, MEM_CODE);
	Alloc_Mem(Proj, Chip, MEM_DATA);
    /* Check to see if the resulting memory map is actually valid */
    Check_Mem(Proj, Chip);

    /* Actually allocate the capability IDs of these kernel objects. Both local and global ID must be allocated. */
    Alloc_Captbl(Proj, Chip);

    /* Set the folder up */
    Setup_Folder(Proj, Chip, RME_Path, RVM_Path, Output_Path);
	/* Generate the project-specific files */
	if(strcmp(Proj->Platform,"A7M")==0)
		A7M_Gen_Proj(Proj, Chip, RME_Path, RVM_Path, Output_Path, Format);

    /* Create rme_boot.c */
    struct Cap_Alloc_Info Alloc;
    memset(&Alloc,0,sizeof(struct Cap_Alloc_Info));
    Alloc.Processor_Bits=32;
    Gen_RME_Boot(Proj, Chip, &Alloc, RME_Path, Output_Path);
    /* Create rme_user.c */
    //Gen_RME_User(Proj, Chip, RME_Path, Output_Path);
    /* Generate rvm_boot.c */
    //Gen_RVM_Boot(Proj, Chip, RVM_Path, Output_Path);
    /* Create rvm_user.c */
    //Gen_RVM_User(Proj, Chip, RVM_Path, Output_Path);

	/* All done, free all memory and we quit */
	Free_All();
    return 0;
}
/* End Function:main *********************************************************/

/* Cortex-M (A7M) Toolset *****************************************************
This toolset is for ARMv7-M. Specifically, this suits Cortex-M0+, Cortex-M1,
Cortex-M3, Cortex-M7.
******************************************************************************/

/* Defines *******************************************************************/
/* NVIC grouping */
#define A7M_NVIC_P0S8           (7)    
#define A7M_NVIC_P1S7           (6)
#define A7M_NVIC_P2S6           (5)
#define A7M_NVIC_P3S5           (4)
#define A7M_NVIC_P4S4           (3)
#define A7M_NVIC_P5S3           (2)
#define A7M_NVIC_P6S2           (1)
#define A7M_NVIC_P7S1           (0)
/* CPU type */
#define A7M_CPU_CM0P            (0)
#define A7M_CPU_CM3             (1)
#define A7M_CPU_CM4             (2)
#define A7M_CPU_CM7             (3)
/* FPU type */
#define A7M_FPU_NONE            (0)
#define A7M_FPU_FPV4            (1)
#define A7M_FPU_FPV5_SP         (2)
#define A7M_FPU_FPV5_DP         (3)
/* Endianness */
#define A7M_END_LITTLE          (0)
#define A7M_END_BIG             (1)

/* Page table */
#define A7M_PGT_NOTHING         (0)
#define A7M_PGT_MAPPED          ((struct A7M_Pgtbl*)(-1))

/* RVM page table capability table size */
#define A7M_PGTBL_CAPTBL_SIZE   (4096)
/* The process capability table size limit */
#define A7M_PROC_CAPTBL_LIMIT   (128)
/* The A7M boot-time capability table starting slot */
#define A7M_CAPTBL_START        (8)
/* End Defines ***************************************************************/

/* Structs *******************************************************************/
struct A7M_Pgtbl
{
    /* The start address of the page table */
    ptr_t Start_Addr;
    /* The size order */
    ptr_t Size_Order;
    /* The number order */
    ptr_t Num_Order;
    /* The attribute */
    ptr_t Attr;
    /* The global linear capability ID */
    ptr_t RVM_Capid;
    /* The macro corresponding to the global capid */
    s8* RVM_Capid_Macro;
    /* Whether we have the 8 subregions mapped: 0 - not mapped 1 - mapped other - pointer to the next */
    struct A7M_Pgtbl* Mapping[8];
};

struct A7M_Info
{
    /* The NVIC grouping */
	ptr_t NVIC_Grouping;
    /* The systick value */
	ptr_t Systick_Val;
    /* The CPU type */
    ptr_t CPU_Type;
    /* The FPU type */
    ptr_t FPU_Type;
    /* Endianness - big or little */
    ptr_t Endianness;
    /* The page tables for all processes */
    struct A7M_Pgtbl** Pgtbl;
    /* Global captbl containing pgtbls */
    ptr_t Pgtbl_Captbl_Front;
    struct RVM_Cap_Info* Pgtbl_Captbl;
};
/* End Structs ***************************************************************/

/* C Function Prototypes *****************************************************/
struct A7M_Pgtbl* A7M_Gen_Pgtbl(struct Proc_Info* Proc, struct A7M_Info* A7M,
                                struct Mem_Info* Mem, ptr_t Num, ptr_t Total_Max);
/* End C Function Prototypes *************************************************/

/* Begin Function:A7M_Align ***************************************************
Description : Align the memory according to Cortex-M platform's requirements.
Input       : struct Mem_Info* Mem - The struct containing memory information.
Output      : struct Mem_Info* Mem - The struct containing memory information,
                                     with all memory size aligned.
Return      : ret_t - If the start address and size is acceptable, 0; else -1.
******************************************************************************/
ret_t A7M_Align(struct Mem_Info* Mem)
{
    ptr_t Temp;
    if(Mem->Start!=AUTO)
    {
        /* This memory already have a fixed start address. Can we map it in? */
        if((Mem->Start&0x1F)!=0)
            return -1;
        if((Mem->Size&0x1F)!=0)
            return -1;
        /* This is terrible. Or even horrible, this mapping algorithm is really hard */
    }
    else
    {
        /* This memory's start address is not designated yet. Decide its size after
         * alignment and calculate its start address alignment granularity. 
         * For Cortex-M, the roundup minimum granularity is 1/8 of the nearest power 
         * of 2 for the size. */
        Temp=1;
        while(Temp<Mem->Size)
            Temp<<=1;
        Mem->Align=Temp/8;
        Mem->Size=((Mem->Size-1)/Mem->Align+1)*Mem->Align;
    }
    
	return 0;
}
/* End Function:A7M_Align ****************************************************/

/* Begin Function:A7M_Parse_Options *******************************************
Description : Parse the options that are specific to ARMv7-M.
Input       : struct Proj_Info* Proj - The project structure.
              struct Chip_Info* Chip - The chip structure.
              struct A7M_Info* A7M - The platform sprcific project structure.
Output      : None.
Return      : None.
******************************************************************************/
void A7M_Parse_Options(struct Proj_Info* Proj, struct Chip_Info* Chip, struct A7M_Info* A7M)
{
    s8* Temp;

    /* What is the NVIC grouping that we use? */
    Temp=Raw_Match(&(Proj->RME.Plat_Raw), "NVIC_Grouping");
    if(Temp==0)
        EXIT_FAIL("Missing NVIC grouping settings.");
    if(strcmp(Temp,"0-8")==0)
        A7M->NVIC_Grouping=A7M_NVIC_P0S8;
    else if(strcmp(Temp,"1-7")==0)
        A7M->NVIC_Grouping=A7M_NVIC_P1S7;
    else if(strcmp(Temp,"2-6")==0)
        A7M->NVIC_Grouping=A7M_NVIC_P2S6;
    else if(strcmp(Temp,"3-5")==0)
        A7M->NVIC_Grouping=A7M_NVIC_P3S5;
    else if(strcmp(Temp,"4-4")==0)
        A7M->NVIC_Grouping=A7M_NVIC_P4S4;
    else if(strcmp(Temp,"5-3")==0)
        A7M->NVIC_Grouping=A7M_NVIC_P5S3;
    else if(strcmp(Temp,"6-2")==0)
        A7M->NVIC_Grouping=A7M_NVIC_P6S2;
    else if(strcmp(Temp,"7-1")==0)
        A7M->NVIC_Grouping=A7M_NVIC_P7S1;
    else
        EXIT_FAIL("NVIC grouping value is invalid.");

    /* What is the systick value? */
    Temp=Raw_Match(&(Proj->RME.Plat_Raw), "Systick_Value");
    if(Temp==0)
        EXIT_FAIL("Missing systick value settings.");
    A7M->Systick_Val=Get_Uint(Temp, &Temp[strlen(Temp)-1]);
    if(A7M->Systick_Val>=INVALID)
        EXIT_FAIL("Wrong systick value entered.");

    /* What is the CPU type and the FPU type? */
    Temp=Raw_Match(&(Chip->Attr_Raw), "CPU_Type");
    if(Temp==0)
        EXIT_FAIL("Missing CPU type settings.");
    if(strcmp(Temp,"Cortex-M0+")==0)
        A7M->CPU_Type=A7M_CPU_CM0P;
    else if(strcmp(Temp,"Cortex-M3")==0)
        A7M->CPU_Type=A7M_CPU_CM3;
    else if(strcmp(Temp,"Cortex-M4")==0)
        A7M->CPU_Type=A7M_CPU_CM4;
    else if(strcmp(Temp,"Cortex-M7")==0)
        A7M->CPU_Type=A7M_CPU_CM7;
    else
        EXIT_FAIL("CPU type value is invalid.");
    
    Temp=Raw_Match(&(Chip->Attr_Raw), "FPU_Type");
    if(Temp==0)
        EXIT_FAIL("Missing FPU type settings.");
    if(strcmp(Temp,"None")==0)
        A7M->FPU_Type=A7M_FPU_NONE;
    else if(strcmp(Temp,"Single")==0)
    {
        if(A7M->CPU_Type==A7M_CPU_CM4)
            A7M->FPU_Type=A7M_FPU_FPV4;
        else if(A7M->CPU_Type==A7M_CPU_CM7)
            A7M->FPU_Type=A7M_FPU_FPV5_SP;
        else
            EXIT_FAIL("FPU type and CPU type mismatch.");
    }
    else if(strcmp(Temp,"Double")==0)
    {
        if(A7M->CPU_Type==A7M_CPU_CM7)
            A7M->FPU_Type=A7M_FPU_FPV5_DP;
        else
            EXIT_FAIL("FPU type and CPU type mismatch.");

    }
    else
        EXIT_FAIL("FPU type value is invalid.");

    /* What is the endianness? */
    Temp=Raw_Match(&(Chip->Attr_Raw), "Endianness");
    if(Temp==0)
        EXIT_FAIL("Missing endianness settings.");
    if(strcmp(Temp,"Little")==0)
        A7M->Endianness=A7M_END_LITTLE;
    else if(strcmp(Temp,"Big")==0)
        A7M->Endianness=A7M_END_BIG;
    else
        EXIT_FAIL("Endianness value is invalid.");
}
/* End Function:A7M_Parse_Options ********************************************/

/* Begin Function:A7M_Total_Order *********************************************
Description : Get the total order and the start address of the page table. 
Input       : struct Mem_Info* Mem - The memory block list.
              ptr_t Num - The number of memory blocks in the list.
Output      : ptr_t* Start_Addr - The start address of this page table.
Return      : ptr_t - The total order of the page table.
******************************************************************************/
ptr_t A7M_Total_Order(struct Mem_Info* Mem, ptr_t Num, ptr_t* Start_Addr)
{
    /* Start is inclusive, end is exclusive */
    ptr_t Start;
    ptr_t End;
    ptr_t Total_Order;
    ptr_t Mem_Cnt;

    /* What ranges does these stuff cover? */
    Start=(ptr_t)(-1);
    End=0;
    for(Mem_Cnt=0;Mem_Cnt<Num;Mem_Cnt++)
    {
        if(Start>Mem[Mem_Cnt].Start)
            Start=Mem[Mem_Cnt].Start;
        if(End<(Mem[Mem_Cnt].Start+(Mem[Mem_Cnt].Size-1)))
            End=Mem[Mem_Cnt].Start+(Mem[Mem_Cnt].Size-1);
    }
    
    /* Which power-of-2 box is this in? - do not shift more thyan 32 or you get undefined behavior */
    Total_Order=0;
    while(1)
    {  
        /* No bigger than 32 is ever possible */
        if(Total_Order>=32)
            break;
        if(End<=(ALIGN_POW(Start, Total_Order)+(POW2(Total_Order)-1)))
            break;
        Total_Order++;
    }
    /* If the total order less than 8, we wish to extend that to 8, because if we are smaller than this it makes no sense */
    if(Total_Order<8)
        Total_Order=8;

    /* Do not shift more than 32 or we get undefined behavior */
    if(Total_Order==32)
        *Start_Addr=0;
    else
        *Start_Addr=ALIGN_POW(Start, Total_Order);

    return Total_Order;
}
/* End Function:A7M_Total_Order **********************************************/

/* Begin Function:A7M_Num_Order ***********************************************
Description : Get the number order of the page table. 
Input       : struct Mem_Info* Mem - The memory block list.
              ptr_t Num - The number of memory blocks in the list.
              ptr_t Total_Order - The total order of the page table.
              ptr_t Start_Addr - The start address of the page table.
Output      : None.
Return      : ptr_t - The number order of the page table.
******************************************************************************/
ptr_t A7M_Num_Order(struct Mem_Info* Mem, ptr_t Num, ptr_t Total_Order, ptr_t Start_Addr)
{
    ptr_t Mem_Cnt;
    ptr_t Num_Order;
    ptr_t Pivot_Cnt;
    ptr_t Pivot_Addr;
    ptr_t Cut_Apart;

    /* Can the memory segments get fully mapped in? If yes, there are two conditions
     * that must be met:
     * 1. There cannot be different access permissions in these memory segments.
     * 2. The memory start address and the size must be fully divisible by POW2(Total_Order-3). */
    for(Mem_Cnt=0;Mem_Cnt<Num;Mem_Cnt++)
    {
        if(Mem[Mem_Cnt].Attr!=Mem[0].Attr)
            break;
        if((Mem[Mem_Cnt].Start%POW2(Total_Order-3))!=0)
            break;
        if((Mem[Mem_Cnt].Size%POW2(Total_Order-3))!=0)
            break;
    }

    /* Is this directly mappable? If yes, we always create page tables with 8 pages. */
    if(Mem_Cnt==Num)
    {
        /* Yes, it is directly mappable. We choose the smallest number order, in this way
         * we have the largest size order. This will leave us plenty of chances to use huge
         * pages, as this facilitates delegation as well. Number order = 0 is also possible,
         * as this maps in a single huge page. */
        for(Num_Order=0;Num_Order<=3;Num_Order++)
        {
            for(Mem_Cnt=0;Mem_Cnt<Num;Mem_Cnt++)
            {
                if((Mem[Mem_Cnt].Start%POW2(Total_Order-Num_Order))!=0)
                    break;
                if((Mem[Mem_Cnt].Size%POW2(Total_Order-Num_Order))!=0)
                    break;
            }
            if(Mem_Cnt==Num)
                break;
        }

        if(Num_Order>3)
            EXIT_FAIL("Internal number order miscalculation.");
    }
    else
    {
        /* Not directly mappable. What's the maximum number order that do not cut things apart? */
        Cut_Apart=0;
        for(Num_Order=1;Num_Order<=3;Num_Order++)
        {
            for(Mem_Cnt=0;Mem_Cnt<Num;Mem_Cnt++)
            {
                for(Pivot_Cnt=1;Pivot_Cnt<POW2(Num_Order);Pivot_Cnt++)
                {
                    Pivot_Addr=Start_Addr+Pivot_Cnt*POW2(Total_Order-Num_Order);
                    if((Mem[Mem_Cnt].Start<Pivot_Addr)&&((Mem[Mem_Cnt].Start+(Mem[Mem_Cnt].Size-1))>=Pivot_Addr))
                    {
                        Cut_Apart=1;
                        break;
                    }
                }
                if(Cut_Apart!=0)
                    break;
            }
            if(Cut_Apart!=0)
                break;
        }

        /* For whatever reason, if it breaks, then the last number order must be good */
        if(Num_Order>1)
            Num_Order--;
    }

    return Num_Order;
}
/* End Function:A7M_Num_Order ************************************************/

/* Begin Function:A7M_Map_Page ************************************************
Description : Map pages into the page table as we can. 
Input       : struct Mem_Info* Mem - The memory block list.
              ptr_t Num - The number of memory blocks in the list.
              struct A7M_Pgtbl* Pgtbl - The current page table.
Output      : struct A7M_Pgtbl* Pgtbl - The updated current page table.
Return      : None.
******************************************************************************/
void A7M_Map_Page(struct Mem_Info* Mem, ptr_t Num, struct A7M_Pgtbl* Pgtbl)
{
    ptr_t Page_Cnt;
    ptr_t Mem_Cnt;
    ptr_t Page_Start;
    ptr_t Page_End;
    ptr_t* Page_Num;
    ptr_t Max_Pages;
    ptr_t Max_Mem;

    Page_Num=Malloc(sizeof(ptr_t)*Num);
    if(Page_Num==0)
        EXIT_FAIL("Page count buffer allocation failed.");
    memset(Page_Num, 0, (size_t)(sizeof(ptr_t)*Num));

    /* Use the attribute of the block that covers most pages */
    for(Mem_Cnt=0;Mem_Cnt<Num;Mem_Cnt++)
    {
        for(Page_Cnt=0;Page_Cnt<POW2(Pgtbl->Num_Order);Page_Cnt++)
        {
            Page_Start=Pgtbl->Start_Addr+Page_Cnt*POW2(Pgtbl->Size_Order);
            Page_End=Page_Start+(POW2(Pgtbl->Size_Order)-1);

            if((Mem[Mem_Cnt].Start<=Page_Start)&&((Mem[Mem_Cnt].Start+(Mem[Mem_Cnt].Size-1))>=Page_End))
                Page_Num[Mem_Cnt]++;
        }
    }
    
    Max_Pages=0;
    for(Mem_Cnt=0;Mem_Cnt<Num;Mem_Cnt++)
    {
        if(Page_Num[Mem_Cnt]>Max_Pages)
        {
            Max_Pages=Page_Num[Mem_Cnt];
            Max_Mem=Mem_Cnt;
        }
    }

    /* Is there anything that we should map? If no, we return early */
    if(Max_Pages==0)
        return;
    /* The attribute is the most pronounced memory block's */
    Pgtbl->Attr=Mem[Max_Mem].Attr;

    /* Map whatever we can map, and postpone whatever we will have to postpone */
    for(Page_Cnt=0;Page_Cnt<POW2(Pgtbl->Num_Order);Page_Cnt++)
    {
        Page_Start=Pgtbl->Start_Addr+Page_Cnt*POW2(Pgtbl->Size_Order);
        Page_End=Page_Start+(POW2(Pgtbl->Size_Order)-1);

        /* Can this compartment be mapped? It can if there is one segment covering the range */
        for(Mem_Cnt=0;Mem_Cnt<Num;Mem_Cnt++)
        {
            if((Mem[Mem_Cnt].Start<=Page_Start)&&((Mem[Mem_Cnt].Start+(Mem[Mem_Cnt].Size-1))>=Page_End))
            {
                /* The attribute must be the same as what we map */
                if(Pgtbl->Attr==Mem[Mem_Cnt].Attr)
                    Pgtbl->Mapping[Page_Cnt]=A7M_PGT_MAPPED;
            }
        }
    }
}
/* End Function:A7M_Map_Page *************************************************/

/* Begin Function:A7M_Map_Pgdir ***********************************************
Description : Map page directories into the page table. 
Input       : struct Proc_Info* Proc - The process we are generating pgtbls for.
              struct A7M_Info* A7M - The chip structure.
              struct Mem_Info* Mem - The memory block list.
              ptr_t Num - The number of memory blocks in the list.
              struct A7M_Pgtbl* Pgtbl - The current page table.
Output      : struct A7M_Pgtbl* Pgtbl - The updated current page table.
Return      : None.
******************************************************************************/
void A7M_Map_Pgdir(struct Proc_Info* Proc, struct A7M_Info* A7M, 
                   struct Mem_Info* Mem, ptr_t Num, struct A7M_Pgtbl* Pgtbl)
{
    ptr_t Page_Cnt;
    ptr_t Mem_Cnt;
    ptr_t Page_Start;
    ptr_t Page_End;
    ptr_t Mem_Num;
    struct Mem_Info* Mem_List;
    ptr_t Mem_List_Ptr;

    for(Page_Cnt=0;Page_Cnt<POW2(Pgtbl->Num_Order);Page_Cnt++)
    {
        Page_Start=Pgtbl->Start_Addr+Page_Cnt*POW2(Pgtbl->Size_Order);
        Page_End=Page_Start+(POW2(Pgtbl->Size_Order)-1);

        if(Pgtbl->Mapping[Page_Cnt]==A7M_PGT_NOTHING)
        {
            /* See if any residue memory list are here */
            Mem_Num=0;
            for(Mem_Cnt=0;Mem_Cnt<Num;Mem_Cnt++)
            {
                if((Mem[Mem_Cnt].Start>Page_End)||((Mem[Mem_Cnt].Start+(Mem[Mem_Cnt].Size-1))<Page_Start))
                    continue;
                Mem_Num++;
            }

            if(Mem_Num==0)
                continue;

            Mem_List=Malloc(sizeof(struct Mem_Info)*Mem_Num);
            if(Mem_List==0)
                EXIT_FAIL("Memory list allocation failed.");
                
            /* Collect the memory list */
            Mem_List_Ptr=0;
            for(Mem_Cnt=0;Mem_Cnt<Num;Mem_Cnt++)
            {
                if((Mem[Mem_Cnt].Start>Page_End)||((Mem[Mem_Cnt].Start+(Mem[Mem_Cnt].Size-1))<Page_Start))
                    continue;

                /* Round anything inside to this range */
                if(Mem[Mem_Cnt].Start<Page_Start)
                    Mem_List[Mem_List_Ptr].Start=Page_Start;
                else
                    Mem_List[Mem_List_Ptr].Start=Mem[Mem_Cnt].Start;

                if((Mem[Mem_Cnt].Start+(Mem[Mem_Cnt].Size-1))>Page_End)
                    Mem_List[Mem_List_Ptr].Size=Page_End-Mem_List[Mem_List_Ptr].Start+1;
                else
                    Mem_List[Mem_List_Ptr].Size=Mem[Mem_Cnt].Start+Mem[Mem_Cnt].Size-Mem_List[Mem_List_Ptr].Start;

                Mem_List[Mem_List_Ptr].Attr=Mem[Mem_Cnt].Attr;
                Mem_List[Mem_List_Ptr].Type=Mem[Mem_Cnt].Type;
                /* The alignment is no longer important */
                Mem_List_Ptr++;
            }
            if(Mem_List_Ptr!=Mem_Num)
                EXIT_FAIL("Internal bug occurred at page table allocator.");

            Pgtbl->Mapping[Page_Cnt]=A7M_Gen_Pgtbl(Proc, A7M, Mem_List, Mem_Num, Pgtbl->Size_Order);
        }
    }
}
/* End Function:A7M_Map_Pgdir ************************************************/

/* Begin Function:A7M_Gen_Pgtbl ***********************************************
Description : Recursively construct the page table for the ARMv7-M port.
              This also allocates capid for page tables.
              We have no idea at all how many page tables we are gonna have,
              thus the A7M Pgtbl_Captbl needs to be preallocated with a very large
              number, say, 4096.
Input       : struct Proc_Info* Proc - The process we are generating pgtbls for.
              struct A7M_Info* A7M - The chip structure.
              struct Mem_Info* Mem - The struct containing memory segments to fit
                                     into this level (and below).
              ptr_t Num - The number of memory segments to fit in.
              ptr_t Total_Max - The maximum total order of the page table, cannot
                                be exceeded when deciding the total order of
                                the page table.
Output      : None.
Return      : struct A7M_Pgtbl* - The page table structure returned.
******************************************************************************/
struct A7M_Pgtbl* A7M_Gen_Pgtbl(struct Proc_Info* Proc, struct A7M_Info* A7M,
                                struct Mem_Info* Mem, ptr_t Num, ptr_t Total_Max)
{
    ptr_t Total_Order;
    struct A7M_Pgtbl* Pgtbl;
    static ptr_t Capid=0;
    s8 Buf[16];

    /* Allocate the page table data structure */
    Pgtbl=Malloc(sizeof(struct A7M_Pgtbl));
    if(Pgtbl==0)
        EXIT_FAIL("Page table data structure allocation failed.");
    memset(Pgtbl, 0, sizeof(struct A7M_Pgtbl));

    /* Allocate the capid for this page table */
    Pgtbl->RVM_Capid=Capid;
    sprintf(Buf,"%lld",Pgtbl->RVM_Capid);
    Pgtbl->RVM_Capid_Macro=Make_Macro("RVM_PROC_",Proc->Name,"_PGTBL",Buf);
    A7M->Pgtbl_Captbl[Capid].Proc=Proc;
    A7M->Pgtbl_Captbl[Capid].Cap=(void*)Pgtbl;
    Capid++;
    A7M->Pgtbl_Captbl_Front=Capid;
    if(Capid>A7M_PGTBL_CAPTBL_SIZE)
        EXIT_FAIL("Too many page tables allocated, exceeded the given pgtbl captbl size.");

    /* Total order and start address of the page table */
    Total_Order=A7M_Total_Order(Mem, Num, &(Pgtbl->Start_Addr));
    /* See if this will violate the extension limit */
    if(Total_Order>Total_Max)
        EXIT_FAIL("Memory segment too small, cannot find a reasonable placement.");
    /* Number order */
    Pgtbl->Num_Order=A7M_Num_Order(Mem, Num, Total_Order, Pgtbl->Start_Addr);
    /* Size order */
    Pgtbl->Size_Order=Total_Order-Pgtbl->Num_Order;
    /* Map in all pages */
    A7M_Map_Page(Mem, Num, Pgtbl);
    /* Map in all page directories - recursive */
    A7M_Map_Pgdir(Proc, A7M, Mem, Num, Pgtbl);

    return Pgtbl;
}
/* End Function:A7M_Gen_Pgtbl ************************************************/

/* Begin Function:A7M_Copy_Files **********************************************
Description : Copy all necessary files to the destination folder. This is generic;
              Format-specific files will be copied and generated in the format-specific
              generator.
Input       : struct Proj_Info* Proj - The project structure.
              struct Chip_Info* Chip - The chip structure.
              struct A7M_Info* A7M - The platform-specific project structure.
              s8* RME_Path - The RME root folder path.
              s8* RVM_Path - The RVM root folder path.
              s8* Output_Path - The output folder path.
Output      : None.
Return      : None.
******************************************************************************/
void A7M_Copy_Files(struct Proj_Info* Proj, struct Chip_Info* Chip, struct A7M_Info* A7M,
                    s8* RME_Path, s8* RVM_Path, s8* Output_Path)
{
    s8* Buf1;
    s8* Buf2;

    /* Allocate the buffer */
    Buf1=Malloc(sizeof(s8)*4096);
    if(Buf1==0)
        EXIT_FAIL("Buffer allocation failed");
    Buf2=Malloc(sizeof(s8)*4096);
    if(Buf2==0)
        EXIT_FAIL("Buffer allocation failed");

    /* Perhaps copy some other manuals, etc */

    Free(Buf1);
    Free(Buf2);
}
/* End Function:A7M_Copy_Files ***********************************************/

/* Begin Function:A7M_Gen_Keil_Proj *******************************************
Description : Generate the keil project for ARMv7-M architectures.
Input       : FILE* Keil - The file pointer to the project file.
              s8* Target - The target executable name for the project.
              s8* Device - The device name, must match the keil list.
              s8* Vendor - The vendor name, must match the keil list.
              s8* CPU_Type - The CPU type, must match the keil definitions.
                             "Cortex-M0+", "Cortex-M3"， "Cortex-M4", "Cortex-M7".
              s8* FPU_Type - The FPU type, must match the keil definition.
                             "", "FPU2", "FPU3(SP)", "FPU3(DP)" 
              s8* Endianness - The endianness. "ELITTLE" "EBIG"
              ptr_t Timeopt - Set to 1 when optimizing for time.
              ptr_t Opt - Optimization levels, 1 to 4 corresponds to O0-O3.
              s8** Includes - The include paths.
              ptr_t Include_Num - The number of include paths.
              s8** Paths - The path of the files.
              s8** Files - The file names.
              ptr_t File_Num - The number of files.
Output      : FILE* Keil - The file pointer to the project file.
Return      : None.
******************************************************************************/
void A7M_Gen_Keil_Proj(FILE* Keil,
                       s8* Target, s8* Device, s8* Vendor, 
                       s8* CPU_Type, s8* FPU_Type, s8* Endianness,
                       ptr_t Timeopt, ptr_t Opt,
                       s8** Includes, ptr_t Include_Num,
                       s8** Paths, s8** Files, ptr_t File_Num)
{
    ptr_t Include_Cnt;
    ptr_t File_Cnt;
    s8* Dlloption;

    if(strcmp(CPU_Type, "Cortex-M0+")==0)
        Dlloption="-pCM0+";
    else if(strcmp(CPU_Type, "Cortex-M3")==0)
        Dlloption="-pCM3";
    else if(strcmp(CPU_Type, "Cortex-M4")==0)
        Dlloption="-pCM4";
    else if(strcmp(CPU_Type, "Cortex-M7")==0)
        Dlloption="-pCM7";
    else
        EXIT_FAIL("Internal CPU variant invalid.");

    fprintf(Keil, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n");
    fprintf(Keil, "<Project xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"project_projx.xsd\">\n");
    fprintf(Keil, "  <SchemaVersion>2.1</SchemaVersion>\n");
    fprintf(Keil, "  <Header>### uVision Project, (C) Keil Software</Header>\n");
    fprintf(Keil, "  <Targets>\n");
    fprintf(Keil, "    <Target>\n");
    fprintf(Keil, "      <TargetName>%s</TargetName>\n", Target);
    fprintf(Keil, "      <ToolsetNumber>0x4</ToolsetNumber>\n");
    fprintf(Keil, "      <ToolsetName>ARM-ADS</ToolsetName>\n");
    fprintf(Keil, "      <pCCUsed>ARMCC</pCCUsed>\n");
    fprintf(Keil, "      <uAC6>0</uAC6>\n");
    fprintf(Keil, "      <TargetOption>\n");
    fprintf(Keil, "        <TargetCommonOption>\n");
    fprintf(Keil, "          <Device>%s</Device>\n", Device);
    fprintf(Keil, "          <Vendor>%s</Vendor>\n", Vendor);
    fprintf(Keil, "          <Cpu>IRAM(0x08000000,0x10000) IROM(0x20000000,0x10000) CPUTYPE(\"%s\") %s CLOCK(12000000) %s</Cpu>\n", CPU_Type, FPU_Type, Endianness);
    fprintf(Keil, "          <OutputDirectory>.\\Objects\\</OutputDirectory>\n");
    fprintf(Keil, "          <OutputName>%s</OutputName>\n", Target);
    fprintf(Keil, "          <CreateExecutable>1</CreateExecutable>\n");
    fprintf(Keil, "          <CreateHexFile>1</CreateHexFile>\n");
    fprintf(Keil, "          <DebugInformation>1</DebugInformation>\n");
    fprintf(Keil, "          <BrowseInformation>1</BrowseInformation>\n");
    fprintf(Keil, "          <ListingPath>.\\Listings\\</ListingPath>\n");
    fprintf(Keil, "          <HexFormatSelection>1</HexFormatSelection>\n");
    fprintf(Keil, "          <AfterMake>\n");
    fprintf(Keil, "            <RunUserProg1>0</RunUserProg1>\n");
    fprintf(Keil, "            <RunUserProg2>0</RunUserProg2>\n");
    fprintf(Keil, "            <UserProg1Name></UserProg1Name>\n");
    fprintf(Keil, "            <UserProg2Name></UserProg2Name>\n");
    fprintf(Keil, "            <UserProg1Dos16Mode>0</UserProg1Dos16Mode>\n");
    fprintf(Keil, "            <UserProg2Dos16Mode>0</UserProg2Dos16Mode>\n");
    fprintf(Keil, "            <nStopA1X>0</nStopA1X>\n");
    fprintf(Keil, "            <nStopA2X>0</nStopA2X>\n");
    fprintf(Keil, "          </AfterMake>\n");
    fprintf(Keil, "        </TargetCommonOption>\n");
    fprintf(Keil, "        <CommonProperty>\n");
    fprintf(Keil, "          <UseCPPCompiler>0</UseCPPCompiler>\n");
    fprintf(Keil, "          <RVCTCodeConst>0</RVCTCodeConst>\n");
    fprintf(Keil, "          <RVCTZI>0</RVCTZI>\n");
    fprintf(Keil, "          <RVCTOtherData>0</RVCTOtherData>\n");
    fprintf(Keil, "          <ModuleSelection>0</ModuleSelection>\n");
    fprintf(Keil, "          <IncludeInBuild>1</IncludeInBuild>\n");
    fprintf(Keil, "          <AlwaysBuild>0</AlwaysBuild>\n");
    fprintf(Keil, "          <GenerateAssemblyFile>0</GenerateAssemblyFile>\n");
    fprintf(Keil, "          <AssembleAssemblyFile>0</AssembleAssemblyFile>\n");
    fprintf(Keil, "          <PublicsOnly>0</PublicsOnly>\n");
    fprintf(Keil, "          <StopOnExitCode>3</StopOnExitCode>\n");
    fprintf(Keil, "          <CustomArgument></CustomArgument>\n");
    fprintf(Keil, "          <IncludeLibraryModules></IncludeLibraryModules>\n");
    fprintf(Keil, "          <ComprImg>1</ComprImg>\n");
    fprintf(Keil, "        </CommonProperty>\n");
    fprintf(Keil, "        <DllOption>\n");
    fprintf(Keil, "          <SimDllName>SARMCM3.DLL</SimDllName>\n");
    fprintf(Keil, "          <SimDllArguments> -REMAP -MPU</SimDllArguments>\n");
    fprintf(Keil, "          <SimDlgDll>DCM.DLL</SimDlgDll>\n");
    fprintf(Keil, "          <SimDlgDllArguments>%s</SimDlgDllArguments>\n", Dlloption);
    fprintf(Keil, "          <TargetDllName>SARMCM3.DLL</TargetDllName>\n");
    fprintf(Keil, "          <TargetDllArguments> -MPU</TargetDllArguments>\n");
    fprintf(Keil, "          <TargetDlgDll>TCM.DLL</TargetDlgDll>\n");
    fprintf(Keil, "          <TargetDlgDllArguments>%s</TargetDlgDllArguments>\n", Dlloption);
    fprintf(Keil, "        </DllOption>\n");
    fprintf(Keil, "        <TargetArmAds>\n");
    fprintf(Keil, "          <ArmAdsMisc>\n");
    fprintf(Keil, "            <useUlib>1</useUlib>\n");
    fprintf(Keil, "            <OptFeed>0</OptFeed>\n");
    fprintf(Keil, "          </ArmAdsMisc>\n");
    fprintf(Keil, "          <Cads>\n");
    fprintf(Keil, "            <interw>1</interw>\n");
    fprintf(Keil, "            <Optim>%lld</Optim>\n", Opt);
    fprintf(Keil, "            <oTime>%d</oTime>\n", (Timeopt!=0));
    fprintf(Keil, "            <SplitLS>0</SplitLS>\n");
    fprintf(Keil, "            <OneElfS>1</OneElfS>\n");
    fprintf(Keil, "            <Strict>0</Strict>\n");
    fprintf(Keil, "            <EnumInt>1</EnumInt>\n");
    fprintf(Keil, "            <PlainCh>1</PlainCh>\n");
    fprintf(Keil, "            <Ropi>0</Ropi>\n");
    fprintf(Keil, "            <Rwpi>0</Rwpi>\n");
    fprintf(Keil, "            <wLevel>2</wLevel>\n");
    fprintf(Keil, "            <uThumb>0</uThumb>\n");
    fprintf(Keil, "            <uSurpInc>0</uSurpInc>\n");
    fprintf(Keil, "            <uC99>1</uC99>\n");
    fprintf(Keil, "            <uGnu>1</uGnu>\n");
    fprintf(Keil, "            <useXO>0</useXO>\n");
    fprintf(Keil, "            <v6Lang>1</v6Lang>\n");
    fprintf(Keil, "            <v6LangP>1</v6LangP>\n");
    fprintf(Keil, "            <vShortEn>1</vShortEn>\n");
    fprintf(Keil, "            <vShortWch>1</vShortWch>\n");
    fprintf(Keil, "            <v6Lto>0</v6Lto>\n");
    fprintf(Keil, "            <v6WtE>0</v6WtE>\n");
    fprintf(Keil, "            <v6Rtti>0</v6Rtti>\n");
    fprintf(Keil, "            <VariousControls>\n");
    fprintf(Keil, "              <MiscControls></MiscControls>\n");
    fprintf(Keil, "              <Define></Define>\n");
    fprintf(Keil, "              <Undefine></Undefine>\n");
    fprintf(Keil, "              <IncludePath>");
    /* Print all include paths for C */
    for(Include_Cnt=0;Include_Cnt<Include_Num;Include_Cnt++)
    {
        fprintf(Keil, "%s", Includes[Include_Cnt]);
        if(Include_Cnt<(Include_Num-1))
            fprintf(Keil, ";");
    }
    fprintf(Keil, "</IncludePath>\n");
    fprintf(Keil, "            </VariousControls>\n");
    fprintf(Keil, "          </Cads>\n");
    fprintf(Keil, "          <Aads>\n");
    fprintf(Keil, "            <interw>1</interw>\n");
    fprintf(Keil, "            <Ropi>0</Ropi>\n");
    fprintf(Keil, "            <Rwpi>0</Rwpi>\n");
    fprintf(Keil, "            <thumb>0</thumb>\n");
    fprintf(Keil, "            <SplitLS>0</SplitLS>\n");
    fprintf(Keil, "            <SwStkChk>0</SwStkChk>\n");
    fprintf(Keil, "            <NoWarn>0</NoWarn>\n");
    fprintf(Keil, "            <uSurpInc>0</uSurpInc>\n");
    fprintf(Keil, "            <useXO>0</useXO>\n");
    fprintf(Keil, "            <uClangAs>0</uClangAs>\n");
    fprintf(Keil, "            <VariousControls>\n");
    fprintf(Keil, "              <MiscControls></MiscControls>\n");
    fprintf(Keil, "              <Define></Define>\n");
    fprintf(Keil, "              <Undefine></Undefine>\n");
    fprintf(Keil, "              <IncludePath>");
    /* Print all include paths for assembly */
    for(Include_Cnt=0;Include_Cnt<Include_Num;Include_Cnt++)
    {
        fprintf(Keil, "%s", Includes[Include_Cnt]);
        if(Include_Cnt<(Include_Num-1))
            fprintf(Keil, ";");
    }
    fprintf(Keil, "</IncludePath>\n");
    fprintf(Keil, "            </VariousControls>\n");
    fprintf(Keil, "          </Aads>\n");
    fprintf(Keil, "          <LDads>\n");
    fprintf(Keil, "            <umfTarg>0</umfTarg>\n");
    fprintf(Keil, "            <Ropi>0</Ropi>\n");
    fprintf(Keil, "            <Rwpi>0</Rwpi>\n");
    fprintf(Keil, "            <noStLib>0</noStLib>\n");
    fprintf(Keil, "            <RepFail>1</RepFail>\n");
    fprintf(Keil, "            <useFile>0</useFile>\n");
    fprintf(Keil, "            <TextAddressRange>0x08000000</TextAddressRange>\n");
    fprintf(Keil, "            <DataAddressRange>0x20000000</DataAddressRange>\n");
    fprintf(Keil, "            <pXoBase></pXoBase>\n");
    fprintf(Keil, "            <ScatterFile>.\\Objects\\%s.sct</ScatterFile>\n", Target);
    fprintf(Keil, "            <IncludeLibs></IncludeLibs>\n");
    fprintf(Keil, "            <IncludeLibsPath></IncludeLibsPath>\n");
    fprintf(Keil, "            <Misc></Misc>\n");
    fprintf(Keil, "            <LinkerInputFile></LinkerInputFile>\n");
    fprintf(Keil, "            <DisabledWarnings></DisabledWarnings>\n");
    fprintf(Keil, "          </LDads>\n");
    fprintf(Keil, "        </TargetArmAds>\n");
    fprintf(Keil, "      </TargetOption>\n");
    fprintf(Keil, "      <Groups>\n");
    /* Print all files. We only have two groups in all cases. */
    fprintf(Keil, "        <Group>\n");
    fprintf(Keil, "          <GroupName>%s</GroupName>\n", Target);
    fprintf(Keil, "          <Files>\n");
    for(File_Cnt=0;File_Cnt<File_Num;File_Cnt++)
    {
        fprintf(Keil, "            <File>\n");
        fprintf(Keil, "              <FileName>%s</FileName>\n", Files[File_Cnt]);
        if(Files[File_Cnt][strlen(Files[File_Cnt])-1]=='c')
            fprintf(Keil, "              <FileType>1</FileType>\n");
        else
            fprintf(Keil, "              <FileType>2</FileType>\n");
        fprintf(Keil, "              <FilePath>%s/%s</FilePath>\n", Paths[File_Cnt], Files[File_Cnt]);
        fprintf(Keil, "            </File>\n");
    }
    fprintf(Keil, "          </Files>\n");
    fprintf(Keil, "        </Group>\n");
    fprintf(Keil, "        <Group>\n");
    fprintf(Keil, "          <GroupName>User</GroupName>\n");
    fprintf(Keil, "        </Group>\n");
    fprintf(Keil, "      </Groups>\n");
    fprintf(Keil, "    </Target>\n");
    fprintf(Keil, "  </Targets>\n");
    fprintf(Keil, "</Project>\n");
}
/* End Function:A7M_Gen_Keil_Proj ********************************************/

/* Begin Function:A7M_Gen_Keil_RME ********************************************
Description : Generate the RME files for keil uvision. 
              This includes the platform-specific assembly file and the scatter.
Input       : struct Proj_Info* Proj - The project structure.
              struct Chip_Info* Chip - The chip structure.
              struct A7M_Info* A7M - The port specific structure.
              s8* RME_Path - The RME root folder path.
              s8* Output_Path - The output folder path.
Output      : None.
Return      : None.
******************************************************************************/
void A7M_Gen_Keil_RME(struct Proj_Info* Proj, struct Chip_Info* Chip, struct A7M_Info* A7M, 
                      s8* RME_Path, s8* Output_Path)
{

}
/* End Function:A7M_Gen_Keil_RME *********************************************/

/* Begin Function:A7M_Gen_Keil_RVM ********************************************
Description : Generate the RVM files for keil uvision. 
              This includes the platform-specific assembly file and the scatter.
Input       : struct Proj_Info* Proj - The project structure.
              struct Chip_Info* Chip - The chip structure.
              struct A7M_Info* A7M - The port specific structure.
              s8* RVM_Path - The RVM root folder path.
              s8* Output_Path - The output folder path.
Output      : None.
Return      : None.
******************************************************************************/
void A7M_Gen_Keil_RVM(struct Proj_Info* Proj, struct Chip_Info* Chip, struct A7M_Info* A7M, 
                      s8* RVM_Path, s8* Output_Path)
{

}
/* End Function:A7M_Gen_Keil_RVM *********************************************/

/* Begin Function:A7M_Gen_Keil_Proc *******************************************
Description : Generate the process files for keil uvision. 
              This includes the platform-specific assembly file and the scatter.
Input       : struct Proj_Info* Proj - The project structure.
              struct Chip_Info* Chip - The chip structure.
              struct A7M_Info* A7M - The port specific structure.
              s8* RME_Path - The RME root folder path.
              s8* RVM_Path - The RVM root folder path.
              s8* Output_Path - The output folder path.
Output      : None.
Return      : None.
******************************************************************************/
void A7M_Gen_Keil_Proc(struct Proj_Info* Proj, struct Chip_Info* Chip, struct A7M_Info* A7M, 
                       s8* RME_Path, s8* RVM_Path, s8* Output_Path)
{

}
/* End Function:A7M_Gen_Keil_Proc ********************************************/

/* Begin Function:A7M_Gen_Keil ************************************************
Description : Generate the keil project for ARMv7-M. 
              Keil projects include three parts: 
              .uvmpw (the outside workspace)
              .uvprojx (the specific project files)
              .uvoptx (the options for some unimportant stuff)
Input       : struct Proj_Info* Proj - The project structure.
              struct Chip_Info* Chip - The chip structure.
              struct A7M_Info* A7M - The port specific structure.
              s8* RME_Path - The RME root folder path.
              s8* RVM_Path - The RVM root folder path.
              s8* Output_Path - The output folder path.
Output      : None.
Return      : None.
******************************************************************************/
void A7M_Gen_Keil(struct Proj_Info* Proj, struct Chip_Info* Chip, struct A7M_Info* A7M,
                  s8* RME_Path, s8* RVM_Path, s8* Output_Path)
{
    /* Common for all projects */
    s8* Device;
    s8* Vendor;
    s8* CPU_Type;
    s8* FPU_Type;
    s8* Endianness;
    /* Project specific */
    FILE* Keil;
    s8* Proj_Path;
    s8* Target;
    ptr_t Opt;
    ptr_t Timeopt;
    /* Include path and project file buffer - we never exceed 8 */
    s8* Includes[8];
    ptr_t Include_Num;
    s8* Paths[8];
    s8* Files[8];
    ptr_t File_Num;

    /* Allocate project path buffer */
    Proj_Path=Malloc(4096);
    if(Proj_Path==0)
        EXIT_FAIL("Project path allocation failed");

    /* Decide process specific macros - only STM32 is like this */
    if((strncmp(Proj->Chip,"STM32", 5)==0))
    {
        if(strncmp(Proj->Chip,"STM32F1", 7)==0)
            Device=Proj->Chip;
        else
        {
            Device=Malloc(((ptr_t)strlen(Proj->Fullname))+1);
            strcpy(Device, Proj->Fullname);
            /* Except for STM32F1, all chips end with suffix, and the last number is substituted with 'x' */
            Device[strlen(Proj->Fullname)-1]='x';
        }
    }
    else
        Device=Proj->Chip;

    Vendor=Chip->Vendor;

    switch(A7M->CPU_Type)
    {
        case A7M_CPU_CM0P:CPU_Type="Cortex-M0+";break;
        case A7M_CPU_CM3:CPU_Type="Cortex-M3";break;
        case A7M_CPU_CM4:CPU_Type="Cortex-M4";break;
        case A7M_CPU_CM7:CPU_Type="Cortex-M7";break;
        default:EXIT_FAIL("Wrong CPU type selected.");break;
    }

    switch(A7M->FPU_Type)
    {
        case A7M_FPU_NONE:FPU_Type="";break;
        case A7M_FPU_FPV4:FPU_Type="FPU2";break;
        case A7M_FPU_FPV5_SP:FPU_Type="FPU3(SFPU)";break;
        case A7M_FPU_FPV5_DP:FPU_Type="FPU3(DFPU)";break;
        default:EXIT_FAIL("Wrong FPU type selected.");break;
    }

    if(A7M->Endianness==A7M_END_LITTLE)
        Endianness="ELITTLE";
    else if(A7M->Endianness==A7M_END_BIG)
        Endianness="EBIG";
    else
        EXIT_FAIL("Wrong endianness specified.");

    /* Generate the RME keil project first */
    sprintf(Proj_Path, "%s/M7M1_MuEukaron/Project/%s_RME.uvprojx", Output_Path, Proj->Name);
    Keil=fopen(Proj_Path, "wb");
    Target="RME";
    Opt=Proj->RME.Comp.Opt+1;
    Timeopt=Proj->RME.Comp.Prio;
    /* Allocate the include list */
    Includes[0]=".";
    Includes[1]="./Source";
    Includes[2]="./Include";
    Includes[3]="../MEukaron/Include";
    Include_Num=4;
    /* Allocate the file list */
    Paths[0]="./Source";
    Files[0]="rme_boot.c";
    Paths[1]="./Source";
    Files[1]="rme_user.c";
    Paths[2]="../MEukaron/Kernel";
    Files[2]="rme_kernel.c";
    Paths[3]="../MEukaron/Platform/A7M";
    Files[3]="rme_platform_a7m.c";
    if(A7M->CPU_Type==A7M_CPU_CM0P)
    {
        Paths[4]="../MEukaron/Platform/A7M";
        Files[4]="rme_platform_a7m0.s";
    }
    else
    {
        Paths[4]="../MEukaron/Platform/A7M";
        Files[4]="rme_platform_a7m.s";
    }
    File_Num=5;
    /* Generate whatever files that are keil specific */
    A7M_Gen_Keil_RME(Proj, Chip, A7M, RME_Path, Output_Path);
    /* Actually generate the project */
    A7M_Gen_Keil_Proj(Keil, Target, Device, Vendor, CPU_Type, FPU_Type, Endianness,
                      Timeopt, Opt,
                      Includes, Include_Num,
                      Paths, Files, File_Num);
    fclose(Keil);

    /* Generate the RVM keil project then */

    /* Generate all other projects one by one */

    /* Finally, generate the uvmpw at the root folder */
    
    /* Free project buffer */
    Free(Proj_Path);
}
/* End Function:A7M_Gen_Keil *************************************************/

/* Begin Function:A7M_Gen_Makefile ********************************************
Description : Generate the makefile project for ARMv7-M. 
Input       : struct Proj_Info* Proj - The project structure.
              struct Chip_Info* Chip - The chip structure.
              struct A7M_Info* A7M - The port specific structure.
              ptr_t Output_Type - The output type.
              s8* Output_Path - The output folder path.
              s8* RME_Path - The RME root folder path.
              s8* RVM_Path - The RVM root folder path.
Output      : None.
Return      : struct A7M_Pgtbl* - The page table structure returned.
******************************************************************************/
void A7M_Gen_Makefile(struct Proj_Info* Proj, struct Chip_Info* Chip, struct A7M_Info* A7M,
                      ptr_t Output_Type, s8* Output_Path, s8* RME_Path, s8* RVM_Path)
{
    /* Generate the makefile project */
}
/* End Function:A7M_Gen_Makefile *********************************************/

/* Begin Function:A7M_Gen_Check ***********************************************
Description : Check if the input is really valid for the ARMv7-M port.
Input       : struct Proj_Info* Proj - The project structure.
              struct Chip_Info* Chip - The chip structure.
Output      : None.
Return      : None.
******************************************************************************/
void A7M_Gen_Check(struct Proj_Info* Proj, struct Chip_Info* Chip)
{
    ptr_t Proc_Cnt;
    ptr_t Mem_Cnt;

    for(Proc_Cnt=0;Proc_Cnt<Proj->Proc_Num;Proc_Cnt++)
    {
        /* Check memory blocks - they must be at least readable */
        for(Mem_Cnt=0;Mem_Cnt<Proj->Proc[Proc_Cnt].Mem_Num;Mem_Cnt++)
        {
            if((Proj->Proc[Proc_Cnt].Mem[Mem_Cnt].Attr&MEM_READ)==0)
                EXIT_FAIL("All memory allocated must be readable on A7M.");
        }
        /* Check if the capability table of processes, plus the extra captbl space
         * required, exceeds 128 - this is simply not allowed for processes. */
        if((Proj->Proc[Proc_Cnt].Captbl_Front+Proj->Proc[Proc_Cnt].Extra_Captbl)>A7M_PROC_CAPTBL_LIMIT)
            EXIT_FAIL("One of the processes have more capabilities in its capability table than allowed.");
    }
}
/* End Function:A7M_Gen_Check ************************************************/

/* Begin Function:A7M_Gen_Proj ************************************************
Description : Generate the project for Cortex-M based processors.
Input       : struct Proj_Info* Proj - The project structure.
              struct Chip_Info* Chip - The chip structure.
              s8* RME_Path - The RME root folder path.
              s8* RVM_Path - The RVM root folder path.
              s8* Output_Path - The output folder path.
              s8* Format - The output format.
Output      : None.
Return      : None.
******************************************************************************/
void A7M_Gen_Proj(struct Proj_Info* Proj, struct Chip_Info* Chip,
                  s8* RME_Path, s8* RVM_Path, s8* Output_Path, s8* Format)
{
    ptr_t Proc_Cnt;
    struct A7M_Info* A7M;

    /* Allocate architecture-specific project structure */
    A7M=Malloc(sizeof(struct A7M_Info));
    if(A7M==0)
        EXIT_FAIL("Platform specific project struct allocation failed.");
    /* Parse A7M-specific options */
    A7M_Parse_Options(Proj, Chip, A7M);
    /* Check if we can really create such a system */
    A7M_Gen_Check(Proj, Chip);
    /* Allocate page table global captbl - it is unlikely that this number gets exceeded */
    A7M->Pgtbl_Captbl=Malloc(sizeof(struct RVM_Cap_Info)*A7M_PGTBL_CAPTBL_SIZE);
    /* Allocate page tables for all processes */
    A7M->Pgtbl=Malloc(sizeof(struct A7M_Pgtbl*)*Proj->Proc_Num);
    if(A7M->Pgtbl==0)
        EXIT_FAIL("Project page table allocation failed.");
    for(Proc_Cnt=0;Proc_Cnt<Proj->Proc_Num;Proc_Cnt++)
    {
        A7M->Pgtbl[Proc_Cnt]=A7M_Gen_Pgtbl(&(Proj->Proc[Proc_Cnt]),A7M,
                                           Proj->Proc[Proc_Cnt].Mem,Proj->Proc[Proc_Cnt].Mem_Num,32);
        if(A7M->Pgtbl[Proc_Cnt]==0)
            EXIT_FAIL("Page table generation failed.");
    }

    /* Set up the folder structures, and copy all files to where they should be */
    A7M_Copy_Files(Proj, Chip, A7M, RME_Path, RVM_Path, Output_Path);
    /* Write the files that are tool-independent */
    //A7M_Gen_Scripts(Proj, Chip, A7M, RME_Path, RVM_Path, Output_Path);

	/* Create the folder first and copy all necessary files into whatever possible */
    if(strcmp(Format, "keil")==0)
        A7M_Gen_Keil(Proj, Chip, A7M, RME_Path, RVM_Path, Output_Path);
    else
        EXIT_FAIL("Other projects not supported at the moment.");
}
/* End Function:A7M_Gen_Proj *************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
