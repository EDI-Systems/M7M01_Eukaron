#!/bin/bash
# Run the RV64V kernel with its embedded init image on QEMU virt.
set -e
cd "$(dirname "$0")"
SMP="${SMP:-1}"
make
qemu-system-riscv64 -machine virt -smp "$SMP" -m 512M -bios default \
    -kernel Object/RME.bin \
    -icount shift=3 -nographic
