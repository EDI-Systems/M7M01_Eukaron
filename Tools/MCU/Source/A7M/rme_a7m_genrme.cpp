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


/* Begin Function:RME_Gen::Setup_Conf *****************************************
Description : Crank the platform configuration headers for RME.
Input       : struct Proj_Info* Proj - The project structure.
              struct Chip_Info* Chip - The chip structure.
              s8_t* RME_Path - The RME root folder path.
              s8_t* Output_Path - The output folder path.
Output      : None.
Return      : None.
******************************************************************************/
void RME_Gen::Setup_Conf(struct Proj_Info* Proj, struct Chip_Info* Chip, s8_t* RME_Path, s8_t* Output_Path)
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
/* End Function:RME_Gen::Setup_Conf ******************************************/