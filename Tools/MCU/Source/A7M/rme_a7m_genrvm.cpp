
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
