
# Config ######################################################################
TARGET=RME
CPU=-march=rv64gc -mabi=lp64d

CC=riscv64-unknown-elf-gcc
OBJCOPY=riscv64-unknown-elf-objcopy
CFLAGS=-O2 -mcmodel=medany -static -fmessage-length=0 -ffreestanding -fno-pic -fno-builtin -fno-strict-aliasing -fno-common -nostdlib -fno-stack-protector
WFLAGS=-Wall
DFLAGS=-g3
LDFLAGS=-T rme_platform_qemu_virt.ld -static -ffreestanding -fno-pic -fno-builtin -fno-strict-aliasing -fno-common -nostdlib -Wl,--build-id=none -fno-stack-protector -z noexecstack
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
# kernel references that project: this build invokes its build script and
# copies the emitted init.bin into our Object/ folder.
RUST_INIT_DIR=../../../M7M03_Anomalocaris
RUST_INIT_BUILD=$(RUST_INIT_DIR)/build.sh
RUST_ORIG_INIT_BIN=$(RUST_INIT_DIR)/build/init.bin
INIT_BIN=$(OBJDIR)/init.bin
# End User init program ########################################################

# Build #######################################################################
COBJS+=$(CSRCS:%.c=%.o)
COBJS+=$(ASRCS:%.s=%.o)
CDEPS=$(CSRCS:%.c=%.d)

DEP=$(OBJDIR)/$(notdir $(@:%.o=%.d))
OBJ=$(OBJDIR)/$(notdir $@)

# Build all
all: mkdir $(COBJS) $(TARGET) $(TARGET).bin $(INIT_BIN)

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

$(TARGET): $(COBJS)
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
# End User init program ########################################################
	
# Clean up
.PHONY: clean
clean:
	-rm -rf $(OBJDIR)

# Dependencies
-include $(wildcard $(OBJDIR)/*.d)
# End Build ###################################################################

# End Of File #################################################################
