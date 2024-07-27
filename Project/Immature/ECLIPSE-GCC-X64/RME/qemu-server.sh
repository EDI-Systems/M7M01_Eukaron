qemu-system-x86_64 \
-serial vc:800x600 \
-net none \
-smp 1,sockets=4,maxcpus=8 \
-numa node,nodeid=0 -numa node,nodeid=1 -numa node,nodeid=2 -numa node,nodeid=3 \
-m 512 \
-cdrom Debug/os.iso \
-monitor stdio
