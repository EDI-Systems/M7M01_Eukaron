# Config ######################################################################
TARGET=RME
CPU=-m64

CC=gcc
CFLAGS=-O2 -mcmodel=kernel -static -fmessage-length=0 -ffreestanding -fno-pic -fno-builtin -fno-strict-aliasing -fno-common -nostdlib -fno-stack-protector -mtls-direct-seg-refs -mno-red-zone
WFLAGS=-Wall
DFLAGS=-g3
LDFLAGS=-T ../../Include/Platform/X64/rme_platform_x64.ld -static -ffreestanding -fno-pic -fno-builtin -fno-strict-aliasing -fno-common -nostdlib -Wl,--build-id=none -fno-stack-protector -z noexecstack
OBJDIR=Object
# End Config ##################################################################

# Source ######################################################################
INCS+=-I.
INCS+=-I/usr/include/x86_64-linux-gnu
INCS+=-I../../Include

CSRCS+=./UVM.c
CSRCS+=../../Source/Kernel/rme_kernel.c
CSRCS+=../../Source/Platform/X64/rme_platform_x64.c
ASRCS+=../../Source/Platform/X64/rme_platform_x64_gcc.s
LIBS=-lpthread
# End Source ##################################################################

# User ########################################################################
-include user
# End User ####################################################################

# Build #######################################################################
COBJS+=$(CSRCS:%.c=%.o)
COBJS+=$(ASRCS:%.s=%.o)
CDEPS=$(CSRCS:%.c=%.d)

DEP=$(OBJDIR)/$(notdir $(@:%.o=%.d))
OBJ=$(OBJDIR)/$(notdir $@)

# Build all
all: mkdir $(COBJS) $(TARGET)

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
	@echo "    BIN     $(notdir $@)"
	@$(CC) $(OBJDIR)/*.o $(CPU) $(LIBS) $(LDFLAGS) -o $(OBJ)

# Clean up
.PHONY: clean
clean:
	-rm -rf $(OBJDIR)

# Dependencies
-include $(wildcard $(OBJDIR)/*.d)
# End Build ###################################################################

# End Of File #################################################################
