################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/MEukaron/Platform/X64/platform_x64.c 

S_UPPER_SRCS += \
/media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/MEukaron/Platform/X64/platform_x64_asm.S 

OBJS += \
./Platform/platform_x64.o \
./Platform/platform_x64_asm.o 

C_DEPS += \
./Platform/platform_x64.d 


# Each subdirectory must supply rules for building sources it contributes
Platform/platform_x64.o: /media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/MEukaron/Platform/X64/platform_x64.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -m64 -mcmodel=kernel -nostdlib -D"likely(x)=__builtin_expect(!!(x), 1)" -D"unlikely(x)=__builtin_expect(!!(x), 0)" -I/media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/Project/ECLIPSE-GCC-X64/RME/../../../MEukaron/Include -O3 -mno-sse -g3 -Wall -c -fmessage-length=0 -fno-pic -static -fno-builtin -fno-strict-aliasing -ffreestanding -fno-common -fno-stack-protector -mtls-direct-seg-refs -mno-red-zone -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Platform/platform_x64_asm.o: /media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/MEukaron/Platform/X64/platform_x64_asm.S
	@echo 'Building file: $<'
	@echo 'Invoking: GCC Assembler'
	gcc -c -fmessage-length=0 -fno-pic -static -fno-builtin -fno-strict-aliasing -ffreestanding -fno-common -nostdlib -fno-stack-protector -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


