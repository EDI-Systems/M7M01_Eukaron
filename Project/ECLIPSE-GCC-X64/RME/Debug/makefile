################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Platform/subdir.mk
-include Kernel/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all:
	$(MAKE) --no-print-directory pre-build
	$(MAKE) --no-print-directory main-build

# Main-build Target
main-build: RME

# Tool invocations
RME: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	ld -Map RME.map -mcmodel=kernel -nodefaultlibs -m elf_x86_64 -T /media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/Project/ECLIPSE-GCC-X64/RME/../../../MEukaron/Include/Platform/X64/platform_x64.ld -o "RME" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) RME
	-@echo ' '

pre-build:
	-../copy.sh
	-@echo ' '

post-build:
	-../iso.sh
	-@echo ' '

.PHONY: all clean dependents main-build pre-build post-build

-include ../makefile.targets
