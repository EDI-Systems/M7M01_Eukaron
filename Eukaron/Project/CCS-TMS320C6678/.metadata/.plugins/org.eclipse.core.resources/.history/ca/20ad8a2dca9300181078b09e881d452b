################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := D:/Program_Files_x86/TI/ccs740/ccsv7/tools/compiler/ti-cgt-c6000_8.2.2

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./Kernel/rme_captbl.obj" \
"./Kernel/rme_kernel.obj" \
"./Kernel/rme_kotbl.obj" \
"./Kernel/rme_pgtbl.obj" \
"./Kernel/rme_prcthd.obj" \
"./Kernel/rme_siginv.obj" \
"./Platform/rme_platform_c66x.obj" \
"./Platform/rme_platform_c66x_asm.obj" \
"F:/Code_Library/MCU/Mutatus/M7M1_MuEukaron/MEukaron/Include/Platform/C66X/Chips/TMS320C6678/rme_platform_TMS320C6678.cmd" \
$(GEN_CMDS__FLAG) \
-llibc.a \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include Kernel/subdir_vars.mk
-include Platform/subdir_vars.mk
-include Kernel/subdir_rules.mk
-include Platform/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
RME.out \

EXE_OUTPUTS__QUOTED += \
"RME.out" \

BIN_OUTPUTS += \
RME.hex \

BIN_OUTPUTS__QUOTED += \
"RME.hex" \


# All Target
all:
	@$(MAKE) --no-print-directory pre-build
	@$(MAKE) --no-print-directory main-build

# Main-build Target
main-build: $(OBJS) $(GEN_CMDS)
	@$(MAKE) --no-print-directory -Onone "RME.out"

# Tool invocations
RME.out: $(OBJS) $(GEN_CMDS)
	@echo 'Building target: "$@"'
	@echo 'Invoking: C6000 Linker'
	"D:/Program_Files_x86/TI/ccs740/ccsv7/tools/compiler/ti-cgt-c6000_8.2.2/bin/cl6x" -mv6600 --define=likely(x)="__builtin_expect(!!(x), 1)" --define=unlikely(x)="__builtin_expect(!!(x), 0)" -g --diag_warning=225 --diag_wrap=off --display_error_number --mem_model:data=far -z -m"RME.map" -i"D:/Program_Files_x86/TI/ccs740/ccsv7/tools/compiler/ti-cgt-c6000_8.2.2/lib" -i"D:/Program_Files_x86/TI/ccs740/ccsv7/tools/compiler/ti-cgt-c6000_8.2.2/include" --reread_libs --diag_suppress=10068 --diag_suppress=10261 --diag_wrap=off --display_error_number --issue_remarks --warn_sections --xml_link_info="RME_linkInfo.xml" --rom_model -o "RME.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '

RME.hex: $(EXE_OUTPUTS)
	@echo 'Building files: $(strip $(EXE_OUTPUTS__QUOTED))'
	@echo 'Invoking: C6000 Hex Utility'
	"D:/Program_Files_x86/TI/ccs740/ccsv7/tools/compiler/ti-cgt-c6000_8.2.2/bin/hex6x"  -o "RME.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building: $(strip $(EXE_OUTPUTS__QUOTED))'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(BIN_OUTPUTS__QUOTED)$(EXE_OUTPUTS__QUOTED)
	-$(RM) "Kernel\rme_captbl.obj" "Kernel\rme_kernel.obj" "Kernel\rme_kotbl.obj" "Kernel\rme_pgtbl.obj" "Kernel\rme_prcthd.obj" "Kernel\rme_siginv.obj" "Platform\rme_platform_c66x.obj" "Platform\rme_platform_c66x_asm.obj" 
	-$(RM) "Kernel\rme_captbl.d" "Kernel\rme_kernel.d" "Kernel\rme_kotbl.d" "Kernel\rme_pgtbl.d" "Kernel\rme_prcthd.d" "Kernel\rme_siginv.d" "Platform\rme_platform_c66x.d" 
	-$(RM) "Platform\rme_platform_c66x_asm.d" 
	-@echo 'Finished clean'
	-@echo ' '

pre-build:
	-F:\Code_Library\MCU\Mutatus\M7M1_MuEukaron\Project\CCS-TMS320C6678\RME/copy.bat
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

