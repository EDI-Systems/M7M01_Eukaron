# Config ######################################################################
TARGET=UVM
CPU=-m64

CC=gcc
CFLAGS=-O3 -static -fmessage-length=0 -ffreestanding -fno-pic -fno-builtin -fno-strict-aliasing -fno-common -nostdlib -fno-stack-protector -mtls-direct-seg-refs -mno-red-zone
WFLAGS=-Wall
DFLAGS=-g3
LDFLAGS=-T ./Include/Platform/X64/uvmlib_x64.ld -static -ffreestanding -fno-pic -fno-builtin -fno-strict-aliasing -fno-common -nostdlib -Wl,--build-id=none -fno-stack-protector -z noexecstack
OBJDIR=Object
# End Config ##################################################################

# Source ######################################################################
INCS+=-I/usr/include/x86_64-linux-gnu
INCS+=-I./Include
INCS+=-I../Include

CSRCS+=./Platform/X64/uvmlib_x64.c
CSRCS+=./Init/init.c
CSRCS+=./Init/syssvc.c

ASRCS+=./Platform/X64/uvmlib_x64_asm.s
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
