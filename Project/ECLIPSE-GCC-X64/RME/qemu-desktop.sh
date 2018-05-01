# typical desktop system, 1 numa node, 1 socket, 4 processors, 8GB RAM */
qemu-system-x86_64 -serial vc:800x600 -net none -smp 2 -m 8192 -cdrom Debug/os.iso -monitor stdio -d int -D qemu-log.txt
