#!/bin/bash
qemu-system-riscv64 -machine virt -smp 1 -m 512M -bios default -kernel Object/RME.bin -device loader,file=Object/init.bin,addr=0x81030000 -nographic