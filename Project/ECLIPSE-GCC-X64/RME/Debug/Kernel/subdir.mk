################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/MEukaron/Kernel/captbl.c \
/media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/MEukaron/Kernel/kernel.c \
/media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/MEukaron/Kernel/kotbl.c \
/media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/MEukaron/Kernel/pgtbl.c \
/media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/MEukaron/Kernel/prcthd.c \
/media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/MEukaron/Kernel/siginv.c 

OBJS += \
./Kernel/captbl.o \
./Kernel/kernel.o \
./Kernel/kotbl.o \
./Kernel/pgtbl.o \
./Kernel/prcthd.o \
./Kernel/siginv.o 

C_DEPS += \
./Kernel/captbl.d \
./Kernel/kernel.d \
./Kernel/kotbl.d \
./Kernel/pgtbl.d \
./Kernel/prcthd.d \
./Kernel/siginv.d 


# Each subdirectory must supply rules for building sources it contributes
Kernel/captbl.o: /media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/MEukaron/Kernel/captbl.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -m64 -mcmodel=kernel -nostdlib -D"likely(x)=__builtin_expect(!!(x), 1)" -D"unlikely(x)=__builtin_expect(!!(x), 0)" -I/media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/Project/ECLIPSE-GCC-X64/RME/../../../MEukaron/Include -O3 -mno-sse -g3 -Wall -c -fmessage-length=0 -fno-pic -static -fno-builtin -fno-strict-aliasing -ffreestanding -fno-common -fno-stack-protector -mtls-direct-seg-refs -mno-red-zone -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Kernel/kernel.o: /media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/MEukaron/Kernel/kernel.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -m64 -mcmodel=kernel -nostdlib -D"likely(x)=__builtin_expect(!!(x), 1)" -D"unlikely(x)=__builtin_expect(!!(x), 0)" -I/media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/Project/ECLIPSE-GCC-X64/RME/../../../MEukaron/Include -O3 -mno-sse -g3 -Wall -c -fmessage-length=0 -fno-pic -static -fno-builtin -fno-strict-aliasing -ffreestanding -fno-common -fno-stack-protector -mtls-direct-seg-refs -mno-red-zone -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Kernel/kotbl.o: /media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/MEukaron/Kernel/kotbl.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -m64 -mcmodel=kernel -nostdlib -D"likely(x)=__builtin_expect(!!(x), 1)" -D"unlikely(x)=__builtin_expect(!!(x), 0)" -I/media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/Project/ECLIPSE-GCC-X64/RME/../../../MEukaron/Include -O3 -mno-sse -g3 -Wall -c -fmessage-length=0 -fno-pic -static -fno-builtin -fno-strict-aliasing -ffreestanding -fno-common -fno-stack-protector -mtls-direct-seg-refs -mno-red-zone -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Kernel/pgtbl.o: /media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/MEukaron/Kernel/pgtbl.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -m64 -mcmodel=kernel -nostdlib -D"likely(x)=__builtin_expect(!!(x), 1)" -D"unlikely(x)=__builtin_expect(!!(x), 0)" -I/media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/Project/ECLIPSE-GCC-X64/RME/../../../MEukaron/Include -O3 -mno-sse -g3 -Wall -c -fmessage-length=0 -fno-pic -static -fno-builtin -fno-strict-aliasing -ffreestanding -fno-common -fno-stack-protector -mtls-direct-seg-refs -mno-red-zone -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Kernel/prcthd.o: /media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/MEukaron/Kernel/prcthd.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -m64 -mcmodel=kernel -nostdlib -D"likely(x)=__builtin_expect(!!(x), 1)" -D"unlikely(x)=__builtin_expect(!!(x), 0)" -I/media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/Project/ECLIPSE-GCC-X64/RME/../../../MEukaron/Include -O3 -mno-sse -g3 -Wall -c -fmessage-length=0 -fno-pic -static -fno-builtin -fno-strict-aliasing -ffreestanding -fno-common -fno-stack-protector -mtls-direct-seg-refs -mno-red-zone -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Kernel/siginv.o: /media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/MEukaron/Kernel/siginv.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -m64 -mcmodel=kernel -nostdlib -D"likely(x)=__builtin_expect(!!(x), 1)" -D"unlikely(x)=__builtin_expect(!!(x), 0)" -I/media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/Project/ECLIPSE-GCC-X64/RME/../../../MEukaron/Include -O3 -mno-sse -g3 -Wall -c -fmessage-length=0 -fno-pic -static -fno-builtin -fno-strict-aliasing -ffreestanding -fno-common -fno-stack-protector -mtls-direct-seg-refs -mno-red-zone -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


