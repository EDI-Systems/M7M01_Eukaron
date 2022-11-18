################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
main.obj: ../main.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C6000 Compiler'
	"D:/Program_Files_x86/TI/ccs740/ccsv7/tools/compiler/ti-cgt-c6000_8.2.2/bin/cl6x" -mv6600 --include_path="F:/Code_Library/MCU/Mutatus/M7M1_MuEukaron/Project/CCS-TMS320C6678/RME" --include_path="D:/Program_Files_x86/TI/ccs740/ccsv7/tools/compiler/ti-cgt-c6000_8.2.2/include" -g --diag_warning=225 --diag_wrap=off --display_error_number --preproc_with_compile --preproc_dependency="main.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


