################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/MEukaron/Kernel/rme_captbl.c \
/media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/MEukaron/Kernel/rme_kernel.c \
/media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/MEukaron/Kernel/rme_kotbl.c \
/media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/MEukaron/Kernel/rme_pgtbl.c \
/media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/MEukaron/Kernel/rme_prcthd.c \
/media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/MEukaron/Kernel/rme_siginv.c 

OBJS += \
./Kernel/rme_captbl.o \
./Kernel/rme_kernel.o \
./Kernel/rme_kotbl.o \
./Kernel/rme_pgtbl.o \
./Kernel/rme_prcthd.o \
./Kernel/rme_siginv.o 

C_DEPS += \
./Kernel/rme_captbl.d \
./Kernel/rme_kernel.d \
./Kernel/rme_kotbl.d \
./Kernel/rme_pgtbl.d \
./Kernel/rme_prcthd.d \
./Kernel/rme_siginv.d 


# Each subdirectory must supply rules for building sources it contributes
Kernel/rme_captbl.o: /media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/MEukaron/Kernel/rme_captbl.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -m64 -mcmodel=kernel -nostdlib -D"likely(x)=__builtin_expect(!!(x), 1)" -D"unlikely(x)=__builtin_expect(!!(x), 0)" -I/media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/Project/ECLIPSE-GCC-X64/RME/../../../MEukaron/Include -O3 -mno-sse -g3 -Wall -c -fmessage-length=0 -fno-pic -static -fno-builtin -fno-strict-aliasing -ffreestanding -fno-common -fno-stack-protector -mtls-direct-seg-refs -mno-red-zone -Wno-maybe-uninitialized -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Kernel/rme_kernel.o: /media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/MEukaron/Kernel/rme_kernel.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -m64 -mcmodel=kernel -nostdlib -D"likely(x)=__builtin_expect(!!(x), 1)" -D"unlikely(x)=__builtin_expect(!!(x), 0)" -I/media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/Project/ECLIPSE-GCC-X64/RME/../../../MEukaron/Include -O3 -mno-sse -g3 -Wall -c -fmessage-length=0 -fno-pic -static -fno-builtin -fno-strict-aliasing -ffreestanding -fno-common -fno-stack-protector -mtls-direct-seg-refs -mno-red-zone -Wno-maybe-uninitialized -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Kernel/rme_kotbl.o: /media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/MEukaron/Kernel/rme_kotbl.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -m64 -mcmodel=kernel -nostdlib -D"likely(x)=__builtin_expect(!!(x), 1)" -D"unlikely(x)=__builtin_expect(!!(x), 0)" -I/media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/Project/ECLIPSE-GCC-X64/RME/../../../MEukaron/Include -O3 -mno-sse -g3 -Wall -c -fmessage-length=0 -fno-pic -static -fno-builtin -fno-strict-aliasing -ffreestanding -fno-common -fno-stack-protector -mtls-direct-seg-refs -mno-red-zone -Wno-maybe-uninitialized -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Kernel/rme_pgtbl.o: /media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/MEukaron/Kernel/rme_pgtbl.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -m64 -mcmodel=kernel -nostdlib -D"likely(x)=__builtin_expect(!!(x), 1)" -D"unlikely(x)=__builtin_expect(!!(x), 0)" -I/media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/Project/ECLIPSE-GCC-X64/RME/../../../MEukaron/Include -O3 -mno-sse -g3 -Wall -c -fmessage-length=0 -fno-pic -static -fno-builtin -fno-strict-aliasing -ffreestanding -fno-common -fno-stack-protector -mtls-direct-seg-refs -mno-red-zone -Wno-maybe-uninitialized -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Kernel/rme_prcthd.o: /media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/MEukaron/Kernel/rme_prcthd.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -m64 -mcmodel=kernel -nostdlib -D"likely(x)=__builtin_expect(!!(x), 1)" -D"unlikely(x)=__builtin_expect(!!(x), 0)" -I/media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/Project/ECLIPSE-GCC-X64/RME/../../../MEukaron/Include -O3 -mno-sse -g3 -Wall -c -fmessage-length=0 -fno-pic -static -fno-builtin -fno-strict-aliasing -ffreestanding -fno-common -fno-stack-protector -mtls-direct-seg-refs -mno-red-zone -Wno-maybe-uninitialized -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Kernel/rme_siginv.o: /media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/MEukaron/Kernel/rme_siginv.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -m64 -mcmodel=kernel -nostdlib -D"likely(x)=__builtin_expect(!!(x), 1)" -D"unlikely(x)=__builtin_expect(!!(x), 0)" -I/media/pry/Code/Code_Library/MCU/Mutatus/M7M1_MuEukaron/Project/ECLIPSE-GCC-X64/RME/../../../MEukaron/Include -O3 -mno-sse -g3 -Wall -c -fmessage-length=0 -fno-pic -static -fno-builtin -fno-strict-aliasing -ffreestanding -fno-common -fno-stack-protector -mtls-direct-seg-refs -mno-red-zone -Wno-maybe-uninitialized -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


