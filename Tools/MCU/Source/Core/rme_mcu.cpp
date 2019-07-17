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
#include "sys/types.h"
#include "sys/stat.h"

#include "xml.h"
#include "pbfs.h"

#if(defined _MSC_VER)
#include "Windows.h"
#include "shlwapi.h"
#elif(defined linux)
#include <dirent.h>
#include <errno.h>
#else
#error "The target platform is not supported. Please compile on Windows or Linux."
#endif

#define __HDR_DEFS__
#include "rme_mcu.h"
#include "rme_a7m.h"
#undef __HDR_DEFS__

#define __HDR_STRUCTS__
#include "rme_mcu.h"
#include "rme_a7m.h"
#undef __HDR_STRUCTS__

/* Private include */
#include "rme_mcu.h"

#define __HDR_PUBLIC_MEMBERS__
#include "rme_a7m.h"
#undef __HDR_PUBLIC_MEMBERS__
/* End Includes **************************************************************/

/* Begin Function:Memcpy ******************************************************
Description : Memcpy wrapper for 64-bit XML library.
Input       : void* Src - The source string.
              xml_ptr_t Num - The number to copy.
Output      : void* Dst - The destination string.
Return      : void* - The destination is returned.
******************************************************************************/
void* Memcpy(void* Dst, void* Src, xml_ptr_t Num)
{
    return memcpy(Dst, Src, (size_t)Num);
}
/* End Function:Memcpy *******************************************************/

/* Begin Function:Strncmp *****************************************************
Description : Strncmp wrapper for 64-bit XML library.
Input       : s8_t* Str1 - The first string.
              s8_t* Str2 - The second string.
              ptr_t Num - The number of characters to compare.
Output      : None.
Return      : ret_t - If Str1 is bigger, positive; if equal, 0; if Str2 is bigger,
                      negative.
******************************************************************************/
ret_t Strncmp(s8_t* Str1, s8_t* Str2, ptr_t Num)
{
    return strncmp(Str1,Str2,(size_t)Num);
}
/* End Function:Strncmp ******************************************************/

/* Begin Function:Strlen ******************************************************
Description : Strlen wrapper for 64-bit XML library.
Input       : s8_t* Str - The Input string.
Output      : None.
Return      : ptr_t - The length of the string.
******************************************************************************/
ptr_t Strlen(s8_t* Str)
{
    return strlen(Str);
}
/* End Function:Strlen *******************************************************/

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
        EXIT_FAIL("Memory allocation failed.");

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

/* Begin Function:Dir_Present *************************************************
Description : Figure out whether the directory is present.
Input       : s8_t* Path - The path to the directory.
Output      : None.
Return      : ret_t - 0 for present, -1 for non-present.
******************************************************************************/
ret_t Dir_Present(s8_t* Path)
{
#ifdef _MSC_VER
    u32_t Attr;
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
Description : Figure out whether the directory is empty. When using this function,
              the directory must be present.
Input       : s8_t* Path - The path to the directory.
Output      : None.
Return      : ret_t - 0 for empty, -1 for non-empty.
******************************************************************************/
ret_t Dir_Empty(s8_t* Path)
{
#ifdef _MSC_VER
    u32_t Attr;
    Attr=PathIsDirectoryEmpty(Path);
    if(Attr!=0)
        return 0;
    else
        return -1;
#else
    ptr_t Num;
    DIR* Dir;

    Dir=opendir(Path);
    if(Dir==0)
        return -1;

    while(1)
    {
        if(readdir(Dir)==0)
            break;

        Num++;
        if(Num>2)
        {
            closedir(Dir);
            return -1;
        }
    }

    closedir(Dir);
    return 0;
#endif
}
/* End Function:Dir_Empty ****************************************************/

/* Begin Function:Get_Size ****************************************************
Description : Get the size of the file. The file is known to be present somewhere.
Input       : s8_t* Path - The path of the file.
Output      : None.
Return      : ptr_t - The size of the file.
******************************************************************************/
ptr_t Get_Size(s8_t* Path)
{
    struct stat Buf;
    if(stat(Path,&Buf)!=0)
        EXIT_FAIL("Windows/Linux stat failed.");
    return Buf.st_size;
}
/* End Function:Get_Size *****************************************************/

/* Begin Function:Make_Dir ****************************************************
Description : Create a directory if it does not exist.
Input       : s8_t* Path - The path to the directory.
Output      : None.
Return      : ret_t - 0 for successful, -1 for failure.
******************************************************************************/
void Make_Dir(s8_t* Path)
{
    if(Dir_Present(Path)==0)
        return;

#ifdef _WIN32
    if(CreateDirectory(Path, NULL)!=0)
        return;
#else
    if(mkdir(Path, S_IRWXU)==0)
        return;
#endif

    EXIT_FAIL("Folder creation failed.");
}
/* End Function:Make_Dir *****************************************************/

/* Begin Function:Copy_File ***************************************************
Description : Copy a file from some position to another position. If the file
              exists, we need to overwrite it with the new files.
Input       : s8_t* Dst - The destination path.
              s8_t* Src - The source path.
Output      : None.
Return      : ret_t - 0 for successful, -1 for failure.
******************************************************************************/
void Copy_File(s8_t* Dst, s8_t* Src)
{
    FILE* Dst_File;
    FILE* Src_File;
    s8_t Buf[128];
    ptr_t Size;

    Src_File=fopen(Src, "rb");
    if(Src_File==0)
        EXIT_FAIL("Cannot open source file.");
    /* This will wipe the contents of the file */
    Dst_File=fopen(Dst, "wb");
    if(Dst_File==0)
        EXIT_FAIL("Cannot open destination file.");

    Size=fread(Buf, 1, 128, Src_File);
    while(Size!=0)
    {
        fwrite(Buf, 1, (size_t)Size, Dst_File);
        Size=fread(Buf, 1, 128, Src_File);
    }

    fclose(Src_File);
    fclose(Dst_File);
}
/* End Function:Copy_File ****************************************************/

/* Begin Function:Read_File ***************************************************
Description : Read the file content into a buffer.
Input       : s8_t* Path - The path to the file.
Output      : None.
Return      : u8_t* - The buffer returned.
******************************************************************************/
u8_t* Read_File(s8_t* Path)
{
    ptr_t Size;
    u8_t* Buf;
    FILE* File;

    Size=Get_Size(Path);
    Buf=Malloc(Size+1);

    File=fopen(Path, "rb");
    if(File==0)
        EXIT_FAIL("Cannot read file.");

    fread(Buf, 1, (size_t)Size, File);
    Buf[Size]='\0';

    return Buf;
}
/* End Function:Read_File ****************************************************/

/* Begin Function:Cmdline_Proc ************************************************
Description : Preprocess the input parameters, and generate a preprocessed
              instruction listing with all the comments stripped.
Input       : int argc - The number of arguments.
              char* argv[] - The arguments.
Output      : s8_t** Input_File - The input project file path.
              s8_t** Output_File - The output folder path, must be empty.
			  s8_t** RME_Path - The RME root folder path, must contain RME files.
			  s8_t** RVM_Path - The RME root folder path, must contain RME files.
			  s8_t** Format - The output format.
Return      : None.
******************************************************************************/
void Cmdline_Proc(int argc, char* argv[], s8_t** Input_File, s8_t** Output_Path,
                  s8_t** RME_Path, s8_t** RVM_Path, s8_t** Format)
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

/* Begin Function:Read_XML ****************************************************
Description : Read the content of the whole XML file into the buffer. This only
              reads the XML.
Input       : s8_t* Path - The path to the file.
Output      : None.
Return      : s8_t* - The buffer containing the file contents.
******************************************************************************/
s8_t* Read_XML(s8_t* Path)
{
	ptr_t Size;
	FILE* Handle;
	s8_t* Buf;
    
	Size=Get_Size(Path);

	Handle=fopen(Path,"r");
	if(Handle==0)
		EXIT_FAIL("Input file open failed.");


	Buf=Malloc(Size+1);
	fread(Buf, 1, (size_t)Size, Handle);

	fclose(Handle);
	Buf[Size]='\0';

	return Buf;
}
/* End Function:Read_XML *****************************************************/

/* Begin Function:Parse_Proj_RME **********************************************
Description : Parse the RME section of the user-supplied project configuration file.
Input       : struct RME_Info* RME - The RME information.
              xml_node_t* Node - The RME section's XML node.
Output      : struct RME_Info* RME - The updated RME information.
Return      : None.
******************************************************************************/
void Parse_Proj_RME(struct RME_Info* RME, xml_node_t* Node)
{
    xml_node_t* Compiler;
    xml_node_t* General;
    xml_node_t* Platform;
    xml_node_t* Chip;
    xml_node_t* Temp;
    struct Raw_Info* Raw;

    /* Compiler */
    if((XML_Child(Node,"Compiler",&Compiler)<0)||(Compiler==0))
        EXIT_FAIL("RME Complier section missing.");
    /* General */
    if((XML_Child(Node,"General",&General)<0)||(General==0))
        EXIT_FAIL("RME General section missing.");
    /* Platform */
    if((XML_Child(Node,"Platform",&Platform)<0)||(Platform==0))
        EXIT_FAIL("RME Platform section missing.");
    /* Chip */
    if((XML_Child(Node,"Chip",&Chip)<0)||(Chip==0))
        EXIT_FAIL("RME chip section missing.");

    /* Parse compiler section */
    Parse_Compiler(&(RME->Comp),Compiler);

    /* Parse general section */
    /* Code start address */
    if((XML_Child(General,"Code_Start",&Temp)<0)||(Temp==0))
        EXIT_FAIL("RME General Code_Start section missing.");
    if(XML_Get_Hex(Temp,&(RME->Code_Start))<0)
        EXIT_FAIL("RME General Code_Start is not a valid hex integer.");
    /* Code size */
    if((XML_Child(General,"Code_Size",&Temp)<0)||(Temp==0))
        EXIT_FAIL("RME General Code_Size section missing.");
    if(XML_Get_Hex(Temp,&(RME->Code_Size))<0)
        EXIT_FAIL("RME General Code_Size is not a valid hex integer.");
    /* Data start address */
    if((XML_Child(General,"Data_Start",&Temp)<0)||(Temp==0))
        EXIT_FAIL("RME General Data_Start section missing.");
    if(XML_Get_Hex(Temp,&(RME->Data_Start))<0)
        EXIT_FAIL("RME General Data_Start is not a valid hex integer.");
    /* Data size */
    if((XML_Child(General,"Data_Size",&Temp)<0)||(Temp==0))
        EXIT_FAIL("RME General Data_Size section missing.");
    if(XML_Get_Hex(Temp,&(RME->Data_Size))<0)
        EXIT_FAIL("RME General Data_Size is not a valid hex integer.");
    /* Stack size */
    if((XML_Child(General,"Stack_Size",&Temp)<0)||(Temp==0))
        EXIT_FAIL("RME General Stack_Size section missing.");
    if(XML_Get_Hex(Temp,&(RME->Stack_Size))<0)
        EXIT_FAIL("RME General Stack_Size is not a valid hex integer.");
    /* Extra Kmem */
    if((XML_Child(General,"Extra_Kmem",&Temp)<0)||(Temp==0))
        EXIT_FAIL("RME General Extra_Kmem section missing.");
    if(XML_Get_Hex(Temp,&(RME->Extra_Kmem))<0)
        EXIT_FAIL("RME General Extra_Kmem is not a valid hex integer.");
    /* Kmem_Order */
    if((XML_Child(General,"Kmem_Order",&Temp)<0)||(Temp==0))
        EXIT_FAIL("RME General Kmem_Order section missing.");
    if(XML_Get_Uint(Temp,&(RME->Kmem_Order))<0)
        EXIT_FAIL("RME General Kmem_Order is not a valid unsigned integer.");
    /* Priorities */
    if((XML_Child(General,"Kern_Prios",&Temp)<0)||(Temp==0))
        EXIT_FAIL("RME General Kern_Prios section missing.");
    if(XML_Get_Uint(Temp,&(RME->Kern_Prios))<0)
        EXIT_FAIL("RME General Kern_Prios is not a valid unsigned integer.");

    /* Now read the platform section */
    List_Crt(&(RME->Plat));
    if(XML_Child(Platform,0,&Temp)<0)
        EXIT_FAIL("Internal error.");
    while(Temp!=0)
    {
        Raw=Malloc(sizeof(struct Raw_Info));
        if(XML_Get_Tag(Temp,Malloc,&Raw->Tag)<0)
            EXIT_FAIL("RME Platform tag read failed.");
        if(XML_Get_Val(Temp,Malloc,&Raw->Val)<0)
            EXIT_FAIL("RME Platform value read failed.");
        
        List_Ins(&(Raw->Head),RME->Plat.Prev,&(RME->Plat));
        if(XML_Child(Platform,"",&Temp)<0)
            EXIT_FAIL("Internal error.");
    }

    /* Now read the chip section */
    List_Crt(&(RME->Chip));
    if(XML_Child(Chip,0,&Temp)<0)
        EXIT_FAIL("Internal error.");
    while(Temp!=0)
    {
        Raw=Malloc(sizeof(struct Raw_Info));
        if(XML_Get_Tag(Temp,Malloc,&Raw->Tag)<0)
            EXIT_FAIL("RME Chip tag read failed.");
        if(XML_Get_Val(Temp,Malloc,&Raw->Val)<0)
            EXIT_FAIL("RME Chip value read failed.");

        List_Ins(&(Raw->Head),RME->Chip.Prev,&(RME->Chip));
        if(XML_Child(Chip,"",&Temp)<0)
            EXIT_FAIL("Internal error.");
    }
}
/* End Function:Parse_Proj_RME ***********************************************/

/* Begin Function:Parse_Proj_RVM **********************************************
Description : Parse the RVM section of the user-supplied project configuration file.
Input       : struct RVM_Info* RVM - The RVM information.
              xml_node_t* Node - The RVM section's XML node.
Output      : struct RVM_Info* RVM - The updated RVM information.
Return      : None.
******************************************************************************/
void Parse_Proj_RVM(struct RVM_Info* RVM, xml_node_t* Node)
{
    /* We don't parse RVM now as the functionality is not supported */
    xml_node_t* Compiler;
    xml_node_t* General;
    xml_node_t* VMM;
    xml_node_t* Temp;

    /* Compiler */
    if((XML_Child(Node,"Compiler",&Compiler)<0)||(Compiler==0))
        EXIT_FAIL("RVM Complier section missing.");
    /* General */
    if((XML_Child(Node,"General",&General)<0)||(General==0))
        EXIT_FAIL("RVM General section missing.");
    /* VMM */
    if((XML_Child(Node,"VMM",&VMM)<0)||(VMM==0))
        EXIT_FAIL("RVM VMM section missing.");

    /* Parse Compiler section */
    Parse_Compiler(&(RVM->Comp),Compiler);

    /* Now read the contents of the General section */
    /* Code size */
    if((XML_Child(General,"Code_Size",&Temp)<0)||(Temp==0))
        EXIT_FAIL("RVM General Code_Size section missing.");
    if(XML_Get_Hex(Temp,&(RVM->Code_Size))<0)
        EXIT_FAIL("RVM General Code_Size is not a valid hex integer.");
    /* Data size */
    if((XML_Child(General,"Data_Size",&Temp)<0)||(Temp==0))
        EXIT_FAIL("RVM General Data_Size section missing.");
    if(XML_Get_Hex(Temp,&(RVM->Data_Size))<0)
        EXIT_FAIL("RVM General Data_Size is not a valid hex integer.");
    /* Stack size */
    if((XML_Child(General,"Stack_Size",&Temp)<0)||(Temp==0))
        EXIT_FAIL("RVM General Stack_Size section missing.");
    if(XML_Get_Hex(Temp,&(RVM->Stack_Size))<0)
        EXIT_FAIL("RVM General Stack_Size is not a valid hex integer.");
    /* Extra Captbl */
    if((XML_Child(General,"Extra_Captbl",&Temp)<0)||(Temp==0))
        EXIT_FAIL("RVM General Extra_Captbl section missing.");
    if(XML_Get_Uint(Temp,&(RVM->Extra_Captbl))<0)
        EXIT_FAIL("RVM General Extra_Captbl is not a valid unsigned integer.");
    /* Recovery */
    if((XML_Child(General,"Recovery",&Temp)<0)||(Temp==0))
        EXIT_FAIL("RVM General Recovery section missing.");
    if((Temp->XML_Val_Len==6)&&(strncmp(Temp->XML_Val,"Thread",6)==0))
        RVM->Recovery=RECOVERY_THD;
    else if((Temp->XML_Val_Len==7)&&(strncmp(Temp->XML_Val,"Process",7)==0))
        RVM->Recovery=RECOVERY_PROC;
    else if((Temp->XML_Val_Len==6)&&(strncmp(Temp->XML_Val,"System",6)==0))
        RVM->Recovery=RECOVERY_SYS;
    else
        EXIT_FAIL("RVM General Recovery option is malformed.");

    /* The VMM section is currently unused. We don't care about this now */
}
/* End Function:Parse_Proj_RVM ***********************************************/

/* Begin Function:Parse_Proj_Proc *********************************************
Description : Parse project process section.
Input       : struct Proj_Info* Proj - The project information.
              xml_node_t* Node - The process section's XML node.
Output      : struct Proj_Info* Proj - The updated process information.
Return      : None.
******************************************************************************/
void Parse_Proj_Proc(struct Proj_Info* Proj, xml_node_t* Node)
{
    xml_node_t* General;
    xml_node_t* Compiler;
    xml_node_t* Memory;
    xml_node_t* Thread;
    xml_node_t* Invocation;
    xml_node_t* Port;
    xml_node_t* Receive;
    xml_node_t* Send;
    xml_node_t* Vector;

    xml_node_t* Trunk;
    xml_node_t* Temp;
    struct Proc_Info* Proc;

    if(XML_Child(Node,0,&Trunk)<0)
        EXIT_FAIL("Internal error.");

    List_Crt(&(Proj->Proc));

    while(Trunk!=0)
    {
        Proc=Malloc(sizeof(struct Proc_Info));

        /* General */
        if((XML_Child(Trunk,"General",&General)<0)||(General==0))
            EXIT_FAIL("Process General section missing.");

        /* Compiler */
        if((XML_Child(Trunk,"Compiler",&Compiler)<0)||(Compiler==0))
            EXIT_FAIL("Process Compiler section missing.");

        /* Memory */
        if((XML_Child(Trunk,"Memory",&Memory)<0)||(Memory==0))
            EXIT_FAIL("Process Memory section missing.");

        /* Thread */
        if((XML_Child(Trunk,"Thread",&Thread)<0)||(Thread==0))
            EXIT_FAIL("Process Thread section missing.");

        /* Invocation */
        if((XML_Child(Trunk,"Invocation",&Invocation)<0)||(Invocation==0))
            EXIT_FAIL("Process Invocation section missing.");

        /* Port */
        if((XML_Child(Trunk,"Port",&Port)<0)||(Port==0))
            EXIT_FAIL("Process Port section missing.");

        /* Receive */
        if((XML_Child(Trunk,"Receive",&Receive)<0)||(Receive==0))
            EXIT_FAIL("Process Receive section missing.");

        /* Send */
        if((XML_Child(Trunk,"Send",&Send)<0)||(Send==0))
            EXIT_FAIL("Process Send section missing.");

        /* Vector */
        if((XML_Child(Trunk,"Vector",&Vector)<0)||(Vector==0))
            EXIT_FAIL("Process Vector section missing.");

        /* Parse General section */
        /* Name */
        if((XML_Child(General,"Name",&Temp)<0)||(Temp==0))
            EXIT_FAIL("Process Name section missing.");
        if(XML_Get_Val(Temp,Malloc,&(Proc->Name))<0)
            EXIT_FAIL("Internal error.");

        /* Extra Captbl */
        if((XML_Child(General,"Extra_Captbl",&Temp)<0)||(Temp==0))
            EXIT_FAIL("Process Extra_Captbl section missing.");
        if(XML_Get_Uint(Temp,&(Proc->Extra_Captbl))<0)
            EXIT_FAIL("Internal error.");

        /* Parse Compiler section */
        Parse_Compiler(&(Proc->Comp),Compiler);
        /* Parse Memory section */
        Parse_Proc_Mem(Proc,Memory);
        /* Parse Thread section */
        Parse_Proc_Thd(Proc,Thread);
        /* Parse Invocation section */
        Parse_Proc_Inv(Proc,Invocation);
        /* Parse Port section */
        Parse_Proc_Port(Proc,Port);
        /* Parse Receive section */
        Parse_Proc_Recv(Proc,Receive);
        /* Parse Send section */
        Parse_Proc_Send(Proc,Send);
        /* Parse Vector section */
        Parse_Proc_Vect(Proc,Vector);

        List_Ins(&(Proc->Head),Proj->Proc.Prev,&(Proj->Proc));

        if(XML_Child(Node,"",&Trunk)<0)
            EXIT_FAIL("Internal error.");
    }
}
/* End Function:Parse_Proj_Proc **********************************************/

/* Begin Function:Lower_Case **************************************************
Description : Convert the string to lower case.
Input       : s8_t* Str - The string.
Output      : s8_t* Str - The updated string.
Return      : None.
******************************************************************************/
void Lower_Case(s8_t* Str)
{
    ptr_t Count;

    Count=0;
    while(Str[Count]!='\0')
    {
        Str[Count]=tolower(Str[Count]);
        Count++;
    }
}
/* End Function:Lower_Case ***************************************************/

/* Begin Function:Parse_Proj **************************************************
Description : Parse the project description file, and fill in the struct.
Input       : s8_t* Proj_File - The buffer containing the project file contents.
Output      : None.
Return      : struct Proj_Info* - The struct containing the project information.
******************************************************************************/
struct Proj_Info* Parse_Proj(s8_t* Proj_File)
{
    xml_node_t* Node;
    xml_node_t* Temp;
    xml_node_t* RME;
    xml_node_t* RVM;
    xml_node_t* Process;
    struct Proj_Info* Proj;

    /* Allocate the project information structure */
    Proj=Malloc(sizeof(struct Proj_Info));

    /* Parse the XML content */
    if(XML_Parse(&Node,Proj_File)<0)
        EXIT_FAIL("Project XML is malformed.");
    if((Node->XML_Tag_Len!=7)||(strncmp(Node->XML_Tag,"Project",7)!=0))
        EXIT_FAIL("Project XML is malformed.");

    /* Name */
    if((XML_Child(Node,"Name",&Temp)<0)||(Temp==0))
        EXIT_FAIL("Project Name section missing.");
    if(XML_Get_Val(Temp,Malloc,&(Proj->Name))<0)
        EXIT_FAIL("Internal error.");

    /* Platform */
    if((XML_Child(Node,"Platform",&Temp)<0)||(Temp==0))
        EXIT_FAIL("Project Platform section missing.");
    if(XML_Get_Val(Temp,Malloc,&(Proj->Plat_Name))<0)
        EXIT_FAIL("Internal error.");

    /* Platform to lower case */
    if(XML_Get_Val(Temp,Malloc,&(Proj->Lower_Plat))<0)
        EXIT_FAIL("Internal error.");
    Lower_Case(Proj->Lower_Plat);

    /* Chip_Class */
    if((XML_Child(Node,"Chip_Class",&Temp)<0)||(Temp==0))
        EXIT_FAIL("Project Chip_Class section missing.");
    if(XML_Get_Val(Temp,Malloc,&(Proj->Chip_Class))<0)
        EXIT_FAIL("Internal error.");

    /* Chip_Full */
    if((XML_Child(Node,"Chip_Full",&Temp)<0)||(Temp==0))
        EXIT_FAIL("Project Chip_Full section missing.");
    if(XML_Get_Val(Temp,Malloc,&(Proj->Chip_Full))<0)
        EXIT_FAIL("Internal error.");

    /* RME */
    if((XML_Child(Node,"RME",&RME)<0)||(RME==0))
        EXIT_FAIL("Project RME section missing.");

    /* RVM */
    if((XML_Child(Node,"RVM",&RVM)<0)||(RVM==0))
        EXIT_FAIL("Project RVM section missing.");

    /* Process */
    if((XML_Child(Node,"Process",&Process)<0)||(Process==0))
        EXIT_FAIL("Project Process section missing.");

    /* Parse RME section */
    Parse_Proj_RME(&(Proj->RME),RME);
    /* Parse RVM section */
    Parse_Proj_RVM(&(Proj->RVM),RVM);
    /* Parse Process section */
    Parse_Proj_Proc(Proj,Process);

    /* Destroy XML DOM */
    if(XML_Del(Node)<0)
        EXIT_FAIL("Internal error.");

    return Proj;
}
/* End Function:Parse_Proj ***************************************************/

/* Begin Function:Parse_Chip_Mem **********************************************
Description : Parse the memory section of a particular chip.
Input       : struct Chip_Info* Chip - The chip information.
              xml_node_t* Node - The process section's XML node.
Output      : struct Chip_Info* Chip - The updated chip information.
Return      : None.
******************************************************************************/
void Parse_Chip_Mem(struct Chip_Info* Chip, xml_node_t* Node)
{
    xml_node_t* Trunk;
    xml_node_t* Temp;
    struct Mem_Info* Mem;

    if(XML_Child(Node,0,&Trunk)<0)
        EXIT_FAIL("Internal error.");

    List_Crt(&(Chip->Code));
    List_Crt(&(Chip->Data));
    List_Crt(&(Chip->Device));

    while(Trunk!=0)
    {
        Mem=Malloc(sizeof(struct Mem_Info));

        /* Start */
        if((XML_Child(Trunk,"Start",&Temp)<0)||(Temp==0))
            EXIT_FAIL("Chip Memory Start section missing.");
        if(XML_Get_Hex(Temp,&(Mem->Start))<0)
            EXIT_FAIL("Chip Memory Start is not a valid hex integer.");

        /* Size */
        if((XML_Child(Trunk,"Size",&Temp)<0)||(Temp==0))
            EXIT_FAIL("Chip Memory Size section missing.");
        if(XML_Get_Hex(Temp,&(Mem->Size))<0)
            EXIT_FAIL("Chip Memory Size is not a valid hex integer.");
        if(Mem->Size==0)
            EXIT_FAIL("Chip Memory Size cannot be zero.");
        if((Mem->Start+Mem->Size)>0x100000000ULL)
            EXIT_FAIL("Chip Memory Size out of bound.");

        /* Type */
        if((XML_Child(Trunk,"Type",&Temp)<0)||(Temp==0))
            EXIT_FAIL("Chip Memory Type section missing.");
        if((Temp->XML_Val_Len==4)&&(strncmp(Temp->XML_Val,"Code",4)==0))
            List_Ins(&(Mem->Head),Chip->Code.Prev,&(Chip->Code));
        else if((Temp->XML_Val_Len==4)&&(strncmp(Temp->XML_Val,"Data",4)==0))
            List_Ins(&(Mem->Head),Chip->Data.Prev,&(Chip->Data));
        else if((Temp->XML_Val_Len==6)&&(strncmp(Temp->XML_Val,"Device",6)==0))
            List_Ins(&(Mem->Head),Chip->Device.Prev,&(Chip->Device));
        else
            EXIT_FAIL("Chip Memory Type is malformed.");

        if(XML_Child(Node,"",&Trunk)<0)
            EXIT_FAIL("Internal error.");
    }
}
/* End Function:Parse_Chip_Mem ***********************************************/

/* Begin Function:Parse_Chip_Option *******************************************
Description : Parse the option section of a particular chip.
Input       : struct Chip_Info* Chip - The chip information.
              xml_node_t* Node - The option section's XML node.
Output      : struct Chip_Info* Chip - The updated chip information.
Return      : None.
******************************************************************************/
void Parse_Chip_Option(struct Chip_Info* Chip, xml_node_t* Node)
{
    xml_node_t* Trunk;
    xml_node_t* Temp;
    struct Chip_Option_Info* Option;

    if(XML_Child(Node,0,&Trunk)<0)
        EXIT_FAIL("Internal error.");

    List_Crt(&(Chip->Option));

    while(Trunk!=0)
    {
        Option=Malloc(sizeof(struct Chip_Option_Info));

        /* Name */
        if((XML_Child(Trunk,"Name",&Temp)<0)||(Temp==0))
            EXIT_FAIL("Chip Option Name section missing.");
        if(XML_Get_Val(Temp,Malloc,&(Option->Name))<0)
            EXIT_FAIL("Internal error.");

        /* Type */
        if((XML_Child(Trunk,"Type",&Temp)<0)||(Temp==0))
            EXIT_FAIL("Chip Option Type section missing.");
        if((Temp->XML_Val_Len==5)&&(strncmp(Temp->XML_Val,"Range",5)==0))
            Option->Type=OPTION_RANGE;
        else if((Temp->XML_Val_Len==6)&&(strncmp(Temp->XML_Val,"Select",6)==0))
            Option->Type=OPTION_SELECT;
        else
            EXIT_FAIL("Chip Option Type is malformed.");

        /* Macro */
        if((XML_Child(Trunk,"Macro",&Temp)<0)||(Temp==0))
            EXIT_FAIL("Chip Option Macro section missing.");
        if(XML_Get_Val(Temp,Malloc,&(Option->Macro))<0)
            EXIT_FAIL("Internal error.");

        /* Range */
        if((XML_Child(Trunk,"Range",&Temp)<0)||(Temp==0))
            EXIT_FAIL("Chip Option Range section missing.");
        if(XML_Get_Val(Temp,Malloc,&(Option->Range))<0)
            EXIT_FAIL("Internal error.");

        List_Ins(&(Option->Head),Chip->Option.Prev,&(Chip->Option));

        if(XML_Child(Node,"",&Trunk)<0)
            EXIT_FAIL("Internal error.");
    }
}
/* End Function:Parse_Chip_Option ********************************************/

/* Begin Function:Parse_Chip_Vect *********************************************
Description : Parse the vector section of a particular chip.
Input       : struct Chip_Info* Chip - The chip information.
              xml_node_t* Node - The vector section's XML node.
Output      : struct Chip_Info* Chip - The updated chip information.
Return      : None.
******************************************************************************/
void Parse_Chip_Vect(struct Chip_Info* Chip, xml_node_t* Node)
{
    xml_node_t* Trunk;
    xml_node_t* Temp;
    struct Chip_Vect_Info* Vect;

    if(XML_Child(Node,0,&Trunk)<0)
        EXIT_FAIL("Internal error.");

    List_Crt(&(Chip->Vect));

    while(Trunk!=0)
    {
        Vect=Malloc(sizeof(struct Chip_Vect_Info));

        /* Name */
        if((XML_Child(Trunk,"Name",&Temp)<0)||(Temp==0))
            EXIT_FAIL("Chip Vector Name section missing.");
        if(XML_Get_Val(Temp,Malloc,&(Vect->Name))<0)
            EXIT_FAIL("Internal error.");

        /* Number */
        if((XML_Child(Trunk,"Number",&Temp)<0)||(Temp==0))
            EXIT_FAIL("Chip Vector Number section missing.");
        if(XML_Get_Uint(Temp,&(Vect->Num))<0)
            EXIT_FAIL("Chip Vector Number is not an unsigned integer.");

        List_Ins(&(Vect->Head),Chip->Vect.Prev,&(Chip->Vect));

        if(XML_Child(Node,"",&Trunk)<0)
            EXIT_FAIL("Internal error.");
    }
}
/* End Function:Parse_Chip_Vect ********************************************/

/* Begin Function:Parse_Chip **************************************************
Description : Parse the chip description file, and fill in the struct.
Input       : s8_t* Chip_File - The buffer containing the chip file contents.
Output      : None.
Return      : struct Chip_Info* - The struct containing the chip information.
******************************************************************************/
struct Chip_Info* Parse_Chip(s8_t* Chip_File)
{
    xml_node_t* Attribute;
    xml_node_t* Memory;
    xml_node_t* Option;
    xml_node_t* Vector;
    xml_node_t* Node;
    xml_node_t* Temp;
    struct Raw_Info* Raw;
    struct Chip_Info* Chip;

    /* Allocate the project information structure */
    Chip=Malloc(sizeof(struct Chip_Info));
   
    /* Parse the XML content */
    if(XML_Parse(&Node,Chip_File)<0)
        EXIT_FAIL("Chip XML is malformed.");
    if((Node->XML_Tag_Len!=4)||(strncmp(Node->XML_Tag,"Chip",4)!=0))
        EXIT_FAIL("Chip XML is malformed.");

    /* Class */
    if((XML_Child(Node,"Class",&Temp)<0)||(Temp==0))
        EXIT_FAIL("Chip Class section missing.");
    if(XML_Get_Val(Temp,Malloc,&(Chip->Class))<0)
        EXIT_FAIL("Internal error.");

    /* Compatible */
    if((XML_Child(Node,"Platform",&Temp)<0)||(Temp==0))
        EXIT_FAIL("Chip Compatible section missing.");
    if(XML_Get_Val(Temp,Malloc,&(Chip->Compat))<0)
        EXIT_FAIL("Internal error.");

    /* Vendor */
    if((XML_Child(Node,"Vendor",&Temp)<0)||(Temp==0))
        EXIT_FAIL("Chip Vendor section missing.");
    if(XML_Get_Val(Temp,Malloc,&(Chip->Vendor))<0)
        EXIT_FAIL("Internal error.");

    /* Platform */
    if((XML_Child(Node,"Platform",&Temp)<0)||(Temp==0))
        EXIT_FAIL("Chip Platform section missing.");
    if(XML_Get_Val(Temp,Malloc,&(Chip->Plat))<0)
        EXIT_FAIL("Internal error.");

    /* Cores */
    if((XML_Child(Node,"Cores",&Temp)<0)||(Temp==0))
        EXIT_FAIL("Chip Cores section missing.");
    if(XML_Get_Uint(Temp,&(Chip->Cores))<0)
        EXIT_FAIL("Chip Cores is not an unsigned integer.");

    /* Regions */
    if((XML_Child(Node,"Regions",&Temp)<0)||(Temp==0))
        EXIT_FAIL("Chip Regions section missing.");
    if(XML_Get_Uint(Temp,&(Chip->Regions))<0)
        EXIT_FAIL("Chip Regions is not an unsigned integer.");
    
    /* Attribute */
    if((XML_Child(Node,"Attribute",&Attribute)<0)||(Attribute==0))
        EXIT_FAIL("Chip Attribute section missing.");

    /* Memory */
    if((XML_Child(Node,"Memory",&Memory)<0)||(Memory==0))
        EXIT_FAIL("Chip Memory section missing.");

    /* Option */
    if((XML_Child(Node,"Option",&Option)<0)||(Option==0))
        EXIT_FAIL("Chip Option section missing.");

    /* Vector */
    if((XML_Child(Node,"Vector",&Vector)<0)||(Vector==0))
        EXIT_FAIL("Chip Vector section missing.");

    /* Parse Attribute section */
    List_Crt(&(Chip->Attr));
    if(XML_Child(Attribute,0,&Temp)<0)
        EXIT_FAIL("Internal error.");
    while(Temp!=0)
    {
        Raw=Malloc(sizeof(struct Raw_Info));
        if(XML_Get_Tag(Temp,Malloc,&Raw->Tag)<0)
            EXIT_FAIL("Chip Attribute tag read failed.");
        if(XML_Get_Val(Temp,Malloc,&Raw->Val)<0)
            EXIT_FAIL("Chip Attribute value read failed.");

        List_Ins(&(Raw->Head),Chip->Attr.Prev,&(Chip->Attr));
        if(XML_Child(Attribute,"",&Temp)<0)
            EXIT_FAIL("Internal error.");
    }

    /* Parse Memory section */
    Parse_Chip_Mem(Chip,Memory);
    /* Parse Option section */
    Parse_Chip_Option(Chip,Option);
    /* Parse Vector section */
    Parse_Chip_Vect(Chip,Vector);

    /* Destroy XML DOM */
    if(XML_Del(Node)<0)
        EXIT_FAIL("Internal error.");

    return Chip;
}
/* End Function:Parse_Chip ***************************************************/

/* Begin Function:Try_Bitmap **************************************************
Description : See if this bitmap segment is already covered.
Input       : s8_t* Bitmap - The bitmap.
              ptr_t Start - The starting bit location.
              ptr_t Size - The number of bits.
Output      : None.
Return      : ret_t - If can be marked, 0; else -1.
******************************************************************************/
ret_t Try_Bitmap(s8_t* Bitmap, ptr_t Start, ptr_t Size)
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
Description : Actually mark this bitmap segment. Each bit is always 4 bytes.
Input       : s8_t* Bitmap - The bitmap.
              ptr_t Start - The starting bit location.
              ptr_t Size - The number of bits.
Output      : s8_t* Bitmap - The updated bitmap.
Return      : None.
******************************************************************************/
void Mark_Bitmap(s8_t* Bitmap, ptr_t Start, ptr_t Size)
{
    ptr_t Count;

    for(Count=0;Count<Size;Count++)
        Bitmap[(Start+Count)/8]|=POW2((Start+Count)%8);
}
/* End Function:Mark_Bitmap **************************************************/

/* Begin Function:Fit_Static_Mem **********************************************
Description : Populate the memory data structure with this memory segment.
              This operation will be conducted with no respect to whether this
              portion have been populated with someone else.
Input       : struct Mem_Map* Map - The memory map.
              ptr_t Start - The start address of the memory.
              ptr_t Size - The size of the memory.
Output      : struct Mem_Map* Map - The updated memory map.
Return      : ret_t - If successful, 0; else -1.
******************************************************************************/
ret_t Fit_Static_Mem(struct Mem_Map* Map, ptr_t Start, ptr_t Size)
{
    ptr_t Rel_Start;
    struct Mem_Info* Mem;
    struct Mem_Map_Info* Info;

    /* See if we can even find a segment that accomodates this */
    for(EACH(struct Mem_Map_Info*,Info,Map->Chip_Mem))
    {
        Mem=Info->Mem;
        if((Start>=Mem->Start)&&(Start<=(Mem->Start+Mem->Size-1)))
            break;
    }

    /* Must be in this segment. See if we can fit there */
    if(IS_HEAD(Info,Map->Chip_Mem))
        return -1;
    if((Mem->Start+Mem->Size-1)<(Start+Size-1))
        return -1;
    
    /* It is clear that we can fit now. Mark all the bits. We do not check it it
     * is already marked, because we allow overlapping. */
    Rel_Start=Start-Mem->Start;
    Mark_Bitmap(Info->Bitmap,Rel_Start/4,Size/4);
    return 0;
}
/* End Function:Fit_Static_Mem ***********************************************/

/* Begin Function:Fit_Auto_Mem ************************************************
Description : Fit the auto-placed memory segments to a fixed location.
Input       : struct Mem_Map* Map - The memory map.
              ptr_t Mem_Num - The memory info number in the process memory array.
Output      : struct Mem_Map* Map - The updated memory map.
Return      : ret_t - If successful, 0; else -1.
******************************************************************************/
ret_t Fit_Auto_Mem(struct Mem_Map* Map, struct Mem_Info* Mem)
{
    ptr_t Start_Addr;
    ptr_t End_Addr;
    ptr_t Try_Addr;
    ptr_t Bitmap_Start;
    ptr_t Bitmap_Size;
    struct Mem_Info* Fit;
    struct Mem_Map_Info* Info;

    /* Find somewhere to fit this memory trunk, and if found, we will populate it */
    for(EACH(struct Mem_Map_Info*,Info,Map->Chip_Mem))
    {
        Fit=Info->Mem;
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
            Bitmap_Size=Mem->Size/4;

            if(Try_Bitmap(Info->Bitmap,Bitmap_Start,Bitmap_Size)==0)
            {
                /* Found a fit */
                Mark_Bitmap(Info->Bitmap,Bitmap_Start,Bitmap_Size);
                Mem->Start=Try_Addr;
                return 0;
            }
        }
    }

    /* Can't find any fit */
    return -1;
}
/* End Function:Fit_Auto_Mem *************************************************/

/* Begin Function:Compare_Addr ************************************************
Description : When doing merge sort, compare the address of memory segments.
Input       : struct List* First - The first memory segment.
              struct List* Second - The second memory segment.
Output      : None.
Return      : ret_t - If First>Second, 1, if First<Second, -1; else 0.
******************************************************************************/
ret_t Compare_Addr(struct List* First, struct List* Second)
{
    struct Mem_Map_Info* First_Info;
    struct Mem_Map_Info* Second_Info;

    First_Info=(struct Mem_Map_Info*)First;
    Second_Info=(struct Mem_Map_Info*)Second;

    if((First_Info->Mem->Start)>(Second_Info->Mem->Start))
        return 1;
    else if((First_Info->Mem->Start)<(Second_Info->Mem->Start))
        return -1;

    return 0;
}
/* End Function:Compare_Addr *************************************************/

/* Begin Function:Compare_Size ************************************************
Description : When doing merge sort, compare the size of memory segments.
Input       : struct List* First - The first memory segment.
              struct List* Second - The second memory segment.
Output      : None.
Return      : ret_t - If First>Second, 1, if First<Second, -1; else 0.
******************************************************************************/
ret_t Compare_Size(struct List* First, struct List* Second)
{
    struct Mem_Map_Info* First_Info;
    struct Mem_Map_Info* Second_Info;

    First_Info=(struct Mem_Map_Info*)First;
    Second_Info=(struct Mem_Map_Info*)Second;

    if((First_Info->Mem->Size)>(Second_Info->Mem->Size))
        return 1;
    else if((First_Info->Mem->Size)<(Second_Info->Mem->Size))
        return -1;

    return 0;
}
/* End Function:Compare_Size *************************************************/

/* Begin Function:Merge_Sort **************************************************
Description : Merge sort the list according to a compare function.
Input       : struct List* List - The List containing the objects to be sorted.
Output      : struct List* List - The sorted list.
Return      : None.
******************************************************************************/
void Merge_Sort(struct List* List, ret_t (*Compare)(struct List*, struct List*))
{
    struct List Head;
    struct List* Trav;
    struct List* Insert;
    struct List* Temp;

    /* There are zero or one element in it, return immediately */
    if(List->Next==List->Prev)
        return;

    /* Half the list into two parts */
    List_Crt(&Head);
    Temp=0;
    Trav=List->Next;
    while(Trav!=List)
    {
        if(Temp==0)
        {
            Temp=Trav;
            Trav=Trav->Next;
            List_Del(Temp->Prev,Temp->Next);
            List_Ins(Temp,Head.Prev,&Head);
        }
        else
        {
            Temp=0;
            Trav=Trav->Next;
        }
    }

    /* Merge sort the two sections */
    Merge_Sort(List, Compare);
    Merge_Sort(&Head, Compare);

    /* Merge the Head into the List */
    Trav=List->Next;
    Insert=Head.Next;
    while((Trav!=List)&&(Insert!=&Head))
    {
        while(Compare(Insert,Trav)<0)
        {
            Temp=Insert;
            Insert=Insert->Next;
            List_Del(Temp->Prev,Temp->Next);
            List_Ins(Temp,Trav->Prev,Trav);
            if(Insert==&Head)
                break;
        }
        Trav=Trav->Next;
    }

    /* If there still are elements in Head, they must all be bigger than List */
    Insert=Head.Next;
    while(Insert!=&Head)
    {
        Temp=Insert;
        Insert=Insert->Next;
        List_Del(Temp->Prev,Temp->Next);
        List_Ins(Temp,List->Prev,List);
    }
}
/* End Function:Merge_Sort ***************************************************/

/* Begin Function:Alloc_Code **************************************************
Description : Allocate the code section of all processes.
Input       : struct Proj_Info* Proj - The struct containing the project information.
              struct Chip_Info* Chip - The struct containing the chip information.
Output      : struct Proj_Info* Proj - The struct containing the project information,
                                       with all memory location allocated.
Return      : None.
******************************************************************************/
void Alloc_Code(struct Proj_Info* Proj, struct Chip_Info* Chip)
{
    struct Mem_Map* Map;
    struct Mem_Info* Mem;
    struct Mem_Map_Info* Info;
    struct Proc_Info* Proc;

    Map=Malloc(sizeof(struct Mem_Map));

    /* Insert all memory trunks in a incremental order by address */
    List_Crt(&(Map->Chip_Mem));
    for(EACH(struct Mem_Info*,Mem,Chip->Code))
    {
        Info=Malloc(sizeof(struct Mem_Map_Info));
        Info->Mem=Mem;
        Info->Bitmap=Malloc((Mem->Size/4)+1);
        memset(Info->Bitmap,0,(size_t)((Mem->Size/4)+1));
        List_Ins(&(Info->Head),Map->Chip_Mem.Prev,&(Map->Chip_Mem));
    }
    Merge_Sort(&(Map->Chip_Mem),Compare_Addr);

    /* Now populate the RME & RVM sections - must be continuous */
    if(Fit_Static_Mem(Map, Proj->RME.Code_Start,Proj->RME.Code_Size)!=0)
        EXIT_FAIL("RME Code section is invalid.");
    if(Fit_Static_Mem(Map, Proj->RME.Code_Start+Proj->RME.Code_Size,Proj->RVM.Code_Size)!=0)
        EXIT_FAIL("RVM Code section is invalid.");

    /* Merge sort all processes's memory in according to their size */
    List_Crt(&(Map->Proc_Mem));
    for(EACH(struct Proc_Info*,Proc,Proj->Proc))
    {
        for(EACH(struct Mem_Info*,Mem,Proc->Code))
        {
            /* If this memory is not auto memory, we wait to deal with it */
            Info=Malloc(sizeof(struct Mem_Map_Info));
            Info->Mem=Mem;
            if(Info->Mem->Start!=AUTO)
            {
                if(Fit_Static_Mem(Map, Info->Mem->Start, Info->Mem->Size)!=0)
                    EXIT_FAIL("Process Code section is invalid.");
                Free(Info);
                continue;
            }
            /* No bitmap for such memory trunks waiting to be allocated */
            List_Ins(&(Info->Head),Map->Proc_Mem.Prev,&(Map->Proc_Mem));
        }
    }
    Merge_Sort(&(Map->Proc_Mem),Compare_Size);

    /* Fit whatever that does not have a fixed address */
    for(EACH(struct Mem_Map_Info*,Info,Map->Proc_Mem))
    {
        if(Fit_Auto_Mem(Map, Info->Mem)!=0)
            EXIT_FAIL("Process Code memory fitter failed.");
    }

    /* Clean up before returning */
    while(!IS_HEAD(Map->Chip_Mem.Next,Map->Chip_Mem))
    {
        Info=(struct Mem_Map_Info*)(Map->Chip_Mem.Next);
        List_Del(Info->Head.Prev,Info->Head.Next);
        Free(Info->Bitmap);
        Free(Info);
    }

    while(!IS_HEAD(Map->Proc_Mem.Next,Map->Proc_Mem))
    {
        Info=(struct Mem_Map_Info*)(Map->Proc_Mem.Next);
        List_Del(Info->Head.Prev,Info->Head.Next);
        Free(Info);
    }
   
    Free(Map);
}
/* End Function:Alloc_Code ***************************************************/

/* Begin Function:Check_Code **************************************************
Description : Check if the code layout is valid.
Input       : struct Proj_Info* Proj - The struct containing the project information.
              struct Chip_Info* Chip - The struct containing the chip information.
Output      : None.
Return      : None.
******************************************************************************/
void Check_Code(struct Proj_Info* Proj, struct Chip_Info* Chip)
{
    struct Mem_Info* Mem1;
    struct Mem_Info* Mem2;
    struct Mem_Map_Info* Info;
    struct Mem_Map* Map;
    struct Proc_Info* Proc;

    Map=Malloc(sizeof(struct Mem_Map));
    List_Crt(&(Map->Proc_Mem));

    /* Insert all primary code sections into that list and then sort */
    for(EACH(struct Proc_Info*,Proc,Proj->Proc))
    {
        Info=Malloc(sizeof(struct Mem_Map_Info));
        Info->Mem=(struct Mem_Info*)(Proc->Code.Next);
        List_Ins(&(Info->Head),Map->Proc_Mem.Prev,&(Map->Proc_Mem));
    }

    Merge_Sort(&(Map->Proc_Mem),Compare_Addr);

    /* Check if adjacent memories will overlap */
    for(EACH(struct Mem_Map_Info*,Info,Map->Proc_Mem))
    {
        /* If we still have something after us, check for overlap */
        if(!IS_HEAD(Info->Head.Next,Map->Proc_Mem))
        {
            Mem1=Info->Mem;
            Mem2=((struct Mem_Map_Info*)(Info->Head.Next))->Mem;
            if((Mem1->Start+Mem1->Size)>Mem2->Start)
                EXIT_FAIL("Process primary Code section overlapped.");
        }
    }

    /* Clean up */
    while(!IS_HEAD(Map->Proc_Mem.Next,Map->Proc_Mem))
    {
        Info=(struct Mem_Map_Info*)(Map->Proc_Mem.Next);
        List_Del(Info->Head.Prev,Info->Head.Next);
        Free(Info);
    }

    Free(Map);
}
/* End Function:Check_Code ***************************************************/

/* Begin Function:Alloc_Data **************************************************
Description : Allocate the data section of all processes.
Input       : struct Proj_Info* Proj - The struct containing the project information.
              struct Chip_Info* Chip - The struct containing the chip information.
Output      : struct Proj_Info* Proj - The struct containing the project information,
                                       with all memory location allocated.
Return      : None.
******************************************************************************/
void Alloc_Data(struct Proj_Info* Proj, struct Chip_Info* Chip)
{
    struct Mem_Map* Map;
    struct Mem_Map_Info* Info;
    struct Proc_Info* Proc;
    struct Mem_Info* Mem;

    Map=Malloc(sizeof(struct Mem_Map));

    /* Insert all memory trunks in a incremental order by address */
    List_Crt(&(Map->Chip_Mem));
    for(EACH(struct Mem_Info*,Mem,Chip->Data))
    {
        Info=Malloc(sizeof(struct Mem_Map_Info));
        Info->Mem=Mem;
        Info->Bitmap=Malloc((Mem->Size/4)+1);
        memset(Info->Bitmap,0,(size_t)((Mem->Size/4)+1));
        List_Ins(&(Info->Head),Map->Chip_Mem.Prev,&(Map->Chip_Mem));
    }
    Merge_Sort(&(Map->Chip_Mem),Compare_Addr);

    /* Now populate the RME & RVM sections - must be continuous */
    if(Fit_Static_Mem(Map, Proj->RME.Data_Start,Proj->RME.Data_Size)!=0)
        EXIT_FAIL("RME Data section is invalid.");
    if(Fit_Static_Mem(Map, Proj->RME.Data_Start+Proj->RME.Data_Size,Proj->RVM.Data_Size)!=0)
        EXIT_FAIL("RVM Data section is invalid.");

    /* Merge sort all processes's memory in according to their size */
    List_Crt(&(Map->Proc_Mem));
    for(EACH(struct Proc_Info*,Proc,Proj->Proc))
    {
        for(EACH(struct Mem_Info*,Mem,Proc->Data))
        {
            /* If this memory is not auto memory, we wait to deal with it */
            Info=Malloc(sizeof(struct Mem_Map_Info));
            Info->Mem=Mem;
            if(Mem->Start!=AUTO)
            {
                if(Fit_Static_Mem(Map, Mem->Start, Mem->Size)!=0)
                    EXIT_FAIL("Process Data section is invalid.");
                Free(Info);
                continue;
            }
            /* No bitmap for such memory trunks waiting to be allocated */
            List_Ins(&(Info->Head),Map->Proc_Mem.Prev,&(Map->Proc_Mem));
        }
    }
    Merge_Sort(&(Map->Proc_Mem),Compare_Size);

    /* Fit whatever that does not have a fixed address */
    for(EACH(struct Mem_Map_Info*,Info,Map->Proc_Mem))
    {
        if(Fit_Auto_Mem(Map, Info->Mem)!=0)
            EXIT_FAIL("Process Data memory fitter failed.");
    }

    /* Clean up before returning */
    while(!IS_HEAD(Map->Chip_Mem.Next,Map->Chip_Mem))
    {
        Info=(struct Mem_Map_Info*)(Map->Chip_Mem.Next);
        List_Del(Info->Head.Prev,Info->Head.Next);
        Free(Info->Bitmap);
        Free(Info);
    }

    while(!IS_HEAD(Map->Proc_Mem.Next,Map->Proc_Mem))
    {
        Info=(struct Mem_Map_Info*)(Map->Proc_Mem.Next);
        List_Del(Info->Head.Prev,Info->Head.Next);
        Free(Info);
    }
   
    Free(Map);
}
/* End Function:Alloc_Data ***************************************************/

/* Begin Function:Check_Device ************************************************
Description : Check the device memory to make sure that all of them falls into range.
              There are faster algorithms; but we don't do such optimization now.
Input       : struct Proj_Info* Proj - The struct containing the project information.
              struct Chip_Info* Chip - The struct containing the chip information.
Output      : None.
Return      : None.
******************************************************************************/
void Check_Device(struct Proj_Info* Proj, struct Chip_Info* Chip)
{
    struct Mem_Info* Proc_Mem;
    struct Mem_Info* Chip_Mem;
    struct Proc_Info* Proc;

    /* Is it true that the device memory is in device memory range? */
    for(EACH(struct Proc_Info*,Proc,Proj->Proc))
    {
        for(EACH(struct Mem_Info*,Proc_Mem,Proc->Device))
        {
            for(EACH(struct Mem_Info*,Chip_Mem,Chip->Device))
            {
                if(Proc_Mem->Start>=Chip_Mem->Start)
                {
                    if((Proc_Mem->Start+Proc_Mem->Size)<=(Chip_Mem->Start+Chip_Mem->Size))
                        break;
                }
            }

            if(IS_HEAD(Chip_Mem,Chip->Device))
                EXIT_FAIL("Process Device segment is out of bound.");
        }
    }
}
/* End Function:Check_Device *************************************************/

/* Begin Function:Check_Input *************************************************
Description : Check if the input from XML is valid. If it is invalid, we abort
              immediately.
Input       : struct Proj_Info* Proj - The project information struct.
              struct Chip_Info* Chip - The chip information struct.
Output      : None.
Return      : None.
******************************************************************************/
void Check_Input(struct Proj_Info* Proj, struct Chip_Info* Chip)
{
    s8_t* Full_Pos;
    struct Proc_Info* Proc;
    struct Mem_Info* Mem;

    /* Check platform validity */
    if(strcmp(Proj->Plat_Name, Chip->Plat)!=0)
        EXIT_FAIL("Platform conflict.");

    /* Check chip class validity */
    if(strcmp(Proj->Chip_Class, Chip->Class)!=0)
        EXIT_FAIL("Chip class conflict.");

    /* Check chip full name validity */
    Full_Pos=strstr(Chip->Compat, Proj->Chip_Full);
    if(Full_Pos!=0)
    {
        if((Full_Pos[1]!=',')&&(Full_Pos[1]!='\0'))
            EXIT_FAIL("Chip full name not found.");
    }

    /* The chip shall have at least one code memory section and one data memory section */
    if(Chip->Code.Next==&(Chip->Code))
        EXIT_FAIL("Chip does not have a Code section.");
    if(Chip->Data.Next==&(Chip->Data))
        EXIT_FAIL("Chip does not have a Data section.");

    /* All Chip memory segments shall be aligned to 32 bytes */
    for(EACH(struct Mem_Info*,Mem,Chip->Code))
    {
        if((Mem->Size&0x1F)!=0)
            EXIT_FAIL("Chip Code section not aligned to 32-byte boundary.");
    }
    for(EACH(struct Mem_Info*,Mem,Chip->Data))
    {
        if((Mem->Size&0x1F)!=0)
            EXIT_FAIL("Chip Data section not aligned to 32-byte boundary.");
    }
    for(EACH(struct Mem_Info*,Mem,Chip->Device))
    {
        if((Mem->Size&0x1F)!=0)
            EXIT_FAIL("Chip Device section not aligned to 32-byte boundary.");
    }

    /* Every process must have at least one code and data segment, and they must be static. 
     * The primary code segment must be RXS, the primary data segment must allow RWS */
    for(EACH(struct Proc_Info*,Proc,Proj->Proc))
    {
        if(Proc->Code.Next==&(Proc->Code))
            EXIT_FAIL("Process does not have a Code section.");
        if(Proc->Data.Next==&(Proc->Data))
            EXIT_FAIL("Process does not have a Data section.");

        Mem=(struct Mem_Info*)(Proc->Code.Next);
        if(((Mem->Attr)&(MEM_READ|MEM_EXECUTE|MEM_STATIC))!=(MEM_READ|MEM_EXECUTE|MEM_STATIC))
            EXIT_FAIL("Process primary Code section does not have RXS attribute.");
        
        Mem=(struct Mem_Info*)(Proc->Data.Next);
        if(((Mem->Attr)&(MEM_READ|MEM_WRITE|MEM_STATIC))!=(MEM_READ|MEM_WRITE|MEM_STATIC))
            EXIT_FAIL("Process primary Data section does not have RWS attribute.");

        /* All process memory segments shall be aligned to 32 bytes */
        for(EACH(struct Mem_Info*,Mem,Proc->Code))
        {
            if((Mem->Size&0x1F)!=0)
                EXIT_FAIL("Process Code section not aligned to 32-byte boundary.");
        }
        for(EACH(struct Mem_Info*,Mem,Proc->Data))
        {
            if((Mem->Size&0x1F)!=0)
                EXIT_FAIL("Process Data section not aligned to 32-byte boundary.");
        }
        for(EACH(struct Mem_Info*,Mem,Proc->Device))
        {
            if((Mem->Size&0x1F)!=0)
                EXIT_FAIL("Process Device section not aligned to 32-byte boundary.");
        }

        /* All process shall have at least one thread */
        if(IS_HEAD(Proc->Thd.Next,Proc->Thd))
            EXIT_FAIL("Process does not have a Thread.");
    }
}
/* End Function:Check_Input **************************************************/

/* Begin Function:Strcicmp ****************************************************
Description : Compare two strings in a case insensitive way.
Input       : s8_t* Str1 - The first string.
              s8_t* Str2 - The second string.
Output      : None.
Return      : ret_t - If two strings are equal, then 0; if the first is bigger, 
                      then positive; else negative.
******************************************************************************/
ret_t Strcicmp(s8_t* Str1, s8_t* Str2)
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

/* Begin Function:Check_Name **************************************************
Description : See if the names are valid C identifiers.
Input       : struct Proj_Info* Proj - The project information struct.
Output      : None.
Return      : None.
******************************************************************************/
void Check_Name(s8_t* Name)
{
    ptr_t Count;
    /* Should not begin with number */
    if((Name[0]>='0')&&(Name[0]<='9'))
        EXIT_FAIL("Name is not a valid C identifier.");

    Count=0;
    while(1)
    {
        Count++;
        if(Name[Count]=='\0')
            return;
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
    
    EXIT_FAIL("Name is not a valid C identifier.");
}
/* End Function:Check_Name ***************************************************/

/* Begin Function:Check_Vect **************************************************
Description : Detect vector conflicts in the system. Every vector name is globally
              unique - it cannot overlap with any other vector in any other process.
              If one of the processes have a vector endpoint, then no other process
              can have the same one.
Input       : struct Proj_Info* Proj - The project information struct.
Output      : None.
Return      : None.
******************************************************************************/
void Check_Vect(struct Proj_Info* Proj)
{
    struct Proc_Info* Proc;
    struct Vect_Info* Vect;
    struct Proc_Info* Proc_Temp;
    struct Vect_Info* Vect_Temp;
    
    for(EACH(struct Proc_Info*,Proc,Proj->Proc))
    {
        for(EACH(struct Vect_Info*,Vect,Proc->Vect))
        {
            for(EACH(struct Proc_Info*,Proc_Temp,Proj->Proc))
            {
                for(EACH(struct Vect_Info*,Vect_Temp,Proc->Vect))
                {
                    if(Vect_Temp==Vect)
                        continue;
            
                    if(Strcicmp(Vect->Name, Vect_Temp->Name)==0)
                        EXIT_FAIL("Duplicate Vector endpoints are now allowed.");
                }
            }
        }
    } 
}
/* End Function:Check_Vect ***************************************************/

/* Begin Function:Check_Conflict **********************************************
Description : Detect namespace conflicts in the system. It also checks if the
              names are at least regular C identifiers.
Input       : struct Proj_Info* Proj - The project information struct.
Output      : None.
Return      : None.
******************************************************************************/
void Check_Conflict(struct Proj_Info* Proj)
{
    struct Proc_Info* Proc;
    struct Proc_Info* Proc_Temp;
    struct Thd_Info* Thd;
    struct Thd_Info* Thd_Temp;
    struct Inv_Info* Inv;
    struct Inv_Info* Inv_Temp;
    struct Port_Info* Port;
    struct Port_Info* Port_Temp;
    struct Recv_Info* Recv;
    struct Recv_Info* Recv_Temp;
    struct Send_Info* Send;
    struct Send_Info* Send_Temp;

    /* Are there two processes with the same name? */
    for(EACH(struct Proc_Info*,Proc,Proj->Proc))
    {
        /* Check for duplicate processes */
        Check_Name(Proc->Name);
        for(EACH(struct Proc_Info*,Proc_Temp,Proj->Proc))
        {
            if(Proc_Temp==Proc)
                continue;
            if(Strcicmp(Proc_Temp->Name,Proc->Name)==0)
                EXIT_FAIL("Duplicate Process name.");
        }

        /* Check for duplicate threads */
        for(EACH(struct Thd_Info*,Thd,Proc->Thd))
        {
            Check_Name(Thd->Name);
            for(EACH(struct Thd_Info*,Thd_Temp,Proc->Thd))
            {
                if(Thd_Temp==Thd)
                    continue;
                if(Strcicmp(Thd_Temp->Name,Thd->Name)==0)
                    EXIT_FAIL("Duplicate Thread name.");
            }
        }

        /* Check for duplicate invocations */
        for(EACH(struct Inv_Info*,Inv,Proc->Inv))
        {
            Check_Name(Inv->Name);
            for(EACH(struct Inv_Info*,Inv_Temp,Proc->Inv))
            {
                if(Inv_Temp==Inv)
                    continue;
                if(Strcicmp(Inv_Temp->Name,Inv->Name)==0)
                    EXIT_FAIL("Duplicate Invocation name.");
            }
        }

        /* Check for duplicate ports */
        for(EACH(struct Port_Info*,Port,Proc->Port))
        {
            Check_Name(Port->Name);
            Check_Name(Port->Proc_Name);
            if(Strcicmp(Port->Proc_Name,Proc->Name)==0)
                EXIT_FAIL("Port cannot target within the same Process.");
            for(EACH(struct Port_Info*,Port_Temp,Proc->Port))
            {
                if(Port_Temp==Port)
                    continue;
                if((Strcicmp(Port_Temp->Name,Port->Name)==0)&&
                   (Strcicmp(Port_Temp->Proc_Name,Port->Proc_Name)==0))
                    EXIT_FAIL("Duplicate Port name.");
            }
        }

        /* Check for duplicate receive endpoints */
        for(EACH(struct Recv_Info*,Recv,Proc->Recv))
        {
            Check_Name(Recv->Name);
            for(EACH(struct Recv_Info*,Recv_Temp,Proc->Recv))
            {
                if(Recv_Temp==Recv)
                    continue;
                if(Strcicmp(Recv_Temp->Name,Recv->Name)==0)
                    EXIT_FAIL("Duplicate Receive endpoint name.");
            }
        }

        /* Check for duplicate send endpoints */
        for(EACH(struct Send_Info*,Send,Proc->Send))
        {
            Check_Name(Send->Name);
            Check_Name(Send->Proc_Name);
            for(EACH(struct Send_Info*,Send_Temp,Proc->Send))
            {
                if(Send_Temp==Send)
                    continue;
                if((Strcicmp(Send_Temp->Name,Send->Name)==0)&&
                   (Strcicmp(Send_Temp->Proc_Name,Send->Proc_Name)==0))
                    EXIT_FAIL("Duplicate Send name.");
            }
        }
    }

    /* Check for duplicate vector endpoints- they are globally unique */
    Check_Vect(Proj);
}
/* End Function:Check_Conflict ***********************************************/

/* Begin Function:Alloc_Local_Capid *******************************************
Description : Allocate local capability IDs for all kernel objects. 
              Only ports, receive endpoints, send endpoints and vector endpoints
              have such ID.
Input       : struct Proj_Info* Proj - The project structure.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : None.
******************************************************************************/
void Alloc_Local_Capid(struct Proj_Info* Proj)
{
    ptr_t Capid;
    struct Proc_Info* Proc;
    struct Port_Info* Port;
    struct Recv_Info* Recv;
    struct Send_Info* Send;
    struct Vect_Info* Vect;

    for(EACH(struct Proc_Info*,Proc,Proj->Proc))
    {
        Capid=0;

        for(EACH(struct Port_Info*,Port,Proc->Port))
            Port->Cap.Loc_Capid=Capid++;

        for(EACH(struct Recv_Info*,Recv,Proc->Recv))
            Recv->Cap.Loc_Capid=Capid++;

        for(EACH(struct Send_Info*,Send,Proc->Send))
            Send->Cap.Loc_Capid=Capid++;

        for(EACH(struct Vect_Info*,Vect,Proc->Vect))
            Vect->Cap.Loc_Capid=Capid++;

        Proc->Captbl_Front=Capid;
    }
}
/* End Function:Alloc_Local_Capid ********************************************/

/* Begin Function:Alloc_Global_Capid ******************************************
Description : Allocate (relative) global capability IDs for all kernel objects. 
              Each global object will reside in its onw capability table. 
              This facilitates management, and circumvents the capability size
              limit that may present on 32-bit systems.
              How many distinct kernel objects are there? We just need to add up
              the following: All captbls (each process have one), all processes,
              all threads, all invocations, all receive endpoints. The ports and
              send endpoints do not have a distinct kernel object; the vector 
              endpoints are created by the kernel at boot-time, while the pgtbls
              are decided by architecture-specific code.
Input       : struct Proj_Info* Proj - The project structure.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : None.
******************************************************************************/
void Alloc_Global_Capid(struct Proj_Info* Proj)
{
    struct Proc_Info* Proc;
    struct Thd_Info* Thd;
    struct Inv_Info* Inv;
    struct Recv_Info* Recv;
    struct Vect_Info* Vect;
    struct RVM_Cap_Info* Cap;

    List_Crt(&(Proj->RVM.Captbl));
    List_Crt(&(Proj->RVM.Proc));
    List_Crt(&(Proj->RVM.Thd));
    List_Crt(&(Proj->RVM.Inv));
    List_Crt(&(Proj->RVM.Recv));
    List_Crt(&(Proj->RVM.Vect));

    Proj->RVM.Captbl_Front=0;
    Proj->RVM.Proc_Front=0;
    Proj->RVM.Thd_Front=0;
    Proj->RVM.Inv_Front=0;
    Proj->RVM.Recv_Front=0;
    Proj->RVM.Vect_Front=0;

    for(EACH(struct Proc_Info*,Proc,Proj->Proc))
    {
        /* Fill in all captbls and processes */
        Cap=Malloc(sizeof(struct RVM_Cap_Info));
        Cap->Proc=Proc;
        Cap->Cap=Proc;
        Proc->Captbl_Cap.RVM_Capid=Proj->RVM.Captbl_Front++;
        List_Ins(&(Cap->Head),Proj->RVM.Captbl.Prev,&(Proj->RVM.Captbl));
        
        Cap=Malloc(sizeof(struct RVM_Cap_Info));
        Cap->Proc=Proc;
        Cap->Cap=Proc;
        Proc->Proc_Cap.RVM_Capid=Proj->RVM.Proc_Front++;
        List_Ins(&(Cap->Head),Proj->RVM.Proc.Prev,&(Proj->RVM.Proc));

        /* Fill in all threads */
        for(EACH(struct Thd_Info*,Thd,Proc->Thd))
        {
            Cap=Malloc(sizeof(struct RVM_Cap_Info));
            Cap->Proc=Proc;
            Cap->Cap=Thd;
            Thd->Cap.RVM_Capid=Proj->RVM.Thd_Front++;
            List_Ins(&(Cap->Head),Proj->RVM.Thd.Prev,&(Proj->RVM.Thd));
        }

        /* Fill in all invocations */
        for(EACH(struct Inv_Info*,Inv,Proc->Inv))
        {
            Cap=Malloc(sizeof(struct RVM_Cap_Info));
            Cap->Proc=Proc;
            Cap->Cap=Inv;
            Inv->Cap.RVM_Capid=Proj->RVM.Inv_Front++;
            List_Ins(&(Cap->Head),Proj->RVM.Inv.Prev,&(Proj->RVM.Inv));
        }

        /* Fill in all receive endpoints */
        for(EACH(struct Recv_Info*,Recv,Proc->Recv))
        {
            Cap=Malloc(sizeof(struct RVM_Cap_Info));
            Cap->Proc=Proc;
            Cap->Cap=Recv;
            Recv->Cap.RVM_Capid=Proj->RVM.Recv_Front++;
            List_Ins(&(Cap->Head),Proj->RVM.Recv.Prev,&(Proj->RVM.Recv));
        }

        /* Fill in all vector endpoints */
        for(EACH(struct Vect_Info*,Vect,Proc->Vect))
        {
            Cap=Malloc(sizeof(struct RVM_Cap_Info));
            Cap->Proc=Proc;
            Cap->Cap=Vect;
            Vect->Cap.RVM_Capid=Proj->RVM.Vect_Front++;
            List_Ins(&(Cap->Head),Proj->RVM.Vect.Prev,&(Proj->RVM.Vect));
        }
    }
}
/* End Function:Alloc_Global_Capid *******************************************/

/* Begin Function:Make_Macro **************************************************
Description : Concatenate at most 4 parts into a macro, and turn everything uppercase.
Input       : s8_t* Str1 - The first part.
              s8_t* Str2 - The second part.
              s8_t* Str3 - The third part.
              s8_t* Str4 - The fourth part.
Output      : None.
Return      : s8_t* - The macro returned. This is allocated memory.
******************************************************************************/
s8_t* Make_Macro(s8_t* Str1, s8_t* Str2, s8_t* Str3, s8_t* Str4)
{
    ptr_t Count;
    ptr_t Len;
    s8_t* Ret;

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
              one will be allocated.
              The allocation table is shown below:
-------------------------------------------------------------------------------
Type            Local                           Global
-------------------------------------------------------------------------------
Process         -                               RVM_PROC_<PROCNAME>
-------------------------------------------------------------------------------
Captbl          -                               RVM_CAPTBL_<PROCNAME>
-------------------------------------------------------------------------------
Thread          -                               RVM_PROC_<PROCNAME>_THD_<THDNAME>
-------------------------------------------------------------------------------
Invocation      -                               RVM_PROC_<PROCNAME>_INV_<INVNAME>
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
    struct Proc_Info* Proc;
    struct Thd_Info* Thd;
    struct Inv_Info* Inv;
    struct Port_Info* Port;
    struct Recv_Info* Recv;
    struct Send_Info* Send;
    struct Vect_Info* Vect;

    for(EACH(struct Proc_Info*,Proc,Proj->Proc))
    {
        /* Processes and their capability tables */
        Proc->Proc_Cap.RVM_Macro=Make_Macro("RVM_PROC_",Proc->Name,"","");
        Proc->Captbl_Cap.RVM_Macro=Make_Macro("RVM_CAPTBL_",Proc->Name,"","");

        /* Threads - RVM only */
        for(EACH(struct Thd_Info*,Thd,Proc->Thd))
            Thd->Cap.RVM_Macro=Make_Macro("RVM_PROC_",Proc->Name,"_THD_",Thd->Name);

        /* Invocations - RVM only */
        for(EACH(struct Inv_Info*,Inv,Proc->Inv))
            Inv->Cap.RVM_Macro=Make_Macro("RVM_PROC_",Proc->Name,"_INV_",Inv->Name);

        /* Ports - Local only */
        for(EACH(struct Port_Info*,Port,Proc->Port))
            Port->Cap.Loc_Macro=Make_Macro("PORT_",Port->Name,"","");

        /* Receive endpoints - RVM and local */
        for(EACH(struct Recv_Info*,Recv,Proc->Recv))
        {
            Recv->Cap.Loc_Macro=Make_Macro("RECV_",Recv->Name,"","");
            Recv->Cap.RVM_Macro=Make_Macro("RVM_PROC_",Proc->Name,"_RECV_",Recv->Name);
        }

        /* Send endpoints - Local only */
        for(EACH(struct Send_Info*,Send,Proc->Send))
            Send->Cap.Loc_Macro=Make_Macro("SEND_",Send->Name,"","");

        /* Vector endpoints - RVM, RME and local */
        for(EACH(struct Vect_Info*,Vect,Proc->Vect))
        {
            Vect->Cap.Loc_Macro=Make_Macro("VECT_",Vect->Name,"","");
            Vect->Cap.RVM_Macro=Make_Macro("RVM_BOOT_VECT_",Vect->Name,"","");
            Vect->Cap.RME_Macro=Make_Macro("RME_BOOT_VECT_",Vect->Name,"","");
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
    struct Proc_Info* Proc;
    struct Proc_Info* Proc_Temp;
    struct Port_Info* Port;
    struct Inv_Info* Inv;
    struct Send_Info* Send;
    struct Recv_Info* Recv;
    struct Vect_Info* Vect;
    struct Chip_Vect_Info* Chip_Vect;

    for(EACH(struct Proc_Info*,Proc,Proj->Proc))
    {
        /* For every port, there must be a invocation somewhere */
        for(EACH(struct Port_Info*,Port,Proc->Port))
        {
            for(EACH(struct Proc_Info*,Proc_Temp,Proj->Proc))
            {
                if(strcmp(Proc_Temp->Name, Port->Proc_Name)==0)
                    break;
            }
            if(IS_HEAD(Proc_Temp,Proj->Proc))
                EXIT_FAIL("Invalid Process for Port.");

            for(EACH(struct Inv_Info*,Inv,Proc_Temp->Inv))
            {
                if(strcmp(Inv->Name, Port->Name)==0)
                {
                    Port->Cap.RVM_Capid=Inv->Cap.RVM_Capid;
                    Port->Cap.RVM_Macro=Inv->Cap.RVM_Macro;
                    break;
                }
            }
            if(IS_HEAD(Inv,Proc->Inv))
                EXIT_FAIL("Invalid Invocation for Port.");
        }

        /* For every send endpoint, there must be a receive endpoint somewhere */
        for(EACH(struct Send_Info*,Send,Proc->Send))
        {
            for(EACH(struct Proc_Info*,Proc_Temp,Proj->Proc))
            {
                if(strcmp(Proc_Temp->Name, Send->Proc_Name)==0)
                    break;
            }
            if(IS_HEAD(Proc_Temp,Proj->Proc))
                EXIT_FAIL("Invalid Process for Send endpoint.");

            for(EACH(struct Recv_Info*,Recv,Proc_Temp->Recv))
            {
                if(strcmp(Recv->Name, Send->Name)==0)
                {
                    Send->Cap.RVM_Capid=Recv->Cap.RVM_Capid;
                    Send->Cap.RVM_Macro=Recv->Cap.RVM_Macro;
                    break;
                }
            }
            if(IS_HEAD(Recv,Proc->Recv))
                EXIT_FAIL("Invalid Receive endpoint for Send endpoint.");
        }

        /* For every vector, there must be a corresponding chip interrupt vector somewhere */
        for(EACH(struct Vect_Info*,Vect,Proc->Vect))
        {
            for(EACH(struct Chip_Vect_Info*,Chip_Vect,Chip->Vect))
            {
                if(strcmp(Chip_Vect->Name, Vect->Name)==0)
                {
                    Vect->Num=Chip_Vect->Num;
                    break;
                }
            }
            if(IS_HEAD(Chip_Vect,Chip->Vect))
                EXIT_FAIL("Invalid Chip vector for Vector endpoint.");
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
    Check_Conflict(Proj);
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

/* Begin Function:Get_Kmem_Size ***********************************************
Description : Get the size of the kernel memory, and generate the initial states
              for kernel object creation.
Input       : struct Proj_Info* Proj - The project structure.
              ptr_t Capacity - The capacity of the capability table.
              ptr_t Init_Captbl_Size - The initial capability table's size;
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : None.
******************************************************************************/
void Get_Kmem_Size(struct Proj_Info* Proj, ptr_t Capacity, ptr_t Init_Captbl_Size)
{
    struct RVM_Cap_Info* Info;
    struct Proc_Info* Proc;
    struct Pgtbl_Info* Pgtbl;
    ptr_t Cap_Front;
    ptr_t Kmem_Front;

    /* Compute initial state when creating the vectors */
    Cap_Front=0;
    Kmem_Front=0;
    /* Initial capability table */
    Cap_Front++;
    Kmem_Front+=KOTBL_ROUND(CAPTBL_SIZE(Init_Captbl_Size,Proj->Plat.Word_Bits));
    /* Initial page table */
    Cap_Front++;
    Kmem_Front+=KOTBL_ROUND(Proj->Plat.Pgtbl_Size(8,1));
    /* Initial RVM process */
    Cap_Front++;
    Kmem_Front+=KOTBL_ROUND(PROC_SIZE(Proj->Plat.Word_Bits));
    /* Initial kcap and kmem */
    Cap_Front+=2;
    /* Initial tick timer/interrupt endpoint */
    Cap_Front+=2;
    Kmem_Front+=2*KOTBL_ROUND(SIG_SIZE(Proj->Plat.Word_Bits));
    /* Initial thread */
    Cap_Front++;
    Kmem_Front+=KOTBL_ROUND(Proj->Plat.Thd_Size);

    Proj->RME.Map.Vect_Cap_Front=Cap_Front;
    Proj->RME.Map.Vect_Kmem_Front=Kmem_Front;

    /* Compute initial state before entering the RVM */
    /* Capability tables for containing vector endpoints */
    Cap_Front+=(Proj->RVM.Vect_Front+(Capacity-1))/Capacity;
    Kmem_Front+=CAPTBL_TOTAL(Proj->RVM.Vect_Front,Capacity,Proj->Plat.Word_Bits);
    /* Vector endpoint themselves */
    Kmem_Front+=Proj->RVM.Vect_Front*KOTBL_ROUND(SIG_SIZE(Proj->Plat.Word_Bits));

    Proj->RVM.Map.Before_Cap_Front=Cap_Front;
    Proj->RVM.Map.Before_Kmem_Front=Kmem_Front;

    /* Compute initial state before creating capability tables */
    /* Three threads for RVM - now only one will be started */
    Cap_Front+=3;
    Kmem_Front+=3*KOTBL_ROUND(A7M_THD_SIZE);

    Proj->RVM.Map.Captbl_Cap_Front=Cap_Front;
    Proj->RVM.Map.Captbl_Kmem_Front=Kmem_Front;

    /* Compute initial state before creating page tables */
    /* Capability tables for containing capability tables */
    Cap_Front+=(Proj->RVM.Captbl_Front+(Capacity-1))/Capacity;
    Kmem_Front+=CAPTBL_TOTAL(Proj->RVM.Captbl_Front,Capacity,Proj->Plat.Word_Bits);
    /* Capability tables themselves */
    for(EACH(struct RVM_Cap_Info*,Info,Proj->RVM.Captbl))
    {
        Proc=Info->Cap;
        if((Proc->Captbl_Front+Proc->Extra_Captbl)>Capacity)
            EXIT_FAIL("Process capability table too large.");

        Kmem_Front+=KOTBL_ROUND(CAPTBL_SIZE(Proc->Captbl_Front+Proc->Extra_Captbl,Proj->Plat.Word_Bits));
    }

    Proj->RVM.Map.Pgtbl_Cap_Front=Cap_Front;
    Proj->RVM.Map.Pgtbl_Kmem_Front=Kmem_Front;

    /* Compute initial state before creating processes */
    /* Capability tables for containing page tables */
    Cap_Front+=(Proj->RVM.Pgtbl_Front+(Capacity-1))/Capacity;
    Kmem_Front+=CAPTBL_TOTAL(Proj->RVM.Pgtbl_Front,Capacity,Proj->Plat.Word_Bits);
    /* Page table themselves */
    for(EACH(struct RVM_Cap_Info*,Info,Proj->RVM.Pgtbl))
    {
        Pgtbl=Info->Cap;
        Kmem_Front+=KOTBL_ROUND(Proj->Plat.Pgtbl_Size(Pgtbl->Num_Order,Pgtbl->Is_Top));
    }
    
    Proj->RVM.Map.Proc_Cap_Front=Cap_Front;
    Proj->RVM.Map.Proc_Kmem_Front=Kmem_Front;

    /* Compute initial state before creating threads */
    /* Capability tables for containing processes */
    Cap_Front+=(Proj->RVM.Proc_Front+(Capacity-1))/Capacity;
    Kmem_Front+=CAPTBL_TOTAL(Proj->RVM.Proc_Front,Capacity,Proj->Plat.Word_Bits);
    /* Processes themselves */
    Kmem_Front+=Proj->RVM.Proc_Front*KOTBL_ROUND(PROC_SIZE(Proj->Plat.Word_Bits));

    Proj->RVM.Map.Thd_Cap_Front=Cap_Front;
    Proj->RVM.Map.Thd_Kmem_Front=Kmem_Front;

    /* Compute initial state before creating invocations */
    /* Capability tables for containing threads */
    Cap_Front+=(Proj->RVM.Thd_Front+(Capacity-1))/Capacity;
    Kmem_Front+=CAPTBL_TOTAL(Proj->RVM.Thd_Front,Capacity,Proj->Plat.Word_Bits);
    /* Threads themselves */
    Kmem_Front+=Proj->RVM.Thd_Front*KOTBL_ROUND(Proj->Plat.Thd_Size);

    Proj->RVM.Map.Inv_Cap_Front=Cap_Front;
    Proj->RVM.Map.Inv_Kmem_Front=Kmem_Front;

    /* Compute initial state before creating receive endpoints */
    /* Capability tables for containing invocations */
    Cap_Front+=(Proj->RVM.Inv_Front+(Capacity-1))/Capacity;
    Kmem_Front+=CAPTBL_TOTAL(Proj->RVM.Inv_Front,Capacity,Proj->Plat.Word_Bits);
    /* Invocations themselves */
    Kmem_Front+=Proj->RVM.Inv_Front*KOTBL_ROUND(Proj->Plat.Inv_Size);

    Proj->RVM.Map.Recv_Cap_Front=Cap_Front;
    Proj->RVM.Map.Recv_Kmem_Front=Kmem_Front;

    /* Compute the final utilization */
    /* Capability tables for containing receive endpoints */
    Cap_Front+=(Proj->RVM.Recv_Front+(Capacity-1))/Capacity;
    Kmem_Front+=CAPTBL_TOTAL(Proj->RVM.Recv_Front,Capacity,Proj->Plat.Word_Bits);
    /* Receive endpoints themselves */
    Kmem_Front+=Proj->RVM.Recv_Front*KOTBL_ROUND(SIG_SIZE(Proj->Plat.Word_Bits));

    Proj->RVM.Map.After_Cap_Front=Cap_Front;
    Proj->RVM.Map.After_Kmem_Front=Kmem_Front;
}
/* End Function:Get_Kmem_Size ************************************************/

/* Begin Function:Alloc_RME_Kmem **********************************************
Description : Allocate the kernel objects and memory for RME itself.
Input       : struct Proj_Info* Proj - The project structure.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : None.
******************************************************************************/
void Alloc_RME_Kmem(struct Proj_Info* Proj)
{
    /* Code section */
    Proj->RME.Map.Code_Base=Proj->RME.Code_Start;
    Proj->RME.Map.Code_Size=Proj->RME.Code_Size;

    /* Data section */
    Proj->RME.Map.Data_Base=Proj->RME.Data_Start;
    Proj->RME.Map.Data_Size=Proj->RME.Data_Size;

    /* The data section should at least be as large as what is to be allocated */
    if(Proj->RME.Map.Data_Size<=(KERNEL_INTF_SIZE+Proj->RME.Stack_Size+Proj->RVM.Map.After_Kmem_Front))
        EXIT_FAIL("RME General Data_Size is not big enough.");

    /* Interrupt flag section - cut out from the data section */
    Proj->RME.Map.Intf_Base=Proj->RME.Map.Data_Base+Proj->RME.Map.Data_Size-KERNEL_INTF_SIZE;
    Proj->RME.Map.Intf_Size=KERNEL_INTF_SIZE;
    if(Proj->RME.Map.Intf_Base<=Proj->RME.Map.Data_Base)
        EXIT_FAIL("RME General Data_Size is not big enough.");
    Proj->RME.Map.Data_Size=Proj->RME.Map.Intf_Base-Proj->RME.Map.Data_Base;

    /* Stack section - cut out from the data section */
    Proj->RME.Map.Stack_Base=Proj->RME.Map.Data_Base+Proj->RME.Map.Data_Size-Proj->RME.Stack_Size;
    Proj->RME.Map.Stack_Size=Proj->RME.Stack_Size;
    if(Proj->RME.Map.Stack_Base<=Proj->RME.Map.Data_Base)
        EXIT_FAIL("RME General Data_Size is not big enough.");
    Proj->RME.Map.Data_Size=Proj->RME.Map.Stack_Base-Proj->RME.Map.Data_Base;

    /* Kernel memory section - cut out from the data section */
    Proj->RME.Map.Kmem_Base=Proj->RME.Map.Data_Base+Proj->RME.Map.Data_Size-Proj->RVM.Map.After_Kmem_Front;
    Proj->RME.Map.Kmem_Base=KOTBL_ROUND(Proj->RME.Map.Kmem_Base)-KOTBL_ROUND(1);
    Proj->RME.Map.Kmem_Size=Proj->RVM.Map.After_Kmem_Front;
    if(Proj->RME.Map.Kmem_Base<=Proj->RME.Map.Data_Base)
        EXIT_FAIL("RME General Data_Size is not big enough.");
    Proj->RME.Map.Data_Size=Proj->RME.Map.Kmem_Base-Proj->RME.Map.Data_Base;
}
/* End Function:Alloc_RME_Kmem ***********************************************/

/* Begin Function:Alloc_RVM_Mem **********************************************
Description : Allocate the kernel objects and memory for RVM user-level library.
Input       : struct Proj_Info* Proj - The project structure.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : None.
******************************************************************************/
void Alloc_RVM_Mem(struct Proj_Info* Proj)
{
    /* Code section */
    Proj->RVM.Map.Code_Base=Proj->RME.Code_Start+Proj->RME.Code_Size;
    Proj->RVM.Map.Code_Size=Proj->RVM.Code_Size;

    /* Data section */
    Proj->RVM.Map.Data_Base=Proj->RME.Data_Start+Proj->RME.Data_Size;
    Proj->RVM.Map.Data_Size=Proj->RVM.Data_Size;

    /* The data section should at least be as large as what is to be allocated */
    if(Proj->RVM.Map.Data_Size<=(3*Proj->RVM.Stack_Size))
        EXIT_FAIL("RVM General Data_Size is not big enough.");

    /* Guard stack section - cut out from the data section */
    Proj->RVM.Map.Guard_Stack_Base=Proj->RVM.Map.Data_Base+Proj->RVM.Map.Data_Size-Proj->RVM.Stack_Size;
    Proj->RVM.Map.Guard_Stack_Size=Proj->RVM.Stack_Size;
    if(Proj->RVM.Map.Guard_Stack_Base<=Proj->RVM.Map.Data_Base)
        EXIT_FAIL("RVM General Data_Size is not big enough.");
    Proj->RVM.Map.Data_Size=Proj->RVM.Map.Guard_Stack_Base-Proj->RVM.Map.Data_Base;

    /* VMM stack section - cut out from the data section */
    Proj->RVM.Map.VMM_Stack_Base=Proj->RVM.Map.Data_Base+Proj->RVM.Map.Data_Size-Proj->RVM.Stack_Size;
    Proj->RVM.Map.VMM_Stack_Size=Proj->RVM.Stack_Size;
    if(Proj->RVM.Map.VMM_Stack_Base<=Proj->RVM.Map.Data_Base)
        EXIT_FAIL("RVM General Data_Size is not big enough.");
    Proj->RVM.Map.Data_Size=Proj->RVM.Map.VMM_Stack_Base-Proj->RVM.Map.Data_Base;
    
    /* Interrupt stack section - cut out from the data section */
    Proj->RVM.Map.Intd_Stack_Base=Proj->RVM.Map.Data_Base+Proj->RVM.Map.Data_Size-Proj->RVM.Stack_Size;
    Proj->RVM.Map.Intd_Stack_Size=Proj->RVM.Stack_Size;
    if(Proj->RVM.Map.Intd_Stack_Base<=Proj->RVM.Map.Data_Base)
        EXIT_FAIL("RVM General Data_Size is not big enough.");
    Proj->RVM.Map.Data_Size=Proj->RVM.Map.Intd_Stack_Base-Proj->RVM.Map.Data_Base;
}
/* End Function:Alloc_RVM_Mem ************************************************/

/* Begin Function:Alloc_Proc_Mem **********************************************
Description : Allocate process memory.
Input       : struct Proj_Info* Proj - The project structure.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : None.
******************************************************************************/
void Alloc_Proc_Mem(struct Proj_Info* Proj)
{
    struct Proc_Info* Proc;
    struct Thd_Info* Thd;
    struct Inv_Info* Inv;
    struct Mem_Info* Mem;
    
    for(EACH(struct Proc_Info*,Proc,Proj->Proc))
    {
        Mem=(struct Mem_Info*)(Proc->Code.Next);
        Proc->Map.Code_Base=Mem->Start;
        Proc->Map.Code_Size=Mem->Size;
        Proc->Map.Entry_Code_Front=Proc->Map.Code_Base;
        Mem=(struct Mem_Info*)(Proc->Data.Next);
        Proc->Map.Data_Base=Mem->Start;
        Proc->Map.Data_Size=Mem->Size;

        /* Threads come first */
        for(EACH(struct Thd_Info*,Thd,Proc->Thd))
        {
            /* Allocate stack from the main data memory */
            Thd->Map.Stack_Base=Proc->Map.Data_Base+Proc->Map.Data_Size-Thd->Stack_Size;
            Thd->Map.Stack_Size=Thd->Stack_Size;
            if(Thd->Map.Stack_Base<=Proc->Map.Data_Base)
                EXIT_FAIL("Process Data_Size is not big enough.");
            Proc->Map.Data_Size=Thd->Map.Stack_Base-Proc->Map.Data_Base;

            /* Allocate entry from code memory */
            Thd->Map.Entry_Addr=Proc->Map.Entry_Code_Front;
            Proc->Map.Entry_Code_Front+=Proj->Plat.Word_Bits*ENTRY_SLOT_SIZE/8;

            /* The parameter is always the param, turned into an unsigned integer */
            Thd->Map.Param_Value=strtoull(Thd->Parameter,0,0);
        }

        /* Then invocations */
        for(EACH(struct Inv_Info*,Inv,Proc->Inv))
        {
            /* Allocate stack from the main data memory */
            Inv->Map.Stack_Base=Proc->Map.Data_Base+Proc->Map.Data_Size-Inv->Stack_Size;
            Inv->Map.Stack_Size=Inv->Stack_Size;
            if(Inv->Map.Stack_Base<=Proc->Map.Data_Base)
                EXIT_FAIL("Process Data_Size is not big enough.");
            Proc->Map.Data_Size=Inv->Map.Stack_Base-Proc->Map.Data_Base;

            /* Allocate entry from code memory */
            Inv->Map.Entry_Addr=Proc->Map.Entry_Code_Front;
            Proc->Map.Entry_Code_Front+=Proj->Plat.Word_Bits*ENTRY_SLOT_SIZE/8;
        }
    }
}
/* End Function:Alloc_Proc_Mem ***********************************************/

/* Begin Function:Alloc_Mem ***************************************************
Description : Allocate the kernel objects and memory.
Input       : struct Proj_Info* Proj - The project structure.
Output      : struct Proj_Info* Proj - The updated project structure.
Return      : None.
******************************************************************************/
void Alloc_Mem(struct Proj_Info* Proj)
{
    ptr_t Capacity;
    
    Capacity=Proj->Plat.Captbl_Capacity;

    /* Compute the kernel memory needed, disregarding the initial
     * capability table size because we don't know its size yet */
    Get_Kmem_Size(Proj,Capacity,0);

    /* Are we exceeding the maximum of our capability tables? */
    if((Proj->RVM.Map.After_Cap_Front+Proj->RVM.Extra_Captbl)>Capacity)
        EXIT_FAIL("RVM capability table too large.");

    /* Now recompute to get the real usage */
    Get_Kmem_Size(Proj,Capacity,Proj->RVM.Map.After_Cap_Front+Proj->RVM.Extra_Captbl);

    /* Populate RME information */
    Alloc_RME_Kmem(Proj);
    /* Populate RVM information */
    Alloc_RVM_Mem(Proj);
    /* Populate Process information */
    Alloc_Proc_Mem(Proj);
}
/* End Function:Alloc_Mem ****************************************************/

/* Begin Function:Make_Str ****************************************************
Description : Concatenate two strings and return the result.
Input       : s8_t* Str1 - The first string.
              s8_t* Str2 - The second string.
Output      : None.
Return      : s8_t* - The final result.
******************************************************************************/
s8_t* Make_Str(s8_t* Str1, s8_t* Str2)
{
    s8_t* Ret;

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
Input       : struct List* Info - The list containing all raw information.
              s8_t* Tag - The tag for the information.
Output      : None.
Return      : s8_t* - The value for the information.
******************************************************************************/
s8_t* Raw_Match(struct List* Raw, s8_t* Tag)
{
    struct Raw_Info* Info;

    for(EACH(struct Raw_Info*,Info,*Raw))
    {
        if(strcmp(Info->Tag,Tag)==0)
            return Info->Val;
    }

    return 0;
}
/* End Function:Raw_Match ****************************************************/

/* Begin Function:Write_Src_Desc **********************************************
Description : Output the header that is sticked to every C file.
Input       : FILE* File - The pointer to the file.
              s8_t* Filename - The name of the file.
              s8_t* Description - The description of the file.
Output      : FILE* File - The pointer to the updated file.
Return      : None.
******************************************************************************/
void Write_Src_Desc(FILE* File, s8_t* Filename, s8_t* Description)
{
    s8_t Date[64];
    time_t Time;
    struct tm* Time_Struct;

    time(&Time);
    Time_Struct=localtime(&Time);
    sprintf(Date,"%02d/%02d/%d",Time_Struct->tm_mday,Time_Struct->tm_mon+1,Time_Struct->tm_year+1900);

    fprintf(File, "/******************************************************************************\n");
    fprintf(File, "Filename    : %s\n", Filename);
    fprintf(File, "Author      : %s\n", CODE_AUTHOR);
    fprintf(File, "Date        : %s\n", Date);
    fprintf(File, "License     : %s\n", "LGPL v3+; see COPYING for details.");
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
              s8_t* Funcname - The name of the function.
Output      : FILE* File - The pointer to the updated file.
Return      : None.
******************************************************************************/
void Write_Func_Desc(FILE* File, s8_t* Funcname)
{
    ptr_t Len;
    s8_t Buf[256];

    for(Len=sprintf(Buf, "/* Begin Function:%s ", Funcname);Len<79;Len++)
        Buf[Len]='*';
    Buf[Len]='\0';
    fprintf(File, "%s\n",Buf);
}
/* End Function:Write_Func_Desc **********************************************/

/* Begin Function:Write_Func_None *********************************************
Description : Write the rest of the function header, assuming no input, output
              and return.
Input       : FILE* File - The pointer to the file.
Output      : FILE* File - The pointer to the updated file.
Return      : None.
******************************************************************************/
void Write_Func_None(FILE* File)
{
    fprintf(File, "Input       : None.\n");
    fprintf(File, "Output      : None.\n");
    fprintf(File, "Return      : None.\n");
    fprintf(File, "******************************************************************************/\n");
}
/* End Function:Write_Func_None **********************************************/

/* Begin Function:Write_Func_Footer *******************************************
Description : Output the footer that is appended to every C function.
Input       : FILE* File - The pointer to the file.
              s8_t* Funcname - The name of the function.
Output      : FILE* File - The pointer to the updated file.
Return      : None.
******************************************************************************/
void Write_Func_Footer(FILE* File, s8_t* Funcname)
{
    ptr_t Len;
    s8_t Buf[256];

    for(Len=sprintf(Buf, "/* End Function:%s ", Funcname);Len<78;Len++)
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
              s8_t* Macro - The macro.
              s8_t* Value - The value of the macro.
              s8_t* Align - The alignment, must be bigger than 12.
Output      : None.
Return      : None.
******************************************************************************/
void Make_Define_Str(FILE* File, s8_t* Macro, s8_t* Value, ptr_t Align)
{
    s8_t Buf[32];

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
              s8_t* Macro - The macro.
              ptr_t Value - The value of the macro.
              s8_t* Align - The alignment, must be bigger than 12.
Output      : None.
Return      : None.
******************************************************************************/
void Make_Define_Int(FILE* File, s8_t* Macro, ptr_t Value, ptr_t Align)
{
    s8_t Buf[32];

    /* Print to file */
    sprintf(Buf, "#define %%-%llds    (%%lld)\n", Align-4-8);
    fprintf(File, Buf, Macro, Value);
}
/* End Function:Make_Define_Int **********************************************/

/* Begin Function:Make_Define_Hex *********************************************
Description : Make a define statement in the file. The define statement can have
              three parts, which will be converted to uppercase and concatenated
              together.
              The value here is a hex integer.
Input       : FILE* File - The file structure.
              s8_t* Macro - The macro.
              ptr_t Value - The value of the macro.
              s8_t* Align - The alignment, must be bigger than 12.
Output      : None.
Return      : None.
******************************************************************************/
void Make_Define_Hex(FILE* File, s8_t* Macro, ptr_t Value, ptr_t Align)
{
    s8_t Buf[32];

    /* Print to file */
    sprintf(Buf, "#define %%-%llds    (0x%%llX)\n", Align-4-8);
    fprintf(File, Buf, Macro, Value);
}
/* End Function:Make_Define_Hex **********************************************/

/* Begin Function:Setup_RME_Folder ********************************************
Description : Setup the folder contents for RME.
Input       : struct Proj_Info* Proj - The project structure.
              struct Chip_Info* Chip - The chip structure.
              s8_t* RME_Path - The RME root folder path.
              s8_t* Output_Path - The output folder path.
Output      : None.
Return      : None.
******************************************************************************/
void Setup_RME_Folder(struct Proj_Info* Proj, struct Chip_Info* Chip, s8_t* RME_Path, s8_t* Output_Path)
{
    s8_t* Buf1;
    s8_t* Buf2;

    /* Allocate the buffer */
    Buf1=Malloc(4096);
    Buf2=Malloc(4096);

    /* RME directory */
    MAKE_DIR(Buf1,"%s/M7M1_MuEukaron",Output_Path);
    MAKE_DIR(Buf1,"%s/M7M1_MuEukaron/Documents",Output_Path);
    MAKE_DIR(Buf1,"%s/M7M1_MuEukaron/MEukaron",Output_Path);
    MAKE_DIR(Buf1,"%s/M7M1_MuEukaron/MEukaron/Include",Output_Path);
    MAKE_DIR(Buf1,"%s/M7M1_MuEukaron/MEukaron/Include/Kernel",Output_Path);
    MAKE_DIR(Buf1,"%s/M7M1_MuEukaron/MEukaron/Include/Platform",Output_Path);
    MAKE_DIR(Buf1,"%s/M7M1_MuEukaron/MEukaron/Include/Platform/%s",Output_Path,Proj->Plat_Name);
    MAKE_DIR(Buf1,"%s/M7M1_MuEukaron/MEukaron/Include/Platform/%s/Chips",Output_Path,Proj->Plat_Name);
    MAKE_DIR(Buf1,"%s/M7M1_MuEukaron/MEukaron/Include/Platform/%s/Chips/%s",Output_Path,Proj->Plat_Name,Chip->Class);
    MAKE_DIR(Buf1,"%s/M7M1_MuEukaron/MEukaron/Kernel",Output_Path);
    MAKE_DIR(Buf1,"%s/M7M1_MuEukaron/MEukaron/Platform",Output_Path);
    MAKE_DIR(Buf1,"%s/M7M1_MuEukaron/MEukaron/Platform/%s",Output_Path,Proj->Plat_Name);
    MAKE_DIR(Buf1,"%s/M7M1_MuEukaron/Project",Output_Path);
    MAKE_DIR(Buf1,"%s/M7M1_MuEukaron/Project/Source",Output_Path);
    MAKE_DIR(Buf1,"%s/M7M1_MuEukaron/Project/Include",Output_Path);

    /* Copy kernel file, kernel header, platform file, platform header, and chip headers */
    sprintf(Buf1,"%s/M7M1_MuEukaron/Documents/EN_M7M1_Microkernel-RTOS-User-Manual.pdf",Output_Path);
    sprintf(Buf2,"%s/Documents/EN_M7M1_Microkernel-RTOS-User-Manual.pdf",RME_Path);
    Copy_File(Buf1, Buf2);
    sprintf(Buf1,"%s/M7M1_MuEukaron/Documents/CN_M7M1_Microkernel-RTOS-User-Manual.pdf",Output_Path);
    sprintf(Buf2,"%s/Documents/CN_M7M1_Microkernel-RTOS-User-Manual.pdf",RME_Path);
    Copy_File(Buf1, Buf2);
    sprintf(Buf1,"%s/M7M1_MuEukaron/MEukaron/Kernel/rme_kernel.c",Output_Path);
    sprintf(Buf2,"%s/MEukaron/Kernel/rme_kernel.c",RME_Path);
    Copy_File(Buf1, Buf2);
    /* The toolchain specific one will be created when we are playing with toolchains */
    sprintf(Buf1,"%s/M7M1_MuEukaron/MEukaron/Platform/%s/rme_platform_%s.c",Output_Path,Proj->Plat_Name,Proj->Lower_Plat);
    sprintf(Buf2,"%s/MEukaron/Platform/%s/rme_platform_%s.c",RME_Path,Proj->Plat_Name,Proj->Lower_Plat);
    Copy_File(Buf1, Buf2);
    sprintf(Buf1,"%s/M7M1_MuEukaron/MEukaron/Include/rme.h",Output_Path);
    sprintf(Buf2,"%s/MEukaron/Include/rme.h",RME_Path);
    Copy_File(Buf1, Buf2);
    sprintf(Buf1,"%s/M7M1_MuEukaron/MEukaron/Include/Kernel/rme_kernel.h",Output_Path);
    sprintf(Buf2,"%s/MEukaron/Include/Kernel/rme_kernel.h",RME_Path);
    Copy_File(Buf1, Buf2);
    sprintf(Buf1,"%s/M7M1_MuEukaron/MEukaron/Include/Platform/%s/rme_platform_%s.h",Output_Path,Proj->Plat_Name,Proj->Lower_Plat);
    sprintf(Buf2,"%s/MEukaron/Include/Platform/%s/rme_platform_%s.h",RME_Path,Proj->Plat_Name,Proj->Lower_Plat);
    Copy_File(Buf1, Buf2);
    sprintf(Buf1,"%s/M7M1_MuEukaron/MEukaron/Include/Platform/%s/Chips/%s/rme_platform_%s.h",
                  Output_Path,Proj->Plat_Name,Chip->Class,Chip->Class);
    sprintf(Buf2,"%s/MEukaron/Include/Platform/%s/Chips/%s/rme_platform_%s.h", RME_Path,Proj->Plat_Name,Chip->Class,Chip->Class);
    Copy_File(Buf1, Buf2);

    Free(Buf1);
    Free(Buf2);
}
/* End Function:Setup_RME_Folder *********************************************/

/* Begin Function:Setup_RVM_Folder ********************************************
Description : Setup the folder contents for RVM.
Input       : struct Proj_Info* Proj - The project structure.
              struct Chip_Info* Chip - The chip structure.
              s8_t* RVM_Path - The RVM root folder path.
              s8_t* Output_Path - The output folder path.
Output      : None.
Return      : None.
******************************************************************************/
void Setup_RVM_Folder(struct Proj_Info* Proj, struct Chip_Info* Chip, s8_t* RVM_Path, s8_t* Output_Path)
{
    s8_t* Buf1;
    s8_t* Buf2;

    /* Allocate the buffer */
    Buf1=Malloc(4096);
    Buf2=Malloc(4096);

    /* RME directory */
    MAKE_DIR(Buf1,"%s/M7M2_MuAmmonite",Output_Path);
    MAKE_DIR(Buf1,"%s/M7M2_MuAmmonite/Documents",Output_Path);
    MAKE_DIR(Buf1,"%s/M7M2_MuAmmonite/MAmmonite",Output_Path);
    MAKE_DIR(Buf1,"%s/M7M2_MuAmmonite/MAmmonite/Include",Output_Path);
    MAKE_DIR(Buf1,"%s/M7M2_MuAmmonite/MAmmonite/Include/Init",Output_Path);
    MAKE_DIR(Buf1,"%s/M7M2_MuAmmonite/MAmmonite/Include/Platform",Output_Path);
    MAKE_DIR(Buf1,"%s/M7M2_MuAmmonite/MAmmonite/Include/Platform/%s",Output_Path,Proj->Plat_Name);
    MAKE_DIR(Buf1,"%s/M7M2_MuAmmonite/MAmmonite/Include/Platform/%s/Chips",Output_Path,Proj->Plat_Name);
    MAKE_DIR(Buf1,"%s/M7M2_MuAmmonite/MAmmonite/Include/Platform/%s/Chips/%s",Output_Path,Proj->Plat_Name,Chip->Class);
    MAKE_DIR(Buf1,"%s/M7M2_MuAmmonite/MAmmonite/Init",Output_Path);
    MAKE_DIR(Buf1,"%s/M7M2_MuAmmonite/MAmmonite/Platform",Output_Path);
    MAKE_DIR(Buf1,"%s/M7M2_MuAmmonite/MAmmonite/Platform/%s",Output_Path,Proj->Plat_Name);
    MAKE_DIR(Buf1,"%s/M7M2_MuAmmonite/Project",Output_Path);
    MAKE_DIR(Buf1,"%s/M7M2_MuAmmonite/Project/Source",Output_Path);
    MAKE_DIR(Buf1,"%s/M7M2_MuAmmonite/Project/Include",Output_Path);

    /* Copy kernel file, kernel header, platform file, platform header, and chip headers */
    sprintf(Buf1,"%s/M7M2_MuAmmonite/Documents/EN_M7M2_RT-Runtime-User-Manual.pdf",Output_Path);
    sprintf(Buf2,"%s/Documents/EN_M7M2_RT-Runtime-User-Manual.pdf",RVM_Path);
    Copy_File(Buf1, Buf2);
    sprintf(Buf1,"%s/M7M2_MuAmmonite/Documents/CN_M7M2_RT-Runtime-User-Manual.pdf",Output_Path);
    sprintf(Buf2,"%s/Documents/CN_M7M2_RT-Runtime-User-Manual.pdf",RVM_Path);
    Copy_File(Buf1, Buf2);
    /* Currently the VMM and Posix is disabled, thus only the init is copied. */
    sprintf(Buf1,"%s/M7M2_MuAmmonite/MAmmonite/Init/rvm_init.c",Output_Path);
    sprintf(Buf2,"%s/MAmmonite/Init/rvm_init.c",RVM_Path);
    Copy_File(Buf1, Buf2);
    /* The toolchain specific one will be created when we are playing with toolchains */
    sprintf(Buf1,"%s/M7M2_MuAmmonite/MAmmonite/Platform/%s/rvm_platform_%s.c",Output_Path,Proj->Plat_Name,Proj->Lower_Plat);
    sprintf(Buf2,"%s/MAmmonite/Platform/%s/rvm_platform_%s.c",RVM_Path,Proj->Plat_Name,Proj->Lower_Plat);
    Copy_File(Buf1, Buf2);
    sprintf(Buf1,"%s/M7M2_MuAmmonite/MAmmonite/Include/rvm.h",Output_Path);
    sprintf(Buf2,"%s/MAmmonite/Include/rvm.h",RVM_Path);
    Copy_File(Buf1, Buf2);
    sprintf(Buf1,"%s/M7M2_MuAmmonite/MAmmonite/Include/Init/rvm_init.h",Output_Path);
    sprintf(Buf2,"%s/MAmmonite/Include/Init/rvm_init.h",RVM_Path);
    Copy_File(Buf1, Buf2);
    sprintf(Buf1,"%s/M7M2_MuAmmonite/MAmmonite/Include/Platform/%s/rvm_platform_%s.h",Output_Path,Proj->Plat_Name,Proj->Lower_Plat);
    sprintf(Buf2,"%s/MAmmonite/Include/Platform/%s/rvm_platform_%s.h",RVM_Path,Proj->Plat_Name,Proj->Lower_Plat);
    Copy_File(Buf1, Buf2);
    sprintf(Buf1,"%s/M7M2_MuAmmonite/MAmmonite/Include/Platform/%s/Chips/%s/rvm_platform_%s.h",
                 Output_Path,Proj->Plat_Name,Chip->Class,Chip->Class);
    sprintf(Buf2,"%s/MAmmonite/Include/Platform/%s/Chips/%s/rvm_platform_%s.h",RVM_Path,Proj->Plat_Name,Chip->Class,Chip->Class);
    Copy_File(Buf1, Buf2);

    Free(Buf1);
    Free(Buf2);
}
/* End Function:Setup_RVM_Folder *********************************************/

/* Begin Function:Setup_RME_Conf **********************************************
Description : Crank the platform configuration headers for RME.
Input       : struct Proj_Info* Proj - The project structure.
              struct Chip_Info* Chip - The chip structure.
              s8_t* RME_Path - The RME root folder path.
              s8_t* Output_Path - The output folder path.
Output      : None.
Return      : None.
******************************************************************************/
void Setup_RME_Conf(struct Proj_Info* Proj, struct Chip_Info* Chip, s8_t* RME_Path, s8_t* Output_Path)
{
    /* Create the file and the file header */
    s8_t* Buf;
    FILE* File;

    Buf=Malloc(4096);

    /* Generate rme_platform.h */
    sprintf(Buf, "%s/M7M1_MuEukaron/MEukaron/Include/Platform/rme_platform.h", Output_Path);
    File=fopen(Buf, "wb");
    if(File==0)
        EXIT_FAIL("rme_platform.h open failed.");

    Write_Src_Desc(File, "rme_platform.h", "The platform selection header.");
    fprintf(File, "/* Platform Includes *********************************************************/\n");
    fprintf(File, "#include \"Platform/%s/rme_platform_%s.h\"\n", Proj->Plat_Name, Proj->Lower_Plat);
    fprintf(File, "/* End Platform Includes *****************************************************/\n\n");
    Write_Src_Footer(File);
    fclose(File);

    /* Generate rme_platform.h */
    sprintf(Buf, "%s/M7M1_MuEukaron/MEukaron/Include/Platform/%s/rme_platform_%s_conf.h", Output_Path, Proj->Plat_Name, Proj->Lower_Plat);
    File=fopen(Buf, "wb");
    sprintf(Buf, "rme_platform_%s_conf.h", Proj->Lower_Plat);
    if(File==0)
        EXIT_FAIL("rme_platform_xxx_conf.h open failed.");

    Write_Src_Desc(File, Buf, "The platform chip selection header.");
    fprintf(File, "/* Platform Includes *********************************************************/\n");
    fprintf(File, "#include \"Platform/%s/Chips/%s/rme_platform_%s.h\"\n", Proj->Plat_Name, Chip->Class, Chip->Class);
    fprintf(File, "/* End Platform Includes *****************************************************/\n\n");
    Write_Src_Footer(File);
    fclose(File);

    Free(Buf);
}
/* End Function:Setup_RME_Conf ***********************************************/

/* Begin Function:Setup_RVM_Conf **********************************************
Description : Crank the platform configuration headers for RVM.
Input       : struct Proj_Info* Proj - The project structure.
              struct Chip_Info* Chip - The chip structure.
              s8_t* RVM_Path - The RME root folder path.
              s8_t* Output_Path - The output folder path.
Output      : None.
Return      : None.
******************************************************************************/
void Setup_RVM_Conf(struct Proj_Info* Proj, struct Chip_Info* Chip, s8_t* RVM_Path, s8_t* Output_Path)
{
    /* Create the file and the file header */
    s8_t* Buf;
    FILE* File;

    Buf=Malloc(4096);

    /* Generate rme_platform.h */
    sprintf(Buf, "%s/M7M2_MuAmmonite/MAmmonite/Include/Platform/rvm_platform.h", Output_Path);
    File=fopen(Buf, "wb");
    if(File==0)
        EXIT_FAIL("rvm_platform.h open failed.");

    Write_Src_Desc(File, "rvm_platform.h", "The platform selection header.");
    fprintf(File, "/* Platform Includes *********************************************************/\n");
    fprintf(File, "#include \"Platform/%s/rvm_platform_%s.h\"\n", Proj->Plat_Name, Proj->Lower_Plat);
    fprintf(File, "/* End Platform Includes *****************************************************/\n\n");
    Write_Src_Footer(File);
    fclose(File);

    /* Generate rme_platform.h */
    sprintf(Buf, "%s/M7M2_MuAmmonite/MAmmonite/Include/Platform/%s/rme_platform_%s_conf.h", Output_Path, Proj->Plat_Name, Proj->Lower_Plat);
    File=fopen(Buf, "wb");
    sprintf(Buf, "rvm_platform_%s_conf.h", Proj->Lower_Plat);
    if(File==0)
        EXIT_FAIL("rvm_platform_xxx_conf.h open failed.");

    Write_Src_Desc(File, Buf, "The platform chip selection header.");
    fprintf(File, "/* Platform Includes *********************************************************/\n");
    fprintf(File, "#include \"Platform/%s/Chips/%s/rvm_platform_%s.h\"\n", Proj->Plat_Name, Chip->Class, Chip->Class);
    fprintf(File, "/* End Platform Includes *****************************************************/\n\n");
    Write_Src_Footer(File);
    fclose(File);

    Free(Buf);
}
/* End Function:Setup_RVM_Conf ***********************************************/

/* Begin Function:Print_RME_Inc ***********************************************
Description : Generate the RME-related include section.
Input       : FILE* File - The file to print to.
              struct Proj_Info* Proj - The project structure.
Output      : None.
Return      : None.
******************************************************************************/
void Print_RME_Inc(FILE* File, struct Proj_Info* Proj)
{
    /* Print includes */
    fprintf(File, "#define __HDR_DEFS__\n");
    fprintf(File, "#include \"Platform/%s/rme_platform_%s.h\"\n", Proj->Plat_Name, Proj->Lower_Plat);
    fprintf(File, "#include \"Kernel/rme_kernel.h\"\n");
    fprintf(File, "#undef __HDR_DEFS__\n\n");
    fprintf(File, "#define __HDR_STRUCTS__\n");
    fprintf(File, "#include \"Platform/%s/rme_platform_%s.h\"\n", Proj->Plat_Name, Proj->Lower_Plat);
    fprintf(File, "#include \"Kernel/rme_kernel.h\"\n");
    fprintf(File, "#undef __HDR_STRUCTS__\n\n");
    fprintf(File, "#define __HDR_PUBLIC_MEMBERS__\n");
    fprintf(File, "#include \"Platform/%s/rme_platform_%s.h\"\n", Proj->Plat_Name, Proj->Lower_Plat);
    fprintf(File, "#include \"Kernel/rme_kernel.h\"\n");
    fprintf(File, "#undef __HDR_PUBLIC_MEMBERS__\n\n");
}
/* End Function:Print_RME_Inc ************************************************/

/* Begin Function:Gen_RME_Boot ************************************************
Description : Generate the rme_boot.h and rme_boot.c. These files are mainly
              responsible for setting up interrupt endpoints.
Input       : struct Proj_Info* Proj - The project structure.
              struct Chip_Info* Chip - The chip structure.
              s8_t* RME_Path - The RME root folder path.
              s8_t* Output_Path - The output folder path.
Output      : None.
Return      : None.
******************************************************************************/
void Gen_RME_Boot(struct Proj_Info* Proj, struct Chip_Info* Chip, s8_t* RME_Path, s8_t* Output_Path)
{
    s8_t* Buf;
    FILE* File;
    ptr_t Obj_Cnt;
    struct Vect_Info* Vect;
    struct RVM_Cap_Info* Info;
    ptr_t Cap_Front;
    ptr_t Capacity;
    ptr_t Captbl_Size;

    Buf=Malloc(4096);

    /* Generate rme_boot.h */
    sprintf(Buf, "%s/M7M1_MuEukaron/Project/Include/rme_boot.h", Output_Path);
    File=fopen(Buf, "wb");
    if(File==0)
        EXIT_FAIL("rme_boot.h open failed.");
    Write_Src_Desc(File, "rme_boot.h", "The boot-time initialization file header.");
    fprintf(File, "/* Defines *******************************************************************/\n");
    fprintf(File, "/* Vector endpoint capability tables */\n");

    /* Vector capability table */
    Cap_Front=Proj->RME.Map.Vect_Cap_Front;
    Capacity=Proj->Plat.Captbl_Capacity;
    for(Obj_Cnt=0;Obj_Cnt<Proj->RVM.Vect_Front;Obj_Cnt+=Capacity)
    {
        sprintf(Buf, "RME_BOOT_CTVECT%lld",Obj_Cnt/Capacity);
        Make_Define_Int(File, Buf, Cap_Front++, MACRO_ALIGNMENT);
    }

    /* Vector endpoints */
    fprintf(File, "\n/* Vector endpoints */\n");
    for(EACH(struct RVM_Cap_Info*,Info,Proj->RVM.Vect))
    {
        Vect=(struct Vect_Info*)(Info->Cap);
        sprintf(Buf, "RME_CAPID(RME_BOOT_CTVECT%lld,%lld)", Obj_Cnt/Capacity, Obj_Cnt%Capacity);
        Make_Define_Str(File, Vect->Cap.RME_Macro, Buf, MACRO_ALIGNMENT);
    }
    fprintf(File, "/* End Defines ***************************************************************/\n\n");
    Write_Src_Footer(File);
    fclose(File);

    /* Generate rme_boot.c */
    sprintf(Buf, "%s/M7M1_MuEukaron/Project/Source/rme_boot.c", Output_Path);
    File=fopen(Buf, "wb");
    if(File==0)
        EXIT_FAIL("rme_boot.c open failed.");
    Write_Src_Desc(File, "rme_boot.c", "The boot-time initialization file.");

    /* Print all header includes */
    fprintf(File, "/* Includes ******************************************************************/\n");
    Print_RME_Inc(File, Proj);
    fprintf(File, "#include \"rme_boot.h\"\n");
    fprintf(File, "/* End Includes **************************************************************/\n\n");

    /* Print all global variables and prototypes */
    fprintf(File, "/* Private Global Variables **************************************************/\n");
    for(EACH(struct RVM_Cap_Info*,Info,Proj->RVM.Vect))
    {
        Vect=(struct Vect_Info*)(Info->Cap);
        fprintf(File, "static struct RME_Sig_Struct* %s_Vect_Sig;\n", Vect->Name);
    }
    fprintf(File, "/* End Private Global Variables **********************************************/\n\n");
    fprintf(File, "/* Private C Function Prototypes *********************************************/\n");
    for(EACH(struct RVM_Cap_Info*,Info,Proj->RVM.Vect))
    {
        Vect=(struct Vect_Info*)(Info->Cap);
        fprintf(File, "static rme_ptr_t RME_Vect_%s_User(rme_ptr_t Int_Num);\n", Vect->Name);
    }
    fprintf(File, "/* End Private C Function Prototypes *****************************************/\n\n");
    fprintf(File, "/* Public C Function Prototypes **********************************************/\n");
    fprintf(File, "void RME_Boot_Vect_Init(struct RME_Cap_Captbl* Captbl, rme_ptr_t Cap_Front, rme_ptr_t Kmem_Front);\n");
    fprintf(File, "rme_ptr_t RME_Boot_Vect_Handler(rme_ptr_t Vect_Num);\n");
    fprintf(File, "/* End Public C Function Prototypes ******************************************/\n\n");

    /* Boot-time setup routine for the interrupt endpoints */
    Write_Func_Desc(File, "RME_Boot_Vect_Init");
    fprintf(File, "Description : Initialize all the vector endpoints at boot-time.\n");
    fprintf(File, "Input       : rme_ptr_t Cap_Front - The current capability table frontier.\n");
    fprintf(File, "              rme_ptr_t Kmem_Front - The current kernel absolute memory frontier.\n");
    fprintf(File, "Output      : None.\n");
    fprintf(File, "Return      : None.\n");
    fprintf(File, "******************************************************************************/\n");
    fprintf(File, "void RME_Boot_Vect_Init(struct RME_Cap_Captbl* Captbl, rme_ptr_t Cap_Front, rme_ptr_t Kmem_Front)\n");
    fprintf(File, "{\n");
    fprintf(File, "    rme_ptr_t Cur_Addr;\n\n");
    fprintf(File, "    /* The address here shall match what is in the generator */\n");
    fprintf(File, "    RME_ASSERT(Cap_Front==%lld);\n", Proj->RME.Map.Vect_Cap_Front);
    fprintf(File, "    RME_ASSERT(Kmem_Front==0x%llX);\n\n", Proj->RME.Map.Vect_Kmem_Front+Proj->RME.Map.Kmem_Base);
    fprintf(File, "    Cur_Addr=Kmem_Front;\n");
    fprintf(File, "    /* Create all the vector capability tables first */\n");
    for(Obj_Cnt=0;Obj_Cnt<Proj->RVM.Vect_Front;Obj_Cnt+=Capacity)
    {
        if(Proj->RVM.Vect_Front>=(Obj_Cnt+1)*Capacity)
            Captbl_Size=Capacity;
        else
            Captbl_Size=Proj->RVM.Vect_Front%Capacity;

        fprintf(File, "    RME_ASSERT(_RME_Captbl_Boot_Crt(Captbl, RME_BOOT_CAPTBL, RME_BOOT_CTVECT%lld, Cur_Addr, %lld)==0);\n", 
                Obj_Cnt/Capacity,Captbl_Size);
        fprintf(File, "    Cur_Addr+=RME_KOTBL_ROUND(RME_CAPTBL_SIZE(%lld));\n",Captbl_Size);
    }
    fprintf(File, "\n    /* Then all the vectors */\n");
    Obj_Cnt=0;
    for(EACH(struct RVM_Cap_Info*,Info,Proj->RVM.Vect))
    {
        Vect=(struct Vect_Info*)(Info->Cap);
        fprintf(File, "    %s_Vect_Sig=(struct RME_Sig_Struct*)Cur_Addr;\n", Vect->Name);
        fprintf(File, "    RME_ASSERT(_RME_Sig_Boot_Crt(Captbl, RME_BOOT_CTVECT%lld, %s, Cur_Addr)==0);\n",
                      Obj_Cnt/Capacity, Vect->Cap.RME_Macro);
        fprintf(File, "    Cur_Addr+=RME_KOTBL_ROUND(RME_SIG_SIZE);\n");
        Obj_Cnt++;
    }
    fprintf(File, "}\n");
    Write_Func_Footer(File, "RME_Boot_Vect_Init");

    /* Print the interrupt relaying function */
    Write_Func_Desc(File, "RME_Boot_Vect_Handler");
    fprintf(File, "Description : The interrupt handler entry for all the vectors.\n");
    fprintf(File, "Input       : rme_ptr_t Vect_Num - The vector number.\n");
    fprintf(File, "Output      : None.\n");
    fprintf(File, "Return      : rme_ptr_t - The number of signals to send to the generic vector endpoint.\n");
    fprintf(File, "******************************************************************************/\n");
    fprintf(File, "rme_ptr_t RME_Boot_Vect_Handler(rme_ptr_t Vect_Num)\n");
    fprintf(File, "{\n");
    fprintf(File, "    rme_ptr_t Send_Num;\n\n");
    fprintf(File, "    switch(Vect_Num)\n");
    fprintf(File, "    {\n");
    for(EACH(struct RVM_Cap_Info*,Info,Proj->RVM.Vect))
    {
        Vect=(struct Vect_Info*)(Info->Cap);
        fprintf(File, "        /* %s */\n", Vect->Name);
        fprintf(File, "        case %lld:\n", Vect->Num);
        fprintf(File, "        {\n");
        fprintf(File, "            Send_Num=RME_Vect_%s_User(Vect_Num);\n", Vect->Name);
        fprintf(File, "            _RME_Kern_Snd(%s_Vect_Sig);\n", Vect->Name);
        fprintf(File, "            return Send_Num;\n");
        fprintf(File, "        }\n");
    }
    fprintf(File, "        default: break;\n");
    fprintf(File, "    }\n");
    fprintf(File, "    return 1;\n");
    fprintf(File, "}\n");
    Write_Func_Footer(File, "RME_Boot_Vect_Handler");

    /* The rest are interrupt endpoint user preprocessing functions */
    for(EACH(struct RVM_Cap_Info*,Info,Proj->RVM.Vect))
    {
        Vect=(struct Vect_Info*)(Info->Cap);
        sprintf(Buf, "RME_Vect_%s_User", Vect->Name);
        Write_Func_Desc(File, Buf);
        fprintf(File, "Description : The user top-half interrupt handler for %s.\n", Vect->Name);
        fprintf(File, "Input       : rme_ptr_t Int_Num - The interrupt number.\n");
        fprintf(File, "Output      : None.\n");
        fprintf(File, "Return      : rme_ptr_t - The number of signals to send to the generic vector endpoint.\n");
        fprintf(File, "******************************************************************************/\n");
        fprintf(File, "rme_ptr_t RME_Vect_%s_User(rme_ptr_t Int_Num)\n", Vect->Name);
        fprintf(File, "{\n");
        fprintf(File, "    /* Add code here */\n\n");
        fprintf(File, "    return 0;\n");
        fprintf(File, "}\n");
        Write_Func_Footer(File, Buf);
    }

    /* Close the file */
    Write_Src_Footer(File);
    fclose(File);
    Free(Buf);
}
/* End Function:Gen_RME_Boot *************************************************/

/* Begin Function:Gen_RME_User ************************************************
Description : Generate the rme_user.c. This file is mainly responsible for user-
              supplied hooks. If the user needs to add functionality, consider
              modifying this file.
Input       : struct Proj_Info* Proj - The project structure.
              struct Chip_Info* Chip - The chip structure.
              s8_t* RME_Path - The RME root folder path.
              s8_t* Output_Path - The output folder path.
Output      : None.
Return      : None.
******************************************************************************/
void Gen_RME_User(struct Proj_Info* Proj, struct Chip_Info* Chip, s8_t* RME_Path, s8_t* Output_Path)
{
    s8_t* Buf;
    FILE* File;
    
    Buf=Malloc(4096);

    /* Create user stubs - pre initialization and post initialization */
    /* Generate rme_user.c */
    sprintf(Buf, "%s/M7M1_MuEukaron/Project/Source/rme_user.c", Output_Path);
    File=fopen(Buf, "wb");
    if(File==0)
        EXIT_FAIL("rme_user.c open failed.");
    Write_Src_Desc(File, "rme_user.c", "The user hook file.");

    /* Print all header includes */
    fprintf(File, "/* Includes ******************************************************************/\n");
    Print_RME_Inc(File, Proj);
    fprintf(File, "#include \"rme_boot.h\"\n");
    fprintf(File, "/* End Includes **************************************************************/\n\n");

    /* Print all global prototypes */
    fprintf(File, "/* Public C Function Prototypes **********************************************/\n");
    fprintf(File, "void RME_Boot_Pre_Init(void);\n");
    fprintf(File, "void RME_Boot_Post_Init(void);\n");
    fprintf(File, "/* End Public C Function Prototypes ******************************************/\n\n");

    /* Preinitialization of hardware */
    Write_Func_Desc(File, "RME_Boot_Pre_Init");
    fprintf(File, "Description : Initialize critical hardware before any kernel initialization takes place.\n");
    Write_Func_None(File);
    fprintf(File, "void RME_Boot_Pre_Init(void)\n");
    fprintf(File, "{\n");
    fprintf(File, "    /* Add code here */\n");
    fprintf(File, "}\n");
    Write_Func_Footer(File, "RME_Boot_Pre_Init");

    /* Postinitialization of hardware */
    Write_Func_Desc(File, "RME_Boot_Post_Init");
    fprintf(File, "Description : Initialize hardware after all kernel initialization took place.\n");
    Write_Func_None(File);
    fprintf(File, "void RME_Boot_Post_Init(void)\n");
    fprintf(File, "{\n");
    fprintf(File, "    /* Add code here */\n");
    fprintf(File, "}\n");
    Write_Func_Footer(File, "RME_Boot_Post_Init");

    /* Close the file */
    Write_Src_Footer(File);
    fclose(File);
    Free(Buf);
}
/* End Function:Gen_RME_User *************************************************/

/* Begin Function:Print_RVM_Inc ***********************************************
Description : Generate the RVM-related include section.
Input       : FILE* File - The file to print to.
              struct Proj_Info* Proj - The project structure.
Output      : None.
Return      : None.
******************************************************************************/
void Print_RVM_Inc(FILE* File, struct Proj_Info* Proj)
{
    /* Print includes */
    fprintf(File, "#define __HDR_DEFS__\n");
    fprintf(File, "#include \"Platform/%s/rvm_platform_%s.h\"\n", Proj->Plat_Name, Proj->Lower_Plat);
    fprintf(File, "#include \"Init/rvm_syssvc.h\"\n");
    fprintf(File, "#include \"Init/rvm_init.h\"\n");
    fprintf(File, "#undef __HDR_DEFS__\n\n");
    fprintf(File, "#define __HDR_STRUCTS__\n");
    fprintf(File, "#include \"Platform/%s/rme_platform_%s.h\"\n", Proj->Plat_Name, Proj->Lower_Plat);
    fprintf(File, "#include \"Init/rvm_syssvc.h\"\n");
    fprintf(File, "#include \"Init/rvm_init.h\"\n");
    fprintf(File, "#undef __HDR_STRUCTS__\n\n");
    fprintf(File, "#define __HDR_PUBLIC_MEMBERS__\n");
    fprintf(File, "#include \"Platform/%s/rme_platform_%s.h\"\n", Proj->Plat_Name, Proj->Lower_Plat);
    fprintf(File, "#include \"Init/rvm_syssvc.h\"\n");
    fprintf(File, "#include \"Init/rvm_init.h\"\n");
    fprintf(File, "#undef __HDR_PUBLIC_MEMBERS__\n\n");
}
/* End Function:Print_RVM_Inc ************************************************/

/* Begin Function:Cons_RVM_Pgtbl **********************************************
Description : Construct the page table for RVM. This will produce the desired final
              page table tree, and is recursive.
Input       : FILE* File - The file output.
              struct Pgtbl_Info* Pgtbl - The page table structure.
Output      : None.
Return      : None.
******************************************************************************/
void Cons_RVM_Pgtbl(FILE* File, struct Pgtbl_Info* Pgtbl)
{
    ptr_t Count;
    struct Pgtbl_Info* Child;

    /* Construct whatever page table to this page table */
    for(Count=0;Count<POW2(Pgtbl->Num_Order);Count++)
    {
        Child=Pgtbl->Child_Pgdir[Count];
        if(Child==0)
            continue;
        
        fprintf(File, "    RVM_ASSERT(RVM_Pgtbl_Cons(%s, 0x%llX, %s, %s)==0);\n",
                      Pgtbl->Cap.RVM_Macro, Count, Child->Cap.RVM_Macro, "RVM_PGTBL_ALL_PERM");

        /* Recursively call this for all the page tables */
        Cons_RVM_Pgtbl(File, Child);
    }
}
/* End Function:Cons_RVM_Pgtbl ***********************************************/

/* Begin Function:Map_RVM_Pgtbl ***********************************************
Description : Map pages into a page table. This is not recursive.
Input       : FILE* File - The file output.
              struct Pgtbl_Info* Pgtbl - The page table structure.
              ptr_t Init_Size_Ord - The initial page table's size order.
Output      : None.
Return      : None.
******************************************************************************/
void Map_RVM_Pgtbl(FILE* File, struct Pgtbl_Info* Pgtbl, ptr_t Init_Size_Ord)
{
    ptr_t Count;
    ptr_t Attr;
    ptr_t Pos_Src;
    ptr_t Index;
    ptr_t Page_Start;
    ptr_t Page_Size;
    ptr_t Page_Num;
    ptr_t Init_Size;
    s8_t Flags[256];

    Page_Size=POW2(Pgtbl->Size_Order);
    Page_Num=POW2(Pgtbl->Num_Order);
    Init_Size=POW2(Init_Size_Ord);

    /* Map whatever pages into this page table */
    for(Count=0;Count<Page_Num;Count++)
    {
        Attr=Pgtbl->Child_Page[Count];
        if(Attr==0)
            continue;

        /* Compute flags */
        Flags[0]='\0';

        if((Attr&MEM_READ)!=0)
            strcat(Flags,"RVM_PGTBL_READ|");
        if((Attr&MEM_WRITE)!=0)
            strcat(Flags,"RVM_PGTBL_WRITE|");
        if((Attr&MEM_EXECUTE)!=0)
            strcat(Flags,"RVM_PGTBL_EXECUTE|");
        if((Attr&MEM_CACHEABLE)!=0)
            strcat(Flags,"RVM_PGTBL_CACHEABLE|");
        if((Attr&MEM_BUFFERABLE)!=0)
            strcat(Flags,"RVM_PGTBL_BUFFERABLE|");
        if((Attr&MEM_STATIC)!=0)
            strcat(Flags,"RVM_PGTBL_STATIC|");

        Flags[strlen(Flags)-1]='\0';

        /* Compute Pos_Src and Index */
        Page_Start=Pgtbl->Start_Addr+Count*Page_Size;
        Pos_Src=Page_Start/Init_Size;
        Index=(Page_Start-Pos_Src*Init_Size)/Page_Size;

        fprintf(File, "    RVM_ASSERT(RVM_Pgtbl_Add(%s, 0x%llX, \\\n"
                      "                             %s, \\\n"
                      "                             %s, 0x%llX, 0x%llX)==0);\n",
                      Pgtbl->Cap.RVM_Macro, Count, Flags, "RVM_BOOT_PGTBL", Pos_Src, Index);
    }
}
/* End Function:Map_RVM_Pgtbl ************************************************/

/* Begin Function:Init_RVM_Pgtbl **********************************************
Description : Initialize page tables.
Input       : FILE* File - The file output.
              struct Pgtbl_Info* Pgtbl - The page table structure.
              ptr_t Init_Size_Ord - The initial page table's size order.
              ptr_t Init_Num_Ord - The initial page table's number order.
Output      : None.
Return      : None.
******************************************************************************/
void Init_RVM_Pgtbl(FILE* File, struct Proj_Info* Proj)
{
    struct Proc_Info* Proc;
    struct Pgtbl_Info* Pgtbl;
    struct RVM_Cap_Info* Info;

    /* Do page table construction first */
    for(EACH(struct Proc_Info*,Proc,Proj->Proc))
    {
        fprintf(File, "\n    /* Constructing page tables for process: %s */\n",Proc->Name);
        Cons_RVM_Pgtbl(File,Proc->Pgtbl);
    }
    
    /* Then do the mapping for all page tables */
    fprintf(File, "\n    /* Mapping pages into page tables */\n");
    for(EACH(struct RVM_Cap_Info*,Info,Proj->RVM.Pgtbl))
    {
        Pgtbl=Info->Cap;
        Map_RVM_Pgtbl(File, Pgtbl,
                      Proj->Plat.Word_Bits-Proj->Plat.Init_Pgtbl_Num_Ord);
    }
}
/* End Function:Init_RVM_Pgtbl ***********************************************/

/* Begin Function:Gen_RVM_Boot ************************************************
Description : Generate the rvm_boot.h and rvm_boot.c. They are mainly responsible
              for setting up all the kernel objects. If RVM or Posix functionality
              is enabled, these kernel objects will also be handled by such file.
Input       : struct Proj_Info* Proj - The project structure.
              struct Chip_Info* Chip - The chip structure.
              s8_t* RVM_Path - The RVM root folder path.
              s8_t* Output_Path - The output folder path.
Output      : None.
Return      : None.
******************************************************************************/
void Gen_RVM_Boot(struct Proj_Info* Proj, struct Chip_Info* Chip, s8_t* RVM_Path, s8_t* Output_Path)
{
    s8_t* Buf;
    FILE* File;
    ptr_t Obj_Cnt;
    struct RVM_Cap_Info* Info;
    struct Pgtbl_Info* Pgtbl;
    struct Proc_Info* Proc;
    struct Thd_Info* Thd;
    struct Inv_Info* Inv;
    struct Port_Info* Port;
    struct Recv_Info* Recv;
    struct Send_Info* Send;
    struct Vect_Info* Vect;
    ptr_t Cap_Front;
    ptr_t Capacity;
    ptr_t Captbl_Size;

    Buf=Malloc(4096);

    /* Generate rvm_boot.h */
    sprintf(Buf, "%s/M7M2_MuAmmonite/Project/Include/rvm_boot.h", Output_Path);
    File=fopen(Buf, "wb");
    if(File==0)
        EXIT_FAIL("rvm_boot.h open failed.");
    Write_Src_Desc(File, "rvm_boot.h", "The boot-time initialization file header.");
    fprintf(File, "/* Defines *******************************************************************/\n");

    /* Vector capability tables & Vectors */
    Cap_Front=Proj->RME.Map.Vect_Cap_Front;
    Capacity=Proj->Plat.Captbl_Capacity;
    fprintf(File, "/* Vector capability table capability tables */\n");
    for(Obj_Cnt=0;Obj_Cnt<Proj->RVM.Vect_Front;Obj_Cnt+=Capacity)
    {
        sprintf(Buf, "RVM_BOOT_CTVECT%lld",Obj_Cnt/Capacity);
        Make_Define_Int(File, Buf, Cap_Front++, MACRO_ALIGNMENT);
    }
    fprintf(File, "\n/* Vectors */\n");
    for(EACH(struct RVM_Cap_Info*,Info,Proj->RVM.Vect))
    {
        Vect=(struct Vect_Info*)(Info->Cap);
        sprintf(Buf, "RVM_CAPID(RVM_BOOT_CTVECT%lld,%lld)", 
                     Vect->Cap.RVM_Capid/Capacity, Vect->Cap.RVM_Capid%Capacity);
        Make_Define_Str(File, Vect->Cap.RVM_Macro, Buf, MACRO_ALIGNMENT);
    }

    /* There is a gap - the RVM needs to create its own kernel objects */
    /* Captbl capability tables & Captbls */
    Cap_Front=Proj->RVM.Map.Captbl_Cap_Front;
    fprintf(File, "\n/* Process capability table capability tables */\n");
    for(Obj_Cnt=0;Obj_Cnt<Proj->RVM.Captbl_Front;Obj_Cnt+=Capacity)
    {
        sprintf(Buf, "RVM_BOOT_CTCAPTBL%lld",Obj_Cnt/Capacity);
        Make_Define_Int(File, Buf, Cap_Front++, MACRO_ALIGNMENT);
    }
    fprintf(File, "\n/* Process capability tables */\n");
    for(EACH(struct RVM_Cap_Info*,Info,Proj->RVM.Captbl))
    {
        Proc=(struct Proc_Info*)(Info->Cap);
        sprintf(Buf, "RVM_CAPID(RVM_BOOT_CTCAPTBL%lld,%lld)", 
                     Proc->Captbl_Cap.RVM_Capid/Capacity, Proc->Captbl_Cap.RVM_Capid%Capacity);
        Make_Define_Str(File, Proc->Captbl_Cap.RVM_Macro, Buf, MACRO_ALIGNMENT);
    }
    if(Cap_Front!=Proj->RVM.Map.Pgtbl_Cap_Front)
        EXIT_FAIL("Internal capability table computation failure.");

    /* Pgtbl capability tables & Pgtbls */
    Cap_Front=Proj->RVM.Map.Pgtbl_Cap_Front;
    fprintf(File, "\n/* Process page table capability tables */\n");
    for(Obj_Cnt=0;Obj_Cnt<Proj->RVM.Pgtbl_Front;Obj_Cnt+=Capacity)
    {
        sprintf(Buf, "RVM_BOOT_CTPGTBL%lld",Obj_Cnt/Capacity);
        Make_Define_Int(File, Buf, Cap_Front++, MACRO_ALIGNMENT);
    }
    fprintf(File, "\n/* Process page tables */\n");
    for(EACH(struct RVM_Cap_Info*,Info,Proj->RVM.Pgtbl))
    {
        Pgtbl=(struct Pgtbl_Info*)(Info->Cap);
        sprintf(Buf, "RVM_CAPID(RVM_BOOT_CTPGTBL%lld,%lld)", 
                     Pgtbl->Cap.RVM_Capid/Capacity, Pgtbl->Cap.RVM_Capid%Capacity);
        Make_Define_Str(File, Pgtbl->Cap.RVM_Macro, Buf, MACRO_ALIGNMENT);
    }
    if(Cap_Front!=Proj->RVM.Map.Proc_Cap_Front)
        EXIT_FAIL("Internal capability table computation failure.");

    /* Process capability tables & Processes */
    fprintf(File, "\n/* Process capability tables */\n");
    for(Obj_Cnt=0;Obj_Cnt<Proj->RVM.Proc_Front;Obj_Cnt+=Capacity)
    {
        sprintf(Buf, "RVM_BOOT_CTPROC%lld",Obj_Cnt/Capacity);
        Make_Define_Int(File, Buf, Cap_Front++, MACRO_ALIGNMENT);
    }
    fprintf(File, "\n/* Processes */\n");
    for(EACH(struct RVM_Cap_Info*,Info,Proj->RVM.Proc))
    {
        Proc=(struct Proc_Info*)(Info->Cap);
        sprintf(Buf, "RVM_CAPID(RVM_BOOT_CTPROC%lld,%lld)",
                     Proc->Proc_Cap.RVM_Capid/Capacity, Proc->Proc_Cap.RVM_Capid%Capacity);
        Make_Define_Str(File, Proc->Proc_Cap.RVM_Macro, Buf, MACRO_ALIGNMENT);
    }
    if(Cap_Front!=Proj->RVM.Map.Thd_Cap_Front)
        EXIT_FAIL("Internal capability table computation failure.");

    /* Thread capability tables & Threads */
    fprintf(File, "\n/* Thread capability tables */\n");
    for(Obj_Cnt=0;Obj_Cnt<Proj->RVM.Thd_Front;Obj_Cnt+=Capacity)
    {
        sprintf(Buf, "RVM_BOOT_CTTHD%lld",Obj_Cnt/Capacity);
        Make_Define_Int(File, Buf, Cap_Front++, MACRO_ALIGNMENT);
    }
    fprintf(File, "\n/* Threads */\n");
    for(EACH(struct RVM_Cap_Info*,Info,Proj->RVM.Thd))
    {
        Thd=(struct Thd_Info*)(Info->Cap);
        sprintf(Buf, "RVM_CAPID(RVM_BOOT_CTTHD%lld,%lld)",
                     Thd->Cap.RVM_Capid/Capacity, Thd->Cap.RVM_Capid%Capacity);
        Make_Define_Str(File, Thd->Cap.RVM_Macro, Buf, MACRO_ALIGNMENT);
    }
    if(Cap_Front!=Proj->RVM.Map.Inv_Cap_Front)
        EXIT_FAIL("Internal capability table computation failure.");

    /* Invocation capability tables & Invocations */
    fprintf(File, "\n/* Invocation capability tables */\n");
    for(Obj_Cnt=0;Obj_Cnt<Proj->RVM.Inv_Front;Obj_Cnt+=Capacity)
    {
        sprintf(Buf, "RVM_BOOT_CTINV%lld",Obj_Cnt/Capacity);
        Make_Define_Int(File, Buf, Cap_Front++, MACRO_ALIGNMENT);
    }
    fprintf(File, "\n/* Invocations */\n");
    for(EACH(struct RVM_Cap_Info*,Info,Proj->RVM.Inv))
    {
        Inv=(struct Inv_Info*)(Info->Cap);
        sprintf(Buf, "RVM_CAPID(RVM_BOOT_CTINV%lld,%lld)",
                     Inv->Cap.RVM_Capid/Capacity, Inv->Cap.RVM_Capid%Capacity);
        Make_Define_Str(File, Inv->Cap.RVM_Macro, Buf, MACRO_ALIGNMENT);
    }
    if(Cap_Front!=Proj->RVM.Map.Recv_Cap_Front)
        EXIT_FAIL("Internal capability table computation failure.");

    /* Receive endpoint capability tables & Receive endpoints */
    fprintf(File, "\n/* Receive endpoint capability tables */\n");
    for(Obj_Cnt=0;Obj_Cnt<Proj->RVM.Recv_Front;Obj_Cnt+=Capacity)
    {
        sprintf(Buf, "RVM_BOOT_CTRECV%lld",Obj_Cnt/Capacity);
        Make_Define_Int(File, Buf, Cap_Front++, MACRO_ALIGNMENT);
    }
    fprintf(File, "\n/* Receive endpoints */\n");
    for(EACH(struct RVM_Cap_Info*,Info,Proj->RVM.Recv))
    {
        Recv=(struct Recv_Info*)(Info->Cap);
        sprintf(Buf, "RVM_CAPID(RVM_BOOT_CTRECV%lld,%lld)",
                     Recv->Cap.RVM_Capid/Capacity, Recv->Cap.RVM_Capid%Capacity);
        Make_Define_Str(File, Recv->Cap.RVM_Macro, Buf, MACRO_ALIGNMENT);
    }
    if(Cap_Front!=Proj->RVM.Map.After_Cap_Front)
        EXIT_FAIL("Internal capability table computation failure.");
    
    /* Extra capability table frontier */
    fprintf(File, "\n/* Capability table frontier */\n");
    sprintf(Buf, "%lld",Proj->RVM.Map.After_Cap_Front);
    Make_Define_Str(File, "RVM_BOOT_CAP_FRONTIER", Buf, MACRO_ALIGNMENT);
    /* Extra kernel memory frontier */
    fprintf(File, "\n/* Kernel memory frontier */\n");
    sprintf(Buf, "0x%llX",Proj->RVM.Map.After_Kmem_Front);
    Make_Define_Str(File, "RVM_BOOT_KMEM_FRONTIER", Buf, MACRO_ALIGNMENT);

    /* Finish file generation */
    fprintf(File, "/* End Defines ***************************************************************/\n\n");
    Write_Src_Footer(File);
    fclose(File);

    /* Generate rvm_boot.c */
    sprintf(Buf, "%s/M7M2_MuAmmonite/Project/Source/rvm_boot.c", Output_Path);
    File=fopen(Buf, "wb");
    if(File==0)
        EXIT_FAIL("rvm_boot.c open failed.");
    Write_Src_Desc(File, "rvm_boot.c", "The boot-time initialization file.");

    /* Print all header includes */
    fprintf(File, "/* Includes ******************************************************************/\n");
    Print_RVM_Inc(File, Proj);
    fprintf(File, "#include \"rvm_boot.h\"\n");
    fprintf(File, "/* End Includes **************************************************************/\n\n");

    /* Print all global variables and prototypes */
    fprintf(File, "/* Private C Function Prototypes *********************************************/\n");
    fprintf(File, "/* Kernel object creation */\n");
    fprintf(File, "static void RVM_Boot_Captbl_Crt(void);\n");
    fprintf(File, "static void RVM_Boot_Pgtbl_Crt(void);\n");
    fprintf(File, "static void RVM_Boot_Proc_Crt(void);\n");
    fprintf(File, "static void RVM_Boot_Inv_Crt(void);\n");
    fprintf(File, "static void RVM_Boot_Recv_Crt(void);\n\n");
    fprintf(File, "/* Kernel object initialization */\n");
    fprintf(File, "static void RVM_Boot_Captbl_Init(void);\n");
    fprintf(File, "static void RVM_Boot_Pgtbl_Init(void);\n");
    fprintf(File, "static void RVM_Boot_Proc_Init(void);\n");
    fprintf(File, "static void RVM_Boot_Inv_Init(void);\n");
    fprintf(File, "static void RVM_Boot_Recv_Init(void);\n");
    fprintf(File, "/* End Private C Function Prototypes *****************************************/\n\n");
    fprintf(File, "/* Public C Function Prototypes **********************************************/\n");
    fprintf(File, "void RVM_Boot_Kobj_Crt(void);\n");
    fprintf(File, "void RVM_Boot_Kobj_Init(void);\n");
    fprintf(File, "/* End Public C Function Prototypes ******************************************/\n\n");

    /* Capability table creation */
    Write_Func_Desc(File, "RVM_Boot_Captbl_Crt");
    fprintf(File, "Description : Create all capability tables at boot-time.\n");
    Write_Func_None(File);
    fprintf(File, "void RVM_Boot_Captbl_Crt(void)\n");
    fprintf(File, "{\n");
    fprintf(File, "    rme_ptr_t Cur_Addr;\n\n");
    fprintf(File, "    Cur_Addr==0x%llX;\n\n", Proj->RVM.Map.Captbl_Kmem_Front);
    fprintf(File, "    /* Create all the capability table capability tables first */\n");
    for(Obj_Cnt=0;Obj_Cnt<Proj->RVM.Captbl_Front;Obj_Cnt+=Capacity)
    {
        if(Proj->RVM.Captbl_Front>=(Obj_Cnt+1)*Capacity)
            Captbl_Size=Capacity;
        else
            Captbl_Size=Proj->RVM.Captbl_Front%Capacity;

        fprintf(File, "    RVM_ASSERT(RVM_Captbl_Crt(RVM_BOOT_CAPTBL, RVM_BOOT_INIT_KMEM, RVM_BOOT_CTCAPTBL%lld, Cur_Addr, %lld)==0);\n", 
                Obj_Cnt/Capacity,Captbl_Size);
        fprintf(File, "    Cur_Addr+=RVM_KOTBL_ROUND(RVM_CAPTBL_SIZE(%lld));\n",Captbl_Size);
    }
    fprintf(File, "\n    /* Then the capability tables themselves */\n");
    for(EACH(struct RVM_Cap_Info*,Info,Proj->RVM.Captbl))
    {
        Proc=(struct Proc_Info*)(Info->Cap);

        fprintf(File, "    RVM_ASSERT(RVM_Captbl_Crt(RVM_BOOT_CTCAPTBL%lld, RVM_BOOT_INIT_KMEM, %lld, Cur_Addr, %lld)==0);\n",
                Proc->Captbl_Cap.RVM_Capid/Capacity, Proc->Captbl_Cap.RVM_Capid%Capacity, Proc->Captbl_Front+Proc->Extra_Captbl);
        fprintf(File, "    Cur_Addr+=RME_KOTBL_ROUND(RVM_CAPTBL_SIZE(%lld));\n", Proc->Captbl_Front+Proc->Extra_Captbl);
    }

    fprintf(File, "\n    RME_ASSERT(Cur_Addr==0x%llX);\n", Proj->RVM.Map.Pgtbl_Kmem_Front);
    fprintf(File, "}\n");
    Write_Func_Footer(File, "RVM_Boot_Captbl_Crt");

    /* Page table creation */
    Write_Func_Desc(File, "RVM_Boot_Pgtbl_Crt");
    fprintf(File, "Description : Create all page tables at boot-time.\n");
    Write_Func_None(File);
    fprintf(File, "void RVM_Boot_Pgtbl_Crt(void)\n");
    fprintf(File, "{\n");
    fprintf(File, "    rme_ptr_t Cur_Addr;\n\n");
    fprintf(File, "    Cur_Addr==0x%llX;\n\n", Proj->RVM.Map.Pgtbl_Kmem_Front);
    fprintf(File, "    /* Create all the page tables capability tables first */\n");
    for(Obj_Cnt=0;Obj_Cnt<Proj->RVM.Pgtbl_Front;Obj_Cnt+=Capacity)
    {
        if(Proj->RVM.Pgtbl_Front>=(Obj_Cnt+1)*Capacity)
            Captbl_Size=Capacity;
        else
            Captbl_Size=Proj->RVM.Pgtbl_Front%Capacity;

        fprintf(File, "    RVM_ASSERT(RVM_Captbl_Crt(RVM_BOOT_CAPTBL, RVM_BOOT_INIT_KMEM, RVM_BOOT_CTPGTBL%lld, Cur_Addr, %lld)==0);\n", 
                Obj_Cnt/Capacity,Captbl_Size);
        fprintf(File, "    Cur_Addr+=RVM_KOTBL_ROUND(RVM_CAPTBL_SIZE(%lld));\n",Captbl_Size);
    }
    fprintf(File, "\n    /* Then the page tables themselves */\n");
    for(EACH(struct RVM_Cap_Info*,Info,Proj->RVM.Pgtbl))
    {
        Pgtbl=Info->Cap;

        fprintf(File, "    RVM_ASSERT(RVM_Pgtbl_Crt(RVM_BOOT_CTPGTBL%lld, RVM_BOOT_INIT_KMEM, %lld, Cur_Addr, 0x%llX, %lld, %lld, %lld)==0);\n",
                Proc->Captbl_Cap.RVM_Capid/Capacity, Proc->Captbl_Cap.RVM_Capid%Capacity,
                Pgtbl->Start_Addr,(ptr_t)(Pgtbl->Is_Top!=0),Pgtbl->Size_Order, Pgtbl->Num_Order);

        if(Pgtbl->Is_Top!=0)
            fprintf(File, "    Cur_Addr+=RME_KOTBL_ROUND(RVM_PGTBL_SIZE_TOP(%lld));\n", Pgtbl->Num_Order);
        else
            fprintf(File, "    Cur_Addr+=RME_KOTBL_ROUND(RVM_PGTBL_SIZE_NOM(%lld));\n", Pgtbl->Num_Order);
    }

    fprintf(File, "\n    RME_ASSERT(Cur_Addr==0x%llX);\n", Proj->RVM.Map.Proc_Kmem_Front);
    fprintf(File, "}\n");
    Write_Func_Footer(File, "RVM_Boot_Pgtbl_Crt");

    /* Process creation */
    Write_Func_Desc(File, "RVM_Boot_Proc_Crt");
    fprintf(File, "Description : Create all processes at boot-time.\n");
    Write_Func_None(File);
    fprintf(File, "void RVM_Boot_Proc_Crt(void)\n");
    fprintf(File, "{\n");
    fprintf(File, "    rme_ptr_t Cur_Addr;\n\n");
    fprintf(File, "    Cur_Addr=0x%llX;\n\n", Proj->RVM.Map.Proc_Kmem_Front);
    fprintf(File, "    /* Create all the process capability tables first */\n");
    for(Obj_Cnt=0;Obj_Cnt<Proj->RVM.Proc_Front;Obj_Cnt+=Capacity)
    {
        if(Proj->RVM.Proc_Front>=(Obj_Cnt+1)*Capacity)
            Captbl_Size=Capacity;
        else
            Captbl_Size=Proj->RVM.Proc_Front%Capacity;

        fprintf(File, "    RVM_ASSERT(RVM_Captbl_Crt(RVM_BOOT_CAPTBL, RVM_BOOT_INIT_KMEM, RVM_BOOT_CTPROC%lld, Cur_Addr, %lld)==0);\n", 
                Obj_Cnt/Capacity,Captbl_Size);
        fprintf(File, "    Cur_Addr+=RVM_KOTBL_ROUND(RVM_CAPTBL_SIZE(%lld));\n",Captbl_Size);
    }
    fprintf(File, "\n    /* Then the processes themselves */\n");
    for(EACH(struct RVM_Cap_Info*,Info,Proj->RVM.Proc))
    {
        Proc=(struct Proc_Info*)(Info->Cap);
        fprintf(File, "    RVM_ASSERT(RVM_Proc_Crt(RVM_BOOT_CTPROC%lld, RVM_BOOT_INIT_KMEM, %lld, %s, %s, Cur_Addr)==0);\n",
                Proc->Proc_Cap.RVM_Capid/Capacity, Proc->Proc_Cap.RVM_Capid%Capacity, Proc->Proc_Cap.RVM_Macro, Proc->Pgtbl->Cap.RVM_Macro);
        fprintf(File, "    Cur_Addr+=RME_KOTBL_ROUND(RVM_PROC_SIZE);\n");
    }
    fprintf(File, "\n    RME_ASSERT(Cur_Addr==0x%llX);\n", Proj->RVM.Map.Thd_Kmem_Front);
    fprintf(File, "}\n");
    Write_Func_Footer(File, "RVM_Boot_Proc_Crt");

    /* Thread creation */
    Write_Func_Desc(File, "RVM_Boot_Thd_Crt");
    fprintf(File, "Description : Create all threads at boot-time.\n");
    Write_Func_None(File);
    fprintf(File, "void RVM_Boot_Thd_Init(void)\n");
    fprintf(File, "{\n");
    fprintf(File, "    rme_ptr_t Cur_Addr;\n\n");
    fprintf(File, "    Cur_Addr=0x%llX;\n\n", Proj->RVM.Map.Thd_Kmem_Front);
    fprintf(File, "    /* Create all the thread capability tables first */\n");
    for(Obj_Cnt=0;Obj_Cnt<Proj->RVM.Thd_Front;Obj_Cnt+=Capacity)
    {
        if(Proj->RVM.Thd_Front>=(Obj_Cnt+1)*Capacity)
            Captbl_Size=Capacity;
        else
            Captbl_Size=Proj->RVM.Thd_Front%Capacity;

        fprintf(File, "    RVM_ASSERT(RVM_Captbl_Crt(RVM_BOOT_CAPTBL, RVM_BOOT_INIT_KMEM, RVM_BOOT_CTTHD%lld, Cur_Addr, %lld)==0);\n", 
                Obj_Cnt/Capacity,Captbl_Size);
        fprintf(File, "    Cur_Addr+=RVM_KOTBL_ROUND(RVM_CAPTBL_SIZE(%lld));\n",Captbl_Size);
    }
    fprintf(File, "\n    /* Then the threads themselves */\n");
    for(EACH(struct RVM_Cap_Info*,Info,Proj->RVM.Thd))
    {
        Thd=(struct Thd_Info*)(Info->Cap);
        Proc=Info->Proc;
        fprintf(File, "    RVM_ASSERT(RVM_Thd_Crt(RVM_BOOT_CTTHD%lld, RVM_BOOT_INIT_KMEM, %lld, %s, %lld, Cur_Addr)==0);\n",
                Thd->Cap.RVM_Capid/Capacity, Thd->Cap.RVM_Capid%Capacity, Proc->Proc_Cap.RVM_Macro, Thd->Priority);
        fprintf(File, "    Cur_Addr+=RME_KOTBL_ROUND(RVM_THD_SIZE);\n");
    }
    fprintf(File, "\n    RME_ASSERT(Cur_Addr==0x%llX);\n", Proj->RVM.Map.Inv_Kmem_Front);
    fprintf(File, "}\n");
    Write_Func_Footer(File, "RVM_Boot_Thd_Crt");

    /* Invocation creation */
    Write_Func_Desc(File, "RVM_Boot_Inv_Crt");
    fprintf(File, "Description : Create all invocations at boot-time.\n");
    Write_Func_None(File);
    fprintf(File, "void RVM_Boot_Inv_Crt(void)\n");
    fprintf(File, "{\n");
    fprintf(File, "    rme_ptr_t Cur_Addr;\n\n");
    fprintf(File, "    Cur_Addr=0x%llX;\n\n", Proj->RVM.Map.Inv_Kmem_Front);
    fprintf(File, "    /* Create all the invocation capability tables first */\n");
    for(Obj_Cnt=0;Obj_Cnt<Proj->RVM.Inv_Front;Obj_Cnt+=Capacity)
    {
        if(Proj->RVM.Inv_Front>=(Obj_Cnt+1)*Capacity)
            Captbl_Size=Capacity;
        else
            Captbl_Size=Proj->RVM.Inv_Front%Capacity;

        fprintf(File, "    RVM_ASSERT(RVM_Captbl_Crt(RVM_BOOT_CAPTBL, RVM_BOOT_INIT_KMEM, RVM_BOOT_CTINV%lld, Cur_Addr, %lld)==0);\n", 
                Obj_Cnt/Capacity,Captbl_Size);
        fprintf(File, "    Cur_Addr+=RVM_KOTBL_ROUND(RVM_CAPTBL_SIZE(%lld));\n",Captbl_Size);
    }
    fprintf(File, "\n    /* Then the invocations themselves */\n");
    for(EACH(struct RVM_Cap_Info*,Info,Proj->RVM.Inv))
    {
        Inv=(struct Inv_Info*)(Info->Cap);
        Proc=Info->Proc;
        fprintf(File, "    RVM_ASSERT(RVM_Inv_Crt(RVM_BOOT_CTINV%lld, RVM_BOOT_INIT_KMEM, %lld, %s, Cur_Addr)==0);\n",
                Inv->Cap.RVM_Capid/Capacity, Inv->Cap.RVM_Capid%Capacity, Proc->Proc_Cap.RVM_Macro);
        fprintf(File, "    Cur_Addr+=RME_KOTBL_ROUND(RVM_INV_SIZE);\n");
    }
    fprintf(File, "\n    RME_ASSERT(Cur_Addr==0x%llX);\n", Proj->RVM.Map.Recv_Kmem_Front);
    fprintf(File, "}\n");
    Write_Func_Footer(File, "RVM_Boot_Inv_Crt");

    /* Receive endpoint creation */
    Write_Func_Desc(File, "RVM_Boot_Recv_Crt");
    fprintf(File, "Description : Create all receive endpoints at boot-time.\n");
    Write_Func_None(File);
    fprintf(File, "void RVM_Boot_Recv_Crt(void)\n");
    fprintf(File, "{\n");
    fprintf(File, "    rme_ptr_t Cur_Addr;\n\n");
    fprintf(File, "    Cur_Addr=0x%llX;\n\n", Proj->RVM.Map.Recv_Kmem_Front);
    fprintf(File, "    /* Create all the receive endpoint capability tables first */\n");
    for(Obj_Cnt=0;Obj_Cnt<Proj->RVM.Recv_Front;Obj_Cnt+=Capacity)
    {
        if(Proj->RVM.Recv_Front>=(Obj_Cnt+1)*Capacity)
            Captbl_Size=Capacity;
        else
            Captbl_Size=Proj->RVM.Recv_Front%Capacity;

        fprintf(File, "    RVM_ASSERT(RVM_Captbl_Crt(RVM_BOOT_CAPTBL, RVM_BOOT_INIT_KMEM, RVM_BOOT_CTRECV%lld, Cur_Addr, %lld)==0);\n", 
                Obj_Cnt/Capacity,Captbl_Size);
        fprintf(File, "    Cur_Addr+=RVM_KOTBL_ROUND(RVM_CAPTBL_SIZE(%lld));\n",Captbl_Size);
    }
    fprintf(File, "\n    /* Then the receive endpoints themselves */\n");
    for(EACH(struct RVM_Cap_Info*,Info,Proj->RVM.Recv))
    {
        Recv=(struct Recv_Info*)(Info->Cap);
        fprintf(File, "    RVM_ASSERT(RVM_Sig_Crt(RVM_BOOT_CTRECV%lld, RVM_BOOT_INIT_KMEM, %lld, Cur_Addr)==0);\n",
                Recv->Cap.RVM_Capid/Capacity, Recv->Cap.RVM_Capid%Capacity);
        fprintf(File, "    Cur_Addr+=RME_KOTBL_ROUND(RVM_SIG_SIZE);\n");
    }
    fprintf(File, "\n    RME_ASSERT(Cur_Addr==0x%llX);\n", Proj->RVM.Map.After_Kmem_Front);
    fprintf(File, "}\n");
    Write_Func_Footer(File, "RVM_Boot_Recv_Crt");

    /* Main creation function */
    Write_Func_Desc(File, "RVM_Boot_Kobj_Crt");
    fprintf(File, "Description : Create all kernel objects at boot-time.\n");
    Write_Func_None(File);
    fprintf(File, "void RVM_Boot_Kobj_Crt(void)\n");
    fprintf(File, "{\n");
    fprintf(File, "    RVM_Boot_Captbl_Crt();\n");
    fprintf(File, "    RVM_Boot_Pgtbl_Crt();\n");
    fprintf(File, "    RVM_Boot_Proc_Crt();\n");
    fprintf(File, "    RVM_Boot_Thd_Crt();\n");
    fprintf(File, "    RVM_Boot_Inv_Crt();\n");
    fprintf(File, "    RVM_Boot_Recv_Crt();\n");
    fprintf(File, "}\n");
    Write_Func_Footer(File, "RVM_Boot_Kobj_Crt");
 
    /* Capability table initialization */
    Write_Func_Desc(File, "RVM_Boot_Captbl_Init");
    fprintf(File, "Description : Initialize the capability tables of all processes.\n");
    Write_Func_None(File);
    fprintf(File, "void RVM_Boot_Captbl_Init(void)\n");
    fprintf(File, "{");
    for(EACH(struct Proc_Info*,Proc,Proj->Proc))
    {
        fprintf(File, "\n    /* Initializing captbl for process: %s */\n", Proc->Name);

        /* Ports */
        fprintf(File, "    /* Ports */\n");
        for(EACH(struct Port_Info*,Port,Proc->Port))
        {
            fprintf(File, "    RVM_ASSERT(RVM_Captbl_Add(%s, %lld, RVM_CTINV%lld, %lld, %s)==0);\n",
                    Proc->Captbl_Cap.RVM_Macro, Port->Cap.Loc_Capid, Port->Cap.RVM_Capid/Capacity, Port->Cap.RVM_Capid%Capacity,
                    "RME_INV_FLAG_ACT");
        }

        /* Receive endpoints */
        fprintf(File, "    /* Receive endpoints */\n");
        for(EACH(struct Recv_Info*,Recv,Proc->Recv))
        {
            fprintf(File, "    RVM_ASSERT(RVM_Captbl_Add(%s, %lld, RVM_CTRECV%lld, %lld, %s)==0);\n",
                    Proc->Captbl_Cap.RVM_Macro, Recv->Cap.Loc_Capid, Recv->Cap.RVM_Capid/Capacity, Recv->Cap.RVM_Capid%Capacity,
                    "RME_SIG_FLAG_SND|RME_SIG_FLAG_RCV");
        }

        /* Send endpoints */
        fprintf(File, "    /* Send endpoints */\n");
        for(EACH(struct Send_Info*,Send,Proc->Send))
        {
            fprintf(File, "    RVM_ASSERT(RVM_Captbl_Add(%s, %lld, RVM_CTRECV%lld, %lld, %s)==0);\n",
                    Proc->Captbl_Cap.RVM_Macro, Send->Cap.Loc_Capid, Send->Cap.RVM_Capid/Capacity, Send->Cap.RVM_Capid%Capacity,
                    "RME_SIG_FLAG_SND");
        }

        /* Vector endpoints */
        fprintf(File, "    /* Vector endpoints */\n");
        for(EACH(struct Vect_Info*,Vect,Proc->Vect))
        {
            fprintf(File, "    RVM_ASSERT(RVM_Captbl_Add(%s, %lld, RVM_CTVECT%lld, %lld, %s)==0);\n",
                    Proc->Captbl_Cap.RVM_Macro, Vect->Cap.Loc_Capid, Vect->Cap.RVM_Capid/Capacity, Vect->Cap.RVM_Capid%Capacity,
                    "RME_SIG_FLAG_RCV");
        }
    }
    fprintf(File, "}\n");
    Write_Func_Footer(File, "RVM_Boot_Captbl_Init");

    /* Page table initialization */
    Write_Func_Desc(File, "RVM_Boot_Pgtbl_Init");
    fprintf(File, "Description : Initialize the page tables of all processes.\n");
    Write_Func_None(File);
    fprintf(File, "void RVM_Boot_Pgtbl_Init(void)\n");
    fprintf(File, "{\n");
    /* Recursively print all page table initialization routine */
    Init_RVM_Pgtbl(File,Proj);
    fprintf(File, "}\n");
    Write_Func_Footer(File, "RVM_Boot_Pgtbl_Init");

    /* Thread initialization */
    Write_Func_Desc(File, "RVM_Boot_Thd_Init");
    fprintf(File, "Description : Initialize the all threads.\n");
    Write_Func_None(File);
    fprintf(File, "void RVM_Boot_Thd_Init(void)\n");
    fprintf(File, "{\n");
    fprintf(File, "    rvm_ptr_t Init_Stack_Addr;\n");
    for(EACH(struct Proc_Info*,Proc,Proj->Proc))
    {
        fprintf(File, "    \n    /* Initializing thread for process: %s */\n", Proc->Name);
        
        for(EACH(struct Thd_Info*,Thd,Proc->Thd))
        {
            fprintf(File, "    RVM_ASSERT(RVM_Thd_Sched_Bind(%s, RVM_INIT_GUARD_THD, RVM_INIT_GUARD_SIG, %s, %lld)==0);\n",
                    Thd->Cap.RVM_Macro, Thd->Cap.RVM_Macro, Thd->Priority);
            fprintf(File, "    Init_Stack_Addr=RVM_Stack_Init(0x%llX, 0x%llX, 0x%llX, 0x%llX);\n",
                    Thd->Map.Stack_Base, Thd->Map.Stack_Size, Thd->Map.Entry_Addr, Proc->Map.Entry_Code_Front);
            fprintf(File, "    RVM_ASSERT(RVM_Thd_Exec_Set(%s, 0x%llX, Init_Stack_Addr, 0x%llX)==0);\n",
                    Thd->Cap.RVM_Macro, Thd->Map.Entry_Addr, Thd->Map.Param_Value);
        }
    }
    fprintf(File, "}\n");
    Write_Func_Footer(File, "RVM_Boot_Thd_Init");

    /* Invocation initialization */
    Write_Func_Desc(File, "RVM_Boot_Inv_Init");
    fprintf(File, "Description : Initialize the all invocations.\n");
    Write_Func_None(File);
    fprintf(File, "void RVM_Boot_Inv_Init(void)\n");
    fprintf(File, "{\n");
    fprintf(File, "    rvm_ptr_t Init_Stack_Addr;\n");
    for(EACH(struct Proc_Info*,Proc,Proj->Proc))
    {
        fprintf(File, "\n    /* Initializing invocation for process: %s */\n", Proc->Name);
        
        for(EACH(struct Inv_Info*,Inv,Proc->Inv))
        {
            fprintf(File, "    Init_Stack_Addr=RVM_Stack_Init(0x%llX, 0x%llX, 0x%llX, 0x%llX);\n",
                    Inv->Map.Stack_Base, Inv->Map.Stack_Size, Inv->Map.Entry_Addr, Proc->Map.Entry_Code_Front);
            /* We always return directly on fault for MCUs, because RVM does not do fault handling there */
            fprintf(File, "    RVM_ASSERT(RVM_Inv_Set(%s, 0x%llX, Init_Stack_Addr, 1)==0);\n",
                    Inv->Cap.RVM_Macro, Inv->Map.Entry_Addr);
        }
    }
    fprintf(File, "}\n");
    Write_Func_Footer(File, "RVM_Boot_Inv_Init");

    /* Receive endpoint initialization - no need at all */

    /* Main initialization function */
    Write_Func_Desc(File, "RVM_Boot_Kobj_Init");
    fprintf(File, "Description : Initialize all kernel objects at boot-time.\n");
    Write_Func_None(File);
    fprintf(File, "void RVM_Boot_Kobj_Init(void)\n");
    fprintf(File, "{\n");
    fprintf(File, "    RVM_Boot_Captbl_Init();\n");
    fprintf(File, "    RVM_Boot_Pgtbl_Init();\n");
    fprintf(File, "    RVM_Boot_Thd_Init();\n");
    fprintf(File, "    RVM_Boot_Inv_Init();\n");
    fprintf(File, "    RVM_Boot_Recv_Init();\n");
    fprintf(File, "}\n");
    Write_Func_Footer(File, "RVM_Boot_Kobj_Init");

    /* Close the file */
    Write_Src_Footer(File);
    fclose(File);
    Free(Buf);
}
/* End Function:Gen_RVM_Boot *************************************************/

/* Begin Function:Gen_RVM_User ************************************************
Description : Generate the rvm_user.c. This file is mainly responsible for user-
              supplied hooks. If the user needs to add functionality, consider
              modifying this file.
Input       : struct Proj_Info* Proj - The project structure.
              struct Chip_Info* Chip - The chip structure.
              s8_t* RVM_Path - The RVM root folder path.
              s8_t* Output_Path - The output folder path.
Output      : None.
Return      : None.
******************************************************************************/
void Gen_RVM_User(struct Proj_Info* Proj, struct Chip_Info* Chip, s8_t* RVM_Path, s8_t* Output_Path)
{
    s8_t* Buf;
    FILE* File;
    
    Buf=Malloc(4096);

    /* Create user stubs - pre initialization and post initialization */
    /* Generate rvm_user.c */
    sprintf(Buf, "%s/M7M2_MuAmmonite/Project/Source/rvm_user.c", Output_Path);
    File=fopen(Buf, "wb");
    if(File==0)
        EXIT_FAIL("rvm_user.c open failed.");
    Write_Src_Desc(File, "rvm_user.c", "The user hook file.");

    /* Print all header includes */
    fprintf(File, "/* Includes ******************************************************************/\n");
    Print_RVM_Inc(File, Proj);
    fprintf(File, "#include \"rvm_boot.h\"\n");
    fprintf(File, "/* End Includes **************************************************************/\n\n");

    /* Print all global prototypes */
    fprintf(File, "/* Public C Function Prototypes **********************************************/\n");
    fprintf(File, "void RVM_Boot_Pre_Init(void);\n");
    fprintf(File, "void RVM_Boot_Post_Init(void);\n");
    fprintf(File, "/* End Public C Function Prototypes ******************************************/\n\n");

    /* Preinitialization of hardware */
    Write_Func_Desc(File, "RVM_Boot_Pre_Init");
    fprintf(File, "Description : Initialize critical hardware before any kernel object creation takes place.\n");
    Write_Func_None(File);
    fprintf(File, "void RVM_Boot_Pre_Init(void)\n");
    fprintf(File, "{\n");
    fprintf(File, "    /* Add code here */\n");
    fprintf(File, "}\n");
    Write_Func_Footer(File, "RVM_Boot_Pre_Init");

    /* Postinitialization of hardware */
    Write_Func_Desc(File, "RVM_Boot_Post_Init");
    fprintf(File, "Description : Initialize hardware after all kernel object creation took place.\n");
    Write_Func_None(File);
    fprintf(File, "void RVM_Boot_Post_Init(void)\n");
    fprintf(File, "{\n");
    fprintf(File, "    /* Add code here */\n");
    fprintf(File, "}\n");
    Write_Func_Footer(File, "RVM_Boot_Post_Init");

    /* Close the file */
    Write_Src_Footer(File);
    fclose(File);
    Free(Buf);
}
/* End Function:Gen_RVM_User *************************************************/

/* Begin Function:main ********************************************************
Description : The entry of the tool.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
int main(int argc, char* argv[])
{
	/* The command line arguments */
	s8_t* Input_Path;
	s8_t* Output_Path;
	s8_t* RME_Path;
	s8_t* RVM_Path;
	s8_t* Format;
	/* The input buffer */
	s8_t* Input_Buf;
    /* The path synthesis buffer */
    s8_t* Path_Buf;
	/* The project and chip pointers */
	struct Proj_Info* Proj;
	struct Chip_Info* Chip;

	/* Initialize memory pool */
	List_Crt(&Mem_List);

/* Phase 1: Process command line and do parsing ******************************/
    /* Process the command line first */
    Cmdline_Proc(argc,argv, &Input_Path, &Output_Path, &RME_Path, &RVM_Path, &Format);
	/* Read project XML file */
	Input_Buf=Read_File(Input_Path);
	Proj=Parse_Proj(Input_Buf);
	Free(Input_Buf);
	/* Read chip XML file */
    Path_Buf=Malloc(4096);
    sprintf(Path_Buf, "%s/MEukaron/Include/Platform/%s/Chips/%s/rme_platform_%s.xml",
                      RME_Path, Proj->Plat_Name, Proj->Chip_Class, Proj->Chip_Class);
	Input_Buf=Read_File(Path_Buf);
	Chip=Parse_Chip(Input_Buf);
	Free(Input_Buf);
    Free(Path_Buf);
    /* Decide platform functions */
    if(strcmp(Proj->Plat_Name,"A7M")==0)
        A7M_Plat_Select(Proj);
    else
		EXIT_FAIL("Other platforms not currently supported.");
    /* Parse general options of the architecture */
	Proj->Plat.Parse_Options(Proj,Chip);
    /* Check the general validity of everything */
    Check_Input(Proj, Chip);
	Proj->Plat.Check_Input(Proj,Chip);

/* Phase 2: Allocate kernel objects ******************************************/
	/* Align memory to what it should be */
	Proj->Plat.Align_Mem(Proj);
	/* Allocate and check code memory */
	Alloc_Code(Proj, Chip);
    Check_Code(Proj, Chip);
    /* Allocate data memory */
	Alloc_Data(Proj, Chip);
    /* Check device memory */
    Check_Device(Proj, Chip);
    /* Allocate the local and global capid of all kernel objects, except for page tables */
    Alloc_Captbl(Proj, Chip);
	/* Allocate page tables */
	Proj->Plat.Alloc_Pgtbl(Proj, Chip);
    /* Allocate kernel memory */
    Alloc_Mem(Proj);

/* Phase 3: Generate the project files ***************************************/
    /* Set the folder up */
    Setup_RME_Folder(Proj, Chip, RME_Path, Output_Path);
    Setup_RVM_Folder(Proj, Chip, RVM_Path, Output_Path);
    /* Set the configuration header up */
    Setup_RME_Conf(Proj, Chip, RME_Path, Output_Path);
    Setup_RVM_Conf(Proj, Chip, RVM_Path, Output_Path);
    /* Generate generic RME related files */
    Gen_RME_Boot(Proj, Chip, RME_Path, Output_Path);
    Gen_RME_User(Proj, Chip, RME_Path, Output_Path);
    /* Generate generic RVM related files */
    Gen_RVM_Boot(Proj, Chip, RVM_Path, Output_Path);
    Gen_RVM_User(Proj, Chip, RVM_Path, Output_Path);
    /* Generate generic files for every single project */
    Gen_Proj_User(Proj, Chip, RVM_Path, Output_Path);
    /* Generate target related files */
    Proj->Plat.Gen_Proj(Proj, Chip, Output_Path);
    
	Free_All();

    return 0;
}
/* End Function:main *********************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
