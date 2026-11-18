# Config ######################################################################
TARGET=RME
CROSS ?= riscv64-unknown-elf-

CC=$(CROSS)gcc
OBJCOPY=$(CROSS)objcopy
CPU=-march=rv64gc -mabi=lp64d -mcmodel=medany

CFLAGS=-O2 -ffreestanding -fno-builtin -fno-pic -fno-strict-aliasing \
       -fno-common -nostdlib -fno-stack-protector -fno-jump-tables \
       -fno-tree-switch-conversion
WFLAGS=-Wall
DFLAGS=-g3
LDFLAGS=-T rme_platform_spacemit_k3_rt24.ld -static -nostdlib \
        -Wl,--build-id=none -Wl,-Map,$(OBJDIR)/$(TARGET).map
OBJDIR=Object
# End Config ##################################################################

# Source ######################################################################
INCS+=-I.
INCS+=-I../../Include

CSRCS+=../../Source/Kernel/rme_kernel.c
CSRCS+=../../Source/Platform/RV64V/rme_platform_rv64v.c
ASRCS+=../../Source/Platform/RV64V/rme_platform_rv64v_gcc.s
# End Source ##################################################################

# User ########################################################################
-include user
# End User ####################################################################

# User init program ############################################################
# The init program is written in Rust and lives in M7M03_Anomalocaris. The
# kernel embeds the resulting init.bin as an ELF object before the BSS.
RUST_INIT_DIR=../../../M7M03_Anomalocaris
RUST_INIT_BUILD=$(RUST_INIT_DIR)/build.sh
RUST_ORIG_INIT_BIN=$(RUST_INIT_DIR)/build/init.bin
INIT_BIN=$(OBJDIR)/init.bin
INIT_OBJ=$(OBJDIR)/init_image.o
# End User init program ########################################################

# Loader ######################################################################
# The standalone loader in M7M04_RT24Loader consumes the kernel image. It is
# built after the kernel so that the raw binary is always up to date.
RT24_LOADER_DIR=../../../M7M04_RT24Loader
# End Loader ##################################################################

# Build #######################################################################
COBJS+=$(CSRCS:%.c=%.o)
COBJS+=$(ASRCS:%.s=%.o)
CDEPS=$(CSRCS:%.c=%.d)

DEP=$(OBJDIR)/$(notdir $(@:%.o=%.d))
OBJ=$(OBJDIR)/$(notdir $@)

# Build all
all: mkdir $(COBJS) $(TARGET) $(TARGET).bin loader

# Create output folder
mkdir:
	$(shell if [ ! -e $(OBJDIR) ];then mkdir -p $(OBJDIR); fi)

# Compile C sources
%.o:%.c
	@echo "    CC      $(notdir $<)"
	@$(CC) -c $(CPU) $(INCS) $(CFLAGS) $(DFLAGS) $(WFLAGS) -MMD -MP -MF "$(DEP)" "$<" -o "$(OBJ)";

#Assemble asm file
%.o: %.s
	@echo "    AS      $(notdir $<)"
	@$(CC) -c $(CPU) $(INCS) $(CFLAGS) $(DFLAGS) $(WFLAGS) -MMD -MP -MF "$(DEP)" "$<" -o "$(OBJ)"

# User init program ############################################################
# Built by M7M03_Anomalocaris (Rust). Marked phony so the kernel always pulls
# the freshly built init.bin from the init project's build script.
.PHONY: $(INIT_BIN)
$(INIT_BIN):
	@echo "    RUST    $(notdir $(INIT_BIN)) via M7M03_Anomalocaris"
	@$(RUST_INIT_BUILD)
	@cp $(RUST_ORIG_INIT_BIN) $(INIT_BIN)

$(INIT_OBJ): $(INIT_BIN)
	@echo "    OBJCOPY $(notdir $@) (embed init image)"
	@$(OBJCOPY) -I binary -O elf64-littleriscv -B riscv --rename-section .data=.init_image "$<" "$@"
# End User init program ########################################################

$(TARGET): $(COBJS) $(INIT_OBJ) rme_platform_spacemit_k3_rt24.ld
	@echo "    ELF     $(notdir $@)"
	@$(CC) $(OBJDIR)/*.o $(CPU) $(LDFLAGS) -o $(OBJ)

$(TARGET).bin: $(TARGET)
	@echo "    BIN     $(notdir $@)"
	@$(OBJCOPY) -O binary -S $(OBJDIR)/$(TARGET) $(OBJDIR)/$(TARGET).bin

# Loader ######################################################################
# Build the standalone loader consuming the kernel image. The kernel raw binary
# is rebuilt first so that the loader always embeds the current image.
loader: $(OBJDIR)/$(TARGET).bin
	@echo "    LOADER  $(RT24_LOADER_DIR)"
	@$(MAKE) -C $(RT24_LOADER_DIR) RME_DIR="$(CURDIR)"
# End Loader ##################################################################

# Clean up
.PHONY: clean
clean:
	-rm -rf $(OBJDIR)

# Dependencies
-include $(wildcard $(OBJDIR)/*.d)
# End Build ###################################################################

# End Of File #################################################################
