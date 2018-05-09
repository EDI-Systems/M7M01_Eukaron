################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/media/pry/Code/Code_Library/MCU/Mutatus/M7M3_MuRex/Project/ECLIPSE-GCC-X64/UVM/Debug/UVM.c 

OBJS += \
./User/UVM.o 

C_DEPS += \
./User/UVM.d 


# Each subdirectory must supply rules for building sources it contributes
User/UVM.o: /media/pry/Code/Code_Library/MCU/Mutatus/M7M3_MuRex/Project/ECLIPSE-GCC-X64/UVM/Debug/UVM.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -m64 -mcmodel=kernel -nostdlib -I/media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/Project/ECLIPSE-GCC-X64/RME/../../../MEukaron/Include -O3 -mno-sse -g3 -Wall -c -fmessage-length=0 -fno-pic -static -fno-builtin -fno-strict-aliasing -ffreestanding -fno-common -fno-stack-protector -mtls-direct-seg-refs -mno-red-zone -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


