# Build inputs and outputs
.DEFAULT_GOAL := all
OBJDIR ?= Object
UVM_SRC ?= ../../../RME_X64_UVM/UVM.c

KERNEL_DIR := ../../Source/Kernel
PLATFORM_DIR := ../../Source/Platform/X64
LINKER_SCRIPT := ../../Include/Platform/X64/rme_platform_x64.ld
BUILD_FILE := $(lastword $(MAKEFILE_LIST))

ELF := $(OBJDIR)/RME
C_OBJS := $(OBJDIR)/UVM.o \
          $(OBJDIR)/rme_kernel.o \
          $(OBJDIR)/rme_platform_x64.o
BOOT_OBJ := $(OBJDIR)/rme_platform_x64_gcc.o
OBJS := $(C_OBJS) $(BOOT_OBJ)

# Compiler and linker flags
CC := gcc
ARCH_FLAGS := -m64
CPPFLAGS += -I. -I../../Include
CFLAGS ?= -O3 -g3 -Wall
ASFLAGS ?= -g3
KERNEL_CFLAGS := -mcmodel=kernel -ffreestanding -fno-pic \
                 -fno-strict-aliasing -fno-common -fno-stack-protector \
                 -mtls-direct-seg-refs -mno-red-zone -mgeneral-regs-only
LDFLAGS += -static -nostdlib -no-pie -Wl,--build-id=none -z max-page-size=0x1000 -z noexecstack

.PHONY: all RME clean
.DELETE_ON_ERROR:

# Build targets
all RME: $(ELF)

$(ELF): $(OBJS) $(LINKER_SCRIPT)
	@printf '  LD  %s\n' "$@"
	@$(CC) $(ARCH_FLAGS) $(LDFLAGS) -T "$(LINKER_SCRIPT)" $(OBJS) -o "$@"

$(OBJDIR)/UVM.o: $(UVM_SRC)
$(OBJDIR)/rme_kernel.o: $(KERNEL_DIR)/rme_kernel.c
$(OBJDIR)/rme_platform_x64.o: $(PLATFORM_DIR)/rme_platform_x64.c

$(C_OBJS):
	@printf '  CC  %s\n' "$(notdir $<)"
	@$(CC) $(ARCH_FLAGS) $(CPPFLAGS) $(KERNEL_CFLAGS) $(CFLAGS) -MMD -MP -c "$<" -o "$@"

$(BOOT_OBJ): $(PLATFORM_DIR)/rme_platform_x64_gcc.s
	@printf '  AS  %s\n' "$(notdir $<)"
	@$(CC) $(ARCH_FLAGS) $(ASFLAGS) -c "$<" -o "$@"

#Object directory creation
$(OBJS): $(BUILD_FILE) | $(OBJDIR)

$(OBJDIR):
	mkdir -p -- "$@"

# Clean up
clean:
	rm -f -- "$(ELF)" $(OBJS) $(OBJS:.o=.d)

-include $(C_OBJS:.o=.d)
