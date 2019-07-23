/******************************************************************************
Filename    : rme_genrme.cpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The rme folder generation class.
******************************************************************************/

/* Includes ******************************************************************/
#include "list"
#include "string"
#include "memory"
#include "vector"
#include "stdexcept"

#define __HDR_DEFS__
#include "Main/rme_mcu.hpp"

#include "Gen/rme_doc.hpp"
#include "Gen/rme_genrme.hpp"
#undef __HDR_DEFS__

#define __HDR_CLASSES__
#include "Gen/rme_doc.hpp"
#include "Gen/rme_genrme.hpp"
#undef __HDR_CLASSES__
/* End Includes **************************************************************/
namespace rme_mcu
{
/* Begin Function:RME_User::Read **********************************************
Description : Read the rme_user.c file, which contains all the user modifiable functions.
Input       : FILE* File - The file to read from.
Output      : None.
Return      : None.
******************************************************************************/
void RME_User::Read(FILE* File)
{
    /* Currently left empty - should construct the rme_user.c document tree */
}
/* End Function:RME_User::Read ***********************************************/

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

    return 0;
}
/* End Function:main *********************************************************/




}
/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
