/******************************************************************************
Filename    : rme_mcu.c
Author      : pry
Date        : 20/04/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The configuration generator for the MCU ports. This does not
              apply to the desktop or mainframe port; it uses its own generator.
			  This project uses a garbage collection method that is more advanced.
              1.Read the project-level configuration XMLs and device-level 
                configuration XMLs into its internal data structures. Should
                we find any parsing errors, we report and error out.
              2.Call the port-level generator to generate the project for that port.
                1.Detect any errors in the configuration structure. If any is found,
                  error out.
                2.Align memory. For program memory and data memory, rounding their
                  size is allowed; for specifically pointed out memory, rounding
                  their size is not allowed. 
                3.Generate memory map. This places all the memory segments into
                  the memory map, and fixes their specific size, etc. If the chip's
                  memory is deemed too small, we will resort to a larger chip.
                4.Generate the kernel object script. This now fleshes out all the kernel
                  objects which the Init is responsible for creation in the system.
                  Vectors is an exception: the kernel creates these endpoints and 
                  the Init just makes necessary endpoint delegations. If we encounter
                  cases where the kernel object creation takes too much memory (exceeds)
                  the hard limit, etc, emit an error so the user knows to give more
                  kernel memory. For capability table sizes, same. The kernel memory
                  cannot be AUTO and must be specified because this has direct relation
                  to do with the former memory allocation process. Capability table 
                  sizes, on the other hand, is calculated based on the present usage
                  plus an user given extra number (should the user decide to do add).
                5.Generate, copy all necessary files and setup the directory structure.
                  Should we encounter any file I/O error, report and exit.
                6.Call the tool-level project generator to generate project files. Should
                  the tool have any project group or workspace creation capability, create
                  the project group or workspace.
                  7. The generator should generate separate projects for the RME.
                  8. Then generates project for RVM. 
                  9. And generates project for all other processes.
              3.Report to the user that the project generation is complete.

#error: add m7m2-root folder. RVM is always necessary. Hand-crafted stuff should always be there for convenience. 
when making init, always run one virtual machine??? Current organization is just terrible.
Additionally, how much memory does it take? totally unknown to us.
Another issue: how to make the example programs? Is the current one too hard? 
We should remove the VMM functionality now, maybe we can add it back later on.
We also have a bunch of other stuff that is waiting for us. How to deal with this?
The MCU user-level lib includes the following:

Basics -> Some to be created, some is there.
VMM -> This is the VMM.
Posix -> This is the posix.
This is going to look very bad. Very bad.
How to make the example project - That becomes really tough. Can't make use of the VMM anymore.Try_Bitmap
We cannot delete the project folder either. This is nuts. 
Or, maybe, we can consider merging the RVM with the RME, combined in a single file implementation of the user-level.
Additionally, The performance of the system is still unmeasured. This is not good at all.
The HAL organization is also nuts.

Are we really gonna do that? Or, what will M7M2 do? We cannot always rely on the M5P1.
Or, consider removing M7M2, and make the whole thing, including all drivers, M7M1. This is 
sensible, but there's the risk of producing something very large and very complex.

Additionally - how is this going to integrate with the rest of the stuff? How? this is going to
be very hard again. 
Anything that is standalone compilable should be a standalone project. Or, it should not be
standalone at all.
M7M2 should be merged with M7M1, due to the fact that it is not standalone testable.
Because all user-level test cases should involve M7M2. 

Very hard still. we have RVM now, and we know how to do it, and it can compile to a standalone
binary.
simple. Just use two stuff, one is the RVM, another is the binary, in the same folder. 
we still need M7M2 path. This is still necessary.
For this project to compile, you need all three in the same folder to work together, as
they are made to work together.
Let's just provide all MCU examples with that. This is probably the best we can do.
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
/* Optimization levels */
#define OPT_O0          0
#define OPT_O1          1
#define OPT_O2          2
#define OPT_O3          3
#define OPT_OS          4
/* Library choice */
#define LIB_SMALL       0
#define LIB_FULL        1
/* Capability ID placement */
#define AUTO            ((ptr_t)(-1))
#define INVALID         ((ptr_t)(-2))
/* Recovery options */
#define RECOVERY_THD    (0)
#define RECOVERY_PROC   (1)
#define RECOVERY_SYS    (2)
/* Memory types */
#define MEM_CODE        0
#define MEM_DATA        1
#define MEM_DEVICE      2
/* Memory access permissions */
#define MEM_READ        0
#define MEM_WRITE       1
#define MEM_EXECUTE     2
#define MEM_BUFFERABLE  3
#define MEM_CACHEABLE   4
#define MEM_STATIC      5
/* Endpoint types */
#define ENDP_SEND       0
#define ENDP_RECEIVE    1
#define ENDP_HANDLER    2
/* Option types */
#define OPTION_RANGE    0
#define OPTION_SELECT   1
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
        EXIT_FAIL("%s label is malformed.", (NAME)); \
    if(Compare_Label((LABEL_START), (LABEL_END), (NAME))!=0) \
        EXIT_FAIL("%s label not found.", (NAME)); \
    Start=Val_End; \
} \
while(0)
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
typedef s32 ret_t;
typedef s32 cnt_t;
typedef u32 ptr_t;
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
	cnt_t Opt;
	cnt_t Lib;
};
/* Raw information to be fed to the platform-specific parser */
struct Raw_Info
{
	cnt_t Num;
	s8** Tag;
	s8** Value;
};
/* RME kernel information */
struct RME_Info
{
	struct Comp_Info Comp;
	ptr_t Code_Start;
	ptr_t Code_Size;
	ptr_t Data_Start;
	ptr_t Data_Size;
	ptr_t Extra_Kmem;
	ptr_t Kmem_Order;
	ptr_t Kern_Prios;
	struct Raw_Info Plat_Raw;
	struct Raw_Info Chip_Raw;
};
/* RVM user-level library information. */
struct RVM_Info
{
	struct Comp_Info Comp;
    ptr_t Code_Size;
    ptr_t Data_Size;
	ptr_t Extra_Captbl;
	ptr_t Recovery;
};
/* Memory segment information */
struct Mem_Info
{
	ptr_t Start;
	ptr_t Size;
	cnt_t Type;
	ptr_t Attr;
    ptr_t Align;
};
/* Thread information */
struct Thd_Info
{
	s8* Name;
	s8* Entry;
	ptr_t Stack_Addr;
	ptr_t Stack_Size;
	ptr_t Parameter;
	ptr_t Priority;
};
/* Invocation information */
struct Inv_Info
{
	s8* Name;
	s8* Entry;
	ptr_t Stack_Addr;
	ptr_t Stack_Size;
};
/* Port information */
struct Port_Info
{
	s8* Name;
    s8* Process;
};
/* Endpoint information */
struct Endp_Info
{
	s8* Name;
	ptr_t Type;
    s8* Process;
};
/* Process information */
struct Proc_Info
{
    s8* Name;
	ptr_t Extra_Captbl;
	struct Comp_Info Comp;
	cnt_t Mem_Num;
	struct Mem_Info* Mem;
	cnt_t Thd_Num;
	struct Thd_Info* Thd;
	cnt_t Inv_Num;
	struct Inv_Info* Inv;
	cnt_t Port_Num;
	struct Port_Info* Port;
	cnt_t Endp_Num;
	struct Endp_Info* Endp;
};
/* Whole project information */
struct Proj_Info
{
	s8* Name;
    s8* Platform;
	s8* Chip;
	struct RME_Info RME;
	struct RVM_Info RVM;
	cnt_t Proc_Num;
	struct Proc_Info* Proc;
};
/* Chip option macro informations */
struct Option_Info
{
	s8* Name;
	cnt_t Type;
	s8* Macro;
    /* Only one of these will take effect */
    ptr_t Range_Min;
    ptr_t Range_Max;
    ptr_t Select_Num;
	s8** Select_Opt;
};
/* Vector informations */
struct Vect_Info
{
	s8* Name;
	ptr_t Number;
};
/* Chip information - this is platform independent as well */
struct Chip_Info
{
	s8* Name;
	ptr_t Plat_Type;
	ptr_t Cores;
    ptr_t Regions;
	ptr_t Mem_Num;
	struct Mem_Info* Mem;
	cnt_t Option_Num;
	struct Option_Info* Option;
	cnt_t Vect_Num;
	struct Vect_Info* Vect;
};

/* Memory map information - min granularity 4B */
struct Mem_Map
{
    ptr_t Mem_Num;
    s8** Mem_Bitmap;
    struct Mem_Info** Mem_Array;

    ptr_t Proc_Mem_Num;
    struct Mem_Info** Proc_Mem_Array;
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
	Addr=malloc(Size+sizeof(struct List));
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
	List=(struct List*)(((ptr_t)Addr)-sizeof(struct List));
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

/* Begin Function:Cmdline_Proc ************************************************
Description : Preprocess the input parameters, and generate a preprocessed
              instruction listing with all the comments stripped.
Input       : int argc - The number of arguments.
              char* argv[] - The arguments.
Output      : s8** Input_File - The input project file path.
              s8** Output_File - The output folder path, must be empty.
			  s8** RME_Path - The RME root folder path, must contain RME files.
			  s8** RVM_Path - The RME root folder path, must contain RME files.
			  cnt_t* Output_Type - The output type.
Return      : None.
******************************************************************************/
void Cmdline_Proc(int argc,char* argv[], s8** Input_File, s8** Output_Path,
                  s8** RME_Path, s8** RVM_Path, cnt_t* Output_Type)
{
    cnt_t Count;

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
	*Output_Type=0;

    Count=1;
    /* Read the command line one by one */
    while(Count<argc)
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
            if(*Output_Type!=0)
                EXIT_FAIL("Conflicting output project format designated.");

            /* Keil uVision output format */
            if(strcmp(argv[Count+1],"keil")==0)
                *Output_Type=OUTPUT_KEIL;
            /* Eclipse output format */
            else if(strcmp(argv[Count+1],"eclipse")==0)
                *Output_Type=OUTPUT_ECLIPSE;
            /* Plain makefile output format */
            else if(strcmp(argv[Count+1],"makefile")==0)
                *Output_Type=OUTPUT_MAKEFILE;
            else
                EXIT_FAIL("Unrecognized output project format designated.");

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
    if(*Output_Type==0)
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
	fread(Buf, 1, Size, Handle);

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
	cnt_t Count;

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
	cnt_t Num_Elems;
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
	*Val_End=Slide_Ptr-1;
	Close_Label_Start=Slide_Ptr+2;

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
Return      : cnt_t - The number of elements found.
******************************************************************************/
cnt_t XML_Num(s8* Start, s8* End)
{
    cnt_t Num;
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

    return INVALID;
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
        Val*=16;
        if((Start_Ptr[0]>='0')&&(Start_Ptr[0]<='9'))
            Val+=Start_Ptr[0]-'0';
        else
            return INVALID;
    }

    return INVALID;
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
    cnt_t Len;

    Len=End-Start+1;
    if(Len!=strlen(Label))
        return -1;

    if(strncmp(Start,Label,Len)!=0)
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
    cnt_t Count;

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
    /* Library level */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Library");
    if(strncmp(Val_Start,"Small",5)==0)
        Proj->RME.Comp.Lib=LIB_SMALL;
    else if(strncmp(Val_Start,"Full",4)==0)
        Proj->RME.Comp.Lib=LIB_FULL;
    else
        EXIT_FAIL("The library option is malformed.");

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
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Priorities");
    Proj->RME.Priorities=Get_Uint(Val_Start,Val_End);
    if(Proj->RME.Priorities>=INVALID)
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
            Proj->RME.Plat_Raw.Tag[Count]=Get_String(Val_Start, Val_End);
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
            Proj->RME.Chip_Raw.Tag[Count]=Get_String(Val_Start, Val_End);
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
    cnt_t Count;

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
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, VMM);
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
    /* Library level */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Library");
    if(strncmp(Val_Start,"Small",5)==0)
        Proj->RVM.Comp.Lib=LIB_SMALL;
    else if(strncmp(Val_Start,"Full",4)==0)
        Proj->RVM.Comp.Lib=LIB_FULL;
    else
        EXIT_FAIL("The library option is malformed.");

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
              cnt_t Proc_Num - The process number.
              cnt_t Mem_Num - The memory number.
              s8* Str_Start - The start position of the string.
              s8* Str_End - The end position of the string.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t Parse_Process_Memory(struct Proj_Info* Proj, cnt_t Proc_Num, cnt_t Mem_Num, s8* Str_Start, s8* Str_End)
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
    else if(strchr(Attr_Temp,'W')==0)
        Proj->Proc[Proc_Num].Mem[Mem_Num].Attr|=MEM_WRITE;
    else if(strchr(Attr_Temp,'X')==0)
        Proj->Proc[Proc_Num].Mem[Mem_Num].Attr|=MEM_EXECUTE;
    else
        EXIT_FAIL("No access to the memory is allowed.");
    if(strchr(Attr_Temp,'B')!=0)
        Proj->Proc[Proc_Num].Mem[Mem_Num].Attr|=MEM_BUFFERABLE;
    else if(strchr(Attr_Temp,'W')==0)
        Proj->Proc[Proc_Num].Mem[Mem_Num].Attr|=MEM_CACHEABLE;
    else if(strchr(Attr_Temp,'X')==0)
        Proj->Proc[Proc_Num].Mem[Mem_Num].Attr|=MEM_STATIC;
    Free(Attr_Temp);

    return 0;
}
/* End Function:Parse_Process_Memory *****************************************/

/* Begin Function:Parse_Thread ************************************************
Description : Parse the thread section of a particular process.
Input       : struct Proj_Info* Proj - The project structure.
              cnt_t Proc_Num - The process number.
              cnt_t Thd_Num - The thread number.
              s8* Str_Start - The start position of the string.
              s8* Str_End - The end position of the string.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t Parse_Thread(struct Proj_Info* Proj, cnt_t Proc_Num, cnt_t Thd_Num, s8* Str_Start, s8* Str_End)
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
    Proj->Proc[Proc_Num].Thd[Thd_Num].Entry=Get_Hex(Val_Start,Val_End);
    if(Proj->Proc[Proc_Num].Thd[Thd_Num].Entry>=INVALID)
        EXIT_FAIL("Thread stack size read failed.");
    /* Parameter */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Parameter");
    Proj->Proc[Proc_Num].Thd[Thd_Num].Parameter=Get_String(Val_Start,Val_End);
    if(Proj->Proc[Proc_Num].Thd[Thd_Num].Parameter==0)
        EXIT_FAIL("Thread parameter value read failed.");
    /* Priority level */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Priority");
    Proj->Proc[Proc_Num].Thd[Thd_Num].Entry=Get_Uint(Val_Start,Val_End);
    if(Proj->Proc[Proc_Num].Thd[Thd_Num].Entry>=INVALID)
        EXIT_FAIL("Thread priority read failed.");

    return 0;
}
/* End Function:Parse_Thread *************************************************/

/* Begin Function:Parse_Invocation ********************************************
Description : Parse the invocation section of a particular process.
Input       : struct Proj_Info* Proj - The project structure.
              cnt_t Proc_Num - The process number.
              cnt_t Inv_Num - The invocation number.
              s8* Str_Start - The start position of the string.
              s8* Str_End - The end position of the string.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t Parse_Invocation(struct Proj_Info* Proj, cnt_t Proc_Num, cnt_t Inv_Num, s8* Str_Start, s8* Str_End)
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
    if(Proj->Proc[Proc_Num].Inv[Thd_Num].Stack_Addr==INVALID)
        EXIT_FAIL("Invocation stack address read failed.");
    /* Stack size */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Stack_Size");
    Proj->Proc[Proc_Num].Inv[Inv_Num].Entry=Get_Hex(Val_Start,Val_End);
    if(Proj->Proc[Proc_Num].Inv[Inv_Num].Entry>=INVALID)
        EXIT_FAIL("Invocation stack size read failed.");
    
    return 0;
}
/* End Function:Parse_Invocation *********************************************/

/* Begin Function:Parse_Port **************************************************
Description : Parse the port section of a particular process.
Input       : struct Proj_Info* Proj - The project structure.
              cnt_t Proc_Num - The process number.
              cnt_t Port_Num - The port number.
              s8* Str_Start - The start position of the string.
              s8* Str_End - The end position of the string.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t Parse_Port(struct Proj_Info* Proj, cnt_t Proc_Num, cnt_t Port_Num, s8* Str_Start, s8* Str_End)
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
/* End Function:Parse_Port ***************************************************/

/* Begin Function:Parse_Endpoint **********************************************
Description : Parse the endpoint section of a particular process.
Input       : struct Proj_Info* Proj - The project structure.
              cnt_t Proc_Num - The process number.
              cnt_t Endp_Num - The endpoint number.
              s8* Str_Start - The start position of the string.
              s8* Str_End - The end position of the string.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t Parse_Endpoint(struct Proj_Info* Proj, cnt_t Proc_Num, cnt_t Endp_Num, s8* Str_Start, s8* Str_End)
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
    Proj->Proc[Proc_Num].Endp[Endp_Num].Name=Get_String(Val_Start,Val_End);
    if(Proj->Proc[Proc_Num].Endp[Endp_Num].Name==0)
        EXIT_FAIL("Thread name value read failed.");
    /* Type */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Type");
    if(strncmp(Val_Start,"Send",4)==0)
        Proj->Proc[Proc_Num].Endp[Endp_Num].Type=ENDP_SEND;
    else if(strncmp(Val_Start,"Receive",7)==0)
        Proj->Proc[Proc_Num].Endp[Endp_Num].Type=ENDP_RECEIVE;
    else if(strncmp(Val_Start,"Handler",7)==0)
        Proj->Proc[Proc_Num].Endp[Endp_Num].Type=ENDP_HANDLER;
    else
        EXIT_FAIL("The endpoint type is malformed.");
    /* Process */
    if(Proj->Proc[Proc_Num].Endp[Endp_Num].Type==ENDP_SEND)
    {
        GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Process");
        Proj->Proc[Proc_Num].Endp[Endp_Num].Process=Get_String(Val_Start,Val_End);
        if(Proj->Proc[Proc_Num].Endp[Endp_Num].Process==0)
            EXIT_FAIL("Endpoint process value read failed.");
    }
    else
        Proj->Proc[Proc_Num].Port[Endp_Num].Process=0;

    return 0;
}
/* End Function:Parse_Endpoint ***********************************************/

/* Begin Function:Parse_Process ***********************************************
Description : Parse a particular process.
Input       : struct Proj_Info* Proj - The project structure.
              cnt_t Num - The process number.
              s8* Str_Start - The start position of the string.
              s8* Str_End - The end position of the string.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t Parse_Process(struct Proj_Info* Proj, cnt_t Num, s8* Str_Start, s8* Str_End)
{
    s8* Start;
    s8* End;
    s8* Label_Start;
    s8* Label_End;
    s8* Val_Start;
    s8* Val_End;
    cnt_t Count;
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
    s8* Endpoint_Start;
    s8* Endpoint_End;

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
    /* Endpoints section */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Endpoint");
    Endpoint_Start=Val_Start;
    Endpoint_End=Val_End;

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
    /* Library level */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Library");
    if(strncmp(Val_Start,"Small",5)==0)
        Proj->Proc[Num].Comp.Lib=LIB_SMALL;
    else if(strncmp(Val_Start,"Full",4)==0)
        Proj->Proc[Num].Comp.Lib=LIB_FULL;
    else
        EXIT_FAIL("The library option is malformed.");

    /* Parse memory section */
    Start=Memories_Start;
    End=Memories_End;
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
        Parse_Process_Memory(Proj, Num, Count, Val_Start, Val_End);
    }

    /* Parse threads section */
    Start=Thread_Start;
    End=Thread_End;
    Proj->Proc[Num].Thd_Num=XML_Num(Start, End);
    if(Proj->Proc[Num].Thd_Num!=0)
    {
        Proj->Proc[Num].Thd=Malloc(sizeof(struct Thd_Info)*Proj->Proc[Num].Mem_Num);
        if(Proj->Proc[Num].Thd==0)
            EXIT_FAIL("The thread structure allocation failed.");
        for(Count=0;Count<Proj->Proc[Num].Thd_Num;Count++)
        {
            if(XML_Get_Next(Start, End, &Label_Start, &Label_End, &Val_Start, &Val_End)!=0)
                EXIT_FAIL("Unexpected error when parsing thread section.");
            Parse_Thread(Proj, Num, Count, Val_Start, Val_End);
        }
    }

    /* Parse invocations section */
    Start=Invocation_Start;
    End=Invocation_End;
    Proj->Proc[Num].Inv_Num=XML_Num(Start, End);
    if(Proj->Proc[Num].Inv_Num==0)
    {
        if(Proj->Proc[Num].Thd_Num==0)
            EXIT_FAIL("The process is malformed, doesn't contain any threads or invocations.");
    }
    else
    {
        Proj->Proc[Num].Inv=Malloc(sizeof(struct Inv_Info)*Proj->Proc[Num].Inv_Num);
        if(Proj->Proc[Num].Inv==0)
            EXIT_FAIL("The invocation structure allocation failed.");
        for(Count=0;Count<Proj->Proc[Num].Inv_Num;Count++)
        {
            if(XML_Get_Next(Start, End, &Label_Start, &Label_End, &Val_Start, &Val_End)!=0)
                EXIT_FAIL("Unexpected error when parsing invocation section.");
            Parse_Invocation(Proj, Num, Count, Val_Start, Val_End);
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
            Parse_Port(Proj, Num, Count, Val_Start, Val_End);
        }
    }

    /* Parse endpoints section */
    Start=Endpoint_Start;
    End=Endpoint_End;
    Proj->Proc[Num].Endp_Num=XML_Num(Start, End);
    if(Proj->Proc[Num].Endp_Num!=0)
    {
        Proj->Proc[Num].Endp=Malloc(sizeof(struct Endp_Info)*Proj->Proc[Num].Endp_Num);
        if(Proj->Proc[Num].Endp==0)
            EXIT_FAIL("The endpoint structure allocation failed.");
        for(Count=0;Count<Proj->Proc[Num].Endp_Num;Count++)
        {
            if(XML_Get_Next(Start, End, &Label_Start, &Label_End, &Val_Start, &Val_End)!=0)
                EXIT_FAIL("Unexpected error when parsing endpoint section.");
            Parse_Endpoint(Proj, Num, Count, Val_Start, Val_End);
        }
    }

    return 0;
}
/* End Function:Parse_Process ************************************************/

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
    cnt_t Count;
    s8* Label_Start;
    s8* Laben_End;
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
    Start++；
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
        Parse_Process(Proj, Count, Val_Start, Val_End);
    }

    return Proj;
}
/* End Function:Parse_Project ************************************************/

/* Begin Function:Parse_Chip_Memory *******************************************
Description : Parse the memory section of a particular chip.
Input       : struct Chip_Info* Chip - The project structure.
              cnt_t Num - The memory number.
              s8* Str_Start - The start position of the string.
              s8* Str_End - The end position of the string.
Output      : struct Chip_Info* Chip - The updated chip structure.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t Parse_Chip_Memory(struct Chip_Info* Chip, cnt_t Num, s8* Str_Start, s8* Str_End)
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

/* Begin Function:Parse_Option ************************************************
Description : Parse the option section of a particular chip.
Input       : struct Chip_Info* Chip - The project structure.
              cnt_t Num - The option number.
              s8* Str_Start - The start position of the string.
              s8* Str_End - The end position of the string.
Output      : struct Chip_Info* Chip - The updated chip structure.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t Parse_Option(struct Chip_Info* Chip, cnt_t Num, s8* Str_Start, s8* Str_End)
{
    s8* Start;
    s8* End;
    s8* Label_Start;
    s8* Label_End;
    s8* Val_Start;
    s8* Val_End;
    s8* Value_Temp;
    cnt_t Count;

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
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Macro");
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
            if(Start==',')
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
            if(Start==',')
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
/* End Function:Parse_Option *************************************************/

/* Begin Function:Parse_Vector ************************************************
Description : Parse the vector section of a particular chip.
Input       : struct Chip_Info* Chip - The project structure.
              cnt_t Num - The option number.
              s8* Str_Start - The start position of the string.
              s8* Str_End - The end position of the string.
Output      : struct Chip_Info* Chip - The updated chip structure.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t Parse_Vector(struct Chip_Info* Chip, cnt_t Num, s8* Str_Start, s8* Str_End)
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
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Vector");
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
/* Begin Function:Parse_Vector ************************************************

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
    cnt_t Count;
    s8* Label_Start;
    s8* Laben_End;
    s8* Val_Start;
    s8* Val_End;
    struct Chip_Info* Chip;
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
    Start++；
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
    Vectors_Start=Val_Start;
    Vectors_End=Val_End;

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
        Parse_Option(Chip, Count, Val_Start, Val_End);
    }

    /* Vector */
    Start=Vector_Start;
    End=Vector_End;
    Chip->Vector_Num=XML_Num(Start, End);
    if(Chip->Vector_Num==0)
        EXIT_FAIL("The option section is malformed.");
    Chip->Vector=Malloc(sizeof(struct Option_Info)*Chip->Vector_Num);
    if(Chip->Vector==0)
        EXIT_FAIL("The option structure allocation failed.");
    for(Count=0;Count<Chip->Vector_Num;Count++)
    {
        if(XML_Get_Next(Start, End, &Label_Start, &Label_End, &Val_Start, &Val_End)!=0)
            EXIT_FAIL("Unexpected error when parsing option section.");
        Parse_Vector(Chip, Count, Val_Start, Val_End);
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
    cnt_t Proc_Cnt;
    cnt_t Mem_Cnt;

    for(Proc_Cnt=0;Proc_Cnt<Proj->Proc_Num;Proc_Cnt++)
    {
        for(Mem_Cnt=0;Mem_Cnt<Proj->Proc[Proc_Cnt].Mem_Num;Mem_Cnt++)
        {
            if(Align(&(Proj->Proc[Proc_Cnt].Mem[Mem_Cnt])!=0)
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
    cnt_t Pos;
    cnt_t End;

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
    for(End--;End>=Pos;End--)
        Array[End+1]=Array[End];
    
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
    cnt_t Count;

    for(Count=0;Count<Size;Count++)
    {
        if((Bitmap[(Start+Count)/8]&(1<<((Start+Count)%8)))!=0)
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
    cnt_t Count;

    for(Count=0;Count<Size;Count++)
        Bitmap[(Start+Count)/8]|=1<<((Start+Count)%8);
}
/* End Function:Mark_Bitmap **************************************************/

/* Begin Function:Populate_Mem ************************************************
Description : Populate the memory data structure with this memory segment.
Input       : struct Mem_Map* Map - The memory map.
              ptr_t Start - The start address of the memory.
              ptr_t Size - The size of the memory.
Output      : struct Mem_Map* Map - The updated memory map.
Return      : ret_t - If successful, 0; else -1.
******************************************************************************/
ret_t Populate_Mem(struct Mem_Map* Map, ptr_t Start, ptr_t Size)
{
    cnt_t Mem_Cnt;
    ptr_t Rel_Start;
    cnt_t Mark_Start;
    cnt_t Mark_End;

    for(Mem_Cnt=0;Mem_Cnt<Map->Mem_Num;Mem_Cnt++)
    {
        if((Start>=Map->Mem_Array[Mem_Cnt]->Start)&&
           (Start<Map->Mem_Array[Mem_Cnt]->Start+Map->Mem_Array[Mem_Cnt]->Size))
            break;
    }

    /* Must be in this segment. See if we can fit there */
    if(Mem_Cnt==Map->Mem_Num)
        return -1;
    if((Map->Mem_Array[Mem_Cnt]->Start+Map->Mem_Array[Mem_Cnt]->Size)<(Start+Size))
        return -1;
    
    /* It is clear that we can fit now. Mark all the bits */
    Rel_Start=Start-Map->Mem_Array[Mem_Cnt]->Start;
    Mark_Bitmap(Rel_Start/4,Size/4);
    return 0;
}
/* End Function:Populate_Mem *************************************************/

/* Begin Function:Fit_Mem *****************************************************
Description : Fit the auto-placed memory segments to a fixed location.
Input       : struct Mem_Map* Map - The memory map.
              cnt_t Mem_Num - The memory info number in the process memory array.
Output      : struct Mem_Map* Map - The updated memory map.
Return      : ret_t - If successful, 0; else -1.
******************************************************************************/
ret_t Fit_Mem(struct Mem_Map* Map, cnt_t Mem_Num)
{
    cnt_t Fit_Cnt;
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

/* Begin Function:Alloc_Code **************************************************
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
    cnt_t Count;
    cnt_t Proc_Cnt;
    cnt_t Mem_Cnt;
    struct Mem_Map* Map;

    if((Type!=MEM_CODE)&&(Type!=MEM_DATA))
        EXIT_FAIL("Wrong fitting type.");

    Map=Malloc(sizeof(struct Mem_Map));
    if(Map==0)
        EXIT_FAIL("Memory map allocation failed.");

    /* Find all code memory sections */
    Count=0;
    for(Mem_Cnt=0;Mem_Cnt<Chip->Mem_Num;Mem_Cnt++)
    {
        if(Chip->Mem[Mem_Cnt].Type==Type)
            Count++;
    }

    Map->Mem_Num=Count;
    Map->Mem_Array=Malloc(sizeof(struct Mem_Info*)*Count);
    if(Map->Mem_Array==0)
        EXIT_FAIL("Memory map allocation failed.");
    memset(Map->Mem_Array,0,sizeof(struct Mem_Info*)*Count)
    Map->Mem_Bitmap=Malloc(sizeof(s8*)*Mem_Cnt);
    if(Map->Mem_Bitmap==0)
        EXIT_FAIL("Memory map allocation failed.");
    
    /* Insert sort according to the start address */
    for(Mem_Cnt=0;Mem_Cnt<Chip->Mem_Num;Mem_Cnt++)
    {
        if(Chip->Mem[Mem_Cnt].Type==Type)
        {
            if(Insert_Mem(Map->Mem_Array,&Chip->Mem[Mem_Cnt],0)!=0)
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
        memset(Map->Mem_Bitmap[Mem_Cnt],0,(Map->Mem_Array[Mem_Cnt]->Size/4)+1);
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
        if(Populate_Mem(Map, Proj->RME.Code_Start,Proj->RME.Data_Size)!=0)
            EXIT_FAIL("Invalid address designated.");
        if(Populate_Mem(Map, Proj->RME.Code_Start+Proj->RME.Data_Size,Proj->RVM.Data_Size)!=0)
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

    Map->Proc_Mem_Num=Count;
    Map->Proc_Mem_Array=Malloc(sizeof(struct Mem_Info*)*Count);
    if(Map->Proc_Mem_Array==0)
        EXIT_FAIL("Memory map allocation failed.");

    /* Insert sort according to size */
    for(Proc_Cnt=0;Proc_Cnt<Proj->Proc_Num;Proc_Cnt++)
    {
        for(Mem_Cnt=0;Mem_Cnt<Proj->Proc[Proc_Cnt].Mem_Num;Mem_Cnt++)
        {
            if(Proj->Proc[Proc_Cnt].Mem[Mem_Cnt].Type==Type)
            {
                if(Proj->Proc[Proc_Cnt].Mem[Mem_Cnt].Start==AUTO)
                {
                    if(Insert_Mem(Map->Proc_Mem_Array,&(Proj->Proc[Proc_Cnt].Mem[Mem_Cnt]),1)!=0)
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

    /* Clean up before returning */
    for(Mem_Cnt=0;Mem_Cnt<Map->Mem_Num;Mem_Cnt++)
        Free(Map->Mem_Bitmap[Mem_Cnt]);
    for(Mem_Cnt=0;Mem_Cnt<Map->Proc_Mem_Num;Mem_Cnt++)
        Free(Map->Proc_Mem_Bitmap[Mem_Cnt]);
    
    Free(Map->Mem_Array);
    Free(Map->Mem_Bitmap);
    Free(Map->Proc_Mem_Array);
    Free(Map);
}
/* End Function:Alloc_Mem ****************************************************/

/* CMX Toolset ***************************************************************/
ret_t CMX_Align(struct Mem_Info* Mem);
void CMX_Gen_Proj(struct Proj_Info* Proj, struct Chip_Info* Chip);

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
	cnt_t Output_Type;
	/* The input buffer */
	s8* Input_Buf=0;
	/* The file handle */
	FILE* File_Handle;
	ptr_t File_Size;
	/* The project and chip pointers */
	struct Proj_Info* Proj;
	struct Chip_Info* Chip;

	/* Initialize memory pool */
	List_Crt(&Mem_List);

    /* Process the command line first */
    Cmdline_Proc(argc,argv, &Input_Path, &Output_Path, &RME_Path, &RVM_Path, &Output_Type);

	/* Read the project contents */
	Input_Buf=Read_File(Input_Path);
	Proj=Parse_Project(Input_Buf);
	Free(Input_Buf);

	/* Parse the chip in a platform-agnostic way */
	Input_Buf=Read_File(0);
	Chip=Parse_Chip(Input_Buf);
	Free(Input_Buf);

	/* Align memory to what it should be */
	switch(0)
	{
	    case PLAT_CMX:Align_Mem(Proj, CMX_Align); break;
		case PLAT_MIPS:EXIT_FAIL("MIPS not currently supported.");
		case PLAT_RISCV:EXIT_FAIL("RISC-V not currently supported.");
		case PLAT_TCORE:EXIT_FAIL("Tricore not currently supported.");
		default:EXIT_FAIL("Platform invalid, please check.");
	}

	/* Actually allocate the auto memory segments by fixing their start addresses */
	Alloc_Mem(Proj, MEM_CODE);
	Alloc_Mem(Proj, MEM_DATA);

	/* Everything prepared, call the platform specific generator to generate the project fit for compilation */
	switch (0)
	{
		case PLAT_CMX:CMX_Gen_Proj(Proj, Chip); break;
		case PLAT_MIPS:EXIT_FAIL("MIPS not currently supported.");
		case PLAT_RISCV:EXIT_FAIL("RISC-V not currently supported.");
		case PLAT_TCORE:EXIT_FAIL("Tricore not currently supported.");
		default:EXIT_FAIL("Platform invalid, please check.");
	}

	/* All done, free all memory and we quit */
	Free_All();
    return 0;
}
/* End Function:main *********************************************************/

/* Cortex-M (CMX) Toolset *****************************************************
This toolset is for Cortex-M. Specifically, this suits Cortex-M0+, Cortex-M1,
Cortex-M3, Cortex-M7. Cortex-M23 and Cortex-M33 support is still pending at the
moment.
******************************************************************************/

/* Defines *******************************************************************/

/* End Defines ***************************************************************/

/* Structs *******************************************************************/
/* Chip-specific macros */
struct CMX_Chip_Info
{
	s8* Macro_Name;
	s8* Macro_Val;
};
/* Enabled interrupt endpoints */
struct CMX_Vect_Info
{
	s8* Vect_Name;
	s8* Vect_Stat;
};

struct CMX_Pgtbl
{
    ptr_t Start_Addr;
    ptr_t Size_Order;
    ptr_t Num_Order;
    ptr_t Attr;
    /* Whether we have the 8 subregions mapped: 0 - not mapped 1 - mapped other - pointer to the next */
    struct CMX_Pgtbl* Mapping[8];
}

struct CMX_Proc
{
    /* The process information structure */
    struct Proc_Info* Proc;
    /* What leaf page tables are actually needed to conjure this memory map */
    cnt_t Leaf_Pgtbl_Num;
    struct CMX_Pgtbl* Leaf_Pgtbl;
    /* Done. make the higher-level page tables */
};

/* Cortex-M information */
struct CMX_Proj_Info
{
	cnt_t NVIC_Grouping;
	ptr_t Systick_Val;
    
    /* This alignment is really fucking. */
};
/* End Structs ***************************************************************/

/* Begin Function:CMX_Align ***************************************************
Description : Align the memory according to Cortex-M platform's requirements.
Input       : struct Mem_Info* Mem - The struct containing memory information.
Output      : struct Mem_Info* Mem - The struct containing memory information,
                                     with all memory size aligned.
Return      : ret_t - If the start address and size is acceptable, 0; else -1.
******************************************************************************/
ret_t CMX_Align(struct Mem_Info* Mem)
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
        Mem->Size=((Mem->Size-1)/Mem->Align)*Mem->Align;
    }
    
	return 0;
}
/* End Function:CMX_Align ****************************************************/

void CMX_Gen_Keil(void)
{

}

void CMX_Gen_Eclipse(void)
{

}

void CMX_Gen_Makefile(void)
{

}

/* Begin Function:CMX_Gen_Pgtbl ***********************************************
Description : Recursively construct the page table for the Cortex-M port. 
              This only works for ARMv7-M. The port for ARMv8-M is still
              in progress.
Input       : struct Mem_Info* Mem - The struct containing memory segments to fit
                                     into this level (and below).
              cnt_t Num - The number of memory segments to fit in.
              ptr_t Total_Max - The maximum total order of the page table, cannot
                                be exceeded when deciding the total order of
                                the page table.
Output      : None.
Return      : struct CMX_Pgtbl* - The page table structure returned.
******************************************************************************/
struct CMX_Pgtbl* CMX_Gen_Pgtbl(struct Mem_Info* Mem, cnt_t Num, ptr_t Total_Max)
{
    ptr_t Start;
    ptr_t End;
    cnt_t Count;
    cnt_t Mem_Cnt;
    cnt_t Total_Order;
    cnt_t Size_Order;
    cnt_t Num_Order;
    ptr_t Pivot_Addr;
    cnt_t Cut_Apart;
    ptr_t Page_Start;
    ptr_t Page_End;
    struct CMX_Pgtbl* Pgtbl;
    cnt_t Mem_Num;
    struct Mem_Info* Mem_List;

    /* Allocate the page table data structure */
    Pgtbl=Malloc(sizeof(struct CMX_Pgtbl));
    if(Pgtbl==0)
        EXIT_FAIL("Page table data structure allocation failed.");

    /* What ranges does these stuff cover? */
    Start=(ptr_t)(-1);
    End=0;
    for(Mem_Cnt=0;Mem_Cnt<Num;Mem_Cnt++)
    {
        if(Start>Mem[Mem_Cnt].Start)
            Start=Mem[Mem_Cnt].Start;
        if(End<(Mem[Mem_Cnt].Start+Mem[Mem_Cnt].Size))
            End=Mem[Mem_Cnt].Start+Mem[Mem_Cnt].Size;
    }
    
    /* Which power-of-2 box is this in? */
    Total_Order=0;
    while(1)
    {
        if(End<=((Start>>Total_Order)<<Total_Order)+(1<<Total_Order))
            break;
        Total_Order++;
    }
    /* If the total order less than 8, we wish to extend that to 8 */
    if(Total_Order<8)
        Total_Order=8;

    /* See if this will violate the extension limit */
    if(Total_Order>Total_Max)
        EXIT_FAIL("Memory segment too small, cannot find a reasonable placement.");
    
    Pgtbl->Start_Addr=(Start>>Total_Order)<<Total_Order;

    /* Can the memory segments get fully mapped in? If yes, there are two conditions
     * that must be met:
     * 1. There cannot be different access permissions in these memory segments.
     * 2. The memory start address and the size must be fully divisible by 1<<(Total_Order-3). */
    for(Mem_Cnt=0;Mem_Cnt<Num;Mem_Cnt++)
    {
        if(Mem[Mem_Cnt].Attr!=Mem[0].Attr)
            break;
        if(((Mem[Mem_Cnt].Start%(1<<(Total_Order-3)))!=0)||((Mem[Mem_Cnt].Size%(1<<(Total_Order-3)))!=0))
            break;
    }

    /* Is this directly mappable? */
    if(Mem_Cnt==Num)
    {
        /* Yes, we know that number order=3 */
        Num_Order=3;
    }
    else
    {
        /* Not directly mappable. What's the maximum number order that do not cut things apart? If we
        * are forced to cut things apart, we prefer the smallest number order, which is 2 */
        Cut_Apart=0;
        for(Num_Order=1;Num_Order<=3;Num_Order++)
        {
            for(Mem_Cnt=0;Mem_Cnt<Num;Mem_Cnt++)
            {
                for(Count=1;Count<(1<<Num_Order);Count++)
                {
                    Pivot_Addr=(End-Start)/(1<<Num_Order)*Count+Start;
                    if((Mem[Mem_Cnt].Start<Pivot_Addr)&&((Mem[Mem_Cnt].Start+Mem[Mem_Cnt].Size)>Pivot_Addr))
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

        if(Num_Order>1)
        {
            if(Cut_Apart!=0)
                Num_Order--;
        }
    }
    
    Size_Order=Total_Order-Num_Order;

    /* We already know the size and number order of this layer. Map whatever we can map, and 
     * postpone whatever we will have to postpone */
    for(Count=0;Count<(1<<Num_Order);Count++)
    {
        Page_Start=Start+Count*(1<<Size_Order);
        Page_End=Start+(Count+1)*(1<<Size_Order);

        Pgtbl->Mapping[Count]=0;
        /* Can this compartment be mapped? It can if there is one segment covering the range */
        for(Mem_Cnt=0;Mem_Cnt<Num;Mem_Cnt++)
        {
            if((Mem[Mem_Cnt].Start<=Page_Start)&&((Mem[Mem_Cnt].Start+Mem[Mem_Cnt].Size)>=Page_End))
            {
                Pgtbl->Mapping[Count]=1;
                Pgtbl->Attr=Mem[Mem_Cnt].Attr;
            }
        }

        if(Pgtbl->Mapping[Count]==0)
        {
            /* No pages mapped. See if any residue memory list are here */
#error make up the pages, see if there are residue in this range. If there is, create the residue list 
            if(residue)
                Pgtbl->Mapping[Count]=CMX_Gen_Pgtbl(Mem_List, Mem_Num, Size_Order);
        }
    }

    Pgtbl->Size_Order=Size_Order;
    Pgtbl->Num_Order=Num_Order;
    return Pgtbl;
}
/* End Function:CMX_Gen_Pgtbl ************************************************/

void CMX_Gen_Proj(struct Proj_Info* Proj, struct Chip_Info* Chip)
{
    Step1. Create top-level page table. What addresses will this access? What are the two ends?
    Map whatever can be mapped in (8), and pass down whatever that cannot be mapped in.
    Collect the next one. pass down.

    Until all are mapped in.

    Now we know the top-level's content. The top-level always get mapped in with 2 entries.

    We are sure that after each alignment run, each segment can only be 
    for each page table, we may need a series of things to actually implement it.... This is the very hard part.
    Page table setup.
    one memory segment can have many page tables. 
    This should be run for all processes.
    /* Now we are fully clear where the segments are. Need to construct the page table for
     * all these - that's the hard part.
     struct
    /* Further processing, decide the page table layout of everything, and some Cortex-M 
     * macro values */

    /* Check if everything else is valid. If yes, all error output is already done, and we
      will start setting up the project very soon. After this, no error should occur. */

	/* Create the folder first and copy all necessary files into whatever possible */

    /* Write boot-time creation script that creates the interrupt endpoints */

    /* Write the kernel script for sending interrupts to endpoints */

    /* Write boot-time scripts that creates everything else */

    /* Write scripts for all processes */

    /* We're done here. */

    /* Static things are done. What about dynamic things? */

    /* Simple - tweak at your own risk. */

    /* Call project generator to generate the project */
}

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
