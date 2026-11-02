
# Config ######################################################################
TARGET=RME
CPU=-march=rv64gc -mabi=lp64d

CC=riscv64-unknown-elf-gcc
OBJCOPY=riscv64-unknown-elf-objcopy
CFLAGS=-O2 -mcmodel=medany -static -fmessage-length=0 -ffreestanding -fno-pic -fno-builtin -fno-strict-aliasing -fno-common -nostdlib -fno-stack-protector
WFLAGS=-Wall
DFLAGS=-g3
LDFLAGS=-T rme_platform_spacemit_k3.ld -static -ffreestanding -fno-pic -fno-builtin -fno-strict-aliasing -fno-common -nostdlib -Wl,--build-id=none -fno-stack-protector -z noexecstack
OBJDIR=Object
# End Config ##################################################################

# Chip selection ##############################################################
# The chip is selected by this project's own rme_platform_rv64v_conf.h (found
# through -I.), which directly includes the SpacemiT K3 chip header. The
# bootstrap/direct-map 1GB window (K3 DRAM starts at 0x102000000, so the
# window is [0x100000000,0x140000000)) is RME_RV64V_KPGT_PA in that header,
# and the bootstrap root page table is built from it in C - the assembly
# needs no chip-specific value, so there is no build-time symbol here.
# End Chip selection ##########################################################

# Source ######################################################################
INCS+=-I.
INCS+=-I../../Include

CSRCS+=../../Source/Kernel/rme_kernel.c
CSRCS+=../../Source/Platform/RV64V/rme_platform_rv64v.c
ASRCS+=../../Source/Platform/RV64V/rme_platform_rv64v_gcc.s
# End Source ##################################################################

# User init program ############################################################
# The init program is written in Rust and lives in M7M03_Anomalocaris. On the
# K3 board there is no QEMU -device loader, so init.bin is converted into an
# ELF object (.init_image section) and embedded in the kernel image at the
# fixed physical address 0x102300000 by the linker script.
RUST_INIT_DIR=../../../M7M03_Anomalocaris
RUST_INIT_BUILD=$(RUST_INIT_DIR)/build.sh
RUST_ORIG_INIT_BIN=$(RUST_INIT_DIR)/build/init.bin
INIT_BIN=$(OBJDIR)/init.bin
INIT_OBJ=$(OBJDIR)/init_image.o
# End User init program ########################################################

# Build #######################################################################
COBJS+=$(CSRCS:%.c=%.o)
COBJS+=$(ASRCS:%.s=%.o)
CDEPS=$(CSRCS:%.c=%.d)

DEP=$(OBJDIR)/$(notdir $(@:%.o=%.d))
OBJ=$(OBJDIR)/$(notdir $@)

# Build all
all: mkdir $(COBJS) $(INIT_OBJ) $(TARGET) $(TARGET).bin

# Create output folder
mkdir:
	$(shell if [ ! -e $(OBJDIR) ];then mkdir -p $(OBJDIR); fi)

# Compile C sources
%.o:%.c
	@echo "    CC      $(notdir $<)"
	@$(CC) -c $(CPU) $(INCS) $(CFLAGS) $(DFLAGS) $(WFLAGS) -MMD -MP -MF "$(DEP)" "$<" -o "$(OBJ)";

# Assemble asm file - pure GAS; the bootstrap window base is a C constant in
# the chip header, so the assembly needs no chip-specific value and there is
# no ASFLAGS here.
%.o: %.s
	@echo "    AS      $(notdir $<)"
	@$(CC) -c $(CPU) $(INCS) $(CFLAGS) $(DFLAGS) $(ASFLAGS) $(WFLAGS) -MMD -MP -MF "$(DEP)" "$<" -o "$(OBJ)"

$(TARGET): $(COBJS) $(INIT_OBJ)
	@echo "    ELF     $(notdir $@)"
	@$(CC) $(OBJDIR)/*.o $(CPU) $(LIBS) $(LDFLAGS) -o $(OBJ)

$(TARGET).bin: $(TARGET)
	@echo "    BIN     $(notdir $@)"
	@$(OBJCOPY) -O binary -S $(OBJDIR)/$(TARGET) $(OBJDIR)/$(TARGET).bin

# User init program ############################################################
# Built by M7M03_Anomalocaris (Rust). Marked phony so the kernel always pulls
# the freshly built init.bin from the init project's build script.
.PHONY: $(INIT_BIN)
$(INIT_BIN):
	@echo "    RUST    $(notdir $(INIT_BIN)) via M7M03_Anomalocaris"
	@$(RUST_INIT_BUILD)
	@cp $(RUST_ORIG_INIT_BIN) $(INIT_BIN)

# Convert init.bin into an ELF object whose .init_image section the linker
# places at 0x102600000 (see rme_platform_spacemit_k3.ld). NOTE: objcopy
# must NOT be given --rename-section flags here - passing alloc/load/etc.
# zeroes the section content.
$(INIT_OBJ): $(INIT_BIN)
	@echo "    OBJCOPY $(notdir $(INIT_OBJ)) (embed init image)"
	@$(OBJCOPY) -I binary -O elf64-littleriscv -B riscv --rename-section .data=.init_image $(INIT_BIN) $(INIT_OBJ)
# End User init program ########################################################

# Clean up
.PHONY: clean
clean:
	-rm -rf $(OBJDIR)

# Dependencies
-include $(wildcard $(OBJDIR)/*.d)
# End Build ###################################################################

# End Of File #################################################################
