#!/bin/bash
# Run the RV64V kernel on QEMU virt. The port is hard-wired for 8 harts;
# set SMP to a smaller value only for debugging.
SMP="${SMP:-8}"
qemu-system-riscv64 -machine virt -smp "$SMP" -m 512M -bios default \
    -kernel Object/RME.bin \
    -device loader,file=Object/init.bin,addr=0x81030000 \
    -icount shift=3 -nographic
