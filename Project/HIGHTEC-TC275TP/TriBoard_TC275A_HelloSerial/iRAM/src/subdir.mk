################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/hello.c \
../src/system_tc27x.c \
../src/uart_init_poll.c \
../src/uart_poll.c \
../src/usr_sprintf.c 

OBJS += \
./src/hello.o \
./src/system_tc27x.o \
./src/uart_init_poll.o \
./src/uart_poll.o \
./src/usr_sprintf.o 

C_DEPS += \
./src/hello.d \
./src/system_tc27x.d \
./src/uart_init_poll.d \
./src/uart_poll.d \
./src/usr_sprintf.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: TriCore C Compiler'
	"$(TRICORE_TOOLS)/bin/tricore-gcc" -c -I"../h" -fno-common -Os -g3 -W -Wall -Wextra -Wdiv-by-zero -Warray-bounds -Wcast-align -Wignored-qualifiers -Wformat -Wformat-security -DTRIBOARD_TC275A -DBAUDRATE=38400 -DMINIMAL_CODE -fshort-double -mcpu=tc27xx -mversion-info -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


