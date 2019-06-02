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
                4.Generate the kernel object map. This now fleshes out all the kernel
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
	cnt_t Extra_Kmem;
	cnt_t Kmem_Order;
	cnt_t Kern_Prios;
	struct Raw_Info Plat_Raw;
	struct Raw_Info Chip_Raw;
};
/* RVM user-level library information */
struct RVM_Info
{
	struct Comp_Info Comp;
	cnt_t Captbl_Spare;
	cnt_t Recovery;
};
/* Memory segment information */
struct Mem_Info
{
	ptr_t Start;
	ptr_t Size;
	cnt_t Type;
	ptr_t Attr;
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
/* Chip macro informations */
struct Macro_Info
{
	s8* Name;
	cnt_t Type;
	s8* Macro_Name;
	s8* Range;
};
/* Vector informations */
struct Vect_Info
{
	s8* Name;
	s8* Vect_Name;
};
/* Chip information - this is platform independent as well */
struct Chip_Info
{
	s8* Name;
	cnt_t Plat_Type;
	cnt_t Cores;
	cnt_t Mem_Num;
	struct Mem_Info* Mem;
	cnt_t Macro_Num;
	struct Macro_Info* Macro;
	cnt_t Vect_Num;
	struct Vect_Info* Vect;
};

/* Platform specific structures */
/* Cortex-M */
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
/* Cortex-M information */
struct CMX_Info
{
	cnt_t NVIC_Grouping;
	ptr_t Systick_Val;
	cnt_t Chip_Info_Num;
	struct CMX_Chip_Info* Chip_Info;
	cnt_t Vect_Info_Num;
	struct CMX_Vect_Info* Vect_Info;
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
			  s8** Root_Path - The root folder path, must contain RME files.
			  cnt_t* Output_Type - The output type.
Return      : None.
******************************************************************************/
void Cmdline_Proc(int argc,char* argv[], s8** Input_File, s8** Output_Path, s8** Root_Path, cnt_t* Output_Type)
{
    cnt_t Count;

    if(argc!=9)
        EXIT_FAIL("Too many or too few input parameters.\n"
                  "Usage: -i input.xml -o output_path -r rme_root -f format.\n"
                  "       -i: Project description file name and path, with extension.\n"
                  "       -o: Output path, must be empty.\n"
                  "       -r: RME root path, must contain all necessary files.\n"
                  "       -f: Output file format.\n"
                  "           keil: Keil uVision IDE.\n"
                  "           eclipse: Eclipse IDE.\n"
                  "           makefile: Makefile project.\n");

	*Input_File=0;
	*Output_Path=0;
	*Root_Path=0;
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
        else if(strcmp(argv[Count],"-r")==0)
        {
            if(*Root_Path!=0)
                EXIT_FAIL("More than one output path.");

            *Root_Path=argv[Count+1];
            if(Dir_Present(*Root_Path)!=0)
                EXIT_FAIL("Root path is not present.");
            if(Dir_Empty(*Root_Path)==0)
                EXIT_FAIL("Root path is empty, wrong path selected.");

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
    if(*Root_Path==0)
        EXIT_FAIL("No RME root path specified.");
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
    return 0;
}
/* End Function:Parse_RVM ****************************************************/

/* Begin Function:Parse_Mem ***************************************************
Description : Parse the memory section of a particular process.
Input       : struct Proj_Info* Proj - The project structure.
              cnt_t Proc_Num - The process number.
              cnt_t Mem_Num - The memory number.
              s8* Str_Start - The start position of the string.
              s8* Str_End - The end position of the string.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t Parse_Mem(struct Proj_Info* Proj, cnt_t Proc_Num, cnt_t Mem_Num, s8* Str_Start, s8* Str_End)
{
    s8* Start;
    s8* End;
    s8* Label_Start;
    s8* Label_End;
    s8* Val_Start;
    s8* Val_End;
    s8* Attr_Temp;
    cnt_t Count;

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
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Attr");
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
/* End Function:Parse_Mem ****************************************************/

/* Begin Function:Parse_Thd ***************************************************
Description : Parse the thread section of a particular process.
Input       : struct Proj_Info* Proj - The project structure.
              cnt_t Proc_Num - The process number.
              cnt_t Thd_Num - The thread number.
              s8* Str_Start - The start position of the string.
              s8* Str_End - The end position of the string.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t Parse_Thd(struct Proj_Info* Proj, cnt_t Proc_Num, cnt_t Thd_Num, s8* Str_Start, s8* Str_End)
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
/* End Function:Parse_Thd ****************************************************/

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
    s8* Memories_Start;
    s8* Memories_End;
    s8* Threads_Start;
    s8* Threads_End;
    s8* Invocations_Start;
    s8* Invocations_End;
    s8* Ports_Start;
    s8* Ports_End;
    s8* Endpoints_Start;
    s8* Endpoints_End;

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
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Memories");
    Memories_Start=Val_Start;
    Memories_End=Val_End;
    /* Threads section */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Threads");
    Threads_Start=Val_Start;
    Threads_End=Val_End;
    /* Invocations section */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Invocations");
    Invocations_Start=Val_Start;
    Invocations_End=Val_End;
    /* Ports section */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Ports");
    Ports_Start=Val_Start;
    Ports_End=Val_End;
    /* Endpoints section */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Endpoints");
    Endpoints_Start=Val_Start;
    Endpoints_End=Val_End;

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

    /* Parse memories section */
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
        Parse_Memory(Proj, Num, Count, Val_Start, Val_End);
    }

    /* Parse threads section */
    Start=Threads_Start;
    End=Threads_End;
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
    Start=Invocations_Start;
    End=Invocations_End;
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
    Start=Ports_Start;
    End=Ports_End;
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
    Start=Endpoints_Start;
    End=Endpoints_End;
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
            Parse_Endpoing(Proj, Num, Count, Val_Start, Val_End);
        }
    }

    return 0;
}
/* End Function:Parse_Process ************************************************/

/* Begin Function:Parse_Application *******************************************
Description : Parse the application section in the configuration file. An application
              is a set of processes.
Input       : struct Proj_Info* Proj - The project structure.
              s8* Str_Start - The start position of the string.
              s8* Str_End - The end position of the string.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : ret_t - Always 0.
******************************************************************************/
ret_t Parse_Application(struct Proj_Info* Proj, s8* Str_Start, s8* Str_End)
{
    s8* Proc_Start;
    s8* Proc_End;
    s8* Start;
    s8* End;
    s8* Label_Start;
    s8* Label_End;
    s8* Val_Start;
    s8* Val_End;
    cnt_t Count;

    /* We need to figure out how many projects are there and deal with them one by one */
    Start=Str_Start;
    End=Str_End;
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

    return 0;
}
/* End Function:Parse_Application ********************************************/

/* Begin Function:Parse_Proj **************************************************
Description : Parse the project description file, and fill in the struct.
Input       : s8* Proj_File - The buffer containing the project file contents.
Output      : None.
Return      : struct Proj_Info* - The struct containing the project information.
******************************************************************************/
struct Proj_Info* Parse_Proj(s8* Proj_File)
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
    /* Project */
    GET_NEXT_LABEL(Start, End, Label_Start, Label_End, Val_Start, Val_End, "Process");
    if(Parse_Application(Proj, Val_Start, Val_End)!=0)
        EXIT_FAIL("Pricess section parsing failed.");

    return Proj;
}
/* End Function:Parse_Proj ***************************************************/

/* Begin Function:Parse_Chip **************************************************
Description : Parse the chip description file, and fill in the struct.
Input       : s8* Proj_File - The buffer containing the chip file contents.
Output      : None.
Return      : struct Proj_Info* - The struct containing the chip information.
******************************************************************************/
struct Chip_Info* Parse_Chip(s8* Chip_File)
{

}
/* End Function:Parse_Chip ***************************************************/

/* Begin Function:Align_Mem ***************************************************
Description : Align the memory according to the platform's alignment functions.
Input       : struct Proj_Info* Proj - The struct containing the project information.
              ret_t (*Align)(struct Mem_Info*) - The platform's alignment function pointer.
Output      : struct Proj_Info* Proj - The struct containing the project information,
                                       with all memory size aligned.
Return      : None.
******************************************************************************/
void Align_Mem(struct Proj_Info* Proj, ret_t (*Align)(struct Mem_Info*))
{
	/* We have already parsed the whole stuff */

	/* Then bring up the architecture-specific parser */
}
/* End Function:Align_Mem ****************************************************/

/* Begin Function:Alloc_Mem ***************************************************
Description : Actually allocate all the memories that are undecided to their specific
              addresses. After this, all memories will have fixed address ranges.
Input       : struct Proj_Info* Proj - The struct containing the project information.
Output      : struct Proj_Info* Proj - The struct containing the project information,
                                       with all memory location allocated.
Return      : None.
******************************************************************************/
void Alloc_Mem(struct Proj_Info* Proj)
{

}
/* End Function:Alloc_Mem ****************************************************/

/* Cortex-M (CMX) Toolset *****************************************************
This toolset is for Cortex-M. Specifically, this suits Cortex-M0+, Cortex-M1,
Cortex-M3, Cortex-M7. Cortex-M23 and Cortex-M33 support is still pending at the
moment.
******************************************************************************/

/* Begin Function:CMX_Align ***************************************************
Description : Align the memory according to Cortex-M platform's requirements.
Input       : struct Mem_Info* Mem - The struct containing memory information.
Output      : struct Mem_Info* Mem - The struct containing memory information,
                                     with all memory size aligned.
Return      : ret_t - If the start address and size is acceptable, 0; else -1.
******************************************************************************/
ret_t CMX_Align(struct Mem_Info* Mem)
{
	return 0;
}
/* End Function:CMX_Align ****************************************************/

/* Why not consider embedding all the stuff inside this application binary blob and just... generate everything? might be a good idea for productivity.
It will be nice if we can put them together. However, this seems to be very hard. */

/* Hierachy: Project generator (OS generator, Driver generator, Middleware generator). Don't pack them; it is fine if we unpack on spot. */
void CMX_Gen_Keil(void)
{

}

void CMX_Gen_Eclipse(void)
{

}

void CMX_Gen_Makefile(void)
{

}

void CMX_Gen_Proj(struct Proj_Info* Proj, struct Chip_Info* Chip)
{
	/* Create the folder first and copy all necessary files into whatever possible */
}

/* RISC-V Toolset */

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
	s8* Root_Path;
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
    Cmdline_Proc(argc,argv, &Input_Path, &Output_Path, &Root_Path, &Output_Type);

	/* Read the project contents */
	Input_Buf=Read_File(Input_Path);
	Proj=Parse_Proj(Input_Buf);
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
	Alloc_Mem(Proj);

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

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
